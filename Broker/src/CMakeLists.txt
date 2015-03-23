add_subdirectory(device)
add_subdirectory(messages)

# list the source files for the project
set(BROKER_FILES
    CBroker.cpp
    CClockSynchronizer.cpp
    CConnection.cpp
    CConnectionManager.cpp
    CDispatcher.cpp
    CGlobalPeerList.cpp
    CListener.cpp
    CLogger.cpp
    CProtocolSR.cpp
    CPeerNode.cpp
    PeerSets.cpp
    CTimings.cpp
    IProtocol.cpp
    IDGIModule.cpp
    Messages.cpp
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

target_link_libraries(broker messages)
