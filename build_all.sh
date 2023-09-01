#!/usr/bin/env bash

set -ex

NUM_JOBS=""
ROOTDIR=${ROOTDIR:-`pwd`}

EXTERNAL_DIR="${ROOTDIR}/external"

ICEORYX_DIR="${ROOTDIR}/external/iceoryx"
ICEORYX_DEPENDENCY_DIR="${ROOTDIR}/external/install"

FASTDDS_DIR="${ROOTDIR}/external/fastdds"
FASTDDS_DEPENDENCY_DIR="${ROOTDIR}/external/install"

VSOMEIP_DIR="${ROOTDIR}/external/vsomeip"
VSOMEIP_DEPENDENCY_DIR="${ROOTDIR}/external/install"

BUILD_MODE="new"
BUILD_OBJECT="all"

SELFNAME=$0

function usage(){
    cat << EOF
Usage: ${SELFNAME} -m MODE -o OBJECT

OBJECT: all deps modsar
EOF
exit 1
}


function build_modsar(){
    pushd ${ROOTDIR}/buildCentral

    case $BUILD_MODE in
        new)
            ./mk -cb
            rm -rf output/*
        ;;
        clean)
            ./mk -cb
        ;;
        remake)
            echo REMAKE
        ;;
    esac
    
    popd
}

function build_vsomeip(){
    echo "Start build vsomeip"

    pushd ${EXTERNAL_DIR}

    if [ ! -d ${VSOMEIP_DIR} ];then
        git clone https://github.com/COVESA/vsomeip.git
    fi

    pushd ${VSOMEIP_DIR}

    if [[ ${BUILD_MODE} == "new" ]];then
        echo "Clean vsomeip build directory"
        rm -rf ./build
    fi

    mkdir -p build
    cd build

    echo ">>>>>> Start building vsomeip package <<<<<<"
    cmake .. -DCMAKE_INSTALL_PREFIX=${VSOMEIP_DEPENDENCY_DIR}
    cmake --build . --target install -- -j$NUM_JOBS

    popd
    popd
}

function build_fastdds(){
    echo "Start build Fast DDS"

    pushd ${EXTERNAL_DIR}

    if [ ! -d ${FASTDDS_DIR} ];then
        mkdir -p ${FASTDDS_DIR}
    fi
    pushd {FASTDDS_DIR}

    if [ ! -d ${FASTDDS_DIR}/foonathan_memory_vendor ];then
        git clone https://github.com/eProsima/foonathan_memory_vendor.git
    fi
    if [[ ${BUILD_MODE} == "new" ]];then
        rm -rf foonathan_memory_vendor/build
    fi
    mkdir foonathan_memory_vendor/build
    pushd foonathan_memory_vendor/build
    cmake .. -DCMAKE_INSTALL_PREFIX=${FASTDDS_DEPENDENCY_DIR} -DBUILD_SHARED_LIBS=ON
    cmake --build . --target install
    popd

    if [ ! -d ${FASTDDS_DIR}/Fast-CDR ];then
        git clone https://github.com/eProsima/Fast-CDR.git
    fi
    if [[ ${BUILD_MODE} == "new" ]];then
        rm -rf Fast-CDR/build
    fi
    mkdir Fast-CDR/build
    pushd Fast-CDR/build
    cmake ..  -DCMAKE_INSTALL_PREFIX=${FASTDDS_DEPENDENCY_DIR}
    cmake --build . --target install
    popd

    if [ ! -d ${FASTDDS_DIR}/Fast-DDS ];then
        git clone https://github.com/eProsima/Fast-DDS.git
    fi
    if [[ ${BUILD_MODE} == "new" ]];then
        rm -rf Fast-DDS/build
    fi
    mkdir Fast-DDS/build
    pushd Fast-DDS/build
    cmake .. -DCMAKE_INSTALL_PREFIX=${FASTDDS_DEPENDENCY_DIR} -DBUILD_SHARED_LIBS=ON
    cmake --build . --target install -j${NUM_JOBS}
    popd

    popd

    popd
}

function build_iceoryx(){
    echo "Start building iceoryx"

    pushd ${EXTERNAL_DIR}

    if [ ! -d ${ICEORYX_DIR} ];then
        git clone https://github.com/eclipse-iceoryx/iceoryx.git ${ICEORYX_DIR}
    fi

    pushd ${ICEORYX_DIR}

    if [[ ${BUILD_MODE} == "new" ]];then
        echo "Clean iceoryx build directory"
        rm -rf ./build
    fi

    mkdir -p build
    cd build
    echo " [i] Current working directory: $(pwd)"


    echo ">>>>>> Start building iceoryx package <<<<<<"
    echo ${ICEORYX_DEPENDENCY_DIR}
    cmake -DBUILD_SHARED_LIBS=ON -DCMAKE_INSTALL_PREFIX=${ICEORYX_DEPENDENCY_DIR} ../iceoryx_meta
    cmake --build . --target install -- -j$NUM_JOBS

    popd
    popd
}

function build_main(){
    echo "Starting building ..."

    # pushd ${ROOTDIR}/buildCentral

    case $BUILD_OBJECT in
        deps)
            echo "Building dependencies..."
            build_iceoryx
            build_fastdds
            build_vsomeip
        ;;
        modsar)
            echo "Building ModSar module ..."
            build_modsar
        ;;
        all)
            echo "Building dependencies and ModSar ..."
        ;;
        iceoryx)
            echo "Building iceoryx ..."
            build_iceoryx
        ;;
        fastdds)
            echo "Building fastdds ..."
            build_fastdds
        ;;
        vsomeip)
            echo "Building vsomeip ..."
            build_vsomeip
        ;;
        *) echo default
        ;;
    esac

    # popd

    echo "End of building ..."
}

function parse_args()
{
    POSITIONAL=()
    if (( $# == 0 )); then
        usage
    fi

    while (( $# > 0 )); do
        case "${1}" in
            -h|--help)
            usage
            ;;
            -m|--mode)
            shift # shift once since flags have no values
            BUILD_MODE=${1}
            shift
            ;;
            -o|--object)
            shift # shift once since flags have no values
            BUILD_OBJECT=${1}
            shift
            ;;
            *) # unknown flag/switch
            POSITIONAL+=("${1}")
            shift
            ;;
        esac
    done
    
    set -- "${POSITIONAL[@]}" # restore positional params
    
}


function main () {
    parse_args $@
    build_main
}


NUM_JOBS=$(nproc)

main $@
