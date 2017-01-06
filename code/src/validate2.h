////////////////////////////////////////////////////////////
/// Copyright (c) 2016, Sun Yat-sen University,
/// All rights reserved
/// \file validate2.h
/// \brief validate sa and lcp using internal memory sorts and Karp-rabin finger-print function.
///  
/// \note The alphaber size of the input string is assumed to be O(1) or O(n).
/// \note This method requires multiple sequential scans for the input string. During each scan, all the fingerprints are computed by an iterative way. 
/// This has become the bottleneck to the system's running time.
///
/// \author Yi Wu
/// \date 2016.12
///////////////////////////////////////////////////////////


#ifndef VALIDATE2_H
#define VALIDATE2_H

#include "common/common.h"

#include "common/tuples.h"

#include "common/widget.h"

#include "common/basicio.h"

/// \brief a suffix and LCP array validater
template<typename alphabet_type, typename size_type>
class Validate2{

private:
	
	typedef typename ExVector<size_type>::vector size_vector_type;

	typedef typename ExVector<alphabet_type>::vector alphabet_vector_type;

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

	std::string m_t_fn; ///< file name of input string

	std::string m_sa_fn; ///< file name of suffix array

	std::string m_lcp_fn; ///< file name of lcp array

	uint64 m_len; ///< length of input string

	RInterval* m_rinterval; ///< pointer to an instance of RInterval

public:

	/// \brief constructor
	Validate2(const std::string& _t_fn, const std::string& _sa_fn, const std::string& _lcp_fn) {

		m_t_fn = _t_fn;

		m_sa_fn = _sa_fn;

		m_lcp_fn = _lcp_fn;

		m_len = BasicIO::file_size(m_t_fn) / sizeof(alphabet_type);

		std::cerr << "m_len: " << m_len / 1024 / 1024 << " MB" << std::endl;

		m_rinterval = new RInterval(m_len);

		run();
	}

