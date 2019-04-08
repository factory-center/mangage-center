cmake_minimum_required (VERSION 3.3)
project (CarveService)
SET(CMAKE_C_COMPILER "/usr/local/bin/gcc")          
SET(CMAKE_CXX_COMPILER "/usr/local/bin/g++")           
include(CTest)
set (CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "-std=c++0x ${CMAKE_CXX_FLAGS} -g -ftest-coverage -fprofile-arcs -fpermissive")
set(SELF_PATH "../lib32/baoyuan")
# The version number.
set (Tutorial_VERSION_MAJOR 1)
set (Tutorial_VERSION_MINOR 0)
 

add_definitions(-D__GCC)

# add the binary tree to the search path for include files
# so that we will find TutorialConfig.h
include_directories ("${PROJECT_BINARY_DIR}")
#include_directories ("${Boost_INCLUDE_DIRS}")
include_directories ("../include/")


link_directories("../lib32/json")
link_directories("../lib32/boost")
link_directories("../lib32/baoyuan") #添加动态连接库的路径

# add the executable
add_executable (CarveService Carve_Control_ServiceDlg.cpp carve_common_lib.cpp carve_manager.cpp CBaoyuan_Lib_Tool.cpp CCarve.cpp CDevice.cpp log_base.cpp http_server/connection.cpp http_server/deal_http_msg.cpp http_server/http_parser.c http_server/http_settings.cpp http_server/mime_types.cpp http_server/reply.cpp http_server/request_handler.cpp http_server/request_parser.cpp http_server/server.cpp http_server/singleton_server.cpp)
target_link_libraries (CarveService LINK_PUBLIC -lSCIF2 -lboost_system -lboost_thread -lpthread -lboost_filesystem -lboost_chrono -lboost_date_time -lboost_regex -lrt jsoncpp )