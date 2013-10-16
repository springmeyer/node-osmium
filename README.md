# node-osmium

Node.js bindings to [libosmium](https://github.com/osmcode/libosmium).


# Depends

 - Compiler that supports `-std=c++11` (>= clang++ 3.2 || >= g++ 4.8)
 - Node.js v0.10.x (might work with v0.8.x)
 - Boost >= 1.49 with development headers
 - OSM-Binary
 - Protocol buffers
 - zlib

Set these up on Ubuntu Precise (12.04) like:

    echo 'yes' | sudo apt-add-repository ppa:chris-lea/node.js
    echo 'yes' | sudo apt-add-repository ppa:chris-lea/protobuf
    echo 'yes' | sudo apt-add-repository ppa:mapnik/boost
    echo 'yes' | sudo apt-add-repository ppa:ubuntu-toolchain-r/test
    sudo apt-get -y update
    sudo apt-get -y install git gcc-4.7 g++-4.7 build-essential nodejs libboost-dev zlib1g-dev protobuf-compiler libprotobuf-lite7 libprotobuf-dev libexpat1-dev
    git clone https://github.com/scrosby/OSM-binary.git
    cd OSM-binary/src
    make && sudo make install

# Building

To build the bindings:

    git clone https://github.com/osmcode/libosmium.git
    cd libosmium
    git clone https://github.com/springmeyer/node-osmium.git
    cd node-osmium
    # set include/lib paths to osmium depedencies
    export CXXFLAGS="-I/opt/boost-trunk/include"
    export LDFLAGS="-L/opt/boost-trunk/lib"
    npm install

# Testing

Run the tests like:

    npm install mocha
    make test

# Troubleshooting

If you hit an error like the below it means you need a more recent compiler that implements the C++11 language standard

    cc1plus: error: unrecognized command line option ‘-std=c++11’

This error indicates you need the boost development headers installed:

    ../../include/osmium/osm/location.hpp:40:31: fatal error: boost/operators.hpp: No such file or directory

An error like this indicates that your compiler is too old and does not support all needed c++11 features

    ../../include/osmium/io/header.hpp:55:51: sorry, unimplemented: non-static data member initializers
    ../../include/osmium/io/header.hpp:55:51: error: ISO C++ forbids in-class initialization of non-const static member ‘m_has_multiple_object_versions’

And error like this indicates that you need to recompile

    /usr/bin/ld: /usr/lib/gcc/x86_64-linux-gnu/4.7/../../../../lib/libosmpbf.a(fileformat.pb.o): relocation R_X86_64_32 against `.rodata.str1.1' can not be used when making a shared object; recompile with -fPIC
