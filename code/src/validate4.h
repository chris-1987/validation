///////////////////////////////////////////////////////////////////////////////////
/// Copyright (c) 2016, Sun Yat-sen University,
/// All rights reserved
/// \file validate4.h
/// \brief A two-phase validator for suffix and LCP arrays verification.
///
/// A Karp-Rabin method is employed to check SA_LMS & LCP_LMS. 
/// This method is also applied to the verification for SA and LCP, where an implementation can be found in validate3.h/validate3.cpp
/// When finished checking SA_LMS & LCP_LMS, we validate SA & LCP following the idea of the induced-sorting principle and RMQ.
///
/// \note The alphabet size of the input string is assumed to be constant.
/// \note A description (with more details) is available in our draft attached.
/// \author Yi Wu
/// \date 2017.1 
//////////////////////////////////////////////////////////////////////////////////

#ifndef VALIDATE4_H
#define VALIDATE4_H

#include "common/common.h"

#include "common/tuples.h"

#include "common/widget.h"

#include "common/basicio.h"

#include "test.h"

//#define TEST_VALIDATE4 // for test only, comment out the line if not required



/// \brief vaildate SA and LCP following the induced-sorting principle
///
/// \param alphabet_type for elements in T
/// \param alphabet_extension_type for instance, given alphabet_type = uint8, we have alphbet_extension_type = uint16
/// \param size_type for elements in SA/LCP
template<typename alphabet_type, typename alphabet_extension_type, typename size_type>
class Validate4 {

private:

	// alias for vectors, tuples, sorters and comparators
	// vectors
	typedef typename ExVector<alphabet_type>::vector alphabet_vector_type;

	typedef typename ExVector<size_type>::vector size_vector_type;

	// tuples, sorters and comparators
	//
	typedef pair<size_type, size_type> pair1_type;

	typedef tuple_less_comparator_1st<pair1_type> pair1_less_comparator_1st_type; // compare by 1st component in ascending order

	typedef typename ExTupleSorter<pair1_type, pair1_less_comparator_1st_type>::sorter pair1_less_sorter_1st_type;

	typedef tuple_great_comparator_1st<pair1_type> pair1_great_comparator_1st_type; // compare by 1st component in descending order

	typedef typename ExTupleSorter<pair1_type, pair1_great_comparator_1st_type>::sorter pair1_great_sorter_1st_type;

	typedef tuple_less_comparator_2nd<pair1_type> pair1_less_comparator_2nd_type; // compare by (1st, 2nd) components in ascending order

	typedef typename ExTupleSorter<pair1_type, pair1_less_comparator_2nd_type>::sorter pair1_less_sorter_2nd_type;

	//
	typedef pair<size_type, fpa_type> pair2_type;

	typedef tuple_less_comparator_1st<pair2_type> pair2_less_comparator_1st_type; // compare by 1st component in ascending order

	typedef typename ExTupleSorter<pair2_type, pair2_less_comparator_1st_type>::sorter pair2_less_sorter_1st_type;

	//
	typedef pair<size_type, alphabet_type> pair3_type;

	typedef tuple_less_comparator_1st<pair3_type> pair3_less_comparator_1st_type; // compare by 1st component in ascending order

	typedef typename ExTupleSorter<pair3_type, pair3_less_comparator_1st_type>::sorter pair3_less_sorter_1st_type;

	typedef pair<alphabet_type, uint8> pair4_type;

	typedef typename ExVector<pair4_type>::vector pair4_vector_type; //

	//
	typedef triple<size_type, fpa_type, alphabet_extension_type> triple1_type;

	typedef tuple_less_comparator_1st<triple1_type> triple1_less_comparator_1st_type; // compare by 1st component in ascending order

	typedef typename ExTupleSorter<triple1_type, triple1_less_comparator_1st_type>::sorter triple1_less_sorter_1st_type;

	//
	typedef triple<size_type, alphabet_type, uint8> triple2_type;

	typedef tuple_less_comparator_1st<triple2_type> triple2_less_comparator_1st_type; // compare by first component in ascending order

	typedef typename ExTupleSorter<triple2_type, triple2_less_comparator_1st_type>::sorter triple2_less_sorter_1st_type;

	typedef tuple_great_comparator_1st<triple2_type> triple2_great_comparator_1st_type; // compare by first component in descending rder

	typedef typename ExTupleSorter<triple2_type, triple2_great_comparator_1st_type>::sorter triple2_great_sorter_1st_type;
	
	//
	typedef triple<size_type, fpa_type, alphabet_extension_type> triple3_type;

	typedef tuple_less_comparator_1st<triple3_type> triple3_less_comparator_1st_type; // compare by 1st component in ascending order

	typedef typename ExTupleSorter<triple3_type, triple3_less_comparator_1st_type>::sorter triple3_less_sorter_1st_type;

private:

	/// \brief record L-type, S-type and LMS bucket sizes (no matter empty or non-empty) in SA & LCP
	///
	/// Elements in SA & LCP are naturally divided into multiple buckets and each bucket contains all the suffixes starting with an identical character.
	/// Each bucket can be further divided into two parts, whhere the left and right part separately contain the L-type and S-type suffixes, respectively.
	/// For description convenience, we denote the two part as L-type bucket and S-type bucket, respectively.
	struct BktInfo {
	public:

		const alphabet_type ch_max; ///< maximum value for alphabet_type

		std::vector<uint64> m_bkt_size; ///< bucket size

		std::vector<uint64> m_l_bkt_size; ///< L-type bucket size

		std::vector<uint64> m_s_bkt_size; ///< S-type bucket size

		std::vector<uint64> m_lms_bkt_size; ///< LMS bucket size, all the LMS suffixes are S-type suffixes.

	public:

		/// \brief ctor
		///
		///  the number of buckets is ch_max + 1, indexed by 0, 1, ..., ch_max.
		BktInfo() : ch_max(std::numeric_limits<alphabet_type>::max()) {

			m_bkt_size.resize(ch_max + 1, 0);

			m_l_bkt_size.resize(ch_max + 1, 0);

			m_s_bkt_size.resize(ch_max + 1, 0);

			m_lms_bkt_size.resize(ch_max + 1, 0);
		}

		/// \brief count the number of L-type suffixes in the specified bucket
		///
		void add_l(const alphabet_type _ch) {

			++m_l_bkt_size[_ch];

			return;
		}

		/// \brief count the number of S-type suffixes in the specified bucket
		///
		void add_s(const alphabet_type _ch) {

			++m_s_bkt_size[_ch];

			return;
		}

		/// \brief count the number of LMS suffixes in the specified bucket
		///
		void add_lms(const alphabet_type _ch) {

			++m_lms_bkt_size[_ch];

			return;
		}

		/// \brief collect the number of suffixes in the specified bucket
		///
		/// the number of suffixes in a bucket can be obtained by summing up the numbers of L-type and S-type suffixes. 
		void accumulate() {

			for (alphabet_extension_type ch = 0; ch <= ch_max; ++ch) {

				m_bkt_size[ch] = m_l_bkt_size[ch] + m_s_bkt_size[ch];
			}

			return;
		}

		/// \brief return the number of L-type suffixes in the specified bucket
		///
		uint64 get_l_bkt_size(const alphabet_type _ch) const{

			return m_l_bkt_size[_ch];
		}

		/// \brief return the number of S-type suffixes in the specified bucket
		///
		uint64 get_s_bkt_size(const alphabet_type _ch) const{

			return m_s_bkt_size[_ch];
		}

		/// \brief return the number of LMS suffixes in the specified bucket
		///
		uint64 get_lms_bkt_size(const alphabet_type _ch) const{

			return m_lms_bkt_size[_ch];
		}

		/// \brief return the number of suffixes in the specified bucket
		///
		uint64 get_bkt_size(const alphabet_type _ch) const {

			return m_bkt_size[_ch];
		}

		void display() const{

			uint64 total_num = 0;

			uint64 total_l_num = 0, total_s_num = 0, total_lms_num = 0;

			for (alphabet_extension_type ch = 0; ch <= ch_max; ++ch) {

				std::cerr << "ch: " << static_cast<uint64>(ch) << " ";

				std::cerr << "size: " << m_bkt_size[ch] << " ";

				std::cerr << "L-type: " << m_l_bkt_size[ch] << " ";

				std::cerr << "S-type : " << m_s_bkt_size[ch] << " ";

				std::cerr << "LMS: " << m_lms_bkt_size[ch];

				std::cerr << std::endl;

				total_num += m_bkt_size[ch];

				total_l_num += m_l_bkt_size[ch];

				total_s_num += m_s_bkt_size[ch];

				total_lms_num += m_lms_bkt_size[ch];
			}


			std::cerr << "total num: " << total_num << std::endl;
	
			std::cerr << "total l num: " << total_l_num << std::endl;

			std::cerr << "total s num: " << total_s_num << std::endl;

			std::cerr << "total lms num: " << total_lms_num << std::endl;

			return;
		}

	};

	BktInfo m_bkt_info; ///< an instance of BktInfo, collect information when validating SA_LMS & LCP_LMS


	/// \brief validate SA_LMS & LCP_LMS
	///
	/// The Karp-Rabin fingerprinting function is exploited to validate the correctness of SA_LMS & LCP_LMS.
	struct LMSValidate {

	private:

		/// \brief store (R % P)^1, (R % P)^2, (R % P)^4, ...
		///
		struct RInterval {

			fpa_type* m_data; ///< data load

			uint64 m_num;

