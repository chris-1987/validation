////////////////////////////////////////////////////////////
/// Copyright (c) 2016, Sun Yat-sen University,
/// All rights reserved
/// \file asyncio.h
///
/// \brief read/write elements from/into files on disks by asynchronous I/O operations. 
/// 
/// Start a producer thread to sequentially read data into internal buffers, 
/// while the main thread consume the buffers in a FIFO sequence. 
/// The asynchronous read operations are multi-threaded based on producer-consumer model.
///   
/// \author Yi Wu
/// \date 2016.12
///////////////////////////////////////////////////////////

#ifndef __ASYNCIO_H
#define __ASYNCIO_H

#include "../common/common.h"

#include "basicio.h"

#include <algorithm>

#include <condition_variable>

#include <mutex>

#include <thread>

NAMESPACE_UTILITY_BEG

/// \brief read data from external disk using a asynchronous stream
template<typename value_type>
class async_stream_reader{

private:
	
	/// \brief read buffer
	template<typename T>
	struct buffer {

		T* m_content; ///< ptr to buffer payload
		
		uint64 m_size; ///< size of buffer
		
		uiint64 m_filled; ///< number of elements in buffer

		/// \brief ctor
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

			m_filled = std::fread(m_content, sizeof(T), m_size, _f);

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

		bool m_signal_stop; ///< no data remained to be read

		/// \brief ctor
		/// \param _bufnum number of buffers in the queue
		/// \param _bufsize size of each buffer
		buffer_queue(uint64 _bufnum = 0, uint64 _bufsize = 0) {

			m_signal_stop = false;

			for (uint64 i = 0; i < _bufnum; ++i) {

				m_queue.push(new buffer_type(_bufsize));
			}
		}

		/// \brief dtor
		~buffer_queue() {

			while (!m_queue.empty()) {

				buffer_type* buf = m_queue.front();

				m_queue.pop();

				delete buf;
			}
		}

		/// \brief pop the front buffer
		buffer_type* pop() {

			buffer_type* front = m_queue.front();

			m_queue.pop();

			return front;
		}

		/// \brief push into queue
		void push(buffer_type* _buf) {

			/// wait for locking mutex
			std::lock_guard<std::mutex> lk(m_mutex);

			m_queue.push(_buf);

			return;
		}

		/// \brief finish read, send signal
		void send_stop_signal() {

			std::lock_guard<std::mutex> lk(m_mutex);

			m_signal_stop = true;

			return;
		}

		/// \brief check empty of the queue
		bool empty() const {

			return m_queue.empty();
		}
	};

private:
	typedef buffer<value_type> buffer_type;

	typedef buffer_queue<value_type> buffer_queue_type;

	buffer_queue_type* m_empty_buffers; ///< queue of empty buffers

	buffer_queue_type* m_full_buffers; ///< queue of filled buffers

private:

	/// \brief read data in a disk file using stream
	template<typename T>
	static void producer(async_stream_reader<T>* _caller) {

		typedef buffer<T> buffer_type;

		// read file data
		while (true) {
		
			// wait to get lock
			std::unique_lock<std::mutex> lk(_caller->m_empty_buffers->m_mutex);

			// if there exists data remained to be read,
			// then extract an empty buffer for reading data
			while (_caller->m_empty_buffers->empty() && !(_caller->m_empty_buffers->m_signal_stop)) {

				_caller->m_empty_buffers->m_cv.wait(lk);
			}

			// receive the stop signal
			if (_caller->m_empty_buffers->m_signal_stop) {

				lk.unlock();

				break;
			}
			
			// at least one empty buffer is available now, get it 
			buffer_type* buffer = _caller->m_empty_buffers->pop();

			lk.unlock();

			// read a block of data
			buffer->read_from_file(_caller->m_file);

			_caller->m_bytes_read += buffer->size_in_bytes();

			// check if reach the eof file
			bool end_of_file = false;

			if (buffer->m_filled < buffer->m_size) {

				end_of_file = true;
			}

			// append the filled buffer to the filled queue
			// and notify the consumer to process the data
			if (buffer->m_filled > 0) {
		
				_caller->m_full_buffers->push(buffer);
				
				_caller->m_full_buffers->m_cv.notify_one();		
			}
			else { // recyle
			
				_caller->m_empty_buffers->push(buffer);
			}

			// send stop signal if reach the endofbit
			if (end_of_file) {
	
				_caller->m_full_buffers->send_stop_signal();

				_caller->m_full_buffers->m_cv.notify_one();

				break;
			}
		}
	}

public:

	/// \brief extract a filled buffer if exists any
	void consume() {

		// recycle current buffer (finished processing)
		if (m_cur_buffer != nullptr) {

			// push into empty-buffer queue
			m_empty_buffers->push(m_cur_buffer);

			//announce the notice
			m_empty_buffers->m_cv.notify_one();

			m_cur_buffer = nullptr;
		}

		// obtain mutex
		std::unique_lock<std::mutex> lk(m_full_buffers->m_mutex);

		// wait for a filled buffer
		while(m_full_buffers->empty() && !(m_full_buffers->m_signal_stop)) {

			m_full_buffers->m_cv.wait(lk);
		}

		m_cur_buffer_pos = 0;

		if (m_full_buffers->empty()) { // empty

			lk.unlock();

			m_cur_buffer_filled = 0;
		}
		else { // not empty, extract a filled buffer

			m_cur_buffer = m_full_buffers->pop();

			lk.unlock();

			m_cur_buffer_filled = m_cur_buffer->m_filled;
		}

		return;
	}

private:

