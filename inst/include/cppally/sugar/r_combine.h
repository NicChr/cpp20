#ifndef CPPALLY_R_COMBINE_H
#define CPPALLY_R_COMBINE_H

#include <cppally/r_coerce.h>
#include <cppally/r_list_helpers.h>
#include <cppally/sugar/r_make_vec.h>

namespace cppally {

template <RVector T>
T flatten(const r_vec<r_sexp>& x) {
    
    r_size_t n = x.length();
    r_size_t out_size = 0;
    for (r_size_t i = 0; i < n; ++i) {
        out_size += length(x.view(i));
    }

    T out(out_size);
    r_size_t k = 0; 
    r_size_t m;

    for (r_size_t i = 0; i < n; k += m, ++i) {
        T vec = as<T>(x.view(i));
        m = length(vec);
        r_copy_n(out, vec, k, m);
    }
    return out;
}
  
template <typename... Args>
auto combine(Args... args){
    using common_t = common_r_t<as_r_composite_t<Args>...>;
    r_vec<r_sexp> list_of_args = make_vec<r_sexp>(args...);
    return flatten<common_t>(list_of_args);
}

}

#endif
