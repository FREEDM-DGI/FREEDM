.. _configure-device-xml:

Creating a Virtual Device Type
==============================

All physical device types (SST, DESD, FID, etc) must have a corresponding virtual device type defined in the DGI. This virtual device class tells the DGI modules how they can interact with the physical device. Virtual device types are defined in a single XML file located at *config/devices.xml*. This file must either be created, or moved from the samples folder, when the DGI is first installed on a new computer.

.. warning:: The DGI cannot communicate with devices whose type has not been defined in the device.xml configuration file.

Example Device Definition
-------------------------

When a new device type is introduced to the system (such as a new generation of SST), a new virtual device must be defined in this XML configuration file. This tutorial will describe how to modify the device.xml file to introduce a new virtual device type to the DGI. A DESD device with the following properties will be used as an example:

+-------------+--------------------------+----------------------------+
| Device Type | States (Readable Values) | Commands (Writable Values) |
+=============+==========================+============================+
| DESD        | Current,                 | Charge Rate                |
|             | Voltage,                 |                            |
|             | Temperature,             |                            |
|             | State of Charge          |                            |
+-------------+--------------------------+----------------------------+

This sample device meters its internal current, voltage, temperature, and amount of charge. A DGI module can also issue a command to change the charge rate to make the battery charge or discharge. All physical devices should have specifications similar to this sample DESD device, as the DGI's interaction with devices is limited to reading states and issuing commands.

.. note:: The DGI does not support non-numeric values for devices. For instance, the DESD could not have a manufacturer state as the name of a manufacturer is non-numeric.

First examine the structure of the sample configuration file `config/samples/device.xml`.

There is a **<root>** tag which contains several **<deviceType>** subtags. This **<root>** tag is required for all device.xml files, and each device type must be defined under **<root>** in its own **<deviceType>** subtag. To define a new virtual device, the first step is to append an additional **<deviceType>** subtag under **<root>**. If no other devices are defined, then for our tutorial the content of the *device.xml* file should resemble::

    <root>
        <deviceType>
            <!-- (comment) our virtual DESD will be defined here -->
        </deviceType>
    </root>

All the properties of the physical device must be defined under its associated **<deviceType>** subtag. The only required property for a physical device is a unique identifier to differentiate it from other devices. In our case, we are defining a generic DESD device, and so the unique identifier will simply be the string *DESD*. When the DGI needs to access a set of physical devices, it will use this unique identifier in the code. The unique identifier is defined using an **<id>** tag as follows::

    <root>
        <deviceType>
            <id>DESD</id>
        </deviceType>
    </root>

At this point the device has been defined and can be used within the DGI, as although the definition is incomplete for our sample DESD device, all properties of a virtual device other than its unique identifier are optional. However, the sample DESD device has a large number of readable states. Each one of these states must be defined using a separate **<state>** tag. All of the states must be listed in separate **<state>** tags, so **<state>** will appear four times for our DESD device with four unique states::

    <root>
        <deviceType>
            <id>DESD</id>
            <state>current</state>
            <state>voltage</state>
            <state>temperature</state>
            <state>charge</state>
        </deviceType>
    </root>

Again, these string identifiers will be used by the DGI when it attempts to read the current internal state of our new DESD device. The last requirement to finish the definition of our virtual device is to list all of its commands. Commands are specified using a **<command>** tag, and each command must appear within its own tag in the same manner as the states::

    <root>
        <deviceType>
            <id>DESD</id>
            <state>current</state>
            <state>voltage</state>
            <state>temperature</state>
            <state>charge</state>
            <command>chargeRate</state>
        </deviceType>
    </root>

When a state or command consists of multiple words, the recommended approach for its unique identifier is to remove the spaces and capitalize the first letter of each word as in the case of chargeRate. This will reduce the number of potential errors that can be generated by the BOOST XML parser that reads the device.xml configuration file. With this, the device specification for the virtual DESD is complete. It would now be possible to connect the DGI to an actual DESD device using the tutorial on connecting the DGI to physical devices, :ref:`reference-adapters`.

