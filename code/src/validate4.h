////////////////////////////////////////////////////////////
/// Copyright (c) 2016, Sun Yat-sen University,
/// All rights reserved
/// \file validate4.h
/// \brief validate sa_lms & lcp_lms using method 1 (validate3.h/validate3.cpp) and validate sa & lcp following induced sorting principle & RMQ.
///  
/// \note The alphaber size of the input string is assumed to be O(1) only.
/// \note This is the 2nd method reported in xxx.
///
/// \author Yi Wu
/// \date 2017.1
///////////////////////////////////////////////////////////


#ifndef VALIDATE4_H

#define VALIDATE4_H

#include "common/common.h"

#include "common/tuples.h"

#include "common/widget.h"

#include "common/basicio.h"

#define TEST_VALIDATE4


/// \brief validate sa and lcp using Karp-Rabin fingerprinting function
///
/// type of elements in the input string and suffix/LCP array are specified by alphabet_type and size_type, respectively.
template<typename alphabet_type, typename size_type>
class Validate4{

private:
	// alias
	//	
	typedef typename ExVector<alphabet_type>::vector alphabet_vector_type;

	typedef typename ExVector<size_type>::vector size_vector_type;	
	
	// sort by 1st component
	typedef pair<size_type, size_type> pair1_type;

	typedef tuple_less_comparator_1st<pair1_type> pair1_less_comparator_type;

	typedef typename ExTupleSorter<pair1_type, pair1_less_comparator_type>::sorter pair1_less_sorter_type;

	typedef tuple_great_comparator_1st<pair1_type> pair1_great_comparator_type;

	typedef typename ExTupleSorter<pair1_type, pair1_great_comparator_type>::sorter pair1_great_sorter_type;

	// sort by 1st component
	typedef pair<size_type, fpa_type> pair2_type;
	
	typedef tuple_less_comparator_1st<pair2_type> pair2_comparator_type;
	
	typedef typename ExTupleSorter<pair2_type, pair2_comparator_type>::sorter pair2_sorter_type;
	
	// sort by 1st component
	typedef triple<size_type, fpa_type, uint16> triple_type;

	typedef tuple_less_comparator_1st<triple_type> triple_comparator_type;

	typedef typename ExTupleSorter<triple_type, triple_comparator_type>::sorter triple_sorter_type;

	// sort by 1st component
	typedef triple<size_type, uint8, uint8> triple2_type;

	typedef tuple_less_comparator_1st<triple2_type> triple2_comparator_type;

	typedef typename ExTupleSorter<triple2_type, triple2_comparator_type>::sorter triple2_sorter_type;

	// sort by 1st component
	typedef pair<size_type, uint8> pair3_type;

	typedef tuple_less_comparator_1st<pair3_type> pair3_comparator_type;

	typedef typename ExTupleSorter<pair3_type, pair3_comparator_type>::sorter pair3_sorter_type;

	/// \brief bucket information	
	std::vector<uint64> bkt_size; ///< bucket size

	/// \brief retrieve next character
	///
	template<bool rightward>
	struct RetrieveCh{

		alphabet_type cur_ch;

		alphabet_type last_ch;

		alphabet_type next_ch;

		std::vector<uint64> bkt_toscan; ///< number of elements to scan in each bucket

		std::vector<uint64> bkt_scanned; ///< number of elements scanned in each bucket

		std::vector<uint64> bkt_ch; ///< character of each bucket

		uint64 bkt_idx; ///< index of the bucket currently being scanned

		uint64 bkt_num; ///< at most std::numeric_limits<alphabet_type>::max() + 1

		uint64 total_toscan; ///< number of elements to scan

		uint64 total_scanned; ///< number of scanned elements 

		RetrieveCh(const std::vector<uint64>& _bkt_size) {

			// set total_toscan & bkt_toscan & bktnum
			bkt_num = 0;

			total_toscan = 0;

			for (alphabet_type i = 0; i <= std::numeric_limits<alphabet_type>::max(); ++i) {
				
				if (_bkt_size[i] != 0) {

					bkt_toscan->push_back(_bkt_size[i]);

					bkt_ch->push_back(i);

					bkt_num++;

					total_toscan += _bkt_size[i];
				}
			}
			
			// initialize total_scanned & bkt_scanned
			bkt_scanned.resize(bkt_toscan.size());

			total_scanned = 0;

			// initialize cur_ch & bkt_idx
			cur_ch = (rightward == true) ? bkt_ch[0] : bkt_ch[bktnum - 1];

			bkt_idx = (rightward == true) ? 0 : (bkt_num - 1);
		}

