Building The DGI
================

This section gives an outline of the steps required to install the DGI. It is meant to be used as a reference once the remainder of the tutorial has been understood:

    * navigate to the base directory (FREEDM/Broker)
    * ``cmake -DCMAKE_BUILD_TYPE=Release``
    * ``make``
    * ``cp config/samples/\* config/``
    * Configure the DGI
    * run the executable (./PosixBroker)
	
Make sure you have installed all the :ref:`system-requirements`.

The DGI builds its executable ``PosixBroker`` as part of the build process.

To start the build process, navigate to where you have extracted the DGI source code bundle. There are several folders in the extracted bundle, which include documentation for the DGI as well as the DGI build folder. First, change in to the DGI Broker directory which contains the bulk of the DGI code.::

    $ pwd
    /home/scj7t4/FREEDM/Broker

The makefile for the DGI is created by invoking ``cmake`` in the Broker folder::

	cmake -DCMAKE_BUILD_TYPE=Release

CMake verifies that Boost is installed and properly configured. If everything goes well, there will now be a ``makefile`` in the current directory and the DGI can be built by invoking make::

	make

At this point the DGI will build. If you ever make any changes to the DGI code, you can always include them by invoking ``make`` again. When the build process completes there should be a ``PosixBroker`` executable in the ``Broker`` folder. However, in order to use the DGI, we must first configure it.

Go on to :ref:`hostname-static-config`