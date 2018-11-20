#include <iostream>
#include <cmath>
#include <fstream> 
#include <vector>
#include <string>
#include <iostream>
#include <string>
#include <iostream>
//https://docs.scipy.org/doc/numpy/reference/generated/numpy.lib.format.html#module-numpy.lib.format

struct npy_header {
	int X, Y, Z;
	bool isBigEndian;
	short bit;
	bool isFortranOrder;
	int header_length;
	char * the_header;
	int major, minor;
	int data_offset;
};

int parse_npy(char*, npy_header *);
