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