	std::FILE* m_file; ///< file ptr

	uint64 m_bytes_read; ///< number of bytes already read

	uint64 m_cur_buffer_pos; ///< pos of next element to be read in current buffer

	uint64 m_cur_buffer_filled; ///< numbe of elements in current buffer

	buffer_type* m_cur_buffer; ///< ptr to current buffer

	std::thread* m_io_thread; ///< io thread


public:

	/// \brief ctor, read from a disk file
	///
	/// \param _fn filename
	async_stream_reader(const std::string& _fn) {

		init(_fn, (8ul << 20), 4ul, 0ul); 
	}

	/// \brief ctor, read from a file, skip a number of bytes
	///
	/// \param _fn filename
	/// \param _skip_bytes number of skipped bytes
	async_stream_reader(const std::string& _fn, const uint64& _skip_bytes) {
	
		init(_fn, (8ul << 20), 4ul, _skip_bytes);
	}


	/// \brief ctor, available buffer size is given
	/// 
	/// \param _fn filename
	/// \param _avail_mem available memory for buffers in bytes
	/// \param _bufnum number of buffers to be created
	async_stream_reader(const std::string& _fn, const uint64& _avail_mem, const uint64& _bufnum) {

		init(_fn, _avail_mem, _bufnum, 0UL);
	}
	
	/// \brief ctor, available buffer size is given, skip a number of bytes
	///
	/// \param _fn filename
	/// \param _avail_mem available memory in bytes
	/// \param _bufnum number of buffers
	/// \param _skip_bytes number of byts to skip
	async_stream_reader(const std::string& _fn, const uint64& _avail_mem, const uint64& _bufnum, const uint64 _skip_bytes) {

		init(_fn, _avail_mem, _bufnum, _skip_bytes);
	}

	/// \brief init 
	/// 
	/// \param _fn filename
	/// \param _avail_mem available memory in bytes
	/// \param _bufnum number of buffers
	/// \param _skip_bytes number of byts to skip
	void init(const std::string _fn, const uint64& _avail_mem, const uint64& _bufnum, const uint64& _skip_bytes) {

		m_file = basicIO::file_open_nobuf(_fn.c_str(), "r");

		if (_skip_bytes > 0) {

			std::fseek(m_file, _skip_bytes, SEEK_SET);
		}

		m_bytes_read = 0;

		m_cur_buffer_pos = 0;

		m_cur_buffer_filled = 0;

		m_cur_buffer = nullptr;

		uint64 bufsize = std::max(1ul, _avail_mem / sizeof(value_type) / sizeof(_bufnum);
		m_empty_buffers = new buffer_queue_type(_bufnum, _bufsize);

		m_full_buffers = new buffer_queue_type();

		// start
		m_io_thread = new std::thread(producer<value_type>, this);

		return;
	}	

	/// \brief get the next item in current buffer
	value_type read() {

		if (m_cur_buffer_pos == m_cur_buffer_filled) {

			consume();
		}

		return m_cur_buffer->m_content[m_cur_buffer_pos++];
	}

	/// \brief read a sequence of items in the stream
	/// 
	/// \param _des target container
	/// \param _num number of elements to be read
	void read(value_type* _des, uint64 _num) {

		while (_num > 0) {

			if (m_cur_buffer_pos == m_cur_buffer_filled) {

				consume();
			}

			uint64 cur_buf_left = m_cur_buffer_filled - m_cur_buffer_pos;

			uint64 tocopy = std::min(_num, cur_buf_left);

			for (uint64 i = 0; i < tocopy; ++i) {

				des[i] = m_cur_buffer->m_content[m_cur_buffer_pos + i];
			}

			m_cur_buffer_pos += tocopy;

			des += tocopy;

			_num -= tocopy;
		}

		return;
	}

	/// \brief skip a sequence of items in the stream
	void skip(uint64 _num) {

		while (_num > 0) {

			if (m_cur_buffer_pos == m_cur_buffer_filled) {

				consume();
			}

			uint64 toskip =  std::min(_num, m_cur_buffer_filled - m_cur_buffer_pos);

			m_cur_buffer_pos += toskip;

			_num -= toskip;
		}	

		return;
	}

	/// \brief peak the next item in the stream
	value_type peek() {

		if (m_cur_buffer_pos == m_cur_buffer_filled) {

			consume();
		}

		return m_cur_buffer->m_content[m_cur_buffer_pos];
	}


	/// \brief check if the stream is empty
	bool empty() {

		if (m_cur_buffer_pos == m_cur_buffer_filled) {

			consume();
		}

		return (m_cur_buffer_pos == m_cur_buffer_filled);
	}

	/// \brief return const ptr to internal buffer
	const value_Type* get_buf_ptr() const {

		return m_cur_buffer->m_content;
	}

	/// \brief return the number of items in the internal buffer
	uint64 get_buf_filled() const {

		return m_cur_buffer_filled;
	}

	/// \brief get read bytes
	uint64 bytes_read() const {

		reutrn m_bytes_read;
	}

	/// \brief dtor
	~async_stream_reader() {

		m_empty_buffers->send_stop_signal();

		m_empty_buffers->m_cv.notify_one();

		m_io_thread->join();

		delete m_empty_buffers;

		delete m_full_buffers;

		delete m_io_thread;

		if (m_file != stdin) {

			std::fclose(m_file);
		}

		if (m_cur_buffer != nullptr) {

			delete m_cur_buffer;
		}
	}

};

/// \brief write data into a disk file using an asynchronous stream 
template<typename value_type>
class async_stream_writer{

private:

