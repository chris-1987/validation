////////////////////////////////////////////////////////////
/// Copyright (c) 2016, Sun Yat-sen University,
/// All rights reserved
/// \file common.h
/// \brief definition of macro, constant and alias for common use
///
///
/// \author Yi Wu
/// \date 2016.12
///////////////////////////////////////////////////////////


#ifndef __COMMON_H
#define __COMMON_H

#include "namespace.h"

#include "types.h"

#include "stxxl/bits/io/syscall_file.h"

#include "stxxl/vector"

#include <string>

#include <climits>

#include <cassert>

static const size_t K_512 = 512 * 1024; ///< 512 K

static const size_t M_512 = K_512 * 1024; ///< 512 M

using fpa_type = uint32;

using fpb_type = uint64;

static const fpa_type P = 2047824239;

static const fpa_type R = 1732327371;

static const uint_type MAIN_MEM_AVAIL = 3 * 1024 * 1024 * 1024ull; 

static const uint_type BUFF_MEM_AVAIL = 8 * 1024 * 1024ull; // 8 mb for buffer

static const uint8 L_TYPE = 0;

static const uint8 S_TYPE = 1;
//static const uint_type MAIN_MEM_AVAIL = 1024 * 1024; // 3 gb for run

//static const uint_type MAIN_MEM_AVAIL = 150; // 3 gb for run

#endif // common_h
