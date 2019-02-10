rm build/RTPlayStep
mkdir build
cd build
cmake -DDEFINE_RELEASE=ON ..
make -j 4
echo Copying binaries to ../../bin directory, run from there!
rm ../../bin/RTPlayStep
cp RTPlayStep ../../bin
cd ..

