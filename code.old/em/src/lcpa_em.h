#ifndef LCPA_EM_H
#define LCPA_EM_H

#include "../../common/namespace.h"
#include "../../common/common.h"
#include "../../common/data.h"


#define STATISTICS_COLLECTION 

NAMESPACE_LCP_EM_BEG


//! build full-order LCP array

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

	size_t mLen;	

	t_vector_type * mSA;

	t_vector_type * mLCP;

	uint8_vector_type * mS;
	
	FPTYPE_A * mR;

	size_t mRSize;

	typedef Pair<T, T> TupleA;

	typedef TupleLessComparator1<TupleA> TupleALessComparator1; //! key <1st>

	typedef typename ExTupleAscSorter<TupleA, TupleALessComparator1>::sorter AscSorterA1;

	typedef typename ExTupleAscSorter<TupleA, TupleALessComparator1>::comparator AscComparatorA1;
	
	typedef TupleLessComparator2<TupleA> TupleALessComparator2; // ! key <1st, 2nd>

	typedef typename ExTupleAscSorter<TupleA, TupleALessComparator2>::sorter AscSorterA2;

	typedef typename ExTupleAscSorter<TupleA, TupleALessComparator2>::comparator AscComparatorA2;

	typedef Pair<T, FPTYPE_A> TupleB; 

	typedef TupleLessComparator1<TupleB> TupleBLessComparator; //! key <1st>

	typedef typename ExTupleAscSorter<TupleB, TupleBLessComparator>::sorter AscSorterB;

	typedef typename ExTupleAscSorter<TupleB, TupleBLessComparator>::comparator AscComparatorB;

	typedef Triple<T, FPTYPE_A, uint16> TupleC;

	typedef TupleLessComparator1<TupleC> TupleCLessComparator; //! key <1st>

	typedef typename ExTupleAscSorter<TupleC, TupleCLessComparator>::sorter AscSorterC;

	typedef typename ExTupleAscSorter<TupleC, TupleCLessComparator>::comparator AscComparatorC;

public:

	LCPAChecker(std::string & _sFileName, std::string & _saFileName, std::string & _lcpFileName, size_t _len);

	void computeR();

	FPTYPE_A getR(size_t _interval);

	AscSorterB * run1st(AscSorterA1 * _ascSorterA1);

	AscSorterC * run2nd(AscSorterA2 * _ascSorterA2);	

	bool check(AscSorterB * _ascSorterB, AscSorterC * _ascSorterC1, AscSorterC * _ascSorterC2);
};


