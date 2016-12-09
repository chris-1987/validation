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


#ifndef COMMON_H
#define COMMON_H

#include "namespace.h"

#include "types.h"

#include "stxxl/bits/common/uint_types.h"

#include "stxxl/bits/io/syscall_file.h"

#include "stxxl/vector"

#include "stxxl/bits/containers/sorter.h"

#include "stxxl/bits/containers/priority_queue.h"

#include <string>

#include <fstream>

#include <climits>

#include <cassert>

static const size_t MAX_MEM = 3 * 1024 * 1024 * 1024ull; ///< available RAM 

static const size_t K_512 = 512 * 1024; ///< 512K

static const size_t M_512 = K_512 * 1024; ///< 512 M



#endif // common_h
