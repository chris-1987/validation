////////////////////////////////////////////////////////////
/// Copyright (c) 2016, Sun Yat-sen University,
/// All rights reserved
/// \file validate3.h
/// \brief validate sa and lcp using external memory sorts and Karp-rabin finger-print function.
///  
/// \note The alphaber size of the input string is assumed to be O(1) or O(n).
/// \note This is the first method reported in xxx.
///
/// \author Yi Wu
/// \date 2016.12
///////////////////////////////////////////////////////////


#ifndef VALIDATE3_H

#define VALIDATE3_H

#include "common/common.h"

#include "common/tuples.h"

#include "common/widget.h"

#include "common/basicio.h"

#define TEST_VALIDATE3

/// \brief validate sa and lcp using Karp-Rabin fingerprinting function
///
/// type of elements in the input string and suffix/LCP array are specified by alphabet_type and size_type, respectively.
template<typename alphabet_type, typename size_type>
class Validate3{

private:

	/// \brief store (R % P)^1, (R % P)^2, (R % P)^4, ...
	struct RInterval{
	
		fpa_type* m_data;
	
		uint64 m_num;
	
		/// \brief constructor
		RInterval(uint64 _len) {
	
			m_num = 1;
	
			while(_len) {
	
				++m_num;
	
				_len /= 2;
			}
	
			m_data = new fpa_type[m_num];
	
			m_data[0] =  R % P;
	
			for(uint64 i = 1; i < m_num; ++i) {
	
				m_data[i] = static_cast<fpa_type>((static_cast<fpb_type>(m_data[i - 1]) * m_data[i - 1]) % P);
			}
		
			return;
		}

		/// \brief get fpInterval
		fpa_type compute(uint64 _interval) {

			fpa_type ret = 1;
				
			for (uint64 i = 0; i < m_num; ++i) {
	
				if (_interval % 2) {
	
					ret = static_cast<fpa_type>((static_cast<fpb_type>(ret) * m_data[i]) % P);
				}
	
				_interval = _interval / 2;
	
				if (_interval == 0) break;
			}
	
			return ret;
		}	
	};

private:

	std::string m_t_fn; ///< filename for intput string

	std::string m_sa_fn; ///< filename for suffix array	

	std::string m_lcp_fn; ///< filename for LCP array

	uint64 m_len; ////< length of input string/SA/LCP

	RInterval* m_rinterval; ///< pointer to RInterval object

private:

	// alias
	//
	typedef typename ExVector<alphabet_type>::vector alphabet_vector_type;

	typedef typename ExVector<size_type>::vector size_vector_type;	

	// sort by 1st component
	typedef pair<size_type, size_type> pair1_type;

	typedef tuple_less_comparator_1st<pair1_type> pair1_comparator_type;

	typedef typename ExTupleSorter<pair1_type, pair1_comparator_type>::sorter pair1_sorter_type;

	// sort by 1st component
	typedef pair<size_type, fpa_type> pair2_type;

	typedef tuple_less_comparator_1st<pair2_type> pair2_comparator_type;

	typedef typename ExTupleSorter<pair2_type, pair2_comparator_type>::sorter pair2_sorter_type;

	// sort by 1st component
	typedef triple<size_type, fpa_type, uint16> triple_type;

	typedef tuple_less_comparator_1st<triple_type> triple_comparator_type;

	typedef typename ExTupleSorter<triple_type, triple_comparator_type>::sorter triple_sorter_type;

public:

	/// \brief constructor
	///
	/// \param _t_fn  input string
	/// \param _sa_fn suffix array
	/// \param _lcp_fn lcp array
	Validate3(const std::string& _t_fn, const std::string& _sa_fn, const std::string& _lcp_fn) {

		m_t_fn = _t_fn;

		m_sa_fn = _sa_fn;

		m_lcp_fn = _lcp_fn;

		m_len = BasicIO::file_size(_t_fn) / sizeof(alphabet_type);

		m_rinterval = new RInterval(m_len);
	}