			/// \brief ctor
			///
			RInterval(uint64 _len) {

				m_num = 1;

				while (_len) {

					++m_num;

					_len /= 2;
				}

				m_data = new fpa_type[m_num];

				m_data[0] = R % P;

				for (uint64 i = 1; i < m_num; ++i) {

					m_data[i] = static_cast<fpa_type>((static_cast<fpb_type>(m_data[i - 1]) * m_data[i - 1]) % P);
				}

				return;
			}

			/// \brief get fpInterval
			///
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

		alphabet_vector_type* m_t; ///< input string

		size_vector_type* m_sa; ///< input suffix array

		size_vector_type* m_lcp; ///< input LCP array

		uint64 m_len; ///< length of input string

		uint64 m_lms_num; ///< number of LMS suffixes

		RInterval* m_rinterval; ///< pointer to an RInterval object

		size_vector_type* m_sa_lms; ///< pointer to SA_LMS

		size_vector_type* m_lcp_lms; ///< pointer to LCP_LMS

		const alphabet_type ch_max; ///< maximum value of alphabet type

		const size_type val_max; ///< maximum value of size type

	public:
		/// \brief ctor
		///
		LMSValidate(alphabet_vector_type* _t, size_vector_type* _sa, size_vector_type* _lcp) :
			m_t(_t), 
			m_sa(_sa), 
			m_lcp(_lcp), 
			m_len(m_t->size()), 
			m_lms_num(0), 
			m_sa_lms(nullptr), 
			m_lcp_lms(nullptr), 
			ch_max(std::numeric_limits<alphabet_type>::max()), 
			val_max(std::numeric_limits<size_type>::max()) {

			m_rinterval = new RInterval(m_len);
		}

		/// \brief retrive SA_LMS and LCP_LMS from SA and LCP
		///
		void retrieve_lms() {

			// step 1: sort (SA[i], i) by 1st component in descending order
			pair1_great_sorter_1st_type* pair1_great_sorter = new pair1_great_sorter_1st_type(pair1_great_comparator_1st_type(), MAIN_MEM_AVAIL / 2);

			typename size_vector_type::bufreader_type* sa_reader = new typename size_vector_type::bufreader_type(*m_sa);

			for (uint64 idx = 1; !sa_reader->empty(); ++idx, ++(*sa_reader)) {

				pair1_great_sorter->push(pair1_type(*(*sa_reader), idx));
			}

			delete sa_reader; sa_reader = nullptr;

			pair1_great_sorter->sort();

			// step 2: scan T leftward to determine character type and pick LMS suffixes
			pair1_less_sorter_1st_type* pair1_less_sorter = new pair1_less_sorter_1st_type(pair1_less_comparator_1st_type(), MAIN_MEM_AVAIL / 2);

			typename alphabet_vector_type::bufreader_reverse_type* t_rev_reader = new typename alphabet_vector_type::bufreader_reverse_type(*m_t);

			uint8 last_scanned_type, cur_scanned_type;

			alphabet_type last_scanned_ch, cur_scanned_ch;

			uint64 pos = m_len - 1;

			// rightmost character is L_TYPE
			cur_scanned_type = L_TYPE, cur_scanned_ch = *(*t_rev_reader);			

			last_scanned_type = cur_scanned_type, last_scanned_ch = cur_scanned_ch;

			++(*t_rev_reader);

			for (; !t_rev_reader->empty(); --pos, ++(*t_rev_reader), ++(*pair1_great_sorter)) { // process the remaining, check if last scanned is LMS
			
				const pair1_type& tuple = *(*pair1_great_sorter); // for the last scanned suffix

				if (pos != tuple.first) {
				
					std::cerr << "SA is not a permutation\n";

					exit(0);
				}

				cur_scanned_ch = *(*t_rev_reader);

				cur_scanned_type = ((cur_scanned_ch < last_scanned_ch) || (cur_scanned_ch == last_scanned_ch && last_scanned_type == S_TYPE)) ? S_TYPE : L_TYPE;

				if (cur_scanned_type == L_TYPE && last_scanned_type == S_TYPE) { // last scanned is LMS

					++m_lms_num;

					pair1_less_sorter->push(pair1_type(tuple.second, tuple.first)); // push the last scanned
				}

				last_scanned_ch = cur_scanned_ch;

				last_scanned_type = cur_scanned_type;
			}

			// leftmost
			if (pos != (*pair1_great_sorter)->first) {

				std::cerr << "SA is not a permutation\n";

				exit(0);
			}

			++(*pair1_great_sorter);

			delete t_rev_reader; t_rev_reader = nullptr;

			delete pair1_great_sorter; pair1_great_sorter = nullptr;

			pair1_less_sorter->sort();

#ifdef TEST_VALIDATE4
			std::cerr << "m_lms_num: " << m_lms_num << std::endl;
#endif

			// step 3: compute LCP_LMS and redirect SA_LMS & LCP_LMS to the external files
			typename size_vector_type::bufreader_type* lcp_reader = new typename size_vector_type::bufreader_type(*m_lcp);

			m_sa_lms = new size_vector_type(); m_sa_lms->resize(m_lms_num);

			typename size_vector_type::bufwriter_type* sa_lms_writer = new typename size_vector_type::bufwriter_type(*m_sa_lms);

			m_lcp_lms = new size_vector_type(); m_lcp_lms->resize(m_lms_num);

			typename size_vector_type::bufwriter_type* lcp_lms_writer = new typename size_vector_type::bufwriter_type(*m_lcp_lms);

			size_type lcp_min = 0; // the leftmost LMS in SA_LMS has no left neighbor, set lcp_min = 0 to let the LCP-value be 0

			for (uint64 idx = 1; !pair1_less_sorter->empty(); ++(*pair1_less_sorter)) {
			
				const pair1_type& tuple = *(*pair1_less_sorter);

				while (idx != tuple.first) {
				
					lcp_min = std::min(lcp_min, *(*lcp_reader));

					++(*lcp_reader), ++idx;
				}

				// idx == tuple.first, redirect
				(*sa_lms_writer) << tuple.second;

				lcp_min = std::min(lcp_min, *(*lcp_reader));

				++(*lcp_reader), ++idx;

				(*lcp_lms_writer) << lcp_min;

				// reset
				lcp_min = val_max;
			}

			(*sa_lms_writer).finish();

			(*lcp_lms_writer).finish();

			delete sa_lms_writer; sa_lms_writer = nullptr;

			delete lcp_lms_writer; lcp_lms_writer = nullptr;

			delete lcp_reader; lcp_reader = nullptr;

			delete pair1_less_sorter; pair1_less_sorter = nullptr;

			return;
		}

		/// \brief fetch fp[0, SA_LMS[i] - 1]
		///
		pair2_less_sorter_1st_type* fetch_fp() {

			// produce ISA_LMS
			pair1_less_sorter_1st_type* pair1_less_sorter = new pair1_less_sorter_1st_type(pair1_less_comparator_1st_type(), MAIN_MEM_AVAIL / 4);

			typename size_vector_type::bufreader_type* sa_lms_reader = new typename size_vector_type::bufreader_type(*m_sa_lms);

			for (uint64 idx = 1; !sa_lms_reader->empty(); ++idx, ++(*sa_lms_reader)) {

				pair1_less_sorter->push(pair1_type(*(*sa_lms_reader), idx));
			}

			delete sa_lms_reader; sa_lms_reader = nullptr;

			pair1_less_sorter->sort();

			// scan T to iteratively compute fp[0, pos] and sort ISA_LMS back to SA_LMS along with the fingerprints in need
			pair2_less_sorter_1st_type* pair2_less_sorter = new pair2_less_sorter_1st_type(pair2_less_comparator_1st_type(), MAIN_MEM_AVAIL / 4);

			typename alphabet_vector_type::bufreader_type* t_reader = new typename alphabet_vector_type::bufreader_type(*m_t);

			fpa_type fp = 0;

			for (uint64 pos = 0; !pair1_less_sorter->empty(); ++(*pair1_less_sorter)) {

				const pair1_type& tuple = *(*pair1_less_sorter);

				while (pos != tuple.first) {

					fp = static_cast<fpa_type>((static_cast<fpb_type>(fp) * R + (*(*t_reader) + 1)) % P); // plus 1 to avoid equal to 0

					++(*t_reader), ++pos;
				} 

				pair2_less_sorter->push(pair2_type(tuple.second, fp));

				fp = static_cast<fpa_type>((static_cast<fpb_type>(fp) * R + (*(*t_reader) + 1)) % P); // plus 1 to avoid equal to 0

				++(*t_reader), ++pos;				
			}

			delete t_reader; t_reader = nullptr;

			delete pair1_less_sorter; pair1_less_sorter = nullptr;

			pair2_less_sorter->sort();

			return pair2_less_sorter;
		}

