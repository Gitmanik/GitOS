//
// Created by Pawel Reich on 1/15/25.
//

#pragma once
#include "kernel.h"

#define assert(EX) if (!(EX)) kernel_panic ("Assertion failed: %s at %s:%d", #EX, __FILE__, __LINE__)
#define assert_not_reached() assert(0)