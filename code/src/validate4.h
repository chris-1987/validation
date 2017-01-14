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
/// \param alphabet_type for elements in T
/// \param size_type for elements in SA/LCP
template<typename alphabet_type, typename size_type>
class Validate4{

private:
  // alias for vectors, tuples, sorters and comparators
  
  // vectors
  typedef typename ExVector<alphabet_type>::vector alphabet_vector_type;

  typedef typename ExVector<size_type>::vector size_vector_type;

  // tuples, sorters and comparators
  typedef pair<size_type, size_type> pair1_type;

  typedef tuple_less_comparator_1st<pair1_type> pair1_less_comparator_type;

  typedef typename ExTupleSorter<pair1_type, pair1_less_comparator_type>::sorter pair1_less_sorter_type;

  typedef tuple_great_comparator_1st<pair1_type> pair1_great_comparator_type;

  typedef typename ExTupleSorter<pair1_type, pair1_great_comparator_type>::sorter pair1_great_sorter_type;

  typedef pair<size_type, fpa_type> pair2_type;

  typedef tuple_less_comparator_1st<pair2_type> pair2_less_comparator_type;

  typedef typename ExTupleSorter<pair2_type, pair2_less_comparator_type>::sorter pair2_less_sorter_type;

  typedef pair<size_type, uint8> pair3_type;

  typedef tuple_less_comparator_1st<pair3_type> pair3_less_comparator_type;

  typedef typename ExTupleSorter<pair3_type, pair3_less_comparator_type>::sorter pair3_less_sorter_type;

  typedef triple<size_type, fpa_type, uint16> triple1_type;

  typedef tuple_less_comparator_1st<triple1_type> triple1_less_comparator_type;

  typedef ExTupleSorter<triple1_type, triple1_less_comparator_type>::sorter triple1_less_sorter_type;

  typedef triple<size_type, uint8, uint8> triple2_type;

  typedef tuple_less_comparator_1st<triple2_type> triple2_less_comparator_type;

  typedef ExTupleSorter<triple2_type, triple2_less_comparator_type>::sorter triple2_less_sorter_type;
  	
  typedef triple<size_type, fpa_type, uint16> triple3_type;
  
  typedef tuple_less_comparator_1st<triple3_type> triple3_less_comparator_type;
  
  typedef ExTupleSorter<triple3_type, triple3_less_comparator_type>::sorter triple3_less_sorter_type;

private:

	/// \brief record bkt size for L-type, S-type buckets (no matter empty or non-empty) in SA & LCP
	///
  struct BktInfo{
  private:

	  const alphabet_type ch_max; ///< maximum value for alphabet_type
		
    std::vector<uint64> m_bkt_size; ///< size for each bucket

    std::vector<uint64> m_l_bkt_size; ///< size for each L-type bucket

    std::vector<uint64> m_s_bkt_size; ///< size for each S-type bucket

    std::vector<uint64> m_lms_bkt_size; ///< size for each LMS bucket (number of LMS in the corresponding S-type bucket)

  public:

    /// \brief ctor
    ///
    BktInfo() :ch_max(std::numeric_limits<alphabet_type>::max()) {

      m_bkt_size.resize(ch_max + 1);

      m_l_bkt_size.resize(ch_max + 1);

      m_s_bkt_size.resize(ch_max + 1);

      m_lms_bkt_size.resize(ch_max + 1);
    }

    /// \brief count the number of L-type suffixes for each bucket
    ///
    void add_l(const alphabet_type _ch) {

      ++m_l_bkt_size[_ch];

      return;
    }

    /// \brief count the number of S-type suffixes for each bucket
    ///
    void add_s(const alphabet_type _ch) {

      ++m_s_bkt_size[_ch];

      return;
    }

    /// \brief count the number of LMS suffixes for each bucket
    ///
    void add_lms(const alphabet_type _ch) {

      ++m_lms_bkt_size[_ch];

      return;
    }

    /// \brief count the number of suffixes for each bucket
    ///
    void accumulate() {

      for (alphabet_type ch = 0; ch <= ch_max; ++ch) {

        m_bkt_size[ch] = m_l_bkt_size[ch] + m_s_bkt_size[ch];
      }

      return;
    }

    /// \brief return the number of L-type suffixes in a bucket
    ///
    uint64 get_l(const alphabet_type _ch) {

      return m_l_bkt_size[_ch];
    }

    /// \brief return the number of S-type suffixes in a bucket
    ///
    void get_s(const alphabet_type _ch) {

        return m_s_bkt_size[_ch];
    }

    /// \brief return the number of LMS suffixes in a bucket
    ///
    void get_lms(const alphabet_type _ch) {

      return m_lms_bkt_size[_ch];
    }

    /// \brief return the number of suffixes in a bucket
    ///
    void get(const alphabet_type _ch) {

      return m_bkt_size[_ch];
    }

  };
  
  BktInfo& m_bkt_info; ///< instance of BktInfo
  
  
  /// \brief validate SA_LMS and LCP_LMS
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
  	
  	alphabet_vector_type* m_t;
  	
  	size_vector_type* m_sa;
  	
  	size_vector_type* m_lcp;
  	
  	uint64 m_len;
  	
  	RInterval* m_rinterval;
  	
  	LMSValidate(alphabet_vector_type* _t, size_vector_type* _sa, size_vector_type* _lcp) :
  		m_t(_t), m_sa(_sa), m_lcp(_lcp) {
  			
  		m_len = m_t->size();
  		
  		m_rinterval = new RInterval(m_len);	
  	}
  	
