#include <iostream>

#include "lcpa_mpi.h"

int main(int argc, char **argv) {

	MPI_Init(&argc, &argv);

	int commRank; MPI_Comm_rank(MPI_COMM_WORLD, &commRank);

	int commSize; MPI_Comm_size(MPI_COMM_WORLD, &commSize);

	if (commSize != 3) {
	
		std::cerr << "commSize must be 3.\n";
		
		exit(0);
	}

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

			sFileName += "." + std::to_string(commRank);			

			std::fstream *fs = new std::fstream(sFileName.c_str(), std::fstream::binary | std::fstream::in);

			fs->seekg(0, fs->end);
			
			size_t len = fs->tellg();

			delete fs; fs = NULL;
		
			std::string saFileName(argv[1]); saFileName += std::string(".sa5");

			saFileName += "." + std::to_string(commRank);			
	
			std::string lcpFileName(argv[1]); lcpFileName += std::string(".lcp5");
	
			lcpFileName += "." + std::to_string(commRank);

			std::cerr << "len: " << len << std::endl;
		
			lcpa_mpi::LCPAChecker<uint40> lcpaChecker(sFileName, saFileName, lcpFileName, len, commRank, commSize);
		}
	}	
	
	MPI_Finalize();

	return 0;
}
