/**
# -*- coding: utf-8 -*-
#   This work is part of the Core Imaging Library developed by
#   Visual Analytics and Imaging System Group of the Science Technology
#   Facilities Council, STFC

#   Copyright (C) 2018 CCPi
#
#   This program is free software: you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation, either version 3 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program.  If not, see <https://www.gnu.org/licenses/>.

#   Code is derived from code developed by Prof. Brian Bay
*/
/*
Matrix_4d is a key part of the interpolation strategy. It is instantiated with
enough room for voxel values, derivatives, and Lekien coefficients in a single
double vector. Indexing out/in is controlled through get/set return functions,
as well as size of the indices. Internal functions handle derivative calc's by
finite differences. Retrieval of derivative value blocks at corner nodes, used
in Lekien coeff calculations, is also available.

Following the advice of Lekien, a boolean structure is also used to indicate
whether the 64 Lekien coeff's have been calculated for a particular voxel cell.
Timing tests showed that by far the most intensive part of interpolation is the
[64x64][64x1] matrix multiply. Even the derivative calculation is minimal by
comparison. At the expense of storage, when an interp is instantiated, enough
room is created for vals, der's, and Lek coeff's (72 doubles per voxel in the
est_box). The voxel values are loaded, derivatives calculated, and bools set
indicating Lek coeff's not avaialble. Only when coefficients are needed during
a call to tri_cub_Lek are the coefficients for that particular cell determined.
The cell is then flagged as available, and any subsequent interpolation request
within that cell uses the already determined values.

This allows creation of a single interp structure that is physically large to
accomodate early search stages at low quality (linear) without the large penalty
of finding all the Lek coefficients, many of which will never be use during
later high quality (cubic) stages. It also allows a physically large but sparse
floating cloud without a huge Lek coefficient penalty. This will be useful for
coarse microstructures within samples.
*/
// [0] = the voxel value
// [1..7] = derivatives of voxel values
// [8..71] = Lekien coeff's
//

#ifndef MATRIX4D_H
#define MATRIX4D_H

#include <Eigen/Dense>
#include <Eigen/Sparse>

#include "CCPiDefines.h"

// using both or either creates 10x speedup with -O3 comp, haven't tracked down the specifics
using namespace Eigen;	
using namespace std;

/******************************************************************************/
class CCPI_EXPORT Matrix_4d
{
public:
	Matrix_4d(int nx, int ny, int nz);
	~Matrix_4d();

	double get(int x, int y, int z, int k) const
		{ return mat4d[x*nnyzk + y*nnzk + z*nnk + k];}

	void set(int x, int y, int z, int k, double val)
		{ mat4d[x*nnyzk + y*nnzk + z*nnk + k] = val;}

	bool get_Lc_avail(int x, int y, int z) const
		{ return Lc_avail[x*nny*nnz + y*nnz + z];}

	void set_Lc_avail(int x, int y, int z, bool status)
		{ Lc_avail[x*nny*nnz + y*nnz + z] = status;}

	int siz_x() { return nnx; }
	int siz_y() { return nny; }
	int siz_z() { return nnz; }
	int siz_k() { return nnk; }
	int Lek_offset() { return Lek_off; }

	double get_c0(int c0, int k) const { return mat4d[c0 + shift_c0 + k];}
	double get_c1(int c0, int k) const { return mat4d[c0 + shift_c1 + k];}
	double get_c2(int c0, int k) const { return mat4d[c0 + shift_c2 + k];}
	double get_c3(int c0, int k) const { return mat4d[c0 + shift_c3 + k];}

	double get_c4(int c0, int k) const { return mat4d[c0 + shift_c4 + k];}
	double get_c5(int c0, int k) const { return mat4d[c0 + shift_c5 + k];}
	double get_c6(int c0, int k) const { return mat4d[c0 + shift_c6 + k];}
	double get_c7(int c0, int k) const { return mat4d[c0 + shift_c7 + k];}

	void load_vb_k(Eigen::VectorXd &vb, const int c0, const int k);

	void set_c0_k(int c0, int k, double val) { mat4d[c0 + k] = val;}

// order = f,  f'(x), f'(y), f'(z), f''(xy), f''(xz), f''(yz), f'''(xyz)
//	   [0] [1]    [2]    [3]    [4]      [5]      [6]      [7]

	void set_c0_fsp(int c0) {
		mat4d[c0 + 1] = (mat4d[c0 + xh] - mat4d[c0 - xh])/2.0;
		mat4d[c0 + 2] = (mat4d[c0 + yh] - mat4d[c0 - yh])/2.0;
		mat4d[c0 + 3] = (mat4d[c0 + zh] - mat4d[c0 - zh])/2.0;
	}

	void set_c0_fdp(int c0) {
		mat4d[c0 + 4] = (mat4d[c0 + xh + yh] - mat4d[c0 - xh + yh]
			      +  mat4d[c0 - xh - yh] - mat4d[c0 + xh - yh])/4.0;

		mat4d[c0 + 5] = (mat4d[c0 + xh + zh] - mat4d[c0 - xh + zh]
			      +  mat4d[c0 - xh - zh] - mat4d[c0 + xh - zh])/4.0;

		mat4d[c0 + 6] = (mat4d[c0 + yh + zh] - mat4d[c0 - yh + zh]
			      +  mat4d[c0 - yh - zh] - mat4d[c0 + yh - zh])/4.0;
	}

	void set_c0_ftp(int c0) {
		double temp1 = (mat4d[c0 + xh + yh - zh] - mat4d[c0 - xh + yh - zh]
			     +  mat4d[c0 - xh - yh - zh] - mat4d[c0 + xh - yh - zh])/4.0;

		double temp2 = (mat4d[c0 + xh + yh + zh] - mat4d[c0 - xh + yh + zh]
			     +  mat4d[c0 - xh - yh + zh] - mat4d[c0 + xh - yh + zh])/4.0;

		mat4d[c0 + 7] = (temp2 - temp1)/2.0;
	}

private:

	double *mat4d; // Holds the actual data for the marix
	bool *Lc_avail; // Flag: Have Lek coeff's been computed at (x,y,z)?

	int nnx, nny, nnz, nnk; // Length of the matrix in each dimension
	int nnzk;               // Area of the zk plane
	int nnyzk;              // Volume of the yzk space
	int Lek_off;            // Index of the first Lekien coefficient in dimension k

	int xh, yh, zh; // mat4d bytes between coefficients in a given spatial dimension

 // Byte distance in mat4d between the corners of the Leiken cube
	int shift_c0, shift_c1, shift_c2, shift_c3;
	int shift_c4, shift_c5, shift_c6, shift_c7;

};
/******************************************************************************/

#endif