		/// \brief get the head character of currently scanned suffix
		alphabet_type operator*(){

			return cur_ch;
		}

		/// \brief get the head character of the suffix to be scanned (rightward)
		///
		/// \note call the function after confirming the existence of characters remained to scan
		alphabet_type operator++() {

			// move forward, plus one
			bkt_scanned[bkt_idx]++;

			total_scanned++;

			// check if current bucket is finished read
			if (bkt_toscan[bkt_idx] == bkt_scanned[bkt_idx]) {

				if (rightward) {
			
					bkt_idx++;
				}
				else {

					bkt_idx--;
				}

				// check if there still exists a bucket to be scanned
				if (!empty()) { // bkt_idx is valid

					cur_ch = bkt_ch[bkt_idx];
				}
			}
		}

		/// \brief check if all the buckets have been scanned
		///
		bool empty() {

			return total_scanned == total_toscan;	
		}
	};

	/// \brief check induced
	///
	template<bool rightward>
	CheckInduced{

		const std::vector<size_vector_type::const_iterator> m_sa_bkt_iter;

		const std::vector<size_vector_type::const_iterator> m_lcp_bkt_iter;

		const std::vector<size_vector_type::const_reverse_iterator> m_sa_bkt_riter; 

		const std::vector<size_vector_type::const_reverse_iterator> m_lcp_bkt_riter;

		/// \brief ctor
		CheckInduced(const size_vector_type* _sa, const size_vector_type* _lcp, const std::vector<uint64>& _bkt_size) {

			if (rightward == true) {

				uint64 offset = 0;

				m_sa_bkt_iter.resize(_bkt_size.size());
				
				m_lcp_bkt_iter.resize(_bkt_size.size());

				for (uint64 i = 0; i < _bkt_size.size(); ++i) {

					m_sa_bkt_iter[i] = _sa->begin() + offset;

					m_lcp_bkt_iter[i] = _lcp->begin() + offset;

					offset += _bkt_size[i];
				}
			}
			else {

				uint64 offset = 0;

				m_sa_bkt_riter.resize(_bkt_size.size());

				m_lcp_bkt_riter.resize(_bkt_size.size());

				for (uint64 i = _bkt_size.size() - 1; i >= 0; --i) {

					m_sa_bkt_riter[i] = _sa->rbegin() + offset;

					m_lcp_bkt_riter[i] = _lcp->rbegin() + offset;

					offset += _bkt_size[i];
				}
			}	

			
		}

		/// \brief
		/// 
		std::pair<size_type, size_type> get(const alphabet_type _ch) {

			if (rightward) {

				return std::pair<size_type, size_type>(*m_sa_bkt_iter[_ch], *m_lcp_bkt_iter[_ch]);
			}
			else {
			
				return std::pair<size_type, size_type>(*m_lcp_bkt_riter[_ch], *m_lcp_bkt_riter[_ch]);
			}
		}	

		/// \brief
		///
		void forward(const alphabet_type _ch) {
		
			if (rightward) {
	
				m_sa_bkt_iter[_ch]++, m_lcp_bkt_iter[_ch]++;
			}
			else {

				m_sa_bkt_riter[_ch]++, m_lcp_bkt_riter[_ch]++;
			}
		}

		/// \brief
		///
	};

	/// \brief validate sa_lms & lcp_lms following the same idea adopted in validator3.h/validator3.cpp 
	///
	struct LMSValidate{
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

		size_vector_type* m_sa_lms; ///< vector for sa_lms

		size_vector_type* m_lcp_lms; ///< vector for lcp_lms

		uint64 m_lms_num; ///< number of lms suffixes

	public:

