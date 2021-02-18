cmake_minimum_required(VERSION 2.8)
project(webserver)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
include (./cmake/utils.cmake)

set(CMAKE_VERBOSE_MAKEFILE ON)
#set(CMAKE_CXX_FLAGS "SENV{CXXFLAGS} -rdynamic -O3 -g -std=c++11 -Wall -Wno-deprecated -Werror -Wno-unused-function")
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -O3 -fPIC -ggdb -std=c++11 -Wall -Wno-deprecated -Werror -Wno-unused-function -Wno-builtin-macro-redefined -Wno-deprecated-declarations")

include_directories(.)
include_directories(../yaml-cpp/include)
link_directories(/home/marulong/webserver/lib)

set(LIB_SRC
    webserver/address.cc
    webserver/log.cc
    webserver/util.cc
    webserver/config.cc
    webserver/thread.cc
    webserver/fiber.cc
    webserver/scheduler.cc
    webserver/iomanager.cc
    webserver/timer.cc
    webserver/hook.cc
    webserver/fd_manager.cc
    webserver/socket.cc
    webserver/bytearray.cc
    webserver/http/http.cc
    webserver/http/http11_parser.cc
    webserver/http/httpclient_parser.cc
    webserver/http/http_parser.cc
    )

add_library(webserver SHARED ${LIB_SRC})
force_redefine_file_macro_for_sources(webserver)  #__FILE__
#add_library(webserver_static STATIC ${LIB_SRC})
#SET_TARGET(webserver_static PROPERTIES OUTPUT_NAME "webserver")

set(LIB_LIB
    webserver
    dl
    pthread
    yaml-cpp
    )

add_executable(test_config tests/test_config.cc)
add_dependencies(test_config webserver)
force_redefine_file_macro_for_sources(test_config)  #__FILE__
target_link_libraries(test_config webserver ${LIB_LIB})


add_executable(test_thread tests/test_thread.cc)
add_dependencies(test_thread webserver)
force_redefine_file_macro_for_sources(test_thread)  #__FILE__
target_link_libraries(test_thread webserver ${LIB_LIB})

add_executable(test_util tests/test_util.cc)
add_dependencies(test_util webserver)
force_redefine_file_macro_for_sources(test_util)  #__FILE__
target_link_libraries(test_util webserver ${LIB_LIB})

add_executable(test_fiber tests/test_fiber.cc)
add_dependencies(test_fiber webserver)
force_redefine_file_macro_for_sources(test_fiber)  #__FILE__
target_link_libraries(test_fiber webserver ${LIB_LIB})

add_executable(test_scheduler tests/test_scheduler.cc)
add_dependencies(test_scheduler webserver)
force_redefine_file_macro_for_sources(test_scheduler)  #__FILE__
target_link_libraries(test_scheduler webserver ${LIB_LIB})

add_executable(test_iomanager tests/test_iomanager.cc)
add_dependencies(test_iomanager webserver)
force_redefine_file_macro_for_sources(test_iomanager)  #__FILE__
target_link_libraries(test_iomanager webserver ${LIB_LIB})

add_executable(test_hook tests/test_hook.cc)
add_dependencies(test_hook webserver)
force_redefine_file_macro_for_sources(test_hook)  #__FILE__
target_link_libraries(test_hook webserver ${LIB_LIB})

add_executable(test_address tests/test_address.cc)
add_dependencies(test_address webserver)
force_redefine_file_macro_for_sources(test_address)  #__FILE__
target_link_libraries(test_address webserver ${LIB_LIB})

add_executable(test_socket tests/test_socket.cc)
add_dependencies(test_socket webserver)
force_redefine_file_macro_for_sources(test_socket)  #__FILE__
target_link_libraries(test_socket webserver ${LIB_LIB})

add_executable(test_bytearray tests/test_bytearray.cc)
add_dependencies(test_bytearray webserver)
force_redefine_file_macro_for_sources(test_bytearray)  #__FILE__
target_link_libraries(test_bytearray webserver ${LIB_LIB})

add_executable(test_http tests/test_http.cc)
add_dependencies(test_http webserver)
force_redefine_file_macro_for_sources(test_http)  #__FILE__
target_link_libraries(test_http webserver ${LIB_LIB})

add_executable(test_http_parser tests/test_http_parser.cc)
add_dependencies(test_http_parser webserver)
force_redefine_file_macro_for_sources(test_http_parser)  #__FILE__
target_link_libraries(test_http_parser webserver ${LIB_LIB})

add_executable(test_tcp_server tests/test_tcp_server.cc)
add_dependencies(test_tcp_server webserver)
force_redefine_file_macro_for_sources(test_tcp_server)  #__FILE__
target_link_libraries(test_tcp_server webserver ${LIB_LIB})

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

