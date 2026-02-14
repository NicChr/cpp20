#ifndef CPP20_REGISTER_HPP
#define CPP20_REGISTER_HPP

// The R parser will search for the string "[[cpp20::register]]"
#ifdef __R_GENERATE_
  #define CPP20_REGISTER [[cpp20::register]]
#else
  #define CPP20_REGISTER 
#endif

// #define BEGIN_CPP20 try {
//   #define END_CPP20 } catch (std::exception& e) { Rf_errorcall(R_NilValue, "%s", e.what()); } catch (...) { Rf_errorcall(R_NilValue, "C++ exception"); }
  

#endif
