#ifndef KLCPA_EM_H
#define KLCPA_EM_H

#include "../../common/namespace.h"

#include "../../common/common.h"

#include "../../common/data.h"

#define STATISTICS_COLLECTION

#define FPN 1

NAMESPACE_KLCP_EM_BEG

//! K-order LCP array builder using eternal memory

//! \param T1 element type of SA array
//! \param T2 element type of LCP array, embrace the representation range of [0, K].
//! \param B each character is represented by one byte in the input string. For condensation, we use an H-bit integer to represent (H / B) characters during the filter phase.
//! \param H partition (SA[i], SA[i - 1]) into two sets: A = {lcp < H / B} and B = {lcp >= H / B}. Assume H = {16, 32, 64, 128} and B = {4, 8}, we have H / B = {2, 4, 8, 16, 32}.
//! \param Alphabet alphabet size in concern

template<typename T1, typename T2, uint8 B, uint16 H, uint16 Alphabet>
class KLCPABuilder {
private:

	//alias
	typedef typename choose_uint_type<H>::uint_type uint_type;

	typedef typename ExVector<uint8>::vector uint8_vector_type;

	typedef typename ExVector<T1>::vector t1_vector_type;

	typedef typename ExVector<T2>::vector t2_vector_type;

	typedef Pair<T1, T1> TupleA;

	typedef TupleGreatComparator1<TupleA> TupleAGreatComparator1; 

	typedef typename ExTupleAscSorter<TupleA, TupleAGreatComparator1>::sorter AscSorterA1; //!< sort TupleA by 1st component in descending order

	typedef typename ExTupleAscSorter<TupleA, TupleAGreatComparator1>::comparator AscComparatorA1;

	typedef TupleLessComparator2<TupleA> TupleALessComparator2; 

	typedef typename ExTupleAscSorter<TupleA, TupleALessComparator2>::sorter AscSorterA2; //!< sort TupleA by (1st, 2nd) components in ascending order

	typedef typename ExTupleAscSorter<TupleA, TupleALessComparator2>::comparator AscComparatorA2;

	typedef TupleGreatComparator2<TupleA> TupleAGreatComparator2;

	typedef typename ExTupleAscSorter<TupleA, TupleAGreatComparator2>::sorter AscSorterA3; //!< sort TupleA by (1st, 2nd) components in descending order

	typedef typename ExTupleAscSorter<TupleA, TupleAGreatComparator2>::comparator AscComparatorA3;

	typedef Pair<T1, uint_type> TupleB;

	typedef TupleLessComparator1<TupleB> TupleBLessComparator1;

	typedef typename ExTupleAscSorter<TupleB, TupleBLessComparator1>::sorter AscSorterB1; //sort TupleB by 1st component in ascending order

	typedef typename ExTupleAscSorter<TupleB, TupleBLessComparator1>::comparator AscComparatorB1;

	typedef Triple<T1, T1, FPTYPE_A> TupleC;

	typedef TupleLessComparator1<TupleC> TupleCLessComparator1;

	typedef typename ExTupleAscSorter<TupleC, TupleCLessComparator1>::sorter AscSorterC1; //sort TupleC by 1st component in ascending order

	typedef typename ExTupleAscSorter<TupleC, TupleCLessComparator1>::comparator AscComparatorC1;

	typedef Quintuple<T1, T1, FPTYPE_A, FPTYPE_A, FPTYPE_A> TupleD;

	typedef TupleLessComparator1<TupleD> TupleDLessComparator1; 

	typedef typename ExTupleAscSorter<TupleD, TupleDLessComparator1>::sorter AscSorterD1; //sort TupleD by 1st component in ascending order

	typedef typename ExTupleAscSorter<TupleD, TupleDLessComparator1>::comparator AscComparatorD1;

	typedef Triple<T1, T1, uint_type> TupleE;

	typedef TupleLessComparator1<TupleE> TupleELessComparator1; 

	typedef typename ExTupleAscSorter<TupleE, TupleELessComparator1>::sorter AscSorterE1; //sort TupleE by 1st component in ascending order

	typedef typename ExTupleAscSorter<TupleE, TupleELessComparator1>::comparator AscComparatorE1;

	typedef Sixtuple<T1, uint8, uint8, uint8, uint8, T2> TupleF; 

	typedef TupleLessComparator1<TupleF> TupleFLessComparator1;

	typedef typename ExTupleAscSorter<TupleF, TupleFLessComparator1>::sorter AscSorterF1; //sort TupleF by 1st component in ascending order

	typedef typename ExTupleAscSorter<TupleF, TupleFLessComparator1>::comparator AscComparatorF1;

	typedef Quadruple<uint8, T1, T2, T2> TupleG;
	
	typedef TupleGreatComparator2<TupleG> TupleGGreatComparator2;
	
	typedef typename ExTupleMinHeap<TupleG, TupleGGreatComparator2, MAX_MEM / 2, MAX_ITEM>::heap MinHeap; //organize TupleG as a min-heap according to (1st, 2nd) components

	typedef typename MinHeap::block_type MinHeapBlock;

	typedef typename stxxl::read_write_pool<MinHeapBlock> MinHeapPool;

	typedef TupleLessComparator2<TupleG> TupleGLessComparator2;
	
	typedef typename ExTupleMaxHeap<TupleG, TupleGLessComparator2, MAX_MEM / 2, MAX_ITEM>::heap MaxHeap; //organize TuplG as a max-heap according to (1st, 2nd) components

	typedef typename MaxHeap::block_type MaxHeapBlock;

	typedef typename stxxl::read_write_pool<MaxHeapBlock> MaxHeapPool;

	typedef Triple<uint8, uint8, T2> TupleH;

	typedef typename ExVector<TupleH>::vector tupleH_vector_type;

	typedef Triple<uint8, uint8, T2> TupleI;

	typedef typename ExVector<TupleI>::vector tupleI_vector_type;

	typedef Quadruple<uint8, uint8, T2, T2> TupleJ;

	typedef typename ExVector<TupleJ>::vector tupleJ_vector_type;


	//data member
	std::string & mSFileName; //!< file name of input string

	std::string & mSAFileName; //!< file name of input suffix array

	std::string & mLCPFileName; //!< file name of output lcp array

	stxxl::syscall_file * mSFile; //<! file descriptor of input string

	stxxl::syscall_file * mSAFile; //<! file descriptor of input suffix array

	stxxl::syscall_file * mLCPFile; //<! file descriptor of lcp array

	uint8_vector_type * mS; //!< external memory vector for reading the input string

	t1_vector_type * mSA; //!< external memory vector for reading the input suffix array

	t2_vector_type * mLCP; //!< external memory vector for writing the final lcp array

	size_t mSLen; //!< length of input string


	uint_type OFFSET[H / B]; //!< e.g., OFFSET = {0xFF00000000000000, 0x00FF00000000000000, ..., 0x00000000000000FF} provided H = 128 and B = 8.

	uint8_vector_type * mLocalLCP; //!< local LCP for LMS suffixes, compare the leftmost (H / B) characters, where (H / B) < std::numeric_limits<uint8>::max()

	t2_vector_type * mGlobalLCP; //!< global LCP for LMS, compare the leftmost K characters

	t1_vector_type * mPA1; //!< lcp(mPA1[i], mPA2[i]) is no less than (H / B)

	t1_vector_type * mPA2; //!< lcp(mPA1[i], mPA2[i]) is no less than (H / B)

	size_t mPALen; //!< length of mPA1 and mPA2

	AscSorterA2 * mAscSorterA21;

	AscSorterA2 * mAscSorterA22;

	AscSorterA3 * mAscSorterA31;

	AscSorterA3 * mAscSorterA32;

	AscSorterC1 * mAscSorterC11;

	AscSorterC1 * mAscSorterC12;

	AscSorterD1 * mAscSorterD11;

	AscSorterD1 * mAscSorterD12;

	tupleH_vector_type * mSortedLMS;
	
	tupleI_vector_type * mInducedL;

	tupleI_vector_type * mInducedS;

	tupleH_vector_type * mSortedL;

#if FPN == 1

	FPTYPE_A mR[K + 1];

#else

	FPTYPE_A mR[K + K / 2 + 1];

#endif

	size_t mLMSDistribution[Alphabet]; //!< record the distribution of LMS suffixes with different starting characters

	size_t mLTypeDistribution[Alphabet]; //!< record the distribution of L-type suffixes with different starting characters

	size_t mSTypeDistribution[Alphabet]; //!< record the distribution of S-type suffixes with different starting characters

	//! retrieve the starting character when inducing the suffix array / lcp array
	struct RetrieveCh {
	private:
		
		const size_t * distribution; //!< distribution of suffixes with different starting characters

		const std::vector<uint8> distributionCh;

		const std::vector<size_t> distributionNum;

		uint8 bktNum;

		uint8 curBktIdx;
	
		uint8 toScan; //!< number of suffixes to scan in current bucket
		
	public:

