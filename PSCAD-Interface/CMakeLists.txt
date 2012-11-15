# cmake specification for PSCAD-Interface
cmake_minimum_required(VERSION 2.6)
project(PSCAD_INTERFACE)

# find the boost libraries required for this project
find_package(Boost REQUIRED COMPONENTS system thread program_options)

# check for possible error conditions
if(!LINUX)
    message(FATAL_ERROR "This project must be compiled on GNU/Linux.")
endif(!LINUX)

if(!Boost_FOUND)
    message(FATAL_ERROR "The required boost C++ libraries were not found.")
endif(!Boost_FOUND)

# add the boost libraries to the project
include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})

# include project source file directories
include_directories("${CMAKE_SOURCE_DIR}/include")
include_directories("${CMAKE_SOURCE_DIR}/src")

# specify the C++ compiler flags
set(CMAKE_CXX_FLAGS "-std=c++98 -Wextra -Wall -pedantic -pthread")

# list the source files for the project
set(
    PROJECT_FILES
    src/CLogger.cpp
    src/CDeviceSignal.cpp
    src/CTableWriteLock.cpp
    src/CTableReadLock.cpp
    src/CTableManager.cpp
    src/IServer.cpp
    src/CAdapter.cpp
    src/CAdapterSimulation.cpp
    src/CAdapterPscad.cpp
    src/CAdapterRtds.cpp
)
add_library(simserv ${PROJECT_FILES})

# create the project executable
add_executable(driver src/PosixMain.cpp)

# link the executable to its dependencies
target_link_libraries(
    driver
    simserv
    ${Boost_THREAD_LIBRARY}
    ${Boost_SYSTEM_LIBRARY}
    ${Boost_PROGRAM_OPTIONS_LIBRARY}
)
