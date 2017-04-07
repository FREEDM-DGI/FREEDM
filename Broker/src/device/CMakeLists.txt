# list the device source files
set(DEVICE_FILES
    CAdapterFactory.cpp
    CDeviceManager.cpp
    CFakeAdapter.cpp
    CRtdsAdapter.cpp
    IBufferAdapter.cpp
    IAdapter.cpp
    CTcpServer.cpp
    CPnpAdapter.cpp
    SynchronousTimeout.cpp
    CDeviceBuilder.cpp
    CDevice.cpp
   )

# create the device library
add_library(device ${DEVICE_FILES})
