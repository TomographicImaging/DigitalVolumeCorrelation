
#ifndef DVC_H
#define DVC_H

#include <string>

/******************************************************************************/
/******************************************************************************/

	
/******************************************************************************/
/******************************************************************************/

//#include <string>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <limits>
#include <cstdlib>
#include <chrono>
#include <cmath>
#include <stdint.h>
#include <omp.h>
#include <thread>
#include <algorithm>

// adjust Makefile if changes made here
#include "InputRead.h"
#include "DataCloud.h"
#include "Search.h"
#include "Utility.h"
//

/******************************************************************************/
class CCPI_EXPORT StrainCalc
{
public:
	StrainCalc();

	int find_flag(std::string flag, int &argc, char *argv[]);
	int find_flag(std::string flag, int &argc, char *argv[], int &val);
	int find_flag(std::string flag, int &argc, char *argv[], double &val);
	int find_flag(size_t pos, size_t len, std::string flag, int &argc, char *argv[]);

	int nmp() const {return num_mod_para;}

	int ndp_min() const {return num_data_points_min;}
	int ndp_def() const {return num_data_points_def;}

	double objt_min() const {return objmin_thresh_min;}
	double objt_max() const {return objmin_thresh_max;}
	double objt_def() const {return objmin_thresh_def;}

private:
	
	int num_mod_para;				// number of model parameters (terms in the polynomial)

	int num_data_points_min;		// min number for the strain window size
	int num_data_points_def;		// default number for the strain window size
	// max comes from DataCloud .sort file setting

	double objmin_thresh_min;		// min for objmin threshold
	double objmin_thresh_max;		// max for objmin threshold
	double objmin_thresh_def;		// default for objmin threshold


};
/******************************************************************************/


#endif


