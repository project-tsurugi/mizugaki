# Mizugaki - A SQL compiler front end

## Requirements

* CMake `>= 3.16`
* C++ Compiler `>= C++17`
* and see *Dockerfile* section

```sh
# retrieve third party modules
git submodule update --init --recursive
```

### Dockerfile

```dockerfile
FROM ubuntu:22.04

RUN apt update -y && apt install -y git build-essential cmake flex libfl-dev libgflags-dev ninja-build
```

optional packages:

* `bison` (only `>= 3.6`)
* `doxygen`
* `graphviz`
* `clang-tidy-14`

### Install modules

#### tsurugidb modules

This requires below [tsurugidb](https://github.com/project-tsurugi/tsurugidb) modules to be installed.

* [yugawara](https://github.com/project-tsurugi/yugawara)

#### `third_party/hopscotch-map`

NOTE: hopscotch-map will be already installed because [yugawara](https://github.com/project-tsurugi/yugawara) also requires it.

```sh
mkdir -p build-third_party/hopscotch-map
cd build-third_party/hopscotch-map
cmake -G Ninja -DCMAKE_INSTALL_PREFIX=[/path/to/install-prefix] ../../third_party/hopscotch-map
cmake --build . --target install
```

see https://github.com/Tessil/hopscotch-map

#### GNU Bison `>= 3.6`

This project requires GNU Bison `>= 3.6`.
Please run `bison --version` and check the printed version.

```sh
# install packages to build bison
sudo apt update -y
sudo apt install -y curl m4

curl http://ftp.jaist.ac.jp/pub/GNU/bison/bison-3.6.4.tar.gz | tar zxv
cd bison-3.6.4
./configure --prefix=/path/to/install
make -j4
make install # or sudo make install
```

If you install the above to a non-standard path, please specify `-DCMAKE_PREFIX_PATH=</path/to/install>` to cmake.

## How to build

```sh
mkdir -p build
cd build
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
```

available options:

* `-DCMAKE_INSTALL_PREFIX=/path/to/install-root` - change install location
* `-DCMAKE_PREFIX_PATH=/path/to/install-root` - add installation search path (including `bison` command)
* `-DBUILD_SHARED_LIBS=OFF` - create static libraries instead of shared libraries
* `-DBUILD_TESTS=OFF` - don't build test programs
* `-DBUILD_DOCUMENTS=OFF` - don't build documents by doxygen
* `-DBUILD_EXAMPLES=OFF` - don't build example applications
* `-DBUILD_STRICT=OFF` - don't treat compile warnings as build errors
* `-DINSTALL_EXAMPLES=ON` - also install example applications

### install

```sh
cmake --build . --target install
```

### run tests

```sh
ctest
```

### generate documents

```sh
cmake --build . --target doxygen
```

## License

[Apache License, Version 2.0](http://www.apache.org/licenses/LICENSE-2.0)
