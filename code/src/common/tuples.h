#ifndef __TUPLES_H
#define __TUPLES_H

/// \brief structure pair
template<typename T1, typename T2>
struct pair{

	T1 first;

	T2 second;

	/// \brief constructor, default
	pair(): first(0), second(0) {}

	/// \brief constructor, copy
	pair(const pair& _item) : first(_item.first), second(_item.second) {}

	/// \brief constructor, component
	pair(const T1& _first, const T2& _second) : first(_first), second(_second) {}

	/// \brief min value
	static pair& min_value() {

		static pair min_val = pair();

		return min_val;
	}

	/// \brief max vlaue
	static pair& max_value() {

		static pair max_val = pair(std::numeric_limits<T1>::max(), std::numeric_limits<T2>::max());

		return max_val;
	}

	/// \brief output
	friend std::ostream& operator << (std::ostream _os, const pair& _item) {

		return _os << "first: " << _item.first << " second: " << _item.second << std::endl;

	}
}__attribute__((packed));

/// \brief structure triple
template<typename T1, typename T2, typename T3>
struct triple{

	T1 first;

	T2 second;

	T3 third;

	/// \brief constructor, default
	triple(): first(0), second(0), third(0) {}

	/// \brief constructor, copy
	triple(const triple& _item) : first(_item.first), second(_item.second), third(_item.third) {}

	/// \brief constructor, component
	triple(const T1& _first, const T2& _second, const T3& _third) : first(_first), second(_second), third(_third) {}

	/// \brief min value
	static triple& min_value() {

		static triple min_val = triple();

		return min_val;
	}

	/// \brief max value
	static triple& max_value() {

		static triple max_val = triple(std::numeric_limits<T1>::max(), std::numeric_limits<T2>::max(), std::numeric_limits<T3>::max());

		return max_val;
	}

	/// \brief output
	friend std::ostream& operator << (std::ostream _os, const triple& _item) {

		return _os << "first: " << _item.first << " second: " << _item.second << " third: " << _item.third << std::endl;
	}

}__attribute__((packed));


/// \brief structure quadruple
template<typename T1, typename T2, typename T3, typename T4>
struct quadruple{

	T1 first;

	T2 second;

	T3 third;

	T4 forth;

	/// \brief constructor, default
	quadruple(): first(0), second(0), third(0), forth(0) {}

	/// \brief constructor, copy
	quadruple(const quadruple& _item) : first(_item.first), second(_item.second), third(_item.third), forth(_item.forth) {}

	/// \brief constructor, component
	quadruple(const T1& _first, const T2& _second, const T3& _third, const T4& _forth) : first(_first), second(_second), third(_third), forth(_forth) {}

	/// \brief min value
	static quadruple& min_value() {

		static quadruple min_val = quadruple();

		return min_val;
	}

	/// \brief max value
	static quadruple& max_value() {

		static quadruple max_val = quadruple(std::numeric_limits<T1>::max(), std::numeric_limits<T2>::max(), std::numeric_limits<T3>::max(), std::numeric_limits<T4>::max());

		return max_val;
	}

	/// \brief output
	friend std::ostream& operator << (std::ostream _os, const quadruple& _item) {

		return _os << "first: " << _item.first << " second: " << _item.second << " third: " << _item.third << " forth: " << _item.forth << std::endl;
	}

}__attribute__((packed));



#endif

