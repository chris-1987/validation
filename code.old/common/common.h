#ifndef COMMON_H
#define COMMON_H


#include "namespace.h"

#include "myuint128.h"

#include "stxxl/bits/common/uint_types.h"

#include "stxxl/bits/io/syscall_file.h"

#include "stxxl/vector"

#include "stxxl/bits/containers/sorter.h"

#include "stxxl/bits/containers/priority_queue.h"

//#include "mpi.h"

#include <string>

#include <fstream>

#include <climits>

#include <cassert>

//type alias
typedef char int8;

typedef unsigned char uint8;

typedef short int16;

typedef unsigned short uint16;

typedef int int32;

typedef unsigned int uint32;

typedef stxxl::uint40 uint40;

typedef long long int int64;

typedef unsigned long long int uint64;

typedef __int128_t int128;

typedef __uint128_t uint128;

//memory related
static const size_t MAX_MEM = 3 * 1024 * 1024 * 1024UL; //!< available internal memory capacity

static const size_t K_512 = 512 * 1024; 

static const size_t M_512 = 512 * 1024 * 1024;

//static const uint16 K = 8192; //!< K-order, power of 2.

static const uint16 K = 256; //!< K-order, power of 2.




///
NAMESPACE_LCP_RAM_BEG

//parameters
static const uint32 W1 = 32; //!< partition into sets A and B. cache line is 64B, either 32 or 64 may be a proper value.

static const uint32 W2 = 32; //!< directly compare in literal if W <= W1. cache line is 64B, either 32 or 64 may be a proper value

//fingerprint related
typedef int32 FPTYPE_A; //!< must be integer

typedef int64 FPTYPE_B; //!< range as twice as FPTYPE_A

static const FPTYPE_A P = 2047483523; //!< big prime, fp[i] = ((fp[i - 1] * R + s[i]) mod P

static const FPTYPE_A R = 1732938289; //!< prime, ranges in [1, P)

NAMESPACE_LCP_RAM_END



///
NAMESPACE_KLCP_EM_BEG

//parameters
static const uint32 W = 16; //!< use W1 bytes for storing substr, that is W1 * 4 characters (2 bits per character) 

//fingerprint related
typedef int64 FPTYPE_A; //!<must be integer

typedef int128 FPTYPE_B; //!< range as twice as FPTYPE_A

static const FPTYPE_A P = 204748352342391211llu;

static const FPTYPE_A R = 17328327237172377llu;

//meta-template for determining uint_type
template<uint16 W>
struct choose_uint_type{};

template<>
struct choose_uint_type<16>{

	typedef uint16 uint_type;
};

template<>
struct choose_uint_type<32>{

	typedef uint32 uint_type;
};

template<>
struct choose_uint_type<64>{

	typedef uint64 uint_type;
};

template<>
struct choose_uint_type<128>{

	typedef myUint128 uint_type; 
};


static const uint8 L_TYPE = 0;

static const uint8 S_TYPE = 1;

static const uint8 LMS_TYPE = 2;

static const uint8 SENTINEL_TYPE = 3;

static const size_t MAX_ITEM = 512 * 1024;

NAMESPACE_KLCP_EM_END


//
NAMESPACE_LCP_EM_BEG

typedef int32 FPTYPE_A; //!<must be integer

typedef int64 FPTYPE_B; //!< range as twice as FPTYPE_A

static const FPTYPE_A P = 2047824239;

static const FPTYPE_A R = 1732327371;

NAMESPACE_LCP_EM_END

//
NAMESPACE_LCP_EM2_BEG

typedef int32 FPTYPE_A; //!<must be integer

typedef int64 FPTYPE_B; //!< range as twice as FPTYPE_A

static const FPTYPE_A P = 2047824239;

static const FPTYPE_A R = 1732327371;

static const uint8 L_TYPE = 0;

static const uint8 S_TYPE = 1;

static const uint8 LMS_TYPE = 2;

static const uint8 SENTINEL_TYPE = 3;

static const size_t MAX_ITEM = 4 * 1024 * 1024;

NAMESPACE_LCP_EM2_END

//
NAMESPACE_LCP_MPI_BEG

typedef int32 FPTYPE_A; //!<must be integer

typedef int64 FPTYPE_B; //!< range as twice as FPTYPE_A

static const FPTYPE_A P = 2047824239;

static const FPTYPE_A R = 1732327371;




NAMESPACE_LCP_MPI_END

#endif //COMMON_H
