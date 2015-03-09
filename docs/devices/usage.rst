.. _using-devices:

Using Devices in DGI Modules
============================

Once a virtual device type has been defined, and a real physical device has been connected to the DGI, modules can use the devices to read the state of the physical system and send commands to the physical hardware. This tutorial will cover how to use the physical device architecture in DGI modules.

A typical module has the following execution:

1. Retrieve a subset of the physical devices
2. Read the state of the retrieved devices
3. Perform some computation using the state
4. Send commands to a devices based on the computation

This execution pattern corresponds to the three main functions a DGI module can perform using devices:

1. Retrieve a virtual device from the device framework
2. Read the state of a virtual device
3. Send a command to a virtual device

Retrieve a Virtual Device
-------------------------

An object called the device manager is a singleton available to all DGI modules. It stores all of the virtual devices in the system, and provides several functions that enable modules to retrieve a subset of the physical devices. In order to retrieve a device, a module must use the interface provided by the device manager. It is important to recognize that the device manager only stores local devices. Each DGI has a subset of the physical devices in the system, and can not access the devices that do not belong to it. Therefore, no DGI can access the entire system state using its own device manager. In order to read values from devices that belong to other DGI processes, refer to the documentation on state collection.

In order to use the device manager, its header file must be included in your module::

    #include "CDeviceManager.hpp"

The device manager can then be retrieved, and stored if necessary, using its static Instance() function::

    device::CDeviceManager & manager = device::CDeviceManager::Instance();

From the device manager instance, a device can be retrieved through using either its unique identifier or its device type. If a module needs to collect a set of devices of the same type, such as the set of generators in the system, it should use the device type. However, if a module only needs a specific device, such as the one SST associated with the DGI, it should use the device's unique identifier.

Retrieve a Device using its Identifier
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Suppose a module needs to access a device it knows exists with the unique identifier SST5. The following call will store a pointer to that device::

    device::CDevice::Pointer dev;
    dev = device::CDeviceManager::Instance().GetDevice("SST5");

All device pointers must be stored in a :cpp:type:`device::CDevice::Pointer`. The ``device::CDevice::Pointer device::CDeviceManager::GetDevice(std::string)`` function of the device manager can be used to get a pointer to a device with a specific unique identifier, which in this case is SST5. This can be a dangerous function call as there is no guarentee that a device exists with that specific name. If the device manager does not store a device with the given identifier, then it does not throw an exception, but instead returns a null pointer. The pointer can be treated like a boolean truth value to determine whether the call was successful::

    if(!dev)
        // the device was not found! do some recovery action!

Retrieve Devices using their Type
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Suppose a module needs to access all the devices associated with the type DRER. The following call will return a set of the matching devices::

    std::set<device::CDevice::Pointer> devices;
    devices = device::CDeviceManager::Instance().GetDevicesOfType("DRER");

The ``std::set<device::CDevice::Pointer> device::CDeviceManager::GetDevicesOfType(std::string)`` function returns all the devices that associate with a specific type. This function will always return a set of CDevice pointers. If no devices of the specified type are stored in the device manager, then an empty set will be returned. The empty function can be used to determine whether the call was successful at returning any devices::

    if(devices.empty())
        // no devices were found! do some recovery action!

BOOST can be utilized to easily iterate over each device in the resulting set. This requires an additional header to be included in the implementation file::

    #include <boost/foreach.hpp>

And the code to iterate over the result would resemble::

    std::set<device::CDevice::Pointer> devices;
    devices = device::CDeviceManager::Instance().GetDevicesOfType("DRER");
    BOOST_FOREACH(device::CDevice::Pointer dev, devices)
    {
        // dev now stores a pointer to a single DRER device!
    }

Retrieve a Device with an Unknown Identifier
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

