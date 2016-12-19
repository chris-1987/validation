////////////////////////////////////////////////////////////
/// Copyright (c) 2016, Sun Yat-sen University,
/// All rights reserved
/// \file validate2.h
/// \brief Using induced-sorting method and Karp-Rabin fingerprinting function to 
/// validate suffix and LCP arrays
///
/// \author Yi Wu
/// \date 2016.12
///////////////////////////////////////////////////////////


#ifndef VALIDATE2_H
#define VALIDATE2_H

#include "common/common.h"

/// \brief A suffix and LCP array validater
///
/// \note only support O(1) alphabet 
template<typename alphabet_type, typename size_type, typename offset_type> 
class Validator2{
	
private:

	typedef typename ExVector<size_type>::vector size_vector_type;

	typedef typename ExVector<alphabet_type>::vector alphabet_vector_type;

	typedef std::pair<size_type, size_type> pair_type;

	typedef PairLess1st<pair_type> pair_comparator_type;

	typedef typename ExTupleAscSorter<pair_type, pair_comparator_type>::sorter pair_sorter_type;

	typedef std::pair<size_type, fpa_type> pair_type2;

	typedef PairLess1st<pair_type> pair_comparator_type2;

	typedef typename ExTupleAscSorter<pair_type2, Pair_comparator_type2>::sorter pair_sorter_type2;

	typedef typename ExVector<fpa_type>::vector fpa_vector_type;

private:
	
	std::string m_t_fn; ///< t file name

	std::string m_sa_fn; ///< sa file name

	std::string m_lcp_fn; ///< lcp file name

	uint64 m_len; ///< number of elements in t/sa/lcp
	
	uint64 m_lms_num; ///< number of LMS 

	size_vector_type* m_sa_lms;

	size_vector_type* m_lcp_lms;
	
public:

	/// \brief constructor
	Validator2(const std::string& _t_fn, const std::string& _sa_fn, const std::string& _lcp_fn) {

		m_t_fn = _t_fn;

		m_sa_fn = _sa_fn;

		m_lcp_fn = _lcp_fn;

		m_len = BasicIO::file_size(m_t_fn) / sizeof(alphabet_type);

		///use karp rabin method to validate suffix and LCP arrays for LMS suffixes
		checkLMS();		

	}

	/// \brief run checking process
	void run() {

		/// step 1: use karp-rabin method to validate lms_sa and lms_lcp
		checkLMS();
	}


	/// \brief check lms_sa and lms_lcp
	///
	/// use karp-rabin finger-print function to validate lms_sa and lms_lcp
	/// Given the input string of n characters, we check lms_sa and lms_lcp by two different methods in the following two cases:
	/// case (1) if n / (MEM_AVAIL / sizeof(size_type) /3) * 1 + sizeof(size_type * 2 >  
	void check_lms() {

		// step 1
		retrieve_lms();

		
		// step 2
		if (true) { // use block-parition method in RAM


			check_lms_method1();
		}
		else { // use external sorting method

			check_lms_method2();
		}
	}

