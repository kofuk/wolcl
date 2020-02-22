cmake_minimum_required(VERSION 3.0)
project(wolcl)

find_package(PkgConfig REQUIRED)
pkg_check_modules(JSONCPP jsoncpp)
pkg_check_modules(READLINE readline)

add_executable(wolcl main.cc utils.cc Selector.cc WOLHandler.cc)
target_include_directories(wolcl PRIVATE ${JSONCPP_INCLUDE_DIRS})
target_include_directories(wolcl PRIVATE ${READLINE_INCLUDE_DIRS})
target_link_libraries(wolcl ${JSONCPP_LIBRARIES})
target_link_libraries(wolcl ${READLINE_LIBRARIES})

install(TARGETS wolcl DESTINATION bin)
