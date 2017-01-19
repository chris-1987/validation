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
	stxxl::stats *Stats = stxxl::stats::get_instance();

	stxxl::stats_data stats_begin(*Stats);

	stxxl::block_manager *bm = stxxl::block_manager::get_instance();

	//
	uint64 len = BasicIO::file_size(t_fn);

	std::cerr << "Corpora Size: " << len << std::endl;

	//
	Validate4<uint8, uint16, uint40> validate4(t_fn, sa_fn, lcp_fn);

	// check
	if (false == validate4.run()) {

		std::cerr << "check--failed\n";
	}
	else {
	
		std::cerr << "check--passed\n";
	}

	std::cerr << (stxxl::stats_data(*Stats) - stats_begin);
	
	std::cerr << "Peak disk use: " << bm->get_maximum_allocation() << " per character: " << (double)bm->get_maximum_allocation() / len << std::endl;

	std::cerr << "I/O volume: " << Stats->get_written_volume() + Stats->get_read_volume() << " per character: " << ((double) Stats->get_written_volume() + Stats->get_read_volume()) / len << std::endl;		

}


