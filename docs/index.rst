.. FREEDM documentation master file, created by
   sphinx-quickstart on Mon Feb 16 17:00:21 2015.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

Welcome to FREEDM's documentation!
==================================

Intro
-----

This tutorial covers the installation and use of the FREEDM `DGI 2.0.0`_. You should acquire the DGI from this link. If you choose to checkout code from the git repository, you should understand that the code you are pulling is experimental and unsupported.

.. _DGI 2.0.0: https://github.com/scj7t4/FREEDM/archive/2.0.0.tar.gz 

This package includes only the DGI. A separate program from connecting the DGI to PSCAD is available at the `PSCAD Repository`_ on github. A typical simulation environment will include multiple DGI, typically three or five, which may run on the same Linux machine or on different machines. The simulation itself runs on Windows, either in PSCAD, in which case the DGI will communicate with the PSCAD Interface on a Linux machine, or in RSCAD with an RTDS, in which case the DGI will communicate with FPGAs. The DGI can also interact with physical devices that implement its :ref:`pnp-adapter` protocol.

.. _PSCAD Repository: https://github.com/FREEDM-DGI/pscad-interface

Obtaining Support
-----------------
If there is an issue with the installation or use of the FREEDM DGI software, please contact the DGI developers at freedm-dgi-grp at <spamfree> mst dot edu.

Getting Started With DGI:
-------------------------

.. toctree::
   :maxdepth: 2
   
   intro/features
   intro/requirements
   intro/building
   intro/network
   intro/configuration

Interacting With Simulations and Physical Devices:
--------------------------------------------------

.. toctree::   
    :maxdepth: 2
    
    devices/index
    devices/rtds_adapter
    devices/pscad
    devices/rtds
    devices/physical_topology
    devices/devices
    devices/adapters
    devices/usage

Creating Modules
----------------

.. toctree::
    :maxdepth: 2
    
    module_creation/start
    module_creation/scheduling
    module_creation/receiving_messages
    module_creation/message_passing
    devices/usage

Advanced Configuration
----------------------

.. toctree::
    :maxdepth: 2
    
    advanced_config/freedm
    advanced_config/multiple_dgi
    advanced_config/hostname_troubleshooting
    advanced_config/timings
    
    
DGI Modules Reference
----------------------
.. toctree::
    :maxdepth: 2
    
    modules/state_collection
    modules/group_management
    modules/load_balance

DGI Framework Reference
-----------------------

.. toctree::
    :maxdepth: 2
    
    reference/broker
    reference/logger
    

Other
==================
* `Doxygen Documentation <./doxygen/index.html>`_
* :ref:`search`
