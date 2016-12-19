///////////////////////////////////////////////////////////////////////////////////////////////////////
/// Copyright (c) 2016, Sun Yat-sen University,
/// All rights reserved
/// \file validate.h
/// \brief first method for validating suffix and LCP arrays
///
/// Given suffix and LCP arrays, validate their correctness using Karp-Rabin Finger-printing function.
///
/// \note The alphabet is assumed to be constant
/// \author Yi Wu
/// \date 2016.12
////////////////////////////////////////////////////////////////////////////////////////////////////////


#ifndef __VALIDATE_H
#define __VALIDATE_H

#include "common/common.h"

#include "common/basicio.h"

#include "common/widget.h"

#include "stxxl/timer"

#define TEST_1

#define STATISTICS_COLLECTION

/// \brief validate suffix and LCP arrays using Karp-Rarbin finger-printing function
template<typename alphabet_type, typename size_type, typename offset_type>
class Validate{
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

	typedef typename ExVector<alphabet_type>::vector alphabet_vector_type;

	typedef typename ExVector<size_type>::vector size_vector_type;


private:

	std::string m_t_fn; ///< file name of t array

	std::string m_sa_fn; ///< file name of sa array

	std::string m_lcp_fn; ///< file name of lcp array

	uint64 m_len; ///< length of input t/sa/lcp

	RInterval* m_rinterval; 

	alphabet_type m_pre_ch;

	fpa_type m_pre_fp_interval;

public:
	

	/// \brief constructor
	Validate(const std::string& _t_fn, const std::string& _sa_fn, const std::string& _lcp_fn) {

		if (typeid(fpa_type).name() != typeid(typename size_type::low_type).name()) {

			std::cerr << "fpa_type is not same as size_type::low_type.\n";

			exit(EXIT_FAILURE);
		}

		m_t_fn = _t_fn;

		m_sa_fn = _sa_fn;
	
		m_lcp_fn = _lcp_fn;

		m_len = BasicIO::file_size(m_t_fn) / sizeof(alphabet_type);	

		m_rinterval = new RInterval(m_len);
	
	}


	/// \brief destrcutor
	~Validate() {

		delete[] m_rinterval;
	}

