cmake_minimum_required(VERSION 3.22.1 FATAL_ERROR)
project(zio VERSION 1.0)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wextra")
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "14.1.1")
        message(FATAL_ERROR "GCC version must be at least 14.1.1!")
    endif()
endif()



set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -pipe -fomit-frame-pointer -foptimize-sibling-calls -foptimize-strlen -frerun-cse-after-loop -fstrict-aliasing -fweb -ftree-vrp -ftree-pta -ftree-loop-vectorize -fvect-cost-model -finline-functions -funswitch-loops -flto=auto")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DNDEBUG")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -flto=auto")
set(ENV{PKG_CONFIG_PATH} "/usr/local/lib/pkgconfig/")
find_package (glog 0.7.0 REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(unwind REQUIRED IMPORTED_TARGET "libunwind>=1.8.1")
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

add_executable(zio ${SOURCES})
target_link_libraries(zio pthread PkgConfig::zmq glog::glog -lunwind -lunwind-x86_64)