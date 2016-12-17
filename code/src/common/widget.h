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
template<typename T>
struct ExVector {

	typedef typename stxxl::VECTOR_GENERATOR<T, 8 / sizeof(T) + 1, 2, K_512 * sizeof(T)>::result vector;
};

#endif
	
