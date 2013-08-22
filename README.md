# node-osmium

Node.js bindings to [libosmium](https://github.com/osmcode/libosmium).


# Depends

 - Compiler that supports `-std=c++11` (>= clang++ 3.2 || >= g++ 4.8)
 - `libosmpbf`: https://github.com/scrosby/OSM-binary/archive/v1.3.0.tar.gz
 - `libprotobuf-lite` compiled with -std=c++11 (and on OS X `-stdlib=libc++`)


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
