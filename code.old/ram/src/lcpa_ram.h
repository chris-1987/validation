#ifndef LCPA_RAM_H
#define LCPA_RAM_H

#include "../../common/namespace.h"
#include "../../common/common.h"
#include "../../common/data.h"

NAMESPACE_LCP_RAM_BEG

//! check lcp array

//! \param T alphabet type of input string
//! element type of LCP and SA is uint32 by default
template<typename T>
class LCPAChecker{
private:

	//alias
	typedef typename ExVector<T>::vector t_vector_type;

	typedef typename ExVector<uint32>::vector uint32_vector_type;

	//member
	T * mS; //!< ptr to input string

	uint32 * mSA; //!< ptr to input suffix array
	
	uint32 * mLCP; //!< ptr to input lcp array
	
	uint32 mLen; //!< length of input string

	uint32 mMaxLCP; //!< maximum lcp value of mLCP

	FPTYPE_A * mRInterval; //!< ptr to range interval array. mRInterval[i] = mRInterval[i - 1] * R mod P;

	FPTYPE_A * mFP; //!< ptr to fingerprint array

public:
	
	//! ctor
	LCPAChecker(std::string & _sFileName, std::string & _saFileName, std::string & _lcpFileName, uint32 _len, uint32 _maxLCP);

	//! lcp checking entrance
	bool run();
};

//! ctor for LCPAChecker

//! 
template<typename T>
LCPAChecker<T>::LCPAChecker(
	std::string & _sFileName, 
	std::string & _saFileName, 
	std::string & _lcpFileName, 
	uint32 _len, 
	uint32 _maxLCP): 
	mLen(_len), 
	mMaxLCP(_maxLCP){
	
	///step 1: load data into RAM
	//load input string
	stxxl::syscall_file * sFile = new stxxl::syscall_file(_sFileName, 
		stxxl::syscall_file::RDWR | stxxl::syscall_file::DIRECT);

	t_vector_type * exS = new t_vector_type(sFile);

	typename t_vector_type::bufreader_type * exSReader = new typename t_vector_type::bufreader_type(*exS);

	mS = new T[mLen];

	for (uint32 i = 0; i < mLen; ++i, ++(*exSReader)) {

		mS[i] = *(*exSReader);
	}

	delete exSReader; exSReader = NULL; delete exS; exS = NULL; delete sFile; sFile = NULL;

	//load suffix array
	stxxl::syscall_file * saFile = new stxxl::syscall_file(_saFileName, 
		stxxl::syscall_file::RDWR | stxxl::syscall_file::DIRECT);

	uint32_vector_type *exSA = new uint32_vector_type(saFile);

	typename uint32_vector_type::bufreader_type * exSAReader = new typename uint32_vector_type::bufreader_type(*exSA);

	mSA = new uint32[mLen];

	for (uint32 i = 0; i < mLen; ++i, ++(*exSAReader)) {

		mSA[i] = *(*exSAReader);
	}

	delete exSAReader; exSAReader = NULL; delete exSA; exSA = NULL; delete saFile; saFile = NULL;

	//load lcp array
	stxxl::syscall_file *lcpFile = new stxxl::syscall_file(_lcpFileName, 
		stxxl::syscall_file::RDWR | stxxl::syscall_file::DIRECT);

	uint32_vector_type *exLCP = new uint32_vector_type(lcpFile);

	typename uint32_vector_type::bufreader_type * exLCPReader = new typename uint32_vector_type::bufreader_type(*exLCP);

	mLCP = new uint32[mLen];

	for (uint32 i = 0; i < mLen; ++i, ++(*exLCPReader)) {

		mLCP[i] = *(*exLCPReader);
	}
	
	delete exLCPReader; exLCPReader = NULL; delete exLCP; exLCP = NULL; delete lcpFile; lcpFile = NULL;

	///step 2: check the correctness of LCP	
	if(!run()){

		std::cerr<< "check failed!\n" << std::endl;
	}
	else {

		std::cerr << "check passed!\n" << std::endl;
	} 

	delete [] mS; mS = NULL;

	delete [] mSA; mSA = NULL;

	delete [] mLCP; mLCP = NULL;

	return ;
}

