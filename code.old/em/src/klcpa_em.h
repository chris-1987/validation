#ifndef KLCPA_EM_H
#define KLCPA_EM_H

#include "../../common/namespace.h"
#include "../../common/common.h"
#include "../../common/data.h"

NAMESPACE_KLCP_EM_BEG

//! K-order LCP array checker using external memory

//! \param T element type of SA array
//! element type of LCP is uint16. By default, K is smaller than std::numeric_limits<uint16>::max().

template<typename T>
class KLCPAChecker{
private:

	//alias
	typedef typename ExVector<uint8>::vector uint8_vector_type;	

	typedef typename ExVector<T>::vector t_vector_type;

	typedef typename ExVector<uint16>::vector uint16_vector_type;

	typedef Quadruple<T, T, uint16, uint16> TupleA; //!< SA[i], i, lcp[i], lcp[i + 1]

	typedef TupleLessComparator1<TupleA> TupleALessComparator; //! key <1st>

	typedef typename ExTupleAscSorter<TupleA, TupleALessComparator>::sorter AscSorterA;

	typedef typename ExTupleAscSorter<TupleA, TupleALessComparator>::comparator AscComparatorA;

	typedef Sixtuple<T, FPTYPE_A, FPTYPE_A, uint16, uint16, uint16> TupleB; //!< (ISA[i], FPInterval1, FPInterval2, ch1, ch2, lcp1)

	typedef TupleLessComparator1<TupleB> TupleBLessComparator; //! key <1st>	

	typedef typename ExTupleAscSorter<TupleB, TupleBLessComparator>::sorter AscSorterB;

	typedef typename ExTupleAscSorter<TupleB, TupleBLessComparator>::comparator AscComparatorB; 

	//
	uint8_vector_type * mS;
	
	t_vector_type * mSA;

	uint16_vector_type * mLCP;

	size_t mLen;

	FPTYPE_A * mRInterval;

	stxxl::syscall_file * mSFile;

	stxxl::syscall_file * mSAFile;

	stxxl::syscall_file * mLCPFile;

public:

	KLCPAChecker(std::string & _sFileName, std::string & _saFileName, std::string & _lcpFileName, size_t _len);
	
	bool runs();
};


template<typename T>
KLCPAChecker<T>::KLCPAChecker(
	std::string & _sFileName,
	std::string & _saFileName,
	std::string & _lcpFileName,
	size_t _len) :
	mLen(_len){
	
	stxxl::stats *Stats = stxxl::stats::get_instance();

	stxxl::block_manager *bm = stxxl::block_manager::get_instance();

	///step 1: pre-process
	mSFile = new stxxl::syscall_file(_sFileName, 
		stxxl::syscall_file::RDWR | stxxl::syscall_file::DIRECT);

	mSAFile = new stxxl::syscall_file(_saFileName,
		stxxl::syscall_file::RDWR | stxxl::syscall_file::DIRECT);

	mLCPFile = new stxxl::syscall_file(_lcpFileName,
		stxxl::syscall_file::RDWR | stxxl::syscall_file::DIRECT);


	///step 2: build lcp array
	if (!runs()) {
		
		std::cerr << "check failed!\n";
	}
	else {
		
		std::cerr << "check passed\n";
	}

	std::cout << "I/O Volume: " << (Stats->get_written_volume() + Stats->get_read_volume()) / mLen << std::endl;

	std::cout << "Peak Disk Usage: " << bm->get_maximum_allocation() / mLen << std::endl;	

	return ;
}


