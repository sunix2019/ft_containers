#ifndef TESTER_HPP
#define TESTER_HPP

#include <iostream>
#include "Number.hpp"

#ifdef FT
# define PRE ft
# include "../../containers/vector.hpp"
# include "../../containers/stack.hpp"
# include "../../containers/map.hpp"
# include "../../utils/pair.hpp"
#else
# define PRE std
# include <vector>
# include <stack>
# include <map>
# include <iterator>
# include <utility>
#endif

#include <array>
#include <list>

#include <stdlib.h>
#include <time.h> // clock()

#include <algorithm> // std::copy
#include <cmath>	 // hypot set
#include <cstddef>	 // size_t
#include <ctime>
#include <deque>
#include <iomanip>
#include <iostream> // cout
#include <list>		// std::list
#include <numeric>
#include <string>
#include <typeinfo> // typeid

#define cout std::cout
#define endl std::endl
#define cerr std::cerr

// vector_test.cpp
void vector_test();
void stack_test();
void map_test();

#endif
