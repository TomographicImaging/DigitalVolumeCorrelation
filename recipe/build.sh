set -ex
mkdir -p $SRC_DIR/ccpi
cd ${SRC_DIR}/ccpi
if [ `python -c "from __future__ import print_function; import platform; print (platform.system())"`  == "Darwin" ] ;
then 
  echo "Darwin"; 
  
  # cmake ${RECIPE_DIR}/../  \
  #                       -DBUILD_PYTHON_WRAPPER=OFF\
  #                       -DCMAKE_BUILD_TYPE="Release"\
  #                       -DCMAKE_INSTALL_PREFIX=$PREFIX
  #                       -DOPENMP_CXX_INCLUDE_DIR=${CONDA_PREFIX}/include \
  #                       -DOPENMP_LIBRARIES=${CONDA_PREFIX}/lib
  cmake ${RECIPE_DIR}/../ -DCONDA_BUILD=ON \
                        -DBUILD_PYTHON_WRAPPER=OFF \
                        -DCMAKE_BUILD_TYPE="Release"\
                        -DLIBRARY_LIB=$CONDA_PREFIX/lib \
                        -DLIBRARY_INC=$CONDA_PREFIX \
                        -DCMAKE_INSTALL_PREFIX=$PREFIX\
                        -DOPENMP_INCLUDES=${CONDA_PREFIX}/include \
                        -DOPENMP_LIBRARIES=${CONDA_PREFIX}/lib
else 
  echo "something else"; 
  cmake ${RECIPE_DIR}/../  \
                        -DBUILD_PYTHON_WRAPPER=OFF\
                        -DCMAKE_BUILD_TYPE="Release"\
                        -DCMAKE_INSTALL_PREFIX=$PREFIX
fi

cmake --build . --target install
# ${PYTHON} setup.py install