template<typename T>
bool KLCPAChecker<T>::runs() {
	
	///step 1: compute mRInterval[0, K] in RAM
	mRInterval = new FPTYPE_A[K + 1];
	
	mRInterval[0] = 1;

	for (uint16 i = 1; i <= K; ++i) {
			
		mRInterval[i] = static_cast<FPTYPE_A>((static_cast<FPTYPE_B>(mRInterval[i - 1]) * R) % P);
	}


	///step 2: sort (SA[i], LCP[i], LCP[i + 1], i) by SA[i] in ascending order.
	mSA = new t_vector_type(mSAFile);

	typename t_vector_type::bufreader_type *saReader = new typename t_vector_type::bufreader_type(*mSA);
	
	mLCP = new uint16_vector_type(mLCPFile);
	
	typename uint16_vector_type::bufreader_type *lcpReader = new typename uint16_vector_type::bufreader_type(*mLCP);

	AscSorterA *ascSorterA = new AscSorterA(AscComparatorA(), MAX_MEM / 3);

	uint16 curLCP = *(*lcpReader);
	
	++(*lcpReader); // <SA[i], i, LCP[i], LCP[i + 1]>

	T idx = 1; //start from 1 to avoid (0, 0) for STXXL

	for (size_t i = 0; i < mLen - 1; ++i, ++idx, ++(*saReader), ++(*lcpReader)) {//[0, mLen - 1)

		ascSorterA->push(TupleA(*(*saReader), idx, curLCP, *(*lcpReader))); //(SA[i], curLCP, nextLCP, i)

		curLCP = *(*lcpReader);
	}
	
	ascSorterA->push(TupleA(*(*saReader), idx, curLCP, 0)); //[mLen - 1, mLen)

	delete saReader; saReader = NULL; delete mSA; mSA = NULL; delete mSAFile; mSAFile = NULL;
	
	delete lcpReader; lcpReader = NULL; delete mLCP; mLCP = NULL; delete mLCPFile; mLCPFile = NULL;

	ascSorterA->sort();


	///step 3: compute required fpInterval, sort (ISA[i], fpInterval1, fpInterval2) by ISA[i] in ascending order.
	AscSorterB *ascSorterB = new AscSorterB(AscComparatorB(), MAX_MEM / 3);

	mS = new uint8_vector_type(mSFile);

	typename uint8_vector_type::bufreader_type *sReader = new typename uint8_vector_type::bufreader_type(*mS);

	size_t bufSize = MAX_MEM / 3 / (sizeof(uint8) + sizeof(FPTYPE_A));

	FPTYPE_A *fpBuf = new FPTYPE_A[bufSize];

	uint8 *sBuf = new uint8[bufSize];

	size_t toRead = mLen, reading; 

	bool isFirstBlock = true;

	T startOffset, endOffset, pos1, pos2, pos3;

	uint8 ch1, ch2;

	FPTYPE_A preFP = 0, fp1, fp2, fp3, fpInterval1, fpInterval2;

	while (toRead) { //! assumption: toRead > K

		if (isFirstBlock) {

			isFirstBlock = false;

			reading = (toRead > bufSize) ? bufSize : toRead;

			toRead -= reading;

			sBuf[0] = *(*sReader); 

			++(*sReader);

			fpBuf[0] = sBuf[0] + 1; // plus one to avoid 0

			for (size_t i = 1; i < reading; ++i, ++(*sReader)) {
				
				sBuf[i] = *(*sReader);	

				fpBuf[i] = static_cast<FPTYPE_A>((static_cast<FPTYPE_B>(fpBuf[i - 1]) * R + (sBuf[i] + 1)) % P);
			}

			startOffset = 0; 

			endOffset = startOffset + reading - K - 1;
		}
		else {

			reading = (toRead > (bufSize - K)) ? (bufSize - K) : toRead;

			toRead -= reading;

			for (size_t i = 0; i < reading; ++i, ++(*sReader)) {
			
				sBuf[K + i] = *(*sReader);	

				fpBuf[K + i] = static_cast<FPTYPE_A>((static_cast<FPTYPE_B>(fpBuf[K + i - 1]) * R + (sBuf[K + i] + 1)) % P);
			}

			startOffset = endOffset + 1;

			endOffset = startOffset + reading - 1;
		}


		std::cerr << "startOffset: " << startOffset << " endOffset: " << endOffset << std::endl;

		//process leftmost element of current block, where tuple.first == startOffset
		const TupleA & tuple1 = *(*ascSorterA);

		++(*ascSorterA);

		pos1 = tuple1.first - startOffset;

		fp1 = preFP;

		pos2 = pos1 + tuple1.third - 1;

		fp2 = (tuple1.third == 0) ? preFP : fpBuf[pos2];

		ch1 = sBuf[pos2 + 1];

		fpInterval1 = static_cast<FPTYPE_A>((fp2 - (static_cast<FPTYPE_B>(fp1) * mRInterval[tuple1.third]) % P + P) % P);
	
		pos3 = pos1 + tuple1.forth - 1;
	
		fp3 = (tuple1.forth == 0) ? preFP : fpBuf[pos3];

		ch2 = sBuf[pos3 + 1];		

		fpInterval2 = static_cast<FPTYPE_A>((fp3 - (static_cast<FPTYPE_B>(fp1) * mRInterval[tuple1.forth]) % P + P) % P);
	
		ascSorterB->push(TupleB(tuple1.second, fpInterval1, fpInterval2, ch1, ch2, tuple1.third));	

		//process the remaining elements of current block (startOffset, endOffset]
		for (T i = startOffset + 1; i <= endOffset; ++i, ++(*ascSorterA)) {

			const TupleA & tuple2 = *(*ascSorterA);

			pos1 = tuple2.first - startOffset;

			fp1 = fpBuf[pos1 - 1];

			pos2 = pos1 + tuple2.third - 1;

			fp2 = fpBuf[pos2];
			
			ch1 = sBuf[pos2 + 1];

			fpInterval1 = static_cast<FPTYPE_A>((fp2 - (static_cast<FPTYPE_B>(fp1) * mRInterval[tuple2.third]) % P + P) % P);

			pos3 = pos1 + tuple2.forth - 1;

			fp3 = fpBuf[pos3];
		
			ch2 = sBuf[pos3 + 1];
	
			fpInterval2 = static_cast<FPTYPE_A>((fp3 - (static_cast<FPTYPE_B>(fp1) * mRInterval[tuple2.forth]) % P + P) % P);

			ascSorterB->push(TupleB(tuple2.second, fpInterval1, fpInterval2, ch1, ch2, tuple2.third));
		}
			
		preFP = fpBuf[endOffset - startOffset];

		for (uint16 i = 0; i < K; ++i) { //copy last K elements of previous block to current block

			fpBuf[i] = fpBuf[endOffset + 1 - startOffset + i];

			sBuf[i] = sBuf[endOffset + 1 - startOffset + i];
		}
	}

	delete sReader; sReader = NULL;


	//process the final K elements
	startOffset = endOffset + 1;

	std::cerr << "startOffset: " << startOffset << std::endl;	

	bool flag = false;

	const TupleA & tuple3 = *(*ascSorterA);

	++(*ascSorterA);

	pos1 = tuple3.first - startOffset;

	fp1 = preFP;
	
	pos2 = pos1 + tuple3.third - 1;

	fp2 = (tuple3.third == 0) ? preFP : fpBuf[pos2];

	fpInterval1 = static_cast<FPTYPE_A>((fp2 - (static_cast<FPTYPE_B>(fp1) * mRInterval[tuple3.third]) % P + P) % P);

	if (pos2 + startOffset == mLen - 1) { //reaches the boundary

		ch1 = std::numeric_limits<uint8>::max();
		
		flag = true;
	}
	else {

		ch1 = sBuf[pos2 + 1];
	}

	pos3 = pos1 + tuple3.forth - 1;

	fp3 = (tuple3.forth == 0) ? preFP : fpBuf[pos3];

	fpInterval2 = static_cast<FPTYPE_A>((fp3 - (static_cast<FPTYPE_B>(fp1) * mRInterval[tuple3.forth]) % P + P) % P);

	if (pos3 + startOffset == mLen - 1) {

		ch2 = std::numeric_limits<uint8>::max();

		flag = true;
	}	
	else {

		ch2 = sBuf[pos3 + 1];
	}

	ascSorterB->push(TupleB(tuple3.second, fpInterval1, fpInterval2, ch1, ch2, (flag ? std::numeric_limits<uint16>::max() : tuple3.third)));


	for (T i = startOffset + 1 ; !ascSorterA->empty(); ++i, ++(*ascSorterA)) {

		flag = false;

		const TupleA & tuple4 = *(*ascSorterA);
	
		pos1 = tuple4.first - startOffset;
	
		fp1 = fpBuf[pos1 - 1];
				
		pos2 = pos1 + tuple4.third - 1;

		fp2 = fpBuf[pos2];

		fpInterval1 = static_cast<FPTYPE_A>((fp2 - (static_cast<FPTYPE_B>(fp1) * mRInterval[tuple4.third]) % P + P) % P);
		
		if (pos2 + startOffset == mLen - 1) {

			ch1 = std::numeric_limits<uint8>::max();
		
			flag = true;
		}
		else {

			ch1 = sBuf[pos2 + 1];
		}
			
		pos3 = pos1 + tuple4.forth - 1;
	
		fp3 = fpBuf[pos3];
	
		fpInterval2 = static_cast<FPTYPE_A>((fp3 - (static_cast<FPTYPE_B>(fp1) * mRInterval[tuple4.forth]) % P + P) % P);
	
		if (pos3 + startOffset == mLen - 1) {

			ch2 = std::numeric_limits<uint8>::max();

			flag = true;
		}
		else {
	
			ch2 = sBuf[pos3 + 1];
		}	
	
		ascSorterB->push(TupleB(tuple4.second, fpInterval1, fpInterval2, ch1, ch2, (flag ? std::numeric_limits<uint16>::max() : tuple4.third)));
	}

	delete sBuf; sBuf = NULL;

	delete fpBuf; fpBuf = NULL;

	delete ascSorterA; ascSorterA = NULL;

	delete mRInterval; mRInterval = NULL;

	delete sReader; sReader = NULL; delete mS; mS = NULL; delete mSFile; mSFile = NULL;

	ascSorterB->sort();


	///step 3: check the correctness of the LCP array 
	bool isRight = true;

	FPTYPE_A preFPInterval = (*ascSorterB)->third;

	uint8 preCh = (*ascSorterB)->fifth;

	++(*ascSorterB);
	
	for (size_t i = 1; i < mLen; ++i, ++(*ascSorterB)) {

		const TupleB & tuple = *(*ascSorterB);

		if (preFPInterval != tuple.second) {

			std::cerr << "here1: i: " << tuple.first << " fp1: " << tuple.second << " fp2: " << tuple.third << " ch1: " << tuple.forth << " ch2: " << tuple.fifth << " lcp: " << tuple.sixth << std::endl;

			isRight = false;	

			break;
		}	
		else {//equal fpInterval

			if (tuple.sixth != std::numeric_limits<uint16>::max()){

				if (preCh == tuple.forth && tuple.sixth != K) {

					std::cerr << "here2: i: " << tuple.first << " fp1: " << tuple.second << " fp2: " << tuple.third << " ch1: " << tuple.forth << " ch2: " << tuple.fifth << " lcp: " << tuple.sixth << std::endl;
	
					isRight = false;
					
					break;
				}
			}
		}

		preFPInterval = tuple.third;

		preCh = tuple.fifth;
	}
		 
	delete ascSorterB; ascSorterB = NULL;

	return isRight;
}


