#include <iostream>

#include "klcpa_em.h"

int main(int argc, char **argv) {

	//function code: 
	//	1: build K-order LCP array in EM using the proposed probabilistic method
	//      2: check K-order LCP array in EM using the brute-force method
	//	3: check K-order LCP array in EM using the proposed probabilistic method

		
	if (argc != 3) {
		std::cerr << "Please input <sFilePath/ saFilePath / lcpFilePath> <function code>\n";

		exit(0);
	}
	else {

		if (atoi(argv[2]) == 1) {
			
			std::cerr << "build K-order LCP array using the proposed pobabilistic method\n" << std::endl;

			std::string sFileName(argv[1]);

			std::fstream *fs = new std::fstream(sFileName.c_str(), std::fstream::binary | std::fstream::in);

			fs->seekg(0, fs->end);
				
			size_t len = fs->tellg();
	
			delete fs; fs = NULL;

			std::string saFileName(argv[1]); saFileName += std::string(".sa5");

			std::string lcpFileName(argv[1]); lcpFileName += std::string(".klcp5");
			
			std::cerr << "len: " << len << std::endl;
				
			klcpa_em::KLCPABuilder<uint40, 8, 64> klcpaBuilder(sFileName, saFileName, lcpFileName, len);
		}

		if (atoi(argv[2]) == 2) {
	
			std::cerr << "check K-order LCP array using the brute force method\n" << std::endl;

			std::string srcLCPFileName(argv[1]); srcLCPFileName += std::string(".klcp5");
			
			std::string refLCPFileName(argv[1]); refLCPFileName += std::string(".lcp5");
			
			std::string saFileName(argv[1]); saFileName += std::string(".sa5");

			klcpa_em::KLCPABruteForceChecker<uint40> klcpaBruteForceChecker(srcLCPFileName, refLCPFileName, saFileName);
		}

		if (atoi(argv[2]) == 3) {
			
			std::cerr << "check K-order LCP array using the proposed probabilistic method\n" << std::endl;
			
			std::string sFileName(argv[1]);

			std::fstream *fs = new std::fstream(sFileName.c_str(), std::fstream::binary | std::fstream::in);

			fs->seekg(0, fs->end);
			
			size_t len = fs->tellg();

			delete fs; fs = NULL;
		
			std::string saFileName(argv[1]); saFileName += std::string(".sa5");
	
			std::string klcpFileName(argv[1]); klcpFileName += std::string(".klcp5");

			std::cerr << "len: " << len << std::endl;
		
			klcpa_em::KLCPAChecker<uint40> klcpaBuilder(sFileName, saFileName, klcpFileName, len);
		}
	}	


}
