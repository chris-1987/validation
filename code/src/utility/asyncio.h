////////////////////////////////////////////////////////////
/// Copyright (c) 2016, Sun Yat-sen University,
/// All rights reserved
/// \file asyncio.h
/// \brief read/write elements from/into files on disks by asynchronous I/O operations. 
///
/// 
/// \author Yi Wu
/// \date 2016.12
///////////////////////////////////////////////////////////

#ifndef __ASYNCIO_H
#define __ASYNCIO_H

#include "../common/common.h"

#include <algorithm>

#include <condition_variable>

#include <mutex>

#include <thread>

NAMESPACE_UTILITY_BEG

template<typename value_type>
class async_stream_reader{
private:

	template<typename T>
	struct buffer{

		T* mContent;

		size_t mSize;

		size_t mFilled;

		buffer(size_t _size) {

			mSize = size;

			mContent = (T*)malloc(mSize * sizeof(T));

			mFilled = 0;
		}

		void read_from_file(std::FILE* _f) {

			mFilled = std::fread(mContent, sizeof(T), mSize, f);
		}

		size_t size_in_bytes() const {

			return sizeof(T) * mFilled;
		}

		~buffer() {

			delete[] mContent;
		}
	};



};


NAMESPACE_UTILITY_END







#endif // __ASYNCIO_H
