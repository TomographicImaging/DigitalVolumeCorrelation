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
           Srikanth Nagella (UKRI-STFC)
           Edoardo Pasca (UKRI-STFC)
*/
#include "dvc.h"

/******************************************************************************/
void echo_vect_upto(std::vector<double> vect, unsigned int n)
{
	std::cout << std::setprecision(6) << std::fixed;

	for (unsigned int i=0; i<vect.size(); i++)
		if (i<n) std::cout << vect[i] << "\t";

}
/******************************************************************************/
int main(int argc, char *argv[])
{
	int nProcessors = omp_get_num_procs();

	int OMP_NUM_THREADS = omp_get_max_threads();

	if (omp_get_max_threads() > nProcessors || !OMP_NUM_THREADS)
	{
		omp_set_num_threads(nProcessors);
	}

#pragma omp parallel
	{
		if (omp_get_thread_num() == 0)
		{
			cout << endl << "Running with threads = " << omp_get_num_threads() << endl;
		}
	}

	std::cout << "Running dvc built from bspline_interp_merge_master branch" << std::endl;

	InputRead in;

	std::string help("help");
	std::string example("example");
	std::string manual("manual");
	std::string form("form");
	std::string pversion("version");

	// command entered with no arguments
	if (argc == 1) {
		std::cout << endl;
		std::cout << "Options:" << endl;
		std::cout << "dvc \t\t\t// write this list to the terminal window" << endl;
		std::cout << "dvc dvc_in\t\t// execute dvc code with dvc_in controlling the run" << endl;
		std::cout << "dvc version\t\t// print dvc code version to the terminal" << endl;
		std::cout << "dvc help\t\t// provide additional detail about running the dvc code" << endl;
		std::cout << "dvc example\t\t// write dvc_in_example.txt, with brief keyword descriptions, in the working directory" << endl;
		std::cout << "dvc manual\t\t// write DVC_manual.html, with more detailed information, in the working directory" << endl;
		std::cout << "dvc form\t\t// write DVC_input_builder.html, a tool for creating a dvc_input file, in the working directory" << endl;
		std::cout << endl;
		return 0;
	}

	// trap dvc version on the command line, prints and returns
	if (argv[1] == pversion){
		return in.print_current_version();
	}

	// trap help on the command line
	if (argv[1] == help) {
		std::cout << endl;
		std::cout << "For general dvc code information enter 'dvc manul' to generate DVC_manual.html." << endl;
		std::cout << "Program execution is controlled by a key_word based input file." << endl;
		std::cout << "Please refer to an example input file (enter 'dvc example') for content and format." << endl;
		std::cout << "Or type 'dvc form' to generate an interactive tool (DVC_input_builder.html) for creating input files." << endl;
		std::cout << "To run the code type dvc followed by the name of the input file." << endl;
		std::cout << "The input file is evaluated and, if all is good, the run starts." << endl;
		std::cout << "Common problems are incorrect file paths, missing keywords, and invalid parameters." << endl;
		std::cout << "A message is sent to the console window if an input file problem is encountered." << endl;
		std::cout << "Program execution can be lengthy for large point clouds." << endl;
		std::cout << "Results are written to output_filename.disp during execution." << endl;
		std::cout << "Information about the run is written to output_filename.stat during execution." << endl;
		std::cout << "Both files are simple text, and can be opened and viewed at any time." << endl;
		std::cout << endl;
		return 0;
	}

	// trap example on the command line
	if (argv[1] == example) {
		std::cout << "\ndvc_in_example.txt printed in the current working directory" << endl << endl;
		std::ofstream dvc_inp("dvc_in_example.txt");
		in.print_input_example(dvc_inp, "fio_name");
		in.print_input_example(dvc_inp, "vox_data");
		in.print_input_example(dvc_inp, "sub_vols");
		in.print_input_example(dvc_inp, "opt_mthd");
		in.print_input_example(dvc_inp, "opt_tune");
		dvc_inp.seekp(0, dvc_inp.beg);
		dvc_inp.close();
		return 0;
	}

	// trap manual on the command line
	if (argv[1] == manual) {
		// this is the new and improved html manual
		std::cout << "\nDVC_manual.html written to the current working directory" << endl << endl;
    	std::ofstream html("DVC_manual.html");
    	in.print_manual_html(html);
		return 0;
	}

	// trap form on the command line
	if (argv[1] == form) {
		std::cout << "\nDVC_input_builder.html written to the current working directory" << endl << endl;
		std::ofstream f("DVC_input_builder.html", std::ios::binary);
		in.print_input_form_html(f);
		return 0;
	}

	// check and process command line flags that proceed to run
	int map_label;
	bool map_on = false;
	if (in.find_flag("-map", argc, argv, map_label)) {	
		map_on = true;
		std::cout << std::endl << "** mapping point with label " << map_label << std::endl;
	}

	// check to see if the command line contains an accessible file
	std::string fname(argv[1]);
	if(!in.input_file_accessible(fname)) return 0;

	// instantiate storage for run control parameters within Utility
	RunControl run;
	// parse and check the input file, fill RunControl
	if(!in.input_file_read(&run)) return 0;

	// instantiate a DataCloud
	DataCloud data;
	// read point could file
	if(!in.read_point_cloud_generalized(&run, data.labels, data.points, data.added_columns, data.added_column_names)) return 0;
	std::cout << std::endl << "cloud contains " << data.points.size() << " points" << std::endl;
	// sort overall by distance from global starting_point and establish neighbors
	data.organize_cloud(&run);		

	// ***** begin run *****

	// create optimizer from the input parameters
	Search optimize = Search(&run);

	// operator << for Search defined in Search.h/.cpp, returns std::ostream summary of the run parameters for output to terminal
	std::cout << optimize << std::endl << std::endl;

	// establish results file for output while running, in case of interupt
	in.result_header(run.res_fname, optimize.par_min.size());

	int count = 0;			// required for handling of partial runs, num_points_to_process set
	int count_good = 0;
	int count_range = 0;
	int count_convg = 0;

	int ntrg = 1;			// number of correlation file targets, with auto updating of reference in mind, not implemented yet
	int trg = ntrg - 1;		// trg is an index and becomes 0 for standard single corelation runs
							
	std::vector<double> blank_par_min = optimize.par_min;
	for (int i=0; i<blank_par_min.size(); i++) blank_par_min[i] = 0;

	// start run timers
	auto dts = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	auto point_time_start = std::chrono::steady_clock::now();

	// establish status file for output while running, in case of interupt
	in.echo_input(&run);
	in.append_time_date(run.sta_fname, "Run start: ", dts);
	
	unsigned int N = data.points.size();
	if ((run.num_points_to_process > 1) && (run.num_points_to_process < N)) {
		N = run.num_points_to_process;
	}

	// ***** main search loop *****

	for (unsigned int i=0; i<N; i++) {

		// this is the index of the point being processed
		int n = data.order[i];

		// initiate terminal progress output as a single over-written line
		std::cout << "\r" << i+1 << "/" << N << "  " << std::setw(4) << data.labels[n] << "  " << std::setprecision(2) << std::fixed 
			<< "x= " << data.points[n].x() << " " << "y= " << data.points[n].y() << " " << "z= " << data.points[n].z() << " " << std::flush;

		// this does the actual dvc process for the current point in the list
		try
		{
			optimize.process_point(trg, n, map_on, map_label, &data);
		}
		catch (Point_Good)
		{
			count_good += 1;

			// during-run output file update, this outputs in search order, may want to reshuffle at the end
			in.append_result(run.res_fname, data.labels[n], data.points[n], point_good, optimize.obj_min, optimize.par_min);

			// put point results into DataCloud
			data.results[trg][n].status = point_good;
			data.results[trg][n].obj_min = optimize.obj_min;
			for (int j=0; j<run.num_srch_dof; j++) {
				data.results[trg][n].par_min[j] = optimize.par_min[j];
			}
			data.results[trg][n].iter_stats.convg_status = optimize.iter_stats.convg_status;
			data.results[trg][n].iter_stats.nits = optimize.iter_stats.nits;

			// terminal status update for successful search
			terminal_status(optimize);
		}
		catch (Range_Fail)
		{
			count_range += 1;
			in.append_result(run.res_fname, data.labels[n], data.points[n], range_fail, optimize.obj_min, blank_par_min);
			std::cout << "Range_Fail";	// terminal status update
			data.results[trg][n].status = range_fail;
		}
		catch (Convg_Fail)
		{
			count_convg += 1;
			in.append_result(run.res_fname, data.labels[n], data.points[n], convg_fail, optimize.obj_min, blank_par_min);
			std::cout << "Convg_Fail";	// terminal status update
			data.results[trg][n].status = convg_fail;
		}

		count += 1;

		if (count % 100 == 0)	// update during-run status file
		{
			std::ofstream sta_file(run.sta_fname.c_str(), std::ofstream::app);
			auto point_time_status = std::chrono::steady_clock::now();
			std::chrono::duration<double, std::milli> status = point_time_status - point_time_start;
			double status_sec = status.count() / 1000.0;
			sta_file << count << " points of " << data.points.size() << " at " << count / status_sec << " pt/sec" << std::endl;
		}

	}
	std::cout << std::endl << std::endl;

	// ***** all points processed, clean-up *****

	// stop point processing timer
	auto point_time_end = std::chrono::steady_clock::now();
	std::chrono::duration<double, std::milli> elapsed_milliseconds = point_time_end - point_time_start;
	double elapsed_seconds = elapsed_milliseconds.count() / 1000.0;
	std::cout << count / elapsed_seconds << " pt/sec average" << std::endl << std::endl;
	auto dtf = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

	// re-write output file with complete results
	write_result(run.res_fname, trg, count, data);

	// re-write status file with complete results
	write_status(run.sta_fname, in, &run, count, count_good, count_range, count_convg, elapsed_seconds, dts, dtf);

	// Interpolate::print_kernel_times();
	// see Interpolate::kernels for setting up timer functions

	return 0;
}
/******************************************************************************/
void write_result(std::string fname, int trg, int count, DataCloud &data)
{
	std::ofstream res_file(fname.c_str(), std::ios_base::out);

	// write header
	res_file << "," << "x" << "," << "y" << "," << "z";
	res_file << "," << "status";
	res_file << "," << "convg";
	res_file << "," << "niter";
	res_file << "," << "objmin";
	res_file << "," << "u" << "," << "v" << "," << "w";
	res_file << std::endl;

	for (unsigned int i = 0; i<count; i++) {
		int n = data.order[i];

		res_file << std::fixed << std::setprecision(0);
		res_file << data.labels[n];

		res_file << std::fixed << std::setprecision(3);
		res_file << "," << data.points[n].x() << "," << data.points[n].y() << "," << data.points[n].z();

		res_file << std::fixed << std::setprecision(0); 
		res_file << "," << data.results[trg][n].status;

		res_file << "," << CR_str_vec[data.results[trg][n].iter_stats.convg_status];

		res_file << std::fixed << std::setprecision(0);
		res_file << "," << data.results[trg][n].iter_stats.nits;

		res_file << std::fixed << std::setprecision(6);
		res_file << "," << data.results[trg][n].obj_min;

		res_file << std::fixed << std::setprecision(6);
		for (int j=0; j<3; j++) {
			res_file << "," << data.results[trg][n].par_min[j];
		}

		res_file << std::endl;
	}

    return;
}
/******************************************************************************/
void terminal_status(Search &opt)
{
	std::string status_txt;
	if (opt.iter_stats.convg_status == CostChange) {status_txt = "cost in ";}
	if (opt.iter_stats.convg_status == ParameterChange) {status_txt = "step in ";}
	if (opt.iter_stats.convg_status == GradientNorm) {status_txt = "grad in ";}
	std::cout << "Point_Good (" << status_txt << opt.iter_stats.nits << ")";
	if (opt.iter_stats.nits > 9) {std::cout << " ";} else {std::cout << "  ";};
	std::cout << std::setprecision(6);
	std::cout << "obj= " << opt.obj_min << "  ";
	std::cout << std::fixed << std::setprecision(2);
	double u = opt.par_min[0];
	double v = opt.par_min[1];
	double w = opt.par_min[2];
	if (u<0) {std::cout << "u= " << u << " ";} else {std::cout << "u=  " << u << " ";};
	if (v<0) {std::cout << "v= " << v << " ";} else {std::cout << "v=  " << v << " ";};
	if (w<0) {std::cout << "w= " << w << " ";} else {std::cout << "w=  " << w << " ";};

	return;
}
/******************************************************************************/
void write_status(std::string fname, InputRead &in, RunControl *run, 
					int count, int count_good, int count_range, int count_convg, double elapsed_seconds,
						time_t dts, time_t dtf)
{
	in.echo_input(run);

	in.append_time_date(fname, "Run start:\t", dts);
	in.append_time_date(fname, "Run finish:\t", dtf);
	
	// overall run stats here
	std::ofstream sta_file(fname.c_str(), std::ofstream::app );

	sta_file << std::endl;
	sta_file << std::setprecision(0) << std::fixed;
	sta_file << count << " points processed in " << elapsed_seconds << " seconds" << endl;
	sta_file << std::setprecision(3) << std::fixed;
	sta_file << elapsed_seconds /count << " sec/pt" << endl;
	sta_file << count/ elapsed_seconds << " pt/sec" << endl;

	sta_file << std::endl;
	sta_file << "number successful = " << count_good << "\t(" << 100*((double)count_good/(double)count) << "%)" << endl;
	sta_file << "number range fail = " << count_range << "\t(" << 100*((double)count_range/(double)count) << "%)" << endl;
	sta_file << "number convg fail = " << count_convg << "\t(" << 100*((double)count_convg/(double)count) << "%)" << endl;
	sta_file << std::endl;

	return;
}
/******************************************************************************/