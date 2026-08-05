#pragma once

#include <cassert>

#ifndef NDEBUG

#define CONDUCTOR_ASSERT(x)

#else 

#define CONDUCTOR_ASSERT(x) assert((x))

#endif