  	/// \brief retrive SA_LMS and LCP_LMS from SA and LCP
  	void retrieve_lms();
  	
  	/// \brief core part
  	bool run() {
  		
  		// step 1: fetch fp[0, SA[i] - 1]
  		pair2_less_sorter_type* sorter1 = fetch_fp();
  		
  		// step 2: fetch fp[0, SA[i] + LCP[i] - 1]
  		triple3_less_sorter_type* sorter2 = fetch_fp_ch_cur();
  		
  		// step 3: fetch fp[0, SA[i] + LCP[i + 1] - 1]
  		triple3_less_sorter_type* sorter3 = fetch_fp_ch_pre();
  		
  		// step 4: check 
  		bool res = check(sorter1, sorter2, sorter3);
  	}
  	
  	
  };
  
  
  
  /// \brief RMQ
  template<bool rightward>
  struct RMQ{
  private:
  	
  	const alphabet_type ch_max;
  	
  	const alphabet_type val_max;
  	
  	std::vector<size_type> m_rmq;
  		
  public:
  	
  	RMQ() ch_max(std::numeric_limits<alphabet_type>::max()), val_max(std::numeric_limits<size_type>::max()) {
  		
  		m_rmq.resize(ch_max + 1);
  	}
  		
	  /// \brief get 
	  size_type get(const alphabet_type _ch) {
	  		
	  	return m_rmq[_ch];
	  }
	  
	  /// \brief update
	  void update(size_type _val) {
	  	
	  	for (alphabet_type ch = 0; ch <= ch_max; ++ch) {
	  	
	    		if (m_rmq[ch] > _val) m_rmq[ch] = _val;		
	  	}
	  }
  	
  	/// \brief reset
  	void reset(const alphabet_type _ch) {
  		
  			m_rmq[_ch] = val_max;
  	}
  	
  };

  /// \brief validate the SA-values of all the L-type suffixes and the LCP-values btw them and their left neighbors in SA.
  ///
  struct RScan{

  private:
    const alphabet_type ch_max;

		alphabet_vector_type* m_t;
		
		size_vector_type* m_sa;
		
		size_vector_type* m_lcp;
		
		size_vector_type* m_sa_lms;
		
		size_vector_type* m_lcp_lms;
		
    std::vector<uint64> m_l_bkt_toscan; ///< number of L-type suffixes to be scanned in each bucket

    std::vector<uint64> m_l_bkt_scanned; ///< number of L-type suffixes scanned in each bucket

    std::vector<uint64> m_lms_bkt_toscan; ///< number of LMS suffixes to be scanned in each bucket

    std::vector<uint64> m_lms_bkt_scanned; ///< number of LMS suffixes scanned in each bucket

    std::vector<uint64> m_l_bkt_spos; ///< indicate the starting position of each L-type bucket in SA

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
    	ch_max (std::numeric_limits<alphabet_type>::max()), m_t(_t), m_sa(_sa), m_lcp(_lcp), m_sa_lms(_sa_lms), m_lcp_lms(_lcp_lms){

        m_total_l_toscan = m_total_l_scanned = 0;

        m_total_lms_toscan = m_total_lms_scanned = 0;

        uint64 spos = 0;

        for (alphabet_type ch = 0; ch <= ch_max; ++ch) {

            m_l_bkt_toscan.push(_bkt_info.get_l(ch));

            m_lms_bkt_toscan.push(_bkt_info.get_lms(ch));

            m_total_l_toscan += m_l_bkt_toscan[ch];

            m_total_lms_toscan += m_lms_bkt_toscan[ch];

            m_l_bkt_spos.push(spos);

            spos += _bkt_info.get(ch); // offset by bucket size
        }

        typename size_vector_type::const_iterator sa_beg = m_sa->begin();

        typename size_vector_type::const_iterator lcp_beg = m_lcp->begin();
        	
				for (alphabet_type ch = 0; ch <= ch_max; ++ch) {
					
						if (m_l_bkt_toscan[ch] != 0) {
							
							m_cur_l_ch = ch; 
							
							m_sa_l_reader = new typename size_vector_type::bufreader_type(sa_beg, sa_beg + m_l_bkt_toscan[ch]);

       				m_lcp_l_reader = new typename size_vector_type::bufreader_type(lcp_beg, lcp_beg + m_l_bkt_toscan[ch]);		
        	
							break;
						}
							
						sa_beg += _bkt_info.get(ch);
						
						lcp_beg += _bkt_info.get(ch);
				}			
					
				for (alphabet_type ch = 0; ch <= ch_max; ++ch) {
									
						if (m_lms_bkt_toscan[ch] != 0) {m_cur_lms_ch = ch; break;}			
				}
					
        m_l_bkt_scanned.resize(ch_max + 1);

        m_lms_bkt_scanned.resize(ch_max + 1);

        m_sa_lms_reader = new typename size_vector_type::bufreader_type(*m_sa_lms);

        m_lcp_lms_reader = new typename size_vector_type::bufreader_type(*m_lcp_lms);

        m_sa_l_bkt_reader.resize(ch_max + 1);

        m_lcp_l_bkt_reader.resize(ch_max + 1);

        sa_beg = m_sa->begin();

        lcp_beg = m_lcp->begin();

        for (alphabet_type ch = 0; ch <= ch_max; ++ch) {

          if (m_l_bkt_toscan[ch] != 0) {

            m_sa_l_bkt_reader[ch] = new typename size_vector_type::bufreader_type(sa_beg, sa_beg + m_l_bkt_toscan[ch]);

            m_lcp_l_bkt_reader[ch] = new typename size_vector_type::bufreader_type(lcp_beg, lcp_beg + m_l_bkt_toscan[ch]);
					}
					else {
						
						m_sa_l_bkt_reader[ch] = nullptr;
						
						m_lcp_l_bkt_reader[ch] = nullptr;	
					}
					
          sa_beg = sa_beg + _bkt_info.get(ch));

          lcp_beg = lcp_beg + _bkt_info.get(ch));
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
		bool l_bkt_is_empty() {
			
			return m_l_bkt_scanned[m_cur_l_ch] == m_l_bkt_toscan[m_cur_l_ch];	
		}
		
		/// \brief check if no more LMS suffixes to be scanned in current bucket
		///
		bool lms_bkt_is_empty() {
		
			return m_lms_bkt_scanned[m_cur_lms_ch] == m_lms_bkt_toscan[m_cur_l_ch];
		}
		
    /// \brief find next non-empty L-bucket
    ///
    /// \note guarantee that there still exist L-type suffixes to be scanned before calling the function
    void find_next_l_bkt() {
			
      // if finished reading current L-type bucket, then find a non-empty L-type bucket
      // because there exist at least one element to be read, we must have m_l_bkt_scanned[ch] == m_l_bkt_toscan[ch] for some ch <= ch_max
      while (l_bkt_is_empty()) {

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
      while (lms_bkt_is_empty()) {

        ++m_cur_lms_ch;
      }

			// we do not need to relocate pointers to SA_LMS and LCP_LMS
			// do nothing
			
      return;
    }
    
    /// \brief fetch the SA-value for currently scanned L-type suffix and the LCP-value of the suffix and its left neighbor in SA
    ///
    void fetch_l_scanned(size_type& _sa_value, size_type& _lcp_value) {
    		
    	_sa_value = *(*m_sa_l_reader), ++(*m_sa_l_reader);
    	
    	_lcp_value = *(*m_lcp_l_reader), ++(*m_sa_l_reader);
    	
    	++m_l_bkt_scanned[m_cur_l_ch];
    	
    	++total_l_scanned;
    	
    	return;
    }


		/// \brief fetch the SA-value for currently scanned LMS suffix and the LCP-value for the suffix and its left neighbor in SA_LMS
		///
		void fetch_lms_scanned(size_type& _sa_value, size_type& _lcp_value) {
			
			_is_leftmost_lms_in_bkt = (m_lms_scanned[m_cur_l_ch] == 0) ? true : false;
				
			_sa_value = *(*m_sa_lms_reader), ++(*m_sa_lms_reader);
			
			_lcp_value *(*m_lcp_lms_reader), ++(*m_lcp_lms_reader);	
	
			++m_lms_bkt_scanned[m_cur_lms_ch];
			
			++total_lms_scanned;
			
			return;
		}
		
		/// \brief fetch the SA-value for currently induced L-type suffix and the LCP-value for the suffix and its left neighbor in SA
		void fetch_l_induced(const alphabet_type _ch, size_type& _sa_value, size_type& _lcp_value) {
			
			_sa_value = *(*m_sa_l_bkt_reader[_ch]), ++(*m_sa_l_bkt_reader[_ch]);
			
			_lcp_value = *(*m_lcp_l_bkt_reader[_ch]), ++(*m_lcp_l_bkt_reader[_ch]);
			
			return;
		}
			
		
		/// \brief get ch for the L-type bucket currently being scanned
		///
		alphabet_type get_l_bkt_ch() {
			
			return m_cur_l_ch;
		}
		
		/// \brief get ch for the LMS bucket currently being scanned
		//
		alphabet_type get_lms_bkt_ch() {
		
			return m_cur_lms_ch;
		}

		
		/// \brief dtor
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

    /// \brief for each L-type suffix, check the SA-value in SA and the LCP-value between the suffix and its left neighbor in SA.
    ///
    /// case 1: scan: if currently scanned suffix is L-type, then the LCP-value of the suffix and its left neighbor in SA can be retrieved following the sub-cases below:
    ///										sub-case (a): if currently scanned suffix is the leftmost L-type suffix in the bucket, then we instead compute the LCP-value for the suffix and the last scannned one by performing a literal comparison between them (actually, it must be 0 if correct).
    ///										sub-case (b): otherwise, the LCP-value can be directly retrieved from LCP.
    ///								update RMQ with the LCP-value.
    ///         induce: if the preceding suffix is L-type, then the LCP-value of the suffix and its left neighbor in SA can be calculated following the sub-cases below:
    ///                   sub-case (a): if the preceding suffix is the leftmost L-type suffix in the L-type bucket, then we skip it as each S-type bucket contains only LMS ones currently.
    ///                   sub-case (b): otherwise, the LCP-value is 1 + RMQ_VALUE.
    ///								reset the RMQ_VALUE
    ///               validate the correctness of the induced SA-value (that for currently scanned suffix minus 1) and the induced LCP-value (only sub-case (b)) by comparing them with the corresponding values in SA and LCP.
    /// case 2: scan: if currently scanned suffix is LMS, then the LCP-value of the suffix and its left neighbor in SA_LMS can be retrieved following the sub-cases below:
    ///                   sub-case (a): if currently scanned suffix is the leftmost LMS suffix in the bucket, then we instead compute the LCP-value between the suffix and the last scanned one by performing a literal comparison between them.
    ///                   sub-case (b): otherwise, the LCP-value can be directly retrieved from LCP_LMS.
    ///               update RMQ with the LCP-value.
    ///         induce: if the preceding suffix is L-type, then the LCP-value of the suffix and its left neighbor in SA can be calculated following the sub-cases below:
    ///                   sub-case (a): if the preceding suffix is the leftmost L-type suffix in the L-type bucket, then we skip it as each S-type bucket contains only LMS ones currently.
    ///                   sub-case (b): otherwise the LCP-value is 1 + RMQ_VALUE.
    ///								reset the RMQ_VALUE
    ///               validate the correctness of the induced SA-value (that for currently scanned suffix minus 1) and the induced LCP-value (only sub-case (b)) by comparing them with the corresponding values in SA and LCP.
    ///
    /// \note In each bucket, the LCP-value of the leftmost L-type suffix and its left neighbor in SA is not verified yet.
    void run(const triple2_less_sorter_type* _pre_l_items, const triple2_less_sorter_type* _pre_lms_items){

      alphabet_type cur_bkt_ch, pre_ch;
      
      uint8 pre_t;

      size_type sv_cur_scanned, lv_cur_scanned, sv_last_scanned, lv_last_scanned;
      
      size_type sv_induced, lv_induced, sv_induced_fetch, lv_induced_fetch; // fetch from SA and LCP for validation
      
      RMQ<true> rmq_l;

			{ // process the virtual sentinel
				// scan
				sv_cur_scanned = m_sa->size(); // assume to be n
				
				lv_cur_scanned = 0; // no left neighbor in SA, assumed to be 0
				
				// update RMQ
				rmq_l.update(lv_cur_scanned); // update all the entries [0, ch_max] in RMQ
				
				// induce and valiadte
				auto rit = m_t->rbegin();
				
				pre_ch = *rit, pre_t = L_TYPE; // must be L_TYPE

				fetch_l_induced(pre_ch, sv_induced_fetch, lv_induced_fetch);
						
				sv_induced = sv_cur_scanned - 1;
				
				if (sv_induced_fetch != sv_induced) {
					
					std::cerr << "wrong\n";
						
					exit(0);
				}
						
				// lv_induced = xxx; // must be leftmost L-type in the bucket, skip it.
							
				// reset RMQ
				rmq_l.reset(pre_ch);
							
				//
				sv_last_scanned = sv_cur_scanned;
				
				lv_last_scanned = lv_cur_scanned;
			}
			
			{ // process suffixes in the following buckets
				
				bool flag; // indicate current scanned L-type/LMS suffix is the leftmost suffix in the L-type/LMS bucket
				
				bool flags[ch_max + 1] = {true}; // indicate current induced L-type suffix is the leftmost suffix in the L-type bucket, must be initialized to true in advance
				 
				// determine the bucket
				cur_bkt_ch = get_l_bkt_ch(); // there must exist at least one L-type suffix (consisting of the rightmost character, in front of the virtual sentinel)
			
				if (!lms_is_empty() && get_lms_bkt_ch() < cur_bkt_ch) {
				
					cur_bkt_ch = get_lms_bkt_ch();	
				}
				
				while (true) {
					
					flag = true;
					
					// scan L-type bucket
					while (!l_bkt_is_empty() && get_l_bkt_ch() == cur_bkt_ch) {
						
						//scan
						fetch_l_scanned(sv_cur_scanned, lv_cur_scanned);
						
						if (flag == true){// leftmost l-type suffix in the bucket
						
							//instead of using the value retrieved from LCP, we recompute the LCP-value of current suffix and the last scanned suffix
							lv_cur_scanned = (sv_last_scanned == m_sa->size()) ? 0 : bruteforce_compute_lcp(sv_last_scanned, sv_cur_scanned); // last scanned is the sentinel, then 0; otherwise, calling the brute-force method
							
							flag = false;
						}
						
						// update RMQ
						rmq_l.update(lv_cur_scanned); // update all the entries [0, ch_max] in RMQ

						// induce and validate
						pre_ch = *(*pre_l_items).second, pre_t = *(*pre_l_items).third, ++(*pre_l_items);
						
						if (pre_t == L_TYPE) {
											
							fetch_l_induced(pre_ch, sv_induced_fetch, lv_induced_fetch);
							
							sv_induced = sv_cur_scanned - 1;
						
							if (sv_induced != sv_induced_fetch) {
							
									std::cerr << "wrong\n"; 
									
									exit(0);
							}
						
							if (flags[cur_bkt_ch] == true){ // the induced L-type suffix is the leftmost suffix in the L-type bucket
						
								// skip computing lv_induced		
								flags[cur_bkt_ch] = false;	
							}
							else {
							
								lv_induced = rmq_l.get(cur_bkt_ch) + 1;
								
								if (lv_induced != lv_induced_fetch) {
								
									std::cerr << "wrong\n";
									
									exit(0);	
								}
							}
							
							// reset RMQ
							rmq_l.reset(pre_ch);
						}
			
						//
						sv_last_scanned = sv_cur_scanned;
						
						lv_last_scanned = lv_cur_scanned;
					}	
					
					flag = true;
					
					// scan S-type bucket
					while (!lms_bkt_is_empty() && get_lms_bkt_ch() == cur_bkt_ch) {
					
						//scan
						fetch_l_scanned(sv_cur_scanned, lv_cur_scanned, is_leftmost_l_in_bkt);
						
						if (flag == true) { // leftmost l-type suffix in the bucket
							
								// instead of using the value retrieved from LCP_LMS, we compute the LCP-value of current suffix and the last scanned	suffix
								lv_cur_scanned = bruteforce_compute_lcp(sv_last_scanned, sv_cur_scanned;
						}
						
						// update RMQ
						rmq_l.update(lv_cur_scanned); // update all the entries [0, ch_max] in RMQ

						// induce and validate
						pre_ch = *(*pre_l_items).second, pre_t = *(*pre_l_items).third, ++(*pre_l_items);
						
						if (pre_t == L_TYPE) {
							
							fetch_l_induced(pre_ch, sv_induced_fetch, lv_induced_fetch);
												
							sv_induced = sv_cur_scanned - 1;
						
							if (sv_induced != sv_induced_fetch) {
								
									std::cerr << "wrong\n"; 
											
									exit(0);
							}
						
							if (flags[cur_bkt_ch] == true){ // the induced L-type suffix is the leftmost suffix in the L-type bucket
						
								// skip computing lv_induced
								flags[cur_bkt_ch] = false;		
							}
							else {
							
								lv_induced = rmq_l.get(cur_bkt_ch) + 1;
								
								if (lv_induced != lv_induced_fetch) {
								
									std::cerr << "wrong\n";
										
									exit(0);	
								}
							}
							
							// reset RMQ
							rmq_l.reset(pre_ch);
						}
			
						//
						sv_last_scanned = sv_cur_scanned;
						
						lv_last_scanned = lv_cur_scanned;
					}	
					
					// determine next 
					if (!l_is_empty()) {
						
						find_next_l_bkt();
						
						cur_bkt_ch = get_l_bkt_ch();
						
						if (!lms_is_empty()) {
							
							find_next_lms_bkt();
							
							if (cur_bkt_ch > get_lms_bkt_ch())	{
								
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


  /// \brief valiadte the SA-values of all the S-type suffixes and the LCP-values btw them and their left neighbors in SA.
  ///
  struct LScan{

  private:

    const alphabet_type ch_max;

		alphabet_vector_type* m_t;
		
		size_vector_type* m_sa;
		
		size_vector_type* m_lcp;
		
    std::vector<uint64> m_s_bkt_toscan; ///< number of S-type suffixes to be scanned in aech bucket

    std::vector<uint64> m_s_bkt_scanned; ///< number of S-type suffixes scanned in each bucket

    std::vector<uint64> m_l_bkt_toscan; ///< number of L-type suffixes to be scanned in each bucket

    std::vector<uint64> m_l_bkt_scanned; ///< numer of L-type suffixes scanned in each bucket

    std::vector<uint64> m_s_bkt_spos; ///< indicate the starting position of each L-type bucket in SA (from right to left)

    std::vector<uint64> m_l_bkt_spos; ///< indicate the starting position of each S-type bucket in SA (from right to left)

    uint64 total_s_toscan; ///< total number of S-type suffixes to be scanned

    uint64 total_s_scanned; ///< total number of S-type suffixes already scanned

    uint64 total_l_toscan; ///< total number of L-type suffixes to be scanned

    uint64 total_l_scanned; ///< total number of L-type suffixes already scanned

		alphabet_type m_cur_s_ch; ///< S-type bucket currently being scanned
		
		alphabet_type m_cur_l_ch; ///< L-type bucket currently being scanned
		
    typename size_vector_type::bufreader_reverse_type* m_sa_reader; ///< for scan, point to the SA-value for currently scanned suffix (retrieve from SA)

    typename size_vector_type::bufreader_reverse_type* m_lcp_reader; ///< for scan, point to the LCP-value for currently scanned suffix and its right neighbor in SA (retrieve from LCP)

    std::vector<typename size_vector_type::bufreader_reverse_type*> m_sa_s_bkt_reader; ///< for induce, point to the SA-value for the S-type suffix next to be induced in each bucket (retrieve from SA)

    std::vector<typename size_vector_type::bufreader_reverse_type*> m_lcp_s_bkt_reader; ///< for induce, point to the LCP-value for the S-type suffix next to be induced in each bucket and its right neighbor in SA (retrieve from LCP) 

    /// \brief ctor
    ///
    RScan(const BktInfo& _bkt_info, alphabet_type* _t, size_vector_type* _sa, size_vector_type* _lcp) : 
    	ch_max (std::numeric_limits<alphabet_type>::max()), m_sa(_sa), m_lcp(_lcp) {

        m_total_s_toscan = m_total_s_scanned = 0;

        m_total_l_toscan = m_total_l_scanned = 0;

        uint64 spos = 0;

        for (alphabet_type ch = ch_max; ch >= 0; --ch) {

            m_s_bkt_toscan.push(_bkt_info.get_s(ch));

            m_l_bkt_toscan.push(_bkt_info.get_l(ch));

            m_total_s_toscan += m_s_bkt_toscan[ch];

            m_total_l_toscan += m_l_bkt_toscan[ch];

            m_s_bkt_spos.push(spos);

            m_l_bkt_spos.push(spos + _bkt_info.get_s(ch));

            spos += _bkt_info.get(ch);

            if (ch == 0) break; // break dead loop
        }

				for (alphabet_type ch = ch_max; ch >= 0; --ch) {
					
					if (m_l_bkt_toscan[ch] != 0) {m_cur_l_ch = ch; break;}
						
					if (ch == 0) break;
				}
				
				for (alphabet_type ch = ch_max; ch >= 0; --ch) {
					
					if (m_s_bkt_toscan[ch] != 0) {m_cur_s_ch = ch; break;}
						
					if (ch == 0) break;	
				}
					
        m_s_bkt_scanned.resize(ch_max + 1);

        m_l_bkt_scanned.resize(ch_max + 1);

        m_sa_reader = new typename size_vector_type::bufreader_reverse_type(*m_sa);

        m_lcp_reader = new typename size_vector_type::bufreader_reverse_type(*m_lcp);

        m_sa_s_bkt_reader.resize(ch_max + 1);

        m_lcp_s_bkt_reader.resize(ch_max + 1);

        typename size_vector_type::const_reverse_iterator sa_rbeg = _sa->rbegin();

        typename size_vector_type::const_reverse_iterator lcp_rbeg = _lcp->rbegin();

        for (alphabet_type ch = ch_max; ch >= 0; --ch) {

          if (m_s_bkt_toscan[ch] != 0) {

            m_sa_s_bkt_reader[ch] = new typename size_vector_type::bufreader_type(sa_rbeg, sa_rbeg + m_s_bkt_toscan[ch]);

            m_lcp_s_bkt_reader[ch] = new typename size_vector_type::bufreader_type(lcp_rbeg, lcp_rbeg + m_s_bkt_toscan[ch]);
          }
          else {

            m_sa_s_bkt_reader[ch] = nullptr;

            m_lcp_s_bkt_reader[ch] = nullptr;
          }

          sa_rbeg = sa_rbeg + _bkt_info.get(ch);

          lcp_rbeg = lcp_rbeg + _bkt_info.get(ch);
            
          if (ch == 0) break; // break dead loop
        }
    }

    /// \brief check if no more LMS to be scanned
    bool s_is_empty() {

      return m_total_s_toscan == 	m_total_s_scanned;
    }

    /// \brief check if no more L-type to be scanned
    ///
    bool l_is_empty() {

      return m_total_l_toscan == m_total_l_scanned;
    }
    
    /// \brief check if no more S-type suffixes to be scanned in current bucket
    ///
    bool s_bkt_is_empty(){
    
    	return m_s_bkt_scanned[m_cur_s_ch] == m_s_bkt_toscan[m_cur_s_ch];	
    }
    
    /// \brief check if no more L-type suffixes to be scanned in current bucket
    ///
  	bool l_bkt_is_empty() {
  	
  		return m_l_bkt_scanned[m_cur_l_ch] == m_l_bkt_toscan[m_cur_l_ch]; 
  	}
  	
  	/// \brief find next non-empty S-bucket
  	///
  	/// \note guarantee there remains S-type suffixes to be scanned before calling the function
		void find_next_s_bkt() {
		
				while (s_bkt_is_empty()) --m_cur_s_ch;
				
				delete m_sa_s_reader;
				
				typename size_vector_type::const_reverse_iterator sa_crbeg = m_sa->crbegin() + m_s_bkt_spos[cur_s_bkt_ch];
					
				m_sa_s_reader = new typename size_vector_type::bufreader_type(sa_crbeg, sa_crbeg + m_s_bkt_toscan[cur_s_bkt_ch]);
					
				delete m_lcp_s_reader;
				
				typename size_vector_type::const_reverse_iterator lcp_crbeg = m_lcp->crbegin() + m_s_bkt_spos[cur_s_bkt_ch];
					
				m_lcp_s_reader = new typename size_vector_type::bufreader_type(lcp_crbeg, lcp_crbeg + m_s_bkt_toscan[cur_s_bkt_ch]);
	
				return;
		}

  	/// \brief find next non-empty L-bucket
  	///
  	/// \note guarantee there remains L-type suffixes to be scanned before calling the function
		void find_next_l_bkt() {
		
				while (l_bkt_is_empty()) --m_cur_l_ch;
				
				delete m_sa_l_reader;
				
				typename size_vector_type::const_reverse_iterator sa_crbeg = m_sa->crbegin() + m_l_bkt_spos[cur_l_bkt_ch];
					
				m_sa_l_reader = new typename size_vector_type::bufreader_type(sa_crbeg, sa_crbeg + m_l_bkt_toscan[cur_l_bkt_ch]);
					
				delete m_lcp_l_reader;
				
				typename size_vector_type::const_reverse_iterator lcp_crbeg = m_lcp->crbegin() + m_l_bkt_spos[cur_l_bkt_ch];
					
				m_lcp_l_reader = new typename size_vector_type::bufreader_type(lcp_crbeg, lcp_crbeg + m_l_bkt_toscan[cur_l_bkt_ch]);
	
				return;
		}
		
		/// \brief fetch the SA-value for currently scanned S-type suffix and the LCP-value of the suffix and its right neighbor in SA
		///
		/// \note actually both fetch_s_scanned and fetch_l_scanned retrieve elements from m_sa and m_lcp
		void fetch_s_scanned(size_type& _sa_value, size_type& _lcp_value) {
			
			_sa_value = *(*m_sa_reader), ++(*m_sa_reader);
			
			_lcp_value = *(*m_lcp_reader), ++(*m_lcp_reader);
			
			++m_s_bkt_scanned[m_cur_s_ch];
			
			++total_s_scanned;
			
			return;	
		}
		
		
		/// \brief fetch the SA-value for currently scanned L-type suffix and the LCP-value of the suffix and its right neighbor in SA
		///
		/// \note actually both fetch_s_scanned and fetch_l_scanned retrieve elements from m_sa and m_lcp
		void fetch_l_scanned(size_type& _sa_value, size_type& _lcp_value) {
			
			_sa_value = *(*m_sa_reader), ++(*m_sa_reader);
			
			_lcp_value = *(*m_lcp_reader), ++(*m_lcp_reader);
			
			++m_l_bkt_scanned[m_cur_l_ch];
			
			++total_l_scanned;
			
			return;	
		}
		
		/// \brief fetch the SA-value for currently induced S-type suffix and the LCP-value for the suffix and its right neighbor in SA 
		void fetch_s_induced(const alphabet_type _ch, size_type& _sa_value, size_type& _lcp_value) {
			
			_sa_value = *(*m_sa_l_bkt_reader[_ch]), ++(*m_sa_l_bkt_reader[_ch]);
			
			_lcp_value = *(*m_lcp_l_bkt_reader[_ch]), ++(*m_lcp_l_bkt_reader[_ch]);
			
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

    /// \brief de-ctor
    ~LScan() {
    	
				delete m_sa_reader; m_sa_reader = nullptr;
				
				delete m_lcp_reader; m_lcp_reader = nullptr;

        for (alphabet_type ch = 0; ch <= ch_max; ++ch) {
        	
        		delete m_sa_l_bkt_reader[ch]; m_sa_l_bkt_reader[ch] = nullptr;
        		
        		delete m_lcp_l_bkt_reader[ch]; m_lcp_l_bkt_reader[ch] = nullptr;
        }
    }
    
    /// \brief for each S-type suffix, check the SA-value in SA and the LCP-value between the suffix and its right neighbor in SA.
    ///
    /// case 1: scan: if currently scanned suffix $suf is S-type, then the LCP-value of $suf and its right neighbor in SA can be retrieved following the sub-cases below:
    ///										sub-case (a): if $suf is the rightmost S-type suffix in the bucket, then perform a literal comparison between $suf and the last scanned suffix to compute their LCP-value and compare the value with the one recorded in LCP for validation.
    ///										sub-case (b): otherwise, the LCP-value has been verified in the previous iterations and we directly read it from LCP.
    ///								update RMQ with the LCP-value.
		///					induce: if the preceding suffix of the last scanned suffix is S-type, then the LCP-value of the suffix and the currently scanned one can be calculated following the sub-cases below:
		///										sub-case (a): if the preceding suffix is the rightmost S-type suffix in the bucket, then we perform a literal comparison between the suffix and its right neighbor in SA to compute their LCP-value.
		///										sub-case (b): otherwise, the LCP-value is 1 + RMQ_VALUE.
		///								validate the correctness of the induced SA-value and the induced LCP-value.
		///	case 2: scan: if currently scanned suffix $suf is L-type, then the LCP-value of $suf and its right neighbor in SA can be retrieved following the sub-cases below:
		///										sub-case (a): if $suf is the rightmost suffix in SA, then it does not have a right neighbor in SA, skip it.
		///										sub-case (b): otherwise, if $suf is the rightmost L-type suffix in the bucket, then perform a literal comparison between $suf and the last scanned suffix to compute their LCP-value and compare the value with the one recorded in LCP for validation.
		///										sub-case (c): otherwise, the LCP-value has been verified when scanning rightward and we directly read it from LCP.
		///								update RMQ with the LCP-value.
		///					induce: if the preceding suffix of the last scanned suffix is S-type, then the LCP-value of the suffix and the currently scanned one can be calculated following the sub-cases below:
		///										sub-case (a): if the preceding suffix is the rightmost S-type suffix in the bucket, then we perform a literal comparison between the suffix and its right neighbor in SA to compute their LCP-value.
		///										sub-case (b): otherwise, the LCP-value is 1 + RMQ_VALUE.
    ///								validate the correctness of the induced SA-value and the induced LCP-value.
   void run(const triple2_less_sorter_type* _pre_s_items, const triples2_less_sorter_type* _pre_l_items){

      alphabet_type cur_bkt_ch, cur_l_ch, cur_s_ch, pre_ch_of_last_scanned;

			uint8 pre_t_of_last_scanned;
			
      size_type sv_cur_scanned, lv_cur_scanned, sv_last_scanned, lv_last_scanned;
      
      size_type sv_induced, lv_induced, sv_induced_fetch, lv_induced_fetch;
      
      RMQ<false> rmq_s;
      
      bool flag = true; // indicate current scanned S-type/L-type suffix is the rightmost suffix in the S-type/L-type bucket
      	
      bool flags[ch_max + 1] = {true}; // indicate currently induced S-type suffix is the rightmost suffix in the S-type bucket
      	
      { // process the rightmost suffix in SA (which is the largest suffix in T and must be L-type)
      	      	
      	cur_bkt_ch = get_l_bkt_ch();
      	
      	flag = false;
      	
      	//scan 
      	fetch_l_scanned(sv_cur_scanned, lv_cur_scanned);
      	
      	// no right neighbor in SA, thus no need to update RMQ and induce the preceding suffix of the last scanned suffix
      	
      	// 
      	sv_last_scanned = sv_cur_scanned;
      	
      	lv_last_scanned = lv_cur_scanned;
      	
      	pre_ch_of_last_scanned = *(*pre_l_items).second;
      	
      	pre_t_of_last_scanned = *(*pre_l_items).third;
      	

      }
      
      { // process the remaining suffixes
      	// we suppose there remain at least one suffix to be processed, thus l_is_empty() and s_is_empty() will not be all true
      	
      	if (l_bkt_is_empty()) { // currently scanned L-type bucket only contains the rightmost suffix in SA
      		
      		flag = true;
      		
      		if (!l_is_empty()) { // still remain L-type suffixes to scan
      			
      			find_next_l_bkt();
      			
      			cur_bkt_ch = get_l_bkt_ch();
      			
      			if (!s_is_empty() && get_s_bkt_ch() >= cur_bkt_ch) {
      				
      				cur_bkt_ch = get_s_bkt_ch();	
      			}
      		}
					else { // the assumption guarantees that s_is_empty() will not be true if l_is_empty() is true
							
						cur_bkt_ch = get_s_bkt_ch();	
					}
      	}
      	
      	
      	while (true) {
      		
      		while (!s_bkt_is_empty() && get_s_bkt_ch() == cur_bkt_ch) {
      			
      			// induce and validate the SA-value for the suffix induced from the last scanned suffix and the LCP-value of this suffix and its right neighbor in SA
      			if (pre_t_of_last_scanned == S_TYPE) { // the preceding suffix of the last scanned suffix is S-type, must be induced
      				
      				fetch_s_induced(pre_ch_of_last_scanned, sv_induced_fetch, lv_induced_fetch); // fetch the corresponding values from SA and LCP
      				
      				sv_induced = sv_last_scanned - 1;
      				
      				if (sv_induced != sv_induced_fetch) { // check SA-value
      					
									std::cerr << "wrong\n"; 
											
									exit(0);
      				}
      				
      				if (flags[pre_ch_last_scanned] == true) { // the induced S-type is the rightmost in the S-type bucket
      					
      					if (lv_induced_fetch != 0) { // check LCP-value, must be 0
      						
									std::cerr << "wrong\n"; 
											
									exit(0);      						
      					}
      					
      					flags[pre_ch_last_scanned] = false;
      				}
      				else {
      				
								lv_induced = rmq_l.get(pre_ch_of_last_scanned) + 1; // RMQ_VALUE + 1
								
								if (lv_induced != lv_induced_fetch) { // check LCP-value
								
									std::cerr << "wrong\n";
										
									exit(0);	
								}     				
	      			}     				     				

      				//reset RMQ
							rmq_l.reset(pre_ch_of_last_scanned);
      			}
      			
            // scan 
      			fetch_s_scanned(sv_cur_scanned, lv_cur_scanned);			

       			// update rmq with the LCP-value of currently scanned and last scanned suffixes
      			if (flag == true) { // rightmost S-type suffix in the bucket, the LCP-value has not been checked
      				
      				if (lv_last_scanned != bruteforce_compute_lcp(sv_last_scanned, sv_cur_scanned) { // check if correct
      					
      						std::cerr << "wrong\n";
      							
      						exit(0);	
      				}
      				
      				flag = false;
      			}  
      			else { // the LCP-value has been checked in the previous iterations
      			
      				// do nothing	
      			}  			
      		
      			rmq_s.update(lv_last_scanned);
      						
      			//
      			sv_last_scanned = sv_cur_scanned;
      			
      			lv_last_scanned = lv_cur_scanned;    			
      			
      			pre_ch_of_last_scanned = *(*pre_s_items).second;
      			
      			pre_t_of_last_scanned = *(*pre_l_items).third;
      		}
      		
      		flag = true;
      		
      		// scan L-type bucket
      		while (!l_bkt_is_empty() && get_l_bkt_ch() == cur_bkt_ch) {
      			
      			// induce and validate
      			if (pre_t_of_last_scanned == S_TYPE) { // the preceding character of the last scanned suffix is S-type
      				
      				fetch_s_induced(pre_ch_last_scanned)
      				
      				
      			}
      			
      			// scan
      			fetch_l_scanned(sv_cur_scanned, lv_cur_scanned);
      			
      			//
      			if (flag == true) { // rightmost L-type suffix in the bucket, the LCP-value of the suffix and its right neighbor in SA has not been verified
      				
      				if (lv_last_scanned != bruteforce_compute_lcp(sv_cur_scanned, sv_last_scanned)){
      					
      					std::cerr << "wrong\n";
      						
      					exit(0);
      				}
      				
      				flag = false; 				
      			}
      			else {
      				
      				// do nothing
      			}
      			
      			// update rmq
      			rmq_s.update(lv_last_scanned);      			
      			
      			//
      			sv_last_scanned = sv_cur_scanned;
      			
      			lv_last_scanned = lv_cur_scanned;    	
      			
      			pre_ch_of_last_scanned = *(*pre_s_items).second;
      			
      			pre_t_of_last_scanned = *(*pre_l_items).third;		
      		}
      		
      		// determine the next bucket
      		if (!s_is_empty()) {
      			
      			find_next_s_bkt();
      			
      			cur_bkt_ch = get_s_bkt_ch();
      			
      			if (!l_is_emtpy()) {
      				
      				find_next_l_bkt();	
      				
      				if (cur_bkt_ch < get_l_bkt_ch()) {
      					
      					cur_bkt_ch = get_l_bkt_ch();	
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
      }
    }
  };
  
  
  
  Validate4(alphabet_vector_type& _t, size_vector_type& _sa. size_vector_type& _lcp){}
  
  
  
};





#endif // VALIDATE4_H