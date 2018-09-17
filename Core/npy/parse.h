#include <iostream>
#include <cmath>
#include <fstream> 
#include <vector>
#include <string>
#include <iostream>
#include <string>
#include <iostream>

struct npy_header {
	int X, Y, Z;
	bool isBigEndian;
	short bit;
	bool isFortranOrder;
	int header_length;
	char * the_header;
	int major, minor;


};

int parse_npy(char*, npy_header *);
