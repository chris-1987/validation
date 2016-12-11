///////////////////////////////////////////////////////////////////////////////
/// Copyright (c) 2016, Sun Yat-sen University,
/// All rights reserved
/// \file syncio.h
/// \brief read/write elements from/into files on disks by sequential I/O operations.
///
/// 
/// \author Yi Wu
/// \date 2016.12
///////////////////////////////////////////////////////////////////////////////

#ifndef __SYNCIO_H
#define __SYNCIO_H

NAMESPACE_UTILITY_BEG

/// \brief synchronous read operations
class SyncIO {
private:

	/// \brief open file
	static std::FILE* file_open(const std::string _fn, const std::string _mode);

	/// \brief open file
	///
	/// no buffer cache
	static std::FILE* file_open_nobuf(const std::stream _fn, const std::string _mode);

	/// \brief return file size in bytes
	static size_t file_size(const std::string _fn);

	/// \brief check if exists
	bool file_exists(const std::string _fn);

	/// \brief delete file
	void file_delete(const std::string _fn);

	/// \brief write data into file
 	///
	/// output file is specified by file descripter
	/// 
	template<typename T>
	void write_to_file(const T* _src, const size_t& _cnt, std::FILE *_f);

	/// \brief write data to file
	///
	/// Output file is specified by file name.
	/// Close the file after the write operation.
	template<typename T>
	void write_to_file(const T* _src, const size_t& _cnt, const std::string& _fn);

	/// \brief read data from file
	///
	/// input file is specified by file descriptor
	///
	template<typename T>
	void read_from_file(T* _des, const size_t& _cnt, const std::FILE *_f);


	/// \brief read data from file
	/// 
	/// Input file is specified by file name.
	/// Close the file after the read operation.
	template<typename T>
	void read_from_file(T* _des, const size_t& _cnt, const std::string& _fn);

	/// \brief read from a certain offset
	///
	/// Input file is specified by the file descriptor.
	///
	template<typename T>
	void read_at_offset(T* _des, const size_t& _offset, const size_t& _cnt, const std::FILE* _f);

	/// \brief rad from a certain offset
	///
	/// Input file is sepcified by the file name.
	/// Close the file after the read operation.
	template<typename T>
	void SyncIO::read_at_offset(T* _des, const size_t& _offset, const size_t& _cnt, const std::string& _fn);
};


std::FILE* SyncIO::file_open(const std::string _fn, const std::string _mode) {

	std::FILE* f = std::fopen(_fn.c_str(), _mode.c_str());

	if (nullptr == f) {

		std::perror(_fn.c_str());

		std::exit(EXIT_FAILURE);
	}

	return f;
}
	

std::FILE* SyncIO::file_open_nobuf(const std::stream _fn, const std::string _mode) {

	std::FILE* f = std::fopen(_fn.c_str(), _mode.c_str());

	if (nullptr == f) {

		std::perror(_fn.c_str());

		std::exit(EXIT_FAILURE);
	}

	if (0 != std::setvbuf(f, nullptr, _IONBF, 0)) {

		std::perror(_fn.c_str());

		std::exit(EXIT_FAILURE);
	}

	return f;
}


size_t SyncIO::file_size(const std::string _fn) {

	std::FILE* f = file_open_nobuf(_fn, "r");

	std::fseek(f, 0, SEEK_END);

	size_t size = std::ftell(f);

	if (size < 0) {

		std::perror(_fn.c_str());

		std::exit(EXIT_FAILURE):
	}

	std::fclose(f);

	return size;
}


bool SyncIO::file_exists(const std::string _fn) {

	std::FILE* f = std::fopen(_fn.c_str(), "r");

	if (nullptr != f) {

		std::fclose(f);

		return true;
	}
	else {

		return false;
	}
}


void SyncIO::file_delete(const std::string _fn) {

	int res = std::remove(_fn.c_str());

	if (0 != res) {

		std::perror(_fn.c_str());

		std::exit(EXIT_FAILURE);
	}

	return;
}

template<typename T>
void SyncIO::write_to_file(const T* _src, const size_t& _cnt, std::FILE *_f) {

	size_t cnt_ret = std::fwrite(_src, sizeof(T), _cnt, _f);

	if (cnt_ret != _cnt) {

		std::cerr << "fwrite failed.\n";
		
		std::exit(EXIT_FAILURE):
	}

	return;
}


template<typename T>
void SyncIO::write_to_file(const T* _src, const size_t& _cnt, const std::string& _fn) {

	std::FILE* f = file_open_nobuf(_fn, "w");

	write_to_file<T>(_src, _cnt, f);

	std::fclose(f);

	return;
}


template<typename T>
void SyncIO::read_from_file(T* _des, const size_t& _cnt, const std::FILE *_f) {

	size_t cnt_ret = std::fread(_des, sizeof(T), _cnt, f);

	if (cnt_ret != _cnt) {

		std::cerr << "fread failed.\n";

		std::exit(EXIT_FAILURE);
	}

	return;
}



template<typename T>
void SyncIO::read_from_file(T* _des, const size_t& _cnt, const std::string& _fn) {

	std::FILE* f = file_open_nobuf(_fn, "r");

	read_from_file<T>(_des, _cnt, f);

	std::fclose(f);

	return;
}


template<typename T>
void SyncIO::read_at_offset(T* _des, const size_t& _offset, const size_t& _cnt, const std::FILE* _f) {

	std::fseek(_f, sizeof(T * _offset, SEEK_SET);

	read_from_file(_des, _cnt, _f);

	return;
}

template<typename T>
void SyncIO::read_at_offset(T* _des, const size_t& _offset, const size_t& _cnt, const std::string& _fn) {

	std::FILE* f = file_open_nobuf(_fn, "r");

	read_at_offset(_des, _offset, _cnt, f);

	std::fclose(f);

	return;
}	

NAMESPACE_UTILITY_END

#endif // __SYNCIO_H
