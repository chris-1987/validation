////////////////////////////////////////////////////////////
/// Copyright (c) 2016, Sun Yat-sen University,
/// All rights reserved
/// \file tuples.h
/// \brief definition of tuples
///
/// An implentation of self-defined pair and tuple.
/// \author Yi Wu
/// \date 2016.12
///////////////////////////////////////////////////////////


#ifndef TUPLES_H
#define TUPLES_H

/// \brief self-defined pair<1st, 2nd>
template<typename T1, typename T2>
struct Pair{

	T1 first; ///< 1st component

	T2 second; ///< 2nd component

	/// \brief default ctor
	Pair() : 
		first(std::numeric_limits<T1>::min()), 
		second(std::numeric_limits<T2>::min()) {}

	/// \brief ctor, assign two components separately
	Pair(const T1& _first, const T2& _second) :
		first(_first), second(_second){}

	/// \brief copy ctor
	Pair(const Pair& _a) : 
		first(_a.first), second(_a.second){}

	/// \brief return min_value
	static Pair& min() {

		static Pair minVal = Pair();

		return minVal;
	}

	/// \brief return max_value
	static Pair& max() {

		static Pair maxVal = Pair(
			std::numeric_limits<T1>::max(), 
			std::numeric_limits<T2>::max());

		return maxVal;
	}

	/// \brief output 
	friend std::ostream& operator<<(std::ostream& _os, const Pair& _a) {

		return _os << "1st: " << _a.first << " 2nd: " << _a.second << std::endl;
	}


}__attribute__((packed));


/// \brief self-defined triplet
template<typename T1, typename T2, typename T3>
struct Triple{

	T1 first; ///< 1st component

	T2 second; ///< 2nd component

	T3 third; //< 3rd component

	/// \brief default ctor
	Triple() : 
		first(std::numeric_limits<T1>::min()),	
		second(std::numeric_limits<T2>::min()),
		third(std::numeric_limits<T3>::min()) {}

	/// \brief assign components separately
	Triple(const T1& _first, const T2& _second, const T3& _third) :
		first(_first), second(_second), third(_third) {}

	/// \brief copy-ctor
	Triple(const Triple& _a) :
		first(_a.first),
		second(_a.second),
		third(_a.third) {}

	/// \brief return min value
	static Triple& min() {
		
		static Triple minVal = Triple();

		return minVal;
	}

	/// \brief return max value
	static Triple& max() {

		static Triple maxVal = Triple(
			std::numeric_limits<T1>::max(),
			std::numeric_limits<T2>::max(),
			std::numeric_limits<T3>::max());

		return maxVal;
	}

	/// \brief output
	friend std::ostream& operator<<(std::ostream& _os, const Triple& _a) {

		return _os << "1st: " << _a.first << " 2nd: " << _a.second << " 3rd: " << _a.third << std::endl;
	}

}__attribute__((packed));


/// \brief self-defined quadruple
template<typename T1, typename T2, typename T3, typename T4>
struct Quadruple{

	T1 first; ///< 1st component
	
	T2 second; ///< 2nd component

	T3 third; ///< 3rd component

	T4 forth; ///< 4th component

	/// \brief default ctor
	Quadruple() :
		first(std::numeric_limits<T1>::min()),
		second(std::numeric_limits<T2>::min()),
		third(std::numeric_limits<T3>::min()),	
		forth(std::numeric_limits<T4>::min()) {}


	/// \brief assign components separately
	Quadruple(const T1& _first, const T2& _second, const T3& _third, const T4& _forth) :
		first(_first), 
		second(_second), 
		third(_third), 
		forth(_forth) {}

	/// \brief copy ctor
	Quadruple(const Quadruple& _a) :
		first(_a.first),
		second(_a.second),
		third(_a.third),
		forth(_a.forth) {}

	/// \brief return min value
	static Quadruple& min() {

		static Quadruple minVal = Quadruple();

		return minVal;
	}

	/// \brief return max value
	static Quadruple& max() {

		static Quadruple maxVal = Quadruple(
			std::numeric_limits<T1>::max(),
			std::numeric_limits<T2>::max(),
			std::numeric_limits<T3>::max(),
			std::numeric_limits<T4>::max());

		return maxVal;
	}

