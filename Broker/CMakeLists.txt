# cmake specification for FREEDM Broker
cmake_minimum_required(VERSION 2.6)
project(Broker)

set(ALT_BUILD_VERSION "Unversioned")

# configurable project options
# TUTORIAL on how to add a new option:
#   add the option below with a help message defaulted to OFF
#   add the option to Broker/include/config.hpp.cmake as #cmakedefine OPTION
#   activate / deactivate the option as described in the message below
option(BUILD_TESTS "Enable the building and running of unit tests." OFF)
option(SHOW_WARNINGS "warnings displayed during project compile" ON)
option(CUSTOMNETWORK "for network.xml support" OFF)
option(DATAGRAM "for UDP Datagram service w/o sequencing" OFF)
option(USE_DEVICE_PSCAD "Enable the PSCAD simulation interface" OFF)
option(USE_DEVICE_RTDS "Enable the RTDS simulation interface" OFF)
option(SHOW_MESSAGES "Enable help messages during cmake execution" ON)

if(SHOW_MESSAGES)
    message("This project uses custom CMake settings:"
        "\n\tView the available settings with cmake -LH"
        "\n\tChange a setting with -DSETTING=ON/OFF")
endif(SHOW_MESSAGES)

# find the required boost libraries
find_package(Boost REQUIRED COMPONENTS system thread program_options serialization date_time)

if(Boost_FOUND)
    # add the boost libraries to the project
    include_directories(SYSTEM ${Boost_INCLUDE_DIRS})
    link_directories(${Boost_LIBRARY_DIRS})

    # set C++ compiler flags
    set(COMMON_FLAGS "-g -lpthread")
    set(WARNING_FLAGS "")
    if(SHOW_WARNINGS)
        set(WARNING_FLAGS "-Wall -Wextra -pedantic")
    endif(SHOW_WARNINGS)

    set(CMAKE_CXX_FLAGS "-std=c++98 ${COMMON_FLAGS} ${WARNING_FLAGS}")

    # generate the configuration file
    CONFIGURE_FILE(
        ${CMAKE_BINARY_DIR}/include/config.hpp.cmake
        ${CMAKE_BINARY_DIR}/include/config.hpp )
    
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
