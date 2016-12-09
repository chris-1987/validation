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


#ifndef TYPES_H
#define TYPES_H

#include <iostream>
#include <limits>

#include "namespace.h"

// integer alias
using int8 = char;

using uint8 = unsigned char;

using int16 = short;

using uint16 = unsigned short;

using int32 = int;

using uint32 = unsigned int;

using int64 = long long int;

using uint64 = unsigned long long int;

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

/// \brief PS = 8, int64/uint64
///
/// 64-bit operating system, support 64-bit address space
template<>
struct choose_int_types<8>{
	
	using int_type = int64;

	using uint_type = uint64;
};

using int_type = typename choose_int_types<my_pointer_size>::int_type;

using uint_type = typename choose_int_types<my_pointer_size>::uint_type;


/// \brief self-defined uint40
/// 
/// uint8 + uint32
class uint40{

private:
	
	uint32 low; ///< low part
	
	uint8 high; ///< high part

public:

	/// \brief default ctor
	uint40() :
		low(std::numeric_limits<uint32>::min()),
		high(std::numeric_limits<uint8>::min()) {}

	/// \brief component ctor
	uint40(const uint32& _low, const uint8& _high) :
		low(_low),
		high(_high) {}

	/// \brief copy ctor
	uint40(const uint40& _a) : 
		low(_a.low),
		high(_a.high) {}	 	

	/// \brief int32 ctor
	uint40(const int32& _a) :
		low(_a),
		high(0) {}

	/// \brief uint32 ctor
	uint40(const uint32& _a) :
		low(_a),
		high(0) {}

	/// \brief return as an uint64
	uint64 ull() const {

		return (uint64(high)) << 32 | (uint64)low;
	}


	/// \brief implicit cast to uint64
	operator uint64() const {

		return ull();	
	}	

	/// \brief prefix increment
	uint40& operator++() {

		if (low == std::numeric_limits<uint32>::max()) {

			++high, low = 0;
		}
		else {

			++low;
		}

		return *this;
	}

	///\brief prefix decrement
	uint40& operator--() {

		if (low == std::numeric_limits<uint32>::min()) {
			
			--high, low = std::numeric_limits<uint32>::min();
		}
		else {

			--low;
		}

		return *this;
	}

	/// \brief addition assignment
	uint40& operator+= (const uint40& _a) {

		uint64 add = low + _a.low;

		low = (uint32)(add & std::numeric_limits<uint32>::max());

		high = (uint8)(high + _a.high + ((add >> 32) & std::numeric_limits<uint8>::max()));

		return *this;
	}

	/// \brief check equality
	bool operator == (const uint40& _a) const {

		return (low == _a.low) && (high == _a.high);
	}

	/// \brief check inequality
	bool operator != (const uint40& _a) const {

		return (low != _a.low) || (high != _a.high);
	}

	/// \brief less-than 
	bool operator <(const uint40& _a) const {

		return (high < _a.high) || (high == _a.high && low < _a.low);
	}

	/// \brief no-greater-than
	bool operator <=(const uint40& _a) const {

		return (high < _a.high) || (high == _a.high && low <= _a.low);
	}

	/// \brief greater-than
	bool operator >(const uint40& _a) const {

		return (high > _a.high) || (high == _a.high && low > _a.low);
	}

	/// \brief no-less-than
	bool operator >=(const uint40& _a) const {

		return (high > _a.high) || (high == _a.high && low >= _a.low);
	}

	/// \brief output
	friend std::ostream& operator<< (std::ostream& _os, const uint40& _a) {

		return _os << _a.ull();
	}

 	/// \brief return min value
	static uint40& min() {

		static uint40 minVal = uint40();

		return minVal;
	}

	/// \brief return max value
	static uint40& max() {

		static uint40 maxVal = uint40(
			std::numeric_limits<uint32>::max(), 
			std::numeric_limits<uint8>::max());

		return maxVal;
	}

}__attribute__((packed));


/// \brief self-defined uint128
/// 
/// uint64 + uint64
class uint128{

private:
	
	uint64 low; ///< low part
	
	uint64 high; ///< high part

public:

	/// \brief default ctor
	uint128() :
		low(std::numeric_limits<uint64>::min()),
		high(std::numeric_limits<uint64>::min()) {}

	/// \brief component ctor
	uint128(const uint64& _low, const uint64& _high) :
		low(_low),
		high(_high) {}

	/// \brief copy ctor
	uint128(const uint128& _a) : 
		low(_a.low),
		high(_a.high) {}	 	

	/// \brief int32 ctor
	uint128(const int32& _a) :
		low(_a),
		high(0) {}

	/// \brief uint32 ctor
	uint128(const uint32& _a) :
		low(_a),
		high(0) {}

	/// \brief uint64 ctor
	uint128(const uint64& _a) :
		low(_a),
		high(0) {}

	/// \brief leftward shift, bitwise, self
	void operator <<= (const uint8 _bits) {

		high <<= _bits;
		
		high |= low >> (64 - _bits);

		low <<= _bits;
	}

	/// \brief or, bitwise
	void operator |= (const uint8 _ch) {

		low |= _ch;
	}	 

	/// \brief leftward shift, bitwise
	uint128 operator<<(const uint8 _bits) {
	
		uint128 res = 0;

		res.high = high << _bits;

		res.high |= low >> (64 - _bits);

		res.low = low << _bits;
		
		return res;
	}

	/// \brief and, bitwise
	uint128 operator& (const uint128& _a) const {

		uint128 res = 0;

		res.high = high & _a.high;

		res.low = low & _a.low;

		return res;
	} 

	/// \brief equality
	bool operator == (const uint128& _a) const {

		return (low == _a.low) && (high == _a.high);
	}

	/// \brief inequality
	bool operator != (const uint128& _a) const {

		return (low != _a.low) || (high != _a.high);
	}

	/// \brief return min value
	static uint128& min() {

		static uint128 minVal = uint128();

		return minVal;
	}

	/// \brief return max value
	static uint128& max() {

		static uint128 maxVal = uint128(
			std::numeric_limits<uint64>::max(),
			std::numeric_limits<uint64>::max());

		return maxVal;
	}

	friend std::ostream& operator << (std::ostream& _os, const uint128& _a) {

		return _os << "high: " << _a.high << " low: " << _a.low << std::endl;
	}
	/// \brief return as an uint64
	uint64 ull() const {

		return (uint64(high)) << 32 | (uint64)low;
	}

}__attribute__((packed));



#endif // _TYPES_H
