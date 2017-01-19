#include "common/common.h"

#include "common/tuples.h"

#include "common/widget.h"

#include "common/basicio.h" 

#ifndef _TEST_H
#define _TEST_H

template<typename alphabet_type, typename alphabet_extension_type, typename size_type>
class Test{

private:
	typedef typename ExVector<alphabet_type>::vector alphabet_vector_type;

	typedef typename ExVector<size_type>::vector size_vector_type;

	//
	typedef triple<size_type, uint8, uint8> triple2_type;

	typedef tuple_less_comparator_1st<triple2_type> triple2_less_comparator_1st_type;

	typedef typename ExTupleSorter<triple2_type, triple2_less_comparator_1st_type>::sorter triple2_less_sorter_1st_type;

	typedef tuple_great_comparator_1st<triple2_type> triple2_great_comparator_1st_type;

	typedef typename ExTupleSorter<triple2_type, triple2_great_comparator_1st_type>::sorter triple2_great_sorter_1st_type;

	//
	typedef pair<size_type, uint8> pair3_type;

	typedef tuple_less_comparator_1st<pair3_type> pair3_less_comparator_1st_type;
	
	typedef typename ExTupleSorter<pair3_type, pair3_less_comparator_1st_type>::sorter pair3_less_sorter_1st_type;
 
	//
	uint64 m_len;

	alphabet_type* m_s;

	size_type* m_sa;

	size_type* m_lcp;

	uint8* m_t;

	uint64 m_lms_num;

	size_type* m_sa_lms;

	size_type* m_lcp_lms;
 
public:

	/// \brief ctor
	///
	Test(alphabet_vector_type* _ex_s, 
		size_vector_type* _ex_sa, 
		size_vector_type* _ex_lcp) {
	
		m_len = _ex_s->size();

		// load t
		m_s = new alphabet_type[m_len];

		typename alphabet_vector_type::bufreader_type* s_reader = new typename alphabet_vector_type::bufreader_type(*_ex_s);

		for (uint64 i = 0; !s_reader->empty(); ++i, ++(*s_reader)) {

			m_s[i] = *(*s_reader);
		}

		delete s_reader; s_reader = nullptr;

		// load sa
		m_sa = new size_type[m_len];

		typename size_vector_type::bufreader_type* sa_reader = new typename size_vector_type::bufreader_type(*_ex_sa);

		for (uint64 i = 0; !sa_reader->empty(); ++i, ++(*sa_reader)) {

			m_sa[i] = *(*sa_reader);
		}				

		delete sa_reader; sa_reader = nullptr;

		// load lcp
		m_lcp = new size_type[m_len];

		typename size_vector_type::bufreader_type* lcp_reader = new typename size_vector_type::bufreader_type(*_ex_lcp);

		for (uint64 i = 0; !lcp_reader->empty(); ++i, ++(*lcp_reader)) {

			m_lcp[i] = *(*lcp_reader);
		}

		delete lcp_reader; lcp_reader = nullptr;

		// compute t
		m_t = new uint8[m_len];

		m_t[m_len - 1] = L_TYPE; // rightmost is L-type

		for (uint64 i = m_len - 2; i >= 0; --i) {

			m_t[i] = ((m_s[i] < m_s[i + 1]) || (m_s[i] == m_s[i + 1] && m_t[i + 1] == S_TYPE)) ? S_TYPE : L_TYPE;

			if (i == 0) break;
		} 
	}

