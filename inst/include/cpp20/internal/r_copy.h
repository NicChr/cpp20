#ifndef CPP20_R_COPY_H
#define CPP20_R_COPY_H

#include <cpp20/internal/r_vec.h>
#include <cpp20/internal/r_attrs.h>

namespace cpp20 {

inline r_sexp shallow_copy(const r_sexp& x){
    return r_sexp(Rf_shallow_duplicate(x)); 
}

template <RVal T>
inline r_vec<T> deep_copy(const r_vec<T>& x){
    
    r_vec<T> out(r_null);
    r_size_t n = x.length();

    if (!x.is_null()){
        out = r_vec<T>(n);

        // If list, copy list elements
        if constexpr (is<T, r_sexp>){
        for (r_size_t i = 0; i < n; ++i){
            out.set(i, deep_copy(x.get(i)));
        }
        } else {
            r_copy_n(out, x, 0, n);
        }    
        auto attrs = attr::get_attrs(x);
        int n_attrs = attrs.length();
        for (int i = 0; i < n_attrs; ++i){
        attrs.set(i, deep_copy(attrs.get(i)));
        }
        attr::set_attrs(out, attrs);
    }
    return out;
}

inline r_sexp deep_copy(const r_sexp& x){
    return internal::visit_maybe_vector(x, [&](auto vec) -> r_sexp {
        if constexpr (!std::is_null_pointer_v<decltype(vec)>){
            return deep_copy(vec).sexp;
        } else {
            return r_sexp(Rf_duplicate(x));
        }
    });
}

}
#endif