//! k-order lcp array builder using external memory

//! \param T element type of SA
//! \param B each character is represented by one byte in the input string. During the process of our program, we encapsulate every (8 / B) characters into a byte for efficient string comparison, where B = {2, 4, 8} is determined by the alphabet size.
//! \param H partition (SA[i], SA[i - 1] into two sets: A = {lcp < H / B} and B = {lcp >= H / B}. H = {16, 32, 64, 128}
//! element type of LCP is uint16. By default, K is smaller than std::numeric_limits<uint16>::max().

template<typename T, uint8 B = 8, uint16 H = 128>
class KLCPABuilder{

private:

	//alias
	typedef typename choose_uint_type<H>::uint_type uint_type;

	typedef typename ExVector<uint8>::vector uint8_vector_type; 

	typedef typename ExVector<T>::vector t_vector_type;

	typedef typename ExVector<uint16>::vector uint16_vector_type;

	typedef Pair<T, T> TupleA; //!< (SA[i], i) / (PA[i], i)

	typedef TupleLessComparator2<TupleA> TupleALessComparator;

	typedef typename ExTupleAscSorter<TupleA, TupleALessComparator>::sorter AscSorterA; //key <1st, 2nd>

	typedef typename ExTupleAscSorter<TupleA, TupleALessComparator>::comparator AscComparatorA;

	typedef Pair<T, uint_type> TupleB; //!< (ISA[i], substr), substr is of type uint_type

	typedef TupleLessComparator1<TupleB> TupleBLessComparator;

	typedef typename ExTupleAscSorter<TupleB, TupleBLessComparator>::sorter AscSorterB; //key <1st>

	typedef typename ExTupleAscSorter<TupleB, TupleBLessComparator>::comparator AscComparatorB;

	typedef Triple<T, T, FPTYPE_A> TupleC; //!< (IPA[i], i, FPTYPE_A>

	typedef TupleLessComparator2<TupleC> TupleCLessComparator; //!< (IPA[i], i, FPTYPE_A>
	
	typedef typename ExTupleAscSorter<TupleC, TupleCLessComparator>::sorter AscSorterC; //key <1st, 2nd>