		//!ctor
		RetrieveCh(const size_t * _distribution, const bool _rightward) {
			
			bktNum = 0;

			for (uint16 i = 0; i < Alphabet; ++i) {
			
				if (_distribution[i] != 0) {
				
					distributionCh[bktNum] = _distribution[i];

					distributionNum[bktNum] = _distribution[i];

					++bktNum;
				}
			}
			
			curBktIdx = (_rightward) ? distributionCh[0] : distributionCh[bktNum - 1];

			toScan = distributionNum[curBktIdx];
		}

		//! retrieve current bucket
		uint8 operator*() {
			
			return distributionCh[curBktIdx];
		}

		//! move rightward one slot, do not return reference
		void operator++() {
		
			if (toScan == 0) {
			
				++curBktIdx;
			}
	
			--toScan;
		}

		//move leftward one slot, do not return reference
		void operator--() {

			if (toScan == 0) {
			
				--curBktIdx;
			}

			--toScan;
		}
	};


	//! dynamically compute the lcp value using O(Alphabet) space and O(Alphabet * n) time
	struct InducingInfo {
	private:

		uint16 inducingCh[Alphabet]; 

		T2 minLCP[Alphabet]; 

	public:
		
		//! ctor
		InducingInfo() {
		
			for (uint16 i = 0; i < Alphabet; ++i) {
			
				inducingCh[i] = std::numeric_limits<uint16>::max();

				minLCP[i] = std::numeric_limits<T2>::max();
			}
		}

		//! after induce suffix i starting with _induceCh from suffix j starting with _ch
		void setCh(const uint8 _inducedCh, const uint8 _ch) {
				
			inducingCh[_inducedCh] = _ch;
		}

		//! after compute lcp for suffix i starting with _induceCh 
		void setLCP(const uint8 _induceCh, const T2 _lcp) {
		
			minLCP[_induceCh] = _lcp;
		}

		//! get minimum lcp for current suffix i starting with _ch
		T2 getLCP(const uint8 _ch) const {
		
			return minLCP[_ch];
		}

		//! update lcp when inducing L-type suffixes
		void updateLCP_L(const uint8 _inducingCh, const T2 _lcp) {
		
			for (uint8 i = _inducingCh; i < Alphabet; ++i) {
			
				if (minLCP[i] > _lcp) minLCP[i] = _lcp;
			}
		}

		//! update lcp when inducing S-type suffixes
		void updateLCP_S(const uint8 _inducingCh, const T2 _lcp){
		
			for (uint8 i = 0; i <= _inducingCh; ++i) {

				if (minLCP[i] > _lcp) minLCP[i] = _lcp;
			}
		}
	};

public:
	
	//function member
	KLCPABuilder(std::string & _sFileName, std::string & _saFileName, std::string & _lcpFileName, size_t _sLen); //ctor

	void test1();

	void computeLMS(); //compute lcp of sorted LMS suffixes

	void filterRun(); //filterate lms pairs with lcp < H / B 

	void computeOffset(); //compute Offset array

	uint8 computeCommonPrefixLen(const uint_type & _lhs, const uint_type & _rhs, const uint8 _maxLen); //compute common prefix length (at most H / B characters)

	void print_uint128(const uint_type _ut); //print uint128

	void print_uint(const uint_type _ut); //print uint_type (e.g., uint64)

	void computeR(); //compute mR

	void iter1Run(size_t _stride, bool _final); //carry one fingerprint

#ifdef DEBUG




	void iter2Run(size_t _stride1, size_t _stride2, bool _final); //carry three fingeprints

	void finalRun(); //figure out LCP_LMS

	void collectInductionInfo(); //collection information for induction.

	void induceL();

	void induceS();
//
//
//
//
//

//
//
//
//	


#endif

};


//! K-order LCP construction using the proposed probabilistic method

template<typename T1, typename T2, uint8 B, uint16 H, uint16 Alphabet>
KLCPABuilder<T1, T2, B, H, Alphabet>::KLCPABuilder(
	std::string & _sFileName,
	std::string & _saFileName,
	std::string & _lcpFileName,
	size_t _sLen) :
	mSFileName(_sFileName),
	mSAFileName(_saFileName),
	mLCPFileName(_lcpFileName),
	mSLen(_sLen),
	mLocalLCP(NULL),
	mGlobalLCP(NULL),
	mPA1(NULL),
	mPA2(NULL),
	mPALen(0),
	mAscSorterA21(NULL),
	mAscSorterA22(NULL),
	mAscSorterC11(NULL),
	mAscSorterC12(NULL),
	mAscSorterD11(NULL),
	mAscSorterD12(NULL){

	
#ifdef STATISTICS_COLLECTION
	
	stxxl::stats *Stats = stxxl::stats::get_instance();

	stxxl::block_manager *bm = stxxl::block_manager::get_instance();

#endif

	//initialize file descriptors
	mSFile = new stxxl::syscall_file(mSFileName,
		stxxl::syscall_file::RDWR | stxxl::syscall_file::DIRECT);

	mSAFile = new stxxl::syscall_file(mSAFileName,
		stxxl::syscall_file::RDWR | stxxl::syscall_file::DIRECT);

	mLCPFile = new stxxl::syscall_file(mLCPFileName,
		stxxl::syscall_file::CREAT | stxxl::syscall_file::RDWR | stxxl::syscall_file::DIRECT);

	//initialize distribution arrays
	for (uint16 i = 0; i < Alphabet; ++i) {
	
		mLMSDistribution[i] = 0;

		mLTypeDistribution[i] = 0;

		mSTypeDistribution[i] = 0;
	}



	//test1();

	///step 1: compute LCP of sorted LMS suffixes using the proposed probablistic method
	computeLMS();

#ifdef DEBUG
	///step 2: collect information for L-type and S-type induction
	collectInductionInfo();

	///step 3: induce LCP of L-type suffixes from LMS ones
	induceL();


	///step 4: induce LCP of S-type suffixes from L-type ones
	//induceS();


#endif 

	delete mSFile; mSFile = NULL;

	delete mSAFile; mSAFile = NULL;

	delete mLCPFile; mLCPFile = NULL;

#ifdef STATISTICS_COLLECTION

	std::cerr << "I/O volume: " << (Stats->get_written_volume() + Stats->get_read_volume()) / mSLen << std::endl;

	std::cerr << "Peak Disk Usage: " << bm->get_maximum_allocation() / mSLen << std::endl;

#endif

}


//! compute reduction ratio in the internal memory to verify the correctness.

template<typename T1, typename T2, uint8 B, uint16 H, uint16 Alphabet>
void KLCPABuilder<T1, T2, B, H, Alphabet>::test1() {

	//load input string
	mS = new uint8_vector_type(mSFile);

	typename uint8_vector_type::bufreader_type *sReader = new typename uint8_vector_type::bufreader_type(*mS);

	uint8 *sBuf = new uint8[mSLen];

	for (size_t i = 0; !sReader->empty(); ++i, ++(*sReader)) {

		sBuf[i] = *(*sReader);
	}

	delete sReader; sReader = NULL; delete mS; mS = NULL;

	//load suffix array
	mSA = new t1_vector_type(mSAFile);

	typename t1_vector_type::bufreader_type *saReader = new typename t1_vector_type::bufreader_type(*mSA);

	T1 *saBuf = new T1[mSLen];

	for (size_t i = 0; !saReader->empty(); ++i, ++(*saReader)) {

		saBuf[i] = *(*saReader);
	}

	delete saReader; saReader = NULL; delete mSA; mSA = NULL;

	//figure out type array
	uint8 *tBuf = new uint8[mSLen]; //do not use bool, enum is also feasible {L_TYPE = 0, S_TYPE = 1, LMS_TYPE = 2, SENTINEL_TYPE = 3}

	tBuf[mSLen - 1] = L_TYPE; //the rightmost is assumed to be L-type.

	for (int i = mSLen - 2; i >= 0; --i) {

		tBuf[i] = (sBuf[i] < sBuf[i + 1] || (sBuf[i] == sBuf[i + 1] && tBuf[i + 1] == S_TYPE)) ? S_TYPE : L_TYPE;
	}

	//figure out and record lms suffixes in lmsBuf
	T1 *lmsBuf = new T1[mSLen];

	size_t lmsNum = 0; //at most mSLen / 2 LMS characters

	for (size_t i = 0; i < mSLen; ++i) {

		if (0 != saBuf[i]) { //pos == 0, no preceding

			if (tBuf[saBuf[i]] == S_TYPE && tBuf[saBuf[i] - 1] == L_TYPE) {

				lmsBuf[lmsNum++] = saBuf[i];
			}
		}

	}

	std::cerr << "lmsNum: " << lmsNum << std::endl;

	T1 curSA, preSA;

	size_t count = 0;

	size_t minDistance = 0;

	for (size_t i = 1; i < lmsNum; ++i) {

		curSA = lmsBuf[i];

		preSA = lmsBuf[i - 1];

		minDistance = mSLen - std::max(preSA, curSA);

		minDistance = std::min(minDistance, static_cast<size_t>(H / B));

		size_t commonPrefixLen = 0;

		for (uint8 j = 0; j < static_cast<uint8>(minDistance); ++j) {

			if (sBuf[preSA + j] != sBuf[curSA + j]) {

				break;
			}

			++commonPrefixLen;
		}

		if (commonPrefixLen == H / B) {

			++count;
		}
	}


	delete sBuf; sBuf = NULL;

	delete saBuf; saBuf = NULL;

	delete tBuf; tBuf = NULL;

	delete lmsBuf; lmsBuf = NULL;


	std::cerr << "count: " << count << " mSLen: " << mSLen << " ratio: " << (double)count / mSLen << std::endl;
}