		/// \brief fetch fp[0, SA_LMS[i] + LCP_LMS[i] - 1] and T[SA_LMS[i] + LCP_LMS[i]]
		///
		triple3_less_sorter_1st_type* fetch_fp_ch_cur() {

			// sort (SA_LMS[i] + LCP_LMS[i], i) by the components 
			pair1_less_sorter_2nd_type* pair1_less_2nd_sorter = new pair1_less_sorter_2nd_type(pair1_less_comparator_2nd_type(), MAIN_MEM_AVAIL / 4);

			typename size_vector_type::bufreader_type* sa_lms_reader = new typename size_vector_type::bufreader_type(*m_sa_lms);

			typename size_vector_type::bufreader_type* lcp_lms_reader = new typename size_vector_type::bufreader_type(*m_lcp_lms);

			++(*sa_lms_reader), ++(*lcp_lms_reader); // skip the leftmost 

			for (uint64 idx = 1; !sa_lms_reader->empty(); ++idx, ++(*sa_lms_reader), ++(*lcp_lms_reader)) {

				pair1_less_2nd_sorter->push(pair1_type(*(*sa_lms_reader) + *(*lcp_lms_reader), idx));
			}

			delete sa_lms_reader; sa_lms_reader = nullptr;

			delete lcp_lms_reader; lcp_lms_reader = nullptr;

			pair1_less_2nd_sorter->sort();

			// scan T to iteratively compute fp[0, pos] and sort ISA_LMS back to SA_LMS along with the fingerprints in need
			triple3_less_sorter_1st_type* triple3_less_sorter = new triple3_less_sorter_1st_type(triple3_less_comparator_1st_type(), MAIN_MEM_AVAIL / 4);

			typename alphabet_vector_type::bufreader_type* t_reader = new typename alphabet_vector_type::bufreader_type(*m_t);

			fpa_type fp = 0;

			for (uint64 pos = 0; !t_reader->empty(); ++pos, ++(*t_reader)) {
			
				while (!pair1_less_2nd_sorter->empty() && pos == (*pair1_less_2nd_sorter)->first) {
				
					triple3_less_sorter->push(triple3_type((*pair1_less_2nd_sorter)->second, fp, *(*t_reader))); //fp = FP[0, pos - 1], ch = T[pos]

					++(*pair1_less_2nd_sorter);
				}

				if (pair1_less_2nd_sorter->empty()) {

					break;		
				}

				fp = static_cast<fpa_type>((static_cast<fpb_type>(fp) * R + (*(*t_reader) + 1)) % P); // plus 1 to avoid equal to 0
			}

			// (*pair1_less_2nd_sorter)->second == m_len
			while (!pair1_less_2nd_sorter->empty()) {
			
				triple3_less_sorter->push(triple3_type((*pair1_less_2nd_sorter)->second, fp, ch_max + 1)); // fp = FP[0, m_len - 1], ch = max + 1

				++(*pair1_less_2nd_sorter);
			}

			delete t_reader; t_reader = nullptr;

			delete pair1_less_2nd_sorter; pair1_less_2nd_sorter = nullptr;

			triple3_less_sorter->sort();

			return triple3_less_sorter;
		}

		/// \brief fetch fp[0, SA_LMS[i] + LCP_LMS[i + 1] - 1] and T[SA_LMS[i] + LCP_LMS[i + 1]]
		///
		triple3_less_sorter_1st_type* fetch_fp_ch_pre() {

			// sort (SA_LMS[i] + LCP_LMS[i + 1], i) by the components 
			pair1_less_sorter_2nd_type* pair1_less_2nd_sorter = new pair1_less_sorter_2nd_type(pair1_less_comparator_2nd_type(), MAIN_MEM_AVAIL / 4);

			typename size_vector_type::bufreader_type* sa_lms_reader = new typename size_vector_type::bufreader_type(*m_sa_lms);

			typename size_vector_type::bufreader_type* lcp_lms_reader = new typename size_vector_type::bufreader_type(*m_lcp_lms);

			++(*lcp_lms_reader); // only skip the leftmost LCP-value

			for (uint64 idx = 1; !lcp_lms_reader->empty(); ++idx, ++(*sa_lms_reader), ++(*lcp_lms_reader)) {

				pair1_less_2nd_sorter->push(pair1_type(*(*sa_lms_reader) + *(*lcp_lms_reader), idx));
			}

			delete sa_lms_reader; sa_lms_reader = nullptr;

			delete lcp_lms_reader; lcp_lms_reader = nullptr;

			pair1_less_2nd_sorter->sort();

			// scan T to iteratively compute fp[0, pos] and sort ISA_LMS back to SA_LMS along with the fingerprints in need
			triple3_less_sorter_1st_type* triple3_less_sorter = new triple3_less_sorter_1st_type(triple3_less_comparator_1st_type(), MAIN_MEM_AVAIL / 4);

			typename alphabet_vector_type::bufreader_type* t_reader = new typename alphabet_vector_type::bufreader_type(*m_t);

			fpa_type fp = 0;

			for (uint64 pos = 0; !t_reader->empty(); ++pos, ++(*t_reader)) {

				while (!pair1_less_2nd_sorter->empty() && pos == (*pair1_less_2nd_sorter)->first) {

					triple3_less_sorter->push(triple3_type((*pair1_less_2nd_sorter)->second, fp, *(*t_reader)));

					++(*pair1_less_2nd_sorter);
				}

				if (pair1_less_2nd_sorter->empty()) {

					break;
				}

				fp = static_cast<fpa_type>((static_cast<fpb_type>(fp) * R + (*(*t_reader) + 1)) % P); // plus 1 to avoid equal to 0
			}

			// (*pair1_less_2nd_sorter)->second == m_len
			while (!pair1_less_2nd_sorter->empty()) {

				triple3_less_sorter->push(triple3_type((*pair1_less_2nd_sorter)->second, fp, ch_max + 1));

				++(*pair1_less_2nd_sorter);
			}

			delete t_reader; t_reader = nullptr;

			delete pair1_less_2nd_sorter; pair1_less_2nd_sorter = nullptr;

			triple3_less_sorter->sort();

			return triple3_less_sorter;
		}

		/// \brief check the result
		///
		bool check(pair2_less_sorter_1st_type* _sorter1, triple3_less_sorter_1st_type* _sorter2, triple3_less_sorter_1st_type* _sorter3) {
		
			bool is_right = true;

			typename size_vector_type::bufreader_type* lcp_lms_reader = new typename size_vector_type::bufreader_type(*m_lcp_lms);

			fpa_type fp_ival1, fp_ival2;

			alphabet_extension_type ch1, ch2;

			size_type cur_lcp;

			++(*lcp_lms_reader); // skip the leftmost lcp-value

			cur_lcp = *(*lcp_lms_reader);

			fp_ival2 = static_cast<fpa_type>(((*_sorter3)->second - (static_cast<fpb_type>((*_sorter1)->second) * m_rinterval->compute(cur_lcp)) % P + P) % P);

			ch2 = (*_sorter3)->third;

			++(*_sorter3), ++(*_sorter1);

			for (; !_sorter3->empty(); ++(*_sorter3), ++(*_sorter2), ++(*_sorter1)) {

				fp_ival1 = static_cast<fpa_type>(((*_sorter2)->second - (static_cast<fpb_type>((*_sorter1)->second) * m_rinterval->compute(cur_lcp)) % P + P) % P);

				ch1 = (*_sorter2)->third;

				if (fp_ival1 != fp_ival2 || ch1 == ch2) {

					is_right = false;

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

				is_right = false;
			}

			delete lcp_lms_reader; lcp_lms_reader = nullptr;

			return is_right;
		}

		/// \brief core part
		///
		bool run() {

			// step 1: retrieve SA_LMS and LCP_LMS from SA and LCP, respectively.
			retrieve_lms();

			// step 2: fetch fp[0, SA_LMS[i] - 1]
			pair2_less_sorter_1st_type* sorter1 = fetch_fp();

			// step 2: fetch fp[0, SA_LMS[i] + LCP_LMS[i] - 1]
			triple3_less_sorter_1st_type* sorter2 = fetch_fp_ch_cur();

			// step 3: fetch fp[0, SA_LMS[i] + LCP_LMS[i + 1] - 1]
			triple3_less_sorter_1st_type* sorter3 = fetch_fp_ch_pre();

			// step 4: check 
			bool res = check(sorter1, sorter2, sorter3);

			delete sorter1; sorter1 = nullptr;

			delete sorter2; sorter2 = nullptr;

			delete sorter3; sorter3 = nullptr;

			return res;
		}

		/// \brief return m_sa_lms
		///
		size_vector_type* get_sa_lms() {

			return m_sa_lms;
		}

		/// \brief return m_lcp_lms
		///
		size_vector_type* get_lcp_lms() {
		
			return m_lcp_lms;
		}

		~LMSValidate() {
		
			delete m_rinterval; m_rinterval = nullptr;
		}
	};


	/// \brief retrieve preceding items (pre_ch & pre_t) for L-type, S-type and LMS suffixes
	///
	struct RetrievePre {

