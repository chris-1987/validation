#ifndef LCPA_EM2_H
#define LCPA_EM2_H

#include "../../common/namespace.h"
#include "../../common/common.h"
#include "../../common/data.h"

#define STATISTICS_COLLECTION

//#define TEST_PROGRAM 

NAMESPACE_LCP_EM2_BEG



//! LCP checker using Karp-Rabin and Induced-Sorting Principle

//! Support O(1) alphabet (Alphabet <= 256)
template<typename T, size_t Alphabet>
class LCPAChecker{

private:

	//alias
	typedef Pair<T, T> TupleA;

	typedef TupleGreatComparator1<TupleA> TupleAGreatComparator1; //!< greater comparator, sort by 1st component in descending order

	typedef typename ExTupleAscSorter<TupleA, TupleAGreatComparator1>::sorter AscSorterA1;

	typedef typename ExTupleAscSorter<TupleA, TupleAGreatComparator1>::comparator AscComparatorA1;

	typedef TupleLessComparator1<TupleA> TupleALessComparator1; //!< lesser comparator, sort by 1st component in ascending order

	typedef typename ExTupleAscSorter<TupleA, TupleALessComparator1>::sorter AscSorterA2;
	
	typedef typename ExTupleAscSorter<TupleA, TupleALessComparator1>::comparator AscComparatorA2;

	typedef TupleLessComparator2<TupleA> TupleALessComparator2; //!< lesser comparator, sort by (1st, 2nd) components in ascending order

	typedef typename ExTupleAscSorter<TupleA, TupleALessComparator2>::sorter AscSorterA3;
	
	typedef typename ExTupleAscSorter<TupleA, TupleALessComparator2>::comparator AscComparatorA3;

	typedef Pair<T, FPTYPE_A> TupleB;

	typedef TupleLessComparator1<TupleB> TupleBLessComparator1; //!< lesser comparator, sort by 1st component in ascending order

	typedef typename ExTupleAscSorter<TupleB, TupleBLessComparator1>::sorter AscSorterB1;

	typedef typename ExTupleAscSorter<TupleB, TupleBLessComparator1>::comparator AscComparatorB1;

	typedef Triple<T, FPTYPE_A, uint16> TupleC;

	typedef TupleLessComparator1<TupleC> TupleCLessComparator1; //!< lesser comparator, sort by 1st component in ascending order

	typedef typename ExTupleAscSorter<TupleC, TupleCLessComparator1>::sorter AscSorterC1;

	typedef typename ExTupleAscSorter<TupleC, TupleCLessComparator1>::comparator AscComparatorC1;

	typedef Triple<T, uint8, T> TupleD;

	typedef TupleLessComparator1<TupleD> TupleDLessComparator1; //!< lesser comparator, sort by 1st component in ascending order

	typedef typename ExTupleAscSorter<TupleD, TupleDLessComparator1>::sorter AscSorterD1; 
	
	typedef typename ExTupleAscSorter<TupleD, TupleDLessComparator1>::comparator AscComparatorD1;

	typedef Quadruple<T, uint8, uint8, T> TupleE;

	typedef TupleLessComparator1<TupleE> TupleELessComparator1; //!< lesser comparator, sort by 1st component in ascending order

	typedef typename ExTupleAscSorter<TupleE, TupleELessComparator1>::sorter AscSorterE1;
	
	typedef typename ExTupleAscSorter<TupleE, TupleELessComparator1>::comparator AscComparatorE1;

	typedef Pair<uint8, T> TupleF;

	typedef Triple<uint8, uint8, T> TupleG; 

	typedef Triple<uint8, T, T> TupleH; 

	typedef TupleGreatComparator2<TupleH> TupleHGreatComparator2; //!< min-heap comparator, sort in ascending order

	typedef typename ExTupleMinHeap<TupleH, TupleHGreatComparator2, MAX_MEM / 2, MAX_ITEM>::heap MinHeap;

	typedef typename MinHeap::block_type MinHeapBlock;

	typedef typename stxxl::read_write_pool<MinHeapBlock> MinHeapPool;

	typedef TupleLessComparator2<TupleH> TupleHLessComparator2;  //!< max-heap comparator, sort in descending order

	typedef typename ExTupleMaxHeap<TupleH, TupleHLessComparator2, MAX_MEM / 2, MAX_ITEM>::heap MaxHeap;

	typedef typename MaxHeap::block_type MaxHeapBlock;

	typedef typename stxxl::read_write_pool<MaxHeapBlock> MaxHeapPool;

	typedef Quadruple<uint8, uint8, T, T> TupleI; 
	
	typedef typename ExVector<T>::vector t_vector_type;

	typedef typename ExVector<uint8>::vector uint8_vector_type;

	typedef typename ExVector<TupleF>::vector f_vector_type;

	typedef typename ExVector<TupleG>::vector g_vector_type;


	//data member
	std::string & mSFileName; //!< input string file name

	std::string & mSAFileName; //!< SA file name

	std::string & mLCPFileName; //!< LCP file name

	stxxl::syscall_file * mSFile; //!< ptr to handler for input string file

	stxxl::syscall_file * mSAFile; //!< ptr to hanlder for SA file

	stxxl::syscall_file * mLCPFile; //!< ptr to handler for LCP file

	size_t mSLen; //!< length of input string

	uint8_vector_type * mS; //!< ptr to handler for input string

	t_vector_type * mSA; //!< ptr to handler for SA

	t_vector_type * mLCP; //!< ptr to handler for LCP

	FPTYPE_A * mR; //!< ptr to range interval array
  
	size_t mRSize; //!< size of mR

	t_vector_type * mSALMS; //!< ptr to SA_{LMS}

	t_vector_type * mLCPLMS; //!< ptr to LCP_{LMS}

	size_t mLMSDistribution[Alphabet] = {0}; //!< distribution of head characters for LMS_TYPE suffixes 

	size_t mLDistribution[Alphabet] = {0}; //!< distribution of head characters for L_TYPE suffixes

	size_t mSDistribution[Alphabet] = {0}; //!< distribution of head characters for S_TYPE suffixes

	size_t mLMSNum = 0; //!< number of LMS_TYPE suffixes

	size_t mLNum = 0; //!< number of L_TYPE suffixes

	size_t mSNum = 0; //!< number of S_TYPE suffixes

	f_vector_type * mSortedLMS; //!< ptr to sorted LMS suffixes

	g_vector_type * mInducedL; //!< ptr to information array for inducing L_TYPE suffixes

	g_vector_type * mInducedS; //!< ptr to information array for inducing S_TYPE suffixes

	t_vector_type * mSortedLCP_L; //!< ptr to LCP for sorted L_TYPE suffixes

#ifdef TEST_PROGRAM

	t_vector_type * mSortedReverseLCP; //!< ptr to LCP for sorted suffixes

#endif

	uint8 mRightmostLCh; //!< rightmost L_TYPE character
	
public:

	//member function
	LCPAChecker(std::string & _sFileName, std::string & _saFilename, std::string & _lcpFileName, size_t _sLen); //!< ctor

	bool runLMSChecker(); //!< entrance to check LCP_{LMS}

	void retrieveLMS(); //!< retrieve LCP_{LMS} and SA_{LMS} from LCP and SA

	void computeR(); //!< compute mR

	FPTYPE_A getR(size_t _interval); //!< get range interval 

	AscSorterB1 * run1st(AscSorterA2 * _ascSorterA2); //!< compute fp for fp[sa[i]]

	AscSorterC1 * run2nd(AscSorterA3 * _ascSorterA3); //!< compute for for fp[sa[i] + lcp[i]] and fp[sa[i] + lcp[i + 1]]

	bool checkLMS(AscSorterB1 * _ascSorterB1, AscSorterC1 * _ascSorterC11, AscSorterC1 * _ascSorterC12); //!< compare fp to check correctness
	
	bool runInduceChecker(); //!< entrance to check LCP_{L} and LCP_{S}

	void retrieveLMS2(); //!< retrieve preceding information

	bool checkL();  //!< check LCP_{L}

	bool checkS(); //!< check LCP_{S}

	//! inner class, retrieve next character to be scanned

	struct RetrieveCh{
	private:
	
		const size_t * distribution; 

		std::vector<uint8> distributionCh; //!< compact distribution, get rid of empty bucket

		std::vector<size_t> distributionNum; //!< compact distribution, get rid of empty bucket

		uint16 bktNum = 0; //!< number of non-empty buckets, range over [1, 256]

		uint16 curBktIdx; //!< bucket being scanned, range over [0, 255]

		uint16 lastScannedCh = std::numeric_limits<uint16>::max(); //!< record last scanned character
		
		size_t toScanBkt; //!< number of elements remained to be scanned in current bucket

		size_t toScan; //!< number of elements remained to be scanned in total

		size_t totalNum = 0; //!< number of elements in total 

	public:

		//! ctor
		RetrieveCh(const size_t * _distribution, const bool _rightward) : distribution(_distribution){

			for (uint16 i = 0; i < Alphabet; ++i) {

				if (distribution[i] != 0) {

					distributionCh.push_back(i); //character

					distributionNum.push_back(distribution[i]); //quantity

					++bktNum;

					totalNum += distribution[i];	
				}
			}

			curBktIdx = (_rightward) ? 0 : (bktNum - 1);

			toScanBkt = distributionNum[curBktIdx];

			toScan = totalNum;
		}

		//! retrieve current bucket's character
		uint8 operator*() {

			return distributionCh[curBktIdx];
		}
		
