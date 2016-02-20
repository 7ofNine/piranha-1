#!/usr/bin/env bash

# Exit on error
set -e
# Echo each command
set -x

if [[ "${BUILD_TYPE}" == "Debug" ]]; then
    if [[ "${PIRANHA_COMPILER}" == "gcc" ]]; then
        cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS="-fsanitize=address -Os" -DCMAKE_CXX_FLAGS_DEBUG=-g0 -DBUILD_TUTORIAL=yes -DPIRANHA_TEST_SPLIT=yes -DPIRANHA_TEST_SPLIT_NUM=${SPLIT_TEST_NUM} ../;
        make;
        ctest -E "thread|memory" -V;
    elif [[ "${PIRANHA_COMPILER}" == "clang" ]]; then
        cmake -DCMAKE_BUILD_TYPE=Debug -DBUILD_TUTORIAL=yes -DPIRANHA_TEST_SPLIT=yes -DPIRANHA_TEST_SPLIT_NUM=${SPLIT_TEST_NUM} ../;
        make;
        ctest -E "thread" -V;
    fi
elif [[ "${BUILD_TYPE}" == "Release" ]]; then
    cmake -DCMAKE_BUILD_TYPE=Release ../;
    make;
    ctest -E "gastineau|perminov" -V;
elif [[ "${BUILD_TYPE}" == "Python2" ]]; then
    cmake -DCMAKE_BUILD_TYPE=Debug -DBUILD_PYRANHA=yes -DBUILD_TESTS=no -DCMAKE_CXX_FLAGS_DEBUG=-g0 -DCMAKE_CXX_FLAGS=-Os -DCMAKE_INSTALL_PREFIX=/home/travis/.local ../;
    make install;
    pip install --user mpmath;
    python -c "import pyranha.test; pyranha.test.run_test_suite()";
elif [[ "${BUILD_TYPE}" == "Python3" ]]; then
    cmake -DCMAKE_BUILD_TYPE=Debug -DBUILD_PYRANHA=yes -DBUILD_TESTS=no -DCMAKE_CXX_FLAGS_DEBUG=-g0 -DCMAKE_CXX_FLAGS=-Os -DCMAKE_INSTALL_PREFIX=/home/travis/.local -DBoost_PYTHON_LIBRARY_RELEASE=/usr/lib/x86_64-linux-gnu/libboost_python-py32.so -DBoost_PYTHON_LIBRARY_DEBUG=/usr/lib/x86_64-linux-gnu/libboost_python-py32.so -DPYTHON_EXECUTABLE=/usr/bin/python3 -DPYTHON_INCLUDE_DIR=/usr/include/python3.2 -DPYTHON_LIBRARY=/usr/lib/libpython3.2mu.so ../;
    make install;
    wget "http://mpmath.org/files/mpmath-0.19.tar.gz";
    tar xzf mpmath-0.19.tar.gz;
    cd mpmath-0.19;
    python3 setup.py install --user;
    cd ..;
    python3 -c "import pyranha.test; pyranha.test.run_test_suite()";
elif [[ "${BUILD_TYPE}" == "Tutorial" ]]; then
    cmake -DCMAKE_BUILD_TYPE=Debug -DBUILD_TUTORIAL=yes -DBUILD_TESTS=no ../;
    make;
    ctest -V;
elif [[ "${BUILD_TYPE}" == "Doxygen" ]]; then
    cmake ../;
    wget "http://ftp.stack.nl/pub/users/dimitri/doxygen-1.8.11.src.tar.gz";
    tar xzf doxygen-1.8.11.src.tar.gz;
    cd doxygen-1.8.11;
    mkdir build;
    cd build;
    cmake -DCMAKE_INSTALL_PREFIX=/home/travis/.local ../;
    make;
    make install;
    cd ../../doc/doxygen;
    if [[ $(/home/travis/.local/bin/doxygen 2>&1 |cat -| > /dev/null) ]]; then
        echo "Doxygen produced some warnings/errors";
        return 1;
    fi
    echo "Doxygen ran successfully";
fi