		/// \brief ctor
		///
		RetrievePre(alphabet_vector_type* _t, size_vector_type* _sa, pair3_less_sorter_1st_type*& _pre_item_of_lms_sorter, triple2_less_sorter_1st_type*& _pre_item_of_l_sorter, triple2_great_sorter_1st_type*& _pre_item_of_s_sorter, BktInfo& _bkt_info) {
	
			// step 1: sort (SA[i], i) by i in descending order 
			pair1_great_sorter_1st_type* pair1_great_sorter = new pair1_great_sorter_1st_type(pair1_great_comparator_1st_type(), MAIN_MEM_AVAIL / 4);
	
			typename size_vector_type::bufreader_type* sa_reader = new typename size_vector_type::bufreader_type(*_sa);

			for (uint64 idx = 1; !sa_reader->empty(); ++idx, ++(*sa_reader)) {
			
				pair1_great_sorter->push(pair1_type(*(*sa_reader), idx));
			}

			delete sa_reader; sa_reader = nullptr;

			pair1_great_sorter->sort();

			// step 2: record pre_t & pre_ch for L-type, S-type and LMS suffixes separately.
			_pre_item_of_lms_sorter = new pair3_less_sorter_1st_type(pair3_less_comparator_1st_type(), MAIN_MEM_AVAIL / 4);

			_pre_item_of_l_sorter = new triple2_less_sorter_1st_type(triple2_less_comparator_1st_type(), MAIN_MEM_AVAIL / 4);

			_pre_item_of_s_sorter = new triple2_great_sorter_1st_type(triple2_great_comparator_1st_type(), MAIN_MEM_AVAIL / 4);

			typename alphabet_vector_type::bufreader_reverse_type* t_rev_reader = new typename alphabet_vector_type::bufreader_reverse_type(*_t);
			
			alphabet_type cur_scanned_ch, last_scanned_ch;
			
			uint8 cur_scanned_type, last_scanned_type;

			// rightmost character is L_TYPE
			cur_scanned_type = L_TYPE, cur_scanned_ch = *(*t_rev_reader);

			++(*t_rev_reader);
			
			last_scanned_type = cur_scanned_type, last_scanned_ch = cur_scanned_ch;

			for (; !t_rev_reader->empty(); ++(*t_rev_reader), ++(*pair1_great_sorter)) {
			
				cur_scanned_ch = *(*t_rev_reader);

				cur_scanned_type = ((cur_scanned_ch < last_scanned_ch) || (cur_scanned_ch == last_scanned_ch && last_scanned_type == S_TYPE)) ? S_TYPE : L_TYPE;

				if (last_scanned_type == L_TYPE) { // push the preceding item of an L-type suffix
				
					_bkt_info.add_l(last_scanned_ch);

					_pre_item_of_l_sorter->push(triple2_type((*pair1_great_sorter)->second, cur_scanned_ch, cur_scanned_type));
				}
				else { // push the preceding item of an S-type suffix

					_bkt_info.add_s(last_scanned_ch);

					_pre_item_of_s_sorter->push(triple2_type((*pair1_great_sorter)->second, cur_scanned_ch, cur_scanned_type));

					if (cur_scanned_type == L_TYPE) { // push the preceding item of an LMS suffix
					
						_bkt_info.add_lms(last_scanned_ch);

						_pre_item_of_lms_sorter->push(pair3_type((*pair1_great_sorter)->second, cur_scanned_ch));
					}
				}

				last_scanned_ch = cur_scanned_ch;

				last_scanned_type = cur_scanned_type;
			}

			// leftmost (last scanned) must not be LMS and we suppose its preceding to be the sentinel 
			cur_scanned_type = SENTINEL_TYPE, cur_scanned_ch = 0;

			if (last_scanned_type == L_TYPE) { // push the preceding item of an L-type suffix 
			
				_bkt_info.add_l(last_scanned_ch);

				_pre_item_of_l_sorter->push(triple2_type((*pair1_great_sorter)->second, cur_scanned_ch, cur_scanned_type));
			}
			else { // push the preceding item of an S-type suffix

				_bkt_info.add_s(last_scanned_ch);

				_pre_item_of_s_sorter->push(triple2_type((*pair1_great_sorter)->second, cur_scanned_ch, cur_scanned_type));
			}
			

			++(*pair1_great_sorter);

			delete pair1_great_sorter; pair1_great_sorter = nullptr;

			delete t_rev_reader; t_rev_reader = nullptr;


			// compute bucket size by summing up L-type ones and S-type ones
			_bkt_info.accumulate();

			// sort preceding items 
			_pre_item_of_s_sorter->sort();

			_pre_item_of_l_sorter->sort();

			_pre_item_of_lms_sorter->sort();
		}
	};


	/// \brief RMQ
	///
	/// Given SA[k] and SA[k + 1] respectively induced from SA[i] and SA[j], the LCP-value of suf(SA[k]) and suf(SA[k + 1])
	/// is determined by LCP[i + 1, j]. 
	/// The response to the range-minimum query for LCP[i + 1, j] returns the minimum value among LCP[i + 1], .., LCP[j - 1], LCP[j].
	struct RMQ {
	private:

		const alphabet_type ch_max;

		const size_type val_max;

		std::vector<size_type> m_rmq;

	public:

		/// \brief ctor
		RMQ() : ch_max(std::numeric_limits<alphabet_type>::max()), val_max(std::numeric_limits<size_type>::max()) {

			m_rmq.resize(ch_max + 1, 0);
		}

		/// \brief getter 
		/// 
		size_type get(const alphabet_type _ch) {

			return m_rmq[_ch];
		}

		/// \brief update
		///
		/// scan all the elements in m_rmq, update m_rmq[ch] as _val if m_rmq[ch] > _val.
		void update(size_type _val) {

			for (alphabet_extension_type ch = 0; ch <= ch_max; ++ch) {

				if (m_rmq[ch] > _val) m_rmq[ch] = _val;
			}

			return;
		}

		/// \brief reset the range minimum value to val_max
		///
		void reset(const alphabet_type _ch) {

			m_rmq[_ch] = val_max;

			return;
		}

	};

	/// \brief scan SA rightward to validate the SA-values of all the L-type suffixes and the LCP-values of them and their left neighbors in SA.
	///
	/// When scanning rightward, compute the LCP-value of currently scanned suffix and the last scanned one and validate the LCP-value of
	/// currently induced suffix and its left neighbor in SA.
	struct RScan {

	private:
		const alphabet_type ch_max; ///< maximum character 

		alphabet_vector_type* m_t; ///< pointer to input string

		size_vector_type* m_sa; ///< pointer to suffix array

		size_vector_type* m_lcp; ///< pointer to LCP array

		size_vector_type* m_sa_lms; ///< pointer to SA_LMS

		size_vector_type* m_lcp_lms; ///< pointer to LCP_LMS

		uint64 m_len; ///< number of elements in T/SA/LCP

		std::vector<uint64> m_l_bkt_toscan; ///< number of L-type suffixes to be scanned in each bucket

		std::vector<uint64> m_l_bkt_scanned; ///< number of L-type suffixes scanned in each bucket

		std::vector<uint64> m_lms_bkt_toscan; ///< number of LMS suffixes to be scanned in each bucket

		std::vector<uint64> m_lms_bkt_scanned; ///< number of LMS suffixes scanned in each bucket

		std::vector<uint64> m_l_bkt_spos; ///< starting position of each L-type bucket in SA

		uint64 m_total_l_toscan; ///< total number of L-type suffixes to be scanned

		uint64 m_total_l_scanned; ///< total number of L-type suffixes already scanned

		uint64 m_total_lms_toscan; ///< total number of LMS suffixes to be scanned

		uint64 m_total_lms_scanned; ///< total number of LMS suffixes already scanned

		alphabet_type m_cur_l_ch; ///< L-type bucket currently being scanned

		alphabet_type m_cur_lms_ch; ///< LMS bucket currently being scanned

		typename size_vector_type::bufreader_type* m_sa_l_reader; ///< for scan, point to the SA-value for currently scanned L-type suffix (retrieve from SA)

		typename size_vector_type::bufreader_type* m_lcp_l_reader; ///< for scan, point to the LCP-value for currently scanned L-type suffix and its left neighbor in SA (retrieve from LCP)

		typename size_vector_type::bufreader_type* m_sa_lms_reader; ///< for scan, point to the SA-value for currently scanned LMS suffix (retrieve from SA_LMS)

		typename size_vector_type::bufreader_type* m_lcp_lms_reader; ///< for scan, point to the LCP-value for currently scanned LMS suffixes and the rightmost LMS one on its leftside (retrieve from LCP_LMS)

		std::vector<typename size_vector_type::bufreader_type*> m_sa_l_bkt_reader; // for induce, point to the SA_value for the L-type suffix next to be induced in each bucket (retrieve from SA)

		std::vector<typename size_vector_type::bufreader_type*> m_lcp_l_bkt_reader; // for induce, point to the LCP_value for the L-type suffix next to be induced in each bucket and its left neighbor in SA (retrieve from LCP)

	public:
		/// \brief ctor
		///
		RScan(BktInfo& _bkt_info, 
			alphabet_vector_type* _t, 
			size_vector_type* _sa, 
			size_vector_type* _lcp, 
			size_vector_type* _sa_lms, 
			size_vector_type* _lcp_lms) :
			ch_max(std::numeric_limits<alphabet_type>::max()), 
			m_t(_t), 
			m_sa(_sa), 
			m_lcp(_lcp), 
			m_sa_lms(_sa_lms), 
			m_lcp_lms(_lcp_lms),
			m_len(m_t->size()) {

			//
			m_total_l_toscan = m_total_l_scanned = 0;

			m_total_lms_toscan = m_total_lms_scanned = 0;

			//
			uint64 spos = 0;

			for (alphabet_extension_type ch = 0; ch <= ch_max; ++ch) {

				m_l_bkt_toscan.push_back(_bkt_info.get_l_bkt_size(ch));

				m_lms_bkt_toscan.push_back(_bkt_info.get_lms_bkt_size(ch));

				m_total_l_toscan += m_l_bkt_toscan[ch];

				m_total_lms_toscan += m_lms_bkt_toscan[ch];

				m_l_bkt_spos.push_back(spos);

				spos += _bkt_info.get_bkt_size(ch); // move to the start pos of next bucket
			}

			m_l_bkt_scanned.resize(ch_max + 1, 0);

			m_lms_bkt_scanned.resize(ch_max + 1, 0);

			//			
			for (alphabet_extension_type ch = 0; ch <= ch_max; ++ch) { // determine the leftmost non-empty L-type bucket

				if (m_l_bkt_toscan[ch] != 0) {

					m_sa_l_reader = new typename size_vector_type::bufreader_type(m_sa->begin() + m_l_bkt_spos[ch], m_sa->begin() + m_l_bkt_spos[ch] + m_l_bkt_toscan[ch]);

					m_lcp_l_reader = new typename size_vector_type::bufreader_type(m_lcp->begin() + m_l_bkt_spos[ch], m_lcp->begin() + m_l_bkt_spos[ch] + m_l_bkt_toscan[ch]);
	
					m_cur_l_ch = static_cast<alphabet_type>(ch); 
					
					break;
				}
			}

			//
			for (alphabet_extension_type ch = 0; ch <= ch_max; ++ch) { // determine the leftmost non-empty LMS bucket

				if (m_lms_bkt_toscan[ch] != 0) { 
					
					m_cur_lms_ch = static_cast<alphabet_type>(ch);

					break; 
				}
			}

			m_sa_lms_reader = new typename size_vector_type::bufreader_type(*m_sa_lms);

			m_lcp_lms_reader = new typename size_vector_type::bufreader_type(*m_lcp_lms);

			//
			m_sa_l_bkt_reader.resize(ch_max + 1);

			m_lcp_l_bkt_reader.resize(ch_max + 1);

			for (alphabet_extension_type ch = 0; ch <= ch_max; ++ch) {

				if (m_l_bkt_toscan[ch] != 0) {

					m_sa_l_bkt_reader[ch] = new typename size_vector_type::bufreader_type(m_sa->begin() + m_l_bkt_spos[ch], m_sa->begin() + m_l_bkt_spos[ch] + m_l_bkt_toscan[ch]);

					m_lcp_l_bkt_reader[ch] = new typename size_vector_type::bufreader_type(m_lcp->begin() + m_l_bkt_spos[ch], m_lcp->begin() + m_l_bkt_spos[ch] + m_l_bkt_toscan[ch]);
				}
				else {

					m_sa_l_bkt_reader[ch] = nullptr;

					m_lcp_l_bkt_reader[ch] = nullptr;
				}
			}
		}