	/// \brief output 
	friend std::ostream& operator<<(std::ostream& _os, const Quadruple& _a) {	

		return _os << "1st: " << _a.first << " 2nd: " << _a.second << " 3rd: " << _a.third << " forth: " << _a.forth << std::endl;

	}

}__attribute__((packed));


/// \brief self-defined Quintuple
template<typename T1, typename T2, typename T3, typename T4, typename T5>
struct Quintuple{

	T1 first; ///< 1st component

	T2 second; ///< 2nd component

	T3 third; ///< 3rd component

	T4 forth; ///< 4th component

	T5 fifth; ///< 5th component

	/// \brief default ctor
	Quintuple() :
		first(std::numeric_limits<T1>::min()),
		second(std::numeric_limits<T2>::min()),
		third(std::numeric_limits<T3>::min()),
		forth(std::numeric_limits<T4>::min()),
		fifth(std::numeric_limits<T5>::min()){}

	/// \brief assign component separately
	Quintuple(const T1& _first, const T2& _second, const T3& _third, const T4& _forth, const T5& _fifth) :
		first(_first),
		second(_second),
		third(_third),
		forth(_forth),
		fifth(_fifth) {}


	/// \brief copy ctor
	Quintuple(const Quintuple& _a) : 
		first(_a.first),
		second(_a.second),
		third(_a.third),
		forth(_a.forth),
		fifth(_a.fifth) {}

	/// \brief return min value
	static Quintuple& min() {

		static Quintuple minVal = Quintuple();

		return minVal;
	}

	/// \brief return max value
	static Quintuple& max() {

		static Quintuple maxVal = Quintuple(
			std::numeric_limits<T1>::max(),
			std::numeric_limits<T2>::max(),
			std::numeric_limits<T3>::max(),
			std::numeric_limits<T4>::max(),
			std::numeric_limits<T5>::max());

		return maxVal;
	}

	/// \brief output
	friend std::ostream& operator<<(std::ostream& _os, const Quintuple& _a) {

		return _os << "1st: " << _a.first << " 2nd: " << _a.second << " 3rd: " << _a.third << " 4th: " << _a.forth << " 5th: " << _a.fifth << std::endl;
	}
}__attribute__((packed));


/// \brief self-defined sixtuple
template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
struct Sixtuple{

	T1 first; ///< 1st component
	
	T2 second; ///< 2nd component

	T3 third; ///< 3rd component
	
	T4 forth; ///< 4th component

	T5 fifth; ///< 5th component

	T6 sixth; ///< 6th component 

	/// \brief default ctor
	Sixtuple() :
		first(std::numeric_limits<T1>::min()),
		second(std::numeric_limits<T2>::min()),
		third(std::numeric_limits<T3>::min()),
		forth(std::numeric_limits<T4>::min()),
		fifth(std::numeric_limits<T5>::min()),
		sixth(std::numeric_limits<T6>::min()) {}


	/// \brief assign components separately
	Sixtuple(const T1& _first, const T2& _second, const T3& _third, const T4& _forth, const T5& _fifth, const T6& _sixth) :
		first(_first),
		second(_second),
		third(_third),
		forth(_forth),
		fifth(_fifth),
		sixth(_sixth) {}
	
	/// \brief 
	Sixtuple(const Sixtuple& _a) :
		first(_a.first),
		second(_a.second),
		third(_a.third),
		forth(_a.forth),
		fifth(_a.fifth),
		sixth(_a.sixth) {}

	/// \brief
	static Sixtuple& min() {

		static Sixtuple minVal = Sixtuple();

		return minVal;
	}

	/// \brief
	static Sixtuple& max() {

		static Sixtuple maxVal = Sixtuple(
			std::numeric_limits<T1>::max(),
			std::numeric_limits<T2>::max(),
			std::numeric_limits<T3>::max(),
			std::numeric_limits<T4>::max(),
			std::numeric_limits<T5>::max(),
			std::numeric_limits<T6>::max());

		return maxVal;
	}
	
	friend std::ostream& operator<<(std::ostream& _os, const Sixtuple& _a) {

		std::cerr << "1st: " << _a.first << " 2nd: " << _a.second << " 3rd: " << _a.third << " 4th: " << _a.forth << " 5th: " << _a.fifth << " 6th: " << _a.sixth << std::endl; 
	}
}__attribute((packed));

		
#endif // TUPLES_H


