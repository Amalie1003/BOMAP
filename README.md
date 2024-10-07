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
## 预先准备
下载安装编译相关库：
```
$ [sudo] apt-get install build-essential
```
从[官网](https://cmake.org/download/)下载源码，安装高版本cmake。
```
$ [sudo] apt-get install libssl-dev
$ [sudo] apt-get install boost
```
下载[grpc源码](https://github.com/grpc/grpc)并按照[文档](https://github.com/grpc/grpc/blob/master/BUILDING.md)安装。

准备第三方数据库，以mongoDB键值数据库为例（可以通过Connector.cpp修改API切换为其他数据库）。在[官网](https://www.mongodb.com/docs/manual/tutorial/install-mongodb-on-ubuntu-tarball)下载tgz包，[安装](https://www.mongodb.com/docs/manual/tutorial/install-mongodb-on-ubuntu-tarball/)并开启数据库服务。
```
mongod --dbpath /var/lib/mongo --logpath /var/log/mongodb/mongod.log --fork
```
## 编译
```
mkdir build
cd build
cmake ..
make
./test_server
./test_bomap
```
Or you can check `test.sh`.