		/// \brief check if no more L-type to be scanned
		///
		bool l_is_empty() {

			return m_total_l_toscan == m_total_l_scanned;
		}

		/// \brief chec if no more LMS to be scanned
		///
		bool lms_is_empty() {

			return m_total_lms_toscan == m_total_lms_scanned;
		}

		/// \brief check if no more L-type suffixes to be scanned in current bucket
		///
		bool l_cur_bkt_is_empty() {

			return m_l_bkt_scanned[m_cur_l_ch] == m_l_bkt_toscan[m_cur_l_ch];
		}

		/// \brief check if no more LMS suffixes to be scanned in current bucket
		///
		bool lms_cur_bkt_is_empty() {

			return m_lms_bkt_scanned[m_cur_lms_ch] == m_lms_bkt_toscan[m_cur_lms_ch];
		}

		/// \brief find next non-empty L-bucket
		///
		/// \note guarantee that there still exist L-type suffixes to be scanned before calling the function
		void find_next_l_bkt() {

			// if finished reading current L-type bucket, then find a non-empty L-type bucket
			// because there exist at least one element to be read, we must have m_l_bkt_scanned[ch] == m_l_bkt_toscan[ch] for some ch <= ch_max
			while (l_cur_bkt_is_empty()) {

				++m_cur_l_ch;
			}

			// move the pointers to the starting positions of the non-empty L-type bucket in SA and LCP
			delete m_sa_l_reader;

			m_sa_l_reader = new typename size_vector_type::bufreader_type(m_sa->begin() + m_l_bkt_spos[m_cur_l_ch], m_sa->begin() + m_l_bkt_spos[m_cur_l_ch] + m_l_bkt_toscan[m_cur_l_ch]);

			delete m_lcp_l_reader;

			m_lcp_l_reader = new typename size_vector_type::bufreader_type(m_lcp->begin() + m_l_bkt_spos[m_cur_l_ch], m_lcp->begin() + m_l_bkt_spos[m_cur_l_ch] + m_l_bkt_toscan[m_cur_l_ch]);

			return;
		}

		/// \brief find next non-empty LMS bucket
		///
		/// \note guarantee that there still exist LMS suffixes to be scanned before calling the function
		void find_next_lms_bkt() {

			// if finished reading current LMS bucket, then find a non-empty LMS bucket
			// because there exist at least one element to be read, we must have m_lms_bkt_scanned[ch] == m_lms_bkt_toscan[ch] for some ch <= ch_max
			while (lms_cur_bkt_is_empty()) {

				++m_cur_lms_ch;
			}

			// no need to relocate pointers to SA_LMS and LCP_LMS

			return;
		}

		/// \brief fetch the SA-value of currently scanned L-type suffix and the LCP-value of the suffix and its left neighbor in SA
		///
		/// retrieve SA-value and LCP-value from SA and LCP
		/// \note before calling the function, check to determine there remains elements to read
		void fetch_l_scanned(size_type& _sa_value, size_type& _lcp_value) {

			_sa_value = *(*m_sa_l_reader), ++(*m_sa_l_reader);

			_lcp_value = *(*m_lcp_l_reader), ++(*m_lcp_l_reader);

			++m_l_bkt_scanned[m_cur_l_ch];

			++m_total_l_scanned;

			return;
		}


		/// \brief fetch the SA-value of currently scanned LMS suffix and the LCP-value of the suffix and its left neighbor in SA_LMS
		///
		/// retrieve SA-value and LCP-value from SA_LMS and LCP_LMS
		/// \note before calling the function, check to determine there remains elements to read
		void fetch_lms_scanned(size_type& _sa_value, size_type& _lcp_value) {

			_sa_value = *(*m_sa_lms_reader), ++(*m_sa_lms_reader);

			_lcp_value = *(*m_lcp_lms_reader), ++(*m_lcp_lms_reader);

			++m_lms_bkt_scanned[m_cur_lms_ch];

			++m_total_lms_scanned;

			return;
		}

		/// \brief fetch the SA-value for currently induced L-type suffix and the LCP-value for the suffix and its left neighbor in SA
		/// 
		/// retrieve the values from SA and LCP
		void fetch_l_induced(const alphabet_type _ch, size_type& _sa_value, size_type& _lcp_value) {

			_sa_value = *(*m_sa_l_bkt_reader[_ch]), ++(*m_sa_l_bkt_reader[_ch]);

			_lcp_value = *(*m_lcp_l_bkt_reader[_ch]), ++(*m_lcp_l_bkt_reader[_ch]);

			return;
		}

		/// \brief return the initial character of suffixes in current scanned bucket
		///
		alphabet_type get_l_bkt_ch() {

			return m_cur_l_ch;
		}

		/// \brief return the initial character of suffixes in current scanned bucket
		//
		alphabet_type get_lms_bkt_ch() {

			return m_cur_lms_ch;
		}

		/// \brief literally compare two suffixes pointed to by the starting positions 
		///
		uint64 bruteforce_compute_lcp(const size_type& _pos1, const size_type& _pos2) {

			typename alphabet_vector_type::const_iterator it1(m_t->begin() + _pos1), it2(m_t->begin() + _pos2);

			uint64 lcp = 0;

			if (_pos1 < _pos2) {

				for (; it2 != m_t->end(); ++it2, ++it1) {

					if (*it1 != *it2) break;

					++lcp;
				}
			}
			else {

				for (; it1 != m_t->end(); ++it1, ++it2) {

					if (*it1 != *it2) break;

					++lcp;
				}
			}

			return lcp;
		}


		/// \brief de-ctor
		///
		~RScan() {

			delete m_sa_l_reader; m_sa_l_reader = nullptr;

			delete m_lcp_l_reader; m_lcp_l_reader = nullptr;

			delete m_sa_lms_reader; m_sa_lms_reader = nullptr;

			delete m_lcp_lms_reader; m_lcp_lms_reader = nullptr;

			for (alphabet_extension_type ch = 0; ch <= ch_max; ++ch) {

				delete m_sa_l_bkt_reader[ch]; m_sa_l_bkt_reader[ch] = nullptr;

				delete m_lcp_l_bkt_reader[ch]; m_lcp_l_bkt_reader[ch] = nullptr;
			}
		}

