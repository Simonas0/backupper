#define DEBUG

#ifdef DEBUG

#include <iostream>
#define DEBUG_MSG(str) std::clog << str << std::endl

#else

#define DEBUG_MSG(str)

#endif