	/// \brief main program portal
	///
	/// Parition sa and lcp into multiple block
	/// Tackle each piece individually.
	bool run() {
	
#ifdef STATISTICS_COLLECTION

		stxxl::stats *Stats = stxxl::stats::get_instance();

		stxxl::stats_data Stats_begin(*Stats);

		stxxl::block_manager* bm = stxxl::block_manager::get_instance();

		stxxl::timer Timer1;

		Timer1.start();
#endif	

		typedef std::pair<size_type, offset_type> pair_type;

		uint64 block_capacity = (MAIN_MEM_AVAIL) / (sizeof(size_type) + sizeof(offset_type)) / 3;

		pair_type* pair1_block = new pair_type[block_capacity]; // (sa[i], i)

		pair_type* pair2_block = new pair_type[block_capacity]; // (sa[i] + lcp[i] - 1, i)

		pair_type* pair3_block = new pair_type[block_capacity]; // (sa[i] + lcp[i + 1] - 1, i)

#ifdef STATISTICS_COLLECTION

		Timer1.stop();

		std::cerr << "Timer 1: " << Timer1.seconds() << " seconds " << Timer1.mseconds() << " mseconds.\n";
#endif

		//
		uint64 items_toread = m_len;
		
		uint64 items_read = 0;

		uint64 block_id = 0;

		while (items_toread > 0) {

#ifdef STATISTICS_COLLECTION

			stxxl::timer Timer2;

			Timer2.start();

#endif

			uint64 item_num = (items_toread >= block_capacity) ? block_capacity : items_toread;

			stxxl::syscall_file* sa_file = new stxxl::syscall_file(m_sa_fn, stxxl::syscall_file::RDWR | stxxl::syscall_file::DIRECT);
			
			size_vector_type* sa = new size_vector_type(sa_file);
			
			typename size_vector_type::const_iterator sa_beg = sa->begin() + items_read;

			typename size_vector_type::const_iterator sa_end = sa_beg + item_num;

			typename size_vector_type::bufreader_type* sa_reader = new typename size_vector_type::bufreader_type(sa_beg, sa_end, 2ul);

			stxxl::syscall_file* lcp_file = new stxxl::syscall_file(m_lcp_fn, stxxl::syscall_file::RDWR | stxxl::syscall_file::DIRECT);
		
			size_vector_type* lcp = new size_vector_type(lcp_file);

			typename size_vector_type::const_iterator lcp_beg = lcp->begin() + items_read;

			typename size_vector_type::const_iterator lcp_end = lcp_beg + item_num + ((items_toread == item_num) ? 0 : 1);

			typename size_vector_type::bufreader_type* lcp_reader = new typename size_vector_type::bufreader_type(lcp_beg, lcp_end, 2ul);

			for (uint32 j = 0; j < item_num - ((items_toread == item_num) ? 1 : 0); ++j) {

				// pair1_block stores (sa[j], j)
				pair1_block[j].first = *(*sa_reader);

				pair1_block[j].second = j;

				// pair2_block stores (sa[j] + lcp[j], j)
				pair2_block[j].first = *(*sa_reader) + *(*lcp_reader);

				pair2_block[j].second = j;

				// pair3_block stores (sa[j] + lcp[j + 1], j)
				++(*lcp_reader);

				pair3_block[j].first = *(*sa_reader) + *(*lcp_reader); 

				pair3_block[j].second = j;

				// 
				++(*sa_reader);
			}
		
			// special case: rightmost element in rightmost block, lcp[items_toread] does not exist
			if (items_toread == item_num) {

				// pair1_block stores (sa[j], j)
				pair1_block[item_num - 1].first = *(*sa_reader);

				pair1_block[item_num - 1].second = item_num - 1;

				// pair2_block stores (sa[j] + lcp[j], j)
				pair2_block[item_num - 1].first = *(*sa_reader) + *(*lcp_reader);

				pair2_block[item_num - 1].second = item_num - 1;

				//
				++(*sa_reader);
			}
			
			++(*lcp_reader);

			assert(sa_reader->empty() == true);

			assert(lcp_reader->empty() == true);
		
			delete sa_reader; delete sa; delete sa_file;

			delete lcp_reader; delete lcp; delete lcp_file;

#ifdef STATISTICS_COLLECTION

			Timer2.stop();

			std::cerr << "Timer 2: " << Timer2.seconds() << " seconds " << Timer2.mseconds() << " mseconds.\n";
		
			stxxl::timer Timer3;

			Timer3.start();

#endif

			// check block
			bool is_right = check_block(pair1_block, pair2_block, pair3_block, item_num, block_id, block_capacity, items_toread == item_num);

			if (!is_right) return false;

			// adjust
			items_toread -= item_num;

			items_read += item_num;

			block_id++;


#ifdef STATISTICS_COLLECTION

			Timer3.stop();

			std::cerr << "Timer 3: " << Timer3.seconds() << " seconds " << Timer3.mseconds() << " mseconds.\n";
		
#endif
		}

		std::cerr << (stxxl::stats_data(*Stats) - Stats_begin);

		std::cerr << "MAIN_MEM_AVAIL: " << MAIN_MEM_AVAIL << std::endl;

		std::cerr << "total IO volume: " << Stats->get_written_volume() + Stats->get_read_volume() << std::endl;

		std::cerr << "IO volume per ch: " << (Stats->get_written_volume() + Stats->get_read_volume()) / m_len << std::endl;

		std::cerr << "peak disk use per ch: " << bm->get_maximum_allocation() / m_len << std::endl;

		std::cerr << "block num: " << block_id + 1 << std::endl;

		return true;
	}

