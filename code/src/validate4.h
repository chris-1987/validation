///////////////////////////////////////////////////////////////////////////////////
/// Copyright (c) 2016, Sun Yat-sen University,
/// All rights reserved
/// \file validate4.h
/// \brief A two-phase validator for suffix and LCP arrays verification.
///
/// Use method 1 (an implementation can be found in validate3.h/validate3.cpp) to
/// check SA_LMS & LCP_LMS and then use induce-sorting principle together with
/// RMQ technique to check SA & LCP.
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
/// \param alphabet_type type of elements in T
/// \param size_Type type of elements in SA/LCP
template<typename alphabet_type, typename size_type>
class Validate4{
	
private:
	// alias
	//
	typedef typename ExVector<alphabet_type>::vector alphabet_vector_type;
	//
	typedef typename ExVector<size_type>::vector size_vector_type;

	// 
	typedef pair<size_type, size_type> pair1_type; 
		
	typedef tuple_less_comparator_1st<pair1_type> pair1_less_comparator_type;

	typedef typename ExTupleSorter<pair1_type, pair1_less_comparator_type>::sorter pair1_less_sorter_type;
	
	typedef tuple_great_comparator_1st<pair1_type> pair1_great_comparator_type;

	typedef typename ExTupleSorter<pair1_type, pair1_great_comparator_type>::sorter
pair1_great_sorter_type;

	// 
	typedef pair<size_type, fpa_type> pair2_type;

	typedef tuple_less_comparator_1st<pair2_type> pair2_less_comparator_type;

	typedef typename ExTupleSorter<pair2_type, pair2_less_comparator_type>::sorter pair2_less_sorter_type;

	//
	typedef pair<size_type, uint8> pair3_type;

	typedef tuple_less_comparator_1st<pair3_type> pair3_comparator_type;
	
	typedef typename ExTupleSorter<pair3_type, pair3_comparator_type>::sorter pair3_less_sorter_type;
	
	//
	typedef triple<size_type, fpa_type, uint16> triple1_type;

	typedef tuple_less_comparator_1st<triple1_type> triple1_less_comparator_type;

	typedef ExTupleSorter<triple1_type, triple1_less_comparator_type>::sorter
triple1_less_sorter_type;

	//
	typedef triple<size_type, uint8, uint8> triple2_type;

	typedef tuple_less_comparator_1st<triple2_type> triple2_less_comparator_type;

	typedef ExTupleSorter<triple2_type, triple2_less_comparator_type>::sorter triple2_less_sorter_type; 
	
private:
	
	struct BktInfo{
	private:
		
		std::vector<uint64> m_bkt_size;
			
		std::vector<uint64> m_l_bkt_size;
			
		std::vector<uint64> m_s_bkt_size;		
		
		std::vector<uint64> m_lms_bkt_size; 
	
	public:
		
		/// \brief collect bucket size information
		BktInfo() {
			
			m_bkt_size.resize(std::numeric_limits<alphabet_type>::max() + 1);
				
			m_l_bkt_size.resize(std::numeric_limits<alphabet_type>::max() + 1);
				
			m_s_bkt_size.resize(std::numeric_limits<alphabet_type>::max() + 1);
				
			m_lms_bkt_size.resize(std::numeric_limits<alphabet_type>::max() + 1);	
		}
		
		/// \brief count L-type suffixes
		void set_l(const alphabet_type _ch) {
			
			++m_l_bkt_size[_ch];
			
			return;
		}
		
		/// \brief count S-type suffixes
		void set_s(const alphabet_type _ch) {
			
			++m_s_bkt_size[_ch];	
			
			return;
		}
		
		/// \brief count LMS suffixes
		void set_lms(const alphabet_type _ch) {
			
			++m_lms_bkt_size[_ch];	
			
			return;
		}
		
