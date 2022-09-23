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