	/// \brief core part of the program
	///
	bool run() {

		//
		stxxl::stats *Stats = stxxl::stats::get_instance();

		stxxl::stats_data Stats_begin(*Stats);

		stxxl::block_manager *bm = stxxl::block_manager::get_instance();

		stxxl::timer Timer;

		Timer.start();

		// step 1: fetch fp[0, sa[i] - 1]	
		pair2_sorter_type* sorter1 = fetch_fp();

		// step 2: fetch fp[0, sa[i] +lcp[i] - 1]
		triple_sorter_type* sorter2 = fetch_fp_ch_cur();

		// step 3: fetch fp[0, sa[i - 1] + lcp[i] - 1]
		triple_sorter_type* sorter3 = fetch_fp_ch_pre();

		// step 4: check the result
		bool res = check(sorter1, sorter2, sorter3);

		delete sorter1; sorter1 = nullptr;

		delete sorter2; sorter2 = nullptr;

		delete sorter3; sorter3 = nullptr;


		//
		Timer.stop();
		
		std::cerr << "elapsed time: " << Timer.seconds() << " seconds " << Timer.mseconds() << "mseconds.\n";
	
		std::cerr << (stxxl::stats_data(*Stats) - Stats_begin);

		std::cerr << "MAIN_MEM_AVAIL: " << MAIN_MEM_AVAIL << std::endl;

		std::cerr << "total IO volume: " << Stats->get_written_volume() + Stats->get_read_volume() << std::endl;

		std::cerr << "IO volume per ch: " << (Stats->get_written_volume() + Stats->get_read_volume()) / m_len << std::endl;

		std::cerr << "peak disk use per ch: " << bm->get_maximum_allocation() / m_len << std::endl;
		

		
		return res;
	}

	/// \brief fetch fp[0, sa[i] - 1]
	///
	pair2_sorter_type* fetch_fp() {

		// sort <sa[i], i> by 1st component to produce isa
		pair1_sorter_type* pair1_sorter = new pair1_sorter_type(pair1_comparator_type(), MAIN_MEM_AVAIL / 4);

		stxxl::syscall_file* sa_file = new stxxl::syscall_file(m_sa_fn, stxxl::syscall_file::RDWR | stxxl::syscall_file::DIRECT);

		size_vector_type* sa = new size_vector_type(sa_file);

		typename size_vector_type::bufreader_type *sa_reader = new typename size_vector_type::bufreader_type(*sa);
		
		for (uint64 idx = 1; !sa_reader->empty();  ++(*sa_reader), ++idx) {

			pair1_sorter->push(pair1_type(*(*sa_reader), idx));		
		}

		delete sa_reader; sa_reader = nullptr;

		delete sa; sa = nullptr;

		delete sa_file; sa_file = nullptr;

		pair1_sorter->sort();
		
		// scan t iteratively to compute fingerprints
		pair2_sorter_type* pair2_sorter = new pair2_sorter_type(pair2_comparator_type(), MAIN_MEM_AVAIL / 4);

		stxxl::syscall_file* t_file = new stxxl::syscall_file(m_t_fn, stxxl::syscall_file::RDWR | stxxl::syscall_file::DIRECT);

		alphabet_vector_type* t = new alphabet_vector_type(t_file);

		typename alphabet_vector_type::bufreader_type *t_reader = new typename alphabet_vector_type::bufreader_type(*t);

		fpa_type fp = 0;

		for (; !t_reader->empty(); ++(*t_reader), ++(*pair1_sorter)) {

			const pair1_type& tuple = *(*pair1_sorter);
		
			pair2_sorter->push(pair2_type(tuple.second, fp));

			fp = static_cast<fpa_type>((static_cast<fpb_type>(fp) * R + (*(*t_reader) + 1)) % P); // plus 1 to avoid equal to 0
		}

		delete t_reader; t_reader = nullptr;

		delete t; t = nullptr;

		delete t_file; t_file = nullptr;

		delete pair1_sorter; pair1_sorter = nullptr;

		pair2_sorter->sort();

		return pair2_sorter;	
	}