	///
	void retrieve_lms() {

		typedef std::pair<size_type, size_type> pair_type;

		typedef TupleGreatCompartor1st<pair_type> pair_great_comparator_type;

		typedef ExTupleDescSorter<pair_type, pair_great_comparator_type> pair_great_sorter_type;

		stxxl::syscall_file* sa_file = new stxxl::syscall_file(m_sa_fn, stxxl::syscall_file::RDWR | stxxl::syscall_file::DIRECT);
			
		size_vector_type* sa = new size_vector_type(sa_file);
			
		typename size_vector_type::bufreader_type* sa_reader = new typename size_vector_type::bufreader_type(*sa);

		pair_great_sorter_type *pair_great_sorter = new pair_great_sorter_type(pair_great_comparator_type(), MEM_AVAIL / 2);

		for (uint64 i = 1; sa_reader->empty(); ++(*sa_reader), ++i) { // start couting from 1

			pair_great_sorter->push(pair_type(*(*sa_reader), i));
		}

		pair_great_sorter->sort();

		// 
		typedef TupleLessComparator2nd<pair_type> pair_less_comparator_type;

		typedef ExTupleAscSorter<pair_type, pair_less_comparator_type> pair_less_sorter_type;

		stxxl::syscall_file* t_file = new stxxl::syscall_file(m_t_fn, stxxl::syscall_file::RDWR | stxxl::syscall_file::DIRECT);

		alphabet_vector_type* t = new alphabet_vector_type(t_file);

		typename alphabet_vector_type::bufreader_reverse_type* t_reverse_reader = new typename alphabet_vector_type::bufreader_reverse_type(*t);

		pair_less_sorter_type* pair_less_sorter = new pair_less_sorter_type(pair_less_comparator_type(), MAX_AVAIL / 2);

		//
		m_lms_num = 0;
	
		alphabet_type pre_ch = *(*t_reverse_reader);

		uint8 pre_ch_type = L_TYPE, cur_ch_type;

		++(*t_reverse_reader);

		for (; t_reverse->reader->empty(); ++(*t_reverse->reader), ++(*pair_great_sorter)) {

			
			const alphabet_type& cur_ch = *(*t_reverse_reader);

			cur_ch_type = (cur_ch < pre_ch || (cur_ch == pre_ch && pre_ch_type == S_TYPE) ? S_TYPE : L_TYPE;

			if (cur_ch_type == L_TYPE && pre_ch_type == S_TYPE) {
				
				pair_less_sorter->push(pair_type(*(*pair_great_sorter));				
				
				++m_lms_num;
			}

			pre_ch = cur_ch;

			pre_ch_type = cur_ch_type;
		}

		delete t_reverse_reader; t_reverse_reader = nullptr;

		delete t; t = nullptr;

		delete t_file; t_file = nullptr;

		delete pair_great_sorter; pair_great_sorter = nullptr;

		pair_less_sorter->sort();

		stxxl::syscall_file* lcp_file = new stxxl::syscall_file(m_lcp_fn, stxxl::syscall_file::RDWR | stxxl::syscall_file::DIRECT);
		
		size_vector_type* lcp = new size_vector_type(lcp_file);

		typename size_vector_type::bufreader_revers_type* lcp_reader = new typename size_vector_type::bufreader_rerversetype(*lcp);

		m_sa_lms = new size_vector_type(); m_sa_lms->resize(m_lms_num);

		typename size_vector_type::bufwriter_type* sa_lms_writer(*m_sa_lms);

		m_lcp_lms = new size_vector_type(); m_lcp_lms->resize(m_lms_num);
	
		typename size_vector_type::bufwriter_type* lcp_lms_writer(*m_lcp_lms);

		size_type lcp_range_min = std::numeric:limits<size_type>::max();

		size_type pos = 1;

		while (!pair_less_sorter->empty()) {

			while((*pair_less_sorter)->second != pos) {

				++(*lcp_reader);
				
				++pos;
			}

			lcp_range_min = std::min(*(*lcp_reader), lcp_range_min);

			(*sa_lms_writer) << (*pair_less_sorter)->first;	

			(*lcp_lms_writer) << lcp_range_min;

			lcp_range_min = std::numeric_limits<size_type>::max();

			++(*pair_less_comparator);
		}

		(*sa_lms_writer).finish();

		(*lcp_lms_writer).finis();

		delete sa_lms_writer; sa_lms_writer = nullptr;

		delete lcp_lms_writer; lcp_lms_writer = nullptr;

		delete pair_less_comparator; pair_less_comparator = nullptr; 

		delete lcp_reader; lcp_reader = nullptr;

		delete lcp;

		delete lcp_file;
	}


	/// \brief check sa_lms and lcp_lms using block-paritioning method
	void check_lms_method1() {

		typedef std::pair<size_type, offset_type> pair_type;

		uint64 block_capacity = MAIN_MEM_AVAIL / sizeof(size_type) / 3;
		
		uint64 block_num = m_lms_num / block_capacity + (m_lms_num % block_capacity) ? 1 : 0;

		pair_type* pair1_block = new pair_type[block_capacity];

		pair_type* pair2_block = new pair_type[block_capacity];

		pair_type* pair3_block = new pair_type[block_capacity];

		uint64 items_toread = m_lms_num;

		uint64 items_read = 0;

		uint64 block_id = 0;

		while (items_toread > 0) {

			stxxl::timer Timer2;

			Timer2.start();

			uint64 item_num = (items_toread >= block_capacity) ? block_capacity : items_toread;

			typename size_vector_type::const_iterator sa_lms_beg = m_sa_lms->begin() + items_read;

			typename size_vector_type::const_iterator sa_lms_end = sa_lms_beg + item_num;

			typename size_vector_type::bufreader_type* sa_lms_reader = new typename size_vector_type::bufreader_type(sa_beg, sa_end, 2ul);

			typename size_vector_type::const_iterator lcp_beg = m_lcp_lms->begin() + items_read;

			typename size_vector_type::const_iterator lcp_end = lcp_beg + item_num;

			typename size_vector_type::bufreader_type* lcp_lms_reader = new typename size_vector_type::bufreader_type(lcp_beg, lcp_end, 2ul);

			for (uint32 j = 0; j < item_num - ((items_toread == item_num) ? 1 : 0); ++j) {

				pair1_block[j].first = *(*sa_lms_reader);

				pair1_block[j].second = j;

				pair2_block[j].first = *(*sa_lms_reader) + *(*lcp_lms_reader);

				pair2_block[j].second = j;

				++(*lcp_lms_reader);

				pair3_block[j].first = *(*sa_lms_reader) + *(*lcp_lms_reader);
			
				pair3_block[j].second = j;

				++(*sa_lms_reader);
			}
			
			// special case: in the rightmost block, lcp_lms[item_num] doesn't exist
			if (items_toread == item_num) {

				pair1_block[item_num - 1].first = *(*sa_lms_reader);

				pair1_block[item_num - 1].second = item_num - 1;

				pair2_block[item_num - 1].first = *(*sa_lms_reader) + *(*lcp_lms_reader);

				pair2_block[item_num - 1].second = item_num - 1;
			}
		
			++(*lcp_lms_reader);

			assert(sa_lms_reader->empty() == true);

			assert(lcp_lms_reader->empty() == true);

			delete sa_lms_reader; 

			delete lcp_lms_reader;
		
			bool is_right = check_block(pair1_block, pair2_block, pair3_block, item_num, block_id, block_capacity, items_toread == item_num);

			items_toread -= item_num;

			items_read += item_num;

			block_id++;
		}

		return true;
	}

	bool check_block(std::pair<size_type, offset_type>* _pair1_block, std::pair<size_type, offset_type>* _pair2_block, std::pair<size_type, offset_type>* _pair3_block, const uint64& _item_num, const uint64& _block_id, const uint64& _block_capacity, const bool _is_rightmost) {

		typedef std::pair<size_type, offset_type> pair_type;

		std::sort(_pair1_block, _pair1_block + _item_num, PairLess1st<std::pair<size_type, offset_type>>());

		std::sort(_pair2_block, _pair2_block + _item_num, PairLess1st<std::pair<size_type, offset_type>>());

		std::sort(_pair3_block, _pair3_block + _item_num - (_is_rightmost ? 1 : 0), PairLess1st<std::pair<size_type, offset_type>>());

		stxxl::syscall_file t_file(m_t_fn, stxxl::syscall_file::RDWR | stxxl::syscall_file::DIRECT);

		alphabet_vector_type t(&t_file);

		typename alphabet_vector_type::bufreader_type t_reader(t);

		//
		fpa_type fp = 0;
		
		uint64 j1 = 0, j2 = 0, j3 = 0;

		for (uint64 i = 0; i < m_lms_num; ++i) {

			while (j1 < m_lms_num && i == _pair1_block[j1].first) {

				_pair1_block[j1].first.set(fp, 0);

				++j1;
			}

			while (j2 < m_lms_num && i == _pair2_block[j2].first) {

				_pair2_block[j2].first.set(fp, 0);

				++j2;
			}

			while (j3 < m_lms_num && i == _pair3_block[j3].first) {

				_pair3_block[j3].first.set(fp, 0);

				++j3;
			}

			fp = static_cast<fpa_type>((static_cast<fpb_type>(fp) * R + (*t_reader + 1)) % P);

			++t_reader;
		}

		// special case
		while (j1 < m_lms_num) {

			_pair1_block[j1].first.set(fp, 0);

			++j1;
		}

		while (j2 < m_lms_num) {

			_pair2_block[j2].first.set(fp, std::numeric_limits<typename size_type::high_type>::max());

			++j1;
		}

		while (j3 < m_lms_num - (_is_rightmost ? 1 : 0)) {

			_pair3_block[j3].first.set(fp, std::numeric_limits<typename size_type::high_type>::max());
		}
		
		//
		assert(t_reader.empty() == true);

		std::sort(_pair1_block, _pair1_block + _item_num, PairLess2nd<pair_type>());

		std::sort(_pair2_block, _pair2_block + _item_num, PairLess2nd<pair_type>());

		std::sort(_pair3_block, _pair3_block + _item_num - (_is_rightmost ? 1 : 0), PairLess2nd<pair_type>());

		// check result
		alphabet_type pre_ch;

		fpa_type fp_interval, pre_fp_interval;

		//
		typename size_vector_type::const_iterator lcp_lms_beg = lcp_lms->begin();

		typename size_vector_type::const_iterator lcp_lms_end = lcp_lms_beg + _item_num + (_is_rightmost ? 0 : 1);

		typename size_vector_type::bufreader_type lcp_lms_reader(lcp_beg, lcp_end, 2ul);

		// check first pair 
		if (0 == _block_id) {

			pre_ch = _pair3_block[0].first.get_high();

			++lcp_lms_reader;

			pre_fp_interval = static_cast<fpa_type>(((static_cast<fpb_type>(_pair3_block[0].first.get_low()) - static_cast<fpb_type>(_pair1_block[0].first.get_low()) * m_rinterval->compute(*lcp_lms_reader) % P) + P) % P);
		}
		else {

			pre_ch = m_pre_ch;

			pre_fp_interval = m_pre_fp_interval;
		}

		// if leftmost block, skip checking leftmost lcp
		for (uint64 i = ((_block_id == 0) ? 1 : 0); i < _item_num - (_is_rightmost ? 1 : 0); ++i) {

			if (pre_ch == pair2_block[i].first.get_high()) {

				return false;
			}

			fp_interval = static_cast<fpa_type>(((static_cast<fpb_type>(pair2_block[i].first.get_low()) - static_cast<fpb_type>(_pair1_block[i].first.get_low()) * m_rinterval->compute(*lcp_lms_reader) % P) + P) % P);

			if (fp_interval != pre_fp_interval) {

				return false;
			}

			pre_ch = _pair3_block[i].first.get_high();

			++lcp_reader;

			pre_fp_interval = static_cast<fpa_type>(((static_cast<fpb_type>(_pair3_block[i].first.get_low()) - static_cast<fpb_type>(_pair1_block[i].first.get_low()) * m_rinterval->compute(*lcp_lms_reader) % P) + P) % P);
		}

		if (_is_rightmost) {

			if (pre_ch == _pair2_block[_item_num - 1].first.get_high()) return false;


			fp_interval = static_cast<fpa_type>(((static_cast<fpb_type>(_pair2_block[_item_num - 1].first.get_low()) - static_cast<fpb_type>(_pair1_block[_item_num - 1].first.get_low()) * m_rinterval->compute(*lcp_lms_reader) % P) + P) % P);

			if (fp_interval != pre_fp_interval) return false;

			++lcp_reader;
		}
		else {

			m_pre_ch = pre_ch;

			m_pre_fp_interval = pre_fp_interval;
		}

		return true;
	}


	/// \brief check sa_lms and lcp_lms using external memory sorting method
	void check_lms_method2() {

		// step 1: compute fp[0, sa_lms[i] - 1] 
		pair_sorter_type* pair_sorter1 = new pair_sorter_type(pair_comparator_type, MEM_AVAIL / 4);		

		{
			typename size_type_vector::bufreader_type* sa_lms_reader = new typename size_vector_type::bufreader_type(*m_sa_lms);
	
			for (T i = 0; !sa_lms_reader->empty(); ++(*sa_lms_reader), ++i) {// T[0] must not be lms, thus let i start from 0
	
				pair_sorter1->push(pair_type(*(*sa_lms_reader)), i);
			}
	
			assert(sa_lms_reader->empty() == true);

			delete sa_lms_reader; sa_lms_reader = nullptr;
	
			pair_sorter1->sort();
		}

		pair_sorter_type2* pair_sorter2 = new pair_sorter_type2(pair_comparator_type2(), MEM_AVAIL / 4); 
		
		compute_fp(pair_sorter1, pair_sorter2); 

		delete pair_sorter1; pair_sorter1 = nullptr;


		// step 2: compute fp[0, sa_lms[i] + lcp_lms[i] - 1]
		pair_sorter_type* pair_sorter3 = new pair_sorter_type(pair_comparator_type, MEM_AVAIL / 4);

		{
			typename size_type_vector::bufreader_type* sa_lms_reader = new typename size_vector_type::bufreader_type(*m_sa_lms);

			typename size_type_vector::bufreader_type* lcp_lms_reader = new typename size_vector_type::bufreader_type(*m_lcp_lms);
	
			for (T i = 0; ! sa_lms_reader->empty(); ++(*sa_lms_reader), ++(*lcp_lms_reader), ++i) {
	
				pair_sorter3->push(pair_type(*(*sa_lms_reader) + *(*lcp_lms_reader)), i);
			}

			assert(lcp_lms_reader->empty() == true);

			assert(sa_lms_reader->empty() == true);

			delete sa_lms_reader; sa_lms_reader = nullptr;

			delete lcp_lms_reader; lcp_lms_reader = nullptr;

			pair_sorter3->sort();
		}
		
		pair_sorter_type2* pair_sorter4 = new pair_sorter_type2(pair_comparator_type2(), MEM_AVAIL / 4);
	
		compute_fp_ch(pair_sorter3, pair_sorter4);

		delete pair_sorter3; pair_sorter3 = nullptr;

		// step 4: compute fp[0, sa[i] + lcp[i + 1] - 1]
		pair_sorter_type* pair_sorter5 = new pair_sorter_type(pair_comparator_type, MEM_AVAIL / 4);

		{
			typename size_type_vector::bufreader_type* sa_lms_reader = new typename size_vector_type::bufreader_type(*m_sa_lms);

			typename size_type_vector::bufreader_type* lcp_lms_reader = new typename size_vector_type::bufreader_type(*m_lcp_lms);
	
			++(*lcp_lms_reader); // forward one element
		
			for (T i = 0; !lcp_lms_reader->empty(); ++(*sa_lms_reader), ++(*lcp_lms_reader), ++i) {
	
				pair_sorter5->push(pair_type(*(*sa_lms_reader) + *(*lcp_lms_reader)), i);
			}
			
			++(*sa_lms_reader);

			assert(lcp_lms_reader->empty() == true);

			assert(sa_lms_reader->empty() == true);

			delete sa_lms_reader; sa_lms_reader = nullptr;

			delete lcp_lms_reader; lcp_lms_reader = nullptr;

			pair_sorter5->sort();
		}
		
		pair_sorter_type2* pair_sorter6 = new pair_sorter_type(pair_comparator_type(), MEM_AVAIL / 4);

		compute_fp_ch(pair_sorter5, pair_sorter6);

		delete pair_sorter5; pair_sorter5 = nullptr;
	
		//
		check_fp_ch(pair_sorter2, pair_sorter4, pair_sorter6);

		delete pair_sorter2; pair_sorter2 = nullptr;

		delete pair_sorter4; pair_sorter4 = nullptr;

		delete pair_sorter6; pair_sorter6 = nullptr;

		return;
	}


	/// \brief 
	void compute_fp(pair_sorter_type* _pair_sorter1, pair_sorter_type2* _pair_sorter2){

		// scan input T rightward, iteratively compute fp[0, i] and retrieve required fingerprints
		stxxl::syscall_file* t_file = new stxxl::syscall_file(m_t_fn, stxxl::syscall_file::RDWR | stxxl::syscall_file::DIRECT);

		alphabet_vector_type* t = new alphabet_vector_type(t_file);

		typename alphabet_vector_type::bufreader_type* t_reader = new typename alphabet_vector_type::bufreader_type(*t);

		fpa_type fp = 0;


		for (T pos = 0; !t_reader.empty(); ++(*t_reader), ++pos) {

			while ((*_pair_sorter1)->first == pos) {

				_pair_sorter2->push(pair_type2(fp, (*_pair_sorter)->second));
			}		

			fp = static_cast<fpa_type>((static_cast<fpa_typb>(fp) * R + (*(*t_reader) + 1)) % P);
		}

		// t[m_len - 1] is L_TYPE (right before the sentinel), thus not an LMS as well.

		delete t_reader; t_reader = nullptr;

		delete t; t = nullptr;

		delete t_file; t_file = nullptr;

		_pair2_sorter2->sort();

		retuurn;
	} 

	/// \brief 
	void compute_fp_ch(pair_sorter_type* _pair_sorter1, pair_sorter_type* _pair_sorter2){

		// scan input T rightward, iteratively compute fp[0, i] and retrieve required fingerprints
		stxxl::syscall_file* t_file = new stxxl::syscall_file(m_t_fn, stxxl::syscall_file::RDWR | stxxl::syscall_file::DIRECT);

		alphabet_vector_type* t = new alphabet_vector_type(t_file);

		typename alphabet_vector_type::bufreader_type* t_reader = new typename alphabet_vector_type::bufreader_type(*t);

		fpa_type fp = 0;

		for (T pos = 0; !t_reader.empty(); ++(*t_reader), ++pos) {

			while ((*_pair_sorter1)->first == pos) {

				_pair_sorter2->push(pair_type2(size_type(fp, *t_reader), (*_pair_sorter)->second));
			}		

			fp = static_cast<fpa_type>((static_cast<fpa_typb>(fp) * R + (*(*t_reader) + 1)) % P);
		}

		// sa[i] + lcp[i] = m_len or sa[i] + lcp[i + 1] = m_len
		while (!_pair_sorter1->empty()) {

			_pair_sorter2->push(pair_type2(size_type(fp, std::numeric_limits<alphabet_type>::max(), (*_pair_sorter)->second));
		}

		delete t_reader; t_reader = nullptr;

		delete t; t = nullptr;

		delete t_file; t_file = nullptr;

		_pair2_sorter2->sort();

		return;
	} 	

	// 
	void diff_fp_ch(pair_sorter_type2* _pair_sorter1, pair_sorter_type* _pair_sorter2, pair_sorter_type* _pair_sorter3){

		typename size_vector_type::bufreader_type *lms_reader = new typename size_vector_type(*m_lcp_lms);

		fpa_type fp_interval1, fp_interval2;

		uint8 ch1, ch2;

		size_type cur_lcp;

		++(*_pair_sorter2); // skip sa[0] + lcp[0]

		++(*lcp_lms_reader); // skip lcp[0]

		cur_lcp = *(*lcp_lms_reader);


		fp_interval2 = static_cast<fpa_type>(((*_pair_sorter3)->first.get_low() - (static_cast<fpb_type>((*_pair_sorter1)->first) * m_rinterval->compute(cur_lcp)) % P + P) % P);

		ch2 = (*_pair_sorter3)->first.get_low();

		++(*_pair_sorter1);

		++(*_pair_sorter3);

		for (size_t idx = 1; !_pair_sorter1->empty(); ++idx, ++(_pair_sorter), ++(_pair2_sorter), ++(_pair3_sorter)) {


			fpInterval1 = static_cast<fpa_type>(((*_pair_sorter2)->first - 
		}

			 
	}

	// 
	void check_fp_ch(fpa_vector_type* _fp_interval_vec, alphabet_vector_type* _ch_vec, fpa_vector_type* _fp_interval_vec2, alphabet_vector_type* _ch_vec2) {

		fpa_vector_type::bufreader_type fp_reader1(*_fp_interval_vec);

		alphabet_vector_type::bufreader_type ch_reader1(*_ch_vec);

		fpa_vector_type::bufreader_type fp_reader2(*_fp_interval_vec2);

		alphabet_vector_type::bufreader_type ch_reader2(*_ch_vec2);

				
		
	}

};


#endif // VALIDATE2_H
