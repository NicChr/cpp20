#ifndef CPPALLY_R_REPLACE_H
#define CPPALLY_R_REPLACE_H

#include <cppally/r_vec.h>
#include <cppally/r_coerce.h>
#include <cppally/sugar/r_subset.h>
#include <cppally/sugar/r_find.h>

namespace cppally {

template <RVal T>
void r_vec<T>::replace(const r_vec<T>& old_values, const r_vec<T>& new_values){

  if (is_null()) return;

  r_size_t n = length();
  r_size_t oldv_size = old_values.length();
  r_size_t newv_size = new_values.length();
  r_size_t oldvi = 0, newvi = 0;

  if (oldv_size == 0 || newv_size == 0) return;

  if (oldv_size == 1 && newv_size == 1){
    replace(old_values.view(0), new_values.view(0));
    return;
  }

  r_vec<T> prev = as<r_vec<T>>(old_values);
  r_vec<T> repl = as<r_vec<T>>(new_values);

  for (r_size_t i = 0; i < n; 
    recycle_index(oldvi, oldv_size), 
    recycle_index(newvi, newv_size), 
    ++i){
      if (identical(view(i), prev.view(oldvi))){
        set(i, repl.view(newvi));
      }
  }
}

}

#endif
