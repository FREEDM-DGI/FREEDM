# cmake specification for FREEDM Broker
cmake_minimum_required(VERSION 2.6)
project(Broker)

set(ALT_BUILD_VERSION "Unversioned")

# configurable project options
# TUTORIAL on how to add a new option:
#   add the option below with a help message defaulted to OFF
#   add the option to Broker/include/cmake.hpp.cmake as #cmakedefine OPTION
#   activate / deactivate the option as described in the message below
option(SHOW_WARNINGS "warnings displayed during project compile" OFF)
option(CUSTOMNETWORK "for network.xml support" OFF)
option(DATAGRAM "for UDP Datagram service w/o sequencing" OFF)
option(USE_DEVICE_PSCAD "Enable the PSCAD simulation interface" ON)

# find the perl executable
message(STATUS "Checking for perl installation")
find_program(PERL_EXEC perl)

if(PERL_EXEC)
    set(SCRIPT "${CMAKE_BINARY_DIR}/configure.pl")
    set(INFILE "${CMAKE_BINARY_DIR}/hostname.cfg")
    
    # check for required input file
    if(NOT EXISTS ${INFILE})
        message(FATAL_ERROR "Missing required file ${INFILE}")
    endif()
    
    # execute the perl configure script
    message(STATUS "Running configure.pl to generate src/freedm.cfg")
    execute_process(
        COMMAND ${PERL_EXEC} ${SCRIPT} ${INFILE}
        RESULT_VARIABLE CONFIGURE_RESULT
    )
    
    # check the script result
    if(${CONFIGURE_RESULT} MATCHES "1")
        message(FATAL_ERROR "Failed to generate freedm.cfg")
    endif()
else(PERL_EXEC)
    message(FATAL_ERROR "Failed to find perl installation")
endif(PERL_EXEC)

# find the required boost libraries
message(STATUS "Checking for boost installation")
find_package(Boost 1.44.0 COMPONENTS REQUIRED system thread program_options serialization)

if(Boost_FOUND)
    # add the boost libraries to the project
    include_directories(${Boost_INCLUDE_DIRS})
    link_directories(${Boost_LIBRARY_DIRS})

    # set C++ compiler flags
    set(COMMON_FLAGS "")

    set(WARNING_FLAGS "")
    if(SHOW_WARNINGS)
        set(WARNING_FLAGS "-W -Wall -pedantic")
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
endif(Boost_FOUND)
