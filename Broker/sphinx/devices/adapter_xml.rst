.. _adapter-xml-cfg:

Configuring adapter.xml
=======================

DEFINE THE STRUCTURE OF THE XML DOCUMENT WITHOUT A TUTORIAL HERE.

STATE THE PURPOSE OF EACH TAG IN THE DOCUMENT.


Tutorial: Creating adapter.xml
-----------------------------

    #. Create a new file named `adapter.xml` in the `Broker/config/` directory.
    #. Open `Broker/config/freedm.cfg` and add the key `adapter-config=config/adapter.xml` on its own line.  Note that the path is relative to the `Broker/` directory and thus requires the `config/` prefix.
    #. Remove or comment (using #) all other references to `adapter-config` in `freedm.cfg`.

Next, open your new `adapter.xml` file.
    
    #. Add the tag `<root>` to the XML file.
    #. Add the tag `<adapter name = "X" type = "rtds">` as a child of `<root>`.  The value for _name_ must be unique and cannot be shared by multiple adapters.

The file should now resemble::

    <root>
        <adapter name = "MyAdapter" type = "rtds">
        </adapter>
    </root>

Define the State Data
----------------------

The word **state** refers to a value produced by the remote peer and consumed by the FREEDM-DGI.  In other words, a meter reading from some device that is sent as an input to the FREEDM-DGI.  All of the state values sent by an adapter must be specified in the XML specification file.

    #. Add the tag `<state>` as a child of the new `<adapter>`.
    #. For each state variable, repeat the following steps:
    #. Add a new tag `<entry index = "i">` under `<state>`.  The value of `i` must begin with 1 for the first entry and be consecutive for subsequent entries.  This `index` refers to the position of the state variable in an array if the adapter communicates with byte streams.  Therefore, for some adapters, the index must correspond to the actual position of the state variable in the data stream, whereas in other adapters the value is arbitrary.
    #. Add the tag `<device>` under `<entry>`.  The value of this tag represents a unique identifier for some device in the system.  As each state variable belongs to some device, this tag specifies the name of the device associated with the next state variable.  The same device can be specified over multiple entries if that device has multiple state variables.
    #. Add the tag `<signal>` under `<entry>`.  The value of this tag represents the name of the next state variable.  For instance, an SST device would have a `gateway` signal to represent its net generation.  Note that the value for this tag is not arbitrary and each device has a set number of signals it recognizes.  The string identifiers for these signals can be found inside the file `device.xml`. The combination of the `device` and `signal` tags must be unique across all adapters as it is the key for each entry.
    #. Add the tag `<type>` under `<entry>` and set its value to the string identifier of the desired physical device. A list of device classes is in `devices.xml` and the string identifiers are the case-sensitive strings that appears in the `<id>` field of each device type. Therefore, if this entry specifies the `gateway` state variable for an SST device, the tag would have the value `Sst`.

At this point the file should resemble::

    <root>
        <adapter name = "MyAdapter" type = "rtds">
            <state>
                <entry index = "1">
                    <device>MySstDevice</device>
                    <signal>gateway</signal>
                    <type>Sst</type>
                </entry>
            </state>
        </adapter>
    </root>

Define the Command Data
-----------------------

The word command refers to a value produced by the FREEDM-DGI and consumed by the device. The command specification is identical to the state specification except it uses the tag `<command>` instead of `<state>`.  Refer to the above section on state data for the detailed steps of the specification.

At this point the file should resemble::

    <root>
        <adapter name = "MyAdapter" type = "rtds">
            <state>
                <entry index = "1">
                    <device>MySstDevice</device>
                    <signal>gateway</signal>
                    <type>Sst</type>
                </entry>
            </state>
            <command>
                <entry index = "1">
                    <device>MyDesd</device>
                    <signal>storage</signal>
                    <type>Desd</type>
                </entry>
            </command>
        </adapter>
    </root>

Connecting To The Simulation
----------------------------
    #. Add the tag `<info>` as a child of `<adapter>`.
    #. Add the tag `<host>` as a child of `<info>`.  The value of this tag should be the hostname of the simulation interface.  This hostname will be used to initialize a TCP socket connection with the interface when the adapter is constructed.
    #. Add the tag `<port>` as a child of `<info>`.  The value of this tag should be the port number used to communicate with the simulation interface.

At this point the file should resemble::

    <root>
        <adapter name = "MyAdapter" type = "rtds">
            <state>
                <entry index = "1">
                    <device>MySstDevice</device>
                    <signal>gateway</signal>
                    <type>Sst</type>
                </entry>
            </state>
            <command>
                <entry index = "1">
                    <device>MyDesd</device>
                    <signal>storage</signal>
                    <type>Desd</type>
                </entry>
            </command>
            <info>
                <host>r-facts3.device.mst.edu</host>
                <port>5001</host>
            </info>
        </adapter>
    </root>

Troubleshooting
---------------

There are a large number of exceptions associated with the adapter specification due to the importance of correct configuration for runtime.  Most of these exceptions will occur immediately after the FREEDM-DGI is run.  If the FREEDM-DGI runs longer than several seconds, it can be assumed that the configuration was accepted and will produce no future errors.

The following rules must be followed to avoid exceptions:

#. The **name** attribute of each adapter must be unique.
#. The **type** attribute of each adapter must be one of listed, valid types.
#. Neither `<state>` nor `<command>` can be omitted.
#. Entries must begin with index 1 and indexes must be consecutive.
#. A `<device>` cannot be specified in multiple adapters.  Each `<device>` identifier must be restricted to a single adapter.  Within the adapter, the device can have any number of state variables.
#. The `<signal>` must be a string recognized by its associated device.
#. A `<device>` + `<signal>` pair must be unique and specified once across all entries in the specification. 
#. The `<type>` must be recognized by the system as a valid type.  The type should be the `<id>` of a device type located in the `devices.xml` file.