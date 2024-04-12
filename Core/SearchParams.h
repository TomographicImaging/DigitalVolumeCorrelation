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
SearchParams manages the degrees-of-freedom associated with template matching.
Some uses of the transform parameters will want a single vector, some will
want blocks of values. The concept is to interact with a simple vector form
with optimization procedures, but maintain other forms with consistent content.

A system for controlling activation/deactiviation of parameter groups (e.g.
translation and rotation, but not strain) is also needed.

It could be organized so that the only form an outside class can change
directly is the simple vector form, which will internally set the other forms,
those avaialble through return (but not setting) functions.
*/

#ifndef SEARCHPARAMS_H
#define SEARCHPARAMS_H

#include <iostream>
#include <cmath>
#include <vector>
#include "CCPiDefines.h"

/******************************************************************************/
class CCPI_EXPORT SearchParams {
public:
	SearchParams ();

	void set_param_vect (const std::vector<double> &t_vect, int ndof);

	std::vector<double> vect_all() const {return par_vect_all;}

	std::vector< std::vector<double> > matr_rot() const
		{return par_matr_rot;}

	std::vector< std::vector<double> > tens_str() const
		{return par_tens_str;}

	void echo_all ();

private:
	 //(u,v,w,phi,the,psi,exx,eyy,ezz,exy,eyz,exz), angles in rad, nondim strain
	std::vector<double> par_vect_all;

	// Taken from Wolfram convention: A = BCD (pitch-roll-yaw)
	//     [ cphi sphi  0][cthe    0    -sthe][ 1     0    0  ]
	// A = [-sphi cphi  0][ 0      1      0  ][ 0    cpsi spsi]
	//     [  0    0    1][sthe    0     cthe][ 0   -spsi cpsi]
	std::vector< std::vector<double> > par_matr_rot;

	// [exx exy exz]
	// [exy eyy eyz]
	// [exz eyz ezz]
	std::vector< std::vector<double> > par_tens_str;
};
/******************************************************************************/

#endif
