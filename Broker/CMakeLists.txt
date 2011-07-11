# cmake specification for FREEDM Broker
cmake_minimum_required(VERSION 2.6)
project(Broker)

set(ALT_BUILD_VERSION "Unversioned")

# configurable project options
option(BUILD_TESTS "Enable the building and running of unit tests." OFF)

# find the required boost libraries
find_package(Boost REQUIRED COMPONENTS system thread program_options serialization)

if(Boost_FOUND)
    # add the boost libraries to the project
    include_directories(${Boost_INCLUDE_DIRS})
    link_directories(${Boost_LIBRARY_DIRS})

    # set C++ compiler flags
    set(COMMON_FLAGS "")
    set(CMAKE_CXX_FLAGS "-std=c++98 ${COMMON_FLAGS}")

    # include project source directories
    include_directories("${CMAKE_BINARY_DIR}/include")
    include_directories("${CMAKE_BINARY_DIR}/src")

    # goto src/CMakeLists.txt
    add_subdirectory( src )

    if(BUILD_TESTS)
        enable_testing()

        # goto test/CMakeLists.txt
        add_subdirectory(test)
    endif(BUILD_TESTS)
endif(Boost_FOUND)
