#ifndef LCPA_MPI_H
#define LCPA_MPI_H

#include "../../common/namespace.h"
#include "../../common/common.h"
#include "../../common/data.h"

#include "mpi.h"

#define STATISTICS_COLLECTION 

#define DEBUG_MPI

NAMESPACE_LCP_MPI_BEG


//! check full-order LCP array using the proposed probabilistic method

//! \param T element type of SA and LCP arrays
template<typename T>
class LCPAChecker{

private:
	
	typedef typename ExVector<T>::vector t_vector_type; 
	
	typedef typename ExVector<uint8>::vector uint8_vector_type;

	std::string & mSFileName;

	std::string & mSAFileName;

	std::string & mLCPFileName;

	stxxl::syscall_file * mSFile;

	stxxl::syscall_file * mSAFile;

	stxxl::syscall_file * mLCPFile;

	uint8_vector_type * mS;

	t_vector_type * mSA;

	t_vector_type * mLCP;

	size_t mLen;	

	FPTYPE_A * mR;

	size_t mRSize;

	typedef Pair<T, T> TupleA;

	typedef TupleLessComparator1<TupleA> TupleALessComparator1; //!< sort by first component

	typedef typename ExTupleAscSorter<TupleA, TupleALessComparator1>::sorter AscSorterA1;

	typedef typename ExTupleAscSorter<TupleA, TupleALessComparator1>::comparator AscComparatorA1;
	
	typedef TupleLessComparator2<TupleA> TupleALessComparator2; //!< sort by first and second components 

	typedef typename ExTupleAscSorter<TupleA, TupleALessComparator2>::sorter AscSorterA2;

	typedef typename ExTupleAscSorter<TupleA, TupleALessComparator2>::comparator AscComparatorA2;

	typedef Pair<T, FPTYPE_A> TupleB; 

	typedef TupleLessComparator1<TupleB> TupleBLessComparator; //!< sort by first key

	typedef typename ExTupleAscSorter<TupleB, TupleBLessComparator>::sorter AscSorterB;

	typedef typename ExTupleAscSorter<TupleB, TupleBLessComparator>::comparator AscComparatorB;

	typedef Triple<T, FPTYPE_A, uint16> TupleC; //!< use std::numeric_limits<uint16>::max() to denote a sentinel, thus we expand uint8 to uint16

	typedef TupleLessComparator1<TupleC> TupleCLessComparator; //!< sort by first key

	typedef typename ExTupleAscSorter<TupleC, TupleCLessComparator>::sorter AscSorterC;

	typedef typename ExTupleAscSorter<TupleC, TupleCLessComparator>::comparator AscComparatorC;

	AscSorterB * mAscSorterB;

	AscSorterC * mAscSorterC1;

	AscSorterC * mAscSorterC2;

	int mCommRank; //rank of current node in the comm world

	int mCommSize; //size of the comm world

	template<typename Elem>
	struct Buffer{

		size_t mBufLen; //!< buffer length

		size_t mBufPtr; //!< ptr to leftmost unscanned element

		size_t mBufSize; //!< buffer size / capacity

		Elem *mBuf; //!< ptr to buffer 

		//! ctor
		Buffer(size_t _bufSize) : mBufLen(0), mBufPtr(0), mBufSize(_bufSize) {

			mBuf = new Elem[_bufSize];
		}
		
		//! append a new element to the right side
		void push_back(const Elem & _elem) {
		
			mBuf[mBufLen] = _elem;

			++mBufLen;
		}	

		//! return reference of the element pointed to by mBufPtr
		Elem & operator *() {
			
			return mBuf[mBufPtr];
		}
	
		//! advance to next unscanned element
		void operator++() {
		
			++mBufPtr;
		}

		//! rewind back to the left side
		void rewind() {

			mBufPtr = 0;
		}

		//! reset the buffer pointers without deallocation
		void clear() {
		
			mBufPtr = 0;

			mBufLen = 0;
		}