		/// \brief induce and check the order of L-type suffixes and their LCP values 
		///
		/// The SA-value of currently scanned suffix (no matter L-type or LMS) can be directly retrieved from SA or SA_LMS. 
		/// The question is how to retrieve the LCP-value of the suffix and the last scanned one (may not be the neighbor in SA). 
		///	case 1: if currently scanned suffix is L-type, then retrieve the LCP-value following one of the two sub-cases as below:
		///         sub-case (a): if the suffix is the leftmost in the L-type bucket, then perform a literal comparison between the suffix and the last scanned one to obtain their LCP-value.
		///	    sub-case (b): otherwise, the last scanned is also the left neighbor in SA, retrieve the LCP-value from LCP.
		///	case 2: if currently scanned suffix is LMS, then retrieve the LCP-value following one of the two sub-cases as below:
		///	    sub-case (a): if the suffix is the leftmost in the LMS bucket and the last scanned is L-type, then perform a literal comparison between the suffix and the last scanned to obtain their LCP-value.
		///	    sub-case (b): otherwise, the last scanned is also the left neighbor in SA_LMS, retrieve the LCP-value from LCP_LMS.
		/// The SA-value of currently induced L-type suffix can be directly retrieved from SA or SA_LMS.
		/// The question is how to retrieve the LCP-value of the suffix and the last induced.
		///	case 1: if currently induced suffix is the leftmost in the L-type bucket, then do nothing.
		///	case 2: otherwise, the LCP-value is 1 + RMQ_VALUE. 
		bool run(triple2_less_sorter_1st_type* _pre_item_of_l_sorter, pair3_less_sorter_1st_type* _pre_item_of_lms_sorter) {

			alphabet_type cur_bkt_ch, pre_ch;

			uint8 pre_t;

			size_type sv_cur_scanned, lv_cur_scanned, sv_last_scanned, lv_last_scanned; // scanned SA-value & LCP-value

			size_type sv_induced, lv_induced, sv_induced_fetch, lv_induced_fetch; // induced SA-value & LCP-value

			RMQ rmq_l;

			bool flag = true; // indicate whether or not the scanned suffix is the leftmost in the L-type/LMS bucket

			bool flags[ch_max + 1]; // whether or not the induced suffix is the leftmost in the L-type bucket

			for (alphabet_extension_type ch = 0; ch <= ch_max; ++ch) flags[ch] = true;

			{ // process the virtual sentinel

				// step 1: retrieve SA-value and LCP-value
				sv_cur_scanned = m_len; // assumed to be m_len

				lv_cur_scanned = 0; // no left neighbor in SA

				// step 2: update RMQ
				rmq_l.update(lv_cur_scanned); // actually, no need

				// step 3: induce and validate the SA-value and LCP-value of the preceding suffix in T
				auto rit = m_t->rbegin();

				pre_ch = *rit, pre_t = L_TYPE; // the rightmost character must be L-type
		
				fetch_l_induced(pre_ch, sv_induced_fetch, lv_induced_fetch); // retrieve the induced value from SA & LCP 

				sv_induced = sv_cur_scanned - 1; // induce SA-value from currently scanned

				if (sv_induced_fetch != sv_induced) { // validate induced SA-value
					
					std::cerr << "SA-value is not correct\n";

					return false;
				}

				// the induced suffix is the leftmost in the L-type bucket, skip computing & validating the LCP-value
				flags[pre_ch] = false;
			
				rmq_l.reset(pre_ch); // reset rmq

				// step 5: iterates
				sv_last_scanned = sv_cur_scanned;

				lv_last_scanned = lv_cur_scanned;
			}

			{ // process the remaing

				// determine next bucket to scan
				if (l_is_empty()) { // there must exist at least one L-type suffix (induced from the virtual sentinel)
				
					std::cerr << "there must exist at least one L-type suffix, but no available\n";

					return false;
				}

				cur_bkt_ch = get_l_bkt_ch();

				if (!lms_is_empty() && get_lms_bkt_ch() < cur_bkt_ch) {

					cur_bkt_ch = get_lms_bkt_ch();
				}

				// scan buckets in sequence
				while (true) {

					// scan the L-type bucket
					while (!l_cur_bkt_is_empty() && get_l_bkt_ch() == cur_bkt_ch) {

						// step 1: retrieve SA-value & LCP-value from SA & LCP
						fetch_l_scanned(sv_cur_scanned, lv_cur_scanned);

						if (flag == true) {// leftmost in the L-type bucket, can not use the value from LCP

							if (m_len == sv_last_scanned) { // last scanned suffix is the sentinel
							
								lv_cur_scanned = 0;
							}
							else { // bruteforce, actually must be 0 if correct

								lv_cur_scanned = bruteforce_compute_lcp(sv_last_scanned, sv_cur_scanned);
							}

							flag = false;
						}

						// step 2: update RMQ
						rmq_l.update(lv_cur_scanned);

						// step 3: induce and validate
						pre_ch = (*_pre_item_of_l_sorter)->second;

						pre_t = (*_pre_item_of_l_sorter)->third;

						++(*_pre_item_of_l_sorter);

						if (pre_t == L_TYPE) {

							fetch_l_induced(pre_ch, sv_induced_fetch, lv_induced_fetch);

							sv_induced = sv_cur_scanned - 1;

							if (sv_induced != sv_induced_fetch) {

								std::cerr << "SA-value is wrong\n";

								return false;
							}

							if (flags[pre_ch] == true) { // leftmost
								
								// skip computing & validating the LCP-value
								flags[pre_ch] = false;
							}
							else {

								lv_induced = rmq_l.get(pre_ch) + 1;

								if (lv_induced != lv_induced_fetch) {

									std::cerr << "LCP-value is wrong\n";

									return false;
								}
							}

							rmq_l.reset(pre_ch); // reset rmq
						}

						// iterates
						sv_last_scanned = sv_cur_scanned;

						lv_last_scanned = lv_cur_scanned;
					}

					flag = true;

					// scan S-type bucket
					while (!lms_cur_bkt_is_empty() && get_lms_bkt_ch() == cur_bkt_ch) {

						// step 1: retrieve SA-value & LCP-value from SA_LMS & LCP_LMS
						fetch_lms_scanned(sv_cur_scanned, lv_cur_scanned);

						if (flag == true) { // leftmost in the LMS bucket, cannot use the value from LCP_LMS

							if (m_len == sv_last_scanned) { // left neighbor is sentinel

								lv_cur_scanned = 0;
							}
							else {

								lv_cur_scanned = bruteforce_compute_lcp(sv_last_scanned, sv_cur_scanned);
							}

							flag = false;
						}

						// step 2: update RMQ
						rmq_l.update(lv_cur_scanned);

						// step 3: the preceding must be L-type, induce and validate
						pre_ch = (*_pre_item_of_lms_sorter)->second;

						++(*_pre_item_of_lms_sorter);

						fetch_l_induced(pre_ch, sv_induced_fetch, lv_induced_fetch);

						sv_induced = sv_cur_scanned - 1;
	
						if (sv_induced != sv_induced_fetch) {
	
							std::cerr << "SA-value is wrong\n";

							return false;
						}

						if (flags[pre_ch] == true) { // leftmost 

							// skip computing the LCP-value
							flags[pre_ch] = false;
						}
						else {

							lv_induced = rmq_l.get(pre_ch) + 1;

							if (lv_induced != lv_induced_fetch) {

								std::cerr << "LCP-value is wrong\n";

								return false;
							}
						}

						// reset RMQ
						rmq_l.reset(pre_ch);

						// iterates
						sv_last_scanned = sv_cur_scanned;

						lv_last_scanned = lv_cur_scanned;
					}

					flag = true;

					// determine the initial character of the next bucket to scan
					if (!l_is_empty()) {

						find_next_l_bkt();

						cur_bkt_ch = get_l_bkt_ch();

						if (!lms_is_empty()) {

							find_next_lms_bkt();

							if (cur_bkt_ch > get_lms_bkt_ch()) {

								cur_bkt_ch = get_lms_bkt_ch();
							}
						}
					}
					else if (!lms_is_empty()) {

						find_next_lms_bkt();

						cur_bkt_ch = get_lms_bkt_ch();
					}
					else {

						break;
					}
				}
			}

			assert(_pre_item_of_l_sorter->size() == 0);

			assert(_pre_item_of_lms_sorter->size() == 0);

			return true;
		}
	};


	/// \brief scan SA & LCP rightward to validate the SA-values of all the S-type suffixes and the LCP-values of them and their left neighbor in SA.
	///
	struct LScan {

	private:

		const alphabet_type ch_max; ///< maximum character

		alphabet_vector_type* m_t; ///< pointer to the input string

		size_vector_type* m_sa; ///< pointer to the suffix array

		size_vector_type* m_lcp; ///< pointer to the LCP array

		std::vector<uint64> m_s_bkt_toscan; ///< number of S-type suffixes to be scanned in each bucket

		std::vector<uint64> m_s_bkt_scanned; ///< number of S-type suffixes scanned in each bucket

		std::vector<uint64> m_l_bkt_toscan; ///< number of L-type suffixes to be scanned in each bucket

		std::vector<uint64> m_l_bkt_scanned; ///< numer of L-type suffixes scanned in each bucket

		std::vector<uint64> m_s_bkt_spos; ///< indicate the starting position of each L-type bucket in SA (right boundary)

		std::vector<uint64> m_l_bkt_spos; ///< indicate the starting position of each S-type bucket in SA (right boundary)

		uint64 m_total_s_toscan; ///< total number of S-type suffixes to be scanned

		uint64 m_total_s_scanned; ///< total number of S-type suffixes already scanned

		uint64 m_total_l_toscan; ///< total number of L-type suffixes to be scanned

		uint64 m_total_l_scanned; ///< total number of L-type suffixes already scanned

		alphabet_type m_cur_s_ch; ///< S-type bucket currently being scanned

		alphabet_type m_cur_l_ch; ///< L-type bucket currently being scanned

		typename size_vector_type::bufreader_reverse_type* m_sa_rev_reader; ///< for scan, point to the SA-value for currently scanned suffix (retrieve from SA, leftward)

		typename size_vector_type::bufreader_reverse_type* m_lcp_rev_reader; ///< for scan, point to the LCP-value for currently scanned suffix and its right neighbor in SA (retrieve from LCP, leftward)

		std::vector<typename size_vector_type::bufreader_reverse_type*> m_sa_s_bkt_rev_reader; ///< for induce, point to the SA-value for the S-type suffix to be induced in each bucket (retrievd from SA, leftward)

		std::vector<typename size_vector_type::bufreader_reverse_type*> m_lcp_s_bkt_rev_reader; ///< for induce, point to the LCP-value for the S-type suffix to be induced in each bucket (retrieved from LCP, leftward)

