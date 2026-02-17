#ifndef CPP20_DISPATCH_HPP
#define CPP20_DISPATCH_HPP

#include <cpp20.hpp>
#include <tuple>
#include <utility>

namespace cpp20 {

namespace internal {

// Function traits
template <typename> struct fn_traits;

template <typename Ret, typename... Args>
struct fn_traits<Ret(*)(Args...)> {
    using return_type = Ret;
    using args_tuple = std::tuple<Args...>;
    static constexpr size_t arity = sizeof...(Args);
};

// Convert SEXP to C++ type
template <typename T>
T sexp_to_cpp(SEXP x) {
    return as<std::remove_cvref_t<T>>(x);
}

using r_types = std::tuple<
    r_lgl,
    r_int, 
    r_int64, 
    r_dbl, 
    r_str, 
    r_str_view, 
    r_cplx, 
    r_raw, 
    r_sym, 
    r_sexp
>;

// Helper to get the runtime R type ID for a C++ type
template<typename T> constexpr int r_type_id_v = -1;

template<> constexpr int r_type_id_v<r_lgl> = LGLSXP;
template<> constexpr int r_type_id_v<r_int> = INTSXP;
template<> constexpr int r_type_id_v<r_int64> = CPP20_INT64SXP;
template<> constexpr int r_type_id_v<r_dbl> = REALSXP;
template<> constexpr int r_type_id_v<r_str_view> = STRSXP;
template<> constexpr int r_type_id_v<r_str> = STRSXP;
template<> constexpr int r_type_id_v<r_cplx> = CPLXSXP;
template<> constexpr int r_type_id_v<r_raw> = RAWSXP;
template<> constexpr int r_type_id_v<r_sym> = SYMSXP;
template<> constexpr int r_type_id_v<r_sexp> = VECSXP;


// r_vec<T> should also map to types above
template <typename T>
constexpr int r_type_id_v<r_vec<T>> = r_type_id_v<T>;

// Pure C++ types (like int or std::string)
template <typename T>
requires (CastableToRVal<T>)
constexpr int r_type_id_v<T> = r_type_id_v<as_r_val_t<T>>;

// Helper to get Nth element from parameter pack
template <size_t N, typename... Args>
decltype(auto) get_nth(Args&&... args) {
    return std::get<N>(std::forward_as_tuple(std::forward<Args>(args)...));
}

// Base case: All types selected, try to call the functor
template <size_t Remaining, auto Indices, typename... SelectedTypes>
struct MultiDispatcher {
    template <typename Functor, typename... SexpArgs>
    static SEXP dispatch(Functor&& functor, SexpArgs&&... sexp_args) {
        // All types selected - check if functor accepts this combination
        if constexpr (requires { functor.template operator()<SelectedTypes...>(sexp_args...); }) {
            return functor.template operator()<SelectedTypes...>(sexp_args...);
        } else {
            return R_NilValue;
        }
    }
};

// Recursive case: Still have arguments to dispatch
template <size_t Remaining, auto Indices, typename... SelectedTypes>
requires (Remaining > 0)
struct MultiDispatcher<Remaining, Indices, SelectedTypes...> {
    template <typename Functor, typename... SexpArgs>
    static SEXP dispatch(Functor&& functor, SexpArgs&&... sexp_args) {
        // Which template param are we on?
        constexpr size_t template_param_index = Indices.size() - Remaining;
        
        // Which actual argument position does this correspond to?
        constexpr size_t arg_position = Indices[template_param_index];
        
        // Get the SEXP at that position
        SEXP current_sexp = get_nth<arg_position>(sexp_args...);
        int type = CPP20_TYPEOF(current_sexp);
        
        SEXP result = R_NilValue;
        
        // Try each type in r_types for this argument
        auto try_type = [&]<typename T>() {
            if (result != R_NilValue) return;
            if (type != r_type_id_v<T>) return;
            
            // Try scalar T
            result = MultiDispatcher<Remaining - 1, Indices, SelectedTypes..., T>::dispatch(
                std::forward<Functor>(functor), 
                std::forward<SexpArgs>(sexp_args)...
            );
            
            // If scalar didn't work, try r_vec<T>
            if (result == R_NilValue) {
                result = MultiDispatcher<Remaining - 1, Indices, SelectedTypes..., r_vec<T>>::dispatch(
                    std::forward<Functor>(functor), 
                    std::forward<SexpArgs>(sexp_args)...
                );
            }
        };
        
        // Iterate over all types
        constexpr size_t N = std::tuple_size_v<r_types>;
        [&]<size_t... Is>(std::index_sequence<Is...>) {
            (try_type.template operator()<std::tuple_element_t<Is, r_types>>(), ...);
        }(std::make_index_sequence<N>{});
        
        return result;
    }
};

// Entry point: dispatch_template_impl<N>(functor, sexp1, sexp2, ...)
template <size_t N, std::array<size_t, N> Indices, typename Functor, typename... SexpArgs>
SEXP dispatch_template_impl(Functor&& functor, SexpArgs&&... sexp_args) {
    SEXP result = MultiDispatcher<N, Indices>::dispatch(
        std::forward<Functor>(functor), 
        std::forward<SexpArgs>(sexp_args)...
    );
    
    if (result == R_NilValue) {
        abort("No matching template instantiation found for input types");
    }
    
    return result;
}


// Invoke with index sequence
template <auto Fn, typename Ret, typename... Args, size_t... Is>
SEXP invoke_impl(SEXP* sexp_args, std::index_sequence<Is...>) {
    if constexpr (std::is_void_v<Ret>) {
        Fn(sexp_to_cpp<Args>(sexp_args[Is])...);
        return R_NilValue;
    } else {
        return unwrap(as<r_sexp>(Fn(sexp_to_cpp<Args>(sexp_args[Is])...)));
    }
}

}

// Main dispatch - unpack tuple types directly
template <auto Fn, typename... SexpArgs>
SEXP dispatch(SexpArgs... args) {
    using Traits = internal::fn_traits<decltype(Fn)>;
    using ArgsTuple = typename Traits::args_tuple;
    
    static_assert(sizeof...(SexpArgs) == Traits::arity, 
                  "Argument count mismatch");
    
    SEXP arg_array[] = {args...};
    
    // Unpack tuple types using helper
    return []<typename... Args>(SEXP* arr, std::tuple<Args...>*) {
        return internal::invoke_impl<Fn, typename Traits::return_type, Args...>(
            arr, std::make_index_sequence<sizeof...(Args)>{}
        );
    }(arg_array, static_cast<ArgsTuple*>(nullptr));
}

} // namespace cpp20

#endif
