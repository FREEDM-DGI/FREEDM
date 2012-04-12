# list the source files for the project
set(
    BROKER_FILES
    CLogger.cpp
    CBroker.cpp
    CReliableConnection.cpp
    CListener.cpp
    CConnectionManager.cpp
    IProtocol.cpp
    CSRConnection.cpp
    CSUConnection.cpp
    CConnection.cpp
    CDispatcher.cpp
    CMessage.cpp
    IPeerNode.cpp
    gm/GroupManagement.cpp
    lb/LoadBalance.cpp
    sc/CStateCollection.cpp
    CTableStructure.cpp
    CTableRTDS.cpp
    CLineClient.cpp
    CClientRTDS.cpp
    device/CDeviceStructureGeneric.cpp
    device/CDeviceKeyCoupled.cpp
    device/CPhysicalDeviceManager.cpp
    device/CDeviceStructurePSCAD.cpp
    device/CDeviceStructureRTDS.cpp
    device/CDeviceFactory.cpp
    device/PhysicalDeviceTypes.cpp
    device/types/IDeviceLWI.cpp
    device/types/IDevice.cpp
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