	typedef typename ExTupleAscSorter<TupleC, TupleCLessComparator>::comparator AscComparatorC;
		
	typedef Triple<T, T, uint_type> TupleD;

	typedef TupleLessComparator1<TupleD> TupleDLessComparator;
	
	typedef typename ExTupleAscSorter<TupleD, TupleDLessComparator>::sorter AscSorterD; //key <1st>

	typedef typename ExTupleAscSorter<TupleD, TupleDLessComparator>::comparator AscComparatorD;

	stxxl::syscall_file *mSFile;

	stxxl::syscall_file *mSAFile;

	stxxl::syscall_file *mLCPFile;
	
	uint8_vector_type * mS; //!< one byte per character 

	t_vector_type * mSA;  

	uint16_vector_type * mLCP1; //!< two bytes per element

	uint16_vector_type * mLCP2; //!< two bytes per element

	size_t mLen; 

	FPTYPE_A *mRInterval; 

	t_vector_type *mPA1;

	t_vector_type *mPA2;

	size_t mPALen;

	uint_type OFFSET[H / B];

	size_t mFPBufSize;

	FPTYPE_A *mFPBuf;

	AscSorterA *mAscSorterA1;

	AscSorterA *mAscSorterA2;

	AscSorterC *mAscSorterC1;

	AscSorterC *mAscSorterC2;

public:

	KLCPABuilder(std::string & _sFileName, std::string & _saFileName, std::string & _lcpFileName, size_t _len);

	void runs();

	void filterRun();

	uint16 computePrefixLen(const uint_type & _lhs, const uint_type & _rhs, uint16 _maxLen);

	void iterRuns();

	void finalRun();

	void print_uint(uint_type _item);
};

template<typename T, uint8 B, uint16 H>
KLCPABuilder<T, B, H>::KLCPABuilder(
	std::string & _sFileName,
	std::string & _saFileName,
	std::string & _lcpFileName,
	size_t _len) :
	mLen(_len) {

	stxxl::stats *Stats = stxxl::stats::get_instance();

	stxxl::block_manager *bm = stxxl::block_manager::get_instance();

	//step 1: compute OFFSET
	OFFSET[H / B - 1] = static_cast<uint_type>(pow(2, B) - 1);
		
	for (int32 i = H / B - 2; i >= 0; --i) { //std::numeric_limits<int32>::max() > std::numeric_limits<uint16>::max()

		OFFSET[i] = OFFSET[i + 1] << B;		
	}


	//step 2: associate filestreams with external memory vectors
	mSFile = new stxxl::syscall_file(_sFileName,
		stxxl::syscall_file::RDWR | stxxl::syscall_file::DIRECT);

	mSAFile = new stxxl::syscall_file(_saFileName,
		stxxl::syscall_file::RDWR | stxxl::syscall_file::DIRECT);

	mLCPFile = new stxxl::syscall_file(_lcpFileName,
		stxxl::syscall_file::CREAT | stxxl::syscall_file::RDWR | stxxl::syscall_file::DIRECT);


	//step 3: build LCP array
	runs();

	std::cout << "I/O Volume: " << (Stats->get_written_volume() + Stats->get_read_volume()) / mLen << std::endl;

	std::cout << "Peak Disk Usage: " << bm->get_maximum_allocation() / mLen << std::endl;	

	return ;
};

template<typename T, uint8 B, uint16 H>
void KLCPABuilder<T, B, H>::runs() {

	///step 1: compute mRInterval[0, K]
	mRInterval = new FPTYPE_A[K + 1];

	mRInterval[0] = 1;

	for (size_t i = 1; i <= K; ++i) {

		mRInterval[i] = static_cast<FPTYPE_A>((static_cast<FPTYPE_B>(mRInterval[i - 1]) * R) % P);
	}


	///step 2: partition (SA[i], SA[i - 1]) into two sets 
	filterRun();	

	std::cerr << "filterrun end\n";

	///step 3: determine lcp between mPA1 and mPA2
	//perform iterations (FPs)
	iterRuns();
	std::cerr << "iterruns end\n";

	///step 4: perform final run (literally compare H / B characters for each pair)
	finalRun();
	std::cerr << "finalrun end\n";

	delete[] mRInterval;

	return;


}


