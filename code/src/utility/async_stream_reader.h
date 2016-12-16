////////////////////////////////////////////////////////////
/// Copyright (c) 2016, Sun Yat-sen University,
/// All rights reserved
/// \file async_stream_reader.h
///
/// \brief read elements from disk file using asychronous I/O operations
///
/// \author Yi Wu
/// \date 2016.12
///////////////////////////////////////////////////////////

#ifndef __ASYNC_STREAM_READER_H
#define __ASYNC_STREAM_READER_H

#include "../common/common.h"

#include "basicio.h"

#include <algorithm>

#include <condition_variable>

#include <mutex>

#include <thread>

NAMESPACE_UTILITY_BEG

/// \brief read buffer
template<typename T>
struct buffer {
		
	T* m_content; ///< pointer to buffer payload
		
	uint64 m_size; ///< size of buffer
		
	uiint64 m_filled; ///< number of elements in buffer

	/// \brief constructor
	///
	/// \param _size size of buffer
	buffer(uint64 _size) {

		m_size = _size;

		m_content = new T[m_size];

		m_filled = 0;
	}

	/// \brief read a data block from a disk file
	///
	/// \param _f file descriptor
	void read_from_file(std::FILE* _f) {

		m_filled = BasicIO::read_from_file(m_content, sizeof(T), m_size, _f);

		return;
	}


	/// \brief total size of elements in buffer 
	uint64 size_in_bytes() const {

		return sizeof(T) * m_filled;
	}

	/// \brief dtor
	~buffer() {

		delete[] m_content;
	}
};

/// \brief queue of buffers
///
/// Buffers are orgainzed as a queue. Based on the producer-consumer mechanism,
/// data are read into buffers and processed in the RAM.
template<typename buffer_type>
struct buffer_queue{

	std::queue<buffer_type*> m_queue; ///< FIFO queue
		
	std::condition_variable m_cv; ///< condition variable for the queue

	std::mutex m_mutex; ///< mutex for the queue
	
	bool m_no_more_full_buffer_signal; ///< no more full buffer will be created

	/// \brief constructor
	/// \param _bufnum number of buffers in the queue
	/// \param _bufsize size of each buffer
	buffer_queue(uint64 _bufnum = 0, uint64 _bufsize = 0) {

		m_no_more_full_buffer_signal = false;

		for (uint64 i = 0; i < _bufnum; ++i) {

			m_queue.push(new buffer_type(_bufsize));
		}
	}
	
	/// \brief destructor
	~buffer_queue() {

		while (!m_queue.empty()) {

			buffer_type* buf = m_queue.front();

			m_queue.pop();

			delete buf;
		}
	}

	/// \brief use resource, lock must be obtained before calling the function
	buffer_type* pop() {

		buffer_type* front = m_queue.front();

		m_queue.pop();

		return front;
	}

	/// \brief recycle resource, lock must be obtained during the calling 
	void push(buffer_type* _buf) {

		std::lock_guard<std::mutex> lk(m_mutex);

		m_queue.push(_buf);

		return;
	}

	/// \brief I/O thread send the signal when it reaches the endofbits
	void send_no_more_full_buffer_signal() {

		std::lock_guard<std::mutex> lk(m_mutex);

		m_no_more_full_buffer_signal = true;

		return;
	}

	/// \brief check if queue is empty
	bool empty() const {

		return m_queue.empty();
	}
};

/// \brief read data from an external disk file using asynchronous stream
///
/// I/O thread continually continually do the following steps: 
/// (1) retrieve a buffer from empty-buffer queue
/// (2) load data into buffer
/// (3) put buffer into full-buffer queue
/// main thread read elements as following: if current buffer is not finished read, then read the next element; otherwise, put current buffer into empty-buffer queue and retrieve a buffer from full-buffer queue.
template<typename value_type>
class async_stream_reader{

private:

	//
	std::FILE* m_file; ///< file pointer

	uint64 m_bytes_read; ///< number of bytes already read

	uint64 m_cur_buffer_pos; ///<  pos of next element to be read in current buffer

