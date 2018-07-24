
# OSM Data Model Tools

This repository contains software to help understand the current state of the
OSM data model with a view of supporting changes to the data model in the
future. Currently there is no decision to actually change anything in the data
model, but there are some ideas. This software can help make this decision by
creating statistics or other information that can help us evaluate what changes
might be possible and how the data would be affected.

For more background on this see https://github.com/osmlab/osm-data-model.

## Preqrequisites

You need a C++11 compliant compiler. GCC 4.8 and later as well as clang 3.6 and
later are known to work. It should also work on modern Visual Studio C++
compilers.

You also need the following libraries:

    Libosmium (>= 2.14.2)
        https://osmcode.org/libosmium
        Debian/Ubuntu: libosmium2-dev
        Fedora/CentOS: libosmium-devel

    Protozero (>= 1.6.3)
        https://github.com/mapbox/protozero
        Debian/Ubuntu: libprotozero-dev
        Fedora/CentOS: protozero-devel

    bz2lib
        http://www.bzip.org/
        Debian/Ubuntu: libbz2-dev
        Fedora/CentOS: bzip2-devel
        openSUSE: libbz2-devel

    zlib
        https://www.zlib.net/
        Debian/Ubuntu: zlib1g-dev
        Fedora/CentOS: zlib-devel
        openSUSE: zlib-devel

    Expat
        https://libexpat.github.io/
        Debian/Ubuntu: libexpat1-dev
        Fedora/CentOS: expat-devel
        openSUSE: libexpat-devel

    cmake
        https://cmake.org/
        Debian/Ubuntu: cmake
        Fedora/CentOS: cmake
        openSUSE: cmake

## Building

These programs uses CMake for their builds. On Linux and macOS you can build as
follows:

    cd osm-data-model-tools
    mkdir build
    cd build
    cmake ..
    ccmake .  ## optional: change CMake settings if needed
    make

To set the build type call cmake with `-DCMAKE_BUILD_TYPE=type`. Possible
values are empty, Debug, Release, RelWithDebInfo, MinSizeRel. The
default is RelWithDebInfo.

Please read the CMake documentation and get familiar with the `cmake` and
`ccmake` tools which have many more options.

## Documentation

See the [doc](doc/) directory for more documentation.

## Programs

### `duplicate-segments`

Create statistics on duplicated way segments.

### `line-or-polygon`

This program looks at all the ways that are in an OSM file and tries to
classify them as linear objects or area objects depending on the tags the
way has. This is surprisingly complicated and there are several corner
cases.

### `remove-tags`

Remove tags matching a list of patterns from the data.

### `tag-stats`

Create key or tag frequency statistics.

### `way-nodes`

Create statistics about nodes and their frequency as way nodes and members.


## Author

Jochen Topf (jochen@topf.org)