	public:	
		/// \brief ctor
		///
		LScan(BktInfo& _bkt_info, 
			alphabet_vector_type* _t, 
			size_vector_type* _sa, 
			size_vector_type* _lcp) :
			ch_max(std::numeric_limits<alphabet_type>::max()), 
			m_t(_t), 
			m_sa(_sa), 
			m_lcp(_lcp) {

			//
			m_total_s_toscan = m_total_s_scanned = 0;

			m_total_l_toscan = m_total_l_scanned = 0;

			//
			m_s_bkt_toscan.resize(ch_max + 1, 0);
	
			m_s_bkt_scanned.resize(ch_max + 1, 0);

			m_l_bkt_toscan.resize(ch_max + 1, 0);

			m_l_bkt_scanned.resize(ch_max + 1, 0);

			m_s_bkt_spos.resize(ch_max + 1, 0);

			m_l_bkt_spos.resize(ch_max + 1, 0);

			uint64 spos = 0;

			for (alphabet_extension_type ch = 0; ch <= ch_max; ++ch) {

				m_s_bkt_toscan[ch] = _bkt_info.get_s_bkt_size(ch);

				m_l_bkt_toscan[ch] = _bkt_info.get_l_bkt_size(ch);

				m_total_s_toscan += m_s_bkt_toscan[ch];

				m_total_l_toscan += m_l_bkt_toscan[ch];

				m_l_bkt_spos[ch] = spos;

				m_s_bkt_spos[ch] = spos + m_l_bkt_toscan[ch];

				spos += _bkt_info.get_bkt_size(ch);				
			}

			//
			for (alphabet_extension_type ch = ch_max; ch >= 0; --ch) { // find rightmost non-empty S-type bucket

				if (m_s_bkt_toscan[ch] != 0) { 
					
					m_cur_s_ch = static_cast<alphabet_type>(ch); 
					
					break; 
				}

				if (ch == 0) break;
			}

			// determine rightmost non-empty L-type bucket
			for (alphabet_extension_type ch = ch_max; ch >= 0; --ch) { // find rightmost non-empty L-type bucket

				if (m_l_bkt_toscan[ch] != 0) { 
					
					m_cur_l_ch = static_cast<alphabet_type>(ch);

					break; 
				}

				if (ch == 0) break;
			}

			//
			m_sa_rev_reader = new typename size_vector_type::bufreader_reverse_type(*m_sa);

			m_lcp_rev_reader = new typename size_vector_type::bufreader_reverse_type(*m_lcp);

			m_sa_s_bkt_rev_reader.resize(ch_max + 1);

			m_lcp_s_bkt_rev_reader.resize(ch_max + 1);

			for (alphabet_extension_type ch = 0; ch <= ch_max; ++ch) {

				if (m_s_bkt_toscan[ch] != 0) {

					m_sa_s_bkt_rev_reader[ch] = new typename size_vector_type::bufreader_reverse_type(m_sa->begin() + m_s_bkt_spos[ch], m_sa->begin() + m_s_bkt_spos[ch] + m_s_bkt_toscan[ch]);

					m_lcp_s_bkt_rev_reader[ch] = new typename size_vector_type::bufreader_reverse_type(m_lcp->begin() + m_s_bkt_spos[ch], m_lcp->begin() + m_s_bkt_spos[ch] + m_s_bkt_toscan[ch]);
				}
				else {

					m_sa_s_bkt_rev_reader[ch] = nullptr;

					m_lcp_s_bkt_rev_reader[ch] = nullptr;
				}
			}

		}

		/// \brief check if no more LMS to be scanned
		bool s_is_empty() {

			return m_total_s_toscan == m_total_s_scanned;
		}

		/// \brief check if no more L-type to be scanned
		///
		bool l_is_empty() {

			return m_total_l_toscan == m_total_l_scanned;
		}

		/// \brief check if no more S-type suffixes to be scanned in current bucket
		///
		bool s_cur_bkt_is_empty() {

			return m_s_bkt_scanned[m_cur_s_ch] == m_s_bkt_toscan[m_cur_s_ch];
		}

		/// \brief check if no more L-type suffixes to be scanned in current bucket
		///
		bool l_cur_bkt_is_empty() {

			return m_l_bkt_scanned[m_cur_l_ch] == m_l_bkt_toscan[m_cur_l_ch];
		}

		/// \brief find next non-empty S-bucket
		///
		/// \note guarantee there remains S-type suffixes to be scanned before calling the function
		void find_next_s_bkt() {

			while (s_cur_bkt_is_empty()) --m_cur_s_ch;

			return;
		}

		/// \brief find next non-empty L-bucket
		///
		/// \note guarantee there remains L-type suffixes to be scanned before calling the function
		void find_next_l_bkt() {

			while (l_cur_bkt_is_empty()) --m_cur_l_ch;

			return;
		}

		/// \brief fetch the SA-value for currently scanned S-type suffix and the LCP-value of the suffix and its right neighbor in SA
		///
		void fetch_s_scanned(size_type& _sa_value, size_type& _lcp_value) {

			_sa_value = *(*m_sa_rev_reader), ++(*m_sa_rev_reader);

			_lcp_value = *(*m_lcp_rev_reader), ++(*m_lcp_rev_reader);

			++m_s_bkt_scanned[m_cur_s_ch];

			++m_total_s_scanned;

			return;
		}


		/// \brief fetch the SA-value for currently scanned L-type suffix and the LCP-value of the suffix and its right neighbor in SA
		///
		void fetch_l_scanned(size_type& _sa_value, size_type& _lcp_value) {

			_sa_value = *(*m_sa_rev_reader), ++(*m_sa_rev_reader);

			_lcp_value = *(*m_lcp_rev_reader), ++(*m_lcp_rev_reader);

			++m_l_bkt_scanned[m_cur_l_ch];

			++m_total_l_scanned;

			return;
		}

		/// \brief fetch the SA-value for currently induced S-type suffix and the LCP-value for the suffix and its right neighbor in SA 
		///
		void fetch_s_induced(const alphabet_type _ch, size_type& _sa_value, size_type& _lcp_value) {

			_sa_value = *(*m_sa_s_bkt_rev_reader[_ch]), ++(*m_sa_s_bkt_rev_reader[_ch]);

			_lcp_value = *(*m_lcp_s_bkt_rev_reader[_ch]), ++(*m_lcp_s_bkt_rev_reader[_ch]);

			return;
		}

		/// \brief get ch for the S-type bucket currently being scanned
		///
		alphabet_type get_s_bkt_ch() {

			return m_cur_s_ch;
		}

		/// \brief get ch for the LMS bucket currently being scanned
		///
		alphabet_type get_l_bkt_ch() {

			return m_cur_l_ch;
		}

		/// \brief literally compare two suffixes pointed to by the starting positions 
		///
		uint64 bruteforce_compute_lcp(const size_type& _pos1, const size_type& _pos2) {

			typename alphabet_vector_type::const_iterator it1(m_t->begin() + _pos1), it2(m_t->begin() + _pos2);

			uint64 lcp = 0;

			if (_pos1 < _pos2) {

				for (; it2 != m_t->end(); ++it2, ++it1) {

					if (*it1 != *it2) break;

					++lcp;
				}
			}
			else {

				for (; it1 != m_t->end(); ++it1, ++it2) {

					if (*it1 != *it2) break;

					++lcp;
				}
			}

			return lcp;
		}

		/// \brief dtor
		///
		~LScan() {

			delete m_sa_rev_reader; m_sa_rev_reader = nullptr;

			delete m_lcp_rev_reader; m_lcp_rev_reader = nullptr;

			for (alphabet_extension_type ch = 0; ch <= ch_max; ++ch) {

				delete m_sa_s_bkt_rev_reader[ch]; m_sa_s_bkt_rev_reader[ch] = nullptr;

				delete m_lcp_s_bkt_rev_reader[ch]; m_lcp_s_bkt_rev_reader[ch] = nullptr;
			}
		}

