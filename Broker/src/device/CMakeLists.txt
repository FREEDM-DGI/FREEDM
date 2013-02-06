# list the device source files
set(
    DEVICE_FILES
    CAdapterFactory.cpp
    CDeviceManager.cpp
    CFakeAdapter.cpp
    CPscadAdapter.cpp
    CRtdsAdapter.cpp
    IBufferAdapter.cpp
    ITcpAdapter.cpp
    PhysicalDeviceTypes.cpp
   )

# compile the device type source files
aux_source_directory(${CMAKE_CURRENT_BINARY_DIR}/types DEVICE_TYPES)

# generate PhysicalDeviceTypes using the generator
set(
    GENERATOR_OUTPUT
    ${CMAKE_BINARY_DIR}/include/device/PhysicalDeviceTypes.hpp
    ${CMAKE_CURRENT_BINARY_DIR}/PhysicalDeviceTypes.cpp
   )

add_custom_command(
    OUTPUT ${GENERATOR_OUTPUT}
    COMMAND ./DeviceRegistrationGenerator.py
    DEPENDS ${DEVICE_TYPES} )

add_custom_target( GeneratedPhysicalDeviceTypes DEPENDS ${GENERATOR_OUTPUT} )

# create the device library
add_library(device ${DEVICE_FILES} ${DEVICE_TYPES})

# don't compile device until codegen has run
add_dependencies(device GeneratedPhysicalDeviceTypes)
