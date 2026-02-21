#ifndef CPP20_DISPATCH_HPP
#define CPP20_DISPATCH_HPP

#include <cpp20/internal/r_coerce.h>
#include <tuple>
#include <utility>
#include <array>
#include <optional>

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
// The reverse of above
// Special case - never return CHARSXP
template <typename T>
SEXP cpp_to_sexp(T x) {
    if constexpr (RStringType<T>){
        return unwrap(as<r_vec<r_str>>(x));
    } else {
        return as<SEXP>(x);
    }
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
    r_sexp // Catch-all type
>;

// Helper to get the runtime R type ID for a C++ type

template<typename T> constexpr uint16_t r_cpp_boundary_map_v =              std::numeric_limits<uint16_t>::max();
template<> constexpr uint16_t r_cpp_boundary_map_v<r_vec<r_lgl>> =          LGLSXP;
template<> constexpr uint16_t r_cpp_boundary_map_v<r_vec<r_int>> =          INTSXP;
template<> constexpr uint16_t r_cpp_boundary_map_v<r_vec<r_int64>> =        CPP20_INT64SXP;
template<> constexpr uint16_t r_cpp_boundary_map_v<r_vec<r_dbl>> =          REALSXP;
// template<> constexpr uint16_t r_cpp_boundary_map_v<r_vec<r_str_view>> =     STRSXP;
template<> constexpr uint16_t r_cpp_boundary_map_v<r_vec<r_str>> =          STRSXP;
template<> constexpr uint16_t r_cpp_boundary_map_v<r_vec<r_cplx>> =         CPLXSXP;
template<> constexpr uint16_t r_cpp_boundary_map_v<r_vec<r_raw>> =          RAWSXP;
template<> constexpr uint16_t r_cpp_boundary_map_v<r_vec<r_sexp>> =         VECSXP;
// template<> constexpr uint16_t r_cpp_boundary_map_v<r_vec<r_sym>> =          VECSXP;
template<> constexpr uint16_t r_cpp_boundary_map_v<r_str> =                 STRSXP;
template<> constexpr uint16_t r_cpp_boundary_map_v<r_str_view> =            STRSXP;
template<> constexpr uint16_t r_cpp_boundary_map_v<r_sym> =                 SYMSXP;

template<typename T>
requires (RMathType<T> || RStringType<T> || RComplexType<T> || is<T, r_raw>)
inline constexpr uint16_t r_cpp_boundary_map_v<T> = r_cpp_boundary_map_v<r_vec<T>>;

// Pure C/C++ types that are constructible to an RVal
template <typename T>
requires (CastableToRVal<T> && CppType<T>)
inline constexpr uint16_t r_cpp_boundary_map_v<T> = r_cpp_boundary_map_v<as_r_val_t<T>>;

template <typename T>
inline void check_r_cpp_mapping(SEXP x){
    using data_t = std::remove_cvref_t<T>;
    // If input maps to catch-all, this is always fine (provided there wasn't a more precise mapping and that the constraints allow this)
    if constexpr (is_sexp<data_t>){
        return;
    }
    if (r_cpp_boundary_map_v<data_t> != CPP20_TYPEOF(x)){
        abort("Expected input type: %s", type_str<data_t>());
    }
}

// Helper to get Nth element from parameter pack
template <size_t N, typename... Args>
decltype(auto) get_nth(Args&&... args) {
    return std::get<N>(std::forward_as_tuple(std::forward<Args>(args)...));
}

// Helper to get first argument index for a template parameter
template <size_t TemplateParamIdx, size_t NumArgs, auto ArgToTemplateMap>
constexpr size_t first_arg_for_template() {
    for (size_t i = 0; i < NumArgs; ++i) {
        if (ArgToTemplateMap[i] == TemplateParamIdx) {
            return i;
        }
    }
    return NumArgs; // Not found
}

// Verify all args for a template param have the same runtime type
template <size_t TemplateParamIdx, size_t NumArgs, auto ArgToTemplateMap, typename... SexpArgs>
bool verify_template_param_consistency(uint16_t expected_type, SexpArgs&&... sexp_args) {
    bool consistent = true;
    
    auto check_arg = [&]<size_t I>() {
        if constexpr (I < NumArgs) {
            if (ArgToTemplateMap[I] == static_cast<int>(TemplateParamIdx)) {
                SEXP arg = get_nth<I>(std::forward<SexpArgs>(sexp_args)...);
                if (CPP20_TYPEOF(arg) != expected_type) {
                    consistent = false;
                }
            }
        }
    };
    
    [&]<size_t... Is>(std::index_sequence<Is...>) {
        (check_arg.template operator()<Is>(), ...);
    }(std::make_index_sequence<NumArgs>{});
    
    return consistent;
}

// Grouped dispatcher - selects one type per template parameter
template <size_t Remaining, size_t NumArgs, auto ArgToTemplateMap, typename... SelectedTypes>
struct GroupedDispatcher {
    template <typename Functor, typename... SexpArgs>
    static std::optional<SEXP> dispatch(Functor&& functor, SexpArgs&&... sexp_args) {
        // Base case: All template params resolved
        if constexpr (requires { functor.template operator()<SelectedTypes...>(sexp_args...); }) {
            return functor.template operator()<SelectedTypes...>(sexp_args...);
        } else {
            return std::nullopt;
        }
    }
};

// Recursive case: Select type for next template parameter
template <size_t Remaining, size_t NumArgs, auto ArgToTemplateMap, typename... SelectedTypes>
requires (Remaining > 0)
struct GroupedDispatcher<Remaining, NumArgs, ArgToTemplateMap, SelectedTypes...> {
    template <typename Functor, typename... SexpArgs>
    static std::optional<SEXP> dispatch(Functor&& functor, SexpArgs&&... sexp_args) {
        constexpr size_t CurrentTemplateIdx = sizeof...(SelectedTypes);
        
        // Find first argument using this template parameter
        constexpr size_t FirstArgIdx = first_arg_for_template<CurrentTemplateIdx, NumArgs, ArgToTemplateMap>();
        static_assert(FirstArgIdx < NumArgs, "Template parameter not used by any argument");
        
        SEXP representative = get_nth<FirstArgIdx>(sexp_args...);
        uint16_t type = CPP20_TYPEOF(representative);
        
        std::optional<SEXP> result = std::nullopt;
        
        // Try each type in r_types
        auto try_candidate = [&]<typename Cand>() {
            if (result.has_value()) return;
        
            if constexpr (!is_sexp<Cand>) {
                if (type != r_cpp_boundary_map_v<Cand>) return;

                if (!verify_template_param_consistency<CurrentTemplateIdx, NumArgs, ArgToTemplateMap>(
                    type, std::forward<SexpArgs>(sexp_args)...)) {
                    return;
                }
            }
        
            result = GroupedDispatcher<Remaining - 1, NumArgs, ArgToTemplateMap, SelectedTypes..., Cand>::dispatch(
                std::forward<Functor>(functor), 
                std::forward<SexpArgs>(sexp_args)...
            );
        };

        auto try_type = [&]<typename T>() {
        
            // Only try to deduce (vec) type if there is an explicit mapping (not catch-all)
            if constexpr (r_cpp_boundary_map_v<r_vec<T>> != std::numeric_limits<uint16_t>::max()) {
                // Try vector first (it will be rejected early if type != VECSXP/STRSXP/etc)
                try_candidate.template operator()<r_vec<T>>();
            }
            
            // If vector form didn't work (or was rejected by type check), try scalar
            if (!result.has_value()) {
                try_candidate.template operator()<T>();
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

// Entry point with grouping information
// NumTemplateParams: Number of unique template parameters (e.g., 2 for T and U)
// ArgToTemplateMap: Maps each argument position to template param index
//                   -1 means not a template argument
//                   Example: {0, 0, 1, 1} means args 0,1 use T (param 0), args 2,3 use U (param 1)
template <size_t NumTemplateParams, size_t NumArgs, std::array<int, NumArgs> ArgToTemplateMap, 
          typename Functor, typename... SexpArgs>
SEXP dispatch_template_impl(Functor&& functor, SexpArgs&&... sexp_args) {
    static_assert(sizeof...(SexpArgs) == NumArgs, "Argument count mismatch");
    
    std::optional<SEXP> result = GroupedDispatcher<NumTemplateParams, NumArgs, ArgToTemplateMap>::dispatch(
        std::forward<Functor>(functor), 
        std::forward<SexpArgs>(sexp_args)...
    );
    
    if (!result.has_value()) {
        abort("No matching template instantiation found for input types");
    }
    
    return result.value();
}

// Invoke with index sequence
template <auto Fn, typename Ret, typename... Args, size_t... Is>
SEXP invoke_impl(SEXP* sexp_args, std::index_sequence<Is...>) {
    if constexpr (std::is_void_v<Ret>) {
        Fn(sexp_to_cpp<Args>(sexp_args[Is])...);
        return R_NilValue;
    } else {
        return cpp_to_sexp(Fn(sexp_to_cpp<Args>(sexp_args[Is])...));
    }
}

} // namespace internal

// Main dispatch - unpack tuple types directly
template <auto Fn, typename... SexpArgs>
SEXP dispatch(SexpArgs... args) {
    using Traits = internal::fn_traits<decltype(Fn)>;
    using ArgsTuple = typename Traits::args_tuple;
    
    static_assert(sizeof...(SexpArgs) == Traits::arity, 
                  "Argument count mismatch");
                  
    static_assert((std::is_same_v<SexpArgs, SEXP> && ...),  "dispatch<Fn>: all arguments must be SEXP");
    
    SEXP arg_array[] = {args...};
    
    return []<typename... Args>(SEXP* arr, std::tuple<Args...>*) {
        return internal::invoke_impl<Fn, typename Traits::return_type, Args...>(
            arr, std::make_index_sequence<sizeof...(Args)>{}
        );
    }(arg_array, static_cast<ArgsTuple*>(nullptr));
}

} // namespace cpp20

#endif