template<typename T, uint8 B, uint16 H>
void KLCPABuilder<T, B, H>::filterRun() {

	std::cerr << "start filterRun()" << std::endl;

	///step 1: scan mSA to pack (mSA[i], i) and sort the items by mSA[i] in ascending order.
	mSA = new t_vector_type(mSAFile);

	typename t_vector_type::bufreader_type *saReader = new typename t_vector_type::bufreader_type(*mSA);

	AscSorterA * ascSorterA = new AscSorterA(AscComparatorA(), MAX_MEM / 2);

	T idx = 1; //start from 1 to avoid (0, 0) for STXXL
	for (size_t i = 0; i < mLen; ++i, ++idx, ++(*saReader)) {
		
		ascSorterA->push(TupleA(*(*saReader), idx)); 
	}
	
	assert(saReader->empty() == true); 
	
	delete saReader; saReader = NULL;

	ascSorterA->sort();

	std::cerr << "ascSorterA->size(): " << ascSorterA->size() << std::endl;

	///step 2: scan mS and ascSorterA to pack (ISA[i], substr) and sort the items by ISA[i] in ascending order.
	mS = new uint8_vector_type(mSFile);

	typename uint8_vector_type::bufreader_type * sReader = new typename uint8_vector_type::bufreader_type(*mS);

	AscSorterB * ascSorterB = new AscSorterB(AscComparatorB(), MAX_MEM / 2);

	uint_type substr = 0;

	for (size_t i = 0; i < H / B - 1; ++i, ++(*sReader)) {

		substr <<= B;

		substr += *(*sReader); //todo: convert 1 byte to B bits
	}

	for (size_t i = H / B - 1; i < mLen; ++i, ++(*sReader), ++(*ascSorterA)) { 

		substr <<= B;

		substr += *(*sReader); //todo: convert 1 byte to B bits

		ascSorterB->push(TupleB((*ascSorterA)->second, substr));
	}

	for (size_t i = 0; i < H / B - 1; ++i, ++(*ascSorterA)){

		substr <<= B;

		ascSorterB->push(TupleB((*ascSorterA)->second, substr));
	}

	assert(ascSorterA->empty()== true); 
	
	assert(sReader->empty() == true); 

	delete ascSorterA; ascSorterA = NULL;

	delete sReader; sReader = NULL;

	ascSorterB->sort();

	std::cerr << "ascSorterB->size(): " << ascSorterB->size() << std::endl;


	///step 3: compare each pair of substr to determine their common prefix. Meanwhile, produce mPA1 and mPA2.
	mLCP1 = new uint16_vector_type(); mLCP1->resize(mLen);

	typename uint16_vector_type::bufwriter_type *lcpWriter = new typename uint16_vector_type::bufwriter_type(*mLCP1);

	mPA1 = new t_vector_type(); mPA1->resize(mLen);

	typename t_vector_type::bufwriter_type *pa1Writer = new typename t_vector_type::bufwriter_type(*mPA1);

	mPA2 = new t_vector_type();  mPA2->resize(mLen);

	typename t_vector_type::bufwriter_type *pa2Writer = new typename t_vector_type::bufwriter_type(*mPA2);

	mPALen = 0; //number of elements inserted into mPA1/mPA2

	saReader = new typename t_vector_type::bufreader_type(*mSA);

	uint_type preStr;

	T preSA, curSA;

	size_t minDiff;

	uint16 prefixLen;

	(*lcpWriter) << 0; //lcp[0] = 0 by default

	preStr = (*ascSorterB)->second, ++(*ascSorterB);

	preSA = *(*saReader), ++(*saReader);

	for (size_t i = 1; !ascSorterB->empty(); ++i, ++(*ascSorterB), ++(*saReader)) {
		
		const TupleB & tupleB = *(*ascSorterB);

		curSA = *(*saReader);

		minDiff = mLen - std::max(preSA, curSA);

		if (minDiff < H / B) {

			prefixLen = computePrefixLen(tupleB.second, preStr, static_cast<uint16>(minDiff));
		}
		else {

			prefixLen = computePrefixLen(tupleB.second, preStr, H / B);
		}

		if (prefixLen == H / B) {
			
			(*pa1Writer) << curSA;

			(*pa2Writer) << preSA;

			if (2746465 == curSA) std::cerr << "i: " << i << " curSA: " << curSA << " preSA: " << preSA << " mPALen: " << mPALen << std::endl;

			++mPALen;
		}

		(*lcpWriter) << prefixLen; //if prefixLen == H, it means lcp >= H and more work is required to determine the LCP.

		preStr = tupleB.second;

		preSA = curSA;
	}

	(*lcpWriter).finish(); delete lcpWriter; lcpWriter = NULL;

	(*pa1Writer).finish(); delete pa1Writer; pa1Writer = NULL;
	
	(*pa2Writer).finish(); delete pa2Writer; pa2Writer = NULL;

	assert(saReader->empty() == true);

	assert(ascSorterB->empty() == true);

	delete saReader; saReader = NULL;

	delete ascSorterB; ascSorterB = NULL;

	mPA1->resize(mPALen);

	mPA2->resize(mPALen);

	std::cerr << "mPALen: " << mPALen << std::endl;

	std::cerr << "mPA1->size(): " << mPA1->size() << std::endl;

	std::cerr << "mPA2->size(): " << mPA2->size() << std::endl;

	std::cerr << "finish filterRun\n";

	return;
}

template<typename T, uint8 B, uint16 H>
inline uint16 KLCPABuilder<T, B, H>::computePrefixLen(const uint_type & _lhs, const uint_type & _rhs, uint16 _maxLen){

	uint16 prefixLen = 0;

	for (uint16 i = 0; i < _maxLen; ++i) {
		
		if ((_lhs & OFFSET[i]) != (_rhs & OFFSET[i])) {//todo O
		
			break;
		}

		++prefixLen;
	}

	return prefixLen;
}


