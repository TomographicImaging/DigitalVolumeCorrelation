# CCPi-DVC
Digital Volume Correlation

| Master | Development | Anaconda binaries |
|--------|-------------|-------------------|
| [![Build Status](https://github.com/TomographicImaging/DigitalVolumeCorrelation/actions/workflows/conda_build_and_publish.yml/badge.svg?branch=master)](https://github.com/TomographicImaging/DigitalVolumeCorrelation/actions/workflows/conda_build_and_publish.yml) | [![Build Status](https://github.com/TomographicImaging/DigitalVolumeCorrelation/actions/workflows/conda_build_and_publish.yml/badge.svg)](https://github.com/TomographicImaging/DigitalVolumeCorrelation/actions/workflows/conda_build_and_publish.yml) | ![conda version](https://anaconda.org/ccpi/ccpi-dvc/badges/version.svg) ![conda last release](https://anaconda.org/ccpi/ccpi-dvc/badges/latest_release_date.svg) [![conda platforms](https://anaconda.org/ccpi/ccpi-dvc/badges/platforms.svg) ![conda dowloads](https://anaconda.org/ccpi/ccpi-dvc/badges/downloads.svg)](https://anaconda.org/ccpi/ccpi-dvc) |

## Build

The package comes as a [CMake](https://cmake.org) project so you will need CMake (v.>=3) to configure it. Additionally you will need a C compiler, `make` (on linux). The toolkit comes with an executable `dvc` which may be used directly. We provide wrappers for Python.

1. Clone this repository to a directory, i.e. `CCPi-DVC`,
2. create a build directory.
3. Issue `cmake` to configure (or `cmake-gui`, or `ccmake`, or `cmake3`). Use additional flags to fine tune the configuration.

### CMake flags
Flags used during configuration

| CMake flag | type | meaning |
|:---|:----|:----|
| `CMAKE_CXX_STANDARD` | string | Defaults to `C++11`.
| `CMAKE_INSTALL_PREFIX` | path | your favourite install directory |
| `CONDA_BUILD`| bool | `ON\|OFF` whether it is installed with `setup.py install`|
|`BUILD_TEST` | bool | `ON\|OFF` whether to build the test mode|

Here an example of build on Linux:

```bash
git clone https://github.com/TomographicImaging/DigitalVolumeCorrelation
mkdir build
cd build
cmake ../CCPi-DVC -DCONDA_BUILD=OFF -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=<your favourite install directory>
make install
```

## Open MP

### Setting the Number of Threads Using an OpenMP Environment Variable
You can set the number of threads using the environment variable `OMP_NUM_THREADS`. If this is higher than the number of processors it will instead use the number of processors available.