	/// \brief buffer 
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
		}
		
		/// \brief dtor
		~buffer() {

			delete[] m_content;
		}

		/// \brief check is empty
		bool empty() const {

			return m_filled == 0;
		}

		/// \brief check is full
		bool full() const {

			return m_filled == m_size;
		}

		/// \brief return occuppied space in buffer
		uint64 size_in_bytes() const {

			return sizeof(T) * m_filled;
		}

		/// \brief return free space in buffer
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

		bool m_signal_stop; ///< stop signal

		/// \brief a queue of write buffer
		///
		/// \param number of buffers
		/// \param size of each buffer
		buffer_queue(uint64 _bufnum = 0, uint64 _bufsize = 0) {

			m_signal_stop = false;

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

		/// \brief pop a buffer
		buffer_type* pop() {

			buffer_type *front = m_queue.front();

			m_queue.pop();

			return front;
		}

		/// \brief push a buffer
		///
		/// \param ptr to buffer
		void push(buffer_type* _buf) {

			std::lock_guard<std::mutex> lk(m_mutex);
			
			m_queue.push(buf);
		}

		/// \brief stop writting
		void send_stop_signal() {

			std::lock_guard<std::mutex> lk(m_mutex);

			m_signal_stop = true;
		}

		/// \brief check empty
		bool empty() const {
			
			return m_queue.empty();

		}
	};

private:

	typedef buffer<value_type> buffer_type;

	typedef buffer_queue<buffer_type> buffer_queue_type;

	buffer_queue_type* m_empty_buffers; ///< empty buffers

	buffer_queue_type* m_full_buffers; ///< filled buffers

private:

	/// \brief producer
	template<typename T>
	static void producer(async_stream_writer<T>* _caller) {

		typedef buffer<T> buffer_type;

		while(true) {

			std::unique_lock<std::mutex> lk(_caller->m_full_buffers->m_mutex);

			while(_caller->m_full_buffers->empty() && !(_caller->m_full_buffers->m_signal_stop)) {

				_caller->m_full_buffers->m_cv.wait(lk);
			}

			if (_caller->m_full_buffers->empty()) {

				lk.unlock();

				break;
			}

			// extract a filled buffer
			buffer_type* buffer = _caller->m_full_buffers->pop();

			lk.unlock();

			// write data to disk
			buffer->write_to_file(_caller->m_file);

			// recyle the buffer
			_caller->m_empty_buffers->push(buffer);

			_caller->m_empty_buffers->m_cv.notify_one();
		}
	}
	
	// Get a free buffer from the collection of free buffers
	buffer_type* get_empty_buffer() {

		std::unique_lock<std::mutex> lk(m_empty_buffers->m_mutex);

		while(m_empty_buffers->empty()) {

			m_empty_buffers->m_cv.wait(lk);
		}

		buffer_type* front = m_empty_buffers->pop();

		lk.unlock();

		return front;
	}

private:

	std::FILE* m_file;

	uint64 m_bytes_written;

	uint64 m_bufsize;

	buffer_type* m_cur_buffer;

	std::thread* m_io_thread;

public:

	async_stream_writer(const std::string _fn, uint64 _avail_mem = (8ul << 20), uint64 _bufnum = 4ul, std::string _mode = std::string("w")) {

		m_file = BasicIO::file_open_nobuf(_fn.c_str(), write_mode);

		m_bufsize = _avail_mem / sizeof(value_type) / _bufnum;

		m_empty_buffers = new buffer_queue_type(_bufnum, _bufsize);

		m_full_buffers = new buffer_queue_type();

		// initialize empty buffer
		m_cur_buffer = get_empty_buffer();

		m_bytes_written = 0;

		m_io_thread = new std::thread(producer<value_type>, this);		
	}
	
	/// \brief write an item into stream
	void write(value_type x) {

		m_bytes_written += sizeof(value_type);

		m_cur_buffer->m_content[m_cur_buffer->m_filled++] = x;

		if (m_cur_buffer->full()) {

			m_full_buffers->push(m_cur_buffer);

			m_full_buffers->m_cv.noify_one();

			m_cur_buffer = get_empty_buffer();
		}
	}

	/// \brief 
};
NAMESPACE_UTILITY_END







#endif // __ASYNCIO_H
