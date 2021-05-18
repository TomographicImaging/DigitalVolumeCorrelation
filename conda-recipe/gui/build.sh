set -ex
mkdir -p $SRC_DIR
cp -rv $RECIPE_DIR/../../Apps/DVC_configurator/ $SRC_DIR
cd ${SRC_DIR}

${PYTHON} setup.py install