	uint64 m_cur_buffer_filled; ///< number of elements in current buffer

	buffer_type* m_cur_buffer; ///< pointer to current buffer

	std::thread* m_io_thread; ///< io thread
	
	uint64 m_items_toread; ///< number of items to read

	uint64 m_items_read; ///< number of items read	

	//
	typedef buffer<value_type> buffer_type;

	typedef buffer_queue<value_type> buffer_queue_type;

	buffer_queue_type* m_free_buffers; ///< queue of free buffers

	buffer_queue_type* m_full_buffers; ///< queue of full buffers

private:


	/// \brief I/O procedure that load data into RAM using aysnchronous I/O operations
	template<typename T>
	static void io_procedure(async_stream_reader<T>* _caller) {

		typedef buffer<T> buffer_type;

		m_items_read = 0;

		// if there exists elements remained to be read
		while (true) {

			// get lock to m_mutex of m_free_buffer
			std::unique_lock<std::mutex> lk(_caller->m_free_buffers->m_mutex);

			// wait for an empty buffer
			while (_caller->m_free_buffers->empty()) {

				_caller->m_free_buffers->m_cv.wait(lk);
			}

			// retrieve an empty buffer
			buffer_type* buffer = _caller->m_free_buffers->pop(); 

			// unlock
			lk.unlock();

			// fill the buffer
			buffer->read_from_file(_caller->m_file);

			// collect I/O information
			_caller->m_bytes_read += buffer->size_in_bytes();

			// accumulate
			m_items_read += buffer->m_filled;

			// append the buffer to m_full_buffers and notify main thread if it is waiting
			_caller->m_full_buffers->push(buffer);

			_caller->m_full_buffers->m_cv.notify_one();
			
			// send signal to main thread, inform no more full buffers
			if (m_items_toread == m_items_read) {

				_caller->m_full_buffers->send_no_more_full_buffer_signal();

				_caller->m_full_buffers->m_cv.notify_one();

				break;
			}					
		}

		return;
	}

public:

	/// \brief get a full buffer
	void get_full_buffer() {

		// recycle current buffer (finished processing)
		if (m_cur_buffer != nullptr) {

			// push into queue
			m_free_buffers->push(m_cur_buffer);

			// notify the I/O thread if it is waiting for an free buffer
			m_free_buffers->m_cv.notify_one();

			m_cur_buffer = nullptr;
		}

		// obtain mutex
		std::unique_lock<std::mutex> lk(m_full_buffers->m_mutex);

		// wait if no full buffers and I/O thread may produce more full buffers
		while(m_full_buffers->empty() && !(m_full_buffers->m_no_more_full_buffer_signal)) {

			m_full_buffers->m_cv.wait(lk);
		}

		m_cur_buffer_pos = 0;

		if (m_full_buffers->empty()){ // no full buffers, m_cur_buffer_filled = 0

			lk.unlock();

			m_cur_buffer_filled = 0;
		}
		else {

			m_cur_buffer = m_full_buffers->pop(); // locked before calling the function

			lk.unlock();

			m_cur_buffer_filled = m_cur_buffer->m_filled;
		}

		return;
	}



public:

	/// \brief ctor, available buffer size is given, skip a number of bytes
	///
	/// \param _fn filename
	/// \param _avail_mem available memory in bytes
	/// \param _bufnum number of buffers
	/// \param _skip_bytes number of byts to skip
	async_stream_reader(const std::string& _fn, const uint64& _avail_mem = (8ul << 20), const uint64& _bufnum = 4ul) {

		m_items_toread = BasicIO::file_size(_fn) /  sizeof(value_type);

		m_file = BasicIO::file_open_nobuf(_fn.c_str(), "r");

		m_bytes_read = 0;

		m_cur_buffer_pos = 0;

		m_cur_buffer_filled = 0;

		m_cur_buffer = nullptr;

		uint64 bufsize = std::max(1ul, _avail_mem / sizeof(value_type) / _bufnum);

		m_free_buffers = new buffer_queue_type(_bufnum, _bufsize); // initially, all buffers are empty

		m_full_buffers = new buffer_queue_type();

		// start I/O thread
		m_io_thread = new std::thread(io_procedure<value_type>, this);

		return;
	}	

