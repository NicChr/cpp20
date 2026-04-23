#include <cppally.hpp>
using namespace cppally;

[[cppally::register]]
void test_valgrind(){
    r_vec<r_int> int_vec(1000);
    abort("Error"); // This should run C++ destructors and release vector memory
}
