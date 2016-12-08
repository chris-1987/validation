#include <iostream>

#include "lcpa_em2.h"

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
			
			std::string sFileName(std::string("/home/chris/sac-corpus/").append(argv[1]));

			std::fstream *fs = new std::fstream(sFileName.c_str(), std::fstream::binary | std::fstream::in);

			fs->seekg(0, fs->end);
			
			size_t len = fs->tellg();

			delete fs; fs = NULL;
		
			std::string saFileName = sFileName; saFileName += std::string(".sa5");
	
			std::string lcpFileName = sFileName; lcpFileName += std::string(".lcp5");

			lcpa_em2::LCPAChecker<uint40, 256> lcpaChecker(sFileName, saFileName, lcpFileName, len);
		}
	}	


}
