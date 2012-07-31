# list the source files for the project
set(
    BROKER_FILES
    CBroker.cpp
    CConnection.cpp
    CConnectionManager.cpp
    CDispatcher.cpp
    CListener.cpp
    CLogger.cpp
    CMessage.cpp
    CReliableConnection.cpp
    CSRConnection.cpp
    CSUConnection.cpp
    CGlobalPeerList.cpp
    IPeerNode.cpp
    IProtocol.cpp
    IHandler.cpp
    gm/GroupManagement.cpp
    lb/LoadBalance.cpp
    sc/StateCollection.cpp
   )

# set BROKER_VERSION macro : cp version.h.cmake version.h
configure_file(version.h.cmake ${CMAKE_BINARY_DIR}/src/version.h @ONLY)

add_library(broker ${BROKER_FILES})

