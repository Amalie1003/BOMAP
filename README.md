# README
## Introduction
This is the implementation of BOMAP, an oblivious map with constant roundtrips.
## Environments and Requirements
Operating System: Ubuntu 20.04
- g++ 9.4.0
- cmake 3.25.2
- openssl 1.1.1
- boost 1.71.0
- grpc 1.57.0
- mongoDB 7.0.9
- mongo C++ driver 3.10.0

## Before Compile
Start mongoDB service following https://www.mongodb.com/docs/manual/tutorial/install-mongodb-on-ubuntu-tarball/.
```
mongod --dbpath /var/lib/mongo --logpath /var/log/mongodb/mongod.log --fork
```
## Compile
```
mkdir build
cd build
cmake ..
make
```
Or you can check `test.sh`.
