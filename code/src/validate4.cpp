#include "validate4.h"

char* prog_name;

int main(int argc, char **argv) {

	if (argc != 4) {

		std::cerr << "Require 3 arguments: t_file, sa_file and lcp_file\n";
	}

	//
	std::string t_fn(argv[1]);

	std::string sa_fn(argv[2]);

	std::string lcp_fn(argv[3]);

	//
	Validate4<uint8, uint40> validate4(t_fn, sa_fn, lcp_fn);

	// check
	if (false == validate4.run()) {

		std::cerr << "check--failed\n";
	}
	else {
	
		std::cerr << "check--passed\n";
	}
}


