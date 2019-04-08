
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
