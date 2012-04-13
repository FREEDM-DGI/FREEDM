# CMake specification for PSCAD-Interface
cmake_minimum_required (VERSION 2.6)
project (PSCAD_INTERFACE)

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
    message(STATUS "Running configure.pl to generate src/simserv.xml")
    execute_process(
        COMMAND ${PERL_EXEC} ${SCRIPT} ${INFILE}
        RESULT_VARIABLE CONFIGURE_RESULT
    )
    
    # check the script result
    if(${CONFIGURE_RESULT} MATCHES "1")
        message(FATAL_ERROR "Failed to generate simserv.xml")
    endif()
else(PERL_EXEC)
    message(FATAL_ERROR "Failed to find perl installation")
endif(PERL_EXEC)

# find the boost libraries required for this project
find_package (Boost REQUIRED COMPONENTS system thread program_options)

if (Boost_FOUND)
    # add the found libraries to the project
    include_directories(${Boost_INCLUDE_DIRS})
    link_directories(${Boost_LIBRARY_DIRS})

    # goto src/CMakeLists.txt
    add_subdirectory (src)
endif (Boost_FOUND)