		/// \brief induce and check the order of S-type suffixes and their LCP-values
		///
		bool run(triple2_great_sorter_1st_type* _pre_item_of_s_sorter, pair4_vector_type* _pre_item_of_l_vector) {

			alphabet_type cur_bkt_ch, pre_ch;

			uint8 pre_t;

			size_type sv_cur_scanned, lv_cur_scanned, sv_last_scanned = 0, lv_last_scanned = 0;

			size_type sv_induced, lv_induced, sv_induced_fetch, lv_induced_fetch;

			RMQ rmq_s;

			bool is_rightmost = true; // indicate currently scanned is the rightmost in SA

			bool flag = true; // indicate currently scanned is the rightmost in the S-type/L-type bucket

			bool flags[ch_max + 1]; // indicate currently induced S-type suffix is the rightmost in the S-type bucket

			for (alphabet_extension_type ch = 0; ch <= ch_max; ++ch) flags[ch] = true;

			size_type last_lv_induced_fetch[ch_max + 1]; // record the LCP-value of last induced S-type suffix in the bucket

			for (alphabet_extension_type ch = 0; ch <= ch_max; ++ch) last_lv_induced_fetch[ch] = 0;

			typename pair4_vector_type::bufreader_reverse_type* pre_item_of_l_rev_reader = new typename pair4_vector_type::bufreader_reverse_type(*_pre_item_of_l_vector);

			// largest suffix must be L-type
			if (get_l_bkt_ch() <= get_s_bkt_ch()) {

				std::cerr << "the largest suffix must be L-type.\n";

				return false;
			}

			cur_bkt_ch = get_l_bkt_ch();

			while (true) {

				// scan S-type bucket
				while (!s_cur_bkt_is_empty() && get_s_bkt_ch() == cur_bkt_ch) {
			
					// step 1: retrieve SA-value and LCP-value
					fetch_s_scanned(sv_cur_scanned, lv_cur_scanned);

					if (flag == true) { // rightmost in the S-type bucket
			
						// check lv_last scanned, must be 0 if correct
						if (bruteforce_compute_lcp(sv_cur_scanned, sv_last_scanned) != lv_last_scanned) { 

							std::cerr << "LCP-value is wrong\n";

							return false;
						}

						flag = false;
					}

					// step 2: induce & validate
					pre_ch = (*_pre_item_of_s_sorter)->second;

					pre_t = (*_pre_item_of_s_sorter)->third;

					++(*_pre_item_of_s_sorter);

					if (pre_t == S_TYPE) {

						fetch_s_induced(pre_ch, sv_induced_fetch, lv_induced_fetch);

						sv_induced = sv_cur_scanned - 1;

						if (sv_induced != sv_induced_fetch) {

							std::cerr << "SA-value is wrong\n";

							return false;
						}
						
						if (flags[pre_ch] == true) { // rightmost in the S-type bucket

							// do not validate LCP-value here, cache the LCP-value
							flags[pre_ch] = false; 
						}
						else { // now the S-type bucket has at least two suffixes

							// check the LCP-value of the two S-type suffixes induced into the bucket
							lv_induced = rmq_s.get(pre_ch) + 1; 

							if (lv_induced != last_lv_induced_fetch[pre_ch]) {

								std::cerr << "LCP-value is wrong\n";

								return false;
							}
						}
						
						last_lv_induced_fetch[pre_ch] = lv_induced_fetch;

						// reset rmq
						rmq_s.reset(pre_ch);
					}		

					// step 3: update rmq
					rmq_s.update(lv_cur_scanned);
		
					//
					sv_last_scanned = sv_cur_scanned;

					lv_last_scanned = lv_cur_scanned;	
				}
		
				flag = true;
	
				// scan L-type bucket
				while (!l_cur_bkt_is_empty() && get_l_bkt_ch() == cur_bkt_ch) {

					// step 1: retrieve SA-value and LCP-value
					fetch_l_scanned(sv_cur_scanned, lv_cur_scanned);
	
					if (flag == true) { // rightmost in the L-type suffix
			
						if (is_rightmost == true) { // no last scanned suffix

							is_rightmost = false;
						}
						else {

							// check the correctness of lv_last_scanned
							if (bruteforce_compute_lcp(sv_cur_scanned, sv_last_scanned) != lv_last_scanned) {
	
								std::cerr << "LCP-value is wrong\n";
	
								return false;
							}
						}

						flag = false;
					}

					// step 2: induce & validate
					pre_ch = (*pre_item_of_l_rev_reader)->first;

					pre_t = (*pre_item_of_l_rev_reader)->second;

					++(*pre_item_of_l_rev_reader);

					if (pre_t == S_TYPE) {

						fetch_s_induced(pre_ch, sv_induced_fetch, lv_induced_fetch);

						sv_induced = sv_cur_scanned - 1;

						if (sv_induced != sv_induced_fetch) {

							std::cerr << "SA-value is wrong\n";

							return false;
						}

						if (flags[pre_ch] == true) { // rightmost in the S-type bucket
				
							flags[pre_ch] = false;
						}
						else {

							// check the LCP-value of currently induced suffix and the last suffix induced
							// into the same S-type bucket
							lv_induced = rmq_s.get(pre_ch) + 1; 

							if (lv_induced != last_lv_induced_fetch[pre_ch]) {

								std::cerr << "LCP-value is wrong\n";

								return false;
							}
						}

						last_lv_induced_fetch[pre_ch] = lv_induced_fetch;	

						// reset rmq
						rmq_s.reset(pre_ch);
					}

					// step 3: update rmq
					rmq_s.update(lv_cur_scanned);

					//
					sv_last_scanned = sv_cur_scanned;

					lv_last_scanned = lv_cur_scanned;
				}
			
				flag = true;

				// determine the next bucket
				if (!s_is_empty()) {

					find_next_s_bkt();

					cur_bkt_ch = get_s_bkt_ch();

					if (!l_is_empty()) {

						find_next_l_bkt();

						if (cur_bkt_ch < get_l_bkt_ch()) {

							cur_bkt_ch = get_l_bkt_ch();
						}
					}
				}
				else if (!l_is_empty()) {

					find_next_l_bkt();

					cur_bkt_ch = get_l_bkt_ch();
				}
				else {

					break;
				}
			}

			delete pre_item_of_l_rev_reader; pre_item_of_l_rev_reader = nullptr;

			return true;
		}

	};

private:
	
	stxxl::syscall_file* m_t_file;

	stxxl::syscall_file* m_sa_file;

	stxxl::syscall_file* m_lcp_file;
	
	alphabet_vector_type* m_t;

	size_vector_type* m_sa;

	size_vector_type* m_lcp;

	uint64 m_len;

#ifdef TEST_VALIDATE4
	Test<alphabet_type, alphabet_extension_type, size_type>* test;
#endif

public:	
	/// \brief ctor
	///
	Validate4(const std::string& _t_fn, const std::string& _sa_fn, const std::string& _lcp_fn){

		m_t_file = new stxxl::syscall_file(_t_fn, stxxl::syscall_file::RDWR | stxxl::syscall_file::DIRECT);

		m_t = new alphabet_vector_type(m_t_file);

		m_sa_file = new stxxl::syscall_file(_sa_fn, stxxl::syscall_file::RDWR | stxxl::syscall_file::DIRECT);

		m_sa = new size_vector_type(m_sa_file);

		m_lcp_file = new stxxl::syscall_file(_lcp_fn, stxxl::syscall_file::RDWR | stxxl::syscall_file::DIRECT);

		m_lcp = new size_vector_type(m_lcp_file);		

#ifdef TEST_VALIDATE4
		test = new Test<alphabet_type, alphabet_extension_type, size_type>(m_t, m_sa, m_lcp);
#endif
	}

	bool run() {

		size_vector_type* sa_lms = nullptr, *lcp_lms = nullptr;

		{ // generate and validate SA_LMS and LCP_LMS

			LMSValidate lms_validate(m_t, m_sa, m_lcp);

			if (false == lms_validate.run()) {
	
				std::cerr << "SA_LMS & LCP_LMS are wrong.\n";

				exit(0);
			}
			else {

				std::cerr << "SA_LMS & LCP_LMS are right.\n";
			}

			sa_lms = lms_validate.get_sa_lms();

			lcp_lms = lms_validate.get_lcp_lms();
		}

#ifdef TEST_VALIDATE4
		std::cerr << "sa_lms_size: " << sa_lms->size() << " lcp_lms_size: " << lcp_lms->size() << std::endl;

		if (false == test->validate_lms(sa_lms, lcp_lms)) {

			std::cerr << "TEST::SA_LMS & LCP_LMS is wrong.\n";

			exit(0);
		}
		else {

			std::cerr << "TEST::SA_LMS & LCP_LMS is right.\n";
		}		
#endif

		triple2_less_sorter_1st_type *pre_item_of_l_sorter = nullptr;

		triple2_great_sorter_1st_type *pre_item_of_s_sorter = nullptr;

		pair3_less_sorter_1st_type *pre_item_of_lms_sorter = nullptr;

		{ // generate preceding items

			RetrievePre retrieve_pre(m_t, m_sa, pre_item_of_lms_sorter, pre_item_of_l_sorter, pre_item_of_s_sorter, m_bkt_info);
		}


		m_bkt_info.display();

#ifdef TEST_VALIDATE4

		std::cerr << "pre_item_size_of_l_sorter: " << pre_item_of_l_sorter->size() << " pre_item_size_of_s_sorter: " << pre_item_of_s_sorter->size() << " pre_item_size_of_lms_sorter: " << pre_item_of_lms_sorter->size() << std::endl;


		if (false == test->validate_pre_item(pre_item_of_l_sorter, pre_item_of_s_sorter, pre_item_of_lms_sorter)) {

			std::cerr << "Test::pre_item is wrong. \n";

			exit(0);
		}
		else {

			std::cerr << "Test::pre_item is right.\n";
		}
#endif

		{ // validate SA-values for L-type suffixes and LCP-values for these suffixes and their left neighbor in SA
	
			RScan r_scan(m_bkt_info, m_t, m_sa, m_lcp, sa_lms, lcp_lms);

			if (false == r_scan.run(pre_item_of_l_sorter, pre_item_of_lms_sorter)) {

				std::cerr << "RScan is wrong\n";

				exit(0);
			}
			else {

				std::cerr << "RScan is right\n";
			}

			delete sa_lms; sa_lms = nullptr;

			delete lcp_lms; lcp_lms = nullptr;

			delete pre_item_of_lms_sorter; pre_item_of_lms_sorter = nullptr;
		}
	
		{ // validate SA-values for S-type suffixes and LCP-values for these suffixes and their right neighbor in SA

			// redirect preceding items of L-type suffixes to an external memory vector
			pre_item_of_l_sorter->sort(); // resort

			pair4_vector_type* pre_item_of_l_vector = new pair4_vector_type(); // only record pre_ch & pre_t

			pre_item_of_l_vector->resize(pre_item_of_l_sorter->size());

			typename pair4_vector_type::bufwriter_type* pre_item_of_l_writer = new typename pair4_vector_type::bufwriter_type(*pre_item_of_l_vector);

			for (; !pre_item_of_l_sorter->empty(); ++(*pre_item_of_l_sorter)) {

				const triple2_type& tuple = *(*pre_item_of_l_sorter);

				(*pre_item_of_l_writer) << pair4_type(tuple.second, tuple.third);
			}

			(*pre_item_of_l_writer).finish();

			delete pre_item_of_l_writer; pre_item_of_l_writer = nullptr;

			delete pre_item_of_l_sorter; pre_item_of_l_sorter = nullptr;

			//
			LScan l_scan(m_bkt_info, m_t, m_sa, m_lcp);

			if (false == l_scan.run(pre_item_of_s_sorter, pre_item_of_l_vector)) {

				std::cerr << "LScan is wrong\n";

				exit(0);
			}
			else {

				std::cerr << "LScan is right\n";
			}

			delete pre_item_of_l_vector; pre_item_of_l_vector = nullptr;

			delete pre_item_of_s_sorter; pre_item_of_s_sorter = nullptr;
		}

		return true;
	}

	~Validate4() {

		delete m_t; m_t = nullptr;

		delete m_t_file; m_t_file = nullptr;

		delete m_sa; m_sa = nullptr;

		delete m_sa_file; m_sa_file = nullptr;

		delete m_lcp; m_lcp = nullptr;

		delete m_lcp_file; m_lcp_file = nullptr;
	}
};

#endif // VALIDATE4_H
