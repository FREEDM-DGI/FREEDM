.. _reference-logger:

Using The DGI Logger
====================

Note: The DGI is a real-time system and its correctness depends on its performance. However, the amount of data generated at high verbosity reduces the performance of the system. Were it to be run at full verbosity, it would not operate correctly. We generally recommend running the DGI with a global verbosity level of 4 (Status) or 5 (Notice).

The DGI provides nine levels of logging

* 0 - Fatal
* 1 - Alert
* 2 - Error
* 3 - Warn
* 4 - Status
* 5 - Notice
* 6 - Info
* 7 - Debug
* 8 - Trace

There are three methods for adjusting the logging level:

* Set verbosity=n in freedm.cfg to specify the global verbosity
* Call PosixBroker with --verbose=n to override the global verbosity specified in freedm.cfg
* Override the global verbosity for a particular source file in logger.cfg

As greater verbosity levels are desired, a sacrifice in performance must be made (by increasing the timings as explained in :ref:`configure-timings`.

Sometimes you will want more data from a particular source file or module. In this case you should utilize logger.cfg. For example, you may want to run Group Management at greater verbosity in order to pick up AYT and AYC response delays, which are printed at level 6 (Info).

## Editing logger.cfg

The logger.cfg in the sample folder looks like this::

    # Example of how to set a file's verbosity:
    #
    # StateCollection.cpp=2
    #
    # Note that the path to the file is not included.
    # Any files not added here are set to the verbosity specified in freedm.cfg
    #
    # Valid Verbosity Levels:
    # 8 - Trace
    # 7 - Debug
    # 6 - Info
    # 5 - Notice
    # 4 - Status
    # 3 - Warn
    # 2 - Error
    # 1 - Alert
    # 0 - Fatal
    #
    # If a file does not have a logger, you will receive an "Unknown Option" error!!

The location of logger config is specified in ``freedm.cfg`` the default value is set to ``./config/logger.cfg``.

Logger levels are specified by setting a specific logger to a specific level. Most files in DGI have their own logger dedicated for their output. Therefore, most ``.cpp`` files have a logger attached to them that can be used for output. Each module has its own logger.

* ``StateCollection.cpp`` - The logger for State Collection
* ``GroupManagement.cpp`` - The logger for Group Management
* ``LoadBalance.cpp`` - The logger for load balancing.

To set a logger for a file, add a line that looks like::

    StateCollection.cpp=2

Archiving DGI Runs
------------------

The DGI generates a significant amount of data when it is run. This data is saved by DGIProcess, Distributed Timestamp, and Event into a cloud archiver, one file for each DGI. By default the data is sent to Unix stderr as the DGI runs. To redirect the data into the cloud archiver over the communications network, redirect stderr when starting the DGI. The following command runs the DGI and redirects stderr to stdout and pipes it to tee, which will save the data to a file named cloud.DGIx while also printing it to the screen::

`./PosixBroker 2>&1 | tee cloud.DGIx`

where DGIx is the DGI process on a particular SST, x.

The data is stored in the cloud in a text format database, allowing it to be processed by a wide range of existing text processing facilities. We expect the Unix "grep" command to be extremely useful for querying the database given its advanced regular expression capabilities and adjustable levels of output context. If you're interested in grabbing AYT response delays, grep cloud.DGIx for "AYT response received" and then format the result to your liking with gawk or copy it directly into a spreadsheet. As there is really no limit to the data processing capabilities already available for text-based data, you can use whatever tools you want to do whatever you need to do.