Devices without States or Commands
----------------------------------

Not all devices have both states and commands. A second brief example of an FID will illustrate how to define a device that doesn't have any commands. This device can still be used by DGI modules to read the state of the physical system, but the DGI is unable to control the behavior of the device. Consider the following sample device:

+-------------+--------------------------+----------------------------+
| Device Type | States (Readable Values) | Commands (Writable Values) |
+=============+==========================+============================+
| FID         | status (open, closed)    | none                       |
+-------------+--------------------------+----------------------------+

An FID has no commands as it cannot be controlled. Instead, the status of the FID (whether it is opened or closed) is used by the DGI to determine the current topology of the physical system. When a device contains no commands, the **<command>** tag should be omitted entirely from the device specification. As such, the *device.xml* configuration for this device would be::

    <root>
        <deviceType>
            <id>FID</id>
            <state>status</state>
        </deviceType>
    </root>

In the same manner, a device with no states can also be defined through omission of all the **<state>** tags.

(Advanced) Virtual Device Inheritance
-------------------------------------

This section is primarily intended for computer scientists with a background in programming. Virtual devices support inheritance, and one device definition can inherit from any number of other devices. This can be useful to allow for more powerful queries over devices in DGI modules.

For example, a PVArray (solar panel) is a more specific form of a DRER (generator). A DGI module might want to make a query about the total amount of generation in the system, in which case it would request all instances of the DRER device. However, another module might want to determine the current amount of solar generation, in which case it would request all instances of a PVArray. Because a PVArray must be selected for both of these queries, it must recognize both the DRER and PVArray identifiers. We have chosen to use inheritance to support this functionality. Consider the following device specifications:

+-------------+--------------------------+----------------------------+
| Device Type | States (Readable Values) | Commands (Writable Values) |
+=============+==========================+============================+
| DRER        | real power output        | none                       |
+-------------+--------------------------+----------------------------+
| PVArray     | real power output        | on / off                   |
+-------------+--------------------------+----------------------------+

An **<extends>** tag can be used to allow one device type to inherit from another. For our example, the easiest way to define both devices would be::

    <root>
        <deviceType>
            <id>DRER</id>
            <state>realPower</state>
        </deviceType>
        <deviceType>
            <id>PVArray</id>
            <extends>DRER</extends>
            <command>onOff</command>
        </deviceType>
    </root>

In this case, the PVArray type inherits all the states and commands of the DRER type. When a PVArray device is created in the DGI, modules will be able to access its realPower state inherited from the DRER. In addition, the PVArray will respond to both the DRER and PVArray types when the DGI queries for devices. Note that the order of the type definitions is irrelevant in the *device.xml* configuration file; the PVArray could be defined before the DRER device without error so long as the type it inherits from is eventually defined. 

There is no limit to the depth of the inheritance, or the number of types that can be inherited from. In addition, virtual devices do not have the diamond inheritance problem. Consider the following definitions::

    <root>
        <deviceType>
            <id>A</id>
            <state>appearsOnce</state>
        </deviceType>
        <deviceType>
            <id>B</id>
            <extends>A</extends>
        </deviceType>
        <deviceType>
            <id>C</id>
            <extends>A</extends>
        </deviceType>
        <deviceType>
            <id>D</id>
            <extends>B</extends>
            <extends>C</extends>
        </deviceType>
    </root>

This configuration file would create four virtual device types, with each device type having a single appearsOnce state. This example demonstrates three important points:

1. One device can inherit from multiple others (D extends both B and C).
2. There is no limit on the depth of inheritance (D extends A through B and C).
3. There is no diamond inheritance problem (D doesn't have two appearsOnce states).

For further information on how the DGI supports inheritance in virtual devices, refer to the code at ``Broker/src/device/CDeviceBuilder.cpp`` to see how the *device.xml* file is parsed.

