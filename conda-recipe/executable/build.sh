set -ex
mkdir -p $SRC_DIR/ccpi
cp -rv $RECIPE_DIR/../../Wrappers/Python/ $SRC_DIR/ccpi

cp -r ${RECIPE_DIR}/../../Wrappers/Python/ccpi ${SRC_DIR}/ccpi/ccpi
# cp ${RECIPE_DIR}/../setup.py ${SRC_DIR}/ccpi
cd ${SRC_DIR}/ccpi

#:: issue cmake to create setup.py
cmake ${RECIPE_DIR}/../../  \
                        -DBUILD_PYTHON_WRAPPER=OFF\
                        -DCMAKE_BUILD_TYPE="Release"\
                        -DCMAKE_INSTALL_PREFIX=$PREFIX

cmake --build . --target install
# ${PYTHON} setup.py install
