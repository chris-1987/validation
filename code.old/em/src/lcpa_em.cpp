#include <iostream>

#include "lcpa_em.h"

int main(int argc, char **argv) {

	//function code: 
	//	1: check full-order LCP array in EM using the proposed probabilistic method

		
	if (argc != 3) {
		std::cerr << "Please input <sFilePath/ saFilePath / lcpFilePath> <function code>\n";

		exit(0);
	}
	else {

		if (atoi(argv[2]) == 1) {
			
			std::cerr << "check full-order LCP array using the proposed probabilistic method\n" << std::endl;
			
			std::string sFileName(argv[1]);

			std::fstream *fs = new std::fstream(sFileName.c_str(), std::fstream::binary | std::fstream::in);

			fs->seekg(0, fs->end);
			
			size_t len = fs->tellg();

			delete fs; fs = NULL;
		
			std::string saFileName(argv[1]); saFileName += std::string(".sa5");
	
			std::string lcpFileName(argv[1]); lcpFileName += std::string(".lcp5");

			std::cerr << "len: " << len << std::endl;
		
			lcpa_em::LCPAChecker<uint40> lcpaChecker(sFileName, saFileName, lcpFileName, len);
		}
	}	


}