		//! move rightward one slot, do not return ref
		void operator++() {

			lastScannedCh = distributionCh[curBktIdx];

			--toScanBkt;

			--toScan;

			if (toScanBkt == 0) { //current bucket is empty, scan next bucket if any

				++curBktIdx;

				if (toScan != 0) { //no remaining buckets

					toScanBkt = distributionNum[curBktIdx];
				}
			}	
		}

		//! move leftward one slot, do not return ref
		void operator--() {

			lastScannedCh = distributionCh[curBktIdx];
		
			--toScanBkt;

			--toScan;
	
			if (toScanBkt == 0) { //current bucket is empty, scan next bucket if any

				--curBktIdx;
				
				if (toScan != 0) { //no remaining buckets

					toScanBkt = distributionNum[curBktIdx];
				}
			}
		}


		//! check if empty
		bool empty() {
			
			return (toScan == 0);
		}

		//! return last scanned character
		uint16 getLastScannedCh() {

			return lastScannedCh;
		}

		//! return true if currently scanning the leftmost S_TYPE suffix in the bucket
		bool leftmostInBkt_S(){

			return (1 == toScanBkt);
		}

		//! print compact distribution
		void printDistribution() {

			std::cerr << "origin distribution:\n";

			for (size_t i = 0 ; i < Alphabet; ++i) {

				std::cerr << "ch: " << static_cast<uint32>(i) << " num: " << distribution[i] << "\n"; 
			}
			
			std::cerr << std::endl;

		
			std::cerr << "compact distribution:\n";

			for (size_t i = 0 ; i < bktNum; ++i) {

				std::cerr << "ch: " << static_cast<uint32>(distributionCh[i]) << " num: " << distributionNum[i] << "\n"; 
			}
			
			std::cerr << std::endl;
		}
	};

	
	//! inner class, dynamically compute RMQ using O(Alphabet) space and O(Alphabet * n) time
	struct RMQ{
	private:

		T minLCP[Alphabet]; //!< induce[ch] records the RMQ

	public:

		//! ctor
		RMQ(){
			
			for (uint16 i = 0; i < Alphabet; ++i) {
			
				minLCP[i] = std::numeric_limits<T>::max();
			}
		};
	
		//! set range minimum value
		void setRMQ(const uint8 _ch, T _lcp) {
			
			minLCP[_ch] = _lcp;
		}

		//! conduct range minimum query
		T getRMQ(const uint8 _ch) const {

			return minLCP[_ch];
		}
	
		//! update range minimum value when inducing L_TYPE
		void updateRMQ_L(const uint8 _ch, const T _lcp) {

			for (uint16 i = _ch; i < Alphabet; ++i) {

				if (minLCP[i] > _lcp) minLCP[i] = _lcp;
			}
		}	

		//! update range minimum value when inducing S_TYPE
		void updateRMQ_S(const uint8 _ch, const T _lcp) {
			
			for (uint16 i = 0; i <= _ch; ++i) {

				if (minLCP[i] > _lcp) minLCP[i] = _lcp;
			}
		}
	};	

	//! inner class for unit test
	class Test{
	public:
	
		static void testRetrieveLMS(const std::string & _sFileName, const std::string & _saFileName, size_t _len, t_vector_type * _saLMS, t_vector_type * _lcpLMS);

		static void testRetrieveLMS2(const std::string & _sFilename, const std::string & _saFileName, const size_t _len, const size_t _lmsNum, const size_t _lNum, const size_t _sNum, f_vector_type * _sortedLMS, g_vector_type * _inducedL, g_vector_type * _inducedS, size_t * _lmsDistribution, size_t * _lDistribution, size_t * _sDistribution);

		static void testCheckL(const std::string & _sFileName, const std::string & _lcpFileName, const size_t _len, t_vector_type * _sortedLCP_L);

		static void testCheckS(const std::string & _sFileName, const std::string & _saFileName, const std::string & _lcpFileName, const size_t _len, t_vector_type * _sortedReverseLCP);

		static void printMsg(const std::string & _msg);
	};
};


//! ctor for LCPAChecker

// \param T template parameter, element type for sa and lcp arrays.
// \param Alphabet template parameter, alphabet size.
// \param _sFileName filename for input string
// \param _saFileName filename for suffix array
// \param _lcpFilename filename for lcp array
// \param _sLen length of input string

template<typename T, size_t Alphabet>
LCPAChecker<T, Alphabet>::LCPAChecker(
	std::string & _sFileName, 
	std::string & _saFileName, 
	std::string & _lcpFileName, 
	size_t _sLen) :
	mSFileName(_sFileName),
	mSAFileName(_saFileName),
	mLCPFileName(_lcpFileName),
	mSLen(_sLen) {


	std::cerr << "\n*************************************\n";
	std::cerr << "sFileName: " << mSFileName << std::endl;
	std::cerr << "saFileName: " << mSAFileName << std::endl;
	std::cerr << "lcpFileName: " << mLCPFileName << std::endl;
	std::cerr << "sLen: " << mSLen << std::endl;
	std::cerr << "\n*************************************\n";

#ifdef STATISTICS_COLLECTION

	//collects I/O statistics
	stxxl::stats *Stats = stxxl::stats::get_instance();

	stxxl::block_manager * bm = stxxl::block_manager::get_instance();

#endif

	///step 1: pre-process
	mSFile = new stxxl::syscall_file(mSFileName, 
		stxxl::syscall_file::RDWR | stxxl::syscall_file::DIRECT);

	mSAFile = new stxxl::syscall_file(mSAFileName,
		stxxl::syscall_file::RDWR | stxxl::syscall_file::DIRECT);

	mLCPFile = new stxxl::syscall_file(mLCPFileName,
		stxxl::syscall_file::RDWR | stxxl::syscall_file::DIRECT);

	computeR();


	///step 2: check correctness of LCP_{LMS}
	bool lmsCheckRes = runLMSChecker();


	///step 3: check correctness of LCP_{L} and LCP_{S}
	bool induceCheckRes = runInduceChecker();


	///step 4: final check
	if (lmsCheckRes && induceCheckRes) {

		std::cerr << "check passed!" << std::endl;
	}
	else {

		std::cerr << "check failed!" << std::endl;
	}


	//clear up
	delete mSFile; mSFile = NULL;

	delete mSAFile; mSAFile = NULL;

	delete mLCPFile; mLCPFile = NULL;

	delete mR; mR = NULL;

#ifdef STATISTICS_COLLECTION

	std::cerr << "peak disk usage per ch: " << bm->get_maximum_allocation() / mSLen << std::endl;

	std::cerr << "I/O volume per ch: " << (Stats->get_written_volume() + Stats->get_read_volume()) / mSLen << std::endl;	

#endif

}


//! compute R^1, R^2, R^4, ...

template<typename T, size_t Alphabet>
void LCPAChecker<T, Alphabet>::computeR(){

	mRSize = 1;

	size_t tmpLen = mSLen;

	while (tmpLen) {

		++mRSize;

		tmpLen /= 2;
	}

	mR = new FPTYPE_A[mRSize];

	mR[0] = R % P;

	for (size_t i = 1; i < mRSize; ++i) {

		mR[i] = static_cast<FPTYPE_A>((static_cast<FPTYPE_B>(mR[i - 1]) * mR[i - 1]) % P);
	}

	return;
}


//! get range interval with a specific _interval

template<typename T, size_t Alphabet>
FPTYPE_A LCPAChecker<T, Alphabet>::getR(size_t _interval) {

	FPTYPE_A res = 1;

	for (size_t i = 0; i < mRSize; ++i) {
		
		if (_interval % 2) {

			res = static_cast<FPTYPE_A>((static_cast<FPTYPE_B>(res) * mR[i]) % P);
		}


		_interval = _interval / 2;

		if (_interval == 0) break;
	}

	return res;
}


//! entrance for checking LCP_{LMS}

template<typename T, size_t Alphabet>
bool LCPAChecker<T, Alphabet>::runLMSChecker(){
		
	///step 1: retrieve LCP_{LMS} and SA_{LMS}
	retrieveLMS();

#ifdef TEST_PROGRAM	

//	Test::testRetrieveLMS(mSFileName, mSAFileName, mSLen, mSALMS, mLCPLMS);

#endif


	///step 2: compute fp for <sa, idx>
	AscSorterA2 * ascSorterA2 = new AscSorterA2(AscComparatorA2(), MAX_MEM / 4);

	typename t_vector_type::bufreader_type * saLMSReader = new typename t_vector_type::bufreader_type(*mSALMS);

	for (T idx = 1; !saLMSReader->empty(); ++(*saLMSReader), ++idx) {
		
		ascSorterA2->push(TupleA(*(*saLMSReader), idx));
	}	

	delete saLMSReader; saLMSReader = NULL;

	ascSorterA2->sort();

	AscSorterB1 * ascSorterB1 = run1st(ascSorterA2);

	Test::printMsg(std::string("ascSorterB1->size:").append(std::to_string(ascSorterB1->size())).append("\n"));

	
	///step 3: compute fp for <sa + lcp1, idx>
	saLMSReader = new typename t_vector_type::bufreader_type(*mSALMS);

	typename t_vector_type::bufreader_type * lcpLMSReader = new typename t_vector_type::bufreader_type(*mLCPLMS);
	
	AscSorterA3 * ascSorterA31 = new AscSorterA3(AscComparatorA3(), MAX_MEM / 4);

	for (T idx = 1; !saLMSReader->empty(); ++idx, ++(*lcpLMSReader), ++(*saLMSReader)) {

		ascSorterA31->push(TupleA(*(*saLMSReader) + *(*lcpLMSReader), idx));	
	}

	delete saLMSReader; saLMSReader = NULL;

	delete lcpLMSReader; lcpLMSReader = NULL;

	ascSorterA31->sort();

	AscSorterC1 * ascSorterC11 = run2nd(ascSorterA31);		

	Test::printMsg(std::string("ascSorterC11->size:").append(std::to_string(ascSorterC11->size())).append("\n"));


	///step 4: compute fp for <sa + lcp2, idx>
	saLMSReader = new typename t_vector_type::bufreader_type(*mSALMS);

	lcpLMSReader = new typename t_vector_type::bufreader_type(*mLCPLMS);

	AscSorterA3 * ascSorterA32 = new AscSorterA3(AscComparatorA3(), MAX_MEM / 4);

	++(*lcpLMSReader);

	for (T idx = 1; !saLMSReader->empty(); ++idx, ++(*lcpLMSReader), ++(*saLMSReader)) {
		
		ascSorterA32->push(TupleA(*(*saLMSReader) + *(*lcpLMSReader), idx));
	}
							
	delete saLMSReader; saLMSReader = NULL;

	delete mSALMS; mSALMS = NULL;
	
	delete lcpLMSReader; lcpLMSReader = NULL;

	ascSorterA32->sort();

	AscSorterC1 * ascSorterC12 = run2nd(ascSorterA32);

	Test::printMsg(std::string("ascSorterC12->size:").append(std::to_string(ascSorterC12->size())).append("\n"));


	///step 5: check by comparing fp
	bool isRight = checkLMS(ascSorterB1, ascSorterC11, ascSorterC12);

	Test::printMsg(std::string("checkLMS result: ").append(isRight ? "passed" : "failed").append("\n"));


	return isRight;
}





