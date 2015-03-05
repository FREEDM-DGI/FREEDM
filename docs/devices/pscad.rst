.. _pscad-simulation:

Configuring PSCAD
=================

Using the DGI with PSCAD involves three components:

    #. The DGI, configured to connect to a "Simulation server" that interfaces with  PSCAD.
    #. A simulation server that provides a buffer between PSCAD and the DGI
    #. A PSCAD simulation modified to connect to the simulation server.

DGI Configuration
-----------------

In order to use the DGI with PSCAD, you'll need to create or modify the following configuration files:

device.xml
^^^^^^^^^^
`device.xml` is required to use devices. The DGI will run without physical devices if the path to this file is not specified in `freedm.cfg`. In other words, the load balance algorithm will perform no meaningful work and the DGI will not attempt to control the power system. The sample file is guaranteed to work with its associated version of DGI.

`device.xml` defines the different types of devices available in the FREEDM system. DGI ships with a selection of simple devices that it supports natively in its `sample/device.xml` file. You can create your devices or add new parameters to existing devices by modifying `devices.xml`.

If you need to create your own device types, or need to add new parameters to existing devices,
see :ref:`device-xml-cfg`

adapter.xml
^^^^^^^^^^^

`Broker/config/adapter.xml` is also required. See :ref:`adapter-xml-cfg`.

Once you have configured your adapter.xml, next you'll configure your simulation server. 
WHAT DO I NEED TO DO TO SET THIS UP.

Configuring Simulation Server
-----------------------------

Using Sockets In PSCAD
----------------------

Running the PSCAD Simulation
----------------------------
