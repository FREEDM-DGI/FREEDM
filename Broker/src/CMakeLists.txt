# list the source files for the project
set(
    BROKER_FILES
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
    dnp3/DNP3Slave.cpp
    dnp3/SlaveDemo.cpp
   )

# set BROKER_VERSION macro : cp version.h.cmake version.h
configure_file(version.h.cmake ${CMAKE_BINARY_DIR}/src/version.h @ONLY)

add_library(broker ${BROKER_FILES})

# don't compile broker until codegen has run
add_dependencies(broker GeneratedPhysicalDeviceTypes)