template<typename T, size_t Alphabet>
void LCPAChecker<T, Alphabet>::retrieveLMS(){

	///step 1: pack (SA[i], i) and sort TupleA by 1st component in descending order
	mSA = new t_vector_type(mSAFile);

	typename t_vector_type::bufreader_type *saReader = new typename t_vector_type::bufreader_type(*mSA);

	AscSorterA1 * ascSorterA1 = new AscSorterA1(AscComparatorA1(), MAX_MEM / 2);	

	for (T idx = 1; !saReader->empty(); ++idx, ++(*saReader)) {

		ascSorterA1->push(TupleA(*(*saReader), idx));
	}

	delete saReader; saReader = NULL;

	delete mSA; mSA = NULL;

	ascSorterA1->sort();

	std::cerr << "ascSorterA1->size(): " << ascSorterA1->size() << std::endl;


	///step 2: pack (ISA_LMS[i], SA_LMS[i]) and sort TupleA by 1st component in ascending order
	mS = new uint8_vector_type(mSFile);

	typename uint8_vector_type::bufreader_reverse_type *sReverseReader = new typename uint8_vector_type::bufreader_reverse_type(*mS);

	AscSorterA2 * ascSorterA2 = new AscSorterA2(AscComparatorA2(), MAX_MEM / 2);

	uint8 curCh, lastCh;

	uint8 curType, lastType;

	lastCh = *(*sReverseReader); //rightmost is L-type

	lastType = L_TYPE;

	++(*sReverseReader);

	for (; !sReverseReader->empty(); ++(*sReverseReader), ++(*ascSorterA1)) {

		curCh = *(*sReverseReader);

		curType = (curCh < lastCh || (curCh == lastCh && lastType == S_TYPE)) ? S_TYPE : L_TYPE;

		if (curType == L_TYPE && lastType == S_TYPE) { //check if lastCh is LMS
			
			const TupleA & tuple = *(*ascSorterA1);

			ascSorterA2->push(TupleA(tuple.second, tuple.first));
		}

		lastCh = curCh;

		lastType = curType;
	}		

	++(*ascSorterA1); //leftmost must not be LMS

	delete sReverseReader; sReverseReader = NULL;
	
	delete mS; mS = NULL;

	delete ascSorterA1; ascSorterA1 = NULL;

	ascSorterA2->sort();

	std::cerr << "ascSorterA2->size(): " << ascSorterA2->size() << std::endl;


	///step 3: record LCP_{LMS} and SA_{LMS}
	mLCP = new t_vector_type(mLCPFile);

	typename t_vector_type::bufreader_type * lcpReader = new typename t_vector_type::bufreader_type(*mLCP);
	
	mLCPLMS = new t_vector_type(); mLCPLMS->resize(ascSorterA2->size() + 1);

	typename t_vector_type::bufwriter_type * lcpLMSWriter = new typename t_vector_type::bufwriter_type(*mLCPLMS);

	mSALMS = new t_vector_type(); mSALMS->resize(ascSorterA2->size());
	
	typename t_vector_type::bufwriter_type * saLMSWriter = new typename t_vector_type::bufwriter_type(*mSALMS);

	T minLCP = std::numeric_limits<T>::max(), curLCP;

	for (T idx = 1; !lcpReader->empty(); ++(*lcpReader), ++idx) {

		minLCP = std::min(*(*lcpReader), minLCP);

		if (!ascSorterA2->empty() && idx == (*ascSorterA2)->first) {
			
			(*lcpLMSWriter) << minLCP;

			minLCP = std::numeric_limits<T>::max();

			(*saLMSWriter) << (*ascSorterA2)->second;

			++(*ascSorterA2);
		}
	}

	(*lcpLMSWriter) << static_cast<T>(0); //append a sentinel

	(*lcpLMSWriter).finish();
	
	(*saLMSWriter).finish();

	std::cerr << "ascSorterA2->size(): " << ascSorterA2->size() << std::endl;

	delete lcpLMSWriter; lcpLMSWriter = NULL;

	delete saLMSWriter; saLMSWriter = NULL;

	delete ascSorterA2; ascSorterA2 = NULL;

	delete lcpReader; lcpReader = NULL;

	delete mLCP; mLCP = NULL;
}


template<typename T, size_t Alphabet>
typename LCPAChecker<T, Alphabet>::AscSorterB1 * LCPAChecker<T, Alphabet>::run1st(AscSorterA2 * _ascSorter) {
		
	std::cerr << "_ascSorter->size(): " << _ascSorter->size() << std::endl;
		
	mS = new uint8_vector_type(mSFile);

	typename uint8_vector_type::bufreader_type *sReader = new typename uint8_vector_type::bufreader_type(*mS);

	AscSorterB1 *ascSorterB1 = new AscSorterB1(AscComparatorB1(), MAX_MEM / 4);

	FPTYPE_A fp = 0;

	for (T pos = 0; !sReader->empty(); ++(*sReader), ++pos) {
	
		if (!_ascSorter->empty() && pos == (*_ascSorter)->first) { //each position appears at most once

			ascSorterB1->push(TupleB((*_ascSorter)->second, fp));		

			++(*_ascSorter);
		}

		fp = static_cast<FPTYPE_A>((static_cast<FPTYPE_B>(fp) * R + (*(*sReader) + 1)) % P);		
	}
	
	delete sReader; sReader = NULL; 
		
	delete mS; mS = NULL;

	std::cerr << "_ascSorter->size():" << _ascSorter->size() << std::endl;

	delete _ascSorter; _ascSorter = NULL;

	ascSorterB1->sort();

	return ascSorterB1;
}


template<typename T, size_t Alphabet>
typename LCPAChecker<T, Alphabet>::AscSorterC1 * LCPAChecker<T, Alphabet>::run2nd(AscSorterA3 * _ascSorter) {
		
	std::cerr << "_ascSorter->size(): " << _ascSorter->size() << std::endl;
		
	mS = new uint8_vector_type(mSFile);

	typename uint8_vector_type::bufreader_type *sReader = new typename uint8_vector_type::bufreader_type(*mS);

	AscSorterC1 *ascSorterC1 = new AscSorterC1(AscComparatorC1(), MAX_MEM / 4);

	FPTYPE_A fp = 0;

	for (T pos = 0; !sReader->empty(); ++(*sReader), ++pos) {

		while (!_ascSorter->empty() && (*_ascSorter)->first == pos) {

			ascSorterC1->push(TupleC((*_ascSorter)->second, fp, static_cast<uint16>(*(*sReader))));

			++(*_ascSorter);
		}

		fp = static_cast<FPTYPE_A>((static_cast<FPTYPE_B>(fp) * R + (*(*sReader) + 1)) % P);	
	}

	//(*_ascSorter)->first == mSLen
	while (!_ascSorter->empty()) {

		assert(mSLen == (*_ascSorter)->first);
	
		ascSorterC1->push(TupleC((*_ascSorter)->second, fp, std::numeric_limits<uint16>::max()));

		++(*_ascSorter);
	}


	delete sReader; sReader = NULL; 

	delete mS; mS = NULL;

	std::cerr << "_ascSorter->size():" << _ascSorter->size() << std::endl;

	delete _ascSorter; _ascSorter = NULL;

	ascSorterC1->sort();

	return ascSorterC1;
}


