# Changelog
## v26.0.0

- NEW: Faster KD Tree point cloud sorting
- NEW: tri bspline interpolation for subvolume
- Backward incompatible changes:
  - Parameter renaming:
    - disp_max, renamed to step_max. It used to be max displacement for the sample, but is now max change in the parameter vector during point processing. It is a critical parameter, set as small as possible for efficiency and optimization control. The term “step” is being used for optimization tolerancing (see below) and is descriptive in this context.
    - rigid_trans, renamed to start_estimate, more descriptive of how the parameter is used
    - starting_point, renamed to start_position
    - basin_radius, legacy parameter from before point cloud sorting in wavefronts. Now not accessible anymore to users
  - Removed parameters: 
    - subvol_thresh
    - gray_thresh_min
    - gray_thresh_max
    - min_vol_fract
- New parameters:
  - cost_tol, step_tol, max_iter, represent convergence settings
- Objective functions:
  - sad, removed
- Interpolation type:
  - Added tri_bspline interpolation methods, tri_bspline_3, tri_bspline_5, tri_bspline_7
  - nearest, removed




## v25.0.0
* First public release
* Added headers to all files
* Added unit test framework with conda and GitHub Actions on Linux, Windows and Mac arm64

## v22.0.0
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


