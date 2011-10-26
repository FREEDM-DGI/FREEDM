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
    CGenericDevice.cpp
    gm/GroupManagement.cpp
    lb/CLineClient.cpp
    lb/LoadBalance.cpp
    #lb/LBPeerNode.cpp
    sc/CStateCollection.cpp
    )

# set BROKER_VERSION macro : cp version.h.cmake version.h
configure_file(version.h.cmake ${CMAKE_BINARY_DIR}/src/version.h @ONLY)

add_library(broker ${BROKER_FILES})
add_executable(PosixBroker PosixMain.cpp)

#set(Boost_THREAD_LIBRARY "/usr/include/boost_1_47_0/stage/lib/libboost_thread.so")
#set(Boost_SYSTEM_LIBRARY "/usr/include/boost_1_47_0/stage/lib/libboost_system.so")
#set(Boost_SERIALIZATION_LIBRARY "/usr/include/boost_1_47_0/stage/lib/libboost_serialization.so")
#set(Boost_PROGRAM_OPTIONS_LIBRARY "/usr/include/boost_1_47_0/stage/lib/libboost_program_options.so")

# link the executable to its dependencies
target_link_libraries(
    PosixBroker
    broker
    ${Boost_THREAD_LIBRARY}
    ${Boost_SYSTEM_LIBRARY}
    ${Boost_SERIALIZATION_LIBRARY}
    ${Boost_PROGRAM_OPTIONS_LIBRARY}
    ${Boost_DATE_TIME_LIBRARY}
)
