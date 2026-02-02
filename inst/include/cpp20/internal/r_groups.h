#ifndef CPP20_R_GROUPS_H
#define CPP20_R_GROUPS_H

#include <cpp20/internal/r_vec.h>

// In some header like r_group.h
namespace cpp20 {

// 0-indexed group IDs: [0, n - 1]
struct groups {
  r_vec<r_int> ids; 
  int n_groups = 0;
  bool sorted = true;

  // Default constructor
  groups() = default;
  // Manual constructor
  explicit groups(r_vec<r_int> x, int ngroups, bool groups_sorted) : ids(std::move(x)), n_groups(ngroups), sorted(groups_sorted) {}
};



template <RScalar T>
inline groups make_groups(const r_vec<T>& x) {
  using key_type = unwrap_t<T>;

  r_size_t n = x.length();
  groups g;
  g.ids = r_vec<r_int>(n);
  g.n_groups = 0;
  g.sorted = true;

  if (n == 0) return g;

  ankerl::unordered_dense::map<key_type, int> lookup;
  lookup.reserve(n);

  auto* RESTRICT p_x = x.data();
  auto* RESTRICT p_id = g.ids.data();

  int next_id = 0;
  int last_id = 0;
  int id;

  for (r_size_t i = 0; i < n; ++i) {
    key_type key = p_x[i];
    auto [it, inserted] = lookup.try_emplace(key, next_id);
    if (inserted) {
      id = next_id++;
    } else {
      id = it->second;
    }
    p_id[i] = id;

    // check if group IDs are sorted
    // Since g.sorted was set to true by default, once it's set to false we don't need to set again
    if (g.sorted && id < last_id){
      g.sorted = false;
    }
    last_id = id;
  }

  g.n_groups = next_id;
  return g;
}

inline r_vec<r_int> group_starts(const groups& x){
    
    const r_vec<r_int>& group_ids = x.ids;

    int n = group_ids.length();
    int n_groups = x.n_groups;

    r_vec<r_int> out(n_groups);

    if (n_groups == 0){
        return out;
    }
    
    int curr_group;

    if (x.sorted){
        // Initialise just in-case there are groups with no group IDs (e.g. unused factor levels)
        // out.fill(0, n_groups, 0);
        const int* RESTRICT p_ids = group_ids.data();
        int* RESTRICT p_out = out.data();

        curr_group = 0;
        p_out[0] = 0;
        
        for (int i = 1; i < n; ++i){
            // New group
            if (p_ids[i] == (curr_group + 1)){
                p_out[++curr_group] = i;
            }
            //
            // if (p_ids[i] > p_ids[i - 1]){
            //     p_out[++curr_group] = i;
            // }
        }
    } else {

        // Initialise with largest int
        // so that for each group we take the min(out[i], i)
        // After passing through all data, this should reduce to the first location for each group
        out.fill(0, n_groups, r_limits<r_int>::max());
        
        const int* RESTRICT p_ids = group_ids.data();
        int* RESTRICT p_out = out.data();
        int curr_group_start;
        
        for (int i = 0; i < n; ++i){
            curr_group = p_ids[i];
            curr_group_start = p_out[curr_group];
            p_out[curr_group] = std::min(curr_group_start, i);
          }
    
        // This will set groups with no start locations to 0
        // (e.g. undropped factor levels)
        // If uncommenting the below line, make sure to remove RESTRICT keyword from pointers above
        // out.replace(0, n_groups, fill_value, 0);
    }

  return out;
}


}

#endif