	/// \brief fetch fp[0, sa[i] + lcp[i] - 1] and t[sa[i] + lcp[i]]
	///
	triple_sorter_type* fetch_fp_ch_cur() {

		// sort (sa[i] + lcp[i], i) by i
		pair1_sorter_type* pair1_sorter = new pair1_sorter_type(pair1_comparator_type(), MAIN_MEM_AVAIL / 4);
		
		stxxl::syscall_file* sa_file = new stxxl::syscall_file(m_sa_fn, stxxl::syscall_file::DIRECT | stxxl::syscall_file::RDWR);

		size_vector_type* sa = new size_vector_type(sa_file);

		typename size_vector_type::bufreader_type* sa_reader = new typename size_vector_type::bufreader_type(*sa);

		stxxl::syscall_file* lcp_file = new stxxl::syscall_file(m_lcp_fn, stxxl::syscall_file::DIRECT | stxxl::syscall_file::RDWR);

		size_vector_type* lcp = new size_vector_type(lcp_file);

		typename size_vector_type::bufreader_type* lcp_reader = new typename size_vector_type::bufreader_type(*lcp);

		++(*sa_reader), ++(*lcp_reader);

		for (uint64 idx = 1; !sa_reader->empty(); ++(*sa_reader), ++(*lcp_reader), ++idx) {

			pair1_sorter->push(pair1_type(*(*sa_reader) + *(*lcp_reader), idx));
		}

		delete sa_reader; sa_reader = nullptr;

		delete sa; sa = nullptr;

		delete sa_file; sa_file = nullptr;

		delete lcp_reader; lcp_reader = nullptr;
		
		delete lcp; lcp = nullptr;

		delete lcp_file; lcp_file = nullptr;

		pair1_sorter->sort();

		// scan t iteratively to compute fingeprints in need
		triple_sorter_type* triple_sorter = new triple_sorter_type(triple_comparator_type(), MAIN_MEM_AVAIL / 4);

		stxxl::syscall_file* t_file = new stxxl::syscall_file(m_t_fn, stxxl::syscall_file::DIRECT | stxxl::syscall_file::RDWR);

		alphabet_vector_type* t = new alphabet_vector_type(t_file);

		typename alphabet_vector_type::bufreader_type* t_reader = new typename alphabet_vector_type::bufreader_type(*t);

		fpa_type fp = 0;

		uint16 ch;

		for (uint64 pos = 0; !pair1_sorter->empty(); ++(*pair1_sorter)) {

			const pair1_type& tuple = *(*pair1_sorter);

			while (pos != tuple.first) {

				fp = static_cast<fpa_type>((static_cast<fpb_type>(fp) * R + (*(*t_reader) + 1)) % P); // fp[0, tuple.first - 1]

				++(*t_reader);

				++pos;
			}

			ch = (m_len == pos) ? std::numeric_limits<uint16>::max() : (*(*t_reader));

			triple_sorter->push(triple_type(tuple.second, fp, ch));
		}

		delete t_reader; t_reader = nullptr;

		delete t; t = nullptr;

		delete t_file; t_file = nullptr;

		delete pair1_sorter; pair1_sorter = nullptr;

		triple_sorter->sort();

		return triple_sorter;
	}

