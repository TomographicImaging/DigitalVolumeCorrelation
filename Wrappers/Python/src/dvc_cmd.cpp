#include <string>
#include "dvc.h"

int dvc_cmd(std::string filename)
{
    InputRead in;

    //Open the input filename    
	if(!in.input_file_accessible(filename)) return 0;

	// instantiate storage for run control parameters within Utility	
	RunControl run;

	// parse and check the input file

	if(!in.input_file_read(&run)) return 0;	

 	// instantiate a DataCloud
	
	DataCloud data;

 	if(!in.read_point_cloud(&run, data.points, data.labels)) return 0;

	data.organize_cloud(&run);

 	// *** begin run

	time_t start_time_date = time(NULL);
	char* dts = ctime(&start_time_date);

	in.echo_input(&run);
	in.append_time_date(run.sta_fname, "Run start: ", dts);

	// create optimizer from the input parameters

	Search optimize = Search(&run);

	// main search loop

	// establish results file for output while running, in case of interupt
	in.result_header(run.res_fname, optimize.par_min.size());

 	clock_t start_tick = clock();

	int count = 0;
	int count_good = 0;
	int count_range = 0;
	int count_convg = 0;
	int trg = 0;

	std::vector<double> blank_par_min = optimize.par_min;
	for (int i=0; i<blank_par_min.size(); i++) blank_par_min[i] = 0;

 	for (unsigned int i=0; i<data.points.size(); i++) {
		
		// this is the index of the point being processed
		int n = data.order[i];

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

		std::cout << "\n";

		if ((i>9) && (i%10 == 0)) {
			std::ofstream sta_file(run.sta_fname.c_str(), std::ofstream::app );
			clock_t run_tick = clock() - start_tick;
			double run_time = ((double)run_tick)/CLOCKS_PER_SEC;
			sta_file << i  << " points of " << data.points.size() << " at " << run_time/i << " sec/pt\n";
		}

		count += 1;
	}

	// *** all points processed, clean-up
	
	// re-write output in point cloud order, from stored results
	in.result_header(run.res_fname, optimize.par_min.size());
	for (unsigned int i=0; i<data.points.size(); i++)
		in.append_result(run.res_fname, data.labels[i], data.points[i], data.results[trg][i].status, data.results[trg][i].obj_min, data.results[trg][i].par_min);

	in.echo_input(&run);
	in.append_time_date(run.sta_fname, "Run start: ", dts);
	time_t finish_time_date = time(NULL);
	char* dtf = ctime(&finish_time_date);
	in.append_time_date(run.sta_fname, "Run finish: ", dtf);

	// add overall run stats here

	std::ofstream sta_file(run.sta_fname.c_str(), std::ofstream::app );
	clock_t run_tick = clock() - start_tick;
	double run_time = ((double)run_tick)/CLOCKS_PER_SEC;
	sta_file << std::setprecision(3) << std::fixed;
	sta_file << count << " points processed in " << run_time << " seconds\n";
	sta_file << run_time/count << " sec/pt average\n";
	sta_file << "\n";
	sta_file << "number successful = " << count_good << "\t(" << 100*((double)count_good/(double)count) << "%)\n";
	sta_file << "number range fail = " << count_range << "\t(" << 100*((double)count_range/(double)count) << "%)\n";
	sta_file << "number convg fail = " << count_convg << "\t(" << 100*((double)count_convg/(double)count) << "%)\n";                   
    
    return 0;

}