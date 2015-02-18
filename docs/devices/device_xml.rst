.. _device-xml-cfg:

Using devices.xml
=================

Devices are specified using an XML configuration file.  A sample configuration file can be found in `config/samples/device.xml`.

Document Structure
------------------

`devices.xml` begins with a `<root>` tag that contains all the devices within.

Each type of device is encapsulated with a `<deviceType>` tag. Each specified device type must have an `<id>` tag. 

Devices can have any number of commands and states, listed using `<command>` and `<state>` tags respectively. Commands are variables that can be written to by DGI to change the behavior of the device. Similarly, states are values that can be read by the DGI.

There is an `<extends>` tag which allows for inhertiance for devices: one device type can inherit all the states and commands of another type. There is no limit on the number of types one device can inherit from, or the depth of the inheritance. Inheritance can be defined using the `<extends>type</extends>` tag.


Tutorial: Adding A New Device Type
----------------------------------

Open the `device.xml` configuration file and add a new tag `<deviceType>` under root. All of the data in this tag will be used to specify the parameters of the new `DeviceExample` device type. Each `<deviceType>` tag requires a unique identifier that serves as the device name in DGI. Add a tag with the name `<id>` as a child of `<deviceType>` and give it the value of `DeviceExample`. The configuration file should now resemble::

    <root>
      <deviceType>
        <id>DeviceExample</id>
      </deviceType>
    </root>

At this point the device has been defined and can be used inside of DGI. Instances of the device could be added to an adapter (if the adapter had a means to add a device without variables, which is not true for all adapters) and accesses through the device manager using the device type `DeviceExample`. However, the device stores no data that can be read by the DGI and has no settings that can be set by the DGI. These must be added with additional tags.

The `DeviceExample` has a `Readable` state which reads a value from some physical meter for use in the DGI. It uses this state to generate a `Writable` command that is used to control the operation of the meter. These are the only two types of device variables supported by the DGI: states that are read by the DGI, and commands that are issued by the DGI to control the physical system. These variables are defined in the configuration file using the `<state>` and `<command>` tags, followed by a unique identifier that names the state or command. In the example::

    <root>
      <deviceType>
        <id>DeviceExample</id>
        <state>Readable</state>
        <command>Writable</command>
      </deviceType>
    </root>

A device can have any number of states and commands. If there is a `SampleDevice` with two states but no commands, it could be specified through repeated use of the `<state>` tag::

      <deviceType>
        <id>SampleDevice</id>
        <state>StateA</state>
        <state>StateB</state>
      </deviceType>

Your simple device is now created!


