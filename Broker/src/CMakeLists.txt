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
    
    
    vvc/VoltVarCtrl.cpp
    # for VVC 
	vvc/form_Ftheta.cpp
	vvc/form_Fv.cpp
	vvc/form_J.cpp
	vvc/form_Yabc.cpp
	vvc/load_system_data.cpp
	vvc/rename_brn.cpp 
	vvc/V_abc_list.cpp
	vvc/DPF_return7.cpp
    #
   )

# generate the configuration file
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.hpp.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/config.hpp
              )

add_library(broker ${BROKER_FILES})

target_link_libraries(broker messages)