		/// \brief count all the suffixes 
		void set_sum() {
			
			for (alphabet_type ch = 0; ch <= std::numeric_limits<alphabet_type>::max() ; ++ch) {
	
				m_bkt_size[ch] = m_l_bkt_size[ch] + m_s_bkt_size[ch];
				
			}
			
			return;
		}
		
		/// \brief get l-type bucket size
		uint64 get_l(const alphabet_type _ch) {
			
			return m_l_bkt_size[_ch];	
		}
		
		/// \brief get s-type bucket size
		void get_s(const alphabet_type _ch) {
			
				return m_s_bkt_size[_ch];	
		}
		
		/// \brief get lms bucket size
		void get_lms(const alphabet_type _ch) {
			
			return m_lms_bkt_size[_ch];	
		}
		
		/// \brief get bucket size
		void get_sum(const alphabet_type _ch) {
			
			return m_bkt_size[_ch];	
		}

	};

	/// \brief scan rightward to validate the relative order of L-type suffixes and the LCP-values btw them and their left neighbors in SA.
	struct RScan{
		
	private:
			
		const alphabet_type ch_max;
		
		std::vector<uint64> m_l_bkt_toscan; ///< l-type bkt size

		std::vector<uint64> m_l_bkt_scanned;
									
		std::vector<uint64> m_lms_bkt_toscan; ///< lms_type bkt size

		std::vector<uint64> m_lms_bkt_scanned;
			
		std::vector<uint64> m_l_bkt_spos; ///< start offset of each L-type bucket in SA
			
		uint64 total_l_toscan; 
		
		uint64 total_l_scanned;
		
		uint64 total_lms_toscan;
		
		uint64 total_lms_scanned;
			
		typename size_vector_type::bufreader_type* m_sa_l_reader;
			
		typename size_vector_type::bufreader_type* m_lcp_l_reader;
			
		typename size_vector_type::bufreader_type* m_sa_lms_reader;
			
		typename size_vector_type::bufreader_type* m_lcp_lms_reader;
		
		std::vector<typename size_vector_type::bufreader_type*> m_sa_l_bkt_reader;
			
		std::vector<typename size_vector_type::bufreader_type*> m_lcp_l_bkt_reader;

		/// \brief ctor
		///
		RScan(const BktInfo& _bkt_info, size_vector_type* _sa, size_vector_type* _lcp) : ch_max (std::numeric_limits<alphabet_type>::max()) {
			
				total_l_toscan = total_l_scanned = 0;
				
				total_lms_toscan = total_lms_scanned = 0;
				
				uint64 spos = 0;
				
				for (alphabet_type ch = 0; ch <= ch_max; ++ch) {
						
						m_l_bkt_toscan.push(_bkt_info.get_l(ch));
									
						m_lms_bkt_toscan.push(_bkt_info.get_lms(ch));	
						
						total_l_toscan += m_l_bkt_toscan[ch];
						
						total_lms_toscan += m_lms_bkt_toscan[ch];
						
						m_l_bkt_spos.push(spos);
						
						spos += _bkt_info.get(ch);
				}
				
				m_l_bkt_scanned.resize(ch_max+ 1);
				
				m_lms_bkt_scanned.resize(ch_max + 1);
				
				m_sa_l_reader = new typename size_vector_type::bufreader_type(*_sa);
					
				m_lcp_l_reader = new typename size_vector_type::bufreader_type(*_lcp);
				
				m_sa_lms_reader = new typename size_vector_type::bufreader_type(*_sa_lms);
					
				m_lcp_lms_reader = new typename size_vector_type::bufreader_type(*_lcp_lms);
					
				m_sa_l_bkt_reader.resize(ch_max + 1);
				
				m_lcp_l_bkt_reader.resize(ch_max + 1);
				
				typename size_vector_type::const_iterator sa_beg = _sa->begin();
					
				typename size_vector_type::const_iterator lcp_beg = _lcp->begin();
					
				for (alphabet_type ch = 0; ch <= std::numeric_limits<alphabet_type>::max(); ++ch) {
					
					if (m_lms_bkt_toscan[ch] != 0) {
			
						m_sa_l_bkt_reader[ch] = new typename size_vector_type::bufreader_type(sa_beg, sa_beg + m_l_bkt_toscan[i]);
						
						m_lcp_l_bkt_reader[ch] = new typename size_vector_type::bufreader_type(lcp_beg, lcp_beg + m_l_bkt_toscan[i]);

						sa_beg = sa_beg + _bkt_info.get(ch));
					
						lcp_beg = lcp_beg + _bkt_info.get(ch));
					}
					else {
						
						m_sa_l_bkt_reader[ch] = nullptr;
						
						m_lcp_l_bkt_reader[ch] = nullptr;

						sa_beg = sa_beg + _bkt_info.get(ch)); // may exist S-type suffix in the bucket
					
