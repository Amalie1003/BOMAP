# README
## Introduction
This is the implementation of BOMAP, an oblivious map with constant roundtrips.
## Environments and Requirements
Operating System: Ubuntu 20.04
- g++ 9.4.0
- cmake 3.25.3
- openssl 1.1.1
- boost 1.71.0
- grpc 1.57.0
- mongo C++ driver 3.8.0
## Preparation
Download, install, and compile the required libraries.
```
$ [sudo] apt-get install build-essential
```
Download the source code from the [official website](https://cmake.org/download/) and install a newer version of CMake.
```
$ [sudo] apt-get install libssl-dev
$ [sudo] apt-get install boost
```
Download the [gRPC source code](https://github.com/grpc/grpc) and follow the [official instructions](https://github.com/grpc/grpc/blob/master/BUILDING.md) to install it.

Prepare the third-party database. For example, use MongoDB as a key-value store. Download the .tgz package from the [official MongoDB website](https://www.mongodb.com/docs/manual/tutorial/install-mongodb-on-ubuntu-tarball), [install it](https://www.mongodb.com/docs/manual/tutorial/install-mongodb-on-ubuntu-tarball/), and start the database service.
## Compilation
```
mkdir build
cd build
cmake ..
make
./test_server
./test_bomap
```
Or you can check `test.sh`.