template<typename T>
bool LCPAChecker<T>::run(){
	
	///step 1: compute mRInterval[0, maxLCP], double-side closed.
	mRInterval = new FPTYPE_A[mMaxLCP + 1];
	
	mRInterval[0] = 1;

	for (uint32 i = 1; i <= mMaxLCP; ++i) {

		mRInterval[i] = static_cast<FPTYPE_A>((static_cast<FPTYPE_B>(mRInterval[i - 1]) * R) % P);
	}


	///step 2: compute mFP[0, mLen)
	mFP = new FPTYPE_A[mLen];

	mFP[0] = static_cast<FPTYPE_A>(mS[0] + 1);

	for (uint32 i = 1; i < mLen; ++i) {

		mFP[i] = static_cast<FPTYPE_A>((static_cast<FPTYPE_B>(mFP[i - 1]) * R + (mS[i] + 1)) % P);
	}


	///step 3: check the correctness of LCP
	bool isRight = true;

	FPTYPE_A fp1, fp2, fp3, fp4, fpInterval1, fpInterval2;

	uint32 pos1, pos2, pos3, pos4;

	for (uint32 i = 1; i < mLen; ++i) {

		pos1 = mSA[i - 1];

		pos2 = mSA[i];

		if (mLCP[i] == 0) { //lcp = 0, check equality of two head characters

			if (mS[pos1] == mS[pos2]) {

				isRight = false;

				break;
			}
		}
		else { //mLCP[i] > 0

			//compute fpInterval1
			fp1 = (pos1 == 0) ? 0 : mFP[pos1 - 1];

			pos3 = pos1 + mLCP[i] - 1;

			fp3 = mFP[pos3];

			fpInterval1 = static_cast<FPTYPE_A>((fp3 - (static_cast<FPTYPE_B>(fp1) * mRInterval[mLCP[i]]) % P + P) % P);

			//compute fpInterval2
			fp2 = (pos2 == 0) ? 0 : mFP[pos2 - 1];

			pos4 = pos2 + mLCP[i] - 1;

			fp4 = mFP[pos4];

			fpInterval2 = static_cast<FPTYPE_A>((fp4 - (static_cast<FPTYPE_B>(fp2) * mRInterval[mLCP[i]]) % P + P) % P);

			//compare (fpInterval, ch)
			if (fpInterval1 == fpInterval2) { //compare fingerprint

				if (pos3 + 1 < mLen && pos4 + 1 < mLen && mS[pos3 + 1] == mS[pos4 + 1]) {// compare next characters

					isRight = false;

					break;
				}
			}
			else { //fpInterval1 != fpInterval2

				isRight = false;

				break;
			}
		}
	}

	delete[] mRInterval;

	delete[] mFP;

	return isRight;
}

//! build lcp array

//! \param T alphabet type of input string
template<typename T>
class LCPABuilder{
private:

	//alias
	typedef typename ExVector<T>::vector t_vector_type;

	typedef typename ExVector<uint32>::vector uint32_vector_type;

	//member
	T *mS; //!< ptr to input string

	uint32 *mSA; //!< ptr to input suffix array

	uint32 *mLCP; //!< ptr to input lcp array

	uint32 mLen; //!< length of input string

	FPTYPE_A * mRInterval; //!< ptr to range interval array

	FPTYPE_A * mFP; //!< ptr to fingerprint array

	uint32 *mPA1; //!< ptr to position array for set A

	uint32 *mPA2; //!< ptr to position array for set A

	uint32 mPALen; //!< length of position array for set A

public:

	LCPABuilder(std::string _sFileName, std::string _saFileName, std::string _lcpFileName, uint32 _len); 

	void run();

	uint32 computePrefixLen(uint32 _pos1, uint32 _pos2, uint32 _maxPrefixLen);

	void iterRun(int _stride);
};


template<typename T>
LCPABuilder<T>::LCPABuilder(std::string _sFileName, 
	std::string _saFileName, 
	std::string _lcpFileName, 
	uint32 _len): 
	mLen(_len){

	///step 1: load data
	//load input string
	stxxl::syscall_file * sFile = new stxxl::syscall_file(_sFileName, 
		stxxl::syscall_file::RDWR | stxxl::syscall_file::DIRECT);

	t_vector_type *exS = new t_vector_type(sFile);

	typename t_vector_type::bufreader_type *exSReader = new typename t_vector_type::bufreader_type(*exS);

	mS = new T[mLen];

	for (uint32 i = 0; i < mLen; ++i, ++(*exSReader)) {

		mS[i] = *(*exSReader);
	}

	delete exSReader; exSReader = NULL; delete exS; exS = NULL; delete sFile; sFile = NULL;

	//load suffix array
	stxxl::syscall_file *saFile = new stxxl::syscall_file(_saFileName, 
		stxxl::syscall_file::RDWR | stxxl::syscall_file::DIRECT);

	uint32_vector_type *exSA = new uint32_vector_type(saFile);

	typename uint32_vector_type::bufreader_type *exSAReader = new typename uint32_vector_type::bufreader_type(*exSA);

	mSA = new uint32[mLen];

	for (uint32 i = 0; i < mLen; ++i, ++(*exSAReader)) {

		mSA[i] = *(*exSAReader);
	}

	delete exSAReader; exSAReader = NULL; delete exSA; exSA = NULL; delete saFile; saFile = NULL;


	///step 2: build LCP array
	mLCP = new uint32[mLen];

	for (uint32 i = 0; i < mLen; ++i) { // initialize 

		mLCP[i] = std::numeric_limits<uint32>::max();
	}

	run();

	///step 3: output result	
	stxxl::syscall_file *lcpFile = new stxxl::syscall_file(_lcpFileName, 
		stxxl::syscall_file::CREAT | stxxl::syscall_file::RDWR | stxxl::syscall_file::DIRECT);

	uint32_vector_type *exLCP = new uint32_vector_type(lcpFile);

	typename uint32_vector_type::bufwriter_type *exLCPWriter = new typename uint32_vector_type::bufwriter_type(*exLCP);
	
	for (uint32 i = 0; i < mLen; ++i) {

		 (*exLCPWriter) << mLCP[i];
	}

	exLCPWriter->finish();

	delete exLCPWriter; exLCPWriter = NULL; delete exLCP; exLCP = NULL; delete lcpFile; lcpFile = NULL;
}

