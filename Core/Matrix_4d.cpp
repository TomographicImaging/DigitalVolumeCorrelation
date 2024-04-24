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

Author(s): Brian Bay (OSU)
           Srikanth Nagella (UKRI-STFC)

*/
#include "Matrix_4d.h"

/******************************************************************************/
Matrix_4d::Matrix_4d(int nx, int ny, int nz)
{
	nnk = 72;	// total number of values stored at each point
	Lek_off = 8;	// offset to the 1st Lekien coefficient

	nnx = nx;
	nny = ny;
	nnz = nz;

	nnzk = nnz*nnk;
	nnyzk = nny*nnz*nnk;

	xh =  nnyzk;
	yh =  nnzk;
	zh =  nnk;

	mat4d = new double[nnx*nny*nnz*nnk];
	Lc_avail = new bool[nnx*nny*nnz];

	std::fill(Lc_avail, Lc_avail+(nnx*nny*nnz), false);

	// shifts within Matrix_4d to corners wrt the c0 entry, see Lekien

	shift_c0 = 0;				// (ic,ir,is,k)
	shift_c1 = nny*nnz*nnk;			// (ic+1,ir,is,k)
	shift_c2 = nnz*nnk;			// (ic,ir+1,is,k)
	shift_c3 = nny*nnz*nnk + nnz*nnk;	// (ic+1,ir+1,is,k)

	shift_c4 = shift_c0 + nnk;		// (ic,ir,is+1,k)
	shift_c5 = shift_c1 + nnk;		// (ic+1,ir,is+1,k)
	shift_c6 = shift_c2 + nnk;		// (ic,ir+1,is+1,k)
	shift_c7 = shift_c3 + nnk;		// (ic+1,ir+1,is+1,k)
}
/******************************************************************************/
Matrix_4d::~Matrix_4d()
{
	delete[] mat4d;
	delete[] Lc_avail;
}
/******************************************************************************/
void Matrix_4d::load_vb_k(Eigen::VectorXd &vb, const int c0, const int k)
// puts kernel k for each corner associated with c0 into vb, see Lekien
{
	vb(k*8+0) = mat4d[c0 + shift_c0 + k];
	vb(k*8+1) = mat4d[c0 + shift_c1 + k];
	vb(k*8+2) = mat4d[c0 + shift_c2 + k];
	vb(k*8+3) = mat4d[c0 + shift_c3 + k];

	vb(k*8+4) = mat4d[c0 + shift_c4 + k];
	vb(k*8+5) = mat4d[c0 + shift_c5 + k];
	vb(k*8+6) = mat4d[c0 + shift_c6 + k];
	vb(k*8+7) = mat4d[c0 + shift_c7 + k];
}
/******************************************************************************/


