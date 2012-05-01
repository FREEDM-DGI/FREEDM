# list the source files for the project
set(
    BROKER_FILES
    CBroker.cpp
    CClientRTDS.cpp
    CConnection.cpp
    CConnectionManager.cpp
    CDispatcher.cpp
    CLineClient.cpp
    CListener.cpp
    CLogger.cpp
    CMessage.cpp
    CReliableConnection.cpp
    CSRConnection.cpp
    CSUConnection.cpp
    CTableRTDS.cpp
    CTableStructure.cpp
    device/CDeviceFactory.cpp
    device/CDeviceKeyCoupled.cpp
    device/CDeviceStructureGeneric.cpp
    device/CDeviceStructurePSCAD.cpp
    device/CDeviceStructureRTDS.cpp
    device/CPhysicalDeviceManager.cpp
    device/PhysicalDeviceTypes.cpp
    device/types/IDevice.cpp
    IPeerNode.cpp
    IProtocol.cpp
    gm/GroupManagement.cpp
    lb/LoadBalance.cpp
    sc/CStateCollection.cpp
    Utility.cpp
   )

# Generate PhysicalDeviceTypes files based on contents of device/types directory
add_executable(device/DeviceRegistrationGenerator ${CMAKE_CURRENT_BINARY_DIR}/device/DeviceRegistrationGenerator.cpp)
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/device/PhysicalDeviceTypes.hpp ${CMAKE_CURRENT_BINARY_DIR}/device/PhysicalDeviceTypes.cpp
    COMMAND device/DeviceRegistrationGenerator
    DEPENDS device/DeviceRegistrationGenerator )

# set BROKER_VERSION macro : cp version.h.cmake version.h
configure_file(version.h.cmake ${CMAKE_BINARY_DIR}/src/version.h @ONLY)

add_library(broker ${BROKER_FILES})

