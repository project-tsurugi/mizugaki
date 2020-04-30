# Mizugaki - A SQL compiler front end

## Requirements

* CMake `>= 3.10`
* C++ Compiler `>= C++17`
* and see *Dockerfile* section

```sh
# retrieve third party modules
git submodule update --init --recursive
```

### Dockerfile

```dockerfile
FROM ubuntu:18.04

RUN apt update -y && apt install -y git build-essential cmake ninja-build
```

optional packages:

* `doxygen`
* `graphviz`
* `clang-tidy-9`

### Install sub-modules

#### `third_party/takatori`

see [README](https://github.com/project-tsurugi/takatori/blob/master/README.md).

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

## How to build

```sh
mkdir -p build
cd build
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
```

available options:

* `-DCMAKE_INSTALL_PREFIX=/path/to/install-root` - change install location
* `-DBUILD_SHARED_LIBS=OFF` - create static libraries instead of shared libraries
* `-DBUILD_TESTS=OFF` - don't build test programs
* `-DBUILD_DOCUMENTS=OFF` - don't build documents by doxygen
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
