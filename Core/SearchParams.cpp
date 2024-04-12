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
           Shrikanth Nagella (UKRI-STFC)
*/
#include "SearchParams.h"

/******************************************************************************/
SearchParams::SearchParams ()
{
	for (int i=0; i<12; i++) par_vect_all.push_back(0.0);

	std::vector<double> row;

	for (int i=0; i<3; i++) row.push_back(0.0);

	for (int i=0; i<3; i++) par_matr_rot.push_back(row);

	for (int i=0; i<3; i++) par_tens_str.push_back(row);

	// all zero is default for the strain tensor
	// identity is default for the rotation matrix

	par_matr_rot[0][0] = 1;
	par_matr_rot[0][1] = 0;
	par_matr_rot[0][2] = 0;

	par_matr_rot[1][0] = 0;
	par_matr_rot[1][1] = 1;
	par_matr_rot[1][2] = 0;

	par_matr_rot[2][0] = 0;
	par_matr_rot[2][1] = 0;
	par_matr_rot[2][2] = 1;

}
/******************************************************************************/
void SearchParams::set_param_vect (const std::vector<double> &t_vect, int ndof)
{
	// translation is always moving and set
	// (u,v,w)
	// (0,1,2)
	for (unsigned int i=0; i<t_vect.size(); i++)
		par_vect_all[i] = t_vect[i];

	// calculate the rotation matrix
	//(phi,the,psi)
	//( 3,  4,  5 ) -> term numbering in the complete vector
	if (ndof > 3) {

		// test out scaling
//		double phi = 0.1*par_vect_all[3];
//		double the = 0.1*par_vect_all[4];
//		double psi = 0.1*par_vect_all[5];

		// unscaled
		double phi = par_vect_all[3];
		double the = par_vect_all[4];
		double psi = par_vect_all[5];

		// roll pitch yaw
		par_matr_rot[0][0] =   cos(the)*cos(phi);
		par_matr_rot[0][1] =   cos(the)*sin(phi);
		par_matr_rot[0][2] =  -sin(the);

		par_matr_rot[1][0] =  sin(psi)*sin(the)*cos(phi) - cos(psi)*sin(phi);
		par_matr_rot[1][1] =  sin(psi)*sin(the)*sin(phi) + cos(psi)*cos(phi);
		par_matr_rot[1][2] =  cos(the)*sin(psi);

		par_matr_rot[2][0] =  cos(psi)*sin(the)*cos(phi) + sin(psi)*sin(phi);
		par_matr_rot[2][1] =  cos(psi)*sin(the)*sin(phi) - sin(psi)*cos(phi);
		par_matr_rot[2][2] =  cos(the)*cos(psi);
	}

	// fill the strain tensor
	//(exx,eyy,ezz,exy,eyz,exz)
	//( 6,  7,  8,  9,  10, 11) -> term numbering in the complete vector
	if (ndof > 6) {
		par_tens_str[0][0] = par_vect_all[6];
		par_tens_str[0][1] = par_vect_all[9];
		par_tens_str[0][2] = par_vect_all[11];

		par_tens_str[1][0] = par_vect_all[9];
		par_tens_str[1][1] = par_vect_all[7];
		par_tens_str[1][2] = par_vect_all[10];

		par_tens_str[2][0] = par_vect_all[11];
		par_tens_str[2][1] = par_vect_all[10];
		par_tens_str[2][2] = par_vect_all[8];
	}

}
/******************************************************************************/
void SearchParams::echo_all ()
{
	std::cout << "(u,v,w,phi,the,psi,exx,eyy,ezz,exy,eyz,exz)"<< std::endl;

	for (unsigned int i=0; i<par_vect_all.size(); i++)
		std::cout << par_vect_all[i] << " ";
	std::cout << std::endl<< std::endl;

	std::cout << "roll-pitch-yaw" << std::endl;
	std::cout << "    [ cphi sphi  0][cthe 0  -sthe][1    0    0 ]" << std::endl;
	std::cout << "A = [-sphi cphi  0][ 0   1    0  ][0  cpsi spsi]" << std::endl;
	std::cout << "    [  0    0    1][sthe 0   cthe][0 -spsi cpsi]" << std::endl;

	for (unsigned int i=0; i<par_matr_rot.size(); i++) {
		for (unsigned int j=0; j<par_matr_rot[i].size(); j++){
			std::cout << par_matr_rot[i][j] << " ";
		}
		std::cout << std::endl;
	}
	std::cout << std::endl;

	std::cout << std::endl;
	std::cout << "[exx exy exz]" << std::endl;
	std::cout << "[exy eyy eyz]" << std::endl;
	std::cout << "[exz eyz ezz]" << std::endl;

	for (unsigned int i=0; i<par_tens_str.size(); i++) {
		for (unsigned int j=0; j<par_tens_str[i].size(); j++){
			std::cout << par_tens_str[i][j] << " ";
		}
		std::cout << std::endl;
	}
	std::cout << std::endl;
}
/******************************************************************************/