	/// \brief check an sa/lcp block
	bool check_block(std::pair<size_type, uint32>* _pair1_block, std::pair<size_type, uint32>* _pair2_block, std::pair<size_type, uint32>* _pair3_block, const uint64& _item_num, const uint64& _block_id, const uint64& _block_capacity, const bool _is_rightmost) {

#ifdef STATISTICS_COLLECTION

		stxxl::timer Timer4;

		Timer4.start();

#endif

		// sort pairs by first component
		std::sort(_pair1_block, _pair1_block + _item_num, PairLess1st<std::pair<size_type, uint32>>());

		std::sort(_pair2_block, _pair2_block + _item_num, PairLess1st<std::pair<size_type, uint32>>());

		std::sort(_pair3_block, _pair3_block + _item_num - (_is_rightmost ? 1 : 0), PairLess1st<std::pair<size_type, uint32>>());

#ifdef STATISTICS_COLLECTION

		Timer4.stop();

		std::cerr << "Timer 4: " << Timer4.seconds() << " seconds " << Timer4.mseconds() << " mseconds.\n";

		stxxl::timer Timer5;

		Timer5.start();
		
#endif

		// read input string to iteratilvey compute fingerprints
		stxxl::syscall_file t_file(m_t_fn, stxxl::syscall_file::RDWR | stxxl::syscall_file::DIRECT);

		alphabet_vector_type t(&t_file);

		typename alphabet_vector_type::bufreader_type t_reader(t);
		
		// compute fp[0, i - 1] iteratively
		fpa_type fp = 0; //fp[0, -1] = 0
		
		uint64 j1 = 0, j2 = 0, j3 = 0;

		for (uint64 i = 0; i < m_len; ++i) {

			// store fp[0,i - 1] and set ch = 0
			while (j1 < _item_num && i == _pair1_block[j1].first) {

				_pair1_block[j1].first.set(fp, 0); // reuse high/low part to store ch/fp
			
				++j1;
			}

			// store fp[0, i] and ch = *t_reader
			while (j2 < _item_num && i == _pair2_block[j2].first) {

				_pair2_block[j2].first.set(fp, *t_reader);

				++j2;
			}				

			// store fp[0, i] and ch = *t_reader
			while (j3 < _item_num - (_is_rightmost ? 1 : 0) && i == _pair3_block[j3].first) {

				_pair3_block[j3].first.set(fp, *t_reader);

				++j3;
			}

			// compute fp[0, i]
			fp = static_cast<fpa_type>((static_cast<fpb_type>(fp) * R + (*t_reader + 1)) % P); // t_reader + 1 to gurantee non-zero

			++t_reader;
		}

		// special case: m_len
		while (j1 < _item_num) {

			_pair1_block[j1].first.set(fp, 0); // reuse high/low part to store ch/fp
			
			++j1;
		}

		while (j2 < _item_num) {

			_pair2_block[j2].first.set(fp, std::numeric_limits<typename size_type::high_type>::max());

			++j2;
		}				

		while (j3 < _item_num - (_is_rightmost ? 1 : 0)) {

			_pair3_block[j3].first.set(fp, std::numeric_limits<typename size_type::high_type>::max());

			++j3;
		}

		//
		assert(t_reader.empty() == true);

#ifdef STATISTICS_COLLECTION

		Timer5.stop();

		std::cerr << "Timer 5: " << Timer5.seconds() << " seconds " << Timer5.mseconds() << " mseconds.\n";

		stxxl::timer Timer6;

		Timer6.start();
		
#endif

		// sort pairs back to original order 
		std::sort(_pair1_block, _pair1_block + _item_num, PairLess2nd<std::pair<size_type, uint32>>());

		std::sort(_pair2_block, _pair2_block + _item_num, PairLess2nd<std::pair<size_type, uint32>>());

		std::sort(_pair3_block, _pair3_block + _item_num - (_is_rightmost ? 1 : 0), PairLess2nd<std::pair<size_type, uint32>>());


#ifdef STATISTICS_COLLECTION

		Timer6.stop();

		std::cerr << "Timer 6: " << Timer6.seconds() << " seconds " << Timer6.mseconds() << " mseconds.\n";

		stxxl::timer Timer7;

		Timer7.start();		
#endif

		// check result
		alphabet_type pre_ch;

		fpa_type fp_interval, pre_fp_interval;

		//
		stxxl::syscall_file lcp_file(m_lcp_fn, stxxl::syscall_file::RDWR | stxxl::syscall_file::DIRECT);
			
		size_vector_type lcp(&lcp_file);

		typename size_vector_type::const_iterator lcp_beg = lcp.begin() +  _block_id * _block_capacity;

		typename size_vector_type::const_iterator lcp_end = lcp_beg + _item_num + (_is_rightmost ? 0 : 1);

		typename size_vector_type::bufreader_type lcp_reader(lcp_beg, lcp_end, 2ul);

		// check first pair 
		if (0 == _block_id) { // leftmost block, skip checking sa[0]

			pre_ch = _pair3_block[0].first.get_high();

			++lcp_reader;

			pre_fp_interval = static_cast<fpa_type>(((static_cast<fpb_type>(_pair3_block[0].first.get_low()) - static_cast<fpb_type>(_pair1_block[0].first.get_low()) * m_rinterval->compute(*lcp_reader) % P) + P) % P); //fp[sa[0] + lcp[1] - 1] - fp[ - 1]			
		}
		else {

			pre_ch = m_pre_ch;

			pre_fp_interval = m_pre_fp_interval;
		}

		// if leftmost block, skip checking leftmost lcp
		for (uint64 i = ((_block_id == 0) ? 1 : 0); i < _item_num - (_is_rightmost ? 1 : 0); ++i) {

			// compare ch[sa[i] + lcp[i]] and ch[sa[i - 1] + lcp[i]]
			if (pre_ch == _pair2_block[i].first.get_high()) {

				std::cerr << "here1 " << " item_num: " << _item_num << " block: " << _block_id << " pos: " << i << std::endl;

				return false;
			}

			// compare fp(sa[i] + lcp[i]] and fp[sa[i - 1] + lcp[i]]
			fp_interval = static_cast<fpa_type>(((static_cast<fpb_type>(_pair2_block[i].first.get_low()) - static_cast<fpb_type>(_pair1_block[i].first.get_low())* m_rinterval->compute(*lcp_reader) % P) + P) % P);

			if (fp_interval != pre_fp_interval) {
				
				std::cerr << "here2 " << " block: " << _block_id << " pos: " << i << std::endl;

				return false;
			}

			// compute pre_ch = ch[sa[i] + lcp[i + 1]] and pre_fpinterval = fp[sa[i] + lcp[i + 1]]
			pre_ch = _pair3_block[i].first.get_high();

			++lcp_reader;

			pre_fp_interval = static_cast<fpa_type>(((static_cast<fpb_type>(_pair3_block[i].first.get_low()) - static_cast<fpb_type>(_pair1_block[i].first.get_low()) * m_rinterval->compute(*lcp_reader) % P) + P) % P);
		}

		// rightmost block, lcp[item_num] does not exist
		if (_is_rightmost) {

			if (pre_ch == _pair2_block[_item_num - 1].first.get_high()) return false;

			fp_interval = static_cast<fpa_type>(((static_cast<fpb_type>(_pair2_block[_item_num - 1].first.get_low()) - static_cast<fpb_type>(_pair1_block[_item_num - 1].first.get_low())* m_rinterval->compute(*lcp_reader) % P) + P) % P);

			if (fp_interval != pre_fp_interval) return false;

			++lcp_reader;
		}
		else {

			m_pre_ch = pre_ch;

			m_pre_fp_interval = pre_fp_interval;
		}
		
		//assert(lcp_reader.empty() == true);

#ifdef STATISTICS_COLLECTION

		Timer7.stop();

		std::cerr << "Timer 7: " << Timer7.seconds() << " seconds " << Timer7.mseconds() << " mseconds.\n";

#endif

		return true;
	}	

};



#endif // __VALIDATE_H