		/// \brief constructor
		///
		LMSValidate(const std::string& _t_fn, const std::string& _sa_fn, const std::string& _lcp_fn, const uint64 _len) {

			m_t_fn = _t_fn;
	
			m_sa_fn = _sa_fn;
	
			m_lcp_fn = _lcp_fn;
	
			m_len = _len;
	
			m_rinterval = new RInterval(m_len);		

			m_sa_lms = nullptr;

			m_lcp_lms = nullptr;

			m_lms_num = 0;
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

			// step 1: retrieve sa_lms and lcp_lms from sa and lcp
			retrieve_lms();

			// step 2: fetch fp[0, sa[i] - 1]	
			pair2_sorter_type* sorter1 = fetch_fp();
	
			// step 3: fetch fp[0, sa[i] +lcp[i] - 1]
			triple_sorter_type* sorter2 = fetch_fp_ch_cur();

			// step 4: fetch fp[0, sa[i - 1] + lcp[i] - 1]
			triple_sorter_type* sorter3 = fetch_fp_ch_pre();

			// step 5: check the result
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

		/// \brief retrieve LMS
		void retrieve_lms() {

			// step 1: produce isa via sorting (sa[i], i) by 1st component in descending order
			pair1_great_sorter_type* pair1_great_sorter = new pair1_great_sorter_type(pair1_great_comparator_type(), MAIN_MEM_AVAIL / 2);

			stxxl::syscall_file* sa_file = new stxxl::syscall_file(m_sa_fn, stxxl::syscall_file::RDWR | stxxl::syscall_file::DIRECT);

			size_vector_type* sa = new size_vector_type(sa_file);

			typename size_vector_type::bufreader_type* sa_reader = new typename size_vector_type::bufreader_type(*sa);

			for (uint64 idx = 1; !sa_reader->empty(); ++idx, ++(*sa_reader)) {

				pair1_great_sorter->push(pair1_type(*(*sa_reader), idx));
			} 

			delete sa_reader; sa_reader = nullptr;

			delete sa; sa = nullptr;

			delete sa_file; sa_file = nullptr;

			pair1_great_sorter->sort();


			// step 2: scan t to retrieve sa_lms
			pair1_less_sorter_type* pair1_less_sorter = new pair1_less_sorter_type(pair1_less_comparator_type(), MAIN_MEM_AVAIL / 2);
			
			stxxl::syscall_file* t_file = new stxxl::syscall_file(m_t_fn, stxxl::syscall_file::RDWR | stxxl::syscall_file::DIRECT);

			alphabet_vector_type* t = new alphabet_vector_type(t_file);

			typename alphabet_vector_type::bufreader_reverse_type* t_rev_reader = new typename alphabet_vector_type::bufreader_reverse_type(*t);

			uint8 cur_ch, last_ch;

			uint8 cur_type, last_type;

			last_ch = *(*t_rev_reader); // rightmost character is assumed to be L-type

			last_type = L_TYPE; 

			++(*t_rev_reader);

			for (; !t_rev_reader->empty(); ++(*t_rev_reader), ++(*pair1_great_sorter)) {

				cur_ch = *(*t_rev_reader);

				cur_type = (cur_ch < last_ch || (cur_ch == last_ch && last_type == S_TYPE)) ? S_TYPE : L_TYPE;

				if (cur_type == L_TYPE && last_type == S_TYPE) { // check if LMS for the suffix starting at last_ch

					const pair1_type& tuple = *(*pair1_great_sorter);

					pair1_less_sorter->push(pair1_type(tuple.second, tuple.first));

					++m_lms_num; // collect number of lms suffixes
				}

				last_ch = cur_ch;

				last_type = cur_type;
			}

			++(*pair1_great_sorter); // leftmost character is not LMS, skip it

			delete pair1_great_sorter; pair1_great_sorter = nullptr;
			
			delete t_rev_reader; t_rev_reader = nullptr;

			delete t; t = nullptr;

			delete t_file; t_file = nullptr;

			pair1_less_sorter->sort();

			// step 3: compute lcp_lms and redirect both sa_lms & lcp_lms
			// note that the rightmost in SA/LCP is not LMS
			stxxl::syscall_file* lcp_file = new stxxl::syscall_file(m_lcp_fn, stxxl::syscall_file::RDWR | stxxl::syscall_file::DIRECT);

			size_vector_type* lcp = new size_vector_type(lcp_file);

			typename size_vector_type::bufreader_type* lcp_reader = new typename size_vector_type::bufreader_type(*lcp);

			m_sa_lms = new size_vector_type(); m_sa_lms->resize(m_lms_num);

			typename size_vector_type::bufwriter_type* sa_lms_writer = new typename size_vector_type::bufwriter_type(*m_sa_lms);

			m_lcp_lms = new size_vector_type(); m_lcp_lms->resize(m_lms_num);

			typename size_vector_type::bufwriter_type* lcp_lms_writer = new typename size_vector_type::bufwriter_type(*m_lcp_lms);

			size_type min_lcp = std::numeric_limits<size_type>::max(), cur_lcp;

			size_type idx = 1;

			while (!pair1_less_sorter->empty()) { // elements remains to be processed
	
				while (idx <= (*pair1_less_sorter)->first) {

					if (min_lcp > *(*lcp_reader)) {

						min_lcp = *(*lcp_reader);
					}

					idx = idx + 1;
				}

				// assign values
				(*sa_lms_writer) << (*pair1_less_sorter)->second;

				(*lcp_lms_writer) << min_lcp;

				// reset min_lcp
				min_lcp = std::numeric_limits<size_type>::max();

				// process next sa_lms
				++(*pair1_less_sorter);
			}

			sa_lms_writer->finish();

			delete sa_lms_writer; sa_lms_writer = nullptr;
		
			lcp_lms_writer->finish();

			delete lcp_lms_writer; lcp_lms_writer = nullptr;

			delete lcp_reader; lcp_reader = nullptr;

			delete lcp; lcp = nullptr;

			delete lcp_file; lcp_file = nullptr;

			delete pair1_less_sorter; pair1_less_sorter = nullptr;

			return;
		}		



		/// \brief fetch fp[0, sa_lms[i - 1]]
		///
		pair2_sorter_type* fetch_fp() {
	
			pair1_less_sorter_type* pair1_less_sorter = new pair1_less_sorter_type(pair1_less_comparator_type(), MAIN_MEM_AVAIL / 4);

			typename size_vector_type::bufreader_type* sa_lms_reader = new typename size_vector_type::bufreader_type(*m_sa_lms);

			for (uint64 idx = 1; !sa_lms_reader->empty(); ++(*sa_lms_reader), ++idx) {

				pair1_less_sorter->push(pair1_type(*(*sa_lms_reader), idx));
			}				

			delete sa_lms_reader; sa_lms_reader = nullptr;

			pair1_less_sorter->sort();

			// scan t to iteratively compute fp
			pair2_sorter_type* pair2_sorter = new pair2_sorter_type(pair2_comparator_type(), MAIN_MEM_AVAIL / 4);

			stxxl::syscall_file* t_file = new stxxl::syscall_file(m_t_fn, stxxl::syscall_file::RDWR | stxxl::syscall_file::DIRECT);

			alphabet_vector_type* t = new alphabet_vector_type(t_file);

			typename alphabet_vector_type::bufreader_type* t_reader = new typename alphabet_vector_type::bufreader_type(*t);

			fpa_type fp = 0;

			for (uint64 pos = 0; !pair1_less_sorter->empty(); ++(*pair1_less_sorter)) {

				const pair1_type& tuple = *(*pair1_less_sorter);

				while (pos != tuple.first) {
	
					fp  = static_cast<fpa_type>((static_cast<fpb_type>(fp) * R + (*(*t_reader) + 1)) % P);

					++(*t_reader);

					++pos;
				}

				pair2_sorter->push(pair2_type(tuple.second, fp));
			}

			delete t_reader; t_reader = nullptr;

			delete t; t = nullptr;

			delete t_file; t_file = nullptr;

			delete pair1_less_sorter; pair1_less_sorter = nullptr;

			pair2_sorter->sort();

			return pair2_sorter;	
		}


		/// \brief fetch fp[0, sa_lms[i] + lcp_lms[i] - 1] and t[sa_lms[i] + lcp_lms[i]]

		triple_sorter_type* fetch_fp_ch_cur() {

			pair1_less_sorter_type* pair1_less_sorter = new pair1_less_sorter_type(pair1_less_comparator_type(), MAIN_MEM_AVAIL / 4);

			typename size_vector_type::bufreader_type* sa_lms_reader = new typename size_vector_type::bufreader_type(*m_sa_lms);

			typename size_vector_type::bufreader_type* lcp_lms_reader = new typename size_vector_type::bufreader_type(*m_lcp_lms);

			++(*sa_lms_reader), ++(*lcp_lms_reader); // no need to compute fp[0, sa_lms[0] + lcp_lms[0] - 1]

			for (uint64 idx = 1; !sa_lms_reader->empty(); ++(*sa_lms_reader), ++(*lcp_lms_reader), ++idx) {

				pair1_less_sorter->push(pair1_type(*(*sa_lms_reader) + *(*lcp_lms_reader), idx));
			}


			delete sa_lms_reader; sa_lms_reader = nullptr;
			
			delete lcp_lms_reader; lcp_lms_reader = nullptr;

			pair1_less_sorter->sort();

			// scan t to iteratively compute fingerprints in need
			triple_sorter_type* triple_sorter = new triple_sorter_type(triple_comparator_type(), MAIN_MEM_AVAIL / 4);

			stxxl::syscall_file* t_file = new stxxl::syscall_file(m_t_fn, stxxl::syscall_file::DIRECT | stxxl::syscall_file::RDWR);

			alphabet_vector_type* t = new alphabet_vector_type(t_file);

			typename alphabet_vector_type::bufreader_type* t_reader = new typename alphabet_vector_type::bufreader_type(*t);

			fpa_type fp = 0;

			uint16 ch;

			for (uint64 pos = 0; !pair1_less_sorter->empty(); ++(*pair1_less_sorter)) {

				const pair1_type& tuple = *(*pair1_less_sorter);

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
				
			delete pair1_less_sorter; pair1_less_sorter = nullptr;

			triple_sorter->sort();

			return triple_sorter;
		}

		/// \brief fetch fp[0, sa_lms[i - 1] + lcp_lms[i - 1] and t[sa_lms[i - 1] + lcp_lms[i - 1]]
		///
		triple_sorter_type* fetch_fp_ch_pre() {

			pair1_less_sorter_type* pair1_less_sorter = new pair1_less_sorter_type(pair1_less_comparator_type(), MAIN_MEM_AVAIL / 4);

			typename size_vector_type::bufreader_type* sa_lms_reader = new typename size_vector_type::bufreader_type(*m_sa_lms);

			typename size_vector_type::bufreader_type* lcp_lms_reader = new typename size_vector_type::bufreader_type(*m_lcp_lms);

			++(*lcp_lms_reader); // skip the leftmost lcp	

			for (uint64 idx = 1; !lcp_lms_reader->empty(); ++(*sa_lms_reader), ++(*lcp_lms_reader), ++idx) {

				pair1_less_sorter->push(pair1_type(*(*sa_lms_reader) + *(*lcp_lms_reader), idx));	
			}

			delete sa_lms_reader; sa_lms_reader = nullptr;

			delete lcp_lms_reader; lcp_lms_reader = nullptr;

			pair1_less_sorter->sort();

			// scan t to iteratively compute fingerprints in need
			triple_sorter_type* triple_sorter = new triple_sorter_type(triple_comparator_type(), MAIN_MEM_AVAIL / 4);

			stxxl::syscall_file* t_file = new stxxl::syscall_file(m_t_fn, stxxl::syscall_file::DIRECT | stxxl::syscall_file::RDWR);

			alphabet_vector_type* t = new alphabet_vector_type(t_file);

			typename alphabet_vector_type::bufreader_type* t_reader = new typename alphabet_vector_type::bufreader_type(*t);

			fpa_type fp = 0;

			uint16 ch;

			for (uint64 pos = 0; !pair1_less_sorter->empty(); ++(*pair1_less_sorter)) {

				const pair1_type& tuple = *(*pair1_less_sorter);

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

			delete pair1_less_sorter; pair1_less_sorter = nullptr;

			triple_sorter->sort();

			return triple_sorter;
		} 

		/// \brief compare range fingerprints and ending characters to check the result	
		///
		bool check(pair2_sorter_type* _sorter1, triple_sorter_type* _sorter2, triple_sorter_type* _sorter3) {

			bool isRight = true;

			typename size_vector_type::bufreader_type* lcp_lms_reader = new typename size_vector_type::bufreader_type(*m_lcp_lms);

			fpa_type fp_ival1, fp_ival2;

			uint16 ch1, ch2;

			size_type cur_lcp;

			++(*lcp_lms_reader); // skip the leftmost lcp value

			cur_lcp = *(*lcp_lms_reader);

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

				++(*lcp_lms_reader);

				cur_lcp = *(*lcp_lms_reader);

				fp_ival2 = static_cast<fpa_type>(((*_sorter3)->second - (static_cast<fpb_type>((*_sorter1)->second) * m_rinterval->compute(cur_lcp)) % P + P) % P);

				ch2 = (*_sorter3)->third;
			}

			// check final pair
			fp_ival1 = static_cast<fpa_type>(((*_sorter2)->second - (static_cast<fpb_type>((*_sorter1)->second) * m_rinterval->compute(cur_lcp)) % P + P) % P);

			ch1 = (*_sorter2)->third;
	
			if (fp_ival1 != fp_ival2 || ch1 == ch2) {

				isRight = false;
			}

			delete lcp_lms_reader; lcp_lms_reader = nullptr;

			return isRight;
		}
	};


	///

private:

	std::string m_t_fn;

	std::string m_sa_fn;

	std::string m_lcp_fn;

	uint64 m_len; 

	std::vector<uint64> m_l_bkt_size; ///< record size for each L-type bucket 

	std::vector<uint64> m_s_bkt_size; ///< record size for each S-type bucket

	std::vector<uint64> m_lms_bkt_size; ///< record size for each LMS-type bucket

	pair3_sorter_type* m_lms_pre_sorter; ///< sort preceding information for lms-type suffixes

	triple2_sorter_type* m_l_pre_sorter; ///< sort preceding information for l-type suffixes

	triple2_sorter_type* m_s_pre_sorter; ///< sort preceding information for s-type suffixes
	`

public:
	/// \brief constructor
	///
	Validate4(const std::string& _t_fn, const std::string& _sa_fn, const std::string& _lcp_fn) {

		m_t_fn = _t_fn;

		m_sa_fn = _sa_fn;

		m_lcp_fn = _lcp_fn;

		m_len = BasicIO::file_size(_t_fn) / sizeof(alphabet_type);

		m_l_bkt_size.resize(std::numeric_limits<alphabet_type>::max() + 1);

		m_s_bkt_size.resize(std::numeric_limits<alphabet_type>::max() + 1);

		m_lms_bkt_size.resize(std::numeric_limits<alphabet_type>::max() + 1);
	}

	/// \brief core part of the program
	///
	bool run() {

		// step 1: validate the correctness of sa_lms & lcp_lms using the karp-Rabin fingerprinting function
		LMSValidate lms_validate(m_t_fn, m_sa_fn, m_lcp_fn, m_len);

		if (lms_validate.run() == false) {

			std::cerr << "sa_lms & lcp_lms are wrong\n";

			return false;
		}
		else {

			std::cerr << "sa_lms & lcp_lms are correct\n";
		}


		// step 2: validate the correctness of sa & lcp following the induced-sorting principle
		compute_bwt();

		if (check_l() == false) {

			std::cerr << "sa & lcp are wrong\n"; 

			return false;
		}


		if (check_s() == false) {

			std::cerr << "sa & lcp are wrong\n";

			return false;
		}

		return true;
	}

	/// \brief compute bwt 
	void compute_bwt() {
		
		// produce isa
		pair1_great_sorter_type* pair1_great_sorter = new pair1_great_sorter_type(pair1_great_comparator_type(), MAIN_MEM_AVAIL / 4);

		stxxl::syscall_file* sa_file = new stxxl::syscall_file(m_sa_fn, stxxl::syscall_file::RDWR | stxxl::syscall_file::DIRECT);

		size_vector_type* sa = new size_vector_type(sa_file);

		typename size_vector_type::bufreader_type* sa_reader = new typename size_vector_type::bufreader_type(*sa);

		for (uint64 idx = 1; !sa_reader->empty(); ++(*sa_reader), ++idx) {

			pair1_great_sorter->push(pair1_type(*(*sa_reader), idx));
		}
		
		delete sa_reader; sa_reader = nullptr;

		delete sa; sa = nullptr;

		delete sa_file; sa_file = nullptr;

		pair1_great_sorter->sort();
		
		// scan t to collect pre_ch & pre_t for L/S/LMS suffixes 
		m_l_pre_sorter = new triple2_sorter_type(triple2_comparator_type(), MAIN_MEM_AVAIL / 4);

		m_s_pre_sorter = new triple2_sorter_type(triple2_comparator_type(), MAIN_MEM_AVAIL / 4);

		m_lms_pre_sorter = new pair3_sorter_type(pair3_comparator_type(), MAIN_MEM_AVAIL / 4); 

		stxxl::syscall_file* t_file = new stxxl::syscall_file(m_t_fn, stxxl::syscall_file::RDWR | stxxl::syscall_file::DIRECT);

		alphabet_vector_type* t = new alphabet_vector_type(t_file);

		typename alphabet_vector_type::bufreader_reverse_type* t_rev_reader = new typename alphabet_vector_type::bufreader_reverse_type(*t);

		uint8 last_ch, cur_ch;

		uint8 last_type, cur_type;

		last_type = L_TYPE; // rightmost is L-type

		last_ch = *t_rev_reader; // last character

		++(*t_rev_reader);
	
		for (; !t_rev_reader->empty(); ++(*t_rev_reader)) {

			cur_ch = *(*t_rev_reader);

			cur_type = (cur_ch < last_ch || cur_ch == last_ch && last_type == S_TYPE) ? S_TYPE : L_TYPE;

			if (last_type == S_TYPE) {

				if (cur_type == L_TYPE) { // find a LMS

					m_lms_pre_sorter->push(pair3_type((*pair1_great_sorter)->second, cur_ch); // last_ch is LMS-type

					m_s_pre_sorter->push(triple2_type((*pair1_great_sorter)->second, cur_ch, cur_type)); // last_ch is also S-type

					m_s_bkt_size[last_ch]++; // collect bucket information
				}
				else {

					m_s_pre_sorter->push(triple2_type((*pair1_great_sorter)->second, cur_ch, cur_type)); // last_ch is S-type

					m_lms_bkt_size[last_ch]++; // collect bucket information
				}
			}
			else {
				m_l_pre_sorter->push(triple2_type((*pair1_great_sorter)->second, cur_ch, cur_type)); // last_ch is L-type

				m_l_bkt_size[last_ch]++; // collect bucket information
			}

			++(*t_rev_reader);

			last_ch = cur_ch;

			last_type = cur_type;
		}

		// leftmost is not a LMS, collect bucket information
		// we assume the preceding character is a sentinel and denote it by 0
		if (last_type == S_TYPE) {

			m_s_pre_sorter->push(triple2_type((*pair1_great_sorter)->second, 0, SENTINEL_TYPE));

			m_s_bkt_size[last_ch]++;
		}
		else {
			m_l_pre_sorter->push(triple2_type((*pair1_great_sorter)->second, 0, SENTINEL_TYPE));

			m_l_bkt_size[last_ch]++;
		}

		delete t_rev_reader; t_rev_reader = nullptr;

		delete t; t = nullptr;

		delete pair1_great_sorter; pair1_great_sorter = nullptr;

		m_lms_pre_sorter->sort();
	
		m_s_pre_sorter->sort();

		m_l_pre_sorter->sort();
	}

	/// \brief compute the LCP of two suffixes (indicated by their starting positions) by literally comparing their characters
	///
	void bruteforce_compute_lcp(const uint64& _pos1, const uint64& _pos2) {

	
	}

	/// \brief check SA_L/LCP_L 
	///
	bool check_l() {

		bool isRight = true;

		RetrieveCh<true> retrieve_lms_ch(m_lms_bkt_size);

		RetrieveCh<true> retrieve_l_ch(m_l_bkt_size);
	
		uint8 cur_ch;

		// assume there exists a sentinel on the right side
		// its preceding character is an L-type 
				
				

		return true;
	}

	/// \brief check SA_S/LCP_S
	///
	bool check_s() {

		return true;
	}
};

#endif // LCPA_EM_H
