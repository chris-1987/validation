////////////////////////////////////////////////////////////
/// Copyright (c) 2016, Sun Yat-sen University,
/// All rights reserved
/// \file async_stream_writer.h
///
/// \brief write elements into disk file using asynchronous I/O operations. 
/// 
///   
/// \author Yi Wu
/// \date 2016.12
///////////////////////////////////////////////////////////

#ifndef __ASYNC_STREAM_WRITER_H
#define __ASYNC_STREAM_WRITER_H

#include "../common/common.h"

#include "basicio.h"

#include <algorithm>

#include <condition_variable>

#include <mutex>

#include <thread>

NAMESPACE_UTILITY_BEG


/// \brief buffer structure 
template<typename T>
struct buffer{
		
	T* m_content; ///< payload

	uint64 m_size; ///< size of buffer

	uint64 m_filled; ///< number of elements in buffer

	/// \brief default ctor
	///
	/// \param _size buffer size
	buffer(uint64 _size) {

		m_size = size;
	
		m_content = new T[m_size];

		m_filled = 0;
	}

	/// \brief write to file
	///
	/// \param _f file descriptor
	void write_to_file(std::FILE* _f) {

		BasicIO::write_to_file(m_content, m_filled, _f);

		m_filled = 0;
	}
		
	/// \brief dtor
	~buffer() {

		delete[] m_content;
	}

	/// \brief return occupied space (in bytes) in buffer
	uint64 size_in_bytes() const {

		return sizeof(T) * m_filled;
	}

	/// \brief remaing space in the buffer
	uint64 free_space() const {

		return m_size - m_filled;
	}
};


/// \brief buffer queue
template<typename buffer_type>
struct buffer_queue{
	
	std::queue<buffer_type*> m_queue; ///< buffer queue

	std::condition_variable m_cv; ///< condition variable

	std::mutex m_mutex; ///< mutex

	bool m_no_more_full_buffer_signal; ///< main thread inform I/O thread

	/// \brief a queue of write buffer
	///
	/// \param number of buffers
	/// \param size of each buffer
	buffer_queue(const uint64& _bufnum = 0, const uint64& _bufsize = 0) {

		m_no_more_full_buffer_signal = false;

		for (uint64 i = 0; i < _bufnum; ++i) {

			m_queue.push(new buffer_type(_bufsize));
		}
	}

	/// \brief dtor
	~buffer_queue() {

		while(!m_queue.empty()) {

			buffer_type* buf = m_queue.front();

			m_queue.pop();

			delete buf;
		}
	}

	/// \brief pop the front buffer in the queue
	///
	/// \note lock is obtained before calling the function in the code
	buffer_type* pop() {

		buffer_type* front = m_queue.front();

		m_queue.pop();

		return front;
	}

	/// \brief push a buffer
	///
	/// \param ptr to buffer
	/// \note lock before pushing
	void push(buffer_type* _buf) {

		std::lock_guard<std::mutex> lk(m_mutex);
			
		m_queue.push(buf);
	}

	/// \brief stop writting
	/// \note lock before adapting
	void send_no_more_full_buffer_signal() {

		std::lock_guard<std::mutex> lk(m_mutex);

		m_no_more_full_buffer_signal = true;
	}

	/// \brief check if empty
	bool empty() const {
			
		return m_queue.empty();
	}
};


/// \brief write data into a disk file using an asynchronous stream 
template<typename value_type>
class async_stream_writer{

private:

	typedef buffer<value_type> buffer_type;

	typedef buffer_queue<buffer_type> buffer_queue_type;

	buffer_queue_type* m_free_buffers; ///< empty-buffer queue

	buffer_queue_type* m_full_buffers; ///< full-buffer queue

	std::FILE* m_file; ///< file handler

	uint64 m_bytes_written; ///< I/O statistics

	uint64 m_bufsize; ///< buffer size

