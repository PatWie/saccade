rm -rf saccade
rm saccade.tar.gz
mkdir -p saccade/libs
./collect_libs.sh static/saccade saccade/libs/
cp -r /home/patwie/Qt5.9.1/5.9.1/gcc_64/plugins/platforms/ saccade/platforms/
cp static/saccade saccade/saccade.bin
cp ./.wrapper saccade/.wrapper
cp ../README.md saccade/README.md
cp ../LICENSE saccade/LICENSE

cd saccade
ln -s .wrapper saccade
chmod +x saccade
cd ..
tar -zcvf saccade.tar.gz saccade/