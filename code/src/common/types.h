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

#include "stxxl/bits/common/uint_types.h"

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


//
class uint40 {

public:
	
	typedef uint32 low_type;

	typedef uint8 high_type;

private: 
	low_type low;

	high_type high;

  public:
    uint40() {}
    uint40(std::uint32_t l, std::uint8_t h) : low(l), high(h) {}
    uint40(const uint40& a) : low(a.low), high(a.high) {}
    uint40(const std::int32_t& a) : low(a), high(0) {}
    uint40(const std::uint32_t& a) : low(a), high(0) {}
    uint40(const std::uint64_t& a) : low(a & 0xFFFFFFFF), high((a >> 32) & 0xFF) {}
    uint40(const std::int64_t& a) : low(a & 0xFFFFFFFFL), high((a >> 32) & 0xFF) {}

	// set high part
	void set_high(const uint8& _high) {

		high = _high;
	}

	// set low part
	void set_low(const uint32& _low) {

		low = _low;
	}

	//
	void set(const uint32& _low, const uint8 _high) {

		low = _low;

		high = _high;
	}

	//
	uint32 get_low() {

		return low;
	}

	//
	uint8 get_high() {

		return high;
	}

    inline operator uint64_t() const { return (((std::uint64_t)high) << 32) | (std::uint64_t)low;  }
    inline bool operator == (const uint40& b) const { return (low == b.low) && (high == b.high); }
    inline bool operator != (const uint40& b) const { return (low != b.low) || (high != b.high); }
} __attribute__((packed));






class uint128{

private:

	uint64 low; //!< lower part, uint64

	uint64 high; //!< higher part, uint64

public:

	//data member
	static const size_t digits = 128; //!< number of digits/bits, 128

	static const size_t bytes = 16; //!< number of bytes, 16

	
	//function memeber

	//! default ctor
	uint128() : low(0), high(0) {}
		
	//! initialized from lower and higher parts
	uint128(const uint64 & _low, const uint64 & _high) :  low(_low), high(_high) {}

	//! copy ctor
	uint128(const uint128 & _a) : low(_a.low), high(_a.high) {}
	
	//! initialized from an unsigned int
	uint128(const uint32 & _a) : low (_a), high(0) {}

	//! initialized from anint, a >= 0
	uint128(const int32 & _a) : low (_a), high(0) {}

	//! initialized from an unsigned long int
	uint128(const uint64 & _a) : low(_a), high(0) {}

	//! bitwise shift, _bits bits leftward, self revision
	void operator <<= (const uint8 _bits) {

		high <<= _bits;

		high |= low >> (64 - _bits);

		low <<= _bits;
	}

	//! bitwise or
	void operator |= (const uint8 _ch) {
		
		low |= _ch;
	}

	//! biwise shift, _bits bits leftward	
	uint128 operator << (const uint8 _bits) {
		
		static uint128 res = 0;

		res.high = high << _bits;
		
		res.high |= low >> (64 - _bits);

		res.low = low << _bits;

		return res;	
	}

	//! bitwise and
	uint128 operator & (const uint128 & _b) const{
		
		static uint128 res = 0;

		res.high = high & _b.high;

		res.low = low & _b.low;

		return res;
	}

	//! check equality
	bool operator == (const uint128 & b) const {

		return ((low == b.low) && (high == b.high));
	}

	//! check inequality
	bool operator != (const uint128 & b) const {

		return ((low != b.low) || (high != b.high)); //or ! operator ==()
	}	


	static uint128 min() {
		
		static uint128 min_val = uint128(std::numeric_limits<uint64>::min(), std::numeric_limits<uint64>::min());

		return min_val;
	}	

	static uint128 max() {

		static uint128 max_val = uint128(std::numeric_limits<uint64>::max(), std::numeric_limits<uint64>::max());
		
		return max_val;
	}	

	friend std::ostream & operator << (std::ostream & _os, const uint128 & _a) {

		_os << "high: " << _a.high << " low: " << _a.low << std::endl;

		return _os;
	}

	uint64 get_high() const{
	
		return high;
	}

	uint64 get_low() const{
		
		return low;
	}
}__attribute__ ((packed));



namespace std {


template<>
class numeric_limits<uint40> {
  public:
    static uint40 min() {
      return uint40(std::numeric_limits<std::uint32_t>::min(),
          std::numeric_limits<std::uint8_t>::min());
    }

    static uint40 max() {
      return uint40(std::numeric_limits<std::uint32_t>::max(),
          std::numeric_limits<std::uint8_t>::max());
    }
};

template<>
class numeric_limits<uint128>{

public:

	static uint128 min() {
		
		return uint128::min();
	}

	static uint128 max() {
	
		return uint128::max();
	}

};

}
#endif // _TYPES_H
