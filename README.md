docker-static-bin
=================
Very small C sources implementing a few commands need in a `FROM scratch`

## Dependencies

  - C compiler
  - CMake
  - conan

## Developer workflow

    mkdir cmake-build-debug && cd $_
    conan install ..
    cmake -DCMAKE_BUILD_TYPE=Debug ..
    make

## Run tests

Starting from directory created above, after `make` step, run:

    ./bin/test_*
