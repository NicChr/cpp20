#ifndef CPPALLY_R_SEXP_SUGAR_H
#define CPPALLY_R_SEXP_SUGAR_H

// Methods for r_sexp such as vector operations and data frame construction
// the SEXP is visited in a lambda context and method applied to visited type

#include <cppally/r_vec.h>
#include <cppally/r_attrs.h>
#include <cppally/r_visit.h>
#include <cppally/r_vec_fns.h>
#include <cppally/r_list_helpers.h>
#include <string>

namespace cppally {

namespace internal {

inline r_vec<r_sexp> new_df_impl(const r_vec<r_sexp>& cols, bool recycle, int nrows){

    r_size_t n = cols.length();
    r_vec<r_sexp> out(n);

    if (nrows < 0){
        abort("Supply a valid `nrows`");
    }

    if (recycle){
        for (r_size_t i = 0; i < n; ++i){
            out.set(i, rep_len(cols.view(i), nrows));
        }
    } else {
        for (r_size_t i = 0; i < n; ++i){
            out.set(i, cols.view(i));
        }
    }

    // Always provide names
    r_vec<r_str_view> names = cols.names();
    if (names.is_null()){
        names = r_vec<r_str_view>(n);
        for (r_size_t i = 0; i < n; ++i){
            r_str elem( (std::string("col_") + std::to_string(i + 1)).c_str() );
            names.set(i, elem);
        } 
    }
    out.set_names(names); 

    r_vec<r_int> row_names = create_row_names(nrows);
    attr::set_attr(out, symbol::class_sym, r_vec<r_str_view>(1, r_str_view(cached_str<"data.frame">())));
    attr::set_attr(out, symbol::row_names_sym, row_names);
    return out;
}

inline r_vec<r_sexp> new_df_impl(const r_vec<r_sexp>& cols, bool recycle = true){
    r_size_t nrows;
    if (recycle){
        nrows = internal::recycle_size(cols);
    } else {
        if (cols.length() == 0){
            nrows = 0;
        } else {
            nrows = cols.view(0).length();
        }
    }
    return new_df_impl(cols, recycle, nrows);
}

}

// Constructor from list of cols
// Supply a nrows value for a custom recycle length
inline r_df::r_df(const r_vec<r_sexp>& cols, bool recycle) : value(internal::new_df_impl(cols, recycle)){}
inline r_df::r_df(const r_vec<r_sexp>& cols, bool recycle, int nrows) : value(internal::new_df_impl(cols, recycle, nrows)){}
// Atomic vector constructor
template <RScalar T>
inline r_df::r_df(const r_vec<T>& col) : value(internal::new_df_impl(r_vec<r_sexp>(1, r_sexp(static_cast<SEXP>(col))))){}
// Factor constructor
inline r_df::r_df(const r_factors& col) : value(internal::new_df_impl(r_vec<r_sexp>(1, r_sexp(static_cast<SEXP>(col))))){}

template <internal::RSubscript U>
inline r_df r_df::subset(const r_vec<U>& indices) const {
    if (ncol() == 0){
        // We don't have a function atm that tells us what the resulting size should be here
        // So subset a dummy vector
        r_vec<r_int> dummy(nrow()); // Uninitialised dummy vector
        return r_df(r_vec<r_sexp>(), false, dummy.subset(indices).length());
    }
    r_vec<r_sexp> out(ncol());
    internal::view_elements(value, [&]<typename T>(r_size_t i, const T& elem) {
        out.set(i, elem.subset(indices));
    });
    return r_df(out, false, out.view(0).length());
}

}

#endif
