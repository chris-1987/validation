#include <iostream>

#include "lcpa_ram.h"

int main(int argc, char **argv) {

	//function code: 
	//	1: format elements (from uint40 to uint32) for SA and LCP arrays
	//	2: check full-order LCP array using the proposed probabilistic method 
	//	3: build K-order LCP array in RAM using the proposed probabilistic method
	//	4: check K-order LCP array in RAM using the brute-force method

	if (argc != 3) {
		std::cerr << "Please input <sFilePath/ saFilePath / lcpFilePath> <function code>\n";

		exit(0);
	}
	else {
	
		if (atoi(argv[2]) == 1) {

			std::cerr << "format .sa5 and .lcp5 to .sa and .lcp (convert uint40 to uint32)\n";

			std::string saFileName(argv[1]); saFileName += std::string(".sa5");

			std::string lcpFileName(argv[1]); lcpFileName += std::string(".lcp5");

			std::string saOutputFileName(argv[1]); saOutputFileName += std::string(".sa");

			std::string lcpOutputFileName(argv[1]); lcpOutputFileName += std::string(".lcp");

			typedef ExVector<uint40>::vector uint40_vector_type;

			typedef ExVector<uint32>::vector uint32_vector_type;

			//format sa	
			stxxl::syscall_file *saFile = new stxxl::syscall_file(saFileName,
				stxxl::syscall_file::RDWR | stxxl::syscall_file::DIRECT);

			uint40_vector_type *sa = new uint40_vector_type(saFile);

			uint40_vector_type::bufreader_type *saReader = new uint40_vector_type::bufreader_type(*sa);
	
			stxxl::syscall_file *saOutputFile = new stxxl::syscall_file(saOutputFileName,
				stxxl::syscall_file::CREAT | stxxl::syscall_file::RDWR | stxxl::syscall_file::DIRECT);

			uint32_vector_type *saOutput = new uint32_vector_type(saOutputFile);

			uint32_vector_type::bufwriter_type *saWriter = new uint32_vector_type::bufwriter_type(*saOutput);

			for (; !saReader->empty(); ++(*saReader)) {

				(*saWriter) << *(*saReader);
			}

			(*saWriter).finish();

			//format lcp
			stxxl::syscall_file *lcpFile = new stxxl::syscall_file(lcpFileName,
				stxxl::syscall_file::RDWR | stxxl::syscall_file::DIRECT);

			uint40_vector_type *lcp = new uint40_vector_type(lcpFile);

			uint40_vector_type::bufreader_type *lcpReader = new uint40_vector_type::bufreader_type(*lcp);

			stxxl::syscall_file *lcpOutputFile = new stxxl::syscall_file(lcpOutputFileName,
				stxxl::syscall_file::CREAT | stxxl::syscall_file::RDWR | stxxl::syscall_file::DIRECT);

			uint32_vector_type *lcpOutput = new uint32_vector_type(lcpOutputFile);

			uint32_vector_type::bufwriter_type *lcpWriter = new uint32_vector_type::bufwriter_type(*lcpOutput);

			for (; !lcpReader->empty(); ++(*lcpReader)) {

				(*lcpWriter) << *(*lcpReader);
			}
			
			(*lcpWriter).finish();

			delete lcpWriter; lcpWriter = NULL; delete lcpOutput; lcpOutput = NULL; delete lcpOutputFile; lcpOutputFile = NULL;

			delete lcpReader; lcpReader = NULL; delete lcp; lcp = NULL; delete lcpFile; lcpFile = NULL;

			delete saWriter; saWriter = NULL; delete saOutput; saOutput = NULL; delete saOutputFile; saOutputFile = NULL;

			delete saReader; saReader = NULL; delete sa; sa = NULL; delete saFile; saFile = NULL;
		}

		if (atoi(argv[2]) == 2) {

			std::cerr << "check full-order LCP array using the proposed probabilistic method\n" << std::endl;

			//compute len, avgLCP and maxLCP				
			std::string sFileName(argv[1]);

			std::string saFileName(argv[1]); saFileName += std::string(".sa");

			std::string lcpFileName(argv[1]); lcpFileName += std::string(".lcp");

			typedef ExVector<uint32>::vector uint32_vector_type;

			stxxl::syscall_file *lcpFile = new stxxl::syscall_file(lcpFileName,
				stxxl::syscall_file::RDWR | stxxl::syscall_file::DIRECT);

			uint32_vector_type *lcp = new uint32_vector_type(lcpFile);

			uint32_vector_type::bufreader_type *lcpReader = new uint32_vector_type::bufreader_type(*lcp);
			
			uint32 maxLCP = 0, len = 0;
		
			uint64 avgLCP = 0;

			++(*lcpReader); //skip lcp[0]

			++len;

			for ( ; !lcpReader->empty(); ++(*lcpReader), ++len) {
				
				if (maxLCP < *(*lcpReader)) {
		
					maxLCP = *(*lcpReader);
				}

				avgLCP += *(*lcpReader);
			}
		
			avgLCP /= (len - 1);
	
			std::cerr << "len: " << (len / 1024 / 1024) << " maxLCP: " << maxLCP << " avgLCP: " << avgLCP << std::endl;
		
			delete lcpReader; lcpReader = NULL; delete lcp; lcp = NULL; delete lcpFile; lcpFile = NULL;	
	
			lcpa_ram::LCPAChecker<uint8> lcpaChecker(sFileName, saFileName, lcpFileName, len, maxLCP);
		}

		if (atoi(argv[2]) == 3) {
			
			std::cerr << "build K-order LCP array using the proposed pobabilistic method\n" << std::endl;

			//build
			std::string sFileName(argv[1]);

			std::fstream *fs = new std::fstream(sFileName.c_str(), std::fstream::binary | std::fstream::in);

			fs->seekg(0, fs->end);
				
			uint32 len = fs->tellg();
	
			delete fs; fs = NULL;

			std::string saFileName(argv[1]); saFileName += std::string(".sa");

			std::string lcpFileName(argv[1]); lcpFileName += std::string(".klcp");
			
			std::cerr << "len: " << len << std::endl;
				
			lcpa_ram::LCPABuilder<uint8> lcpaBuilder(sFileName, saFileName, lcpFileName, len);
		}

		if (atoi(argv[2]) == 4) {
			
			std::cerr << "check K-order LCP array using the brute-force method\n" << std::endl;
			
			bool isRight = true;
	
			typedef ExVector<uint32>::vector uint32_vector_type;

			std::string lcpFileName(argv[1]); lcpFileName += std::string(".lcp");
			
			stxxl::syscall_file *lcpFile = new stxxl::syscall_file(lcpFileName, 
				stxxl::syscall_file::RDWR | stxxl::syscall_file::DIRECT);

			uint32_vector_type * lcp = new uint32_vector_type(lcpFile);

			uint32_vector_type::bufreader_type * lcpReader = new uint32_vector_type::bufreader_type(*lcp);

			std::string klcpFileName(argv[1]); klcpFileName += std::string(".klcp");
		
			stxxl::syscall_file *klcpFile = new stxxl::syscall_file(klcpFileName,
				stxxl::syscall_file::RDWR | stxxl::syscall_file::DIRECT);
			
			uint32_vector_type * klcp = new uint32_vector_type(klcpFile);

			uint32_vector_type::bufreader_type * klcpReader = new uint32_vector_type::bufreader_type(*klcp);
	
			++(*lcpReader), ++(*klcpReader); //skip the head invalid element
	
			for (uint32 i = 1; !lcpReader->empty(); ++i, ++(*lcpReader), ++(*klcpReader)) {

				if (*(*lcpReader) < *(*klcpReader)) {
					
					isRight = false;
					
					std::cerr << "wrong 1: " << i << " " << *(*lcpReader) << " " << *(*klcpReader) << std::endl;

					break;
				}
				else {

					if (*(*lcpReader) > *(*klcpReader) && *(*klcpReader) != K) {	

						isRight = false;

						std::cerr << "wrong 2: " << i << " " <<  *(*lcpReader) << " " << *(*klcpReader) << std::endl;

						break;
					}
				}
			}				
			
			if (isRight) {
				std::cerr << "check--passed\n";
			}
			else {
				std::cerr << "check--failed\n";
			}
		}
	}	


}