template<typename T> 
void LCPABuilder<T>::run(){

	///step 1: compute mRInterval[0, mLen]
	mRInterval = new FPTYPE_A[mLen + 1];
	
	mRInterval[0] = 1;

	for (uint32 i = 1; i <= mLen; ++i) {

		mRInterval[i] = static_cast<FPTYPE_A>((static_cast<FPTYPE_B>(mRInterval[i - 1]) * R) % P);
	}	

	///step 2: compute fp[0, mLen)
	mFP = new FPTYPE_A[mLen];

	mFP[0] = static_cast<FPTYPE_A>(mS[0] + 1);

	for (uint32 i = 1; i < mLen; ++i) {

		mFP[i] = static_cast<FPTYPE_A>((static_cast<FPTYPE_B>(mFP[i - 1]) * R + (mS[i] + 1)) % P); 
	}

	///step 3: build lcp array
	//substep 1: divide into two sets: A = {(i, j) | lcp(SA[i], SA[i - 1]) >= W1} and B = {(i,j) | lcp(SA[i], SA[i - 1]) < W1}
	mPA1 = new uint32[mLen];

	mPA2 = new uint32[mLen];

	mPALen = 0;

	for (uint32 i = 1; i < mLen; ++i) {

		uint32 prefixLen = computePrefixLen(mSA[i], mSA[i - 1], W1); //compute W1-order common prefix length

		if (prefixLen == W1) { // >= W1, belongs to A

			mPA1[mPALen] = mSA[i];

			mPA2[mPALen] = mSA[i - 1];

			++mPALen;
		}
		else { // < W1, belongs to B

			mLCP[i] = prefixLen;
		}
	}

	//substep 2: perform iterations on set A
	uint32 stride = K / 2; 

	while (stride >= W2){

		iterRun(stride);

		stride >>= 1;
	}

	//substep 3: compare the final W2 characters and compute the final result
	for (uint32 i = 0, j = 1; i < mPALen; ++i) {
	
		uint32 prefixLen = computePrefixLen(mPA1[i], mPA2[i], W2);

		for (; j < mLen; ++j) {

			if (mLCP[j] == std::numeric_limits<uint32>::max()) {

				mLCP[j] = mPA1[i] + prefixLen - mSA[j];
				
				break;
			}
		}
	}

	return;
}	

template<typename T>
void LCPABuilder<T>::iterRun(int _stride){

	uint32 pos1, pos2, pos3, pos4;

	FPTYPE_A fp1, fp2, fp3, fp4, fpInterval1, fpInterval2;

	for (uint32 i = 0; i < mPALen; ++i) {

		//
		pos1 = mPA1[i];

		pos2 = mPA1[i] + _stride - 1; //_stride > 0

		pos3 = mPA2[i];

		pos4 = mPA2[i] + _stride - 1; //_stride > 0

		if (pos2 < mLen && pos4 < mLen) {
			fp1 = (pos1 == 0) ? 0 : mFP[pos1 - 1];

			fp2 = mFP[pos2];
			
			fpInterval1 = static_cast<FPTYPE_A>((fp2 - (static_cast<FPTYPE_B>(fp1) * mRInterval[_stride]) % P + P) % P);
	
			fp3 = (pos3 == 0) ? 0 : mFP[pos3 - 1];
	
			fp4 = mFP[pos4];

			fpInterval2 = static_cast<FPTYPE_A>((fp4 - (static_cast<FPTYPE_B>(fp3) * mRInterval[_stride]) % P + P) % P);
	
			if (fpInterval1 == fpInterval2) {

				mPA1[i] += _stride;
	
				mPA2[i] += _stride;
			}	
		}
	}

	return;
}

template<typename T>
inline uint32 LCPABuilder<T>::computePrefixLen(uint32 _pos1, uint32 _pos2, uint32 _maxPrefixLen){

	_maxPrefixLen = std::min(mLen - std::max(_pos1, _pos2), _maxPrefixLen);

	uint32 prefixLen = 0;

	for (; prefixLen < _maxPrefixLen; ++prefixLen, ++_pos1, ++_pos2) {

		if (mS[_pos1] != mS[_pos2]) {

			break;
		}
	}
	
	return prefixLen;
}

NAMESPACE_LCP_RAM_END

#endif // LCPA_RAM_H

