///////////////////////////////////////////////////////////////////////////////////////////////////////
/// Copyright (c) 2016, Sun Yat-sen University,
/// All rights reserved
/// \file validate.h
/// \brief first method for validating suffix and LCP arrays
///
/// Given suffix and LCP arrays, validate their correctness using Karp-Rabin Finger-printing function.
///
/// \author Yi Wu
/// \date 2016.12
////////////////////////////////////////////////////////////////////////////////////////////////////////


#ifndef __VALIDATE_H
#define __VALIDATE_H

#include "../common/common.h"

/// \brief validate suffix and LCP arrays using Karp-Rarbin finger-printing function
///
/// For i in [0, n), compare fp[sa[i], sa[i] + lcp[i] - 1] with fp[sa[i - 1], sa[i - 1] + lcp[i] - 1] and x[sa[i] + lcp[i]] with x[sa[i - 1] + lcp[i]].
/// 
template<typename T>
class Validate{

	


};




#endif // __VALIDATE_H
