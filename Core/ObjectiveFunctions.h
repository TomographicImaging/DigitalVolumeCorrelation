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
           Edoardo Pasca (UKRI-STFC)
*/
#ifndef OBJECTIVEFUNCTIONS_H
#define OBJECTIVEFUNCTIONS_H

#include <vector>
#include <cmath>
#include <iostream>
#include <iomanip>


/*****************************************************************************/
extern double obj_SAD(const std::vector<double> &ref_subvol, const std::vector<double> &tar_subvol);
extern double obj_SSD(const std::vector<double> &ref_subvol, const std::vector<double> &tar_subvol);
extern double obj_ZSSD(const std::vector<double> &ref_subvol, const std::vector<double> &tar_subvol);

extern double obj_NSSD(const std::vector<double> &ref_subvol, const std::vector<double> &tar_subvol);
extern double obj_ZNSSD(const std::vector<double> &ref_subvol, const std::vector<double> &tar_subvol);

// these overloaded versions return the residual vector as well as the overall objective function value

extern double obj_SAD(const std::vector<double> &ref_subvol, const std::vector<double> &tar_subvol, std::vector<double> &residual);
extern double obj_SSD(const std::vector<double> &ref_subvol, const std::vector<double> &tar_subvol, std::vector<double> &residual);
extern double obj_ZSSD(const std::vector<double> &ref_subvol, const std::vector<double> &tar_subvol, std::vector<double> &residual);

extern double obj_NSSD(const std::vector<double> &ref_subvol, const std::vector<double> &tar_subvol, std::vector<double> &residual);
extern double obj_ZNSSD(const std::vector<double> &ref_subvol, const std::vector<double> &tar_subvol, std::vector<double> &residual);

/******************************************************************************/

#endif
