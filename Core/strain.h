
#ifndef DVC_H
#define DVC_H

#include <string>

/******************************************************************************/
/******************************************************************************/


//
//	dvc.cpp
//
//	Copyright 2014 Brian K. Bay (computer code and all documentation)
//
//	Created:  1 Jan 2014

//	Revised:

	int 		day_rev = 25;
	std::string 	month_rev = "Nov";
	int 		year_rev = 2017;
	
	double version = 1.30;
//
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

	int nmp() const {return num_mod_para;}

	int ndp_min() const {return num_data_points_min;}
	int ndp_max() const {return num_data_points_max;}
	int ndp_def() const {return num_data_points_def;}

private:
	
	int num_mod_para;				// number of model parameters (terms in the polynomial)

	int num_data_points_min;		// min number for the strain window size
	int num_data_points_max;		// max number for the strain window size
	int num_data_points_def;		// default number for the strain window size

};
/******************************************************************************/


#endif


