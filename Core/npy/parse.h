/*
Copyright 2018 United Kingdom Research and Innovation
Copyright 2018 Oregon State University

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.

Author(s): Edoardo Pasca (UKRI-STFC)
*/
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
