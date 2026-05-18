#ifndef CPPALLY_R_COMBINE_H
#define CPPALLY_R_COMBINE_H

#include <cppally/r_coerce.h>
#include <cppally/r_list_helpers.h>
#include <cppally/sugar/r_subset.h>
#include <cppally/sugar/r_rep.h>
#include <cppally/sugar/r_make_vec.h>

namespace cppally {

namespace internal {
inline r_vec<r_str_view> combine_levels(const r_vec<r_str_view>& x_lvls, const r_vec<r_str_view>& y_lvls){
    r_vec<r_int> new_lvl_locations = y_lvls.find(x_lvls, /*invert = */ true);

    if (new_lvl_locations.length() == 0){
        return x_lvls;
    }

    r_vec<r_str_view> new_lvls = y_lvls.subset(new_lvl_locations);

    r_size_t n_lvls = x_lvls.length() + new_lvls.length();
    r_vec<r_str_view> out(n_lvls);
    r_copy_n(out, x_lvls, 0, x_lvls.length());
    r_copy_n(out, new_lvls, x_lvls.length(), new_lvls.length());
    return out;
}
}

inline void r_copy_n(r_factors& target, const r_factors& source, r_size_t target_offset, r_size_t n){
    if (identical(target.levels(), source.levels())){
        r_copy_n(target.value, source.value, target_offset, n);
        return;   
    }
    r_vec<r_str_view> all_levels = internal::combine_levels(target.levels(), source.levels());
    target.set_levels(all_levels);
    r_vec<r_int> new_codes = source.new_codes(all_levels);
    r_copy_n(target.value, new_codes, target_offset, n);

}

inline void r_copy_n(r_df& target, const r_df& source, r_size_t target_offset, r_size_t n){
    if (n == 0) return;
    r_size_t ncols = static_cast<r_size_t>(target.ncol());
    r_vec<r_str_view> colnames = target.colnames();
    for (r_size_t j = 0; j < ncols; ++j){
        r_str_view colname = colnames.view(j);
        target.with_col(j, [&]<typename col_t>(col_t&& tcol) -> void {
            r_copy_n(tcol, col_t(source.get_col(colname)), target_offset, n);
        }, true);
    }
}

template <typename T>
T flatten(const r_vec<r_sexp>& x) = delete;

// Flatten a list of vectors, factors or data frames
// to a single vector, factor or data frame
template <RComposite T>
T flatten(const r_vec<r_sexp>& x) {

    r_size_t n = x.length();
    
    if (n == 0){
        return T();
    }

    r_size_t out_size = 0;
    for (r_size_t i = 0; i < n; ++i) {
        out_size += length(x.view(i));
    }

    // Grab first vector from list and resize it to final size
    T first_vec = as<T>(x.view(0));
    // resize is best here as it resizes, doesn't initialise extra memory
    // and preserves attributes
    T out = resize(first_vec, out_size);

    r_size_t k = length(first_vec), m;
    for (r_size_t i = 1; i < n; k += m, ++i) {
        T vec = as<T>(x.view(i));
        m = length(vec);
        r_copy_n(out, vec, k, m);
    }
    return out;
}

// template <RVector T>
// T flatten(const r_vec<r_sexp>& x) {
    
//     r_size_t n = x.length();
//     r_size_t out_size = 0;
//     for (r_size_t i = 0; i < n; ++i) {
//         out_size += length(x.view(i));
//     }

//     T out(out_size);
//     r_size_t k = 0; 
//     r_size_t m;

//     for (r_size_t i = 0; i < n; k += m, ++i) {
//         T vec = as<T>(x.view(i));
//         m = length(vec);
//         r_copy_n(out, vec, k, m);
//     }
//     return out;
// }

// template <RFactor T>
// T flatten(const r_vec<r_sexp>& x) {
    
//     r_size_t n = x.length();
//     r_size_t out_size = 0;
//     r_vec<r_str_view> all_levels{};
//     std::vector<r_factors> fctrs;
//     fctrs.reserve(n);
//     for (r_size_t i = 0; i < n; ++i) {
//         r_factors fctr = as<r_factors>(x.get(i));
//         fctrs.push_back(fctr);
//         out_size += fctr.length();
//         all_levels = internal::combine_levels(all_levels, fctr.levels());
//     }

//     r_factors out(r_vec<r_int>(out_size), all_levels);
//     r_size_t k = 0; 
//     r_size_t m;

//     for (r_size_t i = 0; i < n; k += m, ++i) {
//         r_factors fctr = fctrs[i];
//         m = fctr.length();
//         r_copy_n(out, fctr, k, m);
//     }
//     return out;
// }

// template <RDataFrame T>
// T flatten(const r_vec<r_sexp>& x) {

//     r_size_t n = x.length();
//     if (n == 0){
//         return r_df();
//     }

//     r_size_t out_size = 0;
//     for (r_size_t i = 0; i < n; ++i) {
//         out_size += length(x.view(i));
//     }

//     // Grab first data frame from list and resize it to final nrows
//     r_df first_df = as<r_df>(x.view(0));
//     // resize is best here as it doesn't initialise memory
//     r_df out = resize(first_df, out_size);

//     // Column-wise copy
//     r_size_t k = 0, m;
//     for (r_size_t i = 0; i < n; k += m, ++i) {
//         r_df df = as<r_df>(x.view(i));
//         m = df.nrow();
//         r_copy_n(out, df, k, m);
//     }
//     return out;
// }

// template <RDataFrame T>
// T flatten(const r_vec<r_sexp>& x) {
    
//     r_size_t n = x.length();
//     r_size_t out_size = 0;
//     for (r_size_t i = 0; i < n; ++i) {
//         out_size += length(x.view(i));
//     }

//     if (n == 0){
//         return r_df();
//     }

//     r_df first_df = as<r_df>(x.view(0));
//     int ncols = first_df.ncol();
//     r_vec<r_str_view> colnames = first_df.colnames();

//     r_vec<r_sexp> out(ncols);
//     for (r_size_t j = 0; j < ncols; ++j) {
//         r_str_view colname = colnames.view(j);
//         view_sexp(first_df.view_col(j), [&]<typename col_t>(col_t&&) -> void {
//             if constexpr (RVector<col_t>){
//                 col_t new_col(out_size);
//                 r_size_t k = 0, m;
//                 for (r_size_t i = 0; i < n; k += m, ++i) {
//                     r_df df = as<r_df>(x.view(i));
//                     m = df.nrow();
//                     r_copy_n(new_col, col_t(df.get_col(colname)), k, m);
//                 }
//                 out.set(j, new_col);
//             } else if constexpr (RFactor<col_t>){
//                 col_t new_col(out_size);
//                 r_size_t k = 0, m;
//                 for (r_size_t i = 0; i < n; k += m, ++i) {
//                     r_df df = as<r_df>(x.view(i));
//                     m = df.nrow();
//                     r_copy_n(new_col, col_t(df.get_col(colname)), k, m);
//                 }
//                 out.set(j, new_col);
//             } else if constexpr (RDataFrame<col_t>){
//                 abort("error");
//             } else {
//                 abort("error");
//             }
//         });
//     }
//     out.set_names(colnames);
//     return r_df(out, /*recycle=*/false, /*nrows=*/static_cast<int>(out_size));
// }
  
template <typename... Args>
auto combine(Args... args){
    using common_t = common_r_t<as_r_composite_t<Args>...>;
    r_vec<r_sexp> list_of_args = make_vec<r_sexp>(args...);
    return flatten<common_t>(list_of_args);
}

}

#endif
