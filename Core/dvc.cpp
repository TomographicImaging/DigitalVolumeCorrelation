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
			cout << "Running with threads = " << omp_get_num_threads() << endl;
		}
	}

	InputRead in;

	std::string help("help");
	std::string example("example");
	std::string manual("manual");

	bool first_point = true;

	// command entered with no arguments
	if (argc == 1)
	{
		std::cout << endl;
		std::cout << "Options:" << endl;
		std::cout << "dvc dvc_in\t\t// execute dvc code with dvc_in controlling the run" << endl;
		std::cout << "dvc help\t\t// provide additional detail about running the dvc code" << endl;
		std::cout << "dvc example\t\t// print dvc_in_example with brief keyword descriptions" << endl;
		std::cout << "dvc manual\t\t// print dvc_manual with more detailed information" << endl;
		std::cout << endl;
		return 0;
	}

	// trap help on the command line
	if (argv[1] == help)
	{
		std::cout << endl;
		std::cout << "Program execution is controlled by a key_word based input file." << endl;
		std::cout << "Please refer to an example input file for content and format." << endl;
		std::cout << "To run the code type dvc followed by the name of the input file." << endl;
		std::cout << "The input file is evaluated and, if all is good, the run starts." << endl;
		std::cout << "Common problems are incorrect file paths, missing keywords, and invalid parameters." << endl;
		std::cout << "A message is sent to the console window if an input file problem is encountered." << endl;
		std::cout << "Progrm execution can be lengthy, on the order of hours for large point clouds." << endl;
		std::cout << "Results are written to output_filename.disp during execution." << endl;
		std::cout << "Information about the run is written to output_filename.stat during execution." << endl;
		std::cout << "Both files are simple text, and can be opened and viewed at any time." << endl;
		std::cout << endl;
		return 0;
	}

	// trap example on the command line
	if (argv[1] == example)
	{
		std::cout << "\ndvc_in_example printed in the current working directory" << endl << endl;
		std::ofstream dvc_inp("dvc_in_example");
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
	if (argv[1] == manual)
	{
		std::cout << "\ndvc_manual printed in the current working directory" << endl << endl;
		std::ofstream dvc_man("dvc_manual");
		in.print_manual_intro(dvc_man);
		in.print_manual_section(dvc_man, "fio_name");
		in.print_manual_section(dvc_man, "vox_data");
		in.print_manual_section(dvc_man, "sub_vols");
		in.print_manual_section(dvc_man, "opt_mthd");
		in.print_manual_section(dvc_man, "opt_tune");
		in.print_manual_output(dvc_man);
		dvc_man.seekp(0, dvc_man.beg);
		dvc_man.close();
		return 0;
	}

	// check to see if the command line argument is an accessible file

	std::string fname(argv[1]);
	if(!in.input_file_accessible(fname)) return 0;

	// instantiate storage for run control parameters within Utility

	RunControl run;

	// parse and check the input file

	if(!in.input_file_read(&run)) return 0;

	// instantiate a DataCloud
	// organize_cloud can be lengthy, does (# points)*(# points) sorting

	DataCloud data;

	if(!in.read_point_cloud(&run, data.points, data.labels)) return 0;

	data.organize_cloud(&run);


	// *** begin run

	auto start_time_test = std::chrono::system_clock::now();

	auto dts = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	in.echo_input(&run);
	in.append_time_date(run.sta_fname, "Run start: ", dts);

	// create optimizer from the input parameters

	Search optimize = Search(&run);

	// *** print configuration to screen
	//std::cout << optimize << std::endl;
	std::string objfun;
	if (run.obj_fcn == SAD) {
		//obj_fcn = &obj_SAD;
		objfun = std::string("objective function SAD");
	}
	if (run.obj_fcn == SSD) {
		//obj_fcn = &obj_SSD;
		objfun = std::string("objective function SSD");
	}
	if (run.obj_fcn == ZSSD) {
		//obj_fcn = &obj_ZSSD;
		objfun = std::string("objective function ZSSD");
	}
	if (run.obj_fcn == NSSD) {
		//obj_fcn = &obj_NSSD;
		objfun = std::string("objective function NSSD");
	}
	if (run.obj_fcn == ZNSSD) {
		//obj_fcn = &obj_ZNSSD;
		objfun = std::string("objective function ZNSSD");
	}
	/*
	std::cout << "Search(" << std::endl <<
		"bytes_per " << optimize.bytes_per << std::endl <<
		"subv_rad " << optimize.subv_rad << std::endl <<
		"subv_num " << optimize.subv_num << std::endl <<
		"obj_fun " << objfun << std::endl <<
		"input shape " << run.vol_wide << " " <<
		run.vol_high << " " <<
		run.vol_tall << std::endl <<
		"subvol aspect " << run.subvol_aspect[0] << " " <<
		run.subvol_aspect[1] << " " <<
		run.subvol_aspect[2] << std::endl <<
		"numr_search_dof " << run.num_srch_dof << std::endl <<
		"disp max " << run.disp_max << std::endl <<
		")";
		*/
#if defined(_WIN32) || defined(__WIN32__)
		//std::cout << std::endl << std::endl << "*************************" << std::endl;
		//std::cout << "Search: ( " << std::endl << optimize <<  std::endl << ") " << std::endl <<std::endl;
#else
	std::cout << std::endl << std::endl << "*************************" << std::endl;
	std::cout << "Search: ( " << std::endl << optimize << std::endl << ") " << std::endl << std::endl;
#endif

	// main search loop

	// establish results file for output while running, in case of interupt
	in.result_header(run.res_fname, optimize.par_min.size());


	int count = 0;
	int count_good = 0;
	int count_range = 0;
	int count_convg = 0;
	int trg = 0;

	std::vector<double> blank_par_min = optimize.par_min;
	for (int i=0; i<blank_par_min.size(); i++) blank_par_min[i] = 0;

	auto point_time_start = std::chrono::steady_clock::now();

	for (unsigned int i=0; i<data.points.size(); i++) {

		// this is the index of the point being processed
		int n = data.order[i];

		std::cout << i << "/" << data.points.size() << " " ;
		std::cout << data.labels[n] << "\t";
		std::cout << std::setprecision(3) << std::fixed;
		std::cout << data.points[n].x() << " ";
		std::cout << data.points[n].y() << " ";
		std::cout << data.points[n].z() << " ";
		std::cout << "\t";

		try
		{
			optimize.process_point(trg, n, &data);
		}
		catch (Point_Good)
		{
			count_good += 1;
			// this outputs in search order, may want to reshuffle at the end
			in.append_result(run.res_fname, data.labels[n], data.points[n], point_good, optimize.obj_min, optimize.par_min);
			std::cout << "Point_Good" << "\t";
			std::cout << std::setprecision(6);
			std::cout << "obj= " << optimize.obj_min;
			std::cout << std::setw(12) << "dx= " << optimize.par_min[0];
			std::cout << std::setw(12) << "dy= " << optimize.par_min[1];
			std::cout << std::setw(12) << "dz= " << optimize.par_min[2];

			// put results into record for this point
			data.results[trg][n].status = point_good;
			data.results[trg][n].obj_min = optimize.obj_min;
			for (int j=0; j<run.num_srch_dof; j++)
				data.results[trg][n].par_min[j] = optimize.par_min[j];
		}
		catch (Range_Fail)
		{
			count_range += 1;
			in.append_result(run.res_fname, data.labels[n], data.points[n], range_fail, optimize.obj_min, blank_par_min);
			std::cout << "Range_Fail";

			data.results[trg][n].status = range_fail;
		}
		catch (Convg_Fail)
		{
			count_convg += 1;
			in.append_result(run.res_fname, data.labels[n], data.points[n], convg_fail, optimize.obj_min, blank_par_min);
			std::cout << "Convg_Fail";

			data.results[trg][n].status = convg_fail;
		}

		std::cout << endl;

		count += 1;
		first_point = false;

		if (count % 10 == 0)
		{
			std::ofstream sta_file(run.sta_fname.c_str(), std::ofstream::app);

			auto point_time_status = std::chrono::steady_clock::now();
			std::chrono::duration<double, std::milli> status = point_time_status - point_time_start;
			double status_sec = status.count() / 1000.0;

			sta_file << count << " points of " << data.points.size() << " at " << count / status_sec << " pt/sec" << std::endl;
		}



	}

	// *** all points processed, clean-up
		// stop point processing timer
	auto point_time_end = std::chrono::steady_clock::now();
	std::chrono::duration<double, std::milli> elapsed_milliseconds = point_time_end - point_time_start;
	double elapsed_seconds = elapsed_milliseconds.count() / 1000.0;
	std::cout << count / elapsed_seconds << " pt/sec average" << std::endl;

	auto dtf = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());


	// re-write output in point cloud order, from stored results
	in.result_header(run.res_fname, optimize.par_min.size());
	for (unsigned int i=0; i<data.points.size(); i++)
		in.append_result(run.res_fname, data.labels[i], data.points[i], data.results[trg][i].status, data.results[trg][i].obj_min, data.results[trg][i].par_min);

	in.echo_input(&run);
	in.append_time_date(run.sta_fname, "Run start:\t", dts);
	in.append_time_date(run.sta_fname, "Run finish:\t", dtf);

	// overall run stats here
	std::ofstream sta_file(run.sta_fname.c_str(), std::ofstream::app );
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

	return 0;
}
/******************************************************************************/
