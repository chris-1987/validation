////////////////////////////////////////////////////////////
/// Copyright (c) 2016, Sun Yat-sen University,
/// All rights reserved
/// \file types.h
/// \brief definition and alias of integer types
///
/// Partly depends on "types.h" in STXXL
/// 
/// \author Yi Wu
/// \date 2016.12
///////////////////////////////////////////////////////////


#ifndef __TYPES_H
#define __TYPES_H

#include <iostream>
#include <limits>
#include <cstdint>

#include "namespace.h"


// integer alias
using int8 = std::int8_t;

using uint8 = std::uint8_t;

using int16 = std::int16_t;

using uint16 = std::uint16_t;

using int32 = std::int32_t;

using uint32 = std::uint32_t;

using int64 = std::int64_t;

using uint64 = std::uint64_t;

// integer type declarations
enum { my_pointer_size = sizeof(void*)};

/// \brief template for choosing int/uint type according to pointer size
template<int PS>
struct choose_int_types{};

/// \brief PS = 4, int32/uint32
///
/// 32-bit operating system, support 32-bit address space
template<>
struct choose_int_types<4>{

	using int_type = int32;

	using uint_type = uint32;
};

/// \brief pointer size = 8, 64-bit
///
/// 64-bit operating system, support 64-bit address space
template<>
struct choose_int_types<8>{
	
	using int_type = int64;

	using uint_type = uint64;
};

using int_type = typename choose_int_types<my_pointer_size>::int_type;

using uint_type = typename choose_int_types<my_pointer_size>::uint_type;

/// \brief self-defined uint pair
/// 
/// uint40(8 + 32) or uint48(16 + 32)
template<typename T>
class uint_pair{

private:
	typedef uint32 low_type; ///< type of low part, uint32

	typedef uint32 high_type; ///< type of high part, uint8 or uint16

	low_type low; ///< low part

	high_type high; ///< high part

	/// \brief default ctor
	uint_pair() : low(0), high(0) {}

	/// \brief component ctor
	uint_pair(const T1& _low, const T2& _high) : low(_low), high(_high) {}

	/// \brief copy ctor
	uint40(const uint_pair& _a) : low(_a.low), high(_a.high) {}

	/// \brief convert from int32
	uint40(const int32& _a) : low(_a.low), high(0) {}

	/// \brief convert from uint32
	uint40(const uint32& _a) : low(_a.low), high(0) {}

	/// \brief convert from uint64
	uint40(const uint64& _a) : low (_a & 0xFFFFFFFF), high ((_a >> 32) & std::numeric_limits<T>::max()) {}

	/// \brief implicitly convert to uint64
	operator uint64_t() const {

		return ((static_cast<uint64>(high) << 32) | (static_cast<uint64>(low));
	}

	/// \brief check equality
	bool operator == (const uint40& _a) const{
	
		return (low == _a.low) && (high == _a.high);
	}

	/// \brief check inequality
	bool operator != (const uint40& _a) const {

		return (low != _a.low) || (high != _a.high);
	}

	/// \brief prefix increment
	void operator++() {

		if (low == std::numeric_limits<uint32>::max()) {

			low = 0;

			++high;
		}

		else {

			++low;
		}

		return;
	}

	/// \brief prefix decrement
	void operator--() {

		if (low == std::numeric_limits<uint32>::min()) {

			low = std::numeric_limits<uint32>::max();

			--high;
		}
		else {

			--low;
		}

		return;	
	}
	
	/// \brief less-than
	bool operator < (const uint_pair& _a) {

		return (high < _a.high) || (high == _a.high && low < _a.low);
	}

	/// \brief no-greater-than
	bool operator <= (const uint_pair& _a) {

		return (high < _a.high) || (high == _a.high && low <= _a.low);
	}

	/// \brief greater-than
	bool operator > (const uint_pair& _a) {

		return (high > _a.high) || (high == _a.high && low > _a.low);
	}

	/// \brief no-lesser-than
	bool operator >= (const uint_pair& _a) {

		return (high > _a.high) || (high == _a.high && low >= _a.low);
	}

}__attribute__((packed));


using uint40 = uint_pair<uint8>; // 40-bit integer

using uint48 = uint_pair<uint16>; // 48-bit integer


/// \brief uint128 (64 + 64)
class uint128{

private:
	
	uint64 low; ///< low part

	uint64 high; ///< high part

public:

	/// \brief default ctor
	uint128() : low(0), high(0) {} 

	/// \brief component ctor
	uint128(const uint64& _low, const uint64& _high) : low(_low), high(_high) {}
		
	/// \brief copy ctor
	uint128(const uint128& _a) : low(_a.low), high(_a.high) {}
	
	/// \brief convert from int32
	uint128(const int32& _a) : low(_a), high(0) {}

	/// \brief convert from uint32
	uint128(const uint32& _a) : low(_a), high(0) {}

	/// \brief convert from uint64
	uint128(const uint64& _a) : low(_a), high(0) {}

	/// \brief bitwise leftward-shift, side-effect
	void operator <<= (const uint8 _bitnum) {

		high <<= _bitnum;

		high |= low >> (64 - _bitnum);

		low <<= _bitnum;

		return;
	}

	/// \brief bitwise leftward-shift, non-side-effect
	uint128 operator << (const uint8 _bitnum) {
		
		static uint128 res = 0;

		res.high = high << _bitnum;

		res.high |= low >> (64 - _bitnum);

		res.low = low << _bitnum;

		return;
	}

	/// \brief get value of high part		
	uint64 getHigh() const {

		return high;
	}	

	/// \brief get value of low part
	uint64 getLow() const {

		return low;
	}

}__attribute__((packed));



namespace std{

/// \brief numeric_limits for uint40
template<>
class numeric_limits<uint40>{

public:

	/// \brief return min value
	static uint40 min() {

		return uint40(std::numeric_limits<uint32>::min(), std::numeric_limits<uint8>::min());
	}

	/// \brief return max value
	static uint40 max() {

		return uint40(std::numeric_limits<uint32>::max(), std::numeric_limits<uint8>::max());
	}
};

/// \brief numeric_limits for uint48
template<>
class numeric_limits<uint48>{

public:

	/// \brief return min value
	static uint48 min() {

		return uint48(std::numeric_limits<uint32>::min(), std::numeric_limits<uint16>::min());
	}

	/// \brief return max value
	static uint48 max() {

		return uint48(std::numeric_limits<uint32>::max(), std::numeric_limits<uint16>::max());
	}
};

/// \brief numeric_limits for uint128
template<>
class numeric_limits<uint128>{

public:

	/// \brief return min value
	static uint128 min() {

		return uint128(std::numeric_limits<uint64>::min(), std::numeric_limits<uint64>::min());
	}

	/// \brief return max value
	static uint128 max() {

		return uint128(std::numeric_limits<uint64>::max(), std::numeric_limits<uint64>::max());
	}
};

}


#endif // _TYPES_H
