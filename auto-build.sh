#!/bin/bash
ROOT=`pwd`

cd ./build/ 
chmod a+x make*
dos2unix *
dos2unix */*


./make-clean
#./make-debug
./make-release

if [ $? -ne 0 ]; then
   echo "build failed, exit"
   exit 1
fi
cd -

echo '============ all success! ================='
exit 0