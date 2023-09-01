## How to use buildCentral

#### 0x01 Introduction

buildCentral is an open source tool for software package management.

https://github.com/jeremyczhen/buildCentral

buildCentral can manage dependency relation in project side, can perform cross-compile for project by config different toolchain.

Currently, buildCentral is used in CT IDC project, compile QNX side packages.

#### 0x02 Create project architecture

In textbox below, you can find the directory structure for the basic example.

```bash
$ ls -l
total 1860
drwxrwxr-x  6 hua hua    4096 8月  24 09:04 build
-rwxrwxr-x  1 hua hua    5058 8月  21 11:26 build_all.sh
drwxrwxr-x  7 hua hua    4096 8月  21 11:16 buildCentral
drwxrwxr-x  6 hua hua    4096 8月  11 10:10 buildsystem
-rw-rw-r--  1 hua hua     684 8月  15 17:18 CMakeLists.txt
drwxrwxr-x  2 hua hua    4096 8月  24 10:46 docs
drwxrwxr-x  3 hua hua    4096 8月  15 17:18 examples
drwxrwxr-x  7 hua hua    4096 8月  23 13:50 external
drwxrwxr-x  4 hua hua    4096 8月  11 10:10 fast-dds-iceoryx-allin
-rw-rw-r--  1 hua hua  302042 8月  11 10:10 ModSarComInterfaces.png
-rw-rw-r--  1 hua hua 1531904 8月  11 10:10 ModSar.qea
drwxrwxr-x  2 hua hua    4096 8月  11 10:10 non-stardard-com
-rw-rw-r--  1 hua hua     499 8月  21 13:44 README.md
drwxrwxr-x  8 hua hua    4096 8月  11 10:10 rpc_method
-rw-rw-r--  1 hua hua      44 8月  23 13:30 test_git.md
drwxrwxr-x  4 hua hua    4096 8月  11 10:10 vrte
drwxrwxr-x  4 hua hua    4096 8月  11 10:10 vsomeip-com-qnx
drwxrwxr-x 10 hua hua    4096 8月  11 10:10 vsomeip-qnx-allin

$ tree examples/helloworld
examples/helloworld
├── CMakeLists.txt
├── inc
│   └── sum.h
├── src
│   ├── main.cpp
│   └── sum.cpp
└── tests
    └── test_sum.cpp
```

You can put project source code in different path, and it can be added into buildCentral by adding items in build_central.cfg with several configurations, like directory, target, dependency and etc.

The above tree view, is an example project, including CMakeLists.txt, include, source, and tests file. It can be used as a template for new project.

#### 0x03 Using CMake in project

CMakeLists.txt for project can be divided into 3 parts.

First part, project configurations:

```cmake
# set cmake version
cmake_minimum_required(VERSION 3.1.3)

# set project name
project(${PACKAGE_NAME} C CXX)

# set compiler standard version, for google test, 14 is needed
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# include buildCentral cmake file
if (DEFINED RULE_DIR)
    include(${RULE_DIR}/rule_base.cmake)
else()
    set(RULE_DIR ${CMAKE_SOURCE_DIR})
    include(rule_base.cmake)
endif()

set(PACKAGE_SOURCE_ROOT ${CMAKE_SOURCE_DIR})


print_variable(PROJECT_ROOT)
print_variable(EXTERNEL_INSTALL)
print_variable(PACKAGE_SOURCE_ROOT)
print_variable(BUILD_OUT)
```

Second part, project executable or library configurations:

```cmake
# set include and link directories
# EXTERNEL_INSTALL, ModSar external libraries include and libraries directory
# PACKAGE_SOURCE_ROOT, current project root dir
# ${PACKAGE_SOURCE_ROOT}/inc, current project inc dir
include_directories(
    ${EXTERNEL_INSTALL}/include
    ${PACKAGE_SOURCE_ROOT}
    ${PACKAGE_SOURCE_ROOT}/inc
    )
link_directories(${EXTERNEL_INSTALL}/lib)

# using glob to get all source file in "${PACKAGE_SOURCE_ROOT}/src/*.cpp"
file(GLOB HELLOWORLD_SRC "${PACKAGE_SOURCE_ROOT}/src/*.cpp")

# add compile executable
add_executable(helloworld ${HELLOWORLD_SRC})

# install target, install helloworld to build/bin
install(TARGETS helloworld DESTINATION ${BUILD_OUT})
```

#### 0x04 Config build_central.cfg

BuildCentral use build_central.cfg file as configration, including package information, compile information, package dependency, and etc.

The build_central.cfg is located in `buildCentral/build_central.cfg` in json format.

The below is the example project _helloworld_ configuration in build_central.cfg.

```json
        "helloworld" : {
            "Path" : "../examples/helloworld",
            "Dependency" : [],
            "Tools" : [],
            "MAKE_VAR" : {"BUILD_OUT":"$PROOT/../build/bin"},
            "MACRO_DEF" : {},
            "ENV_SOURCE_CMD" : "",
            "Label" : "helloworld example",
            "MakeTarget" : "install"
        }
```

The above is _helloworld_ package config in `PACKAGE` string field.
* `Path`: is path to _helloworld_ root directory
* `Dependency`: is project dep package's list, in json list format.
* `Tools`: used to build support lib, not used in ModSar
* `MAKE_VAR`: variable defined here, will be transfered to cmake cmd
* `MACRO_DEF`: variable defined here, will be transfered to cmake cmd
* `ENV_SOURCE_CMD`: it seems that this string not used. 
* `Label`: display name for the project, `mk --list` will display this string.
* `MakeTarget`: execute `mk` compile project, and last make target for project.

```json
        "common-host" : {
            "PACKAGES" : [
                "helloworld"
            ]
        }
```

The above is config for buildCentral build target group. `common-host` means helloworld is compiled in host side.

BuildCentral supports different build target, like host (win or linux), or cross-compile arm-qnx.

#### 0x05 compile project (module) by buildCentral

`mk` command is used to do buildCentral build, in `buildCentral/mk`. It should be executed like below:

```bash
$ cd buildCentral

# build all packages configed in build_central.cfg, clean and build
$ ./mk -cb

# build helloworld package, clean and build
# If dependencies should be build firstly, add -a option
$ ./mk -cb helloworld
```

#### 0x06 compile all projects by build_all.sh

`build_all.sh` is used to build deps libraries, build all packages.

```bash
./build_all.sh -m (new|remake) -o modsar
```