template<typename T, uint8 B, uint16 H>
void KLCPABuilder<T, B, H>::iterRuns() {

	mAscSorterA1 = new AscSorterA(AscComparatorA(), MAX_MEM / 6);

	mAscSorterA2 = new AscSorterA(AscComparatorA(), MAX_MEM / 6);

	typename t_vector_type::bufreader_type *pa1Reader = new typename t_vector_type::bufreader_type(*mPA1);

	typename t_vector_type::bufreader_type *pa2Reader = new typename t_vector_type::bufreader_type(*mPA2);

	///step 1: pack (mPA1[i], i) and (mPA2[i], i), respectively sort the items by mPA1[i] and mPA2[i] in ascending order. 
	for (T i = 1; !pa1Reader->empty(); ++i, ++(*pa1Reader), ++(*pa2Reader)) { //start from 1 to avoid (0, 0) for STXXL

		if (116508 == i) std::cerr << "pa1: " << *(*pa1Reader) << " pa2: " << *(*pa2Reader) << std::endl;
	
		mAscSorterA1->push(TupleA(*(*pa1Reader), i));

		mAscSorterA2->push(TupleA(*(*pa2Reader), i));
	}

	delete pa1Reader; pa1Reader = NULL; delete mPA1; mPA1 = NULL;

	delete pa2Reader; pa1Reader = NULL; delete mPA2; mPA2 = NULL;
	
	mAscSorterA1->sort();

	mAscSorterA2->sort();	

	///step 2: run iterations
	mAscSorterC1 = new AscSorterC(AscComparatorC(), MAX_MEM / 6);

	mAscSorterC2 = new AscSorterC(AscComparatorC(), MAX_MEM / 6);

//	mFPBufSize = MAX_MEM / 3 / sizeof(FPTYPE_A);

	mFPBufSize = MAX_MEM / 100 / sizeof(FPTYPE_A);

	mFPBuf = new FPTYPE_A[mFPBufSize];

	for (uint16 stride = K / 2; stride >= H / B; stride >>= 1) {

		//pack (IPA1[i], fpInterval1) and (IPA2[i], fpInterval2), sort the items by IPA1[i] and IPA2[i] 
		typename uint8_vector_type::bufreader_type *sReader = new uint8_vector_type::bufreader_type(*mS);

		size_t toRead = mLen, reading;

		T startOffset, endOffset, pos1, pos2, pos3, pos4;

		FPTYPE_A fpInterval, fp1, fp2, preFP = 0;

		bool isFirstBlock = true;	

		while (toRead) {

			if (isFirstBlock) { //process the 1st block on the left

				isFirstBlock = false;

				reading = (toRead > mFPBufSize) ? mFPBufSize : toRead;

				toRead -= reading;
				
				//first fp
				mFPBuf[0] = *(*sReader) + 1; //plus 1 to avoid 0

				++(*sReader);
		
				for (size_t i = 1; i < reading; ++i, ++(*sReader)) {
					
					mFPBuf[i] = static_cast<FPTYPE_A>((static_cast<FPTYPE_B>(mFPBuf[i - 1]) * R + (*(*sReader) + 1)) % P);
				}

				startOffset = 0;

				endOffset = startOffset + reading - stride;
			}
			else { //process the remaining blocks, the head (stride - 1) characters are copied from previous block

				reading = (toRead > (mFPBufSize - stride + 1)) ? (mFPBufSize - stride + 1) : toRead;

				toRead -= reading;

				for (size_t i = 0; i < reading; ++i, ++(*sReader)) {

					mFPBuf[stride - 1 + i] = static_cast<FPTYPE_A>((static_cast<FPTYPE_B>(mFPBuf[stride - 1 + i - 1]) * R + (*(*sReader) + 1)) % P);
				}

				startOffset = endOffset + 1;

				endOffset = startOffset + reading - 1;
			}

			std::cerr << "stride: " << stride << " startOffset: " << startOffset << " endOffset: " << endOffset << std::endl;
			
			//process mAscSorter1
			while (!mAscSorterA1->empty() && (*mAscSorterA1)->first <= endOffset) {
				
				const TupleA & tuple1 = *(*mAscSorterA1);

				pos1 = tuple1.first - startOffset;

				fp1 = (0 == pos1) ? preFP : mFPBuf[pos1 - 1];

				pos2 = pos1 + stride - 1; //pos2 + stride - 1 < mLen
		
				fpInterval = static_cast<FPTYPE_A>((mFPBuf[pos2] - (static_cast<FPTYPE_B>(fp1) * mRInterval[stride]) % P + P) % P);

				mAscSorterC1->push(TupleC(tuple1.second, tuple1.first, fpInterval));

				if (116508 ==(*mAscSorterA1)->second) std::cerr << "curSA: " << (*mAscSorterA1)->first << " fpInterval: " << fpInterval << std::endl;

				++(*mAscSorterA1);

			}

			//process mAscSorter2
			while (!mAscSorterA2->empty() && (*mAscSorterA2)->first <= endOffset) {

				const TupleA & tuple2 = *(*mAscSorterA2);

				pos3 = tuple2.first - startOffset;

				fp2 = (0 == pos3) ? preFP : mFPBuf[pos3 - 1];

				pos4 = pos3 + stride - 1; //pos4 + stride - 1 < mLen

				fpInterval = static_cast<FPTYPE_A>((mFPBuf[pos4] - (static_cast<FPTYPE_B>(fp2) * mRInterval[stride]) % P + P) % P);

				mAscSorterC2->push(TupleC(tuple2.second, tuple2.first, fpInterval));

				if (116508 == (*mAscSorterA2)->second) std::cerr << "preSA: " << (*mAscSorterA2)->first << " fpInterval: " << fpInterval << std::endl;

				++(*mAscSorterA2);
			}

			//post-process
			preFP = mFPBuf[endOffset - startOffset];

			for (uint16 i = 0; i < stride - 1; ++i) {
			
				mFPBuf[i] = mFPBuf[endOffset + 1 - startOffset + i];
			}
		}

		delete sReader; sReader = NULL;

		//the length of each remaining is smaller than stride, directly set their fpInterval
		T delta = 0;

		//process the remaining elements
		for ( ; !mAscSorterA1->empty(); ++(*mAscSorterA1)) {

			const TupleA & tuple1 = *(*mAscSorterA1);

			fpInterval = P + (++delta);

			mAscSorterC1->push(TupleC(tuple1.second, tuple1.first, fpInterval));
		}

		//process mAscSorter2
		for ( ; !mAscSorterA2->empty(); ++(*mAscSorterA2)) {

			const TupleA & tuple2 = *(*mAscSorterA2);
	
			fpInterval = P + (++delta);

			mAscSorterC2->push(TupleC(tuple2.second, tuple2.first, fpInterval));
		}	

		mAscSorterA1->clear();

		mAscSorterA2->clear();

		mAscSorterC1->sort();

		mAscSorterC2->sort();


		///step 3: compare fpInterval
		for (T i = 1; !mAscSorterC1->empty(); ++i, ++(*mAscSorterC1), ++(*mAscSorterC2)) { //start from 1 to avoid (0, 0) for STXXL
			
			if ((*mAscSorterC1)->third == (*mAscSorterC2)->third) {

				if (116508 == i) std::cerr << "changed\n";

				mAscSorterA1->push(TupleA((*mAscSorterC1)->second + stride, i));

				mAscSorterA2->push(TupleA((*mAscSorterC2)->second + stride, i));
			}
			else {

				if (116508 == i) std::cerr << "unchanged\n";

				mAscSorterA1->push(TupleA((*mAscSorterC1)->second, i));

				mAscSorterA2->push(TupleA((*mAscSorterC2)->second, i));
			}
		}

		mAscSorterC1->clear();

		mAscSorterC2->clear();

		mAscSorterA1->sort();

		mAscSorterA2->sort();
	}

	
	std::cerr << "mAscSorterA1 size: " << mAscSorterA1->size() << std::endl;

	std::cerr << "mAscSorterA2 size: " << mAscSorterA2->size() << std::endl;
	

	delete mAscSorterC1; mAscSorterC1 = NULL;

	delete mAscSorterC2; mAscSorterC2 = NULL;

	delete mFPBuf; mFPBuf = NULL;

	return ;
}


