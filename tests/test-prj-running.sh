mkdir build-ut
cd build-ut
cmake -DCMAKE_BUILD_TYPE=Debug ..
make test -j8