	/// \brief validate SA_LMS and LCP_LMS
	/// 
	bool validate_lms(size_vector_type* _sa_ex_lms, size_vector_type* _lcp_ex_lms) {

		// count number of LMS 
		m_lms_num = 0;

		for (uint64 i = 1; i < m_len; ++i) {

			if (m_t[i] == S_TYPE && m_t[i - 1] == L_TYPE) {

				++m_lms_num;
			}
		}

		if (m_lms_num != _sa_ex_lms->size()) {

			std::cerr << "m_lms_num is not right\n";

			return false;
		} 

		// compute SA_LMS and LCP_LMS in RAM
		m_sa_lms = new size_type[m_lms_num];

		m_lcp_lms = new size_type[m_lms_num];

		size_type min_lcp = 0;
	
		for (uint64 i = 0, j = 0; i < m_len; ++i) {

			min_lcp = std::min(min_lcp, m_lcp[i]);

			if (0 != m_sa[i] && m_t[m_sa[i]] == S_TYPE && m_t[m_sa[i] - 1] == L_TYPE) {

				m_sa_lms[j] = m_sa[i];

				m_lcp_lms[j] = min_lcp;				

				++j;
			
				min_lcp = std::numeric_limits<size_type>::max();
			}	
		}


		// validate SA_LMS & LCP_LMS
		typename size_vector_type::bufreader_type* sa_lms_reader = new typename size_vector_type::bufreader_type(*_sa_ex_lms);

		typename size_vector_type::bufreader_type* lcp_lms_reader = new typename size_vector_type::bufreader_type(*_lcp_ex_lms);

		for (uint64 i = 0; !sa_lms_reader->empty(); ++i, ++(*sa_lms_reader), ++(*lcp_lms_reader)) {

			if (*(*sa_lms_reader) != m_sa_lms[i] || *(*lcp_lms_reader) != m_lcp_lms[i]) {
	
				std::cerr << "sa_lms or lcp_lms is wrong";

				return false;
			}
		}

		return true;
	}


	/// \brief vaildate preceding items
	///
	bool validate_pre_item(triple2_less_sorter_1st_type * _pre_item_of_l_sorter, triple2_great_sorter_1st_type * _pre_item_of_s_sorter, pair3_less_sorter_1st_type * _pre_item_of_lms_sorter) {

		uint8 pre_t;

		alphabet_type pre_ch;

		// check pre_items for L-type suffixes
		for (uint64 i = 0; i < m_len; ++i) {

			if (m_t[m_sa[i]] == L_TYPE) {

				if (0 != m_sa[i]) {

					pre_ch = m_s[m_sa[i] - 1];

					pre_t = m_t[m_sa[i] - 1];
				}				
				else {

					pre_ch = 0;

					pre_t = SENTINEL_TYPE;
				}

				const triple2_type& tuple = *(*_pre_item_of_l_sorter);

				if (pre_ch != tuple.second || pre_t != tuple.third) {

					std::cerr << "pre items of L-type suffixes are wrong\n";

					return false;
				}

				++(*_pre_item_of_l_sorter);
			}
		}

		assert(_pre_item_of_l_sorter->size() == 0);

		// check pre_items for S-type suffixes
		for (uint64 i = m_len - 1; i >= 0; --i) {

			if (m_t[m_sa[i]] == S_TYPE) {

				if (0 != m_sa[i]) {

					pre_ch = m_s[m_sa[i] - 1];

					pre_t = m_t[m_sa[i] - 1];
				}
				else {

					pre_ch = 0;

					pre_t = SENTINEL_TYPE;
				}

				const triple2_type& tuple = *(*_pre_item_of_s_sorter);

				if (pre_ch != tuple.second || pre_t != tuple.third) {

					std::cerr << "pre items of S-type suffixes are wrong\n";

					return false;
				}

				++(*_pre_item_of_s_sorter);
			}

			if (i == 0) break;
		}

		assert(_pre_item_of_s_sorter->size() == 0);

		// check pre_items for LMS suffixes	
		for (uint64 i = 0; i < m_len; ++i) {

			if (0 != m_sa[i] && m_t[m_sa[i]] == S_TYPE && m_t[m_sa[i] - 1] == L_TYPE) {

				pre_ch = m_s[m_sa[i] - 1];
			
				const pair3_type& tuple = *(*_pre_item_of_lms_sorter);

				if (pre_ch != tuple.second) {

					std::cerr << "pre items of LMS suffixes are wrong\n";

					return false;
				}

				++(*_pre_item_of_lms_sorter);
			}

		}

		assert(_pre_item_of_lms_sorter->size() == 0);

		return true;		
	}


	~Test() {


		delete m_s; m_s = nullptr;

		delete m_sa; m_sa = nullptr;

		delete m_lcp; m_lcp = nullptr;

		delete m_t; m_t = nullptr;

		delete m_sa_lms; m_sa_lms = nullptr;

		delete m_lcp_lms; m_lcp_lms = nullptr;
	}
	
};


#endif // _TEST_H
