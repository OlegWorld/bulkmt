cmake_minimum_required(VERSION 3.9)
project(bulkmt)

find_package(GTest REQUIRED)
find_package (Threads REQUIRED)
find_library(TBB tbb)

add_executable(bulkmt main.cpp command_handler.cpp command_handler.h command_reader.h command_reader.cpp command_processors.h command_processors.cpp abstract_observer.h command_bulk.cpp command_bulk.h thread_utility.h)
add_executable(bulkmt_test test.cpp command_handler.cpp command_handler.h command_reader.h command_reader.cpp command_processors.h command_processors.cpp command_bulk.cpp command_bulk.h)

target_include_directories(bulkmt_test PRIVATE ${GTEST_INCLUDE_DIRS})

set_target_properties(bulkmt PROPERTIES   CXX_STANDARD 17
                                          CXX_STANDARD_REQUIRED ON
                                          COMPILE_OPTIONS "-Wpedantic;-Wall;-Wextra;-Werror")

set_target_properties(bulkmt_test PROPERTIES  CXX_STANDARD 17
                                              CXX_STANDARD_REQUIRED ON)

target_link_libraries(bulkmt ${CMAKE_THREAD_LIBS_INIT} ${TBB} stdc++fs)

target_link_libraries(bulkmt_test ${GTEST_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} ${TBB} stdc++fs)

add_test(test bulkmt_test)

enable_testing()

install(TARGETS bulkmt RUNTIME DESTINATION bin)

set(CPACK_GENERATOR DEB)

set(CPACK_PACKAGE_VERSION_MAJOR "0")
set(CPACK_PACKAGE_VERSION_MINOR "1")
set(CPACK_PACKAGE_VERSION_PATCH "$ENV{TRAVIS_BUILD_NUMBER}")

set(CPACK_PACKAGE_CONTACT osmironov@gmail.com)

include(CPack)
