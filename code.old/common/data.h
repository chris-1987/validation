#ifndef DATA_H
#define DATA_H

#include "common.h"

//! use self-defined rather than std::pair
template<typename T1, typename T2>
struct Pair {

	T1 first;

	T2 second;

	Pair() :
		first(std::numeric_limits<T1>::min()),
		second(std::numeric_limits<T2>::min()){}

	Pair(T1 _first, T2 _second) :
		first(_first),
		second(_second){}

	Pair(const Pair & _item) :
		first(_item.first),
		second(_item.second){}

	static Pair & min() {

		static Pair minVal = Pair();

		return minVal;
	}

	static Pair & max() {

		static Pair maxVal = Pair(std::numeric_limits<T1>::max(), std::numeric_limits<T2>::max());

		return maxVal;
	}

	friend std::ostream & operator<<(std::ostream & _os, const Pair & _item) {
		return _os << "first: " << _item.first << " second: " << _item.second << std::endl;
	}

} 
__attribute__((packed));


template<typename T1, typename T2, typename T3>
struct Triple {

	T1 first;

	T2 second;

	T3 third;

	Triple() :
		first(std::numeric_limits<T1>::min()),
		second(std::numeric_limits<T2>::min()),
		third(std::numeric_limits<T3>::min()){}

	Triple(const Triple & _item) :
		first(_item.first),
		second(_item.second),
		third(_item.third){}

	Triple(T1 _first, T2 _second, T3 _third) :
		first(_first),
		second(_second),
		third(_third) {}

	static Triple & min() {

		static Triple min_val = Triple();

		return min_val;
	}

	static Triple & max() {

		static Triple max_val = Triple(
			std::numeric_limits<T1>::max(),
			std::numeric_limits<T2>::max(),
			std::numeric_limits<T3>::max());

		return max_val;
	}

	friend std::ostream & operator<<(std::ostream & _os, const Triple & _item) {
		return _os << "first: " << _item.first << " second: " << _item.second << " third: " << _item.third << std::endl;
	}

} 
__attribute__((packed));


template<typename T1, typename T2, typename T3, typename T4>
struct Quadruple {

	T1 first;

	T2 second;

	T3 third;

	T4 forth;

	Quadruple() :
		first(std::numeric_limits<T1>::min()),
		second(std::numeric_limits<T2>::min()),
		third(std::numeric_limits<T3>::min()),
		forth(std::numeric_limits<T4>::min()){}

	Quadruple(const T1 & _first, const T2 & _second, const T3 & _third, const T4 & _forth) :
		first(_first), 
		second(_second), 
		third(_third), 
		forth(_forth) {}

	Quadruple(const Quadruple & _item) :
		first(_item.first),
		second(_item.second),
		third(_item.third),
		forth(_item.forth){}

	static Quadruple & min() {

		static Quadruple min_val = Quadruple();
		
		return min_val;
	}

	static Quadruple & max() {

		static Quadruple max_val = Quadruple(
			std::numeric_limits<T1>::max(),
			std::numeric_limits<T2>::max(),
			std::numeric_limits<T3>::max(),
			std::numeric_limits<T4>::max());

		return max_val;
	}

	friend std::ostream & operator<<(std::ostream & _os, const Quadruple & _item) {
		return _os << "first: " << _item.first << " second: " << _item.second << " third: " << _item.third << " forth: " << _item.forth << std::endl;
	}

}
__attribute__((packed));


template<typename T1, typename T2, typename T3, typename T4, typename T5>
struct Quintuple {

	T1 first;

	T2 second;

	T3 third;

	T4 forth;

	T5 fifth;

	Quintuple() :
		first(std::numeric_limits<T1>::min()),
		second(std::numeric_limits<T2>::min()),
		third(std::numeric_limits<T3>::min()),
		forth(std::numeric_limits<T4>::min()),
		fifth(std::numeric_limits<T5>::min()){}

	Quintuple(const T1 & _first, const T2 & _second, const T3 & _third, const T4 & _forth, const T5 & _fifth) :
		first(_first), 
		second(_second), 
		third(_third), 
		forth(_forth),
		fifth(_fifth) {}

	Quintuple(const Quintuple & _item) :
		first(_item.first),
		second(_item.second),
		third(_item.third),
		forth(_item.forth),
		fifth(_item.fifth){}

	static Quintuple & min() {

		static Quintuple min_val = Quintuple();
		
		return min_val;
	}

	static Quintuple & max() {

		static Quintuple max_val = Quintuple(
			std::numeric_limits<T1>::max(),
			std::numeric_limits<T2>::max(),
			std::numeric_limits<T3>::max(),
			std::numeric_limits<T4>::max(),
			std::numeric_limits<T5>::max());

		return max_val;
	}

	friend std::ostream & operator<<(std::ostream & _os, const Quintuple & _item) {
		return _os << "first: " << _item.first << " second: " << _item.second << " third: " << _item.third << " forth: " << _item.forth << " fifth: " << _item.fifth << std::endl;
	}

}
__attribute__((packed));


template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
struct Sixtuple {

	T1 first;

	T2 second;

	T3 third;

