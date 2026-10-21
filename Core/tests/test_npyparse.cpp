#include "parse.h"


int main(int argc, char**argv) {
	npy_header nh;
	if (argc > 1) {
		parse_npy(argv[1], &nh);
		std::cout << argv[1] << " " << nh.X << " " << nh.Y << " " << nh.Z << " " << nh.isBigEndian << " " << nh.bit << " " << nh.isFortranOrder << " " << nh.header_length << " " << (short)nh.major << " " << (short)nh.minor << " " << nh.data_offset << std::endl;
        return 0;
	}
	else return -1;
}