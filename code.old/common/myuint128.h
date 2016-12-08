#ifndef MYUINT128_H
#define MYUINT128_H

#include "namespace.h"

#include "stxxl/bits/common/uint_types.h"

/*!
 * construct an 128-bit unsigned integer stored in 16 bytes
 */

class myUint128{

public:
	
	typedef stxxl::uint8 uint8;

	typedef stxxl::int32 int32;

	typedef stxxl::uint32 uint32;

	typedef stxxl::uint64 uint64;

private:

	uint64 low; //!< lower part, uint64

	uint64 high; //!< higher part, uint64

public:

	//data member
	static const size_t digits = 128; //!< number of digits/bits, 128

	static const size_t bytes = 16; //!< number of bytes, 16

	
	//function memeber

	//! default ctor
	myUint128() : low(0), high(0) {}
		
	//! initialized from lower and higher parts
	myUint128(const uint64 & _low, const uint64 & _high) :  low(_low), high(_high) {}

	//! copy ctor
	myUint128(const myUint128 & _a) : low(_a.low), high(_a.high) {}
	
	//! initialized from an unsigned int
	myUint128(const uint32 & _a) : low (_a), high(0) {}

	//! initialized from anint, a >= 0
	myUint128(const int32 & _a) : low (_a), high(0) {}

	//! initialized from an unsigned long int
	myUint128(const uint64 & _a) : low(_a), high(0) {}

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
	myUint128 operator << (const uint8 _bits) {
		
		static myUint128 res = 0;

		res.high = high << _bits;
		
		res.high |= low >> (64 - _bits);

		res.low = low << _bits;

		return res;	
	}

	//! bitwise and
	myUint128 operator & (const myUint128 & _b) const{
		
		static myUint128 res = 0;

		res.high = high & _b.high;

		res.low = low & _b.low;

		return res;
	}

	//! check equality
	bool operator == (const myUint128 & b) const {

		return ((low == b.low) && (high == b.high));
	}

	//! check inequality
	bool operator != (const myUint128 & b) const {

		return ((low != b.low) || (high != b.high)); //or ! operator ==()
	}	


	static myUint128 min() {
		
		static myUint128 min_val = myUint128(std::numeric_limits<uint64>::min(), std::numeric_limits<uint64>::min());

		return min_val;
	}	

	static myUint128 max() {

		static myUint128 max_val = myUint128(std::numeric_limits<uint64>::max(), std::numeric_limits<uint64>::max());
		
		return max_val;
	}	

	friend std::ostream & operator << (std::ostream & _os, const myUint128 & _a) {

		_os << "high: " << _a.high << " low: " << _a.low << std::endl;

		return _os;
	}

	uint64 getHigh() const{
	
		return high;
	}

	uint64 getLow() const{
		
		return low;
	}
}__attribute__ ((packed));


namespace std {

template<>

class numeric_limits<myUint128>{

public:

	static const bool is_specialized = true;

	static myUint128 min() {
		
		return myUint128::min();
	}

	static myUint128 max() {
	
		return myUint128::max();
	}

	static myUint128 lowest() {

		return min();
	}

	static const bool is_signed = false;

	static const bool is_integer = true;

	static const bool is_exact = true;

	static const int radix = 2;

	static const int digits = myUint128::digits;

	static const myUint128 epsilon() {
		
		return myUint128(0, 0);
	}

	static const myUint128 round_error() {
		
		return myUint128(0, 0);
	}

	static const int min_exponent = 0;

	static const int min_exponent10 = 0;

	static const int max_exponent = 0;

	static const int max_exponent10 = 0;

	static const bool has_infinity = false;
};



} //std


#endif
