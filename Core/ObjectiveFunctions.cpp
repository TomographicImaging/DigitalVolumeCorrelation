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

*/
#include "ObjectiveFunctions.h"

/******************************************************************************/
double obj_SSD(const std::vector<double> &ref_subvol, const std::vector<double> &tar_subvol)
// SSD (Sum of Squared Differences)
{
	double obj = 0.0;
	for (unsigned int i=0; i<ref_subvol.size(); i++) {
		double diff = tar_subvol[i] - ref_subvol[i];
		obj += diff*diff;
	}

	return obj;
}
/******************************************************************************/
double obj_SSD(const std::vector<double> &ref_subvol, const std::vector<double> &tar_subvol, std::vector<double> &residual)
// SSD (Sum of Squared Differences)
{
	double obj = 0.0;
	for (unsigned int i=0; i<ref_subvol.size(); i++) {
		double diff = tar_subvol[i] - ref_subvol[i];
		residual[i] = diff;
		obj += diff*diff;
	}

	return obj;
}
/******************************************************************************/
double obj_ZSSD(const std::vector<double> &ref_subvol, const std::vector<double> &tar_subvol)
// ZSSD (Zero-mean Sum of Squared Differences) with brightness normalization
// average value of subregion subtracted from each voxel to normalize brightness differences between ref and cor
// doe not have a normalized scaling of the objective funciton value, it changes with overall voxel magnitudes
{
	double obj = 0.0;
	double diff = 0.0;
	double avg_ref = 0.0;
	double avg_tar = 0.0;

	int n = ref_subvol.size();

	for (unsigned int i=0; i<n; i++) {
		avg_ref += ref_subvol[i];
		avg_tar += tar_subvol[i];
	}

	avg_ref = avg_ref/n;
	avg_tar = avg_tar/n;

	for (unsigned int i=0; i<n; i++) {
		diff = (tar_subvol[i]-avg_tar) - (ref_subvol[i]-avg_ref);
		obj += diff*diff;
	}

	return obj;
}
/******************************************************************************/
double obj_ZSSD(const std::vector<double> &ref_subvol, const std::vector<double> &tar_subvol, std::vector<double> &residual)
// ZSSD (Zero-mean Sum of Squared Differences) with brightness normalization
// average value of subregion subtracted from each voxel to normalize brightness differences between ref and cor
// doe not have a normalized scaling of the objective function value, it changes with overall voxel magnitudes
{
	double obj = 0.0;
	double diff = 0.0;
	double avg_ref = 0.0;
	double avg_tar = 0.0;

	int n = ref_subvol.size();

	for (unsigned int i=0; i<n; i++) {
		avg_ref += ref_subvol[i];
		avg_tar += tar_subvol[i];
	}

	avg_ref = avg_ref/n;
	avg_tar = avg_tar/n;

	for (unsigned int i=0; i<n; i++) {
		diff = (tar_subvol[i]-avg_tar) - (ref_subvol[i]-avg_ref);
		residual[i] = diff;
		obj += diff*diff;
	}

	return obj;
}
/******************************************************************************/
double obj_NSSD(const std::vector<double> &ref_subvol, const std::vector<double> &tar_subvol)
// NSSD (Normalized Sum of Squared Differences) with contrast normalization
// divided by the vector norm (energy/variance) of the voxel intensities
// objective function values are normalized for overall brigntness changes and are comparable
{
	double obj = 0.0;
	double diff = 0.0;
	double sqrt_sum_ref_sqr = 0.0;
	double sqrt_sum_tar_sqr = 0.0;

	int n = ref_subvol.size();

	for (unsigned int i=0; i<n; i++) {
		sqrt_sum_ref_sqr += ref_subvol[i]*ref_subvol[i];
		sqrt_sum_tar_sqr += tar_subvol[i]*tar_subvol[i];
	}

	sqrt_sum_ref_sqr = sqrt(sqrt_sum_ref_sqr);
	sqrt_sum_tar_sqr = sqrt(sqrt_sum_tar_sqr);

	for (unsigned int i=0; i<n; i++) {
		diff = tar_subvol[i]/sqrt_sum_tar_sqr - ref_subvol[i]/sqrt_sum_ref_sqr;
		obj += diff*diff;
	}

	obj /= 2.0;	// two random full-range subvolumes produce an objective value of 2.0
			

	return obj;
}
/******************************************************************************/
double obj_NSSD(const std::vector<double> &ref_subvol, const std::vector<double> &tar_subvol, std::vector<double> &residual)
// NSSD (Normalized Sum of Squared Differences) with contrast normalization
// divided by the vector norm (energy/variance) of the voxel intensities
// objective function values are normalized for overall brigntness changes and are comparable
{
	double obj = 0.0;
	double diff = 0.0;
	double sqrt_sum_ref_sqr = 0.0;
	double sqrt_sum_tar_sqr = 0.0;

	int n = ref_subvol.size();

	for (unsigned int i=0; i<n; i++) {
		sqrt_sum_ref_sqr += ref_subvol[i]*ref_subvol[i];
		sqrt_sum_tar_sqr += tar_subvol[i]*tar_subvol[i];
	}

	sqrt_sum_ref_sqr = sqrt(sqrt_sum_ref_sqr);
	sqrt_sum_tar_sqr = sqrt(sqrt_sum_tar_sqr);

	for (unsigned int i=0; i<n; i++) {
		diff = tar_subvol[i]/sqrt_sum_tar_sqr - ref_subvol[i]/sqrt_sum_ref_sqr;
		residual[i] = diff;
		obj += diff*diff;
	}

	obj /= 2.0;	// two random full-range subvolumes produce an objective value of 2.0

	return obj;
}
/******************************************************************************/
double obj_ZNSSD(const std::vector<double> &ref_subvol, const std::vector<double> &tar_subvol)
// ZNSSD (Normalized Zero-mean Sum of Squared Differences) with brightness and contrast normalization
{
	double obj = 0.0;
	double diff = 0.0;
	double avg_ref = 0.0;
	double avg_tar = 0.0;
	double sqrt_sum_bar_ref_sqr = 0.0;
	double sqrt_sum_bar_tar_sqr = 0.0;

	int n = ref_subvol.size();

	for (unsigned int i=0; i<n; i++) {
		avg_ref += ref_subvol[i];
		avg_tar += tar_subvol[i];
	}

	avg_ref = avg_ref/n;
	avg_tar = avg_tar/n;

	for (unsigned int i=0; i<n; i++) {
		sqrt_sum_bar_ref_sqr += (ref_subvol[i]-avg_ref)*(ref_subvol[i]-avg_ref);
		sqrt_sum_bar_tar_sqr += (tar_subvol[i]-avg_tar)*(tar_subvol[i]-avg_tar);
	}

	sqrt_sum_bar_ref_sqr = sqrt(sqrt_sum_bar_ref_sqr);
	sqrt_sum_bar_tar_sqr = sqrt(sqrt_sum_bar_tar_sqr);

	for (unsigned int i=0; i<n; i++) {
		diff = ((tar_subvol[i]-avg_tar)/sqrt_sum_bar_tar_sqr) - ((ref_subvol[i]-avg_ref)/sqrt_sum_bar_ref_sqr);
		obj += diff*diff;
	}

	obj /= 2.0;	// two random full-range subvolumes produce an objective value of 2.0
				
	return obj;
}
/******************************************************************************/
double obj_ZNSSD(const std::vector<double> &ref_subvol, const std::vector<double> &tar_subvol, std::vector<double> &residual)
// ZNSSD (Normalized Zero-mean Sum of Squared Differences) with brightness and contrast normalization
{
	double obj = 0.0;
	double diff = 0.0;
	double avg_ref = 0.0;
	double avg_tar = 0.0;
	double sqrt_sum_bar_ref_sqr = 0.0;
	double sqrt_sum_bar_tar_sqr = 0.0;

	int n = ref_subvol.size();

	for (unsigned int i=0; i<n; i++) {
		avg_ref += ref_subvol[i];
		avg_tar += tar_subvol[i];
	}

	avg_ref = avg_ref/n;
	avg_tar = avg_tar/n;

	for (unsigned int i=0; i<n; i++) {
		sqrt_sum_bar_ref_sqr += (ref_subvol[i]-avg_ref)*(ref_subvol[i]-avg_ref);
		sqrt_sum_bar_tar_sqr += (tar_subvol[i]-avg_tar)*(tar_subvol[i]-avg_tar);
	}

	sqrt_sum_bar_ref_sqr = sqrt(sqrt_sum_bar_ref_sqr);
	sqrt_sum_bar_tar_sqr = sqrt(sqrt_sum_bar_tar_sqr);

	for (unsigned int i=0; i<n; i++) {
		diff = ((tar_subvol[i]-avg_tar)/sqrt_sum_bar_tar_sqr) - ((ref_subvol[i]-avg_ref)/sqrt_sum_bar_ref_sqr);
		residual[i] = diff;
		obj += diff*diff;
	}

	obj /= 2.0;	// two random full-range subvolumes produce an objective value of 2.0
				
	return obj;
}
/******************************************************************************/