template<typename T, size_t Alphabet>
bool LCPAChecker<T, Alphabet>::checkLMS(AscSorterB1 * _ascSorterB1, AscSorterC1 * _ascSorterC11, AscSorterC1 * _ascSorterC12) {
	
	bool isRight = true;

	typename t_vector_type::bufreader_type *lcpLMSReader = new typename t_vector_type::bufreader_type(*mLCPLMS);

	FPTYPE_A fpInterval1, fpInterval2;

	uint16 ch1, ch2;

	T curLCP;
	
	//
	++(*_ascSorterC11); //skip leftmost 

	++(*lcpLMSReader); //skip leftmost, must be 0

	curLCP = *(*lcpLMSReader);

	fpInterval2 = static_cast<FPTYPE_A>(((*_ascSorterC12)->second - (static_cast<FPTYPE_B>((*_ascSorterB1)->second) * getR(curLCP)) % P + P) % P);

	ch2 = (*_ascSorterC12)->third;

	++(*_ascSorterB1), ++(*_ascSorterC12);	

	for (size_t idx = 1 ; !_ascSorterC11->empty(); ++idx, ++(*_ascSorterC11), ++(*_ascSorterB1), ++(*_ascSorterC12)) {
		
		fpInterval1 = static_cast<FPTYPE_A>(((*_ascSorterC11)->second - (static_cast<FPTYPE_B>((*_ascSorterB1)->second) * getR(curLCP)) % P + P) % P);
	
		ch1 = (*_ascSorterC11)->third;

		if (fpInterval1 == fpInterval2 && ch1 != ch2) {
		
			//is right	
		}
		else {

			std::cerr << "idx: " << idx << " fpInterval1: " << fpInterval1 << " fpInterval2: " << fpInterval2 << std::endl;

			isRight = false;

			break;
		}

		++(*lcpLMSReader);
	
		curLCP = *(*lcpLMSReader);

		fpInterval2 = static_cast<FPTYPE_A>(((*_ascSorterC12)->second - (static_cast<FPTYPE_B>((*_ascSorterB1)->second) * getR(curLCP)) % P + P) % P);

		ch2 = (*_ascSorterC12)->third;
	}

	
	//clear up
	delete lcpLMSReader; lcpLMSReader = NULL;

	delete _ascSorterB1; _ascSorterB1 = NULL;

	delete _ascSorterC11; _ascSorterC11 = NULL;

	delete _ascSorterC12; _ascSorterC12 = NULL;

	return isRight;
}




template<typename T, size_t Alphabet>
bool LCPAChecker<T, Alphabet>::runInduceChecker() {

	bool isRight = true;

	///step 1: collect information for L-type/S-type induction
	retrieveLMS2();

#ifdef TEST_PROGRAM

	Test::testRetrieveLMS2(mSFileName, mSAFileName, mSLen, mLMSNum, mLNum, mSNum, mSortedLMS, mInducedL, mInducedS, mLMSDistribution, mLDistribution, mSDistribution);

#endif


	///step 2: check L_TYPE
	if (!checkL()) isRight = false;


	///step 3: check S_TYPE
	if (!checkS()) isRight = false;


	return isRight;
}

template<typename T, size_t Alphabet>
void LCPAChecker<T, Alphabet>::retrieveLMS2() {

	///step 1: pack (SA[i], i) and sort TupleA by 1st component in descending order 
	mSA = new t_vector_type(mSAFile);

	typename t_vector_type::bufreader_type * saReader = new typename t_vector_type::bufreader_type(*mSA);

	AscSorterA1 *ascSorterA1 = new AscSorterA1(AscComparatorA1(), MAX_MEM / 4);

	for (T idx = 1; !saReader->empty(); ++(*saReader), ++idx) {

		ascSorterA1->push(TupleA(*(*saReader), idx));
	}	

	delete saReader; saReader = NULL;

	delete mSA; mSA = NULL;

	ascSorterA1->sort();	

	Test::printMsg(std::string("ascSorterA1->size(): ").append(std::to_string(ascSorterA1->size())).append("\n"));


	///step 2: retrieve and pack information for inducing LMS, L_TYPE and S_TYPE
	mS = new uint8_vector_type(mSFile);

	typename uint8_vector_type::bufreader_reverse_type * sReverseReader = new uint8_vector_type::bufreader_reverse_type(*mS);

	AscSorterD1 * ascSorterLMS = new AscSorterD1(AscComparatorD1(), MAX_MEM / 4); //<idx, preCh, reCnt>

	AscSorterE1 * ascSorterL = new AscSorterE1(AscComparatorE1(), MAX_MEM / 4); //<idx, preCh, preT, repCnt>

	AscSorterE1 * ascSorterS = new AscSorterE1(AscComparatorE1(), MAX_MEM / 4); //<idx, preCh, preT, repCnt>

	uint8 curCh, lastCh;

	uint8 curType, lastType;

	T repCnt;

	//process the rightmost L-type suffix
	mRightmostLCh = curCh = *(*sReverseReader), curType = L_TYPE; 

	++mLNum;

	++mLDistribution[curCh];

	repCnt = 1;

	++(*sReverseReader);

	lastCh = curCh, lastType = curType;

	for (; !sReverseReader->empty(); ++(*sReverseReader), ++(*ascSorterA1)) {

		//determine curCh and curType
		curCh = *(*sReverseReader);
	
		if (curCh < lastCh || (lastCh == curCh && lastType == S_TYPE)) {
		
			curType = S_TYPE;

			++mSNum;

			++mSDistribution[curCh];		
		} 
		else {

			curType = L_TYPE;

			++mLNum;

			++mLDistribution[curCh];

			if (lastType == S_TYPE) { //find an LMS

				lastType = LMS_TYPE;

				++mLMSNum;

				++mLMSDistribution[lastCh];
			}
		}	

		
		//retrieve information for suffix starting at lastCh
		if (lastType == L_TYPE) {

			ascSorterL->push(TupleE((*ascSorterA1)->second, curCh, curType, repCnt));
		}
		else if (lastType == S_TYPE) {

			ascSorterS->push(TupleE((*ascSorterA1)->second, curCh, curType, repCnt));
		}		
		else{ //lastType == LMS_TYPE, also an S_TYPE

			ascSorterS->push(TupleE((*ascSorterA1)->second, curCh, curType, repCnt));

			ascSorterLMS->push(TupleD((*ascSorterA1)->second, curCh, repCnt));
		}
	
		//update repetition count for current scanned suffix	
		repCnt = (curCh == lastCh) ? (repCnt + 1) : 1;
	
		lastCh = curCh;

		lastType = curType;
	}

	//retrieve information for suffix starting at leftmost character (must not be LMS_TYPE)
	curCh = 0; //assume the preceding character is sentinel (represented by curCh)

	curType = SENTINEL_TYPE;

	if (lastType == L_TYPE) {

		ascSorterL->push(TupleE((*ascSorterA1)->second, curCh, curType, repCnt));
	}
	else { //lastType == S_TYPE (must not be equal to LMS_TYPE)

		ascSorterS->push(TupleE((*ascSorterA1)->second, curCh, curType, repCnt));
	}		

	delete sReverseReader; sReverseReader = NULL;

	delete mS; mS = NULL;
	
	Test::printMsg(std::string("ascSorterA1->size(): ").append(std::to_string(ascSorterA1->size())).append("\n"));

	delete ascSorterA1; ascSorterA1 = NULL;
	
	Test::printMsg(std::string("mLMSNum: ").append(std::to_string(mLMSNum)).append("\n"));

	Test::printMsg(std::string("mLNum: ").append(std::to_string(mLNum)).append("\n"));

	Test::printMsg(std::string("mSNum: ").append(std::to_string(mSNum)).append("\n"));
	

	///step 3: redirect information for inducing LMS_TYPE
	ascSorterLMS->sort();

	Test::printMsg(std::string("ascSorterLMS->size(): ").append(std::to_string(ascSorterLMS->size())).append("\n"));

	mSortedLMS = new f_vector_type(); mSortedLMS->resize(mLMSNum);

	typename f_vector_type::bufwriter_type * sortedLMSWriter = new typename f_vector_type::bufwriter_type(*mSortedLMS);

	for (; !ascSorterLMS->empty(); ++(*ascSorterLMS)) {

		const TupleD & tuple_d = *(*ascSorterLMS);

		(*sortedLMSWriter) << TupleF(tuple_d.second, tuple_d.third); //<preCh, repCnt>
	}
	
	(*sortedLMSWriter).finish();

	delete sortedLMSWriter; sortedLMSWriter = NULL;

	delete ascSorterLMS; ascSorterLMS = NULL;


	///step 4: redirect information for inducing L_TYPE
	ascSorterL->sort();

	Test::printMsg(std::string("ascSorterL->size(): ").append(std::to_string(ascSorterL->size())).append("\n"));

	mInducedL = new g_vector_type(); mInducedL->resize(mLNum);	

	typename g_vector_type::bufwriter_type * inducedLWriter = new typename g_vector_type::bufwriter_type(*mInducedL);

	for (; !ascSorterL->empty(); ++(*ascSorterL)) {

		const TupleE & tuple_e = *(*ascSorterL);

		(*inducedLWriter) << TupleG(tuple_e.second, tuple_e.third, tuple_e.forth); //<preCh, preT, repCnt>
	}

	(*inducedLWriter).finish();

	delete inducedLWriter; inducedLWriter = NULL;

	delete ascSorterL; ascSorterL = NULL;


	///step 5: redirect information for inducing S_TYPE
	ascSorterS->sort();

	Test::printMsg(std::string("ascSorterS->size(): ").append(std::to_string(ascSorterS->size())).append("\n"));

	mInducedS = new g_vector_type(); mInducedS->resize(mSNum);

	typename g_vector_type::bufwriter_type * inducedSWriter = new typename g_vector_type::bufwriter_type(*mInducedS);
	
	for (; !ascSorterS->empty(); ++(*ascSorterS)) {
		
		const TupleE & tuple_e = *(*ascSorterS);

		(*inducedSWriter) << TupleG(tuple_e.second, tuple_e.third, tuple_e.forth); //<preCh, preT, repCnt>
	}

	(*inducedSWriter).finish();

	delete inducedSWriter; inducedSWriter = NULL;

	delete ascSorterS; ascSorterS = NULL;		
}


