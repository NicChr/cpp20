
#ifndef CPP20_R_DF_H
#define CPP20_R_DF_H

#include <cpp20/r_vec.h>
#include <cpp20/r_attrs.h>
#include <cpp20/r_list_helpers.h>

namespace cpp20 {

namespace internal {

inline r_vec<r_int> create_row_names(int n){

    if (n == 0){
        return r_vec<r_int>();
    } else {
        r_vec<r_int> out(2); 
        out.set(0, na<r_int>());
        out.set(1, r_int(-n));
        return out;
    }
}

inline r_vec<r_sexp> new_df_impl(const r_vec<r_sexp>& cols, int nrows = -1){

    r_size_t n = cols.length();
    r_vec<r_sexp> out(n);

    // Recycle to common size or nrows if nrows > 0
    if (nrows < 0){
        nrows = internal::recycle_size(cols);
    }
    
    for (r_size_t i = 0; i < n; ++i){
        out.set(i, 
        view_sexp(cols.view(i), [nrows](const auto& vec) -> r_sexp {
            if constexpr (!RVector<decltype(vec)> && !RMetaVector<decltype(vec)>){
                abort("Don't know how to visit this r_sexp!");
            } else {
                return r_sexp(vec.rep_len(nrows), internal::view_tag{});
            }
    })
);
    }

    r_vec<r_str_view> names = cols.names();
    if (names.is_null()){
        names = r_vec<r_str_view>(n, blank_r_string);
    }

    r_vec<r_int> row_names = create_row_names(nrows);
    out.set_names(names); 
    attr::set_attr(out, symbol::class_sym, r_vec<r_str_view>(1, "data.frame"));
    attr::set_attr(out, symbol::row_names_sym, row_names);
    return out;
}

inline r_vec<r_sexp> new_df_impl(int nrows){

    r_vec<r_sexp> out{};
    r_vec<r_int> row_names = create_row_names(nrows);
    out.set_names(r_vec<r_str_view>());
    attr::set_attr(out, symbol::class_sym, r_vec<r_str_view>(1, "data.frame"));
    attr::set_attr(out, symbol::row_names_sym, row_names);
    return out;
}
}

struct r_df {

    r_vec<r_sexp> value;
    
    // Default constructor (empty data frame)
    r_df() : value(std::move(internal::new_df_impl(0))) {}

    private: 

    void validate_col_sizes(const r_vec<r_sexp>& x){
        r_size_t n = x.length();
        if (n > 0){
            r_size_t init_size = x.view(0).length();
            if (!internal::can_be_int(x)) [[unlikely]] {
                abort("Data frames can only contain short vectors, please check");
            }
            for (r_size_t i = 0; i < n; ++i){
                if (init_size != x.view(i).length()){
                    abort("All lengths of a data frame must be equal");
                }
            }
        }
    }

    void validate_df(SEXP x){

        if (TYPEOF(x) == NILSXP) return;

        if (TYPEOF(x) != VECSXP){
          abort("SEXP must have vector storage to be constructed as a data frame");
        }
        if (!Rf_inherits(x, "data.frame")){
          abort("SEXP must be of class 'data.frame' to be constructed as a data frame");
        }
        r_vec<r_str_view> names = attr::get_old_names(x);
        if (names.length() != Rf_xlength(x)){
          abort("data frame length of names must match ncol");
        }
        SEXP row_names = Rf_getAttrib(x, symbol::row_names_sym);
        if (TYPEOF(row_names) != STRSXP){
            abort("rownames must be a character vector");
        }
        validate_col_sizes(r_vec<r_sexp>(x, internal::view_tag{}));
      }

    public: 

    // Constructor from existing SEXP
    explicit r_df(SEXP s) : value(s) {validate_df(value);}
    explicit r_df(SEXP s, internal::view_tag) : value(s, internal::view_tag{}) {validate_df(value);}

    explicit r_df(const r_sexp& s) : value(s) {validate_df(value);}
    explicit r_df(const r_sexp& s, internal::view_tag) : value(s, internal::view_tag{}) {validate_df(value);}
    
    // Constructor from list of cols
    // When nrows is negative, each column is recycled to a common max length
    // Supply a nrows value for a custom recycle length
    explicit r_df(const r_vec<r_sexp>& cols, int nrows = -1) : value(std::move(internal::new_df_impl(cols, nrows))){}

    // Implicit conversion to SEXP
    operator SEXP() const noexcept { return unwrap(value); }

    private: 

    // For methods that just return a non-factor (like length())
    #define FORWARD_METHOD(NAME)                               \
        template <typename... Args>                            \
        decltype(auto) NAME(Args&&... args) const {            \
            return value.NAME(std::forward<Args>(args)...);    \
        }

    // For methods that return a df
    #define FORWARD_DF_METHOD(NAME)                                         \
        template <typename... Args>                                         \
        r_df NAME(Args&&... args) const {                              \
            /* Call the method on the underlying r_vec<r_sexp> */           \
            auto new_df = value.NAME(std::forward<Args>(args)...);          \
            /* Direct construction */                                       \
            return r_df(new_df);                                            \
        }

    public: 

    // Inherit standard methods from r_vec<>

    FORWARD_METHOD(is_null)
    FORWARD_METHOD(data)
    FORWARD_METHOD(address)
  
    // Undefine the macros so they don't leak out of the struct
    #undef FORWARD_METHOD
    #undef FORWARD_DF_METHOD


    int nrow() const noexcept {
        return rownames().length();
    }
    int ncol() const noexcept {
        return value.length();
    }

    r_vec<r_str_view> rownames() const {
        return r_vec<r_str_view>(attr::get_attr(value, symbol::row_names_sym));
    }
    r_vec<r_str_view> colnames() const {
        return value.names();
    }

    template <RStringType U>
    void set_rownames(const r_vec<U>& rownames) {
        if (rownames.length() != nrow()){
            abort("`length(rownames)` must match `nrow()`");
        }
        return attr::set_attr(value, symbol::row_names_sym, rownames);
    }
    template <RStringType U>
    void set_colnames(const r_vec<U>& colnames) {
        value.set_names(colnames);
    }

    // IMPORTANT - indices are 1-indexed
    // This has the benefit of allowing empty locations (0) and negative indexing
    template <internal::RSubscript U>
    r_df subset(const r_vec<U>& indices) const {
        r_vec<r_sexp> out;
        if (ncol() == 0){
            // We don't have a function atm that tells us what the resulting size should be here
            // So subset a dummy vector
            r_vec<r_int> dummy(nrow()); // Uninitialised dummy vector
            r_vec<r_int> dummy2 = dummy.subset(indices);
            out = internal::new_df_impl(dummy2.length());
        } else {
            out = internal::new_df_impl(value);
        }
        internal::view_elements(out, [&]<typename T>(r_size_t i, const T& elem) {
            out.set(i, elem.subset(indices));
        });
        return r_df(out);
    }
};

}

#endif
