#!/bin/sh
# run me in the topmost dir of saccade

# create .pro file
python .docker/create_pro.py > saccade.pro

CURRENT_DIR=`pwd`

# build project
sudo docker run --user="$(id -u):$(id -g)" --net=none -v /tmp/saccade:/data -i -t qt59static cmake .
sudo docker run --user="$(id -u):$(id -g)" --net=none -v /tmp/saccade:/data -i -t qt59static qmake .
sudo docker run --user="$(id -u):$(id -g)" --net=none -v /tmp/saccade:/data -i -t qt59static make

SACCADE_DIR=/tmp/saccade_deploy

mkdir ${SACCADE_DIR}
rm -rf ${SACCADE_DIR}
mkdir -p ${SACCADE_DIR}/libs
/tools/collect_libs.sh saccade ${SACCADE_DIR}/libs/

cp -r /home/patwie/Qt5.9.1/5.9.1/gcc_64/plugins/platforms/ ${SACCADE_DIR}/platforms/
cp saccade ${SACCADE_DIR}/saccade.bin
cp stub/.wrapper ${SACCADE_DIR}/.wrapper
cp README.md ${SACCADE_DIR}/README.md
cp LICENSE ${SACCADE_DIR}/LICENSE
cd ${SACCADE_DIR}
ln -s .wrapper saccade
chmod +x saccade
cd ..
tar -zcvf saccade_release.tar.gz saccade/
cd ${CURRENT_DIR}
cp /tmp/saccade_release.tar.gz .

