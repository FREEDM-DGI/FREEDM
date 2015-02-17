.. _configuring-devices:

Configuring Devices
===================

There are three options interaction with the power system in the current version of DGI: PSCAD, RTDS, or physical device interaction through the Plug-N-Play protocol.

If you're only going to use Plug-N-Play go ahead and jump to :ref:`pnp-configuration`

It is worth taking note of the purpose of the configuration files you'll be creating:

device.xml
----------
`Used With: PSCAD, RSCAD`

`device.xml` defines the different types of devices available in the FREEDM system. DGI ships with a selection of simple devices that it supports natively in its `sample/device.xml` file. You can create your devices or add new parameters to existing devices by modifying `devices.xml`.

PUT IN A TABLE OF THE DEVICE TYPES THE DGI SHIPS WITH

adapter.xml
-----------
`Used With: PSCAD, RSCAD`

`adapter.xml` defines the devices attached to the system and how their information is organized into a packet to send to a PSCAD or RSCAD simulation. 

Let's get started! Pick which system you want to use:

See :ref:`pscad-configuration`

Or :ref:`rscad-configuration`

Or :ref:`pnp-configuration`