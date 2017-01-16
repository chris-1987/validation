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

#define TEST_VALIDATE4 // for test only, comment out the line if not required



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
	typedef pair<size_type, size_type> pair1_type;

	typedef tuple_less_comparator_1st<pair1_type> pair1_less_comparator_1st_type; // compare by 1st component in ascending order

	typedef typename ExTupleSorter<pair1_type, pair1_less_comparator_1st_type>::sorter pair1_less_sorter_1st_type;

	typedef tuple_great_comparator_1st<pair1_type> pair1_great_comparator_1st_type; // compare by 1st component in descending order

	typedef typename ExTupleSorter<pair1_type, pair1_great_comparator_1st_type>::sorter pair1_great_sorter_1st_type;

	typedef tuple_less_comparator_2nd<pair1_type> pair1_less_comparator_2nd_type; // compare by (1st, 2nd) components in ascending order

	typedef typename ExTupleSorter<pair1_type, pair1_less_comparator_2nd_type>::sorter pair1_less_sorter_2nd_type;

	typedef pair<size_type, fpa_type> pair2_type;

	typedef tuple_less_comparator_1st<pair2_type> pair2_less_comparator_1st_type; // compare by 1st component in ascending order

	typedef typename ExTupleSorter<pair2_type, pair2_less_comparator_1st_type>::sorter pair2_less_sorter_1st_type;

	typedef pair<size_type, uint8> pair3_type;

	typedef tuple_less_comparator_1st<pair3_type> pair3_less_comparator_1st_type; // compare by 1st component in ascending order

	typedef typename ExTupleSorter<pair3_type, pair3_less_comparator_1st_type>::sorter pair3_less_sorter_1st_type;

	typedef triple<size_type, fpa_type, alphabet_extension_type> triple1_type;

	typedef tuple_less_comparator_1st<triple1_type> triple1_less_comparator_1st_type; // compare by 1st component in ascending order

	typedef ExTupleSorter<triple1_type, triple1_less_comparator_1st_type>::sorter triple1_less_sorter_1st_type;

	typedef triple<size_type, uint8, uint8> triple2_type;

	typedef tuple_less_comparator_1st<triple2_type> triple2_less_comparator_1st_type; // compare by first component in ascending order

	typedef ExTupleSorter<triple2_type, triple2_less_comparator_1st_type>::sorter triple2_less_sorter_1st_type;

	typedef triple<size_type, fpa_type, alphabet_extension_type> triple3_type;

	typedef tuple_less_comparator_1st<triple3_type> triple3_less_comparator_1st_type; // compare by 1st component in ascending order

	typedef ExTupleSorter<triple3_type, triple3_less_comparator_1st_type>::sorter triple3_less_sorter_1st_type;

