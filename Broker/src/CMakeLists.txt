# goto src/device/CMakeLists.txt
add_subdirectory(device)

# list the source files for the project
set(BROKER_FILES
    CBroker.cpp
    CConnection.cpp
    CConnectionManager.cpp
    CDispatcher.cpp
    CGlobalPeerList.cpp
    CListener.cpp
    CLogger.cpp
    CMessage.cpp
    CReliableConnection.cpp
    CSRConnection.cpp
    CSUConnection.cpp
    CSRSWConnection.cpp
    IPeerNode.cpp
    IProtocol.cpp
    IHandler.cpp
    CClockSynchronizer.cpp
    CTimings.cpp
    gm/GroupManagement.cpp
    lb/LoadBalance.cpp
    sc/StateCollection.cpp
   )

# generate the configuration file
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.hpp.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/config.hpp
              )

add_library(broker ${BROKER_FILES})

# don't compile broker until codegen has run
add_dependencies(broker GeneratedPhysicalDeviceTypes)