//! compute lcp of sorted LMS suffixes

template<typename T1, typename T2, uint8 B, uint16 H, uint16 Alphabet>
void KLCPABuilder<T1, T2, B, H, Alphabet>::computeLMS() {

	std::cerr << "start induce LMS" << std::endl;

	std::cerr << "H: " << H << " B: " << B << " H / B: " << H / B << " alphabet: " << Alphabet << std::endl;

	///step 1: pack (SA[i], i), sort tuples by 1st component in descending order
	filterRun();


	///step2: K-order LCP construction using Karp-Rabin
	computeR();

#if FPN == 1

	size_t stride1= K / 2;

	for (; stride1 >= H / B; stride1 >>= 1) {
	
		iter1Run(stride1, (stride1 == H / B));
	}

#else  //FPN = 3

	size_t stride1 = K / 2, stride2 = K / 4;
	
	for (; stride1 > H / B; stride1 >>= 2, stride2 >>= 2) {
	
		iter2Run(stride1, stride2, (stride2 == H / B));
	}
	
	if (stride1 == H / B) {
		
		iter1Run(stride1, true);
	}

#endif

#ifdef DEBUG	
	///step 3: directly compare the last H / B character
	finalRun();
	
#endif
	
	return;
}




//! filterate lcp < H / B

template<typename T1, typename T2, uint8 B, uint16 H, uint16 Alphabet>
void KLCPABuilder<T1, T2, B, H, Alphabet>::filterRun() {
	
	///step 1: pack (SA[i], i) and sort tuples by 1st component in descending orders
	mSA = new t1_vector_type(mSAFile);

	typename t1_vector_type::bufreader_type * saReader = new typename t1_vector_type::bufreader_type(*mSA);
	
	AscSorterA1 * ascSorterA1 = new AscSorterA1(AscComparatorA1(), MAX_MEM / 2);
	
	for (T1 idx = 1; !saReader->empty(); ++idx, ++(*saReader)) {
	
		ascSorterA1->push(TupleA(*(*saReader), idx));		
	}	
	
	delete saReader; saReader = NULL; delete mSA; mSA = NULL;
	
	ascSorterA1->sort();
	
	std::cerr << "ascSorterA1->size(): " << ascSorterA1->size() << std::endl;
	
	
	///step 2: pack (ISA[i], uint_type) and sort tuples by 1st component in ascending order
	mS = new uint8_vector_type(mSFile);
	
	typename uint8_vector_type::bufreader_reverse_type * sReverseReader = new uint8_vector_type::bufreader_reverse_type(*mS);
	
	AscSorterB1 * ascSorterB1 = new AscSorterB1(AscComparatorB1(), MAX_MEM / 2);
		
	uint_type str = 0;
		
	uint8 lastCh, curCh;
	
	uint8 lastType, curType; 
	
	//process the rightmost
	lastCh = *(*sReverseReader); //the rightmost is L-type
		
	lastType = L_TYPE;
	
	str <<= B;
	
	str |= lastCh;
	
	++(*sReverseReader); //do not perform ++(*ascSorterA1)
	
	//process the remaining
	for (; !sReverseReader->empty(); ++(*sReverseReader), ++(*ascSorterA1)) {
			
		curCh = *(*sReverseReader);
			
		curType = (curCh < lastCh || (curCh == lastCh && lastType == S_TYPE)) ? S_TYPE : L_TYPE;			
	
		if (curType == L_TYPE && lastType == S_TYPE) { //check if previously scanned suffix is LMS
		
			ascSorterB1->push(TupleB((*ascSorterA1)->second, str));
		}
	
		str <<= B;
			
		str |= curCh;
		
		lastCh = curCh;
	
		lastType = curType;
	}
	
	++(*ascSorterA1); //the leftmost must not be LMS, skip it.
	
	std::cerr << "ascSorterA1->size(): " << ascSorterA1->size() << std::endl;
	
	assert(ascSorterA1->size() == 0);

	delete ascSorterA1; ascSorterA1 = NULL;
	
	delete sReverseReader; sReverseReader = NULL; delete mS; mS = NULL;
	
	ascSorterB1->sort();
	
	std::cerr << "ascSorterB1->size(): " << ascSorterB1->size() << std::endl;
	
	
	///step 3: classify each pair (LMS_SA[i], LMS_SA[i - 1] into two sets: {lcp < H / B} and {lcp >= H / B}
	computeOffset();

	mSA = new t1_vector_type(mSAFile);

	saReader = new typename t1_vector_type::bufreader_type(*mSA);

	mLocalLCP = new uint8_vector_type(); mLocalLCP->resize(mSLen);
	
	typename uint8_vector_type::bufwriter_type *localLCPWriter = new typename uint8_vector_type::bufwriter_type(*mLocalLCP);
	
	mPA1 = new t1_vector_type(); mPA1->resize(mSLen);
	
	typename t1_vector_type::bufwriter_type *pa1Writer = new typename t1_vector_type::bufwriter_type(*mPA1);
	
	mPA2 = new t1_vector_type(); mPA2->resize(mSLen);
	
	typename t1_vector_type::bufwriter_type *pa2Writer = new typename t1_vector_type::bufwriter_type(*mPA2);
	
	mPALen = 0; 	
			
	uint_type preStr;
	
	T1 preSA, curSA;
	
	size_t minDistance;
		
	uint8 commonPrefixLen; // <= H / B < std::numeric_limits<uint8>::max()
	
	T1 idx = 1; //recall that, the position index starts from 1
	
	//process the first element, of which the lcp = 0
	while (idx != (*ascSorterB1)->first) {
	
		++idx;
		
		++(*saReader);
	}
	
	(*localLCPWriter) << static_cast<uint8>(0);
	
	preStr = (*ascSorterB1)->second;
	
	preSA = *(*saReader);
	
	++(*ascSorterB1);
	
	//process the remaining elements
	for (; !ascSorterB1->empty(); ++(*ascSorterB1)) {
	
		while (idx != (*ascSorterB1)->first) {
	
			++idx;
	
			++(*saReader);
		}
	
		const TupleB & tupleB = *(*ascSorterB1);
	
		curSA = *(*saReader);
	
		minDistance = mSLen - std::max(preSA, curSA);
	
		minDistance = std::min(minDistance, static_cast<size_t>(H / B));
		
		commonPrefixLen = computeCommonPrefixLen(tupleB.second, preStr, static_cast<uint8>(minDistance));
					
		if (commonPrefixLen == static_cast<uint8>(H / B)) {
			
			(*pa1Writer) << curSA;
	
			(*pa2Writer) << preSA;
	
			++mPALen;
		}
	
		(*localLCPWriter) << commonPrefixLen;
	
		preStr = tupleB.second;
	
		preSA = curSA;
	}
		
	localLCPWriter->finish(); delete localLCPWriter; localLCPWriter = NULL;
	
	pa1Writer->finish(); delete pa1Writer; pa1Writer = NULL; mPA1->resize(mPALen);
	
	pa2Writer->finish(); delete pa2Writer; pa2Writer = NULL; mPA2->resize(mPALen);
	
	delete saReader; saReader = NULL; delete mSA; mSA = NULL;
	
	delete ascSorterB1; ascSorterB1 = NULL;
	
	std::cerr << "mPALen: " << mPALen << std::endl;
	
	std::cerr << "reduction ratio: " << static_cast<double>(mPALen) / mSLen << std::endl;
	
	
	///step 4: compute LCP between mPA1[i] and mPA2[i] for all i in [0, mPALen)
	mAscSorterA21 = new AscSorterA2(AscComparatorA2(), MAX_MEM / 6);
	
	mAscSorterA22 = new AscSorterA2(AscComparatorA2(), MAX_MEM / 6);
	
	typename t1_vector_type::bufreader_type * pa1Reader = new typename t1_vector_type::bufreader_type(*mPA1);
	
	typename t1_vector_type::bufreader_type * pa2Reader = new typename t1_vector_type::bufreader_type(*mPA2);
	
	for (T1 idx = 1; !pa1Reader->empty(); ++idx, ++(*pa1Reader), ++(*pa2Reader)) {
	
		mAscSorterA21->push(TupleA(*(*pa1Reader), idx));
	
		mAscSorterA22->push(TupleA(*(*pa2Reader), idx));
	}
		
	delete pa1Reader; pa1Reader = NULL;
	
	delete pa2Reader; pa2Reader = NULL; delete mPA2; mPA2 = NULL;
	
	mAscSorterA21->sort();
	
	mAscSorterA22->sort();
}