template<typename T, uint8 B, uint16 H>
void KLCPABuilder<T, B, H>::finalRun() {

	///step 1: scan mS and mAscSorter1 & mAscSorter2 to pack (ISA[i], substr) and sort the items 
	typename uint8_vector_type::bufreader_type *sReader = new typename uint8_vector_type::bufreader_type(*mS);

	AscSorterD *ascSorterD1 = new AscSorterD(AscComparatorD(), MAX_MEM / 3);

	AscSorterD *ascSorterD2 = new AscSorterD(AscComparatorD(), MAX_MEM / 3);

	uint_type substr = 0;
	
	for (size_t i = 0; i < H / B - 1; ++i, ++(*sReader)) {

		substr <<= B;

		substr += *(*sReader); //todo: convert 1 byte to B bits
	}

	T idx = 0;

	for (size_t i = H / B - 1; i < mLen; ++i, ++idx, ++(*sReader)) {

		substr <<= B;
	
		substr += *(*sReader);
		
		while (!mAscSorterA1->empty() && (*mAscSorterA1)->first == idx) {
			
			ascSorterD1->push(TupleD((*mAscSorterA1)->second, (*mAscSorterA1)->first, substr));

			if (116508 == (*mAscSorterA1)->second) {
			
				std::cerr << "pa1: " << (*mAscSorterA1)->first <<std::endl;

				print_uint(substr);
			}

			++(*mAscSorterA1);
		}

		while (!mAscSorterA2->empty() && (*mAscSorterA2)->first == idx) {

			ascSorterD2->push(TupleD((*mAscSorterA2)->second, (*mAscSorterA2)->first, substr));

			if (116508 == (*mAscSorterA2)->second) {
			
				std::cerr << "pa2: " << (*mAscSorterA2)->first <<std::endl;

				print_uint(substr);
			}

			++(*mAscSorterA2);
		}
	}

	std::cerr << "idx: " << idx << " H / B: " << H / B << std::endl;
	
	for (size_t i = 0; i < H / B - 1; ++i, ++idx) {

		substr <<= B;

		while (!mAscSorterA1->empty() && (*mAscSorterA1)->first == idx) {

			ascSorterD1->push(TupleD((*mAscSorterA1)->second, (*mAscSorterA1)->first, substr));

			++(*mAscSorterA1);
		}


		while (!mAscSorterA2->empty() && (*mAscSorterA2)->first == idx) {

			ascSorterD2->push(TupleD((*mAscSorterA2)->second, (*mAscSorterA2)->first, substr));

			++(*mAscSorterA2);
		}
	}

	//process the remaining elements, where ->first == mLen
	substr <<= B;

	for ( ; !mAscSorterA1->empty(); ++(*mAscSorterA1)) {
		
		assert(mLen == (*mAscSorterA1)->first);
		
		ascSorterD1->push(TupleD((*mAscSorterA1)->second, (*mAscSorterA1)->first, substr));
	}

	for ( ; !mAscSorterA2->empty(); ++(*mAscSorterA2)) {
		
		assert(mLen == (*mAscSorterA2)->first);

		ascSorterD2->push(TupleD((*mAscSorterA2)->second, (*mAscSorterA2)->first, substr));
	}

	delete sReader; sReader = NULL; delete mS; mS = NULL; delete mSFile; mSFile = NULL;

	assert(mAscSorterA1->empty() == true);

	delete mAscSorterA1; mAscSorterA1 = NULL;

	assert(mAscSorterA2->empty() == true); 

	delete mAscSorterA2; mAscSorterA2 = NULL;

	ascSorterD1->sort(); 

	ascSorterD2->sort();

	std::cerr << "ascSorterD1->size(): " << ascSorterD1->size() << std::endl;

	std::cerr << "ascSorterD2->size(): " << ascSorterD2->size() << std::endl;


	///step 2: compare each pair of substr to determine their common prefix. Meanwhile, produce mPA1 and mPA2.
	typename uint16_vector_type::bufreader_type *lcpReader = new typename uint16_vector_type::bufreader_type(*mLCP1);

	mLCP2 = new uint16_vector_type(mLCPFile); mLCP2->resize(mLen);

	typename uint16_vector_type::bufwriter_type *lcpWriter = new typename uint16_vector_type::bufwriter_type(*mLCP2);

	typename t_vector_type::bufreader_type *saReader = new typename t_vector_type::bufreader_type(*mSA);

	uint16 curLCP;

	size_t minDiff;

	uint16 prefixLen;

	++(*saReader), ++(*lcpReader); //skip sa[0] and lcp[0]

	(*lcpWriter) << std::numeric_limits<uint16>::max();

	for (size_t i = 1, j = 0; !saReader->empty(); ++i, ++(*saReader), ++(*lcpReader)) {

		curLCP = *(*lcpReader);

		if (2746465 == *(*saReader)) std::cerr << "2746465 i: " << i << "curLCP: " << curLCP << std::endl;

		if (curLCP == H / B) { //fill correct value into current blank
		
			const TupleD & tuple1 = *(*ascSorterD1);

			const TupleD & tuple2 = *(*ascSorterD2);

			minDiff = mLen - std::max(tuple1.second, tuple2.second);

			if (minDiff < H / B) {

				prefixLen = computePrefixLen(tuple1.third, tuple2.third, static_cast<uint16>(minDiff));
			}
			else {

				prefixLen = computePrefixLen(tuple1.third, tuple2.third, H / B);
			}

			curLCP = tuple1.second + prefixLen - *(*saReader);

				
			if (2746465 == *(*saReader)) {
				print_uint(tuple1.third);

				print_uint(tuple2.third);

				std::cerr << "curSA = 2746465 i: " << i << " j: " << j << " curLCP: " << curLCP << " pa1: " << tuple1.second << " pa2: " << tuple2.second << " prefixLen: " << prefixLen << " sa: " << *(*saReader) << std::endl;

			}
			if (116508 == tuple1.first) {
				print_uint(tuple1.third);

				print_uint(tuple2.third);

				std::cerr << "first == 116508 i: " << i << " j: " << j << " curLCP: " << curLCP << " pa1: " << tuple1.second << " pa2: " << tuple2.second << " prefixLen: " << prefixLen << " sa: " << *(*saReader) << std::endl;

			}

			
			++(*ascSorterD1);

			++(*ascSorterD2);

			++j;
		}	
				
		(*lcpWriter) << curLCP;
	}

	(*lcpWriter).finish();

	delete lcpReader; lcpReader = NULL; delete mLCP1; mLCP1 = NULL;

	delete lcpWriter; lcpWriter = NULL; delete mLCP2; mLCP2 = NULL; delete mLCPFile; mLCPFile = NULL;

	delete saReader; saReader = NULL; delete mSA; mSA = NULL; delete mSAFile; mSAFile = NULL;

	assert(ascSorterD1->empty() == true); 

	delete ascSorterD1; ascSorterD1 = NULL;

	assert(ascSorterD2->empty() == true); 

	delete ascSorterD2; ascSorterD2 = NULL;

	return ;						
}

