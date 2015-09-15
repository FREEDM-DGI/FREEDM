.. _freedm-cfg-detail:

freedm.cfg options
==================

The following options may be specified in a freedm.cfg file.
Additionally, many of these values may be set at runtime using command line switches.
Use ``PosixBroker -h`` for a full list of options.

add-host
--------
Adds a peer to the DGI system. This DGI will attempt to communicate with the specified peer.
DGI will ignore references to itself in this directive.

Example: ``add-host=raichu.freedm:51870``

address
-------
Specifies an interface to bind the DGI to. Defaults to 0.0.0.0 which listens on all interfaces.

Example: ``address=192.168.1.120``
  
port
-------
Specifies the port the DGI should listen to. ``add-host`` directives on other peers should refer to his port.

Example: ``port=51780``

factory-port
------------
Specifies the port for the plug and play session protocol. If omitted, the protocol is not activated.

See :ref:`pnp-adapter`

Example: ``factory-port=60000``

device-config
-------------

Specifies the configuration file for the device types.
This file defines the types of devices available and the signals for those devices.
If this file is not specified the device framework will not be available.

See :ref:`configure-device-xml`

Example: ``device-config=./config/device.xml``

adapter-config
--------------

Specifies the configuration file for the PSCAD/RSCAD interface.
If this file is not specified the PSCAD/RSCAD interface will not be available.

See :ref:`reference-adapters`

Example: ``adapter-config=./config/adapter.xml``

logger-config
--------------
Specifies the configuration file used to control the verbosity of the loggers in the DGI.
If not specified, the value defaults to ``./config/logger.cfg``

See :ref:`reference-logger`

Example ``logger-config=./config/logger.cfg``


timings-config
---------------
Specifies the configuration file used to set the timings of the DGI.
If not specified, the value defaults to ``./config/timings.cfg``

See :ref:`configure-timings`

Example ``timings-config=./config/timings.cfg``

topology-config
----------------
Specifies the topology file to use if you want FIDs to control the connectivity of DGI.
If not specified, the physical topology feature is disabled.

See :ref:`physical-topology`

Example ``topology-config=./config/physical.cfg``

migration-step
---------------
Specifies the size of quantum of power to use during migrations.
This value should be scaled as appropriate for the design of your physical system.
If not specified, this value defaults to 1.

Example ``migration-step=3``

malicious-behavior
-------------------
Specifies if the DGI should act "maliciously."
This switch will cause this DGI, when it is in a demand state, to ignore accept messages.
This will cause a power imbalance which may drive a system to instability.
Defaults to 0 which disables the behavior.
Setting this value to 1 enables the behavior.

Example ``malicious-behavior=1``

check-invariant
------------------
Enables the evaluation of a physical invariant that should protect the physical system from DGIs using the malicious-behavior flag.
Defaults to 0 which disables the invariant check.
Setting this value to 1 enables the check.
 
Example ``check-invariant=1``

verbose
------------------
Sets the logger level of all loggers in the system.
Individual loggers can be overriden by values in a ``logger.cfg`` file.
If omitted, this value will be set to 5.
Zero is the lowest value, 8 is the highest (most verbose) setting.

See :ref:`reference-logger`

Example ``verbose=0``

devices-endpoint
-------------------
Specify an interface that the devices framework will use to communicate.
If not specified, the devices will use any available interface to communicate.

Example ``devices-endpoint=192.168.1.150``

