////////////////////////////////////////////////////////////
/// Copyright (c) 2016, Sun Yat-sen University,
/// All rights reserved
/// \file widget.h
/// \brief sort elements in external memory
///
/// Input elements are grouped into multiple blocks. Elements in each block are sorted in RAM and the result is stored in external memory.
/// The block-wise results are combined into a whole using a heap in RAM at last. 
/// 
/// \author Yi Wu
/// \date 2016.12
///////////////////////////////////////////////////////////

#ifndef __WIDGET_H
#define __WIDGET_H

#include "common.h"

#include "stxxl/sorter"

/// \brief sort pair by 1st component in ascending order
template<typename pair_type>
struct PairLess1st{

	bool operator()(const pair_type& _a, const pair_type& _b) const{

		return _a.first < _b.first;
	}
		
};


/// \brief sort pair by 2nd component in ascending order
template<typename pair_type>
struct PairLess2nd{

	bool operator()(const pair_type& _a, const pair_type& _b) const{

		return _a.second < _b.second;
	}
};

/// \brief template for vector
template<typename value_type>
struct ExVector {

	typedef typename stxxl::VECTOR_GENERATOR<value_type, 8 / sizeof(value_type) + 1, 2, K_512 * sizeof(value_type)>::result vector;
};

/// \brief function object for comparing tuples by their first component in ascending order
template<typename tuple_type>
struct tuple_less_comparator_1st{

	bool operator()(const tuple_type& _a, const tuple_type& _b) const {

		return _a.first < _b.first;
	}

	/// \brief min value
	tuple_type min_value() const {

		return tuple_type::min_value();
	}

	/// \brief max value
	tuple_type max_value() const {

		return tuple_type::max_value();
	}
	
};

/// \brief function object for comparing tuples by their first component in ascending order
template<typename tuple_type>
struct tuple_great_comparator_1st{

	bool operator()(const tuple_type& _a, const tuple_type& _b) const {

		return _a.first > _b.first;
	}

	/// \brief min value
	tuple_type min_value() const {

		return tuple_type::max_value();
	}

	/// \brief max value
	tuple_type max_value() const {

		return tuple_type::min_value();
	}
	
};

/// \brief function object for comparing tuples by their first two component in ascending order
template<typename tuple_type>
struct tuple_less_comparator_2nd{

	bool operator()(const tuple_type& _a, const tuple_type& _b) const {

		if (_a.first == _b.first) return _a.second < _b.second;

		return _a.first < _b.first;
	}

	/// \brief min value
	tuple_type min_value() const {

		return tuple_type::min_value();
	}

	/// \brief max value
	tuple_type max_value() const {

		return tuple_type::max_value();
	}
};

/// \brief template for sorter
template<typename tuple_type, typename tuple_comparator_type>
struct ExTupleSorter{

	typedef typename stxxl::sorter<tuple_type, tuple_comparator_type> sorter;
	
	typedef tuple_comparator_type comparator;	
};

#endif
	