private:

	/// \brief record L-type, S-type and LMS bucket sizes (no matter empty or non-empty) in SA & LCP
	///
	/// Elements in SA & LCP are naturally divided into multiple buckets and each bucket contains all the suffixes starting with an identical character.
	/// Each bucket can be further divided into two parts, whhere the left and right part separately contain the L-type and S-type suffixes, respectively.
	/// For description convenience, we denote the two part as L-type bucket and S-type bucket, respectively.
	struct BktInfo {
	private:

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

			for (alphabet_type ch = 0; ch <= ch_max; ++ch) {

				m_bkt_size[ch] = m_l_bkt_size[ch] + m_s_bkt_size[ch];
			}

			return;
		}

		/// \brief return the number of L-type suffixes in the specified bucket
		///
		uint64 get_l_bkt_size(const alphabet_type _ch) {

			return m_l_bkt_size[_ch];
		}

		/// \brief return the number of S-type suffixes in the specified bucket
		///
		uint64 get_s_bkt_size(const alphabet_type _ch) {

			return m_s_bkt_size[_ch];
		}

		/// \brief return the number of LMS suffixes in the specified bucket
		///
		uint64 get_lms_bkt_size(const alphabet_type _ch) {

			return m_lms_bkt_size[_ch];
		}

		/// \brief return the number of suffixes in the specified bucket
		///
		uint64 get_bkt_size(const alphabet_type _ch) {

			return m_bkt_size[_ch];
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

		/// \brief ctor
		///
		LMSValidate(alphabet_vector_type* _t, size_vector_type* _sa, size_vector_type* _lcp) :
			m_t(_t), m_sa(_sa), m_lcp(_lcp), m_len(m_t->size()), m_lms_num(0), m_sa_lms(nullptr), m_lcp_lms(nullptr), ch_max(std::numeric_limits<alphabet_type>::max()), val_max(std::numeric_limits<size_type>::max()) {

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
			last_scanned_type = L_TYPE, last_scanned_ch = (*t_rev_reader);
			
			if ((*pair1_great_sorter)->first != pos) {
			
				std::cerr << "SA is not a permutation.\n";

				exit(0);
			}

			--pos, ++(*t_rev_reader), ++(*pair1_great_sorter); // rightmost is L-type, skip it

			for (; !t_rev_reader->empty(); --pos, ++(*t_rev_reader), ++(*pair1_great_sorter)) { // process the remaining, check if last scanned is LMS
			
				const pair1_type& tuple = *(*pair1_great_sorter);

				if (tuple.first != pos) {
				
					std::cerr << "SA is not a permutation\n";

					exit(0);
				}

				cur_scanned_ch = *(*t_reav_reader);

				cur_scanned_type = ((cur_scanned_ch < last_scanned_ch) || (cur_scanned_ch == last_scanned_ch && last_scanned_type == S_TYPE)) ? S_TYPE : L_TYPE;

				if (cur_scanned_type == L_TYPE && last_scanned_type == S_TYPE) { // last scanned is LMS

					++m_lms_num;

					pair1_less_sorter->push(pair1_type(tuple.second, tuple.first));
				}

				last_scanned_ch = cur_scanned_ch;

				last_scanned_type = cur_scanned_type;
			}

			delete t_rev_reader; t_rev_reader = nullptr;

			delete pair1_great_sorter; pair1_great_sorter = nullptr;

			pair1_less_sorter->sort();

			// step 3: compute LCP_LMS and redirect SA_LMS & LCP_LMS to the external files
			typename size_vector_type::bufreader_type* lcp_reader = new typename size_vector_type::bufreader_type(*m_lcp);

			m_sa_lms = new size_vector_type(); m_sa_lms->resize(m_lms_num);

			typename size_vector_type::bufwriter_type* sa_lms_writer = new typename size_vector_type::bufwriter_type(*m_sa_lms);

			m_lcp_lms = new size_vector_type(); m_lcp_lms->resize(m_lms_num);

			typename size_vector_type::bufwriter_type* lcp_lms_writer = new typename size_vector_type::bufwriter_type(*m_lcp_lms);

			size_type lcp_min = val_max;

			for (uint64 idx = 1; !(*pair1_less_sorter)->empty(); ++(*pair1_less_sorter)) {
			
				const pair1_type& tuple = *(*pair1_less_sorter);

				while (idx != tuple.first) {
				
					lcp_min = std::min(val_max, *(*lcp_reader));

					++(*lcp_reader), ++idx;
				}

				// idx == tuple.first, redirect
				(*sa_lms_writer) << tuple.second;

				lcp_min = std::min(val_max, *(*lcp_reader));

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
		pair2_less_sorter_1st_type* fetcp_fp() {

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

			for (uint64 pos = 0; !t_reader->empty(); ++(*t_reader), ++pos) {

				if (pos == (*pair1_less_sorter)->first) {

					pair2_less_sorter->push(pair2_type((*pair1_less_sorter)->second, fp)); // fp = FP[0, pos - 1]

					++(*pair1_less_sorter);
				}

				fp = static_cast<fpa_type>((static_cast<fpb_type>(fp) * R + (*(*t_reader) + 1)) % P); // plus 1 to avoid equal to 0
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
			
				while (!(*pair1_less_2nd_sorter)->empty() && (*pair1_less_2nd_sorter)->first == pos) {
				
					triple3_less_sorter->push(triple3_type((*pair1_less_2nd_sorter)->second, fp, *(*t_reader))); //fp = FP[0, pos - 1], ch = T[pos]

					++(*pair1_less_2nd_sorter);
				}

				fp = static_cast<fpa_type>((static_cast<fpb_type>(fp) * R + (*(*t_reader) + 1)) % P); // plus 1 to avoid equal to 0
			}

			// (*pair1_less_2nd_sorter)->second == m_len
			while (!(*pair1_less_2nd_sorter)->empty()) {
			
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

				while (!(*pair1_less_2nd_sorter)->empty() && (*pair1_less_2nd_sorter)->first == pos) {

					triple3_less_sorter->push(triple3_type((*pair1_less_2nd_sorter)->second, fp, *(*t_reader)));

					++(*pair1_less_2nd_sorter);
				}

				fp = static_cast<fpa_type>((static_cast<fpb_type>(fp) * R + (*(*t_reader) + 1)) % P); // plus 1 to avoid equal to 0
			}

			// (*pair1_less_2nd_sorter)->second == m_len
			while (!(*pair1_less_2nd_sorter)->empty()) {

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
		RetrievePre(alphabet_vector_type* _t, size_vector_type* _sa, pair3_less_sorter_1st_type*& _pre_lms_item_sorter, triple2_less_sorter_1st_type*& _pre_l_item_sorter, triple2_less_sorter_1st_type*& _pre_s_item_sorter, BktInfo& _bkt_info) {
		
			// step 1: sort (SA[i], i) by i in descending order 
			pair1_great_sorter_1st_type* pair1_great_sorter = new pair1_great_sorter_1st_type(pair1_great_comparator_1st_type(), MAIN_MEM_AVAIL / 4);
	
			typename size_vector_type::bufreader_type* sa_reader = new typename size_vector_type::bufreader_type(*_sa);

			for (uint64 idx = 1; !sa_reader->empty(); ++idx, ++(*sa_reader)) {
			
				pair1_great_sorter->push(pair1_type(*(*sa_reader), idx));
			}

			delete sa_reader; sa_reader = nullptr;

			pair1_great_sorter->sort();

			// step 2: record pre_t & pre_ch for L-type, S-type and LMS suffixes separately.
			_pre_lms_item_sorter = new pair3_less_sorter_1st_type(pair3_less_comparator_1st_type(), MAIN_MEM_AVAIL / 4);

			_pre_l_item_sorter = new triple2_less_sorter_1st_type(triple2_less_comparator_1st_type(), MAIN_MEM_AVAIL / 4);

			_pre_s_item_sorter = new triple2_less_sorter_1st_type(triple2_less_comparator_1st_type(), MAIN_MEM_AVAIL / 4);

			typename alphabet_vector_type::bufreader_reverse_type* t_rev_reader = new typename alphabet_vector_type::bufreader_reverse_type(*_t);
			
			alphabet_type cur_scanned_ch, last_scanned_ch;
			
			uint8 cur_scanned_type, last_scanned_type;

			// rightmost character is L_TYPE
			cur_scanned_type = L_TYPE, cur_scanned_ch = *(*t_rev_reader);

			++(*t_rev_reader), ++(*pair1_great_sorter);
			
			last_scanned_type = cur_scanned_type, last_scanned_ch = cur_scanned_ch;

			for (; !t_rev_reader->empty(); ++(*t_rev_reader), ++(*pair1_great_sorter)) {
			
				cur_scanned_ch = *(*t_rev_reader);

				cur_scanned_type = ((cur_scanned_ch < last_scanned_ch) || (cur_scanned_ch == last_scanned_ch && last_scanned_ch == S_TYPE)) ? S_TYPE : L_TYPE;

				if (last_scanned_type == L_TYPE) {
				
					_bkt_info.add_l(last_scanned_ch);

					_pre_l_item_sorter->push(triple2((*pair1_great_sorter)->second, cur_scanned_ch, cur_scanned_type));
				}
				else {

					_bkt_info.add_lms(last_scanned_ch);

					_pre_s_item_sorter->push(triple2((*pair1_great_sorter)->second, cur_scanned_ch, cur_scanned_type));

					if (cur_scanned_type == L_TYPE) {
					
						_bkt_info.add_lms(last_scanned_ch);

						_pre_lms_item_sorter->push(pair3((*pair1_great_sorter)->second, cur_scanned_ch));
					}
				}

				last_scanned_ch = cur_scanned_ch;

				last_scanned_type = cur_scanned_type;
			}

			// leftmost (last scanned) must not be LMS and we suppose its preceding to be the sentinel 
			cur_scanned_type = SENTINEL_TYPE, cur_scanned_ch = 0;

			if (last_scanned_ch == L_TYPE) { 
			
				_bkt_info.add_l(last_scanned_ch);

				_pre_l_item_sorter->push(triple2(*(*pair1_great_sorter), cur_scanned_ch, cur_scanned_type));
			}
			else {

				_bkt_info.add_s(last_scanned_ch);

				_pre_s_item_sorter->push(triple2(*(*pair1_great_sorter), cur_scanned_ch, cur_scanned_type));
			}
			
			delete pair1_great_sorter; pair1_great_sorter = nullptr;

			delete t_rev_reader; t_rev_reader = nullptr;

			// compute bucket size by summing up L-type ones and S-type ones
			_bkt_info.accumulate();
		}
	};


#ifdef TEST_MY_TEST

	/// \brief RMQ
	///
	/// given SA[k] and SA[k + 1] induced from SA[i] and SA[j] respectively, the LCP-value of suf(SA[k]) and suf(SA[k + 1]) is determined by
	/// LCP[i + 1, j]. The response to the RMQ for LCP[i + 1, j] returns the minimum of LCP[i + 1, j].
	struct RMQ {
	private:

		const alphabet_type ch_max;

		const alphabet_type val_max;

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

			for (alphabet_type ch = 0; ch <= ch_max; ++ch) {

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
	struct RScan {

	private:
		const alphabet_type ch_max; ///< maximum character 

		alphabet_vector_type* m_t; ///< pointer to input string

		size_vector_type* m_sa; ///< pointer to suffix array

		size_vector_type* m_lcp; ///< pointer to LCP array

		size_vector_type* m_sa_lms; ///< pointer to SA_LMS

		size_vector_type* m_lcp_lms; ///< pointer to LCP_LMS

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

		/// \brief ctor
		///
		RScan(const BktInfo& _bkt_info, alphabet_type* _t, size_vector_type* _sa, size_vector_type* _lcp, size_vector_type* _sa_lms, size_vector_type* _lcp_lms) :
			ch_max(std::numeric_limits<alphabet_type>::max()), m_t(_t), m_sa(_sa), m_lcp(_lcp), m_sa_lms(_sa_lms), m_lcp_lms(_lcp_lms) {

			m_total_l_toscan = m_total_l_scanned = 0;

			m_total_lms_toscan = m_total_lms_scanned = 0;

			uint64 spos = 0;

			for (alphabet_type ch = 0; ch <= ch_max; ++ch) {

				m_l_bkt_toscan.push(_bkt_info.get_l_bkt_size(ch));

				m_lms_bkt_toscan.push(_bkt_info.get_lms_bkt_size(ch));

				m_total_l_toscan += m_l_bkt_toscan[ch];

				m_total_lms_toscan += m_lms_bkt_toscan[ch];

				m_l_bkt_spos.push(spos);



				spos += _bkt_info.get_bkt_size(ch); // move to the start pos of next bucket
			}
			
			for (alphabet_type ch = 0; ch <= ch_max; ++ch) { // determine the leftmost non-empty L-type bucket

				if (m_l_bkt_toscan[ch] != 0) {

					m_sa_l_reader = new typename size_vector_type::bufreader_type(m_sa->begin(), m_sa->begin() + m_l_bkt_spos[ch]);

					m_lcp_l_reader = new typename size_vector_type::bufreader_type(m_lcp->begin(), m_lcp->begin() + m_l_bkt_spos[ch]);
				
					m_cur_l_ch = ch; 
					
					break;
				}
			}

			for (alphabet_type ch = 0; ch <= ch_max; ++ch) { // determine the leftmost non-empty LMS bucket

				if (m_lms_bkt_toscan[ch] != 0) { 
					
					m_cur_lms_ch = ch; break; 
				}
			}

			m_l_bkt_scanned.resize(ch_max + 1, 0);

			m_lms_bkt_scanned.resize(ch_max + 1, 0);

			m_sa_lms_reader = new typename size_vector_type::bufreader_type(*m_sa_lms);

			m_lcp_lms_reader = new typename size_vector_type::bufreader_type(*m_lcp_lms);

			m_sa_l_bkt_reader.resize(ch_max + 1);

			m_lcp_l_bkt_reader.resize(ch_max + 1);

			typename size_vector_type::const_iterator sa_beg = m_sa->begin();

			typename size_vector_type::const_iterator lcp_beg = m_lcp->begin();

			for (alphabet_type ch = 0; ch <= ch_max; ++ch) {

				if (m_l_bkt_toscan[ch] != 0) {

					m_sa_l_bkt_reader[ch] = new typename size_vector_type::bufreader_type(sa_beg, sa_beg + m_l_bkt_toscan[ch]);

					m_lcp_l_bkt_reader[ch] = new typename size_vector_type::bufreader_type(lcp_beg, lcp_beg + m_l_bkt_toscan[ch]);
				}
				else {

					m_sa_l_bkt_reader[ch] = nullptr;

					m_lcp_l_bkt_reader[ch] = nullptr;
				}

				sa_beg = sa_beg + _bkt_info.get_bkt_size(ch));

				lcp_beg = lcp_beg + _bkt_info.get_bkt_size(ch));
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

			return m_lms_bkt_scanned[m_cur_lms_ch] == m_lms_bkt_toscan[m_cur_l_ch];
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

			// point to the start position of the non-empty L-type bucket in SA and LCP
			delete m_sa_l_reader;

			typename size_vector_type::const_iterator sa_beg = _sa->begin() + m_l_bkt_spos[cur_l_bkt_ch];

			m_sa_l_reader = new typename size_vector_type::bufreader_type(sa_beg, sa_beg + m_l_bkt_toscan[cur_l_bkt_ch]);

			delete m_lcp_l_reader;

			typename size_vector_type::const_iterator lcp_beg = _lcp->begin() + m_l_bkt_spos[cur_l_bkt_ch];

			m_lcp_l_reader = new typename size_vector_type::bufreader_type(lcp_beg, lcp_beg + m_l_bkt_toscan[cur_l_bkt_ch]);

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
		/// retrieve the values from SA and LCP
		void fetch_l_scanned(size_type& _sa_value, size_type& _lcp_value) {

			_sa_value = *(*m_sa_l_reader), ++(*m_sa_l_reader);

			_lcp_value = *(*m_lcp_l_reader), ++(*m_lcp_l_reader);

			++m_l_bkt_scanned[m_cur_l_ch];

			++total_l_scanned;

			return;
		}


		/// \brief fetch the SA-value of currently scanned LMS suffix and the LCP-value of the suffix and its left neighbor in SA_LMS
		///
		/// retrieve the values from SA_LMS and LCP_LMS
		void fetch_lms_scanned(size_type& _sa_value, size_type& _lcp_value) {

			_sa_value = *(*m_sa_lms_reader), ++(*m_sa_lms_reader);

			_lcp_value *(*m_lcp_lms_reader), ++(*m_lcp_lms_reader);

			++m_lms_bkt_scanned[m_cur_lms_ch];

			++total_lms_scanned;

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

		/// \brief 
		uint64 bruteforce_compute_lcp(size_type _pos1, size_type _pos2) {

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

			for (alphabet_type ch = 0; ch <= ch_max; ++ch) {

				delete m_sa_l_bkt_reader[ch]; m_sa_l_bkt_reader[ch] = nullptr;

				delete m_lcp_l_bkt_reader[ch]; m_lcp_l_bkt_reader[ch] = nullptr;
			}
		}

		/// \brief induce and check the order of L-type suffixes and their LCP values 
		///
		/// The SA-value of currently scanned suffix (no matter L-type or LMS) can be directly retrieved from SA or SA_LMS. 
		/// The question is how to retrieve the LCP-value of the suffix and the last scanned one (may not be the neighbors in SA). 
		///		case 1: if currently scanned suffix is L-type, then retrieve the LCP-value following one of the two sub-cases as below:
		///         sub-case (a): if the suffix is the leftmost in the L-type bucket, then perform a literal comparison between the suffix and the last scanned one to obtain their LCP-value.
		///			sub-case (b): otherwise, retrieve the LCP-value from LCP.
		///		case 2: if currently scanned suffix is LMS, then retrieve the LCP-value following one of the two sub-cases as below:
		///			sub-case (a): if the suffix is the leftmost in the LMS bucket and the last scanned is L-type, then perform a literal comparison between the suffix and the last scanned to obtain their LCP-value.
		///			sub-case (b): otherwise, retrieve the LCP-value from LCP_LMS.
		/// The SA-value of currently induced L-type suffix can be directly retrieved from SA or SA_LMS.
		/// The question is how to retrieve the LCP-value of the suffix and the last induced.
		///		case 1: if currently induced suffix is the leftmost in the L-type bucket, then do nothing.
		///		case 2: otherwise, the LCP-value is 1 + RMQ_VALUE. 
		void run(const triple2_less_sorter_1st_type* _pre_l_item_sorter, const pair2_less_sorter_1st_type* _pre_lms_item_sorter) {

			alphabet_type cur_bkt_ch, pre_ch;

			uint8 pre_t;

			size_type sv_cur_scanned, lv_cur_scanned, sv_last_scanned, lv_last_scanned; // scanned SA-value & LCP-value

			size_type sv_induced, lv_induced, sv_induced_fetch, lv_induced_fetch; // induced SA-value & LCP-value

			RMQ rmq_l;

			{ // process the virtual sentinel

				// step 1: retrieve SA-value and LCP-value
				sv_cur_scanned = m_sa->size(); // assumed to be m_len

				lv_cur_scanned = 0; // assumed to be 0, as the sentinel is smaller than any other characters

				// step 2: update RMQ
				rmq_l.update(lv_cur_scanned);

				// step 3: induce and validate the SA-value and LCP-value of the preceding suffix in SA
				auto rit = m_t->rbegin();

				pre_ch = *rit, pre_t = L_TYPE; // the virtual sentinel is the smallest, thus the rightmost the character must be L-type
		
				fetch_l_induced(pre_ch, sv_induced_fetch, lv_induced_fetch); // retrieve the induced value from SA & LCP 

				sv_induced = sv_cur_scanned - 1; // induce SA-value from currently scanned

				if (sv_induced_fetch != sv_induced) { // validate induced SA-value

					std::cerr << "wrong\n";

					exit(0);
				}

				// the induced suffix must be the leftmost in the L-type bucket, skip validating the induced LCP-value
			
				// step 4: reset RMQ
				rmq_l.reset(pre_ch);

				// step 5: iterates
				sv_last_scanned = sv_cur_scanned;

				lv_last_scanned = lv_cur_scanned;
			}

			{ // process the remaing

				bool flag = true; // indicate whether or not currently scanned suffix is the leftmost in the L-type/LMS bucket

				bool flags[ch_max + 1] = { true }; // indicate whether or not currently induced suffix is the leftmost in the L-type bucket

				// determine the initial character of the bucket to scan
				if (l_is_empty()) { // there must exist at least one L-type suffix (induced from the virtual sentinel)
				
					std::cerr << "wrong\n";

					exit(0);
				}

				cur_bkt_ch = get_l_bkt_ch();

				if (!lms_is_empty() && get_lms_bkt_ch() < cur_bkt_ch) { // first L-type then LMS in the same bucket,thus must be <

					cur_bkt_ch = get_lms_bkt_ch();
				}

				// scan all the buckets in sequence
				while (true) {

					// scan L-type bucket
					while (!l_cur_bkt_is_empty() && get_l_bkt_ch() == cur_bkt_ch) {

						// step 1: retrieve SA-value & LCP-value from SA & LCP
						fetch_l_scanned(sv_cur_scanned, lv_cur_scanned);

						if (flag == true) {// leftmost in the L-type bucket, the last scanned suffix may not be the left neighbor of currently scanned suffix in SA

							if (sv_last_scanned == m_sa->size()) { // last scanned suffix is the sentinel
							
								lv_cur_scanned = 0;
							}
							else {

								lv_cur_scanned = bruteforce_compute_lcp(sv_last_scanned, sv_cur_scanned);
							}

							flag = false;
						}

						// step 2: update RMQ
						rmq_l.update(lv_cur_scanned);

						// step 3: induce and validate
						pre_ch = (*_pre_l_item_sorter)->second, pre_t = *(*_pre_l_item_sorter).third, ++(*_pre_l_item_sorter);

						if (pre_t == L_TYPE) {

							fetch_l_induced(pre_ch, sv_induced_fetch, lv_induced_fetch);

							sv_induced = sv_cur_scanned - 1;

							if (sv_induced != sv_induced_fetch) {

								std::cerr << "wrong\n";

								exit(0);
							}

							if (flags[pre_ch] == true) { // the induced L-type suffix is the leftmost suffix in the L-type bucket

								// skip computing lv_induced		
								flags[pre_ch] = false;
							}
							else {

								lv_induced = rmq_l.get(pre_ch) + 1;

								if (lv_induced != lv_induced_fetch) {

									std::cerr << "wrong\n";

									exit(0);
								}
							}

							// reset RMQ
							rmq_l.reset(pre_ch);
						}

						// iterates
						sv_last_scanned = sv_cur_scanned;

						lv_last_scanned = lv_cur_scanned;
					}

					flag = true;

					// scan S-type bucket
					while (!lms_cur_bkt_is_empty() && get_lms_bkt_ch() == cur_bkt_ch) {

						// step 1: retrieve SA-value & LCP-value from SA_LMS & LCP_LMS
						fetch_l_scanned(sv_cur_scanned, lv_cur_scanned);

						if (flag == true) { // leftmost in the LMS bucket

							lv_cur_scanned = bruteforce_compute_lcp(sv_last_scanned, sv_cur_scanned);

							flag = false;
						}

						// step 2: update RMQ
						rmq_l.update(lv_cur_scanned);

						// step 3: induce and validate
						pre_ch = *(*_pre_lms_item_sorter).second, pre_t = *(*_pre_lms_item_sorter).third, ++(*_pre_lms_item_sorter);

						if (pre_t == L_TYPE) {

							fetch_l_induced(pre_ch, sv_induced_fetch, lv_induced_fetch);

							sv_induced = sv_cur_scanned - 1;

							if (sv_induced != sv_induced_fetch) {

								std::cerr << "wrong\n";

								exit(0);
							}

							if (flags[pre_ch] == true) { // the induced suffix is the leftmost in the L-type bucket

								// skip computing the LCP-value
								flags[pre_ch] = false;
							}
							else {

								lv_induced = rmq_l.get(pre_ch) + 1;

								if (lv_induced != lv_induced_fetch) {

									std::cerr << "wrong\n";

									exit(0);
								}
							}

							// reset RMQ
							rmq_l.reset(pre_ch);
						}

						// iterates
						sv_last_scanned = sv_cur_scanned;

						lv_last_scanned = lv_cur_scanned;
					}

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

		uint64 total_s_toscan; ///< total number of S-type suffixes to be scanned

		uint64 total_s_scanned; ///< total number of S-type suffixes already scanned

		uint64 total_l_toscan; ///< total number of L-type suffixes to be scanned

		uint64 total_l_scanned; ///< total number of L-type suffixes already scanned

		alphabet_type m_cur_s_ch; ///< S-type bucket currently being scanned

		alphabet_type m_cur_l_ch; ///< L-type bucket currently being scanned

		typename size_vector_type::bufreader_reverse_type* m_sa_rev_reader; ///< for scan, point to the SA-value for currently scanned suffix (retrieve from SA, leftward)

		typename size_vector_type::bufreader_reverse_type* m_lcp_rev_reader; ///< for scan, point to the LCP-value for currently scanned suffix and its right neighbor in SA (retrieve from LCP, leftward)

		std::vector<typename size_vector_type::bufreader_reverse_type*> m_sa_s_bkt_rev_reader; ///< for induce, point to the SA-value for the S-type suffix to be induced in each bucket (retrievd from SA, leftward)

		std::vector<typename size_vector_type::bufreader_reverse_type*> m_lcp_s_bkt_rev_reader; ///< for induce, point to the LCP-value for the S-type suffix to be induced in each bucket (retrieved from LCP, leftward)
	
		/// \brief ctor
		///
		LScan(const BktInfo& _bkt_info, alphabet_type* _t, size_vector_type* _sa, size_vector_type* _lcp) :
			ch_max(std::numeric_limits<alphabet_type>::max()), m_t(_t), m_sa(_sa), m_lcp(_lcp) {

			m_total_s_toscan = m_total_s_scanned = 0;

			m_total_l_toscan = m_total_l_scanned = 0;

			uint64 spos = 0; // leftward scanning

			for (alphabet_type ch = ch_max; ch >= 0; --ch) {

				m_s_bkt_toscan.push(_bkt_info.get_s_bkt_size(ch));

				m_l_bkt_toscan.push(_bkt_info.get_l_bkt_size(ch));

				m_total_s_toscan += m_s_bkt_toscan[ch];

				m_total_l_toscan += m_l_bkt_toscan[ch];

				m_s_bkt_spos.push(spos);

				m_l_bkt_spos.push(spos + _bkt_info.get_s_bkt_size(ch));

				spos += _bkt_info.get_bkt_size(ch);

				if (ch == 0) break; // break dead loop
			}

			// determine rightmost non-empty S-type bucket
			for (alphabet_type ch = ch_max; ch >= 0; --ch) {

				if (m_s_bkt_toscan[ch] != 0) { 
					
					m_cur_s_ch = ch; 
					
					break; 
				}

				if (ch == 0) break;
			}

			// determine rightmost non-empty L-type bucket
			for (alphabet_type ch = ch_max; ch >= 0; --ch) {

				if (m_l_bkt_toscan[ch] != 0) { 
					
					m_cur_l_ch = ch;

					break; 
				}

				if (ch == 0) break;
			}

			m_s_bkt_scanned.resize(ch_max + 1, 0);

			m_l_bkt_scanned.resize(ch_max + 1, 0);

			m_sa_rev_reader = new typename size_vector_type::bufreader_reverse_type(*m_sa);

			m_lcp_rev_reader = new typename size_vector_type::bufreader_reverse_type(*m_lcp);

			m_sa_s_bkt_rev_reader.resize(ch_max + 1);

			m_lcp_s_bkt_rev_reader.resize(ch_max + 1);

			typename size_vector_type::const_reverse_iterator sa_rbeg = _sa->rbegin();

			typename size_vector_type::const_reverse_iterator lcp_rbeg = _lcp->rbegin();

			for (alphabet_type ch = ch_max; ch >= 0; --ch) {

				if (m_s_bkt_toscan[ch] != 0) {

					m_sa_s_bkt_rev_reader[ch] = new typename size_vector_type::bufreader_type(sa_rbeg, sa_rbeg + m_s_bkt_toscan[ch]);

					m_lcp_s_bkt_rev_reader[ch] = new typename size_vector_type::bufreader_type(lcp_rbeg, lcp_rbeg + m_s_bkt_toscan[ch]);
				}
				else {

					m_sa_s_bkt_rev_reader[ch] = nullptr;

					m_lcp_s_bkt_rev_reader[ch] = nullptr;
				}

				sa_rbeg = sa_rbeg + _bkt_info.get_bkt_size(ch); // skip the bucket 

				lcp_rbeg = lcp_rbeg + _bkt_info.get_bkt_size(ch);

				if (ch == 0) break; // break dead loop
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

			++total_s_scanned;

			return;
		}


		/// \brief fetch the SA-value for currently scanned L-type suffix and the LCP-value of the suffix and its right neighbor in SA
		///
		void fetch_l_scanned(size_type& _sa_value, size_type& _lcp_value) {

			_sa_value = *(*m_sa_rev_reader), ++(*m_sa_rev_reader);

			_lcp_value = *(*m_lcp_rev_reader), ++(*m_lcp_rev_reader);

			++m_l_bkt_scanned[m_cur_l_ch];

			++total_l_scanned;

			return;
		}

		/// \brief fetch the SA-value for currently induced S-type suffix and the LCP-value for the suffix and its right neighbor in SA 
		///
		void fetch_s_induced(const alphabet_type _ch, size_type& _sa_value, size_type& _lcp_value) {

			_sa_value = *(*m_sa_s_bkt_rev_reader[_ch]), ++(*m_sa_s_bkt_rev_reader[_ch]);

			_lcp_value = *(*m_lcp_s_bkt_reader[_ch]), ++(*m_lcp_s_bkt_reader[_ch]);

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

		/// \brief 
		uint64 bruteforce_compute_lcp(size_type _pos1, size_type _pos2) {
		
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
		~LScan() {

			delete m_sa_rev_reader; m_sa_rev_reader = nullptr;

			delete m_lcp_rev_reader; m_lcp_rev_reader = nullptr;

			for (alphabet_type ch = 0; ch <= ch_max; ++ch) {

				delete m_sa_s_bkt_rev_reader[ch]; m_sa_s_bkt_rev_reader[ch] = nullptr;

				delete m_lcp_s_bkt_rev_reader[ch]; m_lcp_s_bkt_rev_reader[ch] = nullptr;
			}
		}

		/// \brief induce and check the order of S-type suffixes and and their LCP-values
		///
		/// The SA-value of currently scanned suffix (no matter L-type or S_type) can be directly retrieved from SA,
		/// while the LCP-value of the suffix and the last scanned can be retrieved following one of the two cases below:
		///		case 1: if currently scanned suffix is the rightmost in the L-type bucket, then perform a literal comparison between the suffix and the last scanned to obtain their LCP-value and compare the result to the one retrieved from LCP for validation.
		///		case 2: otherwise, retrieve the LCP-value from LCP.
		/// The SA-value of currently induced S-type suffix can be directly retrieved from SA,
		/// while the LCP-value of the suffix and the last induced one can be obtained following one of the two cases below:
		///		case 1: if currently induced suffix is the rightmost in the S-type bucket, then the LCP-value is 0.
		///		case 2: otherwise, the LCP-value is 1 + RMQ_VALUE.
		bool run(const triple2_less_sorter_1st_type* _pre_s_item_sorter, const triple2_less_sorter_1st_type* _pre_l_item_sorter) {

			alphabet_type cur_bkt_ch, pre_ch_of_last_scanned;

			uint8 pre_t_of_last_scanned;

			size_type sv_cur_scanned, lv_cur_scanned, sv_last_scanned, lv_last_scanned;

			size_type sv_induced, lv_induced, sv_induced_fetch, lv_induced_fetch;

			RMQ rmq_s;

			bool flag = true; // indicate current scanned S-type/L-type suffix is the rightmost suffix in the S-type/L-type bucket

			bool flags[ch_max + 1] = { true }; // indicate currently induced S-type suffix is the rightmost suffix in the S-type bucket

			{ // process the rightmost suffix in SA, which must be L-type

				// determine the initial character of currently scanned bucket
				if (get_l_bkt_ch() <= get_s_bkt_ch()) {

					std::cerr << "wrong\n";

					return false;
				}

				cur_bkt_ch = get_l_bkt_ch();

				flag = false;

				// has no right neighbor, just fetch the SA-value and LCP-values
				fetch_l_scanned(sv_cur_scanned, lv_cur_scanned);

				// iterates
				sv_last_scanned = sv_cur_scanned;

				lv_last_scanned = lv_cur_scanned;

				pre_t_of_last_scanned = (*_pre_l_item_sorter)->second;
				
				pre_ch_of_last_scanned = (*_pre_l_item_sorter)->third;
			}

			{ // process the remaining

				if (l_is_empty() && s_is_empty()) { // no more elements
	
					goto to_end;
				}
				
				//
				if (l_cur_bkt_is_empty()) { // current L-type bucket is empty

					flag = true;

					if (!l_is_empty()) {

						find_next_l_bkt();

						cur_bkt_ch = get_l_bkt_ch();

						if (!s_is_empty() && get_s_bkt_ch() >= cur_bkt_ch) {

							cur_bkt_ch = get_s_bkt_ch();
						}
					}
					else if (!s_is_empty()){

						cur_bkt_ch = get_s_bkt_ch();
					}
					else {
					
						goto to_end;
					}
				}

				//
				while (true) {

					while (!s_cur_bkt_is_empty() && get_s_bkt_ch() == cur_bkt_ch) {

						//  step 1 : induce and validate the SA-value of the suffix induced from the last scanned and the LCP-value of this suffix and its right neighbor in SA
						if (pre_t_of_last_scanned == S_TYPE) {

							fetch_s_induced(pre_ch_of_last_scanned, sv_induced_fetch, lv_induced_fetch);

							sv_induced = sv_last_scanned - 1;

							if (sv_induced != sv_induced_fetch) { // check SA-value

								std::cerr << "wrong\n";

								return false;
							}

							if (flags[pre_ch_last_scanned] == true) { // the induced S-type is the rightmost in the S-type bucket

								if (lv_induced_fetch != 0) {

									std::cerr << "wrong\n";

									return false;
								}

								flags[pre_ch_last_scanned] = false;
							}
							else {

								lv_induced = rmq_l.get(pre_ch_of_last_scanned) + 1; // RMQ_VALUE + 1

								if (lv_induced != lv_induced_fetch) { // check LCP-value

									std::cerr << "wrong\n";

									return false;
								}
							}

							//reset RMQ
							rmq_s.reset(pre_ch_of_last_scanned);
						}

						// step 2: update rmq
						rmq_s.update(lv_last_scanned);

						// iterates
						sv_last_scanned = sv_cur_scanned;

						lv_last_scanned = lv_cur_scanned;

						pre_ch_of_last_scanned = (*_pre_s_item_sorter)->second;

						pre_t_of_last_scanned = (*_pre_l_item_sorter)->third;
					}

					flag = true;

					// scan L-type bucket
					while (!l_cur_bkt_is_empty() && get_l_bkt_ch() == cur_bkt_ch) {

						// step 1: induce and validate
						if (pre_t_of_last_scanned == S_TYPE) {

							fetch_l_induced(pre_ch_of_last_scanned, sv_induced_fetch, lv_induced_fetch);

							sv_induced = sv_last_scanned - 1;

							if (sv_induce != sv_induced_fetch) {

								std::cerr << "wrong\n";

								return false;
							}

							if (flags[pre_ch_last_scanned] == true) { // the induced S-type is the rightmost in the S-type bucket

								if (lv_induced_fetch != 0) {

									std::cerr << "wrong\n";

									return false;
								}

								flags[pre_ch_last_scanned] = false;
							}
							else {

								lv_induced = rmq_l.get(pre_ch_of_last_scanned) + 1; // RMQ_VALUE + 1

								if (lv_induced != lv_induced_fetch) { // check LCP-value

									std::cerr << "wrong\n";

									return false;
								}
							}

							//reset RMQ
							rmq_s.reset(pre_ch_of_last_scanned);
						}

						// step 2: retrieve SA-value and LCP-value
						fetch_l_scanned(sv_cur_scanned, lv_cur_scanned);

						if (flag == true) { // rightmost suffix in the L-type bucket

							// check the LCP-value of currently scanned suffix and the last scanned suffix
							if (lv_last_scanned != bruteforce_compute_lcp(sv_cur_scanned, sv_last_scanned)) {

								std::cerr << "wrong\n";

								return false;
							}

							flag = false;
						}

						// step 3: update rmq
						rmq_s.update(lv_last_scanned);


						// iterates
						sv_last_scanned = sv_cur_scanned;

						lv_last_scanned = lv_cur_scanned;

						pre_ch_of_last_scanned = (*_pre_s_item_sorter)->second;

						pre_t_of_last_scanned = (*_pre_s_item_sorter)->third;
					}

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
			}

			// check the leftmost LCP-value and
			to_end:
			return true;
		}

	};

#endif

private:
		
	alphabet_vector_type* m_t;

	size_vector_type* m_sa;

	size_vector_type* m_lcp;
	
	/// \brief ctor
	///
	Validate4(alphabet_vector_type* _t, size_vector_type* _sa, size_vector_type* _lcp) :
	m_t(_t), m_sa(_sa), m_lcp(_lcp) {}

	bool run() {

		size_vector_type* sa_lms = nullptr, *lcp_lms = nullptr;

		{ // generate and check SA_LMS and LCP_LMS

			LMSValidate lms_validate(m_t, m_sa, m_lcp, sa_lms, lcp_lms);

			lms_validate->run();

			sa_lms = lms_validate.get_sa_lms();

			lcp_lms = lms_validate.get_lcp_lms();

		}

		triple2_less_sorter_1st_type* pre_l_item_sorter = nullptr, *pre_s_item_sorter = nullptr;

		pair2_less_sorter_1st_type* pre_lms_item_sorter = nullptr;

		{ //generate preceding items

			RetrievePre retrieve_pre(m_t, m_sa, pre_lms_item_sorter, pre_l_item_sorter, pre_s_item_sorter, m_bkt_info);
		}

		// validate SA-values for L-type suffixes and LCP-values for these suffixes and their left neighbor in SA
		RScan r_scan(m_bkt_info, m_t, m_sa, m_lcp, sa_lms, lcp_lms);

		r_scan->run(pre_l_items, pre_lms_items);

		// validate SA-values for S-type suffixes and LCP-values for these suffixes and their right neighbor in SA
		LScan l_scan();

		l_scan->run(pre_l_items, pre_s_items);
	}
};

#endif // VALIDATE4_H