///////////////////////////////////////////////////////////////////////////////
/// Copyright (c) 2016, Sun Yat-sen University,
/// All rights reserved
/// \file basicio.h
/// \brief provide basic I/O interface for sequential read/write operations
///
/// 
/// \author Yi Wu
/// \date 2016.12
///////////////////////////////////////////////////////////////////////////////

#ifndef __BASICIO_H
#define __BASICIO_H

NAMESPACE_UTILITY_BEG

/// \brief synchronous read operations
class BasicIO {
private:

	/// \brief open file in specific mode
	///
	/// \param _fn filename
	/// \param _mode mode
	static std::FILE* file_open(const std::string _fn, const std::string _mode);

	/// \brief open file in specific mode with nobuf option
	///
	/// \param _fn filename
	/// \param _mode mode
	static std::FILE* file_open_nobuf(const std::stream _fn, const std::string _mode);

	/// \brief return file size in bytes
	///
	/// \param _fn filename
	static uint64 file_size(const std::string _fn);

	/// \brief check if exists
	///
	/// \param _fn filename
	static bool file_exists(const std::string _fn);

	/// \brief delete file
	///
	/// \param _fn filename
	static void file_delete(const std::string _fn);

	/// \brief put data into a disk file specified by the descriptor
	///
	/// \param _src source
	/// \param _cnt number of elements
	/// \param _f descriptor
	template<typename value_type>
	void write_to_file(const value_type* _src, const uint64& _cnt, std::FILE *_f);

	/// \brief put data into a disk file specified by the name, close the file after the operation.
	///
	/// \param _src source
	/// \param _cnt number of elements
	/// \param _fn filename
	template<typename value_type>
	void write_to_file(const value_type* _src, const uint64& _cnt, const std::string& _fn);

	/// \brief get data from a disk file specified by the descriptor
	///
	/// \param _des destination
	/// \param _cnt number of elements
	/// \param _f descriptor
	template<typename value_type>
	void read_from_file(value_type* _des, const uint64& _cnt, const std::FILE *_f);


	/// \brief read data from a disk file specified by the name, close the file after the operation.
	///
	/// \param _des destination
	/// \param _cnt number of elements
	/// \param _fn filename
	template<typename value_type>
	void read_from_file(value_type* _des, const uint64& _cnt, const std::string& _fn);

	/// \brief read data from a disk file, beginning at the specified offset
	///
	/// \param _des destination
	/// \param _offset offset
	/// \param _cnt number of elements
	/// \param _f descriptor
	template<typename value_type>
	void read_at_offset(value_type* _des, const uint64& _offset, const uint64& _cnt, const std::FILE* _f);

	/// \brief read data from a disk file, beginning at the specified offset
	///
	/// \param _des destination
	/// \param _offset offset
	/// \param _cnt number of elements
	/// \param _fn filename
	template<typename value_type>
	void read_at_offset(value_type* _des, const uint64& _offset, const uint64& _cnt, const std::string& _fn);
};


std::FILE* BasicIO::file_open(const std::string _fn, const std::string _mode) {

	std::FILE* f = std::fopen(_fn.c_str(), _mode.c_str());

	if (nullptr == f) {

		std::perror(_fn.c_str());

		std::exit(EXIT_FAILURE);
	}

	return f;
}
	

std::FILE* BasicIO::file_open_nobuf(const std::stream _fn, const std::string _mode) {

	std::FILE* f = std::fopen(_fn.c_str(), _mode.c_str());

	if (nullptr == f) {

		std::perror(_fn.c_str());

		std::exit(EXIT_FAILURE);
	}

	// no buffer
	if (0 != std::setvbuf(f, nullptr, _IONBF, 0)) {

		std::perror(_fn.c_str());

		std::exit(EXIT_FAILURE);
	}

	return f;
}


uint64 BasicIO::file_size(const std::string _fn) {

	std::FILE* f = file_open_nobuf(_fn, "r");

	std::fseek(f, 0, SEEK_END);

	uint64 size = std::ftell(f);

	if (size < 0) {

		std::perror(_fn.c_str());

		std::exit(EXIT_FAILURE):
	}

	std::fclose(f);

	return size;
}


bool BasicIO::file_exists(const std::string _fn) {

	std::FILE* f = std::fopen(_fn.c_str(), "r");

	if (nullptr != f) {

		std::fclose(f);

		return true;
	}
	else {

		return false;
	}
}


void BasicIO::file_delete(const std::string _fn) {

	int res = std::remove(_fn.c_str());

	if (0 != res) {

		std::perror(_fn.c_str());

		std::exit(EXIT_FAILURE);
	}

	return;
}

template<typename value_type>
void BasicIO::write_to_file(const value_type* _src, const uint64& _cnt, std::FILE *_f) {

	uint64 cnt_ret = std::fwrite(_src, sizeof(value_type), _cnt, _f);

	if (cnt_ret != _cnt) {

		std::cerr << "fwrite failed.\n";
		
		std::exit(EXIT_FAILURE):
	}

	return;
}


template<typename value_type>
void BasicIO::write_to_file(const value_type* _src, const uint64& _cnt, const std::string& _fn) {

	std::FILE* f = file_open_nobuf(_fn, "w");

	write_to_file<value_type>(_src, _cnt, f);

	std::fclose(f);

	return;
}


template<typename value_type>
void BasicIO::read_from_file(value_type* _des, const uint64& _cnt, const std::FILE *_f) {

	uint64 cnt_ret = std::fread(_des, sizeof(value_type), _cnt, _f);

	if (cnt_ret != _cnt) {

		std::cerr << "fread failed.\n";

		std::exit(EXIT_FAILURE);
	}

	return;
}



template<typename value_type>
void BasicIO::read_from_file(value_type* _des, const uint64& _cnt, const std::string& _fn) {

	std::FILE* f = file_open_nobuf(_fn, "r");

	read_from_file<value_type>(_des, _cnt, f);

	std::fclose(f);

	return;
}


template<typename value_type>
void BasicIO::read_at_offset(value_type* _des, const uint64& _offset, const uint64& _cnt, const std::FILE* _f) {

	std::fseek(_f, sizeof(value_type) * _offset, SEEK_SET);

	read_from_file(_des, _cnt, _f);

	return;
}

template<typename value_type>
void BasicIO::read_at_offset(value_type* _des, const uint64& _offset, const uint64& _cnt, const std::string& _fn) {

	std::FILE* f = file_open_nobuf(_fn, "r");

	read_at_offset(_des, _offset, _cnt, f);

	std::fclose(f);

	return;
}	

NAMESPACE_UTILITY_END

#endif // __SYNCIO_H
