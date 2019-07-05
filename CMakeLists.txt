cmake_minimum_required(VERSION 3.2)
project(ModbusServer)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

aux_source_directory(3rdparty/libmodbus DIRSRCS)
include_directories(3rdparty/libmodbus include)

set(SOURCE_FILES src/main.cpp src/modbusadapter.cpp)
add_executable(main ${SOURCE_FILES} ${DIRSRCS})
