cmake_minimum_required(VERSION 2.6)

project(Broker)

set(BROKER_VERSION 1.5.0)

# Options go here; be sure to edit config.hpp.cmake if they're needed for build
option(CUSTOMNETWORK "for network.xml support" OFF)
option(DATAGRAM "for UDP Datagram service w/o sequencing" OFF)
option(DOXYGEN "run Doxygen after project compile" ON)
option(TRACK_HANDLERS "enable Boost.Asio handler tracking" OFF)
option(WARNINGS "warnings displayed during project compile" ON)

# find the required boost libraries
find_package(Boost 1.47 REQUIRED
             COMPONENTS date_time program_options system thread
            )

# add the boost libraries to the project
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})

# set C++ compiler flags
if(WARNINGS)
    set(WARNING_FLAGS "-Wall -Wextra")
else()
    set(WARNING_FLAGS "-w")
endif()

# Use -DCMAKE_BUILD_TYPE to control optimization and debugging
set(CMAKE_CXX_FLAGS "-pedantic -std=c++98 -pthread ${WARNING_FLAGS} ${CMAKE_CXX_FLAGS}")

if(TRACK_HANDLERS)
    add_definitions(-DBOOST_ASIO_ENABLE_HANDLER_TRACKING)
endif()

# include project source directories
# probably would be better to require source files to use relative paths...
include_directories("${PROJECT_SOURCE_DIR}/src")
include_directories("${PROJECT_BINARY_DIR}/src")
include_directories("${PROJECT_SOURCE_DIR}/src/device")
include_directories("${PROJECT_BINARY_DIR}/src/device")
include_directories("${PROJECT_SOURCE_DIR}/src/device/types")
include_directories("${PROJECT_BINARY_DIR}/src/device/types")

# goto src/CMakeLists.txt
add_subdirectory(src)

add_executable(PosixBroker src/PosixMain.cpp)

# don't compile PosixBroker until codegen has run
add_dependencies(PosixBroker GeneratedPhysicalDeviceTypes)

# link the executable to its dependencies
target_link_libraries(PosixBroker
                      broker
                      device
                      ${Boost_THREAD_LIBRARY}
                      ${Boost_SYSTEM_LIBRARY}
                      ${Boost_PROGRAM_OPTIONS_LIBRARY}
                      ${Boost_DATE_TIME_LIBRARY}
                     )

if(DOXYGEN)
    find_package(Doxygen)
    if(DOXYGEN_FOUND)
        add_custom_command(TARGET PosixBroker POST_BUILD
                           COMMAND ${PROJECT_SOURCE_DIR}/builddocs.sh
                          )
    endif()
endif()