	/// \brief run the core part
	bool run() {

		stxxl::stats* Stats = stxxl::stats::get_instance();

		stxxl::stats_data Stats_begin(*Stats);

		stxxl::block_manager* bm = stxxl::block_manager::get_instance();

		// step 1: 
		// read sa & lcp, pack (sa[i], lcp[i], i)  and sort them by 1st component		
		typedef triple<size_type, size_type, size_type> triple_type;

		typedef tuple_less_comparator_1st<triple_type> triple_comparator_type;

		typedef typename ExTupleSorter<triple_type, triple_comparator_type>::sorter triple_sorter_type;

		triple_sorter_type* triple_sorter = new triple_sorter_type(triple_comparator_type(), MAIN_MEM_AVAIL / 2);		

		{			
			stxxl::syscall_file* sa_file = new stxxl::syscall_file(m_sa_fn, stxxl::syscall_file::RDWR | stxxl::syscall_file::DIRECT);

			size_vector_type *sa = new size_vector_type(sa_file);

			typename size_vector_type::bufreader_type* sa_reader = new typename size_vector_type::bufreader_type(*sa);

			stxxl::syscall_file* lcp_file = new stxxl::syscall_file(m_lcp_fn, stxxl::syscall_file::RDWR | stxxl::syscall_file::DIRECT);

			size_vector_type *lcp = new size_vector_type(lcp_file);

			typename size_vector_type::bufreader_type* lcp_reader = new typename size_vector_type::bufreader_type(*lcp);


			// pack (sa[i], lcp[i], i) into a triple and sort the triples by sa[i]
			// note that to avoid stxxl::error, let idx start from 1 rather than 0. Thus, we actually pack (sa[i], lcp[i], i + 1)
			for (uint64 idx = 1; !sa_reader->empty(); ++(*sa_reader), ++(*lcp_reader), ++idx) {
					
				triple_sorter->push(triple_type(*(*sa_reader), *(*lcp_reader), idx));
			}

			delete sa_reader; sa_reader = nullptr;

			delete sa; sa = nullptr;

			delete sa_file; sa_file = nullptr;

			delete lcp_reader; lcp_reader = nullptr;

			delete lcp; lcp = nullptr;

			delete lcp_file; lcp_file = nullptr;

			triple_sorter->sort();
		}


		// step 2: 
		// given sorted (sa[i], lcp[i], i)
		// read t to compute fingerprints, pack (sa[i] + lcp[i], i, fp1 * r_interval1) and sort them by (1st, 2nd) components
		typedef triple<size_type, size_type, fpb_type> triple_type2;

		typedef tuple_less_comparator_2nd<triple_type2> triple_comparator_type2;

		typedef typename ExTupleSorter<triple_type2, triple_comparator_type2>::sorter triple_sorter_type2;

		triple_sorter_type2* triple_sorter2 = new triple_sorter_type2(triple_comparator_type2(), MAIN_MEM_AVAIL / 2);		

		{
			stxxl::syscall_file* t_file = new stxxl::syscall_file(m_t_fn, stxxl::syscall_file::RDWR | stxxl::syscall_file::DIRECT);

			alphabet_vector_type* t = new alphabet_vector_type(t_file);

			typename alphabet_vector_type::bufreader_type* t_reader = new typename alphabet_vector_type::bufreader_type(*t);

			fpa_type fp = 0;

			for (uint64 pos = 0; !t_reader->empty(); ++(*t_reader), ++(*triple_sorter), ++pos) {

				const triple_type& item = *(*triple_sorter);

				triple_sorter2->push(triple_type2(item.first + item.second, item.third, static_cast<fpb_type>(fp) * m_rinterval->compute(item.second)));

				fp = static_cast<fpa_type>((static_cast<fpb_type>(fp) * R + (*(*t_reader) + 1)) % P);
			}	

			delete triple_sorter; triple_sorter = nullptr;

			delete t_reader; t_reader = nullptr;

			delete t; t = nullptr;

			delete t_file; t_file = nullptr;

			triple_sorter2->sort();
		}

		// step 3:
		// given (sa[i] + lcp[i], i, fp1 * r_interval1) by (1st, 2nd) components
		// read t to iteratively compute fingerprints, pack (isa[i], fp1 * rinterval1, ch1, fp_interval1) and sort them by 1st component
		typedef quadruple<size_type, fpb_type, alphabet_type, fpa_type> quadruple_type;

		typedef tuple_less_comparator_1st<quadruple_type> quadruple_comparator_type;

		typedef typename ExTupleSorter<quadruple_type, quadruple_comparator_type>::sorter quadruple_sorter_type;

		quadruple_sorter_type* quadruple_sorter = new quadruple_sorter_type(quadruple_comparator_type(), MAIN_MEM_AVAIL / 2);
	
		{
			stxxl::syscall_file* t_file = new stxxl::syscall_file(m_t_fn, stxxl::syscall_file::RDWR | stxxl::syscall_file::DIRECT);

			alphabet_vector_type* t = new alphabet_vector_type(t_file);

			typename alphabet_vector_type::bufreader_type* t_reader = new typename alphabet_vector_type::bufreader_type(*t);

			fpa_type fp = 0;

			fpa_type fp_interval;

			for (uint64 pos = 0; !t_reader->empty(); ++(*t_reader), ++pos) {

				while (!triple_sorter2->empty() && pos == (*triple_sorter2)->first) {

					const triple_type2& item = *(*triple_sorter2);

					fp_interval = static_cast<fpa_type>((static_cast<fpb_type>(fp) - item.third % P + P) % P);

					quadruple_sorter->push(quadruple_type(item.second, item.third, *(*t_reader), fp_interval));
	
					++(*triple_sorter2);
				}
	
				fp = static_cast<fpa_type>((static_cast<fpb_type>(fp) * R + *(*t_reader) + 1) % P);
			}
	
			// special case: pos == m_len
			while (!triple_sorter2->empty()) {

				const triple_type2& item = *(*triple_sorter2);

				fp_interval = static_cast<fpa_type>((static_cast<fpb_type>(fp) - item.third % P + P) % P);

				quadruple_sorter->push(quadruple_type(item.second, item.third, std::numeric_limits<alphabet_type>::max(), fp_interval));
	
				++(*triple_sorter2);
			}

			delete triple_sorter2; triple_sorter2 = nullptr;
	
			delete t_reader; t_reader = nullptr;
	
			delete t; t = nullptr;
	
			delete t_file; t_file = nullptr;
	
			quadruple_sorter->sort();
		}
#ifdef test2

		// step 5: 
		// given (i, fp1 * r_interval1, ch1, fp1_interval)
		// read sa and lcp to pack (sa[i] + lcp[i + 1], fp1 * rinterval2, ch1, fp_interval1) and sort tuples by 1st component	
		typedef tuple_less_comparator_1st<quadruple_type> quadruple_comparator_type;

		typedef typename ExTupleSorter<quadruple_type, quadruple_comparator_type>::sorter quadruple_sorter_type;

		quadruple_sorter_type* quadruple_sorter = new quadruple_sorter_type(quadruple_comparator_type(), MAIN_MEM_AVAIL / 2);

		ExTupleAscComparator<triple_type4> triple_comparator_type4;

		ExTupleAscSorter<triple_type4, triple_comparator_type4> triple_sorter4;

		{
			
			stxxl::syscall_file* sa_file = new stxxl::syscall_file(m_sa_file, stxxl::syscall_file::RDWR | stxxl::syscall_file::DIRECT);

			size_vector_type *sa = new size_vector_type(sa_file);

			typename size_vector_type::bufreader_type* sa_reader = new typename size_vector_type::bufreader_type(*sa);

			stxxl::syscall_file* lcp_file = new stxxl::syscall_file(m_lcp_file, stxxl::syscall_file::RDWR | stxxl::syscall_file::DIRECT);

			size_vector_type *lcp = new size_vector_type(lcp_file);

			typename size_vector_type::bufreader_type* lcp_reader = new typename size_vector_type::bufreader_type(*lcp);

			fpb_type fp_interval;

			// sa[i] + lcp[i + 1]
			size_type cur_lcp = *(*lcp_reader);

			++(*lcp_reader);

			for (size_type idx = 1; !lcp_reader->empty(); ++(*sa_reader), ++(*lcp_reader), ++idx) {

				const quadruple_type& item = *(*quadruple_sorter);

				fp_interval = std::get<0>(item) / m_rinterval->compute(cur_lcp) * m_rinterval->compute(*(*lcp_reader));

				triple_sorter4->push(quadruple_type(*(*sa_reader) + *(*lcp_reader), fp_interval, std::get<1>(item), std::get<2>(item));
			}
			

			delete quadruple_sorter; quadruple_sorter = nullptr;

			delete sa_reader; sa_reader = nullptr;

			delete sa; sa = nullptr;

			delete sa_file; sa_file = nullptr;

			delete lcp_reader; lcp_reader = nullptr;

			delete lcp; lcp = nullptr;

			delete lcp_file; lcp_file = nullptr;

			quadruple_sorter2->sort();			
		}

		// step 6: read t to iteratively compute fingerprints, check the correctness	
		{
			fpa_type fp = 0;

			fpa_type fp_interval;

			for (size_type pos = 0; !t_reader->emtpy(); ++(*t_reader), ++pos) {

				while (!quadruple_sorter2->empty() && (*quadruple_sorter2)->first == pos) {

					const quadruple_type2& item = *(*quadruple_sorter2);

					fp_interval = static_cast<fpa_type>((static_cast<fpb_type>(fp) - std::get<1>(item) + P) % P + P);

					if (fp_interval != std::get<2>(item) || *(*t_reader) == std::get<3>(item)) {

						return false;
					}
				}

				fp = static_cast<fpa_type>((static_cast<fpb_type>(fp) * R + *(*t_reader) + 1) % P);
			}
	
			// pos == m_len
			while (!quadruple_sorter2->empty()) {

				const quadruple_type2& item = *(*quadruple_sorter2);

				fp_interval = static_cast<fpa_type>((static_cast<fpb_type>(fp) - std::get<1>(item) + P) % P + P);

				if (fp_interval != std::get<2>(item)) {

					return false;
				}

				++(*quadruple_sorter2);
			}
			
			delete quadruple_sorter2; quadruple_sorter2;

			delete t_reader; t_reader = nullptr;

			delete t; t = nullptr;

			delete t_file; t_file = nullptr;
		}

#endif

		std::cerr << stxxl::stats_data(*Stats) - Stats_begin << std::endl;
	
		std::cerr << "mem avail: " << MAIN_MEM_AVAIL / 1024 / 1024 << " MB" << std::endl;

		std::cerr << "total I/O volume: " << Stats->get_written_volume() + Stats->get_read_volume() << std::endl;

		std::cerr << "I/O volume per ch: " << (Stats->get_written_volume() + Stats->get_read_volume()) / m_len << std::endl;

		std::cerr << "total peak disk use: " << bm->get_maximum_allocation() << std::endl;

		std::cerr << "peak disk use per ch: " << bm->get_maximum_allocation() / m_len << std::endl;

		return true;
	}
};


#endif // VALIDATE2_H
