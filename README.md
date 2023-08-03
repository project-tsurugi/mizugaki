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

* `bison` (only `>= 3.5`)
* `doxygen`
* `graphviz`
* `clang-tidy-14`

### Install modules

#### `third_party/yugawara`

see [README](https://github.com/project-tsurugi/yugawara/blob/master/README.md).

#### `third_party/shakujo` (transitive)

see [README](https://github.com/project-tsurugi/shakujo/blob/master/README.md).

#### `third_party/hopscotch-map`

NOTE: `third_party/yugawara` also requires to install this module.

```sh
mkdir -p build-third_party/hopscotch-map
cd build-third_party/hopscotch-map
cmake -G Ninja -DCMAKE_INSTALL_PREFIX=[/path/to/install-prefix] ../../third_party/hopscotch-map
cmake --build . --target install
```

see https://github.com/Tessil/hopscotch-map

#### GNU Bison `>= 3.5`

This project requires GNU Bison `>= 3.5`.
Please run `bison --version` and check the printed version.

```sh
# install packages to build bison
sudo apt update -y
sudo apt install -y curl m4

curl http://ftp.jaist.ac.jp/pub/GNU/bison/bison-3.5.1.tar.gz | tar zxv
cd bison-3.5.1
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
* `-DINSTALL_EXAMPLES=ON` - also install example applications
* `-DFORCE_INSTALL_RPATH=ON` - automatically configure `INSTALL_RPATH` for non-default library paths

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
