# list the device source files
set(
    DEVICE_FILES
    CAdapterFactory.cpp
    CPhysicalDeviceManager.cpp
    CPscadAdapter.cpp
    CRtdsAdapter.cpp
    IBufferAdapter.cpp
    ITcpAdapter.cpp
    PhysicalDeviceTypes.cpp
   )

# generate the device type source files
aux_source_directory(${CMAKE_CURRENT_BINARY_DIR}/types DEVICE_TYPES)

# create the automatic code generation driver
add_executable(DeviceRegistrationGenerator ${CMAKE_CURRENT_BINARY_DIR}/DeviceRegistrationGenerator.cpp)

# generate PhysicalDeviceTypes using the generator
set(
    GENERATOR_OUTPUT
    ${CMAKE_BINARY_DIR}/include/device/PhysicalDeviceTypes.hpp
    ${CMAKE_CURRENT_BINARY_DIR}/PhysicalDeviceTypes.cpp
   )
add_custom_command(
    OUTPUT ${GENERATOR_OUTPUT}
    COMMAND DeviceRegistrationGenerator
    DEPENDS DeviceRegistrationGenerator ${DEVICE_TYPES} )

# create the device library
add_library(device ${DEVICE_FILES} ${DEVICE_TYPES})
