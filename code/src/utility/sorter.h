////////////////////////////////////////////////////////////
/// Copyright (c) 2016, Sun Yat-sen University,
/// All rights reserved
/// \file sorter.h
/// \brief sort elements in external memory
///
/// Input elements are grouped into multiple blocks. Elements in each block are sorted in RAM and the result is stored in external memory.
/// The block-wise results are combined into a whole using a heap in RAM at last. 
/// 
/// \author Yi Wu
/// \date 2016.12
///////////////////////////////////////////////////////////

#ifndef __SORTER_H
#define __SORTER_H

#include "../common/common.h"


NAMESPACE_UTILITY_BEG


class Sorter{

private:


public:
	
	Sorter(std::string& _) {}

};

NAMESPACE_UTILITY_END

#endif
	