There are some cases where a module might not know the name of a specific device, but does know that only a single instance of that device should exist. For example, a DGI should only have a single associated SST device, but a module might not make any assumptions on what the unique identifier for that device could be. In this case, the best solution is to use the ``std::set<device::CDevice::Pointer> device::CDeviceManager::GetDevicesOfType(std::string)`` function with additional error-checking::

    device::CDevice::Pointer dev;
    std::set<device::CDevice::Pointer> devices;
    devices = device::CDeviceManager::Instance().GetDevicesOfType("SST");
    if(devices.size() != 1)
        // unexpected number of devices (should have been 1)! recover!
    dev = *devices.begin();

This code retrieves all of the SST devices, of which there should only be one, and then stores the first SST device in the dev pointer. Be careful with this solution as the dereferencing of the devices set could be disastrous if the set is empty. 

Read a Device State
-------------------

Once a device has been retrieved and stored in a :cpp:type:`device::CDevice::Pointer` object (assumed at this point to be named dev), the device pointer can be used to read a state. This is done through the ``float CDevice::GetState(std::string)`` function, which returns a floating point number that corresponds to the current value of the state known to the DGI::

    float voltage = dev->GetState("voltage");

In this example, if the device did not have a voltage state, the function call would throw an exception. A catch block is required to prevent this exception from causing the DGI to terminate::

    try
    {
        float voltage = dev->GetState("voltage");
    }
    catch(std::exception & e)
    {
        // device does not have a voltage state! recover!
    }

The list of states that are recognized be each device can be found in the *device.xml* configuration file. For each device type, the string identifiers that will not cause exceptions with the GetState call are those specified with the **<state>** tag. To be safe, all uses of the GetState function should be done inside of a try block with a corresponding catch statement.

Set a Device Command
--------------------

A command can be issued to a device pointer using the ``void CDevice::SetCommand(std::string, float)`` function. If the specified command cannot be found, then this function call will throw an exception. The correct usage of this command should resemble::

    try
    {
        dev->SetCommand("rateOfCharge", -0.25);
    }   
    catch(std::exception & e)
    {
        // device does not have a rateOfCharge command! recover!
    }

Example Usage
-------------

The following example code will show how the device framework will be integrated into most modules. In this example, the net generation at a DGI instance is calculated and used to set the charge rate of a battery. As this is an example, the actual calculations involved in the code are nonsensical.

::

    #include "CDeviceManager.hpp"
    #include <boost/foreach.hpp>
    #include <iostream>
    #include <set>

    void YourModule::PerformCalculation()
    {
        std::set<device::CDevice::Pointer> drerSet;
        device::CDevice::Pointer desd;
        float netGeneration;
        float rateOfCharge;
    
        // retrieve the set of DRER devices
        drerSet = device::CDeviceManager::Instance().GetDevicesOfType("DRER");
        if(drerSet.empty())
        {
            std::cout << "Error! No generators!" << std::endl;
            return;
        }
    
        // calculate the net DRER generation
        netGeneration = 0;
        try
        {
            BOOST_FOREACH(device::CDevice::Pointer drer, drerSet)
            {
                netGeneration += drer->GetState("generation");
            }
        }
        catch(std::exception & e)
        {
            std::cout << "Error! Generators did not recognize OUTPUT state!" << std::endl;
            return;
        }
  
        // determine the appropriate battery charge rate (nonsensical)
        rateOfCharge = 0;
        if(netGeneration > 0)
            rateOfCharge = netGeneration;
  
        // retrieve the DESD device
        desd = device::CDeviceManager::Instance().GetDevice("MyDesd");
        if(!desd)
        {
            std::cout << "Error! MyDesd device not found!" << std::endl;
            return;
        }
  
        // set the DESD command
        try
        {
            desd->SetCommand("charge", rateOfCharge);
        }
        catch(std::exception & e)
        {
            std::cout << "Error! Could not set battery CHARGE command!" << std::endl;
        }
    }

These functions should be sufficient for all modules that need to use physical devices. However, additional functions are provided by the device manager. A list of these functions can be obtained from the device manager header file in the DGI code located at ``Broker/src/device/CDeviceManager.hpp``.