		//! destroy the buffer
		~Buffer(){
			
			delete [] mBuf; 

			mBuf = NULL;
		}		
	};

public:

	LCPAChecker(std::string & _sFileName, std::string & _saFileName, std::string & _lcpFileName, size_t _len, int _commRank, int _commSize);

	void computeR();

	FPTYPE_A getR(size_t _interval);

	AscSorterB * run1st(AscSorterA1 * _ascSorterA1);

	AscSorterC * run2nd(AscSorterA2 * _ascSorterA2);	

	template<typename Elem, typename AscSorter> bool checkProcedure(AscSorter * _ascSorter);

	bool check(const FPTYPE_A & _fpInterval1, const FPTYPE_A & _fpInterval2, const uint16 _ch1, const uint16 _ch2);
};


//! ctor
template<typename T>
LCPAChecker<T>::LCPAChecker(
	std::string & _sFileName, 
	std::string & _saFileName, 
	std::string & _lcpFileName,
	size_t _len, 
	int _commRank, 
	int _commSize) :
	mSFileName(_sFileName),
	mSAFileName(_saFileName),
	mLCPFileName(_lcpFileName),
	mLen (_len),
	mCommRank(_commRank),
	mCommSize(_commSize) {

	stxxl::config *cfg = stxxl::config::get_instance();

	std::string diskFileName = "/tmp/stxxl.tmp";
	
	diskFileName += std::to_string(mCommRank);

	stxxl::disk_config disk(diskFileName.c_str(), 1024 * 1024 * 1024, "syscall unlink");

	disk.direct = stxxl::disk_config::DIRECT_ON;

	cfg->add_disk(disk);
	
#ifdef STATISTICS_COLLECTION
	stxxl::stats *Stats = stxxl::stats::get_instance();

	stxxl::block_manager *bm = stxxl::block_manager::get_instance();

#endif

	///step 1: preprocess
	//append a sentinel to input string on each node
	mSFile = new stxxl::syscall_file(mSFileName,
		stxxl::syscall_file::RDWR | stxxl::syscall_file::DIRECT);

	mS = new uint8_vector_type(mSFile);

	mS->push_back(static_cast<uint8>(0));

#ifdef DEBUG_MPI

	std::cerr << "mCommRank: " << mCommRank << " mSLen: " << mS->size() << std::endl;

#endif //DEBUG_MPI

	delete mS; mS = NULL;

	//append a sentinel to lcp array on each node
	mLCPFile = new stxxl::syscall_file(mLCPFileName,
		stxxl::syscall_file::RDWR | stxxl::syscall_file::DIRECT);

	mLCP = new t_vector_type(mLCPFile);

	mLCP->push_back(static_cast<T>(0));

#ifdef DEBUG_MPI

	std::cerr << "mCommRank: " << mCommRank << " mLCPLen: " << mLCP->size() << std::endl;

#endif //DEBUG_MPI

	delete mLCP; mLCP = NULL;


	///step 2: compute fingerprints on each node
	if (mCommRank == 0) { // sort (sa[idx], idx) by (1st) in ascending order and scan mS to iteratively compute fingerprints.

		mSAFile = new stxxl::syscall_file(mSAFileName,
			stxxl::syscall_file::RDWR | stxxl::syscall_file::DIRECT);

		mSA = new t_vector_type(mSAFile);

		typename t_vector_type::bufreader_type *saReader = new typename t_vector_type::bufreader_type(*mSA);

		AscSorterA1 * ascSorterA1 = new AscSorterA1(AscComparatorA1(), MAX_MEM / 2);

		for (T idx = 1; !saReader->empty(); ++idx, ++(*saReader)) {

			ascSorterA1->push(TupleA(*(*saReader), idx));
		}

		delete saReader; saReader = NULL;

		delete mSA; mSA = NULL;

		mAscSorterB = run1st(ascSorterA1);		
	}
	else if (mCommRank == 1) { // sort (sa[idx] + lcp[idx], idx) by (1st, 2nd) in ascending order and scan mS to iteratively compute fingerprints.

		mSAFile = new stxxl::syscall_file(mSAFileName,
			stxxl::syscall_file::RDWR | stxxl::syscall_file::DIRECT);

		mSA = new t_vector_type(mSAFile);

		typename t_vector_type::bufreader_type *saReader = new typename t_vector_type::bufreader_type(*mSA);

		mLCP = new t_vector_type(mLCPFile);

		typename t_vector_type::bufreader_type *lcpReader = new typename t_vector_type::bufreader_type(*mLCP);

		AscSorterA2 * ascSorterA21 = new AscSorterA2(AscComparatorA2(), MAX_MEM / 2);

		for (T idx = 1; !saReader->empty(); ++idx, ++(*lcpReader), ++(*saReader)) {

			ascSorterA21->push(TupleA(*(*saReader) + *(*lcpReader), idx));
		}

		delete saReader; saReader = NULL; 
		
		delete mSA; mSA = NULL;

		delete lcpReader; lcpReader = NULL; 
		
		delete mLCP; mLCP = NULL;

		mAscSorterC1 = run2nd(ascSorterA21);
	}
	else { // sort (sa[idx] + lcp[idx + 1], idx) by (1st, 2nd) in ascending order and scan mS to iteratively compute fingerprints
			
		mSAFile = new stxxl::syscall_file(mSAFileName,
			stxxl::syscall_file::RDWR | stxxl::syscall_file::DIRECT);

		mSA = new t_vector_type(mSAFile);

		typename t_vector_type::bufreader_type * saReader = new typename t_vector_type::bufreader_type(*mSA);

		mLCP = new t_vector_type(mLCPFile);

		typename t_vector_type::bufreader_type * lcpReader = new typename t_vector_type::bufreader_type(*mLCP);

		AscSorterA2 * ascSorterA22 = new AscSorterA2(AscComparatorA2(), MAX_MEM / 2);

		++(*lcpReader);

		for (T idx = 1; !saReader->empty(); ++idx, ++(*lcpReader), ++(*saReader)) {

			ascSorterA22->push(TupleA(*(*saReader) + *(*lcpReader), idx));
		}

		delete saReader; saReader = NULL; 
		
		delete mSA; mSA = NULL;

		delete lcpReader; lcpReader = NULL; 
		
		delete mLCP; mLCP = NULL;

		mAscSorterC2 = run2nd(ascSorterA22);
	}


	///step 3: check 
	bool isRight = true;

	switch (mCommRank) {
	case 0: 

		isRight = checkProcedure<TupleB, AscSorterB>(mAscSorterB);
		
		break;

	case 1:
		isRight = checkProcedure<TupleC, AscSorterC>(mAscSorterC1);

		break;

	case 2:
		isRight = checkProcedure<TupleC, AscSorterC>(mAscSorterC2);

		break;

	default:
			
		;

	}


	int res = 0; 

	MPI_Reduce(&isRight, &res, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

	if (mCommRank == 0) {
	
		if (res == 3) {
			
			std::cerr << "check passed\n";
		}
		else {
		
			std::cerr << "check failed\n";
		}
	}


	///step 4: post-process
	//resize mS
	mS = new uint8_vector_type(mSFile);

	mS->resize(mLen);

	delete mS; mS = NULL; 
	
	//resize mLCP
	mLCP = new t_vector_type(mLCPFile);

	mLCP->resize(mLen);

	delete mLCP; mLCP = NULL; 
	
	//clear
	delete mSFile; mSFile = NULL;

	delete mLCPFile; mLCPFile = NULL;

	delete mSAFile; mSAFile = NULL;


#ifdef STATISTICS_COLLECTION
	std::cout << "mCommRank: " << mCommRank << " I/O Volume: " << (Stats->get_written_volume() + Stats->get_read_volume()) / mLen << std::endl;

	std::cout << "mCommRank: " << mCommRank << " Peak Disk Usage: " << bm->get_maximum_allocation() / mLen << std::endl;	
#endif

	return ;
}

//! compute rank array

template<typename T>
void LCPAChecker<T>::computeR() {
	
	mRSize = 1;

	size_t tmpLen = mLen;
	
	while (tmpLen) {
	
		++mRSize;	

		tmpLen /= 2;
	}

	mR = new FPTYPE_A[mRSize];

	mR[0] = R % P; //r^1

	for (size_t i = 1; i < mRSize; ++i) {

		mR[i] = static_cast<FPTYPE_A>((static_cast<FPTYPE_B>(mR[i - 1]) * mR[i - 1]) % P);
	}

	return;
}


template<typename T>
inline FPTYPE_A LCPAChecker<T>::getR(size_t _interval) {

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


template<typename T>
typename LCPAChecker<T>::AscSorterB * LCPAChecker<T>::run1st(AscSorterA1 * _ascSorter) {
		
	_ascSorter->sort();	

#ifdef DEBUG_MPI

	if (mCommRank == 0) {

		std::cerr << "mCommRank: " << mCommRank << " ascSorter->size(): " << _ascSorter->size() << std::endl;
	}		

#endif //DEBUG_MPI

	mS = new uint8_vector_type(mSFile);

	typename uint8_vector_type::bufreader_type *sReader = new typename uint8_vector_type::bufreader_type(*mS);

	AscSorterB *ascSorterB = new AscSorterB(AscComparatorB(), MAX_MEM / 2);

	FPTYPE_A fp = 0;

	for ( ; !_ascSorter->empty(); ++(*_ascSorter), ++(*sReader)) {

		const TupleA &tuple = *(*_ascSorter);

		ascSorterB->push(TupleB(tuple.second, fp)); //fp = FP(0, tuple.first - 1)
		
		fp = static_cast<FPTYPE_A>((static_cast<FPTYPE_B>(fp) * R + (*(*sReader) + 1)) % P);		
	}

	delete sReader; sReader = NULL; 
	
	delete mS; mS = NULL;

	delete _ascSorter; _ascSorter = NULL;

	ascSorterB->sort();

#ifdef DEBUG_MPI

	if (mCommRank == 0) {

		std::cerr << "mCommRank: " << mCommRank << " ascSorterB->size(): " << ascSorterB->size() << std::endl;
	}

#endif //DEBUG_MPI

	return ascSorterB;
}


template<typename T>
typename LCPAChecker<T>::AscSorterC * LCPAChecker<T>::run2nd(AscSorterA2 * _ascSorter) {
		
	_ascSorter->sort();	

#ifdef DEBUG_MPI

	if (mCommRank == 0) {

		std::cerr << "mCommRank: " << mCommRank << " ascSorter->size(): " << _ascSorter->size() << std::endl;
	}
		
#endif //DEBUG_MPI

	mS = new uint8_vector_type(mSFile);

	typename uint8_vector_type::bufreader_type *sReader = new typename uint8_vector_type::bufreader_type(*mS);

	AscSorterC *ascSorterC = new AscSorterC(AscComparatorC(), MAX_MEM / 2);

	FPTYPE_A fp = 0;

	uint16 ch;
	
	for (T idx = 0; !_ascSorter->empty(); ++(*_ascSorter)) {

		const TupleA &tuple = *(*_ascSorter);

		while (idx != tuple.first) {
			
			fp = static_cast<FPTYPE_A>((static_cast<FPTYPE_B>(fp) * R + (*(*sReader) + 1)) % P);		

			++(*sReader);

			++idx;
		}
		
		ch = (mLen == idx) ? std::numeric_limits<uint16>::max() : (*(*sReader)); 

		ascSorterC->push(TupleC(tuple.second, fp, ch));//fp = FP(0, tuple.first - 1), ch = mS[tuple.first] 	
	}

	delete sReader; sReader = NULL; 
	
	delete mS; mS = NULL;

	delete _ascSorter; _ascSorter = NULL;

	ascSorterC->sort();

#ifdef DEBUG_MPI

	if (mCommRank == 0) {

		std::cerr << "mCommRank: " << mCommRank << " ascSorterC->size(): " << ascSorterC->size() << std::endl;
	}

#endif //DEBUG_MPI

	return ascSorterC;
}


//! split data into partitions and check {0 + i, 3 + i, 6 + i, 9 + i, ...}-th partitions on node i

template<typename T>
template<typename Elem, typename AscSorter>
bool LCPAChecker<T>::checkProcedure(AscSorter * _ascSorter) {

	///step 1: compute R array
	computeR();
	

	///step 2: pre-process
	int isRight = 1; //if right, 1; otherwise 0.

	//size_t recvBufSize = (MAX_MEM / 2) / 6 / std::max(sizeof(TupleB), sizeof(TupleC));

	size_t recvBufSize = (MAX_MEM / 10) / 6 / std::max(sizeof(TupleB), sizeof(TupleC));

	size_t sendBufSize = recvBufSize * 3;
	
	Buffer<Elem> *sendBuf = new Buffer<Elem>(sendBufSize + 3); //sendBufSize + 3

	Buffer<TupleB> *recvBufB = new Buffer<TupleB>(recvBufSize + 1); //recvBufSize + 1

	Buffer<TupleC> *recvBufC1 = new Buffer<TupleC>(recvBufSize + 1); //recvBufSize + 1

	Buffer<TupleC> *recvBufC2 = new Buffer<TupleC>(recvBufSize + 1); //recvBufSize + 1

	size_t toSend = mLen, sending, avgSending, sent = 0; 

	FPTYPE_A fpInterval1, fpInterval2;
	
	uint16 ch1, ch2;

	bool isFirst = true;
	
	Elem preElem;

	T curLCP;
	
	
#ifdef DEBUG_MPI
	

	if (mCommRank == 0) {

		std::cerr << "mCommRank: " << mCommRank << " recvBufSize: " << recvBufSize << " sendBufSize: " << sendBufSize << " toSend: " << toSend << std::endl;
	}

#endif


	while (toSend) {

		sending = (toSend > sendBufSize) ? sendBufSize : toSend;

		avgSending = sending / 3 + ((sending % 3) ? 1 : 0);

#ifdef DEBUG_MPI

	if (mCommRank == 0) {

		std::cerr << "mCommRank: " << mCommRank << " sending: " << sending << " avgSending: " << avgSending << std::endl;
	}

#endif

		//compute sCount (number of elements destined for each node) and startOffset (global startOffset)
		int sCount[3];

		size_t startOffset[3];

		sCount[0] = avgSending;

		startOffset[0] = sent;

		sCount[1] = (sending >= 2 * avgSending) ? avgSending : (sending - avgSending);

		startOffset[1] = sent + sCount[0];

		sCount[2] = sending - sCount[0] - sCount[1];

		startOffset[2] = sent + sCount[0] + sCount[1];

		
#ifdef DEBUG_MPI
			
	if (mCommRank == 0) {

		std::cerr << "mCommRank: " << mCommRank << " sCount: " << sCount[0] << " " << sCount[1] << " " << sCount[2] << std::endl;

		std::cerr << "mCommRank: " << mCommRank << " startOffset: " << startOffset[0] << " " << startOffset[1] << " " << startOffset[2] << std::endl;
	}

#endif

		//borrow one element from previous partition, thus plus 1 for sCount
		sCount[0] = sCount[0] + 1; 

		sCount[1] = (sCount[1] > 0) ? (sCount[1] + 1) : 0; 

		sCount[2] = (sCount[2] > 0) ? (sCount[2] + 1) : 0;

#ifdef DEBUG_MPI
	
	if (mCommRank == 0) {

		std::cerr << "mCommRank: " << mCommRank << " sCount with a borrow element: " << sCount[0] << " " << sCount[1] << " " << sCount[2] << std::endl;
	}

#endif

		//compute sDispl (local displacement in sending buffer)
		int sDispl[3]; 

		sDispl[0] = 0;

		sDispl[1] = sCount[0];

		sDispl[2] = sCount[0] + sCount[1];
		

#ifdef DEBUG_MPI

	if (mCommRank == 0) {

		std::cerr << "mCommRank: " << mCommRank << " sDispl with a borrow element: " << sDispl[0] << " " << sDispl[1] << " " << sDispl[2] << std::endl;
	}

#endif

		//pack data into sending buffer
		sendBuf->clear(); //clear bufPtr and bufLen
	
		for (int i = 0; i < 3; ++i) { //pack 3 successive partitions
	
#ifdef DEBUG_MPI
			
			if (mCommRank == 0) {

				std::cerr << "mCommRank: " << mCommRank << " beg partition " << i << " sendBuf length: " << sendBuf->mBufLen << std::endl;
			}
#endif

			if (sCount[i]) { //check if 0

				sendBuf->push_back(preElem); //borrow an item from previous partition

				for (size_t j = 1; j < sCount[i] - 1; ++j, ++(*_ascSorter)) { //load next (sCount[i] - 2) elements

					sendBuf->push_back(*(*_ascSorter));
				}

				sendBuf->push_back(*(*_ascSorter)); //load final element

				preElem = *(*_ascSorter); //borrowed by the next partition

				++(*_ascSorter);
			}
			else {

				break;
			}
#ifdef DEBUG_MPI

			if (mCommRank == 0) {
		
				std::cerr << "mCommRank: " << mCommRank << " end partition " << i << " sendBuf length: " << sendBuf->mBufLen << std::endl;
			}

#endif

		}
		
		//sCount and sDispl in char for MPI (or use boost::serialize and boost::MPI instead)
		int sCountB[3]; //TupleB, in char

		int sDisplB[3]; //TupleB, in char

		int sCountC[3]; //TupleC, in char

		int sDisplC[3]; //TupleC, in char

		for (int i = 0; i < 3; ++i) {
			
			sCountB[i] = sCount[i] * sizeof(TupleB) / sizeof(char);

			sDisplB[i] = sDispl[i] * sizeof(TupleB) / sizeof(char);

			sCountC[i] = sCount[i] * sizeof(TupleC) / sizeof(char);

			sDisplC[i] = sDispl[i] * sizeof(TupleC) / sizeof(char);
		}

#ifdef DEBUG_MPI
		if (mCommRank == 0) {

			std::cerr << "mCommRank: " << mCommRank << " sCountB: " << sCountB[0] << " " << sCountB[1] << " " << sCountB[2] << std::endl;

			std::cerr << "mCommRank: " << mCommRank << " sDisplB: " << sDisplB[0] << " " << sDisplB[1] << " " << sDisplB[2] << std::endl;
	
			std::cerr << "mCommRank: " << mCommRank << " sCountC: " << sCountC[0] << " " << sCountC[1] << " " << sCountC[2] << std::endl;
	
			std::cerr << "mCommRank: " << mCommRank << " sDisplC: " << sDisplC[0] << " " << sDisplC[1] << " " << sDisplC[2] << std::endl;
		}
#endif

		//scatter bufferB from node 0 to all nodes
		recvBufB->clear();

		MPI_Scatterv(reinterpret_cast<char*>(sendBuf->mBuf), sCountB, sDisplB, MPI_CHAR,
			reinterpret_cast<char*>(recvBufB->mBuf), sCountB[mCommRank], MPI_CHAR, 0, MPI_COMM_WORLD);

		//scatter bufferC1 from node 1 to all nodes
		recvBufC1->clear();

		MPI_Scatterv(reinterpret_cast<char*>(sendBuf->mBuf), sCountC, sDisplC, MPI_CHAR,
			reinterpret_cast<char*>(recvBufC1->mBuf), sCountC[mCommRank], MPI_CHAR, 1, MPI_COMM_WORLD);


		//scatter bufferC2 from node 2 to all nodes
		recvBufC2->clear();

		MPI_Scatterv(reinterpret_cast<char*>(sendBuf->mBuf), sCountC, sDisplC, MPI_CHAR,
			reinterpret_cast<char*>(recvBufC2->mBuf), sCountC[mCommRank], MPI_CHAR, 2, MPI_COMM_WORLD);

		//check a partition on each node
		if (sCount[mCommRank]){

			mLCP = new t_vector_type(mLCPFile);

			typename t_vector_type::const_iterator *it = NULL;

			size_t idx = 0;

			//process first element
			if (isFirst && mCommRank == 0) { //the leftmost element of the first partition is invalid
				
				isFirst = false;

				++(*recvBufB), ++(*recvBufC1), ++(*recvBufC2), ++idx;

				it = new typename t_vector_type::const_iterator();

				*it = mLCP->begin() + startOffset[mCommRank] + 1;
			}
			else {

				it = new typename t_vector_type::const_iterator();

				*it = mLCP->begin() + startOffset[mCommRank];
			}

			curLCP = *(*it);

			fpInterval2 = static_cast<FPTYPE_A>(((*(*recvBufC2)).second -
				(static_cast<FPTYPE_B>((*(*recvBufB)).second) * getR(curLCP)) % P + P) % P);
				
			ch2 = (*(*recvBufC2)).third;

			++(*recvBufB), ++(*recvBufC1), ++(*recvBufC2), ++(*it), ++idx;

			//remaining
			for (; idx < sCount[mCommRank]; ++(*recvBufB), ++(*recvBufC1), ++(*recvBufC2), ++(*it), ++idx) {

				fpInterval1 = static_cast<FPTYPE_A>(((*(*recvBufC1)).second -
					(static_cast<FPTYPE_B>((*(*recvBufB)).second) * getR(curLCP)) % P + P) % P);

				ch1 = (*(*recvBufC1)).third;

				if (!check(fpInterval1, fpInterval2, ch1, ch2)) {
					
					std::cerr << "----match failed. fpInterval1: " << fpInterval1 << " fpInterval2: " << fpInterval2 << " ch1: " << ch1 << " ch2: " << ch2 << std::endl;	
					
					std::cerr << "idx: " << idx << std::endl;
					std::cerr << "recvBufB idx: " << (*(*recvBufB)).first << " fp: " << (*(*recvBufB)).second << std::endl;

					std::cerr << "recvBufC1 idx: " << (*(*recvBufC1)).first << " fp: " << (*(*recvBufC1)).second << " ch1:" << (*(*recvBufC1)).third << std::endl;

					std::cerr << "recvBufC2 idx: " << (*(*recvBufC2)).first << " fp: " << (*(*recvBufC2)).second << " ch2:" << (*(*recvBufC2)).third << std::endl;
				
					isRight = false;

					break;
				}

				curLCP = *(*it);

				fpInterval2 = static_cast<FPTYPE_A>(((*(*recvBufC2)).second -
					(static_cast<FPTYPE_B>((*(*recvBufB)).second) * getR(curLCP)) % P + P) % P);

				ch2 = (*(*recvBufC2)).third;
			}
		
			delete it; it = NULL;

			delete mLCP; mLCP = NULL;
		}

		toSend -= sending;

		sent += sending;
	}


	delete sendBuf; sendBuf = NULL;

	delete recvBufB; recvBufB = NULL;

	delete recvBufC1; recvBufC1 = NULL;

	delete recvBufC2; recvBufC2 = NULL;

	delete _ascSorter; _ascSorter = NULL;

	return isRight;
}


template<typename T>
inline bool LCPAChecker<T>::check(const FPTYPE_A & _fpInterval1, const FPTYPE_A & _fpInterval2, const uint16 _ch1, const uint16 _ch2) {

	bool isRight = true;

	if (_fpInterval1 == _fpInterval2) {

		if (_ch1 == _ch2 && _ch1 != std::numeric_limits<uint16>::max()) {

			isRight = false;
		}
	}
	else {

		isRight = false;
	}

	return isRight;
}

NAMESPACE_LCP_MPI_END

#endif // LCPA_MPI_H