//induce LCP_{L} from LCP_{LMS}
template<typename T, size_t Alphabet>
bool LCPAChecker<T, Alphabet>::checkL() {

	bool isRight = true;

	///step 1: compute fpInduced and fpScanned
	FPTYPE_A fpInduced[Alphabet] = {0};

	FPTYPE_A fpScanned[Alphabet] = {0};

	RMQ rmqL;

	RetrieveCh retrieveCh_LMS(mLMSDistribution, true);

	RetrieveCh retrieveCh_L(mLDistribution, true);

	MinHeapPool * pqLMinPool = new MinHeapPool(MAX_MEM / 4 / MinHeapBlock::raw_size, MAX_MEM / 4 / MinHeapBlock::raw_size);

	MinHeap * pqLMin = new MinHeap(*pqLMinPool);

	typename f_vector_type::bufreader_type * sortedLMSReader = new typename f_vector_type::bufreader_type(*mSortedLMS); 

	typename t_vector_type::bufreader_type * lcpLMSReader = new typename t_vector_type::bufreader_type(*mLCPLMS);

	typename g_vector_type::bufreader_type * inducedL_Reader = new typename g_vector_type::bufreader_type(*mInducedL);

	mSortedLCP_L = new t_vector_type(); mSortedLCP_L->resize(mLNum);

	typename t_vector_type::bufwriter_type * sortedLCP_L_Writer = new typename t_vector_type::bufwriter_type(*mSortedLCP_L); 

	uint8 curBktCh;

	T curLCP, lastRepCnt_L, rank;
	
	//induce the rightmost suffix (L-type) from the virtual sentinel	
	rank = 1; //the rank of the sentinel is set to 1

	curLCP = 0; //the sentinel and the rightmost L-type suffix share no common characters

	pqLMin->push(TupleH(mRightmostLCh, rank, curLCP)); //mRightmostLCh is pre-computed
	
	fpInduced[mRightmostLCh] = static_cast<FPTYPE_A>((static_cast<FPTYPE_B>(fpInduced[mRightmostLCh]) * R + curLCP + 1) % P); //update fpInduced

	//notice: no need to perform ++retrieveCh_LMS, as the sentinel is not included

	++rank;

	//induce the remaining
	curBktCh = *retrieveCh_L; // there must exist at least one L-type

	if (!retrieveCh_LMS.empty() && *retrieveCh_LMS < curBktCh) {

		curBktCh = *retrieveCh_LMS;
	}
	
	while (true) {

		//scan first element of L-type {curBktCh}-bucket
		while (!retrieveCh_L.empty() && *retrieveCh_L == curBktCh) {
			
			const TupleH & tuple_h = pqLMin->top();

			//update fpScanned
			fpScanned[curBktCh] = static_cast<FPTYPE_A>((static_cast<FPTYPE_B>(fpScanned[curBktCh]) * R + tuple_h.third + 1) % P); 

			//determine lcp
			curLCP = (retrieveCh_L.getLastScannedCh() == curBktCh) ? (tuple_h.third + 1) : 0;

			//update RMQ
			rmqL.updateRMQ_L(curBktCh, curLCP);

			//induce L-type
			const TupleG & tuple_g = *(*inducedL_Reader);

			if (tuple_g.second == L_TYPE) {
	
				//update fpInduced
				fpInduced[tuple_g.first] = static_cast<FPTYPE_A>((static_cast<FPTYPE_B>(fpInduced[tuple_g.first]) * R + rmqL.getRMQ(tuple_g.first) + 1) % P);

				pqLMin->push(TupleH(tuple_g.first, rank, rmqL.getRMQ(tuple_g.first)));

				//reset RMQ
				rmqL.setRMQ(tuple_g.first, std::numeric_limits<T>::max());	
			}

			//output sorted L-type
			(*sortedLCP_L_Writer) << curLCP; 	
			
			//record repCnt
			lastRepCnt_L = tuple_g.third;

			//forward ptr
			pqLMin->pop();

			++(*inducedL_Reader);			

			++rank;

			++retrieveCh_L;
		}


		//scan first element of LMS-type {curBktCh}-bucket
		while (!retrieveCh_LMS.empty() && *retrieveCh_LMS == curBktCh) {	
			
			const TupleF & tuple_f = *(*sortedLMSReader);			

			//determine lcp
			if (retrieveCh_LMS.getLastScannedCh() == curBktCh) { //not leftmost LMS in current bucket, directly use LCP_{LMS}

				curLCP = *(*lcpLMSReader);
			} 
			else { //leftmost LMS in current

				if (retrieveCh_L.getLastScannedCh() == curBktCh) { //share the same head character with the last scanned L_TYPE

					curLCP = std::min(lastRepCnt_L, tuple_f.second);
				}
				else {

					curLCP = 0;
				}
			}			

			//update RMQ
			rmqL.updateRMQ_L(curBktCh, curLCP);

			//update fpInduced
			fpInduced[tuple_f.first] = static_cast<FPTYPE_A>((static_cast<FPTYPE_B>(fpInduced[tuple_f.first]) * R + rmqL.getRMQ(tuple_f.first) + 1) % P); 

			//induce L-type
			pqLMin->push(TupleH(tuple_f.first, rank, rmqL.getRMQ(tuple_f.first)));


			//reset RMQ
			rmqL.setRMQ(tuple_f.first, std::numeric_limits<T>::max());	

			//forward ptr
			++(*sortedLMSReader);			

			++(*lcpLMSReader);

			++rank;

			++retrieveCh_LMS;
		}

	
		//determine next bucket
		if (!retrieveCh_L.empty()) {

			curBktCh = *retrieveCh_L;

			if (!retrieveCh_LMS.empty() && *retrieveCh_LMS < curBktCh) {

				curBktCh = *retrieveCh_LMS;
			}
		}	  
		else if (!retrieveCh_LMS.empty()) {
			
			curBktCh = *retrieveCh_LMS;
		}
		else {
			
			break;
		}
	}

	(*sortedLCP_L_Writer).finish();

	//clear up
	delete pqLMinPool; pqLMinPool = NULL;

	delete pqLMin; pqLMin = NULL;
	
	Test::printMsg(std::string("sortedLMSReader->size(): ").append(std::to_string(sortedLMSReader->size()).append("\n")));

	delete sortedLMSReader; sortedLMSReader = NULL; 

	delete mSortedLMS; mSortedLMS = NULL;

	Test::printMsg(std::string("inducedL_Reader->size(): ").append(std::to_string(inducedL_Reader->size()).append("\n")));

	delete inducedL_Reader; inducedL_Reader = NULL;

	delete sortedLCP_L_Writer; sortedLCP_L_Writer = NULL;

	Test::printMsg(std::string("lcpLMSReader->size(): ").append(std::to_string(lcpLMSReader->size()).append("\n")));

	delete lcpLMSReader; lcpLMSReader = NULL;

	delete mLCPLMS; mLCPLMS = NULL;


	///step 2: check equality between fpScanned and fpInduced 
	for (size_t i = 0; i < Alphabet; ++i) {

		if (fpInduced[i] != fpScanned[i]) {

			Test::printMsg(std::string("fpInduced != fpScanned wrong\n"));

			Test::printMsg(std::string("fpInduced[").append(std::to_string(i)).append("]:").append(std::to_string(fpInduced[i])).append(" "));

			Test::printMsg(std::string("fpScanned[").append(std::to_string(i)).append("]:").append(std::to_string(fpScanned[i])).append("\n"));

			
			isRight = false;

			break;
		}
		else {


			//Test::printMsg(std::string("fpInduced[").append(std::to_string(i)).append("]:").append(std::to_string(fpInduced[i])).append(" "));

			//Test::printMsg(std::string("fpScanned[").append(std::to_string(i)).append("]:").append(std::to_string(fpScanned[i])).append("\n"));

		}
	}

#ifdef TEST_PROGRAM
	
	Test::testCheckL(mSFileName, mLCPFileName, mSLen, mSortedLCP_L);

#endif

	Test::printMsg(std::string("checkL result: ").append(isRight ? "passed" : "failed").append("\n"));

	return isRight;
}