						lcp_beg = lcp_beg + _bkt_info.get(ch));
					}
				}
		}
		
		/// \brief no more L-type to be scanned
		///
		bool l_is_empty() {
		
			return total_l_toscan == total_l_scanned;
				
		}
		
		/// \brief no more LMS to be scanned
		bool lms_is_empty() {
			
			return total_lms_toscan == total_lms_scanned;	
		}
		
		/// \brief return the head character of next suffix to scan
		///
		/// \note guarantee that there still exist L-type suffixes to be scanned
		void forward_l(alphabet_type& _cur_ch) {
			
			// jump over current L-type bucket if finished reading and skip all the empty L-type buckets
			while (m_l_bkt_scanned[_cur_ch] == m_l_bkt_toscan[_cur_ch])	{

				++_cur_ch;
			}
	
			// new bucket
			if (m_l_bkt_scanned[_cur_ch] == 0) {
				
				delete m_sa_l_reader; 
				
				typename size_vector_type::const_iterator sa_beg = _sa->begin() + m_l_bkt_spos[_cur_ch];
					
				m_sa_l_reader = new typename size_vector_type::bufreader_type(sa_beg, sa_beg + m_l_bkt_toscan[_cur_ch]);
				
				delete m_lcp_l_reader;
				
				typename size_vector_type::const_iterator lcp_beg = _lcp->begin() + m_l_bkt_spos[_cur_ch];
					
				m_lcp_l_reader = new typename size_vector_type::bufreader_type(lcp_beg, lcp_beg + m_l_bkt_toscan[_cur_ch]);
			}
			
			++m_l_bkt_scanned[_cur_ch];	
						
			return;
		}
		
		/// \brief return the head character of next suffix to scan
		///
		/// \note guarantee that there still exist LMS suffixes to be scanned
		void forward_lms(alphabet_type& _cur_ch) {
			
			while (m_lms_bkt_scanned[_cur_ch] == m_lms_bkt_toscan[_cur_ch]) {
				
				++_cur_ch;
			}	
			
			++m_lms_bkt_scanned[_cur_ch];
			
			return;
		}
		
		/// \brief de-ctor
		~RScan() {
		
		}
		
		/// \brief
		/// 
		/// case 1: if cur_t == L_TYPE, the LCP-value of current suffix and its left neighbor can be retrieved from LCP. Update RMQ with the LCP-value and then induce the preceding suffix;
		///					if pre_t == L_TYPE, the LCP-value of preceding suffix and its left neighbor can be calculated by the following sub-cases:
		///         sub-case (a): pre_ch is the leftmost L-type character in the bucket, then the LCP-value must be 0;
		///		      sub-case (b): otherwise, the LCP-value is 1 + RMQ_VALUE
		/// case 2: if cur_t == LMS, the LCP-value of current suffix and its left neighbor can be retrieved following the two sub-cases below:
		///         sub-case (a): cur_ch is the leftmost S-type character in the S-bucket but not the bucket, then the LCP-value must be calculated by a literal string comparison between it and the rightmost L-type suffix in the bucket.
		///					sub-case (b): otherwise, the LCP-value can be directly retreived from LCP_LMS.
		///					Update RMQ with the LCP-value and then induce the preceding suffix;
		///					if pre_t == L_TYPE, the LCP-value of preceding suffix and its left neighor can be calculated by the following sub-cases:
		///					sub-case (a): pre_ch is the leftmost L-type character in the bucket, then the LCP-value must be 0;
		///					sub-case (b): otherwise, the LCP-value is 1 + RMQ_VALUE
		void run(const triple2_less_sorter_type* _sorter){
			
			alphabet_type cur_bkt_ch, cur_l_ch, cur_lms_ch;
			
			size_type sv_scanned, lv_scanned;
			
			size_type sv_induced, lv_induced;
			
			while (true) {
				
				// scan l-type suffixes
				while (!l_is_empty() && cur_l_ch == cur_bkt_ch) {
	
					// retrieve sa-value and lcp-value from sa and lcp
					size_type sv_scanned = *(*m_sa_l_reader);

					size_type lv_scanned = *(*m_lcp_l_reader);
					
					// update RMQ
					rmq_l.update(cur_bkt_ch, lv);
					
					// retrieve pre_ch & pre_t from m_l_pre;
					alphabet_type pre_ch = *(*_sorter)->second;
					
					alphabet_type pre_t = *(*_sorter)->third;
					
					//
					if (pre_t == L_TYPE) {
						
							// induce		
							sv_induced = sv_scanned - 1;
							
							if () { // first induced into the bucket
								
								lv_induced = 0;
							}
							else {
								
								lv_induced = rmq_l.get(cur_bkt_ch);	
							}
						
							// validate
							if (sa_induced != *(*m_sa_l_bkt_reader[ch]) || lv_induced != *(*m_lcp_l_bkt_reader)) {
								
								std::cerr << "wrong";
									
									exit();	
							}
					}
											
					pre_l_ch = cur_l_ch;
					
					pre_sv_scanned = sv_scanned;
						
					forward_l(cur_l_ch);
				}
				
				// scan leftmost lms suffix in the bucket
				if (!lms_is_empty() && cur_lms_ch == cur_bkt_ch) {
					
					// retrieve sa-value and lcp-value from sa_lms and lcp_lms
					sv_scanned = *(*m_sa_lms_reader);	++(*m_sa_lms_reader);		
	
					lv_scanned = *(*m_lcp_lms_reader); ++(*m_lcp_lms_reader);

					if (pre_l_ch == cur_bkt_ch) { // left neighbor is a L-type suffix in the same bucket, must recompute lv_scanned
						
						lv_scanned = bruteforce_compute_lcp(sv_scanned, pre_sv_scanned);
					}						
					
					// update RMQ
					rmq_l.update(cur_bkt_ch, lv_scanned);
					
					// induce
					sv_induced = sv_scanned - 1;
					
					if () { // first induced into the bucket
						
						lv_induced = 0;	
					}
					else {
						
						lv_induced = rmq_l.get(cur_bkt_ch);
					}
					
					// validate
					if (sa_induced != *(*m_sa_l_bkt_reader[ch]) || lv_induced != *(*m_lcp_l_bkt_reader)) {
					
						std::cerr << "wrong";
						
						exit();	
					}				
					
					pre_l_ch = cur_l_ch;
					
					pre_sv_scanned = sv_scanned;
										
					next_lms(cur_lms_ch);
				}
				
				// scan the remaining lms suffixes
				while (!lms_is_empty() && cur_lms_ch == cur_bkt_ch) {
					
					// retrieve sa-value and lcp-value from sa_lms and lcp_lms
					sv_scanned = *(*m_sa_lms_reader); ++(*m_sa_lms_reader);		
					
					lv_scanned = *(*m_lcp_lms_reader); ++(*m_lcp_lms_reader);
					
					// update RMQ
					rmq_l.update(cur_bkt_ch, lv_scanned);
					
					// induce
					sv_induce = sv_scanned - 1;
					
					if () { // first induced into the bucket
						
						lv_induced = 0;
					}
					else {
						
							lv_induced = rmq_l.get(cur_bkt_ch);
					}
					
					// validate
					if (sa_induced != *(*m_sa_l_bkt_reader[ch]) || lv_induced != *(*m_lcp_l_bkt_reader)) {
					
						std::cerr << "wrong";
						
						exit();	
					}				
					
					pre_l_ch = cur_l_ch;
					
					pre_sv_scanned = sv_scanned;
										
					next_lms(cur_lms_ch);
				}
				
				// determine next bucket
				if (!l_is_empty()) {
					
					cur_bkt_ch = cur_lms_ch;
	
					if (!lms_is_empty() && cur_lms_ch < cur_bkt_ch) {
					
						cur_bkt_ch = cur_lms_ch;
					}
				}
				else if (!lms_is_empty()) {
					
					cur_bkt_ch = cur_lms_ch;	
				}
				else {
					
					break;	
				}
			}
		}
	};
	
	/// \brief scan leftward to validate the relative order of S-type suffixes and the LCP-values btw them and their right neighbors in SA.
	struct LScan{
		
	private:
			
		const alphabet_type ch_max;
		
		std::vector<uint64> m_s_bkt_toscan; ///< s-type bkt size

		std::vector<uint64> m_s_bkt_scanned;
			
		std::vector<uint64> m_l_bkt_toscan; ///< l-type bkt size

		std::vector<uint64> m_l_bkt_scanned;
									
		std::vector<uint64> m_s_bkt_spos; ///< start offset of each S-type bucket in SA (from right to left)
			
		std::vector<uint64> m_l_bkt_spos; ///< start offset of each L-type bucket in SA (from right to left)
			
		uint64 total_s_toscan;
		
		uint64 total_s_scanned;
		
		uint64 total_l_toscan; 
		
		uint64 total_l_scanned;
			
		typename size_vector_type::bufreader_reverse_type* m_sa_s_reader;
			
		typename size_vector_type::bufreader_reverse_type* m_lcp_s_reader;
			
		typename size_vector_type::bufreader_reverse_type* m_sa_l_reader;
			
		typename size_vector_type::bufreader_reverse_type* m_lcp_l_reader;
				
		std::vector<typename size_vector_type::bufreader_reverse_type*> m_sa_s_bkt_reader;
			
		std::vector<typename size_vector_type::bufreader_reverse_type*> m_lcp_s_bkt_reader;

		/// \brief ctor
		///
		RScan(const BktInfo& _bkt_info, size_vector_type* _sa, size_vector_type* _lcp) : ch_max (std::numeric_limits<alphabet_type>::max()) {
		
				total_s_toscan = total_s_scanned = 0;
							
				total_l_toscan = total_l_scanned = 0;
			
				uint64 spos = 0;
				
				for (alphabet_type ch = ch_max; ch >= 0; --ch) {
						
						m_s_bkt_toscan.push(_bkt_info.get_s(ch));	
												
						m_l_bkt_toscan.push(_bkt_info.get_l(ch));
									
						total_s_toscan += m_s_bkt_toscan[ch];
						
						total_l_toscan += m_l_bkt_toscan[ch];
						
						m_s_bkt_spos.push(spos);
						
						m_l_bkt_spos.push(spos + _bkt_info.get_s(ch));
						
						spos += _bkt_info.get(ch);
						
						if (ch == 0) break; // break dead loop
				}
				
				m_s_bkt_scanned.resize(ch_max + 1);
								
				m_l_bkt_scanned.resize(ch_max + 1);
				
				m_sa_s_reader = new typename size_vector_type::bufreader_reverse_type(*_sa);
					
				m_lcp_s_reader = new typename size_vector_type::bufreader_reverse_type(*_lcp);
				
				typename size_vector_type::const_reverse_iterator sa_l_rbeg = xxx;
					
				m_sa_l_reader = new typename size_vector_type::bufreader_type(*sa_l_rbeg);
					
				typename size_vector_type::const_reverse_iterator lcp_l_rbeg = xxx;
					
				m_lcp_l_reader = new typename size_vector_type::bufreader_type(*lcp_l_rbeg);
					
				m_sa_s_bkt_reader.resize(ch_max + 1);
				
				m_lcp_s_bkt_reader.resize(ch_max + 1);
				
				m_sa_l_bkt_reader.resize(ch_max + 1);
				
				m_lcp_l_bkt_reader.resize(ch_max + 1);
					
				typename size_vector_type::const_reverse_iterator sa_rbeg = _sa->rbegin();
					
				typename size_vector_type::const_reverse_iterator lcp_rbeg = _lcp->rbegin();
					
				for (alphabet_type ch = ch_max; ch >= 0; --ch) {
					
					if (m_s_bkt_toscan[ch] != 0) {

						m_sa_s_bkt_reader[ch] = new typename size_vector_type::bufreader_type(sa_rbeg, sa_rbeg + m_s_bkt_toscan[ch]);
						
						m_lcp_s_bkt_reader[ch] = new typename size_vector_type::bufreader_type(lcp_rbeg, lcp_rbeg + m_s_bkt_toscan[ch]);
							
						sa_rbeg = sa_rbeg + m_s_bkt_toscan[ch];
					
						lcp_rbeg = lcp_rbeg + m_s_bkt_toscan[ch];
					}
					else {
						
						m_sa_s_bkt_reader[ch] = nullptr;
						
						m_lcp_s_bkt_reader[ch] = nullptr;			
					}
					
					if (m_l_bkt_toscan[ch] != 0) {
						
						m_sa_s_bkt_reader[ch] = new typename size_vector_type::bufreader_type(sa_rbeg, sa_rbeg + m_l_bkt_toscan[ch]);
						
						m_lcp_s_bkt_reader[ch] = new typename size_vector_type::bufreader_type(lcp_rbeg, lcp_rbeg + m_l_bkt_toscan[ch]);
							
						sa_rbeg = sa_rbeg + m_l_bkt_toscan[ch];
					
						lcp_rbeg = lcp_rbeg + m_l_bkt_toscan[ch];

					}
					else {
						
						m_sa_l_bkt_reader[ch] = nullptr;
						
						m_lcp_l_bkt_reader[ch] = nullptr;			
					}		
					
					if (ch == 0) break; // break dead loop
				}
		}
		
		/// \brief no more LMS to be scanned
		bool s_is_empty() {
			
			return total_s_toscan == total_s_scanned;	
		}
		
		/// \brief no more L-type to be scanned
		///
		bool l_is_empty() {
		
			return total_l_toscan == total_l_scanned;		
		}
		
		/// \brief move forward and get the head character of next S-type suffix to scan
		///
		/// \note guarantee that there still exist S-type suffixes to be scanned
		void forward_s(alphabet_type& _cur_ch) {
			
			// jump over curent S-type bucket if finished reading and skip over all the empty S-type buckets
			while (m_s_bkt_scanned[_cur_ch] == m_s_bkt_toscan[_cur_ch]) {
				
				--_cur_ch;
			}	
			
			// start scanning the new S-bucket
			if (m_s_bkt_scanned[_cur_ch] = 0) {
				
				delete m_sa_s_reader;
				
				typename size_vector_type::const_reverse_iterator sa_rbeg = _sa->rbegin() + m_s_bkt_spos[_cur_ch];
					
				m_sa_s_reader = new typename size_vector_type::bufreader_reverse_type(sa_rbeg, sa_rbeg + m_s_bkt_toscan[_cur_ch]);
					
				delete m_lcp_s_reader;
				
				typename size_vector_type::const_reverse_iterator lcp_rbeg = _lcp->rbegin() + m_s_bkt_spo[_cur_ch];
					
				m_lcp_s_reader = new typename size_vector_type::bufreader_reverse_type(lcp_rbeg, lcp_rbeg + m_s_bkt_toscan[_cur_ch]);
			}
			
			++m_s_bkt_scanned[_cur_ch];
			
			return;
		}
		
		/// \brief return the head character of next suffix to scan
		///
		/// \note guarantee that there still exist L-type suffixes to be scanned
		void forward_l(alphabet_type& _cur_ch) {
			
			// jump over current L-type bucket if finished reading and skip all the empty L-type buckets
			while (m_l_bkt_scanned[_cur_ch] == m_l_bkt_toscan[_cur_ch])	{

				++_cur_ch;
			}
	
			// start scanning the new L-bucket
			if (m_l_bkt_scanned[_cur_ch] == 0) {
				
				delete m_sa_l_reader; 
				
				typename size_vector_type::const_reverse_iterator sa_beg = _sa->rbegin() + m_l_bkt_spos[_cur_ch];
					
				m_sa_l_reader = new typename size_vector_type::bufreader_reverse_type(sa_beg, sa_beg + m_l_bkt_toscan[_cur_ch]);
				
				delete m_lcp_l_reader;
				
				typename size_vector_type::const_reverse_iterator lcp_beg = _lcp->rbegin() + m_l_bkt_spos[_cur_ch];
					
				m_lcp_l_reader = new typename size_vector_type::bufreader_reverse_type(lcp_beg, lcp_beg + m_l_bkt_toscan[_cur_ch]);
			}
			
			++m_l_bkt_scanned[_cur_ch];	
						
			return;
		}
		

		
		/// \brief de-ctor
		~LScan() {
		
		}
		
		/// \brief
		/// 
		/// case 1: if cur_t == S_TYPE, the LCP-value of current suffix and its right neighbor can be retrieved from LCP. Update RMQ with the LCP-value and then induce the preceding suffix;
		///					if pre_t == L_TYPE, the LCP-value of preceding suffix and its left neighbor can be calculated by the following sub-cases:
		///         sub-case (a): pre_ch is the rightmost S-type character in the bucket, then the LCP-value must be 0;
		///		      sub-case (b): otherwise, the LCP-value is 1 + RMQ_VALUE
		/// case 2: if cur_t == S_TYPE, the LCP-value of current suffix and its right neighbor can be retrieved following the two sub-cases below:
		///         sub-case (a): cur_ch is the leftmost S-type character in the S-bucket but not the bucket, then the LCP-value must be calculated by a literal string comparison between it and the rightmost L-type suffix in the bucket.
		///					sub-case (b): otherwise, the LCP-value can be directly retreived from LCP_LMS.
		///					Update RMQ with the LCP-value and then induce the preceding suffix;
		///					if pre_ch == L_TYPE, the LCP-value of preceding suffix and its left neighor can be calculated by the following sub-cases:
		///					sub-case (a): pre_ch is the leftmost L-type character in the bucket, then the LCP-value must be 0;
		///					sub-case (b): otherwise, the LCP-value is 1 + RMQ_VALUE
		void run(const triple2_less_sorter_type* _sorter){
			
			alphabet_type cur_bkt_ch, cur_l_ch, cur_lms_ch;
			
			size_type sv_scanned, lv_scanned;
			
			size_type sv_induced, lv_induced;
			
			while (true) {
				
				// scan l-type suffixes
				while (!l_is_empty() && cur_l_ch == cur_bkt_ch) {
	
					// retrieve sa-value and lcp-value from sa and lcp
					size_type sv_scanned = *(*m_sa_l_reader);

					size_type lv_scanned = *(*m_lcp_l_reader);
					
					// update RMQ
					rmq_l.update(cur_bkt_ch, lv);
					
					// retrieve pre_ch & pre_t from m_l_pre;
					alphabet_type pre_ch = *(*_sorter)->second;
					
					alphabet_type pre_t = *(*_sorter)->third;
					
					//
					if (pre_t == L_TYPE) {
						
							// induce		
							sv_induced = sv_scanned - 1;
							
							if () { // first induced into the bucket
								
								lv_induced = 0;
							}
							else {
								
								lv_induced = rmq_l.get(cur_bkt_ch);	
							}
						
							// validate
							if (sa_induced != *(*m_sa_l_bkt_reader[ch]) || lv_induced != *(*m_lcp_l_bkt_reader)) {
								
								std::cerr << "wrong";
									
									exit();	
							}
					}
											
					pre_l_ch = cur_l_ch;
					
					pre_sv_scanned = sv_scanned;
						
					forward_l(cur_l_ch);
				}
				
				// scan leftmost lms suffix in the bucket
				if (!lms_is_empty() && cur_lms_ch == cur_bkt_ch) {
					
					// retrieve sa-value and lcp-value from sa_lms and lcp_lms
					sv_scanned = *(*m_sa_lms_reader);	++(*m_sa_lms_reader);		
	
					lv_scanned = *(*m_lcp_lms_reader); ++(*m_lcp_lms_reader);

					if (pre_l_ch == cur_bkt_ch) { // left neighbor is a L-type suffix in the same bucket, must recompute lv_scanned
						
						lv_scanned = bruteforce_compute_lcp(sv_scanned, pre_sv_scanned);
					}						
					
					// update RMQ
					rmq_l.update(cur_bkt_ch, lv_scanned);
					
					// induce
					sv_induced = sv_scanned - 1;
					
					if () { // first induced into the bucket
						
						lv_induced = 0;	
					}
					else {
						
						lv_induced = rmq_l.get(cur_bkt_ch);
					}
					
					// validate
					if (sa_induced != *(*m_sa_l_bkt_reader[ch]) || lv_induced != *(*m_lcp_l_bkt_reader)) {
					
						std::cerr << "wrong";
						
						exit();	
					}				
					
					pre_l_ch = cur_l_ch;
					
					pre_sv_scanned = sv_scanned;
										
					next_lms(cur_lms_ch);
				}
				
				// scan the remaining lms suffixes
				while (!lms_is_empty() && cur_lms_ch == cur_bkt_ch) {
					
					// retrieve sa-value and lcp-value from sa_lms and lcp_lms
					sv_scanned = *(*m_sa_lms_reader); ++(*m_sa_lms_reader);		
					
					lv_scanned = *(*m_lcp_lms_reader); ++(*m_lcp_lms_reader);
					
					// update RMQ
					rmq_l.update(cur_bkt_ch, lv_scanned);
					
					// induce
					sv_induce = sv_scanned - 1;
					
					if () { // first induced into the bucket
						
						lv_induced = 0;
					}
					else {
						
							lv_induced = rmq_l.get(cur_bkt_ch);
					}
					
					// validate
					if (sa_induced != *(*m_sa_l_bkt_reader[ch]) || lv_induced != *(*m_lcp_l_bkt_reader)) {
					
						std::cerr << "wrong";
						
						exit();	
					}				
					
					pre_l_ch = cur_l_ch;
					
					pre_sv_scanned = sv_scanned;
										
					next_lms(cur_lms_ch);
				}
				
				// determine next bucket
				if (!l_is_empty()) {
					
					cur_bkt_ch = cur_lms_ch;
	
					if (!lms_is_empty() && cur_lms_ch < cur_bkt_ch) {
					
						cur_bkt_ch = cur_lms_ch;
					}
				}
				else if (!lms_is_empty()) {
					
					cur_bkt_ch = cur_lms_ch;	
				}
				else {
					
					break;	
				}
			}
		}
	};
};





#endif // VALIDATE4_H