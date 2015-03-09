DGI Device Framework
====================

The DGI supports interaction with physical devices such as SSTs and DESDs through its device framework. This framework can communicate with both simulated and real devices. An important distinction is that this communication framework is independent of the protocol used to communicate with other DGI instances.

.. note:: This documentation covers how to connect the DGI to physical power hardware, not other instances of the DGI.

All devices used by the DGI must be defined in the *device.xml* configuration file. If a device is not specified in this file, then the DGI cannot communicate with it. A tutorial for introducing new device types to the DGI can be found at :ref:`configure-device-xml`. The device types supported by the DGI by default, as well as their properties, are included in the following table. These devices can be extended to contain more states and commands than those listed, and are intended as placeholders for the DGI's load balancing algorithm.

+-------------+--------------------------+----------------------------+
| Device Type | States (Readable Values) | Commands (Writable Values) |
+=============|==========================+============================+
| SST         | gateway                  | gateway                    |
+-------------|--------------------------+----------------------------+
| DESD        | storage                  | storage                    |
+-------------|--------------------------+----------------------------+
| DRER        | generation               |                            |
+-------------|--------------------------+----------------------------+
| FID         | state                    |                            |
+-------------|--------------------------+----------------------------+
| Load        | drain                    |                            |
+-------------|--------------------------+----------------------------+
| Logger      | dgiEnable                | groupStatus                |
+-------------|--------------------------+----------------------------+

The communication protocol the DGI uses to communicate with its physical devices depends on the type of physical adapter configured to run with the DGI. For most cases, configuration of the DGI physical adapters requires modification of another *adapter.xml* configuration file. A tutorial for setting up physical adapters, as well as modification of this file, can be found at :ref:`configure-adapters`. The adapter types supported by the DGI are included in the following table.

+--------------+------------------------+-------------------+---------------------+
| Adapter Type | Communication Protocol | Communicates With | Documentation       |
+==============+========================+===================+=====================+
| rtds         | TCP/IP                 | RTDS / PSCAD      | :ref:`rtds-adapter` |
+--------------+------------------------+-------------------+---------------------+
| pnp          | TCP/IP                 | Physical Hardware | :ref:`pnp-adapter`  |
+--------------+------------------------+-------------------+---------------------+
| fake         | none                   | nothing           | undocumented        |
+--------------+------------------------+-------------------+---------------------+

Using devices in DGI modules is done through a class called the device manager. An overview of how modules can use devices, as well as details on the device manager, can be found at :ref:`using-devices`.

Additional documentation for running simulations that communicate with the DGI can be found at :ref:`pscad-simulation` and :ref:`rtds-simulation`.

.. toctree::
    devices
    adapters
    usage
