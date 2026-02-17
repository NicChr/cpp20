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
    return as<std::decay_t<T>>(x);
}

using r_types = std::tuple<
    r_lgl, r_int, r_int64, r_dbl, r_str, r_str_view, r_cplx, r_raw, r_sym, r_sexp,
    bool, int, int64_t, double, const char*, std::complex<double>, Rbyte
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

template<> constexpr int r_type_id_v<bool> = LGLSXP;
template<> constexpr int r_type_id_v<int> = INTSXP;
template<> constexpr int r_type_id_v<int64_t> = CPP20_INT64SXP;
template<> constexpr int r_type_id_v<double> = REALSXP;
template<> constexpr int r_type_id_v<const char*> = STRSXP;
template<> constexpr int r_type_id_v<std::complex<double>> = CPLXSXP;
template<> constexpr int r_type_id_v<Rbyte> = RAWSXP;


template <typename Functor, typename... Args>
SEXP dispatch_template_impl(Functor&& functor, SEXP x, Args... args) {
    int type = CPP20_TYPEOF(x);
    SEXP result = R_NilValue;
    bool found = false;

    // Helper to process one type T
    auto try_type = [&]<typename T>() {
        if (!found && type == r_type_id_v<T>) {
            if constexpr (requires { functor.template operator()<T>(x, args...); }) {
                result = functor.template operator()<T>(x, args...);
                found = true;
            }
        }
    };

    // Iterate over types using index sequence
    constexpr size_t N = std::tuple_size_v<r_types>;
    [&]<size_t... Is>(std::index_sequence<Is...>) {
        (try_type.template operator()<std::tuple_element_t<Is, r_types>>(), ...);
    }(std::make_index_sequence<N>{});

    if (!found) abort("No matching template instantiation found for input type");
    return result;
}

// template <typename Functor>
// SEXP dispatch_template_impl(Functor&& functor, SEXP x) {
//     int type = CPP20_TYPEOF(x);

//     SEXP result = R_NilValue;
//     bool found = false;

//     // Iterate through all known types
//     std::apply([&](auto... dummy) {
//         ((
//             !found && (type == r_type_id_v<std::decay_t<decltype(dummy)>>) ? 
//             (
//                 [&]() {
//                     using T = decltype(dummy);
                    
//                     // CHECK 1: Does C++ Type match R Type? (Done by ternary above)
                    
//                     // CHECK 2: Does the user function accept T? (SFINAE/Concepts)
//                     // The lambda body generates the call. If user's template constraints fail
//                     // for T, this block is invalid and discarded by 'if constexpr'.
//                     if constexpr (requires { functor.template operator()<T>(); }) {
//                         // Call the lambda, which calls the user function
//                         result = functor.template operator()<T>();
//                         found = true;
//                     }
//                 }(), 
//                 0 
//             ) : 0
//         ), ...);
//     }, r_types{});

//     if (!found) {
//         abort("No matching template instantiation found for input type");
//     }
//     return result;
// }



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
