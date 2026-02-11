#ifndef CPP20_R_SORT_H
#define CPP20_R_SORT_H

#include <cpp20/internal/r_groups.h>
#include <cpp20/internal/r_unique.h>
#include <numeric>
#include <algorithm>
#include <ska_sort/ska_sort.hpp>

namespace cpp20 {


// general order vector that sorts `x`
// NAs are ordered last
// Internal function to be used for low overhead sorting small vectors (n<500)
template <RSortable T>
r_vec<r_int> cpp_order(const r_vec<T>& x) {
    int n = x.size();
    r_vec<r_int> p(n);
    std::iota(p.begin(), p.end(), 0);

    if constexpr (RMathType<T>){
        std::sort(p.begin(), p.end(), [&](int i, int j) {
            if (is_na(x.view(i))) return false;
            if (is_na(x.view(j))) return true;
            return unwrap(x.view(i)) < unwrap(x.view(j));
        });
    } else {
        // Below works on strings
        std::sort(p.begin(), p.end(), [&](int i, int j) {
            auto res = x.view(i) < x.view(j);
            if (is_na(res)){
                if (is_na(x.view(i))){
                    return false;
                } else {
                    return true;
                }
            }
            return static_cast<bool>(unwrap(res));
        });
    }
    return p;
}

template <RSortable T>
r_vec<r_int> cpp_stable_order(const r_vec<T>& x) {
    int n = x.size();
    r_vec<r_int> p(n);
    std::iota(p.begin(), p.end(), 0);

    if constexpr (RMathType<T>){
        std::stable_sort(p.begin(), p.end(), [&](int i, int j) {
            if (is_na(x.view(i))) return false;
            if (is_na(x.view(j))) return true;
            return unwrap(x.view(i)) < unwrap(x.view(j));
        });
    } else {
        // Below works on strings
        std::stable_sort(p.begin(), p.end(), [&](int i, int j) {
            auto res = x.view(i) < x.view(j);
            if (is_na(res)){
                if (is_na(x.view(i))){
                    return false;
                } else {
                    return true;
                }
            }
            return static_cast<bool>(unwrap(res));
        });
    }
    return p;
}

// Stable order that preserves order of ties
template <RSortable T>
r_vec<r_int> stable_order(const r_vec<T>& x) {
    int n = x.size();

    if (n < 500){
        return cpp_stable_order(x);
    }

    // ----------------------------------------------------------------------
    // Integers (Stable Radix Sort on uint64_t)
    // ----------------------------------------------------------------------

    if constexpr (RIntegerType<T>){

        r_vec<r_int> p(n);
        auto* RESTRICT p_x = x.data();

        std::vector<uint64_t> pairs(n);

        for (int i = 0; i < n; ++i) {
            int val = p_x[i];
            uint32_t key;
            if (is_na(p_x[i])) {
                key = 0xFFFFFFFF; // Force NA last
            } else {
                key = detail::to_unsigned_or_bool(val);
            }
            // Pack: Key High, Index Low
            pairs[i] = (static_cast<uint64_t>(key) << 32) | static_cast<uint32_t>(i);
        }

        // Fast & Stable (because index is part of the unique value)
        ska_sort(pairs.begin(), pairs.end());

        int* RESTRICT p_out = p.data();
        for (int i = 0; i < n; ++i) {
            p_out[i] = static_cast<int>(pairs[i] & 0xFFFFFFFF);
        }
        return p;
    }

    // ----------------------------------------------------------------------
    // Doubles (Stable Radix Sort on Pair<u64, int>)
    // ----------------------------------------------------------------------
    // Note: Since double is 64-bit, we can't pack (Value+Index) into one 64-bit int.
    // We must use a struct { uint64_t key; int index; } and sort that.

    else if constexpr (RFloatType<T>) {

        r_vec<r_int> p(n);
        auto* RESTRICT p_x = x.data();

        struct DblPair { uint64_t key; int index; };
        std::vector<DblPair> pairs(n);

        for (int i = 0; i < n; ++i) {
            double val = p_x[i];
            uint64_t key;
            if (is_na(p_x[i])) { // Use your is_na check
                key = 0xFFFFFFFFFFFFFFFFULL; // Force NA last
            } else {
                key = detail::to_unsigned_or_bool(val);
            }
            pairs[i] = {key, i};
        }

        // Sort struct using extractor
        ska_sort(pairs.begin(), pairs.end(), [](const DblPair& k){ return k.key; });

        int* RESTRICT p_out = p.data();
        for (int i = 0; i < n; ++i) {
            p_out[i] = pairs[i].index; // 0-based
        }
        return p;
    } else {
        return cpp_stable_order(x);
    }
}

// order function (doesn't preserve order of ties)
template <RSortable T>
r_vec<r_int> order(const r_vec<T>& x) {
    int n = x.size();

    if (n < 500){
        return cpp_order(x);
    }

    // ----------------------------------------------------------------------
    // Integers (ska_sort)
    // ----------------------------------------------------------------------

    if constexpr (RIntegerType<T>) {

        r_vec<r_int> p(n);
        auto* RESTRICT px = x.data();

        struct key_index {
            uint32_t key;
            int index;
        };
        std::vector<key_index> pairs(n);

        // 1. Pack (Key, Index)
        for (int i = 0; i < n; ++i) {
            int val = px[i];
            uint32_t key;
            if (is_na(val)) {
                key = 0xFFFFFFFF; // NA Last
            } else {
                key = detail::to_unsigned_or_bool(val);
            }
            pairs[i] = {key, i};
        }

        ska_sort(pairs.begin(), pairs.end(), [](const key_index& k){ return k.key; });

        int* RESTRICT p_out = p.data();
        OMP_SIMD
        for (int i = 0; i < n; ++i) {
            p_out[i] = pairs[i].index;
        }
        return p;
    }

    // ----------------------------------------------------------------------
    // Doubles (Stable Radix Sort on Pair<u64, int>)
    // ----------------------------------------------------------------------
    // Note: Since double is 64-bit, we can't pack (Value+Index) into one 64-bit int.
    // We must use a struct { uint64_t key; int index; } and sort that.

    else if constexpr (RFloatType<T>) {

        r_vec<r_int> p(n);
        auto* RESTRICT px = x.data();

        struct key_index { uint64_t key; int index; };
        std::vector<key_index> pairs(n);

        for (int i = 0; i < n; ++i) {
            double val = px[i];
            uint64_t key;
            if (is_na(val)) {
                key = 0xFFFFFFFFFFFFFFFFULL; // Force NA last
            } else {
                key = detail::to_unsigned_or_bool(val);

            }
            pairs[i] = {key, i};
        }

        ska_sort(pairs.begin(), pairs.end(), [](const key_index& k){ return k.key; });

        int* RESTRICT p_out = p.data();
        OMP_SIMD
        for (int i = 0; i < n; ++i) {
            p_out[i] = pairs[i].index;
        }
        return p;
    } else if constexpr (RString<T>) {
        r_vec<r_int> p(n);
    
        struct key_index {
            std::string str;
            int index;
            bool is_na;
        };
        std::vector<key_index> pairs;
        pairs.reserve(n);
    
        for (int i = 0; i < n; ++i) {
            if (is_na(x.view(i))) {
                // Use empty string for NA, mark with flag
                pairs.push_back({"", i, true});
            } else {
                pairs.push_back({x.view(i).cpp_str(), i, false});
            }
        }
    
        // Partition: non-NA first, NAs at end
        auto na_start = std::partition(pairs.begin(), pairs.end(),
            [](const key_index& p) { return !p.is_na; });
    
        // Sort non-NA strings using ska_sort
        if (na_start != pairs.begin()) {
            ska_sort(pairs.begin(), na_start, 
                [](const key_index& s) -> const std::string& { 
                    return s.str; 
                });
        }
    
        // Unpack indices
        int* RESTRICT p_out = p.data();
        for (int i = 0; i < n; ++i) {
            p_out[i] = pairs[i].index;
        }
        return p;  
    } else {
        return cpp_order(x);
    }
}



template <RSortable T>
inline bool is_sorted(const r_vec<T>& x) {
    
    r_size_t n = x.length();

    for (r_size_t i = 1; i < n; ++i) {
        if ( !(x.view(i) < x.view(i - 1)).is_false() ){
            return false;
        }
    }
    return true;
}

template <RSortable T>
inline groups make_groups_from_order(const r_vec<T>& x, const r_vec<r_int>& o) {
    r_size_t n = x.length();
    groups g;
    g.ids = r_vec<r_int>(n);
    
    if (n == 0) return g;

    auto* RESTRICT p_id = g.ids.data();
    auto* RESTRICT p_o = o.data();
    
    int current_group = 0;

    p_id[p_o[0]] = 0;

    for (r_size_t i = 1; i < n; ++i) {
        int idx_curr = p_o[i];
        int idx_prev = p_o[i - 1];

        bool is_equal;
        is_equal = identical(x.view(idx_curr), x.view(idx_prev));

        if (!is_equal) {
            current_group++;
        }
        
        p_id[idx_curr] = current_group;
    }

    g.n_groups = current_group + 1;
    g.sorted = is_sorted(g.ids);

    return g;
}

template <RVal T>
r_vec<T> sorted_unique(const r_vec<T>& x) {
    if constexpr (RSortable<T>){
        groups group_info = make_groups_from_order(x, order(x));
        auto starts = group_starts(group_info);
        starts += 1;
        return x.subset(starts);
    } else {
        return unique(x); 
    }
}

}

#endif
