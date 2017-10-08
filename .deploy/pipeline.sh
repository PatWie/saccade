#!/bin/sh
# run me in the topmost dir of saccade

# create .pro file
python .deploy/tools/create_pro.py > saccade.pro

CURRENT_DIR=`pwd`

# build project
cmake .
qmake .
make

SACCADE_DIR=/tmp/saccade_deploy

mkdir ${SACCADE_DIR}
mkdir -p ${SACCADE_DIR}/libs
.deploy/tools/collect_libs.sh saccade ${SACCADE_DIR}/libs/

#cp -r /home/patwie/Qt5.9.1/5.9.1/gcc_64/plugins/platforms/ ${SACCADE_DIR}/platforms/
cp -r /usr/local/Qt-5.9.2/plugins/platforms/ ${SACCADE_DIR}/platforms/
cp saccade ${SACCADE_DIR}/saccade.bin
cp .deploy/stub/.wrapper ${SACCADE_DIR}/.wrapper
cp README.md ${SACCADE_DIR}/README.md
cp LICENSE ${SACCADE_DIR}/LICENSE
cd ${SACCADE_DIR}
ln -s .wrapper saccade
chmod +x saccade
cd ..
dir
tar -zcvf saccade_release.tar.gz ${SACCADE_DIR}
cd ${CURRENT_DIR}
cp /tmp/saccade_release.tar.gz .