//! compute Offset array

template<typename T1, typename T2, uint8 B, uint16 H, uint16 Alphabet>
void KLCPABuilder<T1, T2, B, H, Alphabet>::computeOffset() {

	OFFSET[0] = static_cast<uint_type>(static_cast<uint32>(pow(2, B) - 1));

	for (uint8 i = 1; i < H / B; ++i) {

		OFFSET[i] = OFFSET[i - 1] << B;
	}

	return;
}


//! compute common prefix length

template<typename T1, typename T2, uint8 B, uint16 H, uint16 Alphabet>
inline uint8 KLCPABuilder<T1, T2, B, H, Alphabet>::computeCommonPrefixLen(const uint_type & _lhs, const uint_type & _rhs, const uint8 _maxLen) {
	
	uint8 commonPrefixLen = 0;

	for (uint8 i = 0; i < _maxLen; ++i) {

		if ((_lhs & OFFSET[i]) != (_rhs & OFFSET[i])) {
				
			break;
		}

		++commonPrefixLen;
	}


//	print_uint128(_lhs);
	
//	print_uint128(_rhs);

//	std::cerr << "commonPrefixLen: " << commonPrefixLen << std::endl;
	
//	std::cin.get();

	return commonPrefixLen;
}


//! output uint128 integer

template<typename T1, typename T2, uint8 B, uint16 H, uint16 Alphabet>
void KLCPABuilder<T1, T2, B, H, Alphabet>::print_uint128(const uint_type _ut) {

	uint64 offset64 = static_cast<uint64>(pow(2, B) - 1);

	uint64 low = _ut.getLow();

	for (uint8 i = 0; i < 64 / B; ++i) {

		std::cerr << (low & offset64) << " ";

		low >>= B;
	}

	uint64 high = _ut.getHigh();

	for (uint8 i = 0; i < 64 / B; ++i) {

		std::cerr << (high & offset64) << " ";

		high >>= B;
	}

	std::cerr << std::endl;
}


//!output uint_type integer (except for the situation when typeof(uinttype) == typeof(uint128))
template<typename T1, typename T2, uint8 B, uint16 H, uint16 Alphabet>
void KLCPABuilder<T1, T2, B, H, Alphabet>::print_uint(const uint_type _ut) {

	for (int i = 0; i < H / B; ++i) {

		std::cerr << (_ut & OFFSET[0]) << " ";

		_ut >>= B;
	}

	std::cerr << std::endl;
}




//! compute mR

template<typename T1, typename T2, uint8 B, uint16 H, uint16 Alphabet>
void KLCPABuilder<T1, T2, B, H, Alphabet>::computeR() {

	mR[0] = 1;

#if FPN 1

	for (size_t i = 1; i <= K; ++i) {
	
		mR[i] = static_cast<FPTYPE_A>((static_cast<FPTYPE_B>(mR[i - 1]) * R) % P);
	}

#else
		
	for (size_t i = 1; i <= K + K / 2; ++i) {
	
		mR[i] = static_cast<FPTYPE_A>((static_cast<FPTYPE_B>(mR[i - 1]) * R) % P);
	}

#endif

}


//! carry one fingerprint per iteration