	/// \brief get next element in the stream
	///
	/// \note check if empty before calling the function
	value_type read() {

		if (m_cur_buffer_pos == m_cur_buffer_filled) {

			get_full_buffer(); // get full buffer
		}

		return m_cur_buffer->m_content[m_cur_buffer_pos++];
	}

	/// \brief read a sequence of items in the stream
	/// 
	/// \param _des target container
	/// \param _num number of elements to be read
	/// \note check if empty before calling the function
	void read(value_type* _des, uint64 _num) {

		while (_num > 0) {

			if (m_cur_buffer_pos == m_cur_buffer_filled) {

				get_full_buffer(); // get next full buffer
			}

			uint64 tocopy = std::min(_num, m_cur_buffer_filled - m_cur_buffer_pos);

			for (uint64 i = 0; i < tocopy; ++i) {

				des[i] = m_cur_buffer->m_content[m_cur_buffer_pos + i];
			}

			m_cur_buffer_pos += tocopy;

			des += tocopy;

			_num -= tocopy;
		}

		return;
	}

	/// \brief skip a sequence of elements in the stream
	///
	/// \param _num number of elements to be skipped
	void skip(uint64 _num) {

		while (_num > 0) {

			if (m_cur_buffer_pos == m_cur_buffer_filled) {

				get_full_buffer(); 
			}

			uint64 toskip = std::min(_num, m_cur_buffer_filled - m_cur_buffer_pos);

			m_cur_buffer_pos += toskip;

			_num -= toskip;
		}	

		return;
	}

	/// \brief check if empty
	bool empty() {

		if (m_cur_buffer_pos == m_cur_buffer_filled) {

			get_full_buffer();

			if (m_cur_buffer_filled == 0) {

				returnt true;
			}
		}

		return false;
	}

	/// \brief get read bytes
	uint64 bytes_read() const {

		return m_bytes_read;
	}

	/// \brief destructor
	~async_stream_reader() {

		m_io_thread->join();

		delete m_free_buffers;

		delete m_full_buffers;

		delete m_io_thread;

		std::fclose(m_file);

		if (m_cur_buffer != nullptr) {

			delete m_cur_buffer;
		}
	}

};


/// \brief main thread read multiple disk files simultaneously. 
///
/// For the situation (such as mergesort), a set of buffers are 
/// established for each file and an I/O thread is created to continually load data into buffers for fast read by main thread.
template<typename value_type>
class async_multi_stream_reader{
private:

	typedef buffer<value_type> buffer_type;

	typedef buffer_queue<buffer_type> buffer_queue_type;

	uint64* m_bytes_read; ///< io statistics

	uint64 m_bufsize; ///< size of buffer

	uint64 m_bufnum; ///< number of buffers in a queue, orgained as an FIFO queue for each file
	
	uint64 m_filenum; ///< number of files

	std::FILE **m_files; ///< pointers to files

	buffer_queue_type** m_full_buffer_queues; ///< pointers to full buffer queues

	buffer_queue_type** m_free_buffer_queues; ///< pointers to free buffer queues

	std::thread **m_io_threads; ///< pointers to I/O threads

	uint64* m_items_toread; ///< items to read in each file
	
	uint64* m_items_read; ///< items read in each file

	uint64* m_cur_buffer_pos; ///<  position of next item to be read in current buffer for each file

	uint64* m_cur_buffer_filled; ///< number of items in current buffer for each file

	buffer_type** m_cur_buffer; ///< pointer to current buffer for each file
	

private:

	/// \brief start an I/O thread for each disk file
	///
	/// \param _caller handler managed by main thread
	/// \param _file_idx file index
	template<typename T>
	static void io_procedure(async_multi_stream_reader* _caller, const uint64& _file_idx) {

		typedef buffer<T> buffer_type;

		//
		if (m_items_toread[_file_idx] == 0) return; // file is empty, no need to continue

		//
		m_items_read[_file_idx] = 0;

		while(true) {

			// get lock
			std::unique_lock<std::mutex> lk(_caller->m_free_buffer_queues[_file_idx].m_mutex);

			// wait for an empty buffer
			while (_caller->m_free_buffer_queues[_file_idx].empty()) {

				_caller->m_free_buffer_queues[_file_idx].m_cv.wait(lk);
			}

			// get an empty buffer
			buffer_type* buffer = _caller->m_free_buffer_queues[_file_idx].pop();

			lk.unlock();

			// read a data block
			buffer->read_from_file(_caller->m_files[_file_idx]);

			// collect I/O information
			_caller->m_bytes_read[_file_idx] += buffer->size_in_bytes();

			//
			m_items_read[_file_idx] += buffer->m_filled;

			// recycle buffer and notify main thread		
			_caller->m_full_buffer_queues[_file_idx].push(buffer);

			_caller->m_full_buffer_queues[_file_idx].m_cv.notify_one();

			// reach endofbit
			if (m_items_read[_file_idx] == m_items_toread[_file_idx]) {

				_caller->m_full_buffer_queues[_file_idx]->send_no_more_full_buffer_signal();

				_caller->m_full_buffer_queues[_file_idx]->m_cv.notify_one();

				break;
			}
		}

		return;
	}

	/// \brief get a full buffer
	void get_full_buffer(const uint64& _file_idx) {

		// recycle current buffer if any
		if (m_cur_buffer[_file_idx] != nullptr) {

			// push into queue
			m_empty_buffer_queues[_file_idx]->push(m_cur_buffer[_file_idx]);

			//notify the I/O thread
			m_empty_buffer_queues[_file_idx]->m_cv.notify_one();

			m_cur_buffer[_file_idx] = nullptr;
		}

		// get mutex
		std::unique_lock<std::mutex> lk(m_full_buffer_queues[_file_idx]->m_mutex);

		// wait a full buffer
		while(m_full_buffer_queues[_file_idx]->empty() && !(m_full_buffer_queues[_file_idx]->m_no_more_full_buffer_signal)) {
	
			m_full_buffer_queues[_file_idx]->m_cv.wait(lk);
		}

		m_cur_buffer_pos[_file_idx] = 0;

		// get a full buffer
		if (m_full_buffer_queues[_file_idx]->empty()) {

			lk.unlock();
		
			m_cur_buffer_filled[_file_idx] = 0;
		}
		else {

			m_cur_buffer[_file_idx] = m_full_buffers_queues[_file_idx]->pop();

			lk.unlock();

			m_cur_buffer_filled[_file_idx] = m_cur_buffer[_file_idx]->m_filled;
		}

		return;
	}


public:

	/// \brief constructor
	async_multi_stream_reader(const std::string& _fn_prefix, const uint64& _filenum, const uint64& _mem_avail = (8ul << 20), const uint64& _bufnum = 4ul) {

		m_filenum = _filenum;

		m_bufnum = _bufnum;

		m_bufsize = std::max(1UL, _mem_avail / _filenum / _bufnum / sizeof(value_type));

		//
		m_bytes_read = new uint64[m_filenum];

		m_items_toread = new uint64[m_filenum];
	
		m_items_read = new uint64[m_filenum];

		m_files = new std::FILE*[m_filenum];

		m_cur_buffer_pos = new uint64[m_filenum];

		m_cur_buffer_filled = new uint64[m_filenum];

		m_cur_buffer = new buffer_type*[m_filenum];

		m_full_buffer_queues = new buffer_queue_type*[m_filenum];

		m_free_buffer_queues = new buffer_queue_type*[m_filenum];

		m_io_threads = new std::thread*[m_filenum];

		for (uint64 i = 0; i < m_filenum; ++i) {
	
			//
			m_bytes_read[i] = 0;
		
			//
			std::string fn = _fn_prefix.append(std::to_string(i)); // prefix + file_idx

			m_items_toread[i] = BasicIO::file_size(_fn) / sizeof(value_type);
	
			//
			m_items_read[i] = 0;

			// 	
			m_files[i] = BasicIO::file_open_nobuf(fn.c_str(), "r");

			//
			m_cur_buffer_pos[i] = 0;

			//
			m_cur_buffer_filled[i] = 0;
			
			//
			m_cur_buffer[i] = nullptr;

			//
			m_full_buffer_queues[i] = new buffer_queue_type(m_bufnum, m_bufsize);

			m_free_buffer_queues[i] = new buffer_queue_type();
		
			//
			m_io_threads[i] = new std::thread(io_procedure<value_type>, this, i);
		}
	}

public:

