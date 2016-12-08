//#ifndef KLCPA_RAM_MPI_H
//#define KLCPA_RAM_MPI_H
//
//
//#include "common.h"
//#include "data.h"
//
//
//#define ITERS 3
//
////! K-order LCPA checker
//template<typename T1, typename T2>
//class KLCPAChecker{
//private:
//
//	//alias
//	typedef typename ExVector<T1>::vector t1_vector_type;
//
//	typedef typename ExVector<T2>::vector t2_vector_type;
//
//	//member
//	int mCurRank; //!< rank of current computing node in the comm world
//
//	int mCommSize; //!< size of the comm world 
//
//	MPI_Status * mStatus; 
//
//	size_t mCapacity; //!< maximum characters in a partition
//
//	size_t mLen; //!< length of the partition
//
//	size_t mAddSLen; //!< borrow at most K characters from the next block.
//
//	size_t mAddLCPLen; //!< borrow at most 1 element from the next block.
//
//	T1 *mS;  //!< input string
//	
//	T2 *mSA; //!< input suffix array
//
//	T2 *mLCP; //!< input lcp array
//
//	FPTYPE_A *mFP; //! fingerprint array
//
//	FPTYPE_A mPreLastFP; //FP[0, mLen) in current block
//	
//	FPTYPE_A mCurLastFP; //FP[0, mLen) in previous block
//
//	T2 mPreLastSA; //SA[mLen - 1] in previous block
//
//	FPTYPE_A *mRInterval; //< rinterval array
//
//	size_t * mReqNumSend; //!< number of requests sent to each node
//
//	size_t * mReqNumRecv; //!< number of requests received from each node
//
//	//! request payload 
//	struct ReqPayload{ 
//		T2 pos;
//
//		T2 lcp1;
//
//		T2 lcp2;
//	};
//
//	//! response payload
//	struct ResPayload {
//		FPTYPE_A fpInterval1;
//
//		FPTYPE_A fpInterval2;
//
//		T1 ch1; 
//
//		T2 ch2;
//
//	};
//
//public:	
//	KLCPAChecker(std::string & _sFileName, std::string & _saFilename, std::string & _lcpFileName, size_t _len, size_t _capacity, int _curRank, int _commSize);
//
//	int run();
//};
//
////!Assume x, sa and lcp are evenly distributed onto {b_0, b_1, ..., b_d}. Check lcp_{b_i} on b_i.
//template<typename T1, typename T2>
//KLCPAChecker<T1, T2>::KLCPAChecker(std::string & _sFileName, std::string & _saFilename, 
//	std::string & _lcpFileName, size_t _len, size_t _capacity, int _curRank, int _commSize) : mLen(_len), 
//	mCapacity(_capacity), mCurRank(_curRank), mCommSize(_commSize){
//
//	//step 1: load data
//	stxxl::syscall_file *sFile = new stxxl::syscall_file(_sFileName, stxxl::syscall_file::open_mode::RDWR | stxxl::syscall_file::open_mode::DIRECT);
//	t1_vector_type * exS = new t1_vector_type(sFile);
//	typename t1_vector_type::bufreader_type * exSReader = new typename t1_vector_type::bufreader_type(*exS);
//	
//	stxxl::syscall_file *saFile = new stxxl::syscall_file(_saFilename, stxxl::syscall_file::open_mode::RDWR | stxxl::syscall_file::open_mode::DIRECT);	
//	t2_vector_type *exSA = new t2_vector_type(saFile);
//	typename t2_vector_type::bufreader_type *exSAReader = new typename t2_vector_type::bufreader_type(*exSA);
//
//	stxxl::syscall_file *lcpFile = new stxxl::syscall_file(_saFilename, stxxl::syscall_file::open_mode::RDWR | stxxl::syscall_file::open_mode::DIRECT);	
//	t2_vector_type *exLCP = new t2_vector_type(lcpFile);
//	typename t2_vector_type::bufreader_type *exLCPReader = new typename t2_vector_type::bufreader_type(*exLCP);
//
//	mReqNumSend = new size_t[mCommSize]; //mReqNumSend[i] records the number of requests sent to node i.
//	for (int i = 0; i < mCommSize; ++i) mReqNumSend[i] = 0;
//	
//	mAddSLen = (mCurRank == mCommSize - 1) ? 0 : K; //if not the rightmost block, borrow K characters from the next block
//	mAddLCPLen = (mCurRank == mCommSize - 1) ? 0 : 1;
//
//	T1 *mS = new T1[mLen + mAddSLen];
//	T2 *mSA = new T2[mLen];
//	T2 *mLCP = new T2[mLen + mAddLCPLen];
//
//	for (size_t i = 0; i < mLen; ++i, ++(*exSReader), ++(*exSAReader), ++(*exLCPReader)) {
//		mS[i] = *(*exSReader);
//		mSA[i] = *(*exSAReader), ++mReqNumSend[mSA[i] / mCapacity];
//		mLCP[i] = *(*exLCPReader);
//	}
//
//	delete exSReader; exSReader = NULL; delete exS; exS = NULL; delete sFile; sFile = NULL;
//	delete exSAReader; exSAReader = NULL; delete exSA; exSA = NULL; delete saFile; saFile = NULL;
//	delete exLCPReader; exLCPReader = NULL; delete exLCP; exLCP = NULL; delete lcpFile; lcpFile = NULL;
//
//	///step 2: borrow K characters for mS from next block; borrow 1 element for mLCP from next block.
//	for (int commRank = mCommSize - 1; commRank >= 0; --commRank) {
//		if (mCurRank == commRank) {
//			//recv
//			if (mCurRank != mCommSize - 1) {
//				MPI_Recv((char*)(mS + mLen), K * sizeof(T1) / sizeof(char), MPI_CHAR,
//					mCurRank + 1, mCurRank + 1, MPI_COMM_WORLD, mStatus);
//
//				MPI_Recv((char*)(mLCP + mLen), sizeof(T2) / sizeof(char), MPI_CHAR,
//					mCurRank + 1, mCurRank + 1, MPI_COMM_WORLD, mStatus);
//			}
//
//			//send
//			if (mCurRank != 0) {
//				MPI_Send((char*)mS, K * sizeof(T1) / sizeof(char), MPI_CHAR,
//					mCurRank - 1, mCurRank, MPI_COMM_WORLD);
//
//				MPI_Send((char*)mLCP, sizeof(T2) / sizeof(char), MPI_CHAR,
//					mCurRank - 1, mCurRank, MPI_COMM_WORLD);
//			}
//		}
//	}
//
//	///step 3: check local result
//	int isRight = run();
//
//	///step 4: reduce global result
//	int sum = 0;
//	MPI_Reduce(&isRight, &sum, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
//
//	if (mCurRank == 0) {
//		if (sum == mCommSize) {
//			std::cerr << "check passed!\n";
//		}
//		else {
//			std::cerr << "check failed!\n";
//		}
//	}
//
//	delete mS; mS = NULL;
//	delete mSA; mSA = NULL;
//	delete mLCP; mLCP = NULL;	
//}
//
//template<typename T1, typename T2>
//int KLCPAChecker<T1, T2>::run(){
//	bool isRight = true;
//
//	///step 1: compute fp[0, mLen + K)
//	mFP = new FPTYPE_A[mLen + mAddSLen];
//	
//	mFP[0] = mS[0] + 1; // mS[0] + 1 < std::numeric_limits<FPTYPE_A>::max()
//	for (size_t i = 0; i < mLen + mAddSLen; ++i) {
//		mFP[i] = static_cast<FPTYPE_A>((static_cast<FPTYPE_B>(mFP[i - 1]) * R + (mS[i] + 1)) % P); 		
//	}
//	
//	//step 2: compute lastFP and send it the next computing node as preLastFP
//	mPreLastFP = 0, mCurLastFP = 0;
//	for (int commRank = 0; commRank < mCommSize; ++commRank) {
//		if (mCurRank == commRank) {
//			//recv
//			if (mCurRank != 0) {
//				MPI_Recv((char *)&mPreLastFP, sizeof(FPTYPE_A) / sizeof(char), MPI_CHAR, 
//					mCurRank - 1, mCurRank - 1, MPI_COMM_WORLD, mStatus);
//			}
//
//			//send
//			if (mCurRank != mCommSize - 1){
//				mCurLastFP = static_cast<FPTYPE_A>((static_cast<FPTYPE_B>(mPreLastFP) * mRInterval[mLen] + mFP[mLen - 1]) % P); //!not mLen + K
//				MPI_Send((char *)&mCurLastFP, sizeof(FPTYPE_A) / sizeof(char), MPI_CHAR, 
//					mCurRank + 1, mCurRank, MPI_COMM_WORLD);
//			}
//		}
//	}
//
//	for (int commRank = 0; commRank < mCommSize; ++commRank) {
//		if (mCurRank == commRank) {
//			//recv
//			if (mCurRank != 0) {
//				MPI_Recv((char *)& mPreLastSA, sizeof(T2) / sizeof(char), MPI_CHAR, 
//					mCurRank - 1, mCurRank - 1, MPI_COMM_WORLD, mStatus);
//			}
//
//			//send
//			if (mCurRank != mCommSize - 1) {
//				MPI_Send((char *)& mSA[mLen - 1], sizeof(T2) / sizeof(char), MPI_CHAR,
//					mCurRank, mCurRank, MPI_COMM_WORLD);
//			}
//		}
//	}
//
//	///step 3: check lcp
//	//substep 1: all-to-all mReqNumSend to mReqNumRecv
//	mReqNumRecv = new size_t[mCommSize]; //no need to initialize
//	
//	MPI_Alltoall((char*)mReqNumSend, sizeof(size_t) / sizeof(char), MPI_CHAR, 
//		(char*)mReqNumRecv, sizeof(size_t) / sizeof(char), MPI_CHAR, MPI_COMM_WORLD);
//	
//	FPTYPE_A fp1, fp2, fp3, fp4;
//	T1 ch1, ch2;
//
//	//substep 2: send requests
//	CBuffer<ReqPayload> *reqSendBuf = new CBuffer<ReqPayload>(mReqNumSend, mCommSize);
//
//	for(size_t i = 0; i < mLen - 1; ++i) {
//		reqSendBuf->set(reqDest[i], ReqPayload(mSA[i], mLCP[i], mLCP[i + 1]));
//	}
//
//	reqSendBuf->set(reqDest[mLen - 1], ReqPayload(mSA[mLen - 1], mLCP[mLen - 1], ((mCurRank == mCommSize - 1) ? 0 : mLCP[mLen])));
//	
//
//	//substep 3: receive requests
//	CBuffer<ReqPayload> *reqRecvBuf = new CBuffer<ReqPayload>(mReqNumRecv, mCommSize);
//	CBuffer<ReqPayload>::myAlltoallv(*reqSendBuf, *reqRecvBuf);
//
//	delete reqSendBuf;
//
//
//	//substep 4: compute FPInterval
//	CBuffer<ResPayload> *resSendBuf = new CBuffer<ResPayload>(mReqNumRecv, mCommSize);
//	FPTYPE_A fp1, fp2, fp3, fpInterval1, fpInterval2;
//	T2 pos1, pos2, pos3;
//	T1 ch1, ch2;
//	ReqPayload reqPayload;
//
//	for (size_t i = 0; i < reqRecvBuf->mTotalLen; ++i) {
//		reqPayload = reqRecvBuf[i];
//		pos1 = reqPayload.pos - mCapacity * mCurRank; //localPos
//		fp1 = (pos == 0) ? ((mCurRank == 0)) ? 0 : mPreLastFP) : fp[pos1 - 1];
//
//		if (reqPayload.lcp1 == 0) {
//			fpInterval1 = 0;
//			ch1 = mS[pos1];
//		}
//		else {
//			pos2 = pos1 + reqPayload.lcp1 - 1;
//			fp2 = fp[pos2];
//			fpInterval1 = static_cast<FPTYPE_A>((fp2 - static_cast<FPTYPE_B>(fp1) * mRInterval[lcp1] + P) % P);
//			ch1 = (pos2 == mLen - 1) ? (mCurRank == mCommSize - 1) ? 0 : mS[mLen]) : mS[pos2 + 1];
//		}
//
//		if (reqPayload.lcp2 == 0) {
//			fpInterval2 = 0;
//			ch2 = mS[pos1];
//		}
//		else {
//			pos3 = pos1 + reqPayload.lcp2 - 1;
//			fp3 = fp[pos3];
//			fpInterval2 = static_cast<FPTYPE_A>((fp3 - static_cast<FPTYPE_B>(fp1) * mRInterval[lcp2] + P) % P);
//			ch2 = (pos3 == mLen - 1) ? (mCurRank == mCommSize - 1) ? 0 : mS[mLen]) : mS[pos3 + 1];
//		}
//		
//		resSendBuf->set(ResPayload(fpInterval1, fpInterval2, ch1, ch2));
//	}
//
//	CBuffer<ResPayload> *resRecvBuf = new CBuffer<ResPayload>(mReqNumSend, mCommSize);
//	CBuffer<ResPayload>::myAlltoallv(*resSendBuf, *resRecvBuf);
//
//	delete resSendBuf;
//
//	//substep 5: send/receive lastFPInterval
//	ResPayload preLastResPayload;
//
//	for (int commRank = 0; commRank < mCommSize; ++commRank) {
//		if (mCurRank == commRank) {
//			if (mCurRank != 0) {
//				MPI_Recv((char*)&preLastResPayload, sizeof(ResPayload) / sizeof(char), MPI_CHAR,
//					mCurRank - 1, mCurRank - 1, MPI_COMM_WORLD, mStatus);
//			}
//
//			if (mCurRank != mCommSize - 1) {
//				ResPayload & curLastResPayload = resRecvBuf->mData[resRecvBuf->mCurOffsetPart[reqDest[mLen - 1]]];
//				MPI_Send((char*)&curLastResPayload, sizeof(ResPayload) / sizeof(char), MPI_CHAR,
//					mCurRank, mCurRank, MPI_COMM_WORLD, mStatus);
//			}
//		}
//	}
//
//	FPTYPE_A fpInterval2 = preLastResPayload.fpInterval2;
//	T1 ch2 = preLastResPayload.ch2;
//
//	if (mCurRank != 0) {
//		ResPayload & curResPayload = resRecvBuf->get(mSA[0]);
//
//		if (fpInterval2 == curResPayload.fpInterval1) {
//			if (mPreLastSA + LCP[0] < mTotalLen && mSA[0] + LCP[0] < mTotalLen && ch2 == curResPayload.ch1) {
//				isRight = false;
//			}
//		}
//		else {
//			isRight = false;
//		}
//
//		fpInterval2 = curResPayload.fpInterval2;
//		ch2 = curResPayload.ch2;
//	}
//
//
//	if (isRight) {
//		for (size_t i = 1; i < mLen - 1; ++i) {
//			ResPayload & curResPayload = resRecvBuf->get(mSA[i]);
//
//			if (fpInterval2 == curResPayload.fpInterval2) {
//				if (mSA[i - 1] + LCP[i] < mTotalLen && mSA[i] + LCP[i] < mTotalLen && ch2 == curResPayload.ch1) {
//					isRight = false;
//				}
//			}
//			else {
//				isRight = false;
//			}
//
//			fpInterval2 = curResPayload.fpInterval2;
//			ch2 = curResPayload.ch2;
//		}
//	}
//
//	return isRight ? 1 : 0;
//}
//
////! K-order LCP Builder
//template<typename T1, typename T2>
//class KLCPABuilder{
//private:
//	//alias
//	typedef typename ExVector<T1>::vector t1_vector_type;
//	
//	typedef typename ExVector<T2>::vector t2_vector_type;
//
//	//member
//	T1 *mS; //!<input string
//
//	T2 *mSA;	//!<input suffix array
//
//	T2 *mLCP; //!< output lcp array
//
//	size_t mLen; //!< number of characters in current block
//
//	size_t mAddSLen; //!< borrow K characters from the next block, if current block is not the rightmost. Also for mFP.
//
//	size_t mAddSALen; //!< borrow 1 element from the previous block, if current block is not the leftmost.
//
//	size_t mCapacity; //!< at most mCapacity characters in a single block
//
//	int mCommSize; //!< number of computing nodes
//
//	int mCurCommRank; //!< rank of current computing node
//
//	MPI_Status * mStatus; //!< MPI_Status
//
//	FPTYPE_A mRInterval; //!< rinterval
//
//	FPTYPE_A *mFP; //!< intermediate FP
//
//	FPTYPE_A mPreLastFP;
//
//	FPTYPE_A mCurLastFP;
//
//	int *mPosA1;
//
//	int *mPosA2;
//
//	int mPosALen;
//
//	int *mPosB1;
//
//	int *mPosB2;
//
//	int mPosBLen;
//	
//	int *mSplitFlag;
//
//	struct ReqPayload {
//
//		int pos1; 
//	};
//
//	struct ResPayload {
//
//		FPTYPE_A fpInterval;
//	};
//
//public:
//	KLCPABuilder(std::string _sFilename, std::string _saFileName, std::string _lcpFileName,
//		size_t _len, size_t _capacity, int _curCommRank, int _commSize);
//
//	void run();
//
//	void split();
//
//	void iterRun(int _stride, bool _isA);
//
//};
//
//template<typename T1, typename T2>
//KLCPABuilder<T1, T2>::KLCPABuilder(std::string _sFileName, std::string _saFileName, std::string _lcpFileName, 
//	size_t _len, size_t _capacity, int _curCommRank, int _commSize):
//	mLen(_len), mCapacity(_capacity), mCurCommRank(_curCommRank), mCommSize(_commSize){
//
//	///step1: load data
//
//	//load mS
//	stxxl::syscall_file *sFile = new stxxl::syscall_file(_sFileName, 
//		stxxl::syscall_file::open_mode::RDWR | stxxl::syscall_file::open_mode::DIRECT);
//
//	t1_vector_type *exS = new t1_vector_type(sFile);
//
//	typename t1_vector_type::bufreader_type exSReader(*exS);
//
//	mS = new T1[mLen];
//
//	for (size_t i = 0; i < mLen; ++i, ++(*exS)) {
//
//		mS[i] = *(*exS);
//	}
//	
//	delete exSReader; exSReader = NULL; delete exS; exS = NULL; delete sFile; sFile = NULL;
//
//	//send & receive additional characters in mS
//
//	mAddSLen = (mCurCommRank != mCommSize - 1) ? K : 0;
//
//	for (int commRank = 0; commRank < mCommSize; ++commRank) {
//
//		if (mCurCommRank == commRank) {
//
//			//recv
//			if (mCurCommRank != mCommSize - 1) {
//
//				MPI_Recv(static_cast<char*>(exS + mLen), mAddSLen * sizeof(T1) / sizeof(char), MPI_CHAR,
//					mCurCommRank + 1, mCurCommRank + 1, MPI_COMM_WORLD, mStatus);
//			}
//
//			//send
//			if (mCurCommRank != 0) {
//
//				MPI_Send(static_cast<char*>(exS), mAddSLen * sizeof(T1) / sizeof(char), MPI_CHAR,
//					mCurCommRank - 1, mCurCommRank, MPI_COMM_WORLD);
//			}
//		}
//	}
//
//
//	//load mSA
//	stxxl::syscall_file *saFile = new stxxl::syscallfile(_saFileName, 
//		stxxl::syscall_file::open_mode::RDWR | stxxl::syscall_file::open_mode::DIRECT);
//
//	t2_vector_type *exSA = new t2_vector_type(saFile);
//
//	typename t2_vector_type::bufreader_type exSAReader(*exSA);		
//
//	mAddSALen = (mCurCommRank != 0) ? 1 : 0;
//
//	mSA = new T2[mLen + mAddSALen];
//
//	for (size_t i = 1; i < mLen + mAddSALen; ++i, ++(*exSA)) {
//
//		mSA[i] = *(*exSA);
//	}
//
//	delete exSAReader; exSAReader = NULL; delete exSA; exSA = NULL; delete saFile; saFile = NULL;
//
//	//send & recv one element from previous block
//
//	for (int commRank = 0; commRank < mCommSize; ++commRank) {
//		
//		if (mCurCommRank == commRank) {
//			//recv
//			if (mCurCommRank != 0) {
//				
//				MPI_Recv(static_cast<char*>(mSA), sizeof(T2) / sizeof(char), MPI_CHAR,
//					commRank - 1, commRank - 1, MPI_COMM_WORLD, mStatus);
//			}
//
//			//send
//			if (mCurCommRank != mCommSize - 1) {
//				
//				MPI_Send(static_cast<char*>(mSA + mLen), sizeof(T2) / sizeof(char), MPI_CHAR,
//					commRank, commRank, MPI_COMM_WORLD);
//			}
//		}
//	}
//
//	///step 2: build lcp array
//	mLCP = new T2[mLen];
//
//	run();
//
//	///step 3: output the result
//	stxxl::syscall_file *lcpFile = new stxxl::syscall_file(_lcpFileName, 
//		stxxl::syscall_file::open_mode::CREAT, stxxl::syscall_file::open_mode::RDWR | stxxl::syscall_file::open_mode::DIRECT);
//
//	t2_vector_type *exLCP = new t2_vector_type(lcpFile);
//	
//	typename t2_vector_type::bufwriter_type exLCPWriter(*exLCP);
//
//	for (size_t i = 0; i < mLen; ++i) {
//
//		(*exLCPWriter) << mLCP[i];
//	}
//
//	exLCPWriter->finish();
//
//	delete exLCPWriter; exLCPWriter = NULL; delete exLCP; exLCP = NULL; delete lcpFile; lcpFile = NULL;
//}
//
////! run core part to build lcp
//template<typename T1, typename T2>
//void KLCPABuilder<T1, T2>::run() {
//
//	///step 1: compute RInterval
//	mRInterval = new FPTYPE_A[mLen + 1];
//
//	mRInterval[0] = 1;
//
//	for (size_t i = 1; i <= mLen; ++i) {
//
//		mRInterval[i] = static_cast<FPTYPEA>((static_cast<FPTYPE_B>(mRInterval[i - 1]) * R) % P);
//	}
//
//	///step 2: copute local/ global FP
//
//	//local FP
//	mFP = new FPTYPE_A[mLen + mAddSLen];
//
//	mFP[0] = static_cast<FPTYPE_A>(mS[0] + 1);
//
//	for (size_t i = 1; i < mLen + mAddSLen; ++i) {
//
//		mFP[i] = static_cast<FPTYPE_A>((static_cast<FPTYPE_B>(mFP[i - 1]) * R + (mS[i] + 1)) % P);
//	}
//
//	//send & recv lastFP
//	mPreLastFP, mCurLastFP;
//
//	for (int commRank = 0; commRank < commSize; ++commRank) {
//
//		if (commRank == mCurCommRank) {
//
//			//recv
//			if (commRank != 0) {
//
//				MPI_Recv(static_cast<char*>(&mPreLastFP), sizeof(FPTYPE_A) / sizeof(char), MPI_CHAR,
//					commRank - 1, commRank - 1, MPI_COMM_WORLD, mStatus);
//			}
//
//			//send
//			if (commRank != mCommSize - 1) {
//
//				mCurLastFP = static_cast<FPTYPE_A>((static_cast<FPTYPE_B>(mPreLastFP) * RInterval[mLen] + mFP[mLen - 1]) % P); //global fp
//
//				MPI_Send(static_cast<char*>(&mCurLastFP), sizeof(FPTYPE_A) / sizeof(char), MPI_CHAR,
//					commRank, commRank, MPI_COMM_WORLD);
//			}
//		}
//	}
//
//	//global fp
//	for (size_t i = 0; i < mLen; ++i) {
//
//		mFP[i] = static_cast<FPTYPE_A>((static_cast<FPTYPE_B>(mPreLastFP) * RInterval[i + 1] + mFP[i]) % P);
//	}
//
//	///step 3: split into two groups
//	void split();
//
//
//	///step 4: process mPosA1 and mPosA2 with lcp < W
//	for (int stride = W1 / 2; stride > 0; stride >> 1) {
//		iterRun(stride, true);
//	}
//
//	///step 5: process mPosB1 and mPosB2 with lcp >= K
//	for (int stride = K / 2; stride > W1; stride >> 1) {
//		iterRun(stride, false);
//	}
//
//	///step 6: merge
//	for (size_t i = 1, j = 0, k = 0; i < mLen + mAddSALen; ++i) {
//		mLCP[i - 1] = mSplitFlag[i - 1] ? (PA1[i - 1] - mSA[i]) : (PA3[i - 1] - mSA[i]);
//	}
//
//	return;
//}
//
//template<typename T1, typename T2>
//void KLCPABuilder<T1, T2>::split(){
//
//	///step 1: all-to-all number of requests 
//	int *reqDestNum = new int[mCommSize];
//
//	for (int i = 0; i < mCommSize; ++i) reqDestNum[i] = 0;
//
//	for (int i = 0; i < mLen + mAddSALen; ++i) {
//
//		++reqDestNum[mSA[i] / mCapacity];
//	}
//
//	int * reqSrcNum = new int[mCommSize];
//
//	MPI_Alltoall(reqDestNum, 1, MPI_INT, reqSrcNum, 1, MPI_INT, MPI_COMM_WORLD);
//
//	///step 2: all-to-alll requests
//	CBuffer<ReqPayload> * reqSendBuf = new CBuffer<ReqPayload>(reqDestNum);
//
//	for (size_t i = 0; i < mLen + mAddSALen; ++i) {
//
//		reqSendBuf->mSet(mSA[i]);
//	}
//
//	CBuffer<ReqPayload> * reqRecvBuf = new CBuffer<ReqPayload>(reqSrcNum);
//
//	CBuffer<ReqPayload>::myAlltoallv(reqSendBuf, reqRecvBuf);
//	
//	delete reqSendBuf;
//
//	///step 3: compute W1-order fingerprint
//	CBuffer<ResPayload> *resSendBuf = new CBuffer<ResPayload>(reqSrcNum);
//
//	FPTYPE_A fp1, fp2;
//	int pos1, pos2;
//
//	for (size_t i = 0; i < reqRecvBuf->mTotalLen; ++i) {
//
//		ReqPayload & reqPayload = reqRecvBuf[i];
//
//		pos1 = reqPayload.pos - mCurCommRank * mCapacity;
//
//		fp1 = (pos1 == 0) ? ((mCurCommRank == 0) ? 0 : mPreLastFP) : mFP[pos1 - 1];
//
//		pos2 = pos1 + W1 - 1;
//
//		fp2 = (pos2 >= mLen + mAddSLen) ? 0 : mFP[pos2];
//
//		fpInterval = static_cast<FPTYPE_A>((fp2 - static_cast<FPTYPE_B>(fp1) * mRInterval[W1] + P) % P);
//
//		resSendBuf->set(fpInterval);
//
//	}
//
//	///step 4: all-to-all responses
//	CBuffer<ResPayload> *resRecvBuf = new CBuffer<ResPayload>(reqDestNum);
//
//	CBuffer<ResPayload>::myAlltoallv(resSendBuf, resRecvBuf);
//
//	delete resSendBuf;
//
//	//step 5: compare FPInterval to split into A and B sets. 
//	
//	mPosA1 = new int[mLen];
//
//	mPosA2 = new int[mLen];
//
//	mPosB1 = new int[mLen];
//
//	mposB2 = new int[mLen];
//
//	mSplitFlag = new bool[mLen];
//
//	int prePos = mSA[i - 1];
//
//	FPTYPE_A preFPInterval = resRecvBuf->get(prePos / mCapacity);
//
//	int curPos;
//
//	FPTYPE_A curFPInterval;
//
//	for (size_t i = 1, mPosALen = 0, mPosBLen = 0; i < mLen + mAddSALen; ++i) {
//		
//		curPos = mSA[i];
//		
//		curFPInterval = resRecvBuf->get(curPos / mCapacity);
//
//		if (prePos + W1 >= mTotalLen || curPos + W1 >= mTotalLen) {
//			mPosA1[mPosALen] = prePos;
//			mPosA2[mPosALen++] = curPos;
//
//			mSplitFlag[i] = true;
//		}
//		else {
//			if (preFPInterval == curFPInterval) {
//				mPosB1[mPosBLen] = prePos;
//				mPosB2[mPosBLen++] = curPos;
//
//				mSplitFlag[i] = false;
//			}
//			else {
//				mPosA1[mPosALen] = prePos;
//				mPosA2[mPosALen++] = curPos;
//
//				mSplitFlag[i] = true;
//			}
//		}
//	}
//
//	delete resRecvBuf;
//}
//
////! perform one iter to check  
//template<typename T1, typename T2>
//void KLCPABuilder<T1, T2>::iterRun(int _stride, bool _isA) {
//
//	///step 1: all-to-all number of requests
//	int *posArr1 = (_isA == true) ? mPosA1 : mPosB1;
//
//	int *posArr2 = (_isA == true) ? mPosA2 : mPosB2;
//
//	int len = (_isA == true) ? mPosALen : mPosBLen;
//
//	int * reqDestNum1 = new int[mCommSize];
//
//	int * reqDestNum2 = new int[mCommSize];
//
//	for (int i = 0; i < mCommSize; ++i) reqDestNum1[i] = 0;
//
//	for (int i = 0; i < mCommSize; ++i) reqDestNum2[i] = 0;
//
//	for (int i = 0; i < len; ++i) {
//		
//		++reqDestNum1[posArr1[i] / mCapacity];
//
//		++reqDestNum2[posArr2[i] / mCapacity];
//	}
//
//	int *reqSrcNum1 = new int[mCommSize];
//
//	MPI_Alltoall(reqDestNum1, 1, MPI_INT, reqSrcNum1, 1, MPI_INT, MPI_COMM_WORLD);
//
//	int *reqSrcNum2 = new int[mCommSize];
//
//	MPI_Alltoall(reqDestNum2, 1, MPI_INT, reqSrcNum2, 1, MPI_INT, MPI_COMM_WORLD);
//
//	///step 2: all-to-all requests
//	CBuffer<ReqPayload> *reqSendBuf1 = new CBuffer<ReqPayload>(reqDestNum1);
//
//	CBuffer<ReqPayload> *reqSendBuf2 = new CBuffer<ReqPayload>(reqDestNum2);
//
//	for (size_t i = 0; i < mLen + mAddSALen; ++i) {
//	
//		reqSendBuf1->set(posArr1[i]);
//		
//		reqSendBuf2->set(posArr2[i]);
//	}
//
//	///step 2: all-to-all requests
//	CBuffer<ReqPayload> *reqRecvBuf1 = new CBuffer<ReqPayload>(reqSrcNum1);
//
//	CBuffer<ReqPayload>::myAlltoallv(reqSendBuf1, reqRecvBuf1);
//
//	delete reqSendBuf1;
//
//	CBuffer<ReqPayload> *reqRecvBuf2 = new CBuffer<ReqPayload>(reqSrcNum2);
//
//	CBuffer<ReqPayload>::myAlltoallv(reqSendBuf2, reqRecvBuf2);
//
//	delete reqSendBuf2;
//
//	///step 3: compute stride-order fingerprint
//	CBuffer<ResPayload> *resSendBuf1 = new CBuffer<ResPayload>(reqSrcNum1);
//
//	for (size_t i = 0; i < reqRecvBuf1->mTotalLen; ++i) {
//
//		ReqPayload & reqPayload = reqRecvBuf1[i];
//
//		int pos1 = reqPayload.pos - mCurCommRank * mCapacity;
//
//		FPTYPE_A fp1 = (pos1 == 0) ? ((mCurCommRank == 0) ? 0 : mPreLastFP) : mFP[pos1 - 1];
//
//		int pos2 = pos1 + stride - 1;
//
//		FPTYPE_A fp2 = (pos2 >= mLen + mAddSLen) ? 0 : mFP[pos2];
//
//		FPTYPE_A fpInterval = static_cast<FPTYPE_A>((fp2 - static_cast<FPTYPE_B>(fp1) * mRInterval[stride] + P) % P);
//
//		resSendBuf1->set(fpInterval);
//	}
//
//
//	CBuffer<ResPayload> *resSendBuf2 = new CBuffer<ResPayload>(reqSrcNum2);
//
//	for (size_t i = 0; i < reqRecvBuf2->mTotalLen; ++i) {
//
//		ReqPayload & reqPayload = reqRecvBuf2[i];
//
//		int pos1 = reqPayload.pos - mCurCommRank * mCapacity;
//
//		FPTYPE_A fp1 = (pos1 == 0) ? ((mCurCommRank == 0) ? 0 : mPreLastFP) : mFP[pos1 - 1];
//
//		int pos2 = pos1 + stride - 1;
//
//		FPTYPE_A fp2 = (pos2 >= mLen + mAddSLen) ? 0 : mFP[pos2];
//
//		FPTYPE_A fpInterval = static_cast<FPTYPE_A>((fp2 - static_cast<FPTYPE_B>(fp1) * mRInterval[stride] + P) % P);
//
//		resSendBuf2->set(fpInterval);
//	}
//
//	///step 4: all-to-all responses
//	CBuffer<ResPayload> *resRecvBuf1 = new CBuffer<ResPayload>(reqDestNum1);
//
//	CBuffer<ResPayload>::myAlltoallv(resSendBuf1, resRecvBuf1);
//
//	delete resRecvBuf1;
//
//	CBuffer<ResPayload> *resRecvBuf2 = new CBuffer<ResPayload>(reqDestNum2);
//
//	CBuffer<ResPayload>::myAlltoallv(resSendBuf2, resRecvBuf2);
//
//	delete resRecvBuf2;
//
//	///step 5: compare FPInterval to split into A and B sets. 
//	for (size_t i = 0; i < len; ++i) {
//		int pos1 = posArr1[i];
//		
//		FPTYPE_A fpInterval1 = resRecvBuf1->get(pos1);
//		
//		int pos2 = posArr2[i];
//
//		FPTYPE_A fpInterval2 = resRecvBuf2->get(pos2);
//
//		if (fpInterval1 == fpInterval2 && pos1 + stride - 1 < mTotalLen && pos2 + stride - 1 < mTotalLen) {
//
//			posArr1[i] += stride;
//
//			posArr2[i] += stride;
//		}
//	}
//}
//
//#endif //KLCPA_RAM_MPI_H 