template<typename T1, typename T2, uint8 B, uint16 H, uint16 Alphabet>
void KLCPABuilder<T1, T2, B, H, Alphabet>::iter1Run(size_t _stride, bool _final) {

	///step 1: compute fingerprints
	if (mAscSorterC11 == NULL) mAscSorterC11 = new AscSorterC1(AscComparatorC1(), MAX_MEM / 6);

	if (mAscSorterC12 == NULL) mAscSorterC12 = new AscSorterC1(AscComparatorC1(), MAX_MEM / 6);

	mS = new uint8_vector_type(*mSFile);

	typename uint8_vector_type::bufreader_type *sReader = new typename uint8_vector_type::bufreader_type(*mS);

	size_t fpBufSize = MAX_MEM / 3 / sizeof(FPTYPE_A);

	FPTYPE_A *fpBuf = new FPTYPE_A[fpBufSize];

	size_t toRead = mSLen, reading;

	T1 startOffset, endOffset, pos1, pos2;

	FPTYPE_A fpInterval, fp, preFP = 0;

	bool isFirstBlock = true; //check if currently processing the leftmost block

	//process in a manner of block by block and one block per iteration
	while (toRead) {

		if (isFirstBlock) {
		
			isFirstBlock = false;

			reading = (toRead > fpBufSize) ? fpBufSize : toRead;

			toRead -= reading;
			
			fpBuf[0] = *(*sReader) + 1;
			
			++(*sReader);
			
			for (size_t i = 1; i < reading; ++i, ++(*sReader)) {
			
				fpBuf[i] = static_cast<FPTYPE_A>((static_cast<FPTYPE_B>(fpBuf[i - 1] * R + (*(*sReader) + 1)) % P);
			}
			
			startOffset = 0;
			
			endOffset = startOffset + reading - _stride;
		}
		else {

			reading = (toRead > (fpBufSize - _stride + 1)) ? (fpBufSize - _stride + 1) : toRead;
			
			toRead -= reading;
							
			for (size_t i = 0; i < reading; ++i, ++(*sReader)) {
			
				fpBuf[_stride - 1 + i] = static_cast<FPTYPE_A>((static_cast<FPTYPE_B>(fpBuf[_stride - 1 + i - 1]) * R + (*(*sReader) + 1)) % P);
			}
						
			startOffset = endOffset + 1;
			
			endOffset = startOffset + reading - 1;
		}

		std::cerr << "stride: " << _stride << " startOffset: " << startOffset << " endOffset: " << endOffset << std::endl;

		while (!mAscSorterA21->empty() && (*mAscSorterA21)->first <= endOffset) {
		
			const TupleA & tuple = *(*mAscSorterA21);
		
			pos1 = tuple.first - startOffset;
		
			fp = (0 == pos1) ? preFP : fpBuf[pos1 - 1];
		
			pos2 = pos1 + _stride - 1;
		
			fpInterval = static_cast<FPTYPE_A>((fpBuf[pos2] - (static_cast<FPTYPE_B>(fp) * mR[_stride]) % P + P) % P);
		
			mAscSorterC11->push(TupleC(tuple.second, tuple.first, fpInterval));
		
			++(*mAscSorterA21);
		}
				
		while (!mAscSorterA22->empty() && (*mAscSorterA22)->first <= endOffset) {
		
			const TupleA & tuple = *(*mAscSorterA22);
		
			pos1 = tuple.first - startOffset;
		
			fp = (0 == pos1) ? preFP : fpBuf[pos1 - 1];
		
			pos2 = pos1 + _stride - 1;
		
			fpInterval = static_cast<FPTYPE_A>((fpBuf[pos2] - (static_cast<FPTYPE_B>(fp) * mR[_stride]) % P + P) % P);
		
			mAscSorterC12->push(TupleC(tuple.second, tuple.first, fpInterval));
		
			++(*mAscSorterA22);
		}
			
		preFP = fpBuf[endOffset - startOffset];
		
		for (size_t i = 0; i < _stride - 1; ++i) {
		
			fpBuf[i] = fpBuf[endOffset + 1 - startOffset + i];
		}
	}

	delete sReader; sReader = NULL; delete mS; mS = NULL;

	delete fpBuf; fpBuf = NULL;
	
	//process the remaining elements
	for (; !mAscSorterA21->empty(); ++(*mAscSorterA21)) {
				
		const TupleA & tuple = *(mAscSorterA21);
	
		mAscSorterC11->push(TupleC(tuple.second, tuple.first, P + 1));
	}

	for (; !mAscSorterA22->empty(); ++(*mAscSorterA22)) {
	
		const TupleA & tuple = *(mAscSorterA22);

		mAscSorterC12->push(TupleC(tuple.second, tuple.first, P + 1));
	}

	mAscSorterC11->sort();

	mAscSorterC12->sort();


	///step 2: compare fingerprints
	if (_final) {
	
		delete mAscSorterA21; mAscSorterA21 = NULL;

		delete mAscSorterA22; mAscSorterA22 = NULL;

		mAscSorterA31 = new AscSorterA3(AscComparatorA3(), MAX_MEM / 4);

		mAscSorterA32 = new AscSorterA3(AscComparatorA3(), MAX_MEM / 4);

		for (T1 i = 1; !mAscSorterC11->empty(); ++i, ++(*mAscSorterC11), ++(*mAscSorterC12)) {

			if ((*mAscSorterC11)->third == (*mAscSorterC12)->third && (*mAscSorterC11)->third != P + 1) {

				mAscSorterA31->push(TupleA((*mAscSorterC11)->second + _stride, i));

				mAscSorterA32->push(TupleA((*mAscSorterC12)->second + _stride, i));
			}
			else {

				mAscSorterA31->push(TupleA((*mAscSorterC11)->second, i));

				mAscSorterA32->push(TupleA((*mAscSorterC12)->second, i));
			}
		}

		delete mAscSorterC11; mAscSorterC11 = NULL;

		delete mAscSorterC12; mAscSorterC12 = NULL;

		mAscSorterA31->sort();

		mAscSorterA32->sort();
	}
	else {

		mAscSorterA21->clear();

		mAscSorterA22->clear();

		for (T1 i = 1; !mAscSorterC11->empty(); ++i, ++(*mAscSorterC11), ++(*mAscSorterC12)) {

			if ((*mAscSorterC11)->third == (*mAscSorterC12)->third && (*mAscSorterC11)->third != P + 1) {

				mAscSorterA21->push(TupleA((*mAscSorterC11)->second + _stride, i));

				mAscSorterA22->push(TupleA((*mAscSorterC12)->second + _stride, i));
			}
			else {

				mAscSorterA21->push(TupleA((*mAscSorterC11)->second, i));

				mAscSorterA22->push(TupleA((*mAscSorterC12)->second, i));
			}
		}

		mAscSorterC11->clear();

		mAscSorterC12->clear();

		mAscSorterA21->sort();

		mAscSorterA22->sort();
	}

	return;
}


//! carry three fingerprints per iteration

template<typename T1, typename T2, uint8 B, uint16 H, uint16 Alphabet>
void KLCPABuilder<T1, T2, B, H, Alphabet>::iter2Run(size_t _stride1, size_t _stride2, bool _final) {

	///step 1: compute fingerprints
	if (mAscSorterD11 == NULL) mAscSorterD11 = new AscSorterD1(AscComparatorD1(), MAX_MEM / 6);

	if (mAscSorterD12 == NULL) mAscSorterD12 = new AscSorterD1(AscComparatorD1(), MAX_MEM / 6);

	mS = new uint8_vector_type(*mSFile);

	typename uint8_vector_type::bufreader_type *sReader = new typename uint8_vector_type::bufreader_type(*mS);

	size_t fpBufSize = MAX_MEM / 3 / sizeof(FPTYPE_A);

	FPTYPE_A *fpBuf = new FPTYPE_A[fpBufSize];

	size_t toRead = mSLen, reading;

	T1 startOffset, endOffset, pos1, pos2, pos3, pos4;

	FPTYPE_A fpInterval1, fpInterval2, fpInterval3, fp, preFP = 0;

	bool isFirstBlock = true;

	//process in a manner of block by block
	while (toRead) {

		if (isFirstBlock) {

			isFirstBlock = false;

			reading = (toRead > fpBufSize) ? fpBufSize : toRead;

			toRead -= reading;

			fpBuf[0] = *(*sReader) + 1;

			++(*sReader);

			for (size_t i = 1; i < reading; ++i, ++(*sReader)) {

				fpBuf[i] = static_cast<FPTYPE_A>((static_cast<FPTYPE_B>(fpBuf[i - 1] * R + (*(*sReader) + 1)) % P);
			}

			startOffset = 0;

			endOffset = startOffset + reading - _stride1 - _stride2;
		}
		else {

			reading = (toRead > (fpBufSize - _stride1 - _stride2 + 1)) ? (fpBufSize - _stride1 - _stride2 + 1) : toRead;

			toRead -= reading;

			for (size_t i = 0; i < reading; ++i, ++(*sReader)) {

				fpBuf[_stride1 + _stride2 - 1 + i] = static_cast<FPTYPE_A>((static_cast<FPTYPE_B>(fpBuf[_stride1 + _stride2 - 1 + i - 1]) * R + (*(*sReader) + 1)) % P);
			}

			startOffset = endOffset + 1;

			endOffset = startOffset + reading - 1;
		}

		std::cerr << "stride1: " << _stride1 << " stride2: " << _stride2 << " startOffset: " << startOffset << " endOffset: " << endOffset << std::endl;

		while (!mAscSorterA21->empty() && (*mAscSorterA21)->first <= endOffset) {

			const TupleA & tuple = *(*mAscSorterA21);

			pos1 = tuple.first - startOffset;

			fp1 = (0 == pos1) ? preFP : fpBuf[pos1 - 1];

			pos2 = pos1 + _stride2 - 1;

			fpInterval1 = static_cast<FPTYPE_A>((fpBuf[pos2] - (static_cast<FPTYPE_B>(fp1) * mR[_stride2]) % P + P) % P);

			pos3 = pos1 + _stride1 - 1;
			
			fpInterval2 = static_cast<FPTYPE_A>((fpBuf[pos3] - (static_cast<FPTYPE_B>(fp1) * mR[_stride1]) % P + P) % P);

			pos4 = pos1 + _stride1 + _stride2 - 1;

			fpInterval3 = static_cast<FPTYPE_A>((fpBuf[pos4] - (static_cast<FPTYPE_B>(fp1) * mR[_stride1 + _stride2]) % P + P) % P);

			mAscSorterD11->push(TupleD(tuple.second, tuple.first, fpInterval1, fpInterval2, fpInterval3));

			++(*mAscSorterA21);
		}

		while (!mAscSorterA22->empty() && (*mAscSorterA22)->first <= endOffset) {

			const TupleA & tuple = *(*mAscSorterA22);

			pos1 = tuple.first - startOffset;

			fp1 = (0 == pos1) ? preFP : fpBuf[pos1 - 1];

			pos2 = pos1 + _stride2 - 1;

			fpInterval1 = static_cast<FPTYPE_A>((fpBuf[pos2] - (static_cast<FPTYPE_B>(fp1) * mR[_stride2]) % P + P) % P);

			pos3 = pos1 + _stride1 - 1;

			fpInterval2 = static_cast<FPTYPE_A>((fpBuf[pos3] - (static_cast<FPTYPE_B>(fp1) * mR[_stride1]) % P + P) % P);

			pos4 = pos1 + _stride1 + _stride2 - 1;

			fpInterval3 = static_cast<FPTYPE_A>((fpBuf[pos4] - (static_cast<FPTYPE_B>(fp1) * mR[_stride1 + _stride2]) % P + P) % P);

			mAscSorterD12->push(TupleD(tuple.second, tuple.first, fpInterval1, fpInterval2, fpInterval3));

			++(*mAscSorterA22);
		}

		preFP = fpBuf[endOffset - startOffset];

		for (size_t i = 0; i < _stride1 + _stride2 - 1; ++i) {

			fpBuf[i] = fpBuf[endOffset + 1 - startOffset + i];
		}
	}

	//process [0, _stride2 - 1], fpInterval3 = P + 1
	startOffset = endOffset + 1;

	endOffset = startOffset + _stride2 - 1;

	while (!mAscSorterA21->empty() && (*mAscSorterA21)->first <= endOffset) {

		const TupleA & tuple = *(*mAscSorterA21);

		pos1 = tuple.first - startOffset;

		fp1 = (0 == pos1) ? preFP : fpBuf[pos1 - 1];

		pos2 = pos1 + _stride2 - 1;

		fpInterval1 = static_cast<FPTYPE_A>((fpBuf[pos2] - (static_cast<FPTYPE_B>(fp1) * mR[_stride2]) % P + P) % P);

		pos3 = pos1 + _stride1 - 1;

		fpInterval2 = static_cast<FPTYPE_A>((fpBuf[pos3] - (static_cast<FPTYPE_B>(fp1) * mR[_stride1]) % P + P) % P);

		mAscSorterD11->push(TupleD(tuple.second, tuple.first, fpInterval1, fpInterval2, P + 1));

		++(*mAscSorterA21);
	}

	while (!mAscSorterA22->empty() && (*mAscSorterA22)->first <= endOffset) {

		const TupleA & tuple = *(*mAscSorterA22);

		pos1 = tuple.first - startOffset;

		fp1 = (0 == pos1) ? preFP : fpBuf[pos1 - 1];

		pos2 = pos1 + _stride2 - 1;

		fpInterval1 = static_cast<FPTYPE_A>((fpBuf[pos2] - (static_cast<FPTYPE_B>(fp1) * mR[_stride2]) % P + P) % P);

		pos3 = pos1 + _stride1 - 1;

		fpInterval2 = static_cast<FPTYPE_A>((fpBuf[pos3] - (static_cast<FPTYPE_B>(fp1) * mR[_stride1]) % P + P) % P);

		mAscSorterD12->push(TupleD(tuple.second, tuple.first, fpInterval1, fpInterval2, P + 1));

		++(*mAscSorterA22);
	}

	//process [_stride2, _stride1 - 1], fpInterval2 = fpInterval3 = P + 1
	endOffset = startOffset + _stride1 - 1;

	while (!mAscSorterA21->empty() && (*mAscSorterA21)->first <= endOffset) {

		const TupleA & tuple = *(*mAscSorterA21);

		pos1 = tuple.first - startOffset;

		fp1 = fpBuf[pos1 - 1];

		pos2 = pos1 + _stride2 - 1;

		fpInterval1 = static_cast<FPTYPE_A>((fpBuf[pos2] - (static_cast<FPTYPE_B>(fp1) * mR[_stride2]) % P + P) % P);

		mAscSorterD11->push(TupleD(tuple.second, tuple.first, fpInterval1, P + 1, P + 1));

		++(*mAscSorterA21);
	}

	while (!mAscSorterA22->empty() && (*mAscSorterA22)->first <= endOffset) {

		const TupleA & tuple = *(*mAscSorterA22);

		pos1 = tuple.first - startOffset;

		fp1 = fpBuf[pos1 - 1];

		pos2 = pos1 + _stride2 - 1;

		fpInterval1 = static_cast<FPTYPE_A>((fpBuf[pos2] - (static_cast<FPTYPE_B>(fp1) * mR[_stride2]) % P + P) % P);

		mAscSorterD12->push(TupleD(tuple.second, tuple.first, fpInterval1, P + 1, P + 1));

		++(*mAscSorterA22);
	}

	delete sReader; sReader = NULL; delete mS; mS = NULL;

	delete fpBuf; fpBuf = NULL;

	//process [_stride1, _stride1 + _stride2 - 1), fpInterval1 = fpInterval2 = fpInterval3 = P + 1
	for (; !mAscSorterA21->empty(); ++(*mAscSorterA21)) {

		const TupleA & tuple = *(mAscSorterA21);

		mAscSorterD11->push(TupleD(tuple.second, tuple.first, P + 1, P + 1, P + 1));
	}

	for (; !mAscSorterA22->empty(); ++(*mAscSorterA22)) {

		const TupleA & tuple = *(mAscSorterA22);

		mAscSorterD12->push(TupleC(tuple.second, tuple.first, P + 1, P + 1, P + 1));
	}

	mAscSorterD11->sort();

	mAscSorterD12->sort();


	///step2: compare fingerprints
	if (_final) {

		delete mAscSorterA21; mAscSorterA21 = NULL;

		delete mAscSorterA22; mAscSorterA22 = NULL;

		mAscSorterA31 = new AscSorterA3(AscComparatorA3(), MAX_MEM / 4);

		mAscSorterA32 = new AscSorterA3(AscComparatorA3(), MAX_MEM / 4);

		for (T1 i = 1; !mAscSorterD11->empty(); ++i, ++(*mAscSorterD11), ++(*mAscSorterD12)) {

			if ((*mAscSorterD11)->forth == (*mAscSorterD12)->forth && (*mAscSorterD11)->forth != P + 1) { //check fpInterval2

				if ((*mAscSorterD11)->fifth == (*mAscSorterD12)->fifth && (*mAscSorterD11)->fifth != P + 1) { //check fpInterval3

					mAscSorterA31->push(TupleA((*mAscSorterD11)->second + _stride1 + _stride2, i));

					mAscSorterA32->push(TupleA((*mAscSorterD12)->second + _stride1 + _stride2, i));
				}
				else { //fpInterval3 not equal

					mAscSorterA31->push(TupleA((*mAscSorterD11)->second + _stride1, i));

					mAscSorterA32->push(TupleA((*mAscSorterD12)->second + _stride1, i));
				}
			}
			else { //fpInterval2 not equal

				if ((*mAscSorterD11)->third == (*mAscSorterD12)->third && (*mAscSorterD11)->third != P + 1) { //check fpInterval1

					mAscSorterA31->push(TupleA((*mAscSorterD11)->second + _stride2, i));

					mAscSorterA32->push(TupleA((*mAscSorterD12)->second + _stride2, i));
				}
				else { //fpInterval1 not equal

					mAscSorterA31->push(TupleA((*mAscSorterD11)->second, i));

					mAscSorterA32->push(TupleA((*mAscSorterD12)->second, i));
				}
			}
		}

		delete mAscSorterD11; mAscSorterD11 = NULL;

		delete mAscSorterD12; mAscSorterD12 = NULL;
	}
	else {

		mAscSorterA21->clear();

		mAscSorterA22->clear();

		for (T1 i = 1; !mAscSorterD11->empty(); ++i, ++(*mAscSorterD11), ++(*mAscSorterD12)) {

			if ((*mAscSorterD11)->forth == (*mAscSorterD12)->forth && (*mAscSorterD11)->forth != P + 1) { //check fpInterval2

				if ((*mAscSorterD11)->fifth == (*mAscSorterD12)->fifth && (*mAscSorterD11)->fifth != P + 1) { //check fpInterval3

					mAscSorterA21->push(TupleA((*mAscSorterD11)->second + _stride1 + _stride2, i));

					mAscSorterA22->push(TupleA((*mAscSorterD12)->second + _stride1 + _stride2, i));
				}
				else { //fpInterval3 not equal

					mAscSorterA21->push(TupleA((*mAscSorterD11)->second + _stride1, i));

					mAscSorterA22->push(TupleA((*mAscSorterD12)->second + _stride1, i));
				}
			}
			else { //fpInterval2 not equal

				if ((*mAscSorterD11)->third == (*mAscSorterD12)->third && (*mAscSorterD11)->third != P + 1) { //check fpInterval1

					mAscSorterA21->push(TupleA((*mAscSorterD11)->second + _stride2, i));

					mAscSorterA22->push(TupleA((*mAscSorterD12)->second + _stride2, i));
				}
				else { //fpInterval1 not equal

					mAscSorterA21->push(TupleA((*mAscSorterD11)->second, i));

					mAscSorterA22->push(TupleA((*mAscSorterD12)->second, i));
				}
			}
		}

		mAscSorterD11->clear();

		mAscSorterD12->clear();
	}

	if (_final) {
	
		mAscSorterA31->sort();

		mAscSorterA32->sort();
	}
	else {

		mAscSorterA21->sort();

		mAscSorterA22->sort();
	}

	return;
}



#ifdef DEBUG
template<typename T1, typename T2, uint8 B, uint16 H, uint16 Alphabet>
void KLCPABuilder<T1, T2, B, H, Alphabet>::finalRun() {

	///step 1: pack (idx, IPA[idx], str)
	mS = new uint8_vector_type(mSFile);

	typename uint8_vector_type::bufreader_reverse_type * sReverseReader = new typename uint8_vector_type::bufreader_reverse_type(*mS);

	AscSorterE1 * ascSorterE11 = new AscSorterE1(AscComparatorE1(), MAX_MEM / 4);

	AscSorterE1 * ascSorterE12 = new AscSorterE1(AscComparatorE1(), MAX_MEM / 4);
	
	uint_type str = 0;

	while ((*mAscSorterA31)->first == mSLen) { //must not be empty

		ascSorterE11->push(TupleE((*mAscSorterA31)->second, (*mAscSorterA31)->first, str);

		++(*mAscSorterA31);
	}

	while ((*mAscSorterA32)->first == mSLen) { //must not be empty

		ascSorterE12->push(TupleE((*mAscSorterA32)->second, (*mAscSorterA32)->first, str);

		++(*mAscSorterA32);
	}

	for (T1 idx = mSLen - 1; !sReverseReader->empty(); --idx, ++(*sReverseReader)) {
		
		str <<= B;

		str |= *(*sReverseReader);

		while (!mAscSorterA31->empty() && (*mAscSorterA31)->first == idx) {
		
			ascSorterE11->push(TupleE((*mAscSorterA31)->second, ги*mAscSorterA31)->first, str);

			++(*mAscSorterA31);
		}

		while (!mAscSorterA32->empty() && (*mAscSorterA32)->first == idx) {
		
			ascSorterE12->push(TupleE((*mAscSorterA32)->second, ги*mAscSorterA32)->first, str);

			++(*mAscSorterA32);
		}
	}

	delete mAscSorterA31; mAscSorterA31 = NULL;

	delete mAscSorterA32; mAscSorterA32 = NULL;

	delete sReverseReader; sReverseReader = NULL; delete mS; mS = NULL;

	ascSorterE11->sort();

	ascSorterE12->sort();
	

	///step 2: compare fingerprints
	typename t1_vector_type::bufreader_type *pa1Reader = new typename t1_vector_type::bufreader_type(*mPA1);

	typename uint8_vector_type::bufreader_type *localLCPReader = new typename uint8_vector_type::bufreader_type(*mLocalLCP);

	typename t2_vector_type::bufwriter_type *globalLCPWriter = new typename t2_vector_type::bufwriter_type(*mGlobalLCP);

	size_t minDistance; 

	uint8 curLCP, commonPrefixLen;

	for (; !localLCPReader->empty(); ++(*localLCPReader)) {
	
		curLCP = *(*localLCPReader);

		if (curLCP == static_cast<uint8>(H / B)) { //must be updated
		
			const TupleE & tuple1 = *(*ascSorterE11);

			const TupleE & tuple2 = *(*ascSorterE12);

			minDistance = mSLen - std::max(tuple1.second, tuple2.second);
			
			minDistance = std::min(minDistance, static_cast<size_t>(H / B));

			commonPrefixLen = computeCommonPrefixLen(tuple1.third, tuple2.third, static_cast<uint8>(minDistance));

			curLCP = tuple1.second + commonPrefixLen - *(*pa1Reader);

			++(*ascSorterE11);

			++(*ascSorterE12);

			++(*pa1Reader);
		}
		
		(*globalLCPWriter) << curLCP;
	}

	globalLCPWriter->finish();

	delete pa1Reader; pa1Reader = NULL; delete mPA1; mPA1 = NULL;

	delete localLCPReader; localLCPReader = NULL; delete mLocalLCP; mLocalLCP = NULL;

	delete globalLCPWriter; globalLCPWriter = NULL;

	delete ascSorterE11; ascSorterE11 = NULL;

	delete ascSorterE12; ascSorterE12 = NULL;
}

//! collect collect information of preceding characters / types
template<typename T1, typename T2, uint8 B, uint16 H, uint16 Alphabet>
void KLCPABuilder<T1, T2, B, H, Alphabet>::collectInductionInfo() {

	///step 1: collect information of preceding characters / types
	mSA = new t1_vector_type(mSAFile);

	typename t1_vector_type::bufreader_type * saReader = new typename t1_vector_type::bufreader_type(*mSA);

	AscSorterA1 * ascSorterA1 = new AscSorterA1(AscComparatorA1(), MAX_MEM / 2);

	for (T1 idx = 1; !saReader->empty(); ++(*saReader), ++idx) {

		ascSorterA1->push(TupleA(*(*saReader), idx));
	}

	delete saReader; saReader = NULL; delete mSA; mSA = NULL;

	//
	ascSorterA1->sort();

	uint8 preCh, curCh, lastCh;

	uint8 preType, curType;

	uint16 repCount = 0;

	size_t lmsNum = 0, lTypeNum = 0, sTypeNum = 0;

	mS = new uint8_vector_type(mSFile);

	typename uint8_vector_type::bufreader_reverse_type * sReverseReader = new uint8_vector_type::bufreader_reverse_type(*mS);

	AscSorterF1 * ascSorterF1 = new AscSorterF1(AscComparatorF1(), MAX_MEM / 2);

	curCh = *(*sReverseReader); //the rightmost must be L-type

	curType = L_TYPE;

	++lTypeNum;

	++mLTypeDistribution[curCh];

	++(*sReverseReader);

	for (; !sReverseReader->empty(); ++(*sReverseReader), ++(*ascSorterA1)) {

		//update preCh
		preCh = *(*sReverseReader);

		//determine preType (L-type or S-type)
		if (preCh < curCh || (preCh == curCh && curType == S_TYPE)) {

			preType = S_TYPE;

			++sTypeNum;

			++mSTypeDistribution[preCh];
		}
		else {

			preType = L_TYPE;

			++lTypeNum;

			++mLTypeDistribution[preCh];
		}

		//check if LMS-type
		if (curType == S_TYPE && preType == L_TYPE) {

			curType = LMS_TYPE;

			++lmsNum;

			++mLMSDistribution[preCh];
		}

		//determine repetition count
		if (curCh == lastCh) { //identical, plus 1
		
			repCount = (repCount == K) ? K : (repCount + 1);
		}
		else { //not identical, reset
		
			repCount = 1;
		}

		ascSorterF1->push(TupleF((*ascSorterA1)->second, curCh, curType, preCh, preType, repCount));

		lastCh = curCh;

		curCh = preCh;

		curType = preType;
	}

	//the preceding of leftmost character is assumed to be the sentinel
	preCh = 0; //represent the sentinel by 0

	preType = SENTINEL_TYPE;

	//determine repetition count
	if (curCh == lastCh) { //identical, plus 1

		repCount = (repCount == K) ? K : (repCount + 1);
	}
	else { //not identical, reset

		repCount = 1;
	}

	ascSorterF1->push(TupleF((*ascSorterA1)->second, curCh, curType, preCh, preType, repCount));

	delete sReverseReader; sReverseReader = NULL; delete mS; mS = NULL;

	delete ascSorterA1; ascSorterA1 = NULL;


	//
	ascSorterF1->sort();

	std::cerr << "lmsNum: " << lmsNum << " lTypeNum: " << lTypeNum << " sTypeNum: " << sTypeNum << std::endl;

	//record <curCh, preCh, repCount> for lms
	mSortedLMS = new tupleH_vector_type(); mSortedLMS->resize(lmsNum);

	typename tupleH_vector_type::bufwriter_type * sortedLMS_Writer = new typename tupleH_vector_type::bufwriter_type(*mSortedLMS);

	//record <preCh, preType, repCount> for L-type
	mInducedL = new tupleI_vector_type(); mInducedL->resize(lTypeNum);

	typename tupleI_vector_type::bufwriter_type * inducedL_Writer = new typename tupleI_vector_type::bufwriter_type(*mInducedL);
	
	//record <preCh, preType, repCount> for S-type
	mInducedS = new tupleI_vector_type(); mInducedS->resize(sTypeNum);

	typename tupleI_vector_type::bufwriter_type * inducedS_Writer = new typename tupleI_vector_type::bufwriter_Type(*mInducedS);

	
	for (; !ascSorterF1->empty(); ++(*ascSorterF1)) {

		const TupleF & tuple = *(*ascSorterF1);

		if (tuple.third == LMS_TYPE) {

			(*sortedLMS_Writer) << TupleH(tuple.second, tuple.forth, tuple.sixth); //(curCh, preCh, repCount)
		}

		if (tuple.third == L_TYPE) {

			(*inducedL_Writer) << TupleI(tuple.forth, tuple.fifth, tuple.sixth); //(preCh, preType, repCount)
	
		}

		if (tuple.third == S_TYPE) {

			(*inducedS_Writer) << TupleI(tuple.forth, tuple.fifth, tuple.sixth); //(preCh, preType, repCount)
		}
	}

	(*sortedLMS_Writer).finish(); delete sortedLMS_Writer; sortedLMS_Writer = NULL;

	(*inducedL_Writer).finish(); delete inducedL_Writer; inducedL_Writer = NULL;

	(*inducedS_Writer).finish(); delete inducedS_Writer; inducedS_Writer = NULL;

	delete ascSorterF1; ascSorterF1 = NULL;
}


//! induce LCP of L-type suffixes

template<typename T1, typename T2, uint8 B, uint16 H, uint16 Alphabet>
void KLCPABuilder<T1, T2, B, H, Alphabet>::induceL() {

	InducingInfo inducingInfo;

	MinHeapPool * pqLMinPool = new MinHeapPool(MAX_MEM / 4 / MinHeapBlock::raw_size, MAX_MEM / 4 / MinHeapBlock::raw_size);

	MinHeap * pqLMin = new MinHeap(*pqLMinPool);

	typename tupleH_vector_type::bufreader_type * sortedLMS_reader = new typename tupleH_vector_type::bufreader_type(*mSortedLMS); //(curCh, preCh, repCount)

	typename t2_vector_type::bufreader_type * sortedLMS_LCP_reader = new typename t2_vector_type::bufreader_type(*mGlobalLCP);

	typename tupleI_vector_type::bufreader_type * inducedL_reader = new typename tupleI_vector_type::bufreader_type(*mInducedL); //(preCh, preType, repCount)

	mSortedL = new tupleJ_vector_type(); mSortedL->resize(mInducedL->size());

	typename tupleJ_vector_type::bufwriter_type * sortedL_writer = new typename tupleJ_vector_type::bufwriter_type(*mSortedL);

	uint8 inducedCh, preCh, curCh, curBkt;

	T2 curLCP, inducedLCP, preRepCount;

	T1 rank;

	//process the virtual sentinel to induce the L-type suffix starting at the rightmost character
	RetrieveCh *retrieveLMS = new RetrieveCh(mLMSDistribution, true);

	RetrieveCh *retrieveL = new RetrieveCh(mLTypeDistribution, true);

	curLCP = 0; //the sentinel is a size-one suffix different from any other suffixes

	rank = 1; //rank of the induced suffix

	const TupleI & tupleI = *(*inducedL_reader);

	inducedCh = tupleI.first;  //starting character of the induced suffix

	pqLMin->push(TupleG(inducedCh, rank, curLCP, tupleI.forth)); // <ch, rank, lcp, repCount>

	++(*inducedL_reader);

	//induce the remaining suffixes
	curBkt = pqLMin->top().first; //pqLMin must not be empty
	
	if (!sortedLMS_reader->empty() && (*sortedLMS_reader)->first) {

		curBkt = *(*sortedLMS_reader)->first;
	}

	while (true) {

		//induce L-type in current L-type bucket
		while (*retrieveL == curBkt) {
		
			++rank;

			++retrieveL;

			const TupleG & tupleG = pqLMin->top();

			//compute LCP
			if (!retrieveL.isFirst()) {

				curLCP = (tupleG.third + 1 > K) ? K : (tupleG.third + 1); //compute LCP
			}
			else {
			
				curLCP = 0;
			}

			//update inducingInfo
			inducingInfo.updateLCP_L(curBkt, curLCP); 

			//induce preceding L-type
			const TupleI & tupleI = *(*inducedL_reader);

			if (tupleI.second == L_TYPE) {

				pqLMin->push(TupleG(tupleI.first, rank, inducingInfo.getLCP(tupleI.first), tupleI.forth)); // <ch, rank, lcp, repCount>	
			}

			//push into sortedL
			(*sortedL_writer) << TupleJ(tupleG.first, tupleI.first, curLCP, tupleI.third);

			//reset inducingInfo
			inducingInfo.setCh(tupleI.first, curBkt);

			inducingInfo.setLCP(tupleI.first, std::numeric_limits<uint16>::max());
			
			//
			preCh = tupleH.first;

			preRepCount = tupleH.third;

			//move forward one element
			pqLMin->pop();

			++(*inducedL_reader);
		}

		//induce L-type in current S-type bucket
		while (*retrieveLMS == curBkt) {

			++rank; 

			++retrieveLMS;

			//compute LCP
			const TupleH & tupleH = *(*sortedLMS_reader);

			if (retrieveS.isFirst() && preLTypeCh = curBkt) {
			
				curLCP = std::min(tupleH.third, preRepCount);
			}
			else {

				curLCP = 0;
			}

			//update inducingInfo
			inducingInfo.updateLCP_L(curBkt, curLCP); 

			//induce preceding L-type
			const TupleI & tupleI = *(*inducedL_reader);

			if (tupleI.second == L_TYPE) {

				pqLMin->push(TupleG(tupleI.first, rank, inducingInfo.getLCP(tupleI.first), tupleI.forth)); // <ch, rank, lcp, repCount>	
			}

			//reset inducingInfo
			inducingInfo.setCh(tupleI.first, curBkt);

			inducingInfo.setLCP(tupleI.first, std::numeric_limits<uint16>::max());

			//
			preCh = tupleH.first;

			preRepCount = tupleH.third;

			//move forward one element
			++(*inducedL_reader);

			++(*sortedLMS_reader);

			++(*sortedLMS_LCP_reader);
		}

		//determine next bucket
		if (!pqLMin->empty()) {
		
			curBkt = pqLMin->top().first;

			if (!sortedLMS_reader->empty() && (*sortedLMS_reader)->first < curBkt) {
			
				curBkt = (*sortedLMS_reader)->first;
			}
		}
		else if (!sortedLMS_reader->empty() && (*sortedLMS_reader)->first < curBkt){
			
			curBkt = (*sortedLMS_reader)->first;
		}
		else {
		
			break;
		}
	}
	
	(*sortedL_writer).finish();

	delete pqLMin; pqLMin = NULL; 

	delete pqLMinPool; pqLMinPool = NULL;

	delete sortedLMS_reader; sortedLMS_reader = NULL; delete mSortedLMS; mSortedLMS = NULL;

	delete sortedLMS_LCP_reader; sortedLMS_LCP_reader = NULL; delete mGlobalLCP; mGlobalLCP = NULL;

	delete inducedL_reader; inducedL_reader = NULL;

	delete sortedL_writer; sortedL_writer = NULL;

	delete retrieveLMS; retrieveLMS = NULL;

	delete retrieveL; retrieveL = NULL;

	return;
}

//! induce LCP for S-type suffixes
template<typename T1, typename T2, uint8 B, uint16 H, uint16 Alphabet>
void KLCPABuilder<T1, T2, B, H, Alphabet>::induceS() {

	InducingInfo inducingInfo;

	MaxHeapPool *pqSMaxPool = new MaxHeapPool(MAX_MEM / 4 / MaxHeapBlock::raw_size, MAX_MEM / 4 / MaxHeapBlock::raw_size);

	MaxHeap * pqSMax = new MaxHeap(*pqSMaxPool);

	typename tupleJ_vector_type::bufreader_reverse_type * sortedL_reverseReader = new typename tupleJ_vector_type::bufreader_reverse_type(*mSortedL);

	typename tupleI_vector_type::bufreader_reverse_type * inducedL_reverseReader = new typename tupleI_vector_type::bufreader_reverse_type(*mInducedL); //(preCh, preType, repCount)

	typename tupleI_vector_type::bufreader_reverse_type * inducedS_reverseReader = new typename tupleI_vector_type::bufreader_reverse_type(*mInducedS); //(preCh, preType, repCount)

	typename t2_vector_type::bufwriter_type *lcpWriter = new typename t2_vector_type::bufwriter_type(*mLCP);

	uint8 inducedCh, preCh, curCh, curBkt;

	T2 curLCP, inducedLCP, preRepCount;

	T1 rank;


	//sortedL_reverseReader must not be empty and pqSMax must be empty
	RetrieveCh * retrieveL = RetrieveCh(mLTypeDistribution, true);

	RetrieveCh * retrieveS = RetrieveCh(mSTypeDistribution, true);

	rank = mSLen + 1;

	curBkt = *retrieveL;

	while (true) {

		//induce S-type in current S-type bucket 
		while (*retrieveS == curBkt) {

			--rank;

			--retrieveS;

			const TupleG & tupleG = pqSMax->top();

			//compute LCP
			if (curBkt == *retrieveS) {

				curLCP = (tupleG.forth + 1 == K) ? K : (tupleG.forth + 1);
			}
			else {
				
				if (*retrieveL == curBkt) {

					curLCP = curLCP
				}
				else {

					const TupleJ & tupleJ = *(*sortedL_reverseReader);
					
					curLCP = std::min(tupleG.forth, tupleJ.forth);
				}
			}
	
			//push into mLCP
			(*lcpWriter) << curLCP;

			//update inducingInfo
			inducingInfo.updateLCP_S(curBkt, curLCP);

			//induce preceding S-type
			const TupleI & tupleI = *(*inducedS_reverseReader);

			if (tupleI.second == S_TYPE) {

				pqSMax->push(TupleG(tupleI.first, rank, inducingInfo.getLCP(tupleI.first), tupleI.forth)); // <ch, rank, lcp, repCount>	
			}

			//reset inducingInfo
			inducingInfo.setCh(tupleI.first, curBkt);

			inducingInfo.setLCP(tupleI.first, std::numeric_limits<uint16>::max());

			//
			preCh = tupleG.first;

			preRepCount = tupleG.forth;

			//move forward one element
			pqSMax->pop();

			++(*inducedS_reverseReader);
		}

		//induce S-type in current L-type bucket
		while (*retrieveL == curBkt) {

			--rank;

			--retrieveL;

			const TupleG & tupleG = *(*sortedL_reverseReader);

			//compute LCP
			if (preCh == curBkt) {
			
				curLCP = (tupleG.forth + 1) > K ? K : (tupleG.forth + 1);
			}
			else {
			
				curLCP = 0;
			}

			//push into mLCP
			(*lcpWriter) << curLCP;

			//update inducingInfo
			inducingInfo.updateLCP_S(curBkt,curLCP); 
							
			//induce preceding S-type
			const TupleI & tupleI = *(*inducedL_reverseReader);

			if (tupleI.second == S_TYPE) {

				pqSMax->push(TupleG(tupleI.first, rank, inducingInfo.getLCP(tupleI.first), tupleI.forth)); // <ch, rank, lcp, repCount>	
			}

			preCh = tupleI.first;

			preRepCount = tupleI.forth;

			//move forward one element
			++(*sortedL_reverseReader);

			++(*inducedL_reverseReader);
		}

		if (!pqSMax->empty()) {

			curBkt = pqSMax->top().first;

			if (!sortedLReverseReader->empty() && (*sortedLReverseReader)->first > curBkt) {

				curBkt = (*sortedLReverseReader)->first;
			}
		}
		else if (!sortedLReverseReader->empty()) {

			curBkt = (*sortedLReverseReader)->first;
		}
		else {

			break;
		}
	}

	(*lcpWriter).finish();

	delete pqSMaxPool; pqSMaxPool = NULL;

	delete pqSMax; pqSMax = NULL;

	delete sortedL_reverseReader; sortedL_reverseReader = NULL; delete mSortedL; sortedL = NULL;

	delete inducedL_reverseReader; inducedL_reverseReader = NULL; delete mInducedL; mInducedL = NULL;

	delete inducedS_reverseReader; inducedS_reverseReader = NULL; delete mInducedS; mInducedS = NULL;

	delete retrieveL; retrieveL = NULL;

	delete retrieveS; retrieveS = NULL;

	return;
}


#endif

NAMESPACE_KLCP_EM_END

#endif //KLCPA_EM_H