	/// \brief read an element	
	///
	/// \param _file_idx file index, start from 0
	/// \note check if the stream is empty before calling the function
	value_type read(const uint64& _file_idx) {

		if (m_cur_buffer_pos[_file_idx] == m_cur_buffer_filled[_file_idx]) {

			get_full_buffer(_file_idx);
		}

		return m_cur_buffer[_file_idx]->m_content[m_cur_buffer_pos++];
	}

	/// \brief read a sequence of elements
	///
	/// \param _file_idx file index
	/// \param _des destination array
	/// \param _num _number of files to read
	/// \note check if the stream is empty before calling the function
	void read(const uint64& _file_idx, value_type* _des, uint64 _num) {

		while (_num > 0) {

			if (m_cur_buffer_pos[_file_idx] == m_cur_buffer_filled[_file_idx]) {

				get_full_buffer(_file_idx);
			}

			uint64 tocopy = std::min(_num, m_cur_buffer_filled[_file_idx] - m_cur_buffer_pos[_file_idx]);

			for (uint64 i = 0; i < tocopy; ++i) {

				_des[i] = m_cur_buffer[_file_idx]->m_content[m_cur_buffer_pos + i];
			}

			m_cur_buffer_pos[_file_idx] += tocopy;

			_des += tocopy;

			_num -= tocopy;
		}

		return;
	}	

	/// \brief skip a sequence of elements
	/// 
	/// \param _file_idx file index
	/// \param _num number of elements to skip
	/// \note check if the stream is empty before calling the function
	void skip(const uint64& _file_idx, uint64 _num) {

		while (_num > 0) {

			if (m_cur_buffer_pos[_file_idx] == m_cur_buffer_filled[_file_idx]) {

				get_full_buffer(_file_idx);
			}

			uint64 toskip = std::min(_num, m_cur_buffer_filled[_file_idx] - m_cur_buffer_pos[_file_idx]);

			m_cur_buffer_pos[_file_idx] += toskip;

			_num -= toskip;	
		}
		
		return;
	}

	/// \brief report bytes read
	uint64 bytes_read() {

		uint64 total_bytes_read = 0;

		for (uint64 i = 0; i < m_filenum; ++i) {

			total_byts_read += m_byts_read[i];			
		}

		return total_bytes_read;
	}

	/// \brief check if the stream is empty (reach endofit)
	bool empty(const uint64& _file_idx) {

		if (m_cur_buffer_pos[_file_idx] == m_cur_buffer_filled[_file_idx]) {

			get_full_buffer(_file_idx);

			if (m_cur_buffer_filled[_file_idx] == 0) return true;
		}

		return false;

	}

	/// \brief destructor
	~async_multi_stream_vbyte_reader() {

		// wait for I/O to finish
		for (uint64 i = 0; i < m_filenum; ++i) {

			m_io_threads[i]->join();

			delete m_io_threads[i];

			delete m_full_buffer_queues[i];

			delete m_free_buffer_queues[i];

			std::fclose(m_files[i]);

			delete m_files[i];

			if (m_cur_buffer[i] != nullptr) {

				delete m_cur_buffer[i];
			}
		}

		delete[] m_bytes_read;

		delete[] m_io_threads;

		delete[] m_full_buffer_queues;

		delete[] m_free_buffer_queues;

		delete[] m_files;
		
		delete[] m_cur_buffer;	
	
		delete[] m_items_toread;

		delete[] m_items_read;

		delete[] m_cur_buffer_pos;

		delete[] m_cur_buffer_filled;	
	}
};

#endif
