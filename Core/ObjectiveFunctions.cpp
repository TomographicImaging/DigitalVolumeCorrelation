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

*/
#include "ObjectiveFunctions.h"

/******************************************************************************/
double obj_SAD(const std::vector<double> &ref_subvol, const std::vector<double> &tar_subvol)
{
	double obj = 0.0;
	for (unsigned int i=0; i<ref_subvol.size(); i++) {
		double diff = tar_subvol[i] - ref_subvol[i];
		obj += fabs(diff);
	}

	return obj;
}
/******************************************************************************/
double obj_SAD(const std::vector<double> &ref_subvol, const std::vector<double> &tar_subvol, std::vector<double> &residual)
{
	double obj = 0.0;
	for (unsigned int i=0; i<ref_subvol.size(); i++) {
		double diff = tar_subvol[i] - ref_subvol[i];
		residual[i] = diff;
		obj += fabs(diff);
	}

	return obj;
}
/******************************************************************************/
double obj_SSD(const std::vector<double> &ref_subvol, const std::vector<double> &tar_subvol)
// Pan, Equivalence of Digital Image Correlation Criteria for Pattern Matching, 2010
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
// Pan, Equivalence of Digital Image Correlation Criteria for Pattern Matching, 2010
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
// Pan, Equivalence of Digital Image Correlation Criteria for Pattern Matching, 2010
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
// Pan, Equivalence of Digital Image Correlation Criteria for Pattern Matching, 2010
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
// Pan, Equivalence of Digital Image Correlation Criteria for Pattern Matching, 2010
// subvolume values scaled by sqrt_of_sum_of_squared values
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

	obj /= 2.0;	// this scales results between 0.0 (perfect match) and 1.0 (dark vs bright subvolumes)
			// two random full-range subvolumes produce an objective value of 0.5

	return obj;
}
/******************************************************************************/
double obj_NSSD(const std::vector<double> &ref_subvol, const std::vector<double> &tar_subvol, std::vector<double> &residual)
// Pan, Equivalence of Digital Image Correlation Criteria for Pattern Matching, 2010
// subvolume values scaled by sqrt_of_sum_of_squared values
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

	obj /= 2.0;	// this scales results between 0.0 (perfect match) and 1.0 (dark vs bright subvolumes)
			// two random full-range subvolumes produce an objective value of 0.5

	return obj;
}
/******************************************************************************/
double obj_ZNSSD(const std::vector<double> &ref_subvol, const std::vector<double> &tar_subvol)
// Pan, Equivalence of Digital Image Correlation Criteria for Pattern Matching, 2010
// subvolume mean_offset_values scaled by sqrt_of_sum_of_mean_offset_squared values
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

	obj /= 4.0;	// this scales results between 0.0 (perfect match) and 1.0 (dark vs bright subvolumes)
			// two random full-range subvolumes produce an objective value of 2.0
	return obj;
}
/******************************************************************************/
double obj_ZNSSD(const std::vector<double> &ref_subvol, const std::vector<double> &tar_subvol, std::vector<double> &residual)
// Pan, Equivalence of Digital Image Correlation Criteria for Pattern Matching, 2010
// subvolume mean_offset_values scaled by sqrt_of_sum_of_mean_offset_squared values
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

	obj /= 4.0;	// this scales results between 0.0 (perfect match) and 1.0 (dark vs bright subvolumes)
			// two random full-range subvolumes produce an objective value of 2.0
	return obj;
}
/******************************************************************************/
