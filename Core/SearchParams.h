/*
Copyright 2018 United Kingdom Research and Innovation
Copyright 2018 Oregon State University

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

Author(s): Brian Bay,
           Shrikanth Nagella,
		   
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
