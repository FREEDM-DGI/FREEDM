# list the directories that contain the source files
include_directories ("${PSCAD_INTERFACE_SOURCE_DIR}/include")
include_directories ("${PSCAD_INTERFACE_SOURCE_DIR}/src")
link_directories ("${PSCAD_INTERFACE_BINARY_DIR}/src")

# list the source files for the project
set (MYFILES CSimulationServer.cpp CEventTable.cpp TCPSocket.cpp)

# specify the C++ compiler flags
set (CMAKE_CXX_FLAGS "-std=c++98 -W -Wall -pedantic")

add_library (MYLIB ${MYFILES})
add_executable (driver driver.cpp)

# link the executable to its dependencies
target_link_libraries (driver ${Boost_THREAD_LIBRARY})
target_link_libraries (driver ${Boost_SYSTEM_LIBRARY})
target_link_libraries (driver MYLIB)