template<typename T>
LCPAChecker<T>::LCPAChecker(
	std::string & _sFileName, 
	std::string & _saFileName, 
	std::string & _lcpFileName, 
	size_t _len) : 
	mSFileName(_sFileName),
	mSAFileName(_saFileName),
	mLCPFileName(_lcpFileName),
	mLen (_len) {
	
	
#ifdef STATISTICS_COLLECTION
	stxxl::stats *Stats = stxxl::stats::get_instance();

	stxxl::block_manager *bm = stxxl::block_manager::get_instance();

#endif
	

	///step 1: pre-process
	//compute mR
	computeR();

	//append a sentinel to input string
	mSFile = new stxxl::syscall_file(mSFileName,
		stxxl::syscall_file::RDWR | stxxl::syscall_file::DIRECT);

	mS = new uint8_vector_type(mSFile);

	mS->push_back(static_cast<uint8>(0));
	
	std::cerr << "mSLen: " << mS->size() << std::endl;

	delete mS; mS = NULL;
	
	//append a sentinel to lcp array
	mLCPFile = new stxxl::syscall_file(mLCPFileName,
		stxxl::syscall_file::RDWR | stxxl::syscall_file::DIRECT);

	mLCP = new t_vector_type(mLCPFile);

	mLCP->push_back(static_cast<T>(0));

	std::cerr << "mLCPLen: " << mLCP->size() << std::endl;

	delete mLCP; mLCP = NULL;


	///step 2: compute fp
	//<sa, idx>
	mSAFile = new stxxl::syscall_file(mSAFileName,
		stxxl::syscall_file::RDWR | stxxl::syscall_file::DIRECT);

	mSA = new t_vector_type(mSAFile);	
		
	typename t_vector_type::bufreader_type *saReader = new typename t_vector_type::bufreader_type(*mSA);
		
	AscSorterA1 * ascSorterA1 = new AscSorterA1(AscComparatorA1(), MAX_MEM / 4);
	
	for (T idx = 1; !saReader->empty(); ++idx, ++(*saReader)) {
		
		ascSorterA1->push(TupleA(*(*saReader), idx));	
	}	
		
	delete saReader; saReader = NULL;
	
	delete mSA; mSA = NULL;

	AscSorterB * ascSorterB = run1st(ascSorterA1);

	//<sa + lcp1, idx>
	mSA = new t_vector_type(mSAFile);	

	saReader = new typename t_vector_type::bufreader_type(*mSA);

	mLCP = new t_vector_type(mLCPFile);

	typename t_vector_type::bufreader_type *lcpReader = new typename t_vector_type::bufreader_type(*mLCP);
		
	AscSorterA2 * ascSorterA21 = new AscSorterA2(AscComparatorA2(), MAX_MEM / 4);
	
	for (T idx = 1; !saReader->empty(); ++idx, ++(*lcpReader), ++(*saReader)) {
	
		ascSorterA21->push(TupleA(*(*saReader) + *(*lcpReader), idx));	
	}

	delete saReader; saReader = NULL; delete mSA; mSA = NULL;

	delete lcpReader; lcpReader = NULL; delete mLCP; mLCP = NULL;
	
	AscSorterC * ascSorterC1 = run2nd(ascSorterA21);
	
	//<sa + lcp2, idx>
	mSA = new t_vector_type(mSAFile);	
	
	saReader = new typename t_vector_type::bufreader_type(*mSA);

	mLCP = new t_vector_type(mLCPFile);

	lcpReader = new typename t_vector_type::bufreader_type(*mLCP);

	AscSorterA2 * ascSorterA22 = new AscSorterA2(AscComparatorA2(), MAX_MEM / 4);
	
	++(*lcpReader);

	for (T idx = 1; !saReader->empty(); ++idx, ++(*lcpReader), ++(*saReader)) {
		
		ascSorterA22->push(TupleA(*(*saReader) + *(*lcpReader), idx));
	}
	
	delete saReader; saReader = NULL; delete mSA; mSA = NULL;

	delete lcpReader; lcpReader = NULL; delete mLCP; mLCP = NULL;

	AscSorterC * ascSorterC2 = run2nd(ascSorterA22);


	///step 3: merge to compute and compare <fpInterval, ch>
	bool isRight = check(ascSorterB, ascSorterC1, ascSorterC2);

	if (isRight) {
		
		std::cerr << "check passed\n";
	}
	else {
	
		std::cerr << "check failed\n";
	}

	///step 4: resize mS and mLCP
	mS = new uint8_vector_type(mSFile);

	mS->resize(mLen);

	delete mS; mS = NULL; delete mSFile; mSFile = NULL;

	mLCP = new t_vector_type(mLCPFile);

	mLCP->resize(mLen);

	delete mLCP; mLCP = NULL; delete mLCPFile; mLCPFile = NULL;

	delete mSAFile; mSAFile = NULL;


#ifdef STATISTICS_COLLECTION
	std::cout << "I/O Volume: " << (Stats->get_written_volume() + Stats->get_read_volume()) / mLen << std::endl;

	std::cout << "Peak Disk Usage: " << bm->get_maximum_allocation() / mLen << std::endl;	
#endif

	

	return ;
}

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

	return ;
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

	std::cerr << "ascSorter->size(): " << _ascSorter->size() << std::endl;
		
	mS = new uint8_vector_type(mSFile);

	typename uint8_vector_type::bufreader_type *sReader = new typename uint8_vector_type::bufreader_type(*mS);

	AscSorterB *ascSorterB = new AscSorterB(AscComparatorB(), MAX_MEM / 4);

	FPTYPE_A fp = 0;

	for ( ; !_ascSorter->empty(); ++(*_ascSorter), ++(*sReader)) {

		const TupleA &tuple = *(*_ascSorter);

		ascSorterB->push(TupleB(tuple.second, fp));
		
		fp = static_cast<FPTYPE_A>((static_cast<FPTYPE_B>(fp) * R + (*(*sReader) + 1)) % P);		
	}

	delete sReader; sReader = NULL; delete mS; mS = NULL;

	delete _ascSorter; _ascSorter = NULL;

	ascSorterB->sort();

	std::cerr << "ascSorterB->size(): " << ascSorterB->size() << std::endl;

	return ascSorterB;
}