template<typename T, uint8 B, uint16 H>
void KLCPABuilder<T, B, H>::print_uint(uint_type _item) {

	uint8 buf[H/B];
	
	for (size_t i = 0; i < H / B; ++i) {
		
		buf[i] = _item & OFFSET[H / B - 1];

		_item >>= B;
	}

	for (uint16 i = 0; i < H / B; ++i){
		
		std::cerr << (uint32)(buf[H / B - 1 - i]) << " ";
	}

	std::cerr << std::endl;

}


//! check K-order LCP array using the brute force method

//! \param T type of elements in LCP arrays
template<typename T>
class KLCPABruteForceChecker{
private:

	typedef typename ExVector<T>::vector t_vector_type;

	typedef typename ExVector<uint16>::vector uint16_vector_type;
	
public:
	KLCPABruteForceChecker(std::string & _lcpFileName1, std::string & _lcpFileName2, std::string & _saFileName);

};

template<typename T>
KLCPABruteForceChecker<T>::KLCPABruteForceChecker(std::string & _srcLCPFileName, std::string & _refLCPFileName, std::string & _saFileName){

	bool isRight = true;

	stxxl::syscall_file *srcLCPFile = new stxxl::syscall_file(_srcLCPFileName,
		stxxl::syscall_file::RDWR | stxxl::syscall_file::DIRECT);

	uint16_vector_type *srcLCP = new uint16_vector_type(srcLCPFile);

	typename uint16_vector_type::bufreader_type * srcLCPReader = new typename uint16_vector_type::bufreader_type(*srcLCP);	

	stxxl::syscall_file *refLCPFile= new stxxl::syscall_file(_refLCPFileName,
		stxxl::syscall_file::RDWR | stxxl::syscall_file::DIRECT);	

	t_vector_type * refLCP = new t_vector_type(refLCPFile);

	typename t_vector_type::bufreader_type * refLCPReader = new typename t_vector_type::bufreader_type(*refLCP);

	stxxl::syscall_file *saFile = new stxxl::syscall_file(_saFileName, 
		stxxl::syscall_file::RDWR | stxxl::syscall_file::DIRECT);

	t_vector_type *sa = new t_vector_type(saFile);

	typename t_vector_type::bufreader_type *saReader = new typename t_vector_type::bufreader_type(*sa);

	++(*srcLCPReader), ++(*refLCPReader), ++(*saReader); //skip lcp[0]

	for (size_t i = 1 ; !saReader->empty(); ++(*srcLCPReader), ++(*refLCPReader), ++(*saReader), ++i) {	
		
		if (*(*srcLCPReader) != *(*refLCPReader)) {

			if (*(*srcLCPReader) != K) {

				std::cerr << "i: " << i << " klcp: " << *(*srcLCPReader) << " lcp: " << *(*refLCPReader) << " sa: " << *(*saReader) << std::endl;
						
				std::cerr << "wrong";
				
				isRight = false;

				break;
			}
		}
	}

	if (isRight) {

		std::cerr << "check passed!\n";
	}	
	else {
			
		std::cerr << "check failed!\n";
	}

	delete refLCPReader; refLCPReader = NULL;

	delete refLCP; refLCP = NULL;

	delete refLCPFile; refLCPFile = NULL;	

	delete srcLCPReader; srcLCPReader = NULL;

	delete srcLCP; srcLCP = NULL;

	delete srcLCPFile; srcLCPFile = NULL;

	delete saReader; saReader = NULL;
	
	delete sa; sa = NULL;

	delete saFile; saFile = NULL;

	return ;
}
	




NAMESPACE_KLCP_EM_END

#endif  //KLCPA_EM_H
