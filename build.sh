#!/bin/bash

BUILD_CLEAN=false

while getopts "D" arg
do
    case $arg in
        D)
            echo "will build with clean"
            BUILD_CLEAN=true
            ;;
		?)
            ;;
    esac
done


mkdir -p output
cd output

mkdir -p build
cd build

if [ "$BUILD_CLEAN" = true ] ; then
	echo "start clean"
	rm -rf *
fi

cmake ../..

make -j8

cd ..

mkdir -p bin
cd bin

cp ../build/repo_mirror_push .
cp ../../depend/git_pull.sh .
cp ../../depend/git_push.sh .
cp ../../depend/mirror_pull.sh .
cp ../../depend/mirror_push.sh .
FILE="setting.ini"  
if [ ! -f "$FILE" ]; then  
    cp ../../depend/$FILE .
fi

chmod +x repo_mirror_push
chmod +x git_pull.sh .
chmod +x git_push.sh .
chmod +x mirror_pull.sh .
chmod +x mirror_push.sh .
