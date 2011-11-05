# list the source files for the project
set(
    BROKER_FILES
    CBroker.cpp
    CReliableConnection.cpp
    CListener.cpp
    CConnection.cpp
    CConnectionManager.cpp
    CDispatcher.cpp
    CMessage.cpp
    IPeerNode.cpp
    CPhysicalDeviceManager.cpp
    IDeviceBattery.cpp
    IDeviceLoad.cpp
    IDevicePV.cpp
    CDeviceGeneric.cpp
    CDevicePSCAD.cpp
    CLineClient.cpp
    CDeviceFactory.cpp
    CDeviceFactoryGeneric.cpp
    CDeviceFactoryPSCAD.cpp
    gm/GroupManagement.cpp
    lb/LoadBalance.cpp
    #lb/LBPeerNode.cpp
    sc/CStateCollection.cpp
    )

# set BROKER_VERSION macro : cp version.h.cmake version.h
configure_file(version.h.cmake ${CMAKE_BINARY_DIR}/src/version.h @ONLY)

add_library(broker ${BROKER_FILES})
add_executable(PosixBroker PosixMain.cpp)

# link the executable to its dependencies
target_link_libraries(
    PosixBroker
    broker
    ${Boost_THREAD_LIBRARY}
    ${Boost_SYSTEM_LIBRARY}
    ${Boost_SERIALIZATION_LIBRARY}
    ${Boost_PROGRAM_OPTIONS_LIBRARY}
)
