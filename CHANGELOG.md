# Changelog

## 22.0.0
* Change licence from Alpache 2.0 to GPL v3.0
* Added headers to all files
* add num_points_to_process to the config file to limit the points to be processed.
* add starting_point to the config file to set the x,y,z location of the starting point for DVC analysis
* Removed Eigen library from the source code. Eigen is downloaded from https://github.com/vais-ral/Eigen3 at build time.
* Removed unused and old Python Wrapper code
* Add strain executable to install target

## v21.2.0
* added version from github repo
* added build for MacOSX

## v21.1.0
* Add strain calculation: command line executable "strain"
* Fix missing DLLs error on windows

## v21.0.0
* add OpenMP multithreading to pointcloud sorting
* saves sorted point cloud to file