template<typename T, size_t Alphabet>
bool LCPAChecker<T, Alphabet>::checkS() {

	bool isRight = true;

	///step 1: compute fpInduced and fpScanned
	FPTYPE_A fpInduced[Alphabet] = {0};

	FPTYPE_A fpScanned[Alphabet] = {0};

	RMQ rmqS;

	RetrieveCh retrieveCh_L(mLDistribution, false);

	RetrieveCh retrieveCh_S(mSDistribution, false);

	MaxHeapPool * pqSMaxPool = new MaxHeapPool(MAX_MEM / 4 / MaxHeapBlock::raw_size, MAX_MEM / 4 /MaxHeapBlock::raw_size);

	MaxHeap * pqSMax = new MaxHeap(*pqSMaxPool);

	typename t_vector_type::bufreader_reverse_type * sortedLCP_L_ReverseReader = new typename t_vector_type::bufreader_reverse_type(*mSortedLCP_L); 

	typename g_vector_type::bufreader_reverse_type * inducedL_ReverseReader = new typename g_vector_type::bufreader_reverse_type(*mInducedL);

	typename g_vector_type::bufreader_reverse_type * inducedS_ReverseReader = new typename g_vector_type::bufreader_reverse_type(*mInducedS);

#ifdef TEST_PROGRAM

	mSortedReverseLCP = new t_vector_type(); mSortedReverseLCP->resize(mSLen);

	typename t_vector_type::bufwriter_type *sortedReverseLCPWriter = new typename t_vector_type::bufwriter_type(*mSortedReverseLCP);

#endif

	uint8 curBktCh;

	T lastLCP, curLCP, rank;

	TupleG last_tuple_g;

	TupleH last_tuple_h;

	//the biggest must be L_TYPE
	rank = static_cast<uint64>(mSLen);

	curBktCh = *retrieveCh_L;

	while (true) {
		
		///scan S-type {curBktCh}-bucket	
		//retrieve rightmost S-type in the bucket
		if (!pqSMax->empty() && pqSMax->top().first == curBktCh) {

			const TupleH & cur_tuple_h = pqSMax->top();

			const TupleG & cur_tuple_g = *(*inducedS_ReverseReader);

			//update fpScanned for current scanned element
			fpScanned[curBktCh] = static_cast<FPTYPE_A>((static_cast<FPTYPE_B>(fpScanned[curBktCh]) * R + cur_tuple_h.third + 1) % P);	

			//update last 
			last_tuple_h = pqSMax->top();
 
			last_tuple_g = *(*inducedS_ReverseReader);

			//foward
			pqSMax->pop();
	
			++(*inducedS_ReverseReader);

			--retrieveCh_S;
		}

		//process last element when scanning current element in the same bucket
		while (!pqSMax->empty() && pqSMax->top().first == curBktCh) { 

			const TupleH & cur_tuple_h = pqSMax->top(); 

			const TupleG & cur_tuple_g = *(*inducedS_ReverseReader);

			//update fpScanned for current scanned element
			fpScanned[curBktCh] = static_cast<FPTYPE_A>((static_cast<FPTYPE_B>(fpScanned[curBktCh]) * R + cur_tuple_h.third + 1) % P);

			//determine lcp for the last scanned element in the same bucket
			lastLCP = cur_tuple_h.third + 1; //share a common character, thus plus 1
			
			//induce S_TYPE for last scanned element
			if (last_tuple_g.second == S_TYPE) {

				//update fpInduced for last scanned element
				fpInduced[last_tuple_g.first] = static_cast<FPTYPE_A>((static_cast<FPTYPE_B>(fpInduced[last_tuple_g.first]) * R + rmqS.getRMQ(last_tuple_g.first) + 1) % P);

				pqSMax->push(TupleH(last_tuple_g.first, rank, rmqS.getRMQ(last_tuple_g.first)));

				//reset RMQ
				rmqS.setRMQ(last_tuple_g.first, lastLCP);		
			}
	
			//update RMQ
			rmqS.updateRMQ_S(curBktCh, lastLCP);

#ifdef TEST_PROGRAM
			//output sorted LCP
			(*sortedReverseLCPWriter) << lastLCP;

#endif

			//update last element
			last_tuple_h = cur_tuple_h;

			last_tuple_g = cur_tuple_g;

			//forward ptr
			pqSMax->pop();

			++(*inducedS_ReverseReader);

			--retrieveCh_S;

			--rank;
		}

		//current element is directly induced from last element, process the last element
		while (!retrieveCh_S.empty() && *retrieveCh_S == curBktCh) {
			
			//induce current element from last
			TupleH cur_tuple_h = TupleH(curBktCh, rank + 1, rmqS.getRMQ(curBktCh));

			const TupleG & cur_tuple_g = *(*inducedS_ReverseReader);

			assert(cur_tuple_g.second == S_TYPE && cur_tuple_g.first == curBktCh);

			//update fpScanned for current scanned element
			fpScanned[curBktCh] = static_cast<FPTYPE_A>((static_cast<FPTYPE_B>(fpScanned[curBktCh]) * R + cur_tuple_h.third + 1) % P);

			//determine lcp for last element
			lastLCP = std::min(cur_tuple_g.third, last_tuple_g.third);
	
			//update fpInduced for last scanned element
			fpInduced[curBktCh] = static_cast<FPTYPE_A>((static_cast<FPTYPE_B>(fpInduced[curBktCh]) * R + rmqS.getRMQ(curBktCh) + 1) % P);
			//set RMQ
			rmqS.setRMQ(curBktCh, lastLCP);
			
			//update RMQ
			rmqS.updateRMQ_S(curBktCh, lastLCP);		


#ifdef TEST_PROGRAM

			//output sorted LCP
			(*sortedReverseLCPWriter) << lastLCP;

#endif

			//update last element
			last_tuple_g = cur_tuple_g;

			last_tuple_h = cur_tuple_h;
		
			//forward ptr
			++(*inducedS_ReverseReader);

			--retrieveCh_S;

			--rank;
		}		
	
		//process the leftmost element in current bucket
		if (retrieveCh_S.getLastScannedCh() == curBktCh) {

			//determine lcp
			if (*retrieveCh_L == curBktCh) {
				
				const TupleG & l_tuple_g = *(*inducedL_ReverseReader);

				lastLCP = std::min(last_tuple_g.third, l_tuple_g.third); 
			}
			else {

				lastLCP = 0;
			}

			//induce S_type
			if (last_tuple_g.second == S_TYPE) {

				//update fpInduced
				fpInduced[last_tuple_g.first] = static_cast<FPTYPE_A>((static_cast<FPTYPE_B>(fpInduced[last_tuple_g.first]) * R + rmqS.getRMQ(last_tuple_g.first) + 1) % P);

				pqSMax->push(TupleH(last_tuple_g.first, rank, rmqS.getRMQ(last_tuple_g.first)));

				//set RMQ
				rmqS.setRMQ(last_tuple_g.first, lastLCP);
			}
			
			//update RMQ
			rmqS.updateRMQ_S(curBktCh, lastLCP);

#ifdef TEST_PROGRAM	

			//output sorted LCP
			(*sortedReverseLCPWriter) << lastLCP;

#endif

			--rank;
		}	
		

		///scan L-type {curBktCh}-bkt
		while (!retrieveCh_L.empty() && *retrieveCh_L == curBktCh) {
	
			//determine lcp
			curLCP = *(*sortedLCP_L_ReverseReader);

			//induce S-type
			const TupleG & tuple_g = *(*inducedL_ReverseReader);

			if (tuple_g.second == S_TYPE) {
			
				//update fpInduced
				fpInduced[tuple_g.first] = static_cast<FPTYPE_A>((static_cast<FPTYPE_B>(fpInduced[tuple_g.first]) * R + rmqS.getRMQ(tuple_g.first) + 1) % P);

				pqSMax->push(TupleH(tuple_g.first, rank, rmqS.getRMQ(tuple_g.first)));

				//set RMQ
				rmqS.setRMQ(tuple_g.first, curLCP);
			}

			//update RMQ
			rmqS.updateRMQ_S(curBktCh, curLCP);

#ifdef TEST_PROGRAM

			//output sorted LCP
			(*sortedReverseLCPWriter) << curLCP;

#endif

			//forward ptr
			++(*inducedL_ReverseReader);

			++(*sortedLCP_L_ReverseReader);	

			--rank;

			--retrieveCh_L;
		}		
		
		//determine next bucket
		if (!retrieveCh_S.empty()) {
			
			curBktCh = *retrieveCh_S;

			if (!retrieveCh_L.empty() && *retrieveCh_L > curBktCh) {

				curBktCh = *retrieveCh_L;
			}
		}
		else if (!retrieveCh_L.empty()){

			curBktCh = *retrieveCh_L;
		}
		else {

			break;
		}
	}

#ifdef TEST_PROGRAM

	(*sortedReverseLCPWriter).finish();

#endif


	//clear up
	delete pqSMaxPool; pqSMaxPool = NULL;

	delete pqSMax; pqSMax = NULL;

	Test::printMsg(std::string("sortedLCP_L_ReverseReader->size(): ").append(std::to_string(sortedLCP_L_ReverseReader->size())).append("\n"));
	delete sortedLCP_L_ReverseReader; sortedLCP_L_ReverseReader = NULL;

	delete mSortedLCP_L; mSortedLCP_L = NULL;

	Test::printMsg(std::string("inducedL_ReverseReader->size(): ").append(std::to_string(inducedL_ReverseReader->size())).append("\n"));

	delete inducedL_ReverseReader; inducedL_ReverseReader = NULL;

	delete mInducedL; mInducedL = NULL;

	Test::printMsg(std::string("inducedS_ReverseReader->size(): ").append(std::to_string(inducedS_ReverseReader->size())).append("\n"));

	delete inducedS_ReverseReader; inducedS_ReverseReader = NULL;

	delete mInducedS; mInducedS = NULL;

#ifdef TEST_PROGRAM
	
	delete sortedReverseLCPWriter; sortedReverseLCPWriter = NULL;

	Test::printMsg(std::string("mSortedReverseLCP->size(): ").append(std::to_string(mSortedReverseLCP->size())).append("\n"));

	Test::testCheckS(mSFileName, mSAFileName, mLCPFileName, mSLen, mSortedReverseLCP);

	delete mSortedReverseLCP; mSortedReverseLCP = NULL;

#endif


	///step 2: check equality between fpScanned and fpInduced 
	for (size_t i = 0; i < Alphabet; ++i) {

		if (fpInduced[i] != fpScanned[i]) {

			Test::printMsg(std::string("fpInduced != fpScanned wrong\n"));

			Test::printMsg(std::string("fpInduced[").append(std::to_string(i)).append("]:").append(std::to_string(fpInduced[i])).append(" "));

			Test::printMsg(std::string("fpScanned[").append(std::to_string(i)).append("]:").append(std::to_string(fpScanned[i])).append("\n"));

			
			isRight = false;

			break;
		}
	}
	

	Test::printMsg(std::string("checkS result: ").append(isRight ? "passed" : "failed").append("\n"));

	return isRight;
}


template<typename T, size_t Alphabet>
void LCPAChecker<T, Alphabet>::Test::testRetrieveLMS(const std::string & _sFileName, const std::string & _saFileName, size_t _len, t_vector_type * _saLMS, t_vector_type * _lcpLMS){

	///step 1: load file
	//load s
	std::ifstream sFin(_sFileName, std::ios::in | std::ios::binary);

	uint8 * sBuffer = new uint8[_len];

	sFin.read(reinterpret_cast<char*>(sBuffer), _len * sizeof(uint8) / sizeof(char));

	//load sa	
	std::ifstream saFin(_saFileName, std::ios::in | std::ios::binary);

	T * saBuffer = new T[_len];

	saFin.read(reinterpret_cast<char*>(saBuffer), _len * sizeof(T) / sizeof(char));
	

	///step 2: determine type
	uint8 * tBuffer = new uint8[_len];

	size_t lmsNum = 0;

	//rightmost is L-type
	tBuffer[_len - 1] = L_TYPE;

	for (int i = _len - 2; i >= 0; --i) { //corpora size < 2G

		tBuffer[i] = (sBuffer[i] < sBuffer[i + 1] || (sBuffer[i] == sBuffer[i + 1] && tBuffer[i + 1] == S_TYPE)) ? S_TYPE : L_TYPE;

		if (tBuffer[i] == L_TYPE && tBuffer[i + 1] == S_TYPE) {
			
			tBuffer[i + 1] = LMS_TYPE;

			++lmsNum;
		}
	}


	///step 3: check SA_{LMS}
	std::cerr << "lmsNum in RAM: " << lmsNum << " lmsNum in EM: " << _saLMS->size() << std::endl;

	T *saLMSBuffer = new T[lmsNum];

	for (size_t i = 0, j = 0; i < _len; ++i) {

		if (tBuffer[saBuffer[i]] == LMS_TYPE) {

			saLMSBuffer[j] = saBuffer[i];

			++j;
		}
	}

	delete [] tBuffer; tBuffer = NULL;	

	delete [] saBuffer; saBuffer = NULL;

	typename t_vector_type::bufreader_type * saLMSReader = new typename t_vector_type::bufreader_type(*_saLMS);

	for (size_t i = 0; !saLMSReader->empty(); ++i, ++(*saLMSReader)) {

		if (*(*saLMSReader) != saLMSBuffer[i]) {

			std::cerr << "sa is wrong\n";
			
			std::cerr << "i: " << i << " saLMSReader: " << *(*saLMSReader) << " saLMSBuffer: " << saLMSBuffer[i] << std::endl;
			
			exit(0);
		}
	}	

	delete saLMSReader; saLMSReader = NULL;

	std::cerr << "saLMS is right\n";


	///step 4: check LCP_{LMS}	
	typename t_vector_type::bufreader_type * lcpLMSReader = new typename t_vector_type::bufreader_type(*_lcpLMS);

	++(*lcpLMSReader); //leftmost must be 0

	for (size_t i = 1; i < lmsNum; ++i, ++(*lcpLMSReader)) {

		T pos1 = saLMSBuffer[i];

		T pos2 = saLMSBuffer[i - 1];

		T lcp = *(*lcpLMSReader);

		for (T j = 0; j < lcp; ++j) {

			if (sBuffer[pos1 + j] != sBuffer[pos2 + j]) {								

				std::cerr << "pos1: " << pos1 << " pos2: " << pos2 << " lcp: " << lcp << std::endl;

				std::cerr << "str1: ";
			
				for (size_t k = 0; k < lcp; ++k) {
			
					std::cerr << static_cast<int>(sBuffer[pos1 + k]) << " ";
				}

				std::cerr << std::endl;

				std::cerr << "str2: ";
		
				for (size_t k = 0; k <= lcp; ++k) {
	
					std::cerr << static_cast<int>(sBuffer[pos2 + k]) << " ";
				}

				std::cerr << std::endl;

				std::cerr << "lcp is wrong\n";

				exit(0);
			}
		}

		if (pos1 + lcp == _len || pos2 + lcp == _len) {

			//is right
		}
		else if (sBuffer[pos1 + lcp] != sBuffer[pos2 + lcp]) {
				
			//is right
		}
		else {
		
			std::cerr << "pos1: " << pos1 << " pos2: " << pos2 << " lcp: " << lcp << std::endl;

			std::cerr << "str1: ";
			
			for (size_t j = 0; j <= lcp; ++j) {
			
				std::cerr << sBuffer[pos1 + j] << " ";
			}

			std::cerr << std::endl;

			std::cerr << "str2: ";
		
			for (size_t j = 0; j <= lcp; ++j) {

				std::cerr << sBuffer[pos2 + j] << " ";
			}

			std::cerr << std::endl;
			
			std::cerr << "lcp is wrong\n";

			exit(0);
		}
	}
	
	delete lcpLMSReader; lcpLMSReader = NULL;

	delete [] saLMSBuffer; saLMSBuffer = NULL;

	delete [] sBuffer; sBuffer = NULL;

	std::cerr << "testRetrieveLMS1 passed\n";
}


template<typename T, size_t Alphabet>
void LCPAChecker<T, Alphabet>::Test::testRetrieveLMS2(const std::string & _sFileName, const std::string & _saFileName, const size_t _len, const size_t _lmsNum, const size_t _lNum, const size_t _sNum, f_vector_type * _sortedLMS, g_vector_type * _inducedL, g_vector_type * _inducedS, size_t * _lmsDistribution, size_t * _lDistribution, size_t * _sDistribution) {

	///step 1: load file 
	//load s
	std::ifstream sFin(_sFileName, std::ios::in | std::ios::binary);

	uint8 * sBuffer = new uint8[_len];

	sFin.read(reinterpret_cast<char*>(sBuffer), _len * sizeof(uint8) / sizeof(char));
	
	//load sa
	std::ifstream saFin(_saFileName, std::ios::in | std::ios::binary);

	T * saBuffer = new T[_len];

	saFin.read(reinterpret_cast<char*>(saBuffer), _len * sizeof(T) / sizeof(char));

	
	///step 2: determine type and check type num
	uint8 * tBuffer = new uint8[_len];

	size_t lmsNum = 0, lNum = 0, sNum = 0;

	size_t lmsDistribution[Alphabet] = {0};

	size_t lDistribution[Alphabet] = {0};

	size_t sDistribution[Alphabet] = {0};

	//rightmost is L-type
	tBuffer[_len - 1] = L_TYPE;
	
	++lDistribution[sBuffer[_len - 1]];

	++lNum;

	for (int i = _len - 2; i >= 0; --i) { //corpora size < 2G

		if (sBuffer[i] < sBuffer[i + 1] || (sBuffer[i] == sBuffer[i + 1] && tBuffer[i + 1] == S_TYPE)) {

			tBuffer[i] = S_TYPE;

			++sDistribution[sBuffer[i]];
	
			++sNum;
		}
		else {

			tBuffer[i] = L_TYPE;

			++lDistribution[sBuffer[i]];

			++lNum;
		}
		
		if (tBuffer[i] == L_TYPE && tBuffer[i + 1] == S_TYPE) {
			
			tBuffer[i + 1] = LMS_TYPE;

			++lmsDistribution[sBuffer[i + 1]];

			++lmsNum;
		}
	}
	
	if (lmsNum != _lmsNum || lNum != _lNum || sNum != _sNum) {

		Test::printMsg(std::string("type num wrong\n"));

		Test::printMsg(std::string("lmsNum: ").append(std::to_string(lmsNum)).append("\n"));

		Test::printMsg(std::string("_lmsNum: ").append(std::to_string(_lmsNum)).append("\n"));

		Test::printMsg(std::string("lNum: ").append(std::to_string(lNum)).append("\n"));

		Test::printMsg(std::string("_lNum: ").append(std::to_string(_lNum)).append("\n"));

		Test::printMsg(std::string("sNum: ").append(std::to_string(sNum)).append("\n"));

		Test::printMsg(std::string("_sNum: ").append(std::to_string(_sNum)).append("\n"));

		exit(0);
	}

	for (size_t i = 0; i < Alphabet; ++i) {

		if (lmsDistribution[i] != _lmsDistribution[i]) {
			
			Test::printMsg(std::string("lmsDistribution wrong\n"));
		}

		if (lDistribution[i] != _lDistribution[i]) {
			
			Test::printMsg(std::string("lDistribution wrong\n"));
		}
		if (sDistribution[i] != _sDistribution[i]) {
			
			Test::printMsg(std::string("sDistribution wrong\n"));
		}
	}
	
		
	///step 3: check sortedLMS and inducedL / inducedS
	typename f_vector_type::bufreader_type * sortedLMSReader = new typename f_vector_type::bufreader_type(*_sortedLMS);

	typename g_vector_type::bufreader_type * inducedLReader = new typename g_vector_type::bufreader_type(*_inducedL);

	typename g_vector_type::bufreader_type * inducedSReader = new typename g_vector_type::bufreader_type(*_inducedS);

	T curPos;

	uint8 curType; // curType in {L_TYPE, S_TYPE, LMS_TYPE}
	
	uint8 preType; // preType in {L_TYPE, S_TYPE, SENTINEL_TYPE}

	uint8 preCh;

	T repCnt;

	for (size_t i = 0; i < _len; ++i) {

		curPos = saBuffer[i];

		curType = tBuffer[curPos];

		preType = (0 == curPos) ? SENTINEL_TYPE : ((tBuffer[curPos - 1] == L_TYPE) ? L_TYPE : S_TYPE);

		preCh = (0 == curPos) ? 0 : sBuffer[curPos - 1];

		repCnt = 1;

		for (size_t j = curPos + 1; j < _len; ++j) {

			if (sBuffer[j] == sBuffer[j - 1]) {

				++repCnt;
			}
			else {
		
				 break;
			}
		}

		if (curType == LMS_TYPE) {

			const TupleF & tuple_f = *(*sortedLMSReader);
	
			if (preCh != tuple_f.first) {
	
				Test::printMsg(std::string("LMS_TYPE, preCh wrong\n"));

				exit(0);
			}

			if (repCnt != tuple_f.second) {

				Test::printMsg(std::string("LMS_TYPE, repCnt wrong\n"));

				exit(0);
			}

			++(*sortedLMSReader);
		} 

		if (curType == L_TYPE) {

			const TupleG & tuple_g = *(*inducedLReader);
			
			if (preCh != tuple_g.first) {
				
				Test::printMsg(std::string("L_TYPE, preCh wrong\n"));

				exit(0);
			}

			if (preType != tuple_g.second) {

				Test::printMsg(std::string("L_TYPE, preType wrong\n"));

				exit(0);
			}

			if (repCnt != tuple_g.third) {

				Test::printMsg(std::string("L_TYPE, repCnt wrong\n"));

				exit(0);
			}

			++(*inducedLReader);
		}

		if (curType != L_TYPE) { // S_TYPE or LMS_TYPE
			
			const TupleG & tuple_g = *(*inducedSReader);
	
			if (preCh != tuple_g.first) {
	
				Test::printMsg(std::string("S_TYPE, preCh wrong\n"));

				exit(0);
			}

			if (preType != tuple_g.second) {

				Test::printMsg(std::string("S_TYPE, preType wrong\n"));

				exit(0);
			}

			if (repCnt != tuple_g.third) {

				Test::printMsg(std::string("S_TYPE, repCnt wrong\n"));

				exit(0);
			}

			++(*inducedSReader);
		}
	}	

	delete sortedLMSReader; sortedLMSReader = NULL;

	delete inducedLReader; inducedLReader = NULL;

	delete inducedSReader; inducedSReader = NULL;

	delete sBuffer; sBuffer = NULL;

	delete saBuffer; saBuffer = NULL;

	delete tBuffer; tBuffer = NULL;

	Test::printMsg(std::string("testRetrieveLMS2 passed\n"));
}


