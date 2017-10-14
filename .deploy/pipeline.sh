#!/bin/sh
# run me in the topmost dir of saccade

if [ ! -f ./README.md ]; then
    echo "File not found!"
    exit 1
fi

# create .pro file for qmake
python .deploy/tools/create_pro.py > saccade.pro

CURRENT_DIR=`pwd`

# fill ing *.h.in
cmake .
# create moc files and plugins
qmake .
# build project
make


SACCADE_DIR=/tmp/saccade

mkdir ${SACCADE_DIR}
mkdir -p ${SACCADE_DIR}/libs

# collect all libs that are linked in a dynamic way
.deploy/tools/collect_libs.sh saccade ${SACCADE_DIR}/libs/

# collect all platforms
cp -r /usr/local/Qt-5.9.2/plugins/platforms/ ${SACCADE_DIR}/platforms/
# copy the app
cp saccade ${SACCADE_DIR}/saccade.bin
# copy a wrapper for loading libs and setting paths
cp .deploy/stub/.wrapper ${SACCADE_DIR}/.wrapper
# copy example for unity launcher file
cp .deploy/stub/saccade.desktop ${SACCADE_DIR}/saccade.desktop
cp README.md ${SACCADE_DIR}/README.md
cp LICENSE ${SACCADE_DIR}/LICENSE
cp Icon/256x256/saccade.png ${SACCADE_DIR}/saccade.png
cd ${SACCADE_DIR}
# point alias saccade -> wrap(saccade.bin)
ln -s .wrapper saccade
chmod +x saccade
cd ..
# create tarball for release
tar -C /tmp -zcvf saccade_release.tar.gz saccade
cd ${CURRENT_DIR}
cp /tmp/saccade_release.tar.gz .

