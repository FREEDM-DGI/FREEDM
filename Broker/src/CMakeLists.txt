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
    CProtocolSR.cpp
    CProtocolSU.cpp
    CProtocolSRSW.cpp
    CPeerNode.cpp
    MPeerSets.cpp
    IProtocol.cpp
    IHandler.cpp
    CClockSynchronizer.cpp
    CTimings.cpp
    CPhysicalTopology.cpp
    gm/GroupManagement.cpp
    lb/LoadBalance.cpp
    sc/StateCollection.cpp
   )

# generate the configuration file
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.hpp.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/config.hpp
              )

add_library(broker ${BROKER_FILES})