template<typename T, size_t Alphabet>
void LCPAChecker<T, Alphabet>::Test::testCheckL(const std::string & _sFileName, const std::string & _lcpFileName, const size_t _len, t_vector_type * _sortedLCP_L) {

	///step 1: load file 
	//load s
	std::ifstream sFin(_sFileName, std::ios::in | std::ios::binary);

	uint8 * sBuffer = new uint8[_len];

	sFin.read(reinterpret_cast<char*>(sBuffer), _len * sizeof(uint8) / sizeof(char));
	
	//load lcp
	std::ifstream lcpFin(_lcpFileName, std::ios::in | std::ios::binary);

	T * lcpBuffer = new T[_len];

	lcpFin.read(reinterpret_cast<char*>(lcpBuffer), _len * sizeof(T) / sizeof(char));

	///step 2: determine type and compute number of L-type and S-type for each character
	size_t lNum[Alphabet] = {0};

	size_t sNum[Alphabet] = {0};

	uint8 lastType, curType;

	//rightmost is L_TYPE
	curType = L_TYPE;

	++lNum[sBuffer[_len - 1]];

	lastType = curType;
	
	for (int i = _len - 2; i >= 0; --i) {

		if (sBuffer[i] < sBuffer[i + 1] || (sBuffer[i] == sBuffer[i + 1] && lastType == S_TYPE)) {

			curType = S_TYPE;

			++sNum[sBuffer[i]];
		}
		else {
			
			curType = L_TYPE;

			++lNum[sBuffer[i]];
		}	

		lastType = curType;
	}

	///step 3: check LCP_{L}
	typename t_vector_type::bufreader_type *sortedLCP_L_Reader = new typename t_vector_type::bufreader_type(*_sortedLCP_L);
	
	for (size_t i = 0, k = 0; i < Alphabet; ++i) {

		if (lNum[i] != 0) {
		
			for (size_t j = 0; j < lNum[i]; ++j, ++k) {

				if (lcpBuffer[k] != *(*sortedLCP_L_Reader)) {

					Test::printMsg(std::string("ch: ").append(std::to_string(int(i))).append("\n"));						
					Test::printMsg(std::string("k: ").append(std::to_string(k)).append("\n"));						
					Test::printMsg(std::string("lcpBuffer: ").append(std::to_string(lcpBuffer[k])).append("\n"));
				
					Test::printMsg(std::string("lcpReader: ").append(std::to_string(*(*sortedLCP_L_Reader))).append("\n"));

					Test::printMsg(std::string("check LCP_L wrong"));

					std::cin.get();

					//exit(0);
				}

				++(*sortedLCP_L_Reader);
			}			
		}
	
		if (sNum[i] != 0) {

			for (size_t j = 0; j < sNum[i]; ++j, ++k);
		}	
	}	

	delete sortedLCP_L_Reader; sortedLCP_L_Reader = NULL;

	delete lcpBuffer; lcpBuffer = NULL;

	delete sBuffer; sBuffer = NULL; 

	Test::printMsg(std::string("testCheckL passed\n"));
}


//! check the whole lcp rather than LCP_{S} to verify the correctness of checkS member function
template<typename T, size_t Alphabet>
void LCPAChecker<T, Alphabet>::Test::testCheckS(const std::string & _sFileName, const std::string & _saFileName, const std::string & _lcpFileName, const size_t _len, t_vector_type * _sortedReverseLCP) {

	///step 1: load file 	
	//load s
	std::ifstream sFin(_sFileName, std::ios::in | std::ios::binary);

	uint8 * sBuffer = new uint8[_len];

	sFin.read(reinterpret_cast<char*>(sBuffer), _len * sizeof(uint8) / sizeof(char));

	//load sa
	std::ifstream saFin(_saFileName, std::ios::in | std::ios::binary);

	T * saBuffer = new T[_len];

	saFin.read(reinterpret_cast<char*>(saBuffer), _len * sizeof(T) / sizeof(char));

	//load lcp
	std::ifstream lcpFin(_lcpFileName, std::ios::in | std::ios::binary);

	T * lcpBuffer = new T[_len];

	lcpFin.read(reinterpret_cast<char*>(lcpBuffer), _len * sizeof(T) / sizeof(char));


	///step 2: determine type
	uint8 *tBuffer = new uint8[_len];

	tBuffer[_len - 1] = L_TYPE;

	for (int i = _len - 2; i >= 0; --i) {

		if (sBuffer[i] < sBuffer[i + 1] || (sBuffer[i] == sBuffer[i + 1] && tBuffer[i + 1] == S_TYPE)) {

			tBuffer[i] = S_TYPE;
		}
		else {

			tBuffer[i] = L_TYPE;
		}
	}
	

	///step 3: check the equality of the two lcp arrays
	typename t_vector_type::bufreader_type * sortedLCP_ReverseReader = new typename t_vector_type::bufreader_type(*_sortedReverseLCP);

	for (int i = _len -1; i >= 0; --i, ++(*sortedLCP_ReverseReader)) {
		
		if (lcpBuffer[i] != *(*sortedLCP_ReverseReader)) {

			Test::printMsg(std::string("lcpBuffer != lcpReader wrong\n"));

			Test::printMsg(std::string("lcpBuffer[").append(std::to_string(i)).append("]:").append(std::to_string(lcpBuffer[i])).append(" "));
			Test::printMsg(std::string("lcpReader[").append(std::to_string(i)).append("]:").append(std::to_string(*(*sortedLCP_ReverseReader))).append("\n"));

			std::cerr << "sa: " << saBuffer[i] << std::endl;

			std::cerr << "ch: " << (uint32)sBuffer[saBuffer[i]] << " type: " << (tBuffer[saBuffer[i]] ? "S_TYPE" : "L_TYPE") << std::endl;

			std::cerr << "ch2: " << (uint32)sBuffer[saBuffer[i - 1]] << " type2: " << (tBuffer[saBuffer[i - 1]] ? "S_TYPE" : "L_TYPE") << std::endl;
			std::cerr << "ch3: " << (uint32)sBuffer[saBuffer[i] + 1] << " type2: " << (tBuffer[saBuffer[i] + 1] ? "S_TYPE" : "L_TYPE") << std::endl;


			exit(0);
		}
	}


	//clear up
	delete sBuffer; sBuffer = NULL;

	delete saBuffer; saBuffer = NULL;

	delete lcpBuffer; lcpBuffer = NULL;

	delete tBuffer; tBuffer = NULL;
	
	
	Test::printMsg(std::string("testCheckS passed\n"));	
}


template<typename T, size_t Alphabet>
inline void LCPAChecker<T, Alphabet>::Test::printMsg(const std::string & _msg) {

#ifdef TEST_PROGRAM

	std::cerr << std::string(_msg);

#endif

}
NAMESPACE_LCP_EM2_END


#endif