	/// \brief fetch fp[0, sa[i - 1] + lcp[i] - 1] and t[sa[i - 1] + lcp[i]]
	///
	triple_sorter_type* fetch_fp_ch_pre() {

		// sort (sa[i - 1] + lcp[i], i) by i
		pair1_sorter_type* pair1_sorter = new pair1_sorter_type(pair1_comparator_type(), MAIN_MEM_AVAIL / 4);
		
		stxxl::syscall_file* sa_file = new stxxl::syscall_file(m_sa_fn, stxxl::syscall_file::DIRECT | stxxl::syscall_file::RDWR);

		size_vector_type* sa = new size_vector_type(sa_file);

		typename size_vector_type::bufreader_type* sa_reader = new typename size_vector_type::bufreader_type(*sa);

		stxxl::syscall_file* lcp_file = new stxxl::syscall_file(m_lcp_fn, stxxl::syscall_file::DIRECT | stxxl::syscall_file::RDWR);

		size_vector_type* lcp = new size_vector_type(lcp_file);

		typename size_vector_type::bufreader_type* lcp_reader = new typename size_vector_type::bufreader_type(*lcp);

		++(*lcp_reader); // skip the leftmost lcp

		for (uint64 idx = 1; !lcp_reader->empty(); ++(*sa_reader), ++(*lcp_reader), ++idx) {

			pair1_sorter->push(pair1_type(*(*sa_reader) + *(*lcp_reader), idx));
		}

		delete sa_reader; sa_reader = nullptr;

		delete sa; sa = nullptr;

		delete sa_file; sa_file = nullptr;

		delete lcp_reader; lcp_reader = nullptr;
		
		delete lcp; lcp = nullptr;

		delete lcp_file; lcp_file = nullptr;

		pair1_sorter->sort();

		// scan t to iteratively compute fingeprints in need
		triple_sorter_type* triple_sorter = new triple_sorter_type(triple_comparator_type(), MAIN_MEM_AVAIL / 4);

		stxxl::syscall_file* t_file = new stxxl::syscall_file(m_t_fn, stxxl::syscall_file::DIRECT | stxxl::syscall_file::RDWR);

		alphabet_vector_type* t = new alphabet_vector_type(t_file);

		typename alphabet_vector_type::bufreader_type* t_reader = new typename alphabet_vector_type::bufreader_type(*t);

		fpa_type fp = 0;

		uint16 ch;

		for (uint64 pos = 0; !pair1_sorter->empty(); ++(*pair1_sorter)) {

			const pair1_type& tuple = *(*pair1_sorter);

			while (pos != tuple.first) {

				fp = static_cast<fpa_type>((static_cast<fpb_type>(fp) * R + (*(*t_reader) + 1)) % P);

				++(*t_reader);

				++pos;
			}

			ch = (m_len == pos) ? std::numeric_limits<uint16>::max() : (*(*t_reader));

			triple_sorter->push(triple_type(tuple.second, fp, ch));
		}

		delete t_reader; t_reader = nullptr;

		delete t; t = nullptr;

		delete t_file; t_file = nullptr;

		delete pair1_sorter; pair1_sorter = nullptr;

		triple_sorter->sort();

		return triple_sorter;
	}

	/// \brief compare range fingerprints and ending characters to check the result
	///
	bool check(pair2_sorter_type* _sorter1, triple_sorter_type* _sorter2, triple_sorter_type* _sorter3) {

		bool isRight = true;

		stxxl::syscall_file* lcp_file = new stxxl::syscall_file(m_lcp_fn, stxxl::syscall_file::RDWR | stxxl::syscall_file::DIRECT);

		size_vector_type* lcp = new size_vector_type(lcp_file);

		typename size_vector_type::bufreader_type* lcp_reader = new typename size_vector_type::bufreader_type(*lcp);

		fpa_type fp_ival1, fp_ival2;

		uint16 ch1, ch2;

		size_type cur_lcp;

		++(*lcp_reader); // skip the leftmost lcp value

		cur_lcp = *(*lcp_reader);

		fp_ival2 = static_cast<fpa_type>(((*_sorter3)->second - (static_cast<fpb_type>((*_sorter1)->second) * m_rinterval->compute(cur_lcp)) % P + P) % P);

		ch2 = (*_sorter3)->third;

		++(*_sorter3), ++(*_sorter1);

		for (; !_sorter3->empty(); ++(*_sorter3), ++(*_sorter2), ++(*_sorter1)) {

			fp_ival1 = static_cast<fpa_type>(((*_sorter2)->second - (static_cast<fpb_type>((*_sorter1)->second) * m_rinterval->compute(cur_lcp)) % P + P) % P);

			ch1 = (*_sorter2)->third;

			if (fp_ival1 != fp_ival2 || ch1 == ch2) {
			
				isRight = false;

				break;
			}

			++(*lcp_reader);

			cur_lcp = *(*lcp_reader);

			fp_ival2 = static_cast<fpa_type>(((*_sorter3)->second - (static_cast<fpb_type>((*_sorter1)->second) * m_rinterval->compute(cur_lcp)) % P + P) % P);

			ch2 = (*_sorter3)->third;
		}

		// check final pair
		fp_ival1 = static_cast<fpa_type>(((*_sorter2)->second - (static_cast<fpb_type>((*_sorter1)->second) * m_rinterval->compute(cur_lcp)) % P + P) % P);
	
		ch1 = (*_sorter2)->third;
	
		if (fp_ival1 != fp_ival2 || ch1 == ch2) {
				
			isRight = false;
		}

		delete lcp_reader; lcp_reader = nullptr;

		delete lcp; lcp = nullptr;

		delete lcp_file; lcp_file = nullptr;

		return isRight;
	}

};

#endif // LCPA_EM_H