template<typename T>
typename LCPAChecker<T>::AscSorterC * LCPAChecker<T>::run2nd(AscSorterA2 * _ascSorter) {
		
	_ascSorter->sort();	

	std::cerr << "ascSorter->size(): " << _ascSorter->size() << std::endl;
		
	mS = new uint8_vector_type(mSFile);

	typename uint8_vector_type::bufreader_type *sReader = new typename uint8_vector_type::bufreader_type(*mS);

	AscSorterC *ascSorterC = new AscSorterC(AscComparatorC(), MAX_MEM / 4);

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

		ascSorterC->push(TupleC(tuple.second, fp, ch));
		
	}

	delete sReader; sReader = NULL; delete mS; mS = NULL;

	delete _ascSorter; _ascSorter = NULL;

	ascSorterC->sort();

	std::cerr << "ascSorterC->size(): " << ascSorterC->size() << std::endl;

	return ascSorterC;
}


template<typename T>
bool LCPAChecker<T>::check(AscSorterB * _ascSorterB, AscSorterC * _ascSorterC1, AscSorterC * _ascSorterC2) {
	
	bool isRight = true;

	mLCP = new t_vector_type(mLCPFile);

	typename t_vector_type::bufreader_type *lcpReader = new typename t_vector_type::bufreader_type(*mLCP);

	FPTYPE_A fpInterval1, fpInterval2;

	uint16 ch1, ch2;

	T curLCP;
	
	//
	++(*_ascSorterC1); 

	++(*lcpReader); 	

	curLCP = *(*lcpReader);

	fpInterval2 = static_cast<FPTYPE_A>(((*_ascSorterC2)->second - (static_cast<FPTYPE_B>((*_ascSorterB)->second) * getR(curLCP)) % P + P) % P);

	ch2 = (*_ascSorterC2)->third;

	++(*_ascSorterB), ++(*_ascSorterC2);	

	for (size_t idx = 1 ; !_ascSorterC1->empty(); ++idx, ++(*_ascSorterC1), ++(*_ascSorterB), ++(*_ascSorterC2)) {
		
		fpInterval1 = static_cast<FPTYPE_A>(((*_ascSorterC1)->second - (static_cast<FPTYPE_B>((*_ascSorterB)->second) * getR(curLCP)) % P + P) % P);
	
		ch1 = (*_ascSorterC1)->third;

		if (fpInterval1 == fpInterval2) {
			
			if (ch1 == ch2 && ch1 != std::numeric_limits<uint16>::max()) {
				
				isRight = false;
				
				break;
			}
		}
		else {

			std::cerr << "idx: " << idx << " fpInterval1: " << fpInterval1 << " fpInterval2: " << fpInterval2 << std::endl;

			isRight = false;

			break;
		}
	
		++(*lcpReader);

		curLCP = *(*lcpReader);
		
		fpInterval2 = static_cast<FPTYPE_A>(((*_ascSorterC2)->second - (static_cast<FPTYPE_B>((*_ascSorterB)->second) * getR(curLCP)) % P + P) % P); 
	
		ch2 = (*_ascSorterC2)->third;
	}

	return isRight; 
}


NAMESPACE_LCP_EM_END

#endif // LCPA_EM_H
