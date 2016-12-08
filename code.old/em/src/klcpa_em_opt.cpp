#include <iostream>

#include "klcpa_em_opt.h"

int main(int argc, char **argv) {

	//function code: 
	//	1: build K-order LCP array in EM using the proposed probabilistic method

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

							
			klcpa_em::KLCPABuilder<uint40, uint16, 8, 128, 256> KLCPABuilder(sFileName, saFileName, lcpFileName, len);
		}
	}	
}