	T4 forth;

	T5 fifth;

	T6 sixth;

	Sixtuple() :
		first(std::numeric_limits<T1>::min()),
		second(std::numeric_limits<T2>::min()),
		third(std::numeric_limits<T3>::min()),
		forth(std::numeric_limits<T4>::min()),
		fifth(std::numeric_limits<T5>::min()),
		sixth(std::numeric_limits<T6>::min()){}

	Sixtuple(const T1 & _first, const T2 & _second, const T3 & _third, const T4 & _forth, const T5 & _fifth, const T6 & _sixth) :
		first(_first), 
		second(_second), 
		third(_third), 
		forth(_forth),
		fifth(_fifth),
		sixth(_sixth) {}

	Sixtuple(const Sixtuple & _item) :
		first(_item.first),
		second(_item.second),
		third(_item.third),
		forth(_item.forth),
		fifth(_item.fifth),
		sixth(_item.sixth){}

	static Sixtuple & min() {

		static Sixtuple min_val = Sixtuple();
		
		return min_val;
	}

	static Sixtuple & max() {

		static Sixtuple max_val = Sixtuple(
			std::numeric_limits<T1>::max(),
			std::numeric_limits<T2>::max(),
			std::numeric_limits<T3>::max(),
			std::numeric_limits<T4>::max(),
			std::numeric_limits<T5>::max(),
			std::numeric_limits<T6>::max());

		return max_val;
	}

	friend std::ostream & operator<<(std::ostream & _os, const Sixtuple & _item) {
		return _os << "first: " << _item.first << " second: " << _item.second << " third: " << _item.third << " forth: " << _item.forth << " fifth: " << _item.fifth << " sixth: " << std::endl;
	}

}
__attribute__((packed));
//! lesser comparator 

//! sort tuples in ascending order using external memory (in combination with stxxl::sorter)
template<typename Tuple>
struct TupleLessComparator1 {

	bool operator()(const Tuple & _lhs, const Tuple & _rhs) const {

		return _lhs.first < _rhs.first;
	}

	Tuple & min_value() const {
	
		return Tuple::min();
	}

	Tuple & max_value() const {
	
		return Tuple::max();
	}
};

template<typename Tuple>
struct TupleLessComparator2 {

	bool operator()(const Tuple & _lhs, const Tuple & _rhs) const {

		if (_lhs.first == _rhs.first) {
			
			return _lhs.second < _rhs.second;
		}

		return _lhs.first < _rhs.first;
	}

	Tuple & min_value() const {
	
		return Tuple::min();
	}

	Tuple & max_value() const {
	
		return Tuple::max();
	}
};


template<typename Tuple>
struct TupleLessComparator3 {

	bool operator()(const Tuple & _lhs, const Tuple & _rhs) const {

		if (_lhs.first == _rhs.first) {
		
			if (_lhs.second == _rhs.second) {
				
				return _lhs.third < _rhs.third;
			}	
				
			return _lhs.second < _rhs.second;
		}

		return _lhs.first < _rhs.first;
	}

	Tuple & min_value() const {
	
		return Tuple::min();
	}

	Tuple & max_value() const {
	
		return Tuple::max();
	}
};


//! sort tuples in descending order using external memory (in combination with stxxl::sorter)
template<typename Tuple>
struct TupleGreatComparator1 {

	bool operator()(const Tuple & _lhs, const Tuple & _rhs) const {

		return _lhs.first > _rhs.first;
	}

	Tuple & min_value() const {
	
		return Tuple::max();
	}

	Tuple & max_value() const {
	
		return Tuple::min();
	}
};


template<typename Tuple>
struct TupleGreatComparator2 {

	bool operator()(const Tuple & _lhs, const Tuple & _rhs) const {

		if (_lhs.first == _rhs.first) {
			
			return _lhs.second > _rhs.second;
		}

		return _lhs.first > _rhs.first;
	}

	Tuple & min_value() const {
	
		return Tuple::max();
	}

	Tuple & max_value() const {
	
		return Tuple::min();
	}
};

template<typename T>
struct ExVector {

	typedef typename stxxl::VECTOR_GENERATOR<T, 8 / sizeof(T) + 1, 2, K_512 * sizeof(T)>::result vector;
};


template<typename Tuple, typename TupleLessComparator>
struct ExTupleAscSorter {

	typedef typename stxxl::sorter<Tuple, TupleLessComparator> sorter;

	typedef TupleLessComparator comparator;
};


template<typename Tuple, typename TupleLessComparator, size_t IntMemory, size_t MaxItems, size_t Tune = 6>
struct ExTupleMaxHeap{

	typedef typename stxxl::PRIORITY_QUEUE_GENERATOR<Tuple, TupleLessComparator, IntMemory, MaxItems, Tune>::result heap;
};

template<typename Tuple, typename TupleGreatComparator, size_t IntMemory, size_t MaxItems, size_t Tune = 6>
struct ExTupleMinHeap{

	typedef typename stxxl::PRIORITY_QUEUE_GENERATOR<Tuple, TupleGreatComparator, IntMemory, MaxItems, Tune>::result heap;
};

#endif // DATA_H
