#include "dvc.h"

/******************************************************************************/
void echo_vect_upto(std::vector<double> vect, unsigned int n)
{
	std::cout << std::setprecision(6) << std::fixed;

	for (unsigned int i = 0; i<vect.size(); i++)
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
		return 1;
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
	if (!in.input_file_accessible(fname)) return -1;

	// instantiate storage for run control parameters within Utility

	RunControl run;

	// parse and check the input file

	if (!in.input_file_read(&run)) return -2;

	// instantiate a DataCloud
	// organize_cloud can be lengthy, does (# points)*(# points) sorting

	DataCloud data;

	if (!in.read_point_cloud(&run, data.points, data.labels)) return -3;

	data.organize_cloud(&run);

	std::cout << "Checking results " << std::endl;

	std::vector<std::vector<int>> solution = {};
	std::vector<int> row = {
		0, 1, 3, 4, 2, 7, 5, 6, 8, 9,
		1, 0, 2, 4, 3, 5, 8, 9, 7, 6,
		2, 1, 5, 4, 0, 9, 3, 8, 7, 6,
		3, 4, 0, 7, 1, 8, 6, 5, 9, 2,
		4, 5, 3, 8, 1, 9, 2, 0, 7, 6,
		5, 4, 2, 9, 8, 1, 3, 0, 7, 6,
		6, 7, 3, 8, 4, 0, 1, 9, 5, 2,
		7, 6, 8, 3, 4, 9, 0, 5, 1, 2,
		8, 9, 7, 4, 5, 3, 6, 1, 2, 0,
		9, 8, 5, 4, 7, 2, 3, 1, 0, 6 };
	for (int i = 0; i < 10; i++) {
		std::vector<int> drow = {};
		for (int j = 0; j < 10; j++) {
			drow.push_back(row[i * 10 + j]);
		}
	}

	std::ifstream infile("grid_input.roi.sorted");
	int a, b, c, d, e, f, g, h, l, m;
	int result = 0, j = 0;
	while (infile >> a >> b >> c >> d >> e >> f >> g >> h >> l >> m)
	{
		std::vector<int> riga = { a,b,c,d,e,f,g,h,l,m};
		for (int i = 0; i < 10; i++) {
			//std::cout << "Checking results " << row[i + j* 10] << " " << riga[i] << std::endl;
			if (row[i + j * 10] != riga[i]) {
				result++;
			}
				
		}
		j++;
	}

	std::cout << "Final result is " << result << std::endl;

	return result;
}
/******************************************************************************/