	buffer_type* m_cur_buffer; ///< pointer to current buffer 

	std::thread* m_io_thread; ///< pointer to I/O thread

private:

	/// \brief I/O thread procedure
	template<typename T>
	static void io_procedure(async_stream_writer<T>* _caller) {

		typedef buffer<T> buffer_type;

		while(true) {

			// lock the full-buffer queue
			std::unique_lock<std::mutex> lk(_caller->m_full_buffers->m_mutex);

			// wait for a full buffer
			while(_caller->m_full_buffers->empty() && !(_caller->m_full_buffers->m_no_more_full_buffer_signal)) {

				_caller->m_full_buffers->m_cv.wait(lk);
			}

			// if currently empty and no more full buffers
			if (_caller->m_full_buffers->empty() && _caller->m_full_buffers->m_no_more_full_buffer_signal) {

				lk.unlock();

				break;
			}

			// fetch a full-buffer 
			buffer_type* buffer = _caller->m_full_buffers->pop();

			lk.unlock();

			// write to disk
			buffer->write_to_file(_caller->m_file);
	
			// recycle and notify
			_caller->m_free_buffers->push(buffer);

			_caller->m_free_buffers->m_cv.notify_one();
		}
	}
	
	/// \brief get a free buffer
	/// 
	/// \note can always get an empty buffer
	buffer_type* get_free_buffer() {

		// lock
		std::unique_lock<std::mutex> lk(m_free_buffers->m_mutex);

		// wait for an empty buffe
		while(m_free_buffers->empty()) {

			m_free_buffers->m_cv.wait(lk);
		}

		// get an empty buffer
		buffer_type* front = m_free_buffers->pop();

		// unlock
		lk.unlock();

		return front;
	}

public:

	/// \brief constructor
	async_stream_writer(const std::string _fn, const uint64& _avail_mem = (8ul << 20), const uint64& _bufnum = 4ul, const std::string& _mode = std::string("w")) {

		m_file = BasicIO::file_open_nobuf(_fn.c_str(), _mode);

		m_bufsize = std::max(1ul, _avail_mem / sizeof(value_type) / _bufnum);

		m_free_buffers = new buffer_queue_type(_bufnum, _bufsize);

		m_full_buffers = new buffer_queue_type();

		// initialize empty buffer
		m_cur_buffer = get_free_buffer();

		m_bytes_written = 0;

		m_io_thread = new std::thread(consumer<value_type>, this);	
	}
	
	/// \brief write an element into stream
	void write(const value_type& _item) {

		// collect I/O consumption
		m_bytes_written += sizeof(value_type);

		// fill
		m_cur_buffer->m_content[m_cur_buffer->m_filled++] = _item;

		// if full, then push full-buffer into m_full_buffers
		if (m_cur_buffer->full()) {

			m_full_buffers->push(m_cur_buffer); // lock

			m_full_buffers->m_cv.noify_one(); // notify

			m_cur_buffer = get_empty_buffer();
		}

		return;
	}

	/// \brief write a sequence of items into the stream
	void write(const value_type* _src, uint64 _num) {

		// collect I/O consumption
		m_bytes_written += _num * sizeof(value_type);

		// fill
		while (_num > 0) {
		
			uint64 tocopy = std::min(_num, m_cur_buffer->free_space());

			std::copy(_src, _src + tocopy, m_cur_buffer->m_content + m_cur_buffer->m_filled);

			m_cur_buffer->m_filled += tocopy;

			_src += tocopy;

			_num -= tocopy;

			if (m_cur_buffer->full()) {

				m_full_buffers->push(m_cur_buffer);

				m_full_buffers->m_cv.notify_one();

				m_cur_buffer = get_free_buffer();
			}	
		}	

		return;
	}

	/// \brief collect I/O bytes
	uint64 bytes_written() const {

		return m_byts_written;
	}

	/// \brief dtor
	~async_stream_writer() {

		// current buffer may be non-empty, flush it
		if (!m_cur_buffer->empty()) {

			m_full_buffers->push(m_cur_buffer);

			m_full_buffers->m_cv.notify_one();

			m_cur_buffer = nullptr;
		}
			
		// send no more full buffer signal
		m_full_buffers->send_no_more_full_buffer_signal();

		m_full_buffers->m_cv.notify_one();

		// join
		m_io_thread->join();

		// dealloc
		delete m_empty_buffers;

		delete m_full_buffers;

		delete m_io_thread;

		std::fclose(m_file);

		if (m_cur_buffer != nullptr) {
			
			delete m_cur_buffer;
		}
	}
};


/// \brief multi-stream writer
///
/// Main thread write data to multiple files at the same time. For each file, create a set of buffer and an I/O thread to write data to disk by aysnchronous
/// I/O operations.
template<typename value_type>
class async_multi_stream_writer {

private:

	typedef buffer<value_type> buffer_type;

	typedef buffer_queue_type<buffer_type> buffer_queue_type;

	uint64* m_bytes_written; ///< I/O statistics

	uint64 m_filenum; ///< number of files

	uint64 m_bufsize; ///< size of buffer

	uint64 m_bufnum; ///< number of buffers in a buffer queue for each file

	std::FILE** m_files; ///< handlers of files

	buffer_type* cur_buffers; ///< pointer to current buffer for each file	

	buffer_queue_type** m_full_buffer_queues; ///< full buffer queue

	buffer_queue_type** m_free_buffer_queues; ///< free buffer queue
	
	std::thread** m_io_threads; ///< queue of io threads

private:

	/// \brief I/O thread that flush data into disk
	template<typename T>
	static void io_procedure(async_stream_writer<T>* _caller, const uint64& _file_idx) {

		typedef buffer<T> buffer_type;

		while (true) {

			// lock
			std::unique_lock<std::mutex> lk(_caller->m_full_buffer_queues[_file_idx]);

			// wait for a full buffer
			while (_caller->m_full_buffer_queues[_file_idx].empty() && !(_caller->m_full_buffer_queues[_file_idx].m_no_more_full_buffer_signal) {

				_caller->m_full_buffer_queues[_file_idx].m_cv.wait(lk);
			}
			
			// currently no full buffer and no more full buffer will be produced
			if (_caller->m_full_buffer_queues[_file_idx].empty() && _caller->m_full_buffer_queues[_file_idx].m_no_more_full_buffer_signal) {

				lk.unlock();

				break;
			}

			// get a full buffer
			buffer_type* buffer = _caller->m_full_buffer_queues[_file_idx]->pop();

			lk.unlock();

			// write to disk
			buffer->write_to_file(_caller->m_files[_file_idx]);

			// recycle
			_caller->m_empty_buffer_queues[_file_idx]->push(buffer);

			_caller->m_empty_buffer_queues[_file_idx]->m_cv.notify_one();
		}

		return;
	}

	/// \brief get a free buffer from the queue
	buffer_type* get_free_buffer(const uint64& _file_idx) {

		std::unique_lock<std::mutex> lk(m_free_buffer_queues[_file_idx].m_mutex);

		while (m_free_buffer_queues[_file_idx].empty()) {

			m_free_buffer_queues[_file_idx].m_cv.wait(lk);
		}

		buffer_type* front = m_free_buffer_queues[_file_idx].pop();

		lk.unlock();

		return front;
	}

public:

	/// \brief constructor
	async_multi_stream_writer(const std::string& _fn_prefix, const uint64& _filenum, const uint64& _mem_avail = (8ul << 20), const uint64& _bufnum = 4ul) {

		m_filenum = _filenum;

		m_bufnum = _bufnum;

		m_bufsize = std::max(1ul, _mem_avail / _filenum / _bufnum / sizeof(value_type));

		//
		m_bytes_written = new uint64[m_filenum];
		
		m_files = new std::FILE*[m_filenum];

		m_cur_buffers = new buffer_type*[m_filnum];

		m_full_buffer_queues = new buffer_queue_type*[m_filenum];
			
		m_free_buffer_queues = new buffer_queue_type*[m_filenum];

		m_io_threads = new std::thread*[m_filenum];

		for (uint64 i = 0; i < m_filenum; ++i) {

			//
			m_bytes_written[i] = 0;

			//
			m_files[i] = _f_arr[i];

			m_cur_buffers[i] = get_empty_buffer(i);

			m_full_buffers_queues[i] = new buffer_queue_type(_bufnum, m_bufsize);

			m_free_buffers_queues[i] = new buffer_queue_type();
		
			m_io_threads[i] = new std::thread(io_procedure<value_type>, this, i);
		}
	}


	/// \brief write an item
	void write(const uint64& _file_id, const value_type& _item) {

		m_bytes_write += sizeof(value_type);

		m_cur_buffers[_file_id]->m_content[m_cur_buffers[_file_id]->m_filled++] = _item;

		if (m_buffers[_file_id]->full()) {

			m_full_buffers_queues[_file_id]->push(m_buffers[_file_id]);
			
			m_full_buffers_queues[_file_id]->m_cv.notify_one();

			m_cur_buffers[_file_id] = get_free_buffer(_file_id); 
		}		

		return;
	}

	/// \brief write a sequence of items
	void write(const uint64& _file_id, const value_type* _src, const uint64& _num) {
	
		m_byte_written += _num * sizeof(value_type);

		while (_num > 0) {

			uint64 tocopy = std::min(_num, m_cur_buffers[_file_id]->free_space());

			std::copy(_src, _src + towrite, m_cur_buffers[_file_id]->m_content + m_cur_buffers[_file_id]->m_filled);

			m_buffers[i]->m_filled += tocopy;

			_num -= tocopy;

			_src += tocopy;

			if (m_cur_buffers[_file_idx]->full()) {

				m_full_buffer_queues[_file_id]->push(m_cur_buffers[_file_id]);

				m_full_buffer_queues[_file_id]->m_cv.notify_one();

				m_cur_buffers[_file_id] = get_free_buffer(_file_id);
			}
		}

		return;
	}


	/// \brief return performed I/O in bytes
	std::uint64 bytes_written() const {

		uint64 total_bytes_written = 0;

		for (uint64 i = 0; i < m_filenum; ++i) {
		
			total_bytes_written += m_bytes_writte[i];			
		}

		return total_bytes_written;
	}

	/// \brief destructor
	~async_multi_stream_writer() {

		// flush last non-empty buffers
		for (uint64 i = 0; i < m_filenum; ++i) {

			if (!(m_cur_buffers[i]->empty()) {

				m_full_buffer_queues[i]->push(m_cur_buffers[i]);

				std::unique_lock<std::mutex> lk(m_full_buffer_queues[i].m_mutex);

				m_full_buffer_queues[i].m_no_more_full_buffer_signal = true;

				lk.unlock();

				m_full_buffer_queues[i].notify_one();
			}
		}

		// join
		for (uint64 i = 0; i < m_filenum; ++i) {

			m_io_threads[i]->join();
		}
	
		// clear
		for (uint64 i = 0; i < m_filenum; ++i) {

			delete m_cur_buffers[i];

			std::fclose(m_files[i]);

			delete m_files[i];

			delete m_full_buffer_queues[i];

			delete m_free_buffer_queues[i];

			delete m_io_threads[i];	
		}

		delete[] m_bytes_written;

		delete[] m_cur_buffers;
	
		delete[] m_files;

		delete[] m_full_buffer_queues;

		delete[] m_free_buffer_queues;

		delete[] m_io_threads;
	}	
};


NAMESPACE_UTILITY_END






#endif // __ASYNCIO_H
