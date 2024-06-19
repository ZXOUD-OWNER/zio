cmake_minimum_required(VERSION 3.22.1 FATAL_ERROR)
project(zmqLoadBalancing VERSION 1.0)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20")
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS_RELEASE "-O3")
set(CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG")
set(CMAKE_CXX_FLAGS "-flto=auto")
set(CMAKE_CXX_FLAGS_RELEASE "-march=native -pipe -fomit-frame-pointer -foptimize-sibling-calls -foptimize-strlen -frerun-cse-after-loop -fstrict-aliasing -fweb -ftree-vrp -ftree-pta -ftree-loop-vectorize -fvect-cost-model -finline-functions -funswitch-loops")
set(ENV{PKG_CONFIG_PATH} "/usr/local/lib/pkgconfig/")
find_package(czmq 4.2.1 REQUIRED)
find_package (glog 0.7.0 REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(zmq REQUIRED IMPORTED_TARGET "libzmq>=4.3.5" "libczmq>=4.2.1")


# INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/3rdPart/libyuv/include)
# #INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/3rdPart/nvencSDK/include)
# INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/3rdPart/decklinkSDK/include)
# INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/3rdPart/ffmpeg/include)
# INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/3rdPart/MagewellCaptureLib/include)


LINK_DIRECTORIES("/usr/lib")
INCLUDE_DIRECTORIES("/usr/include")
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/head)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src SOURCES)

add_executable(zmqLoadBalancing ${SOURCES})
target_link_libraries(zmqLoadBalancing pthread PkgConfig::zmq glog::glog -lunwind -lunwind-x86_64)