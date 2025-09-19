#   Copyright 2018 Oregon State University
#   Copyright 2018 United Kingdom Research and Innovation
#
#   This program is free software: you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation, either version 3 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program.  If not, see <https://www.gnu.org/licenses/>.
#
#   Author(s): Edoardo Pasca (UKRI-STFC)
#              Laura Murgatroyd (UKRI-STFC)

set -ex
mkdir -p $SRC_DIR/ccpi
cd ${SRC_DIR}/ccpi
if [ `python -c "from __future__ import print_function; import platform; print (platform.system())"`  == "Darwin" ] ;
then
  echo "Darwin";

  cmake ${RECIPE_DIR}/../ -DCONDA_BUILD=ON \
                        -DCMAKE_BUILD_TYPE="Release"\
                        -DLIBRARY_LIB=$CONDA_PREFIX/lib \
                        -DLIBRARY_INC=$CONDA_PREFIX \
                        -DCMAKE_INSTALL_PREFIX=$PREFIX\
                        -DOPENMP_INCLUDES=${CONDA_PREFIX}/include \
                        -DOPENMP_LIBRARIES=${CONDA_PREFIX}/lib \
                        -DBUILD_TEST=ON
else
  echo "something else";
  cmake ${RECIPE_DIR}/../  \
                        -DCMAKE_BUILD_TYPE="Release" \
                        -DCMAKE_INSTALL_PREFIX=$PREFIX \
                        -DBUILD_TEST=ON
fi

cmake --build . --target install

# ${PYTHON} setup.py install
