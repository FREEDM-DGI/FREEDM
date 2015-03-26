Multiple DGI Per Host
=====================

Performance Consideration
-------------------------
Running multiple DGI per machine will affect timings. The provided timings.cfg files are designed with one DGI per host in mind. If you've got a TS-7800 in a group with Intel Core 2 machines, the TS-7800 is not the one that should be running two DGI at once.

Configuring DGI For Multiple Hosts
----------------------------------

To get multiple DGI on the same machine, copy your ``PosixBroker`` executable and the ``/config`` directory to a new location on that machine, then set up the second DGI to use a different port than the first and make sure all of its peers know about the different port. Here are example configuration files for a three node group, with two DGI on one computer and the third on another:

Zapos DGI #1::

    # Portion of freedm.cfg for zapdos.freedm DGI #1

    add-host=zapdos.freedm:50001
    add-host=raikou.freedm:50000

    address=0.0.0.0
    port=50000

Zapdos DGI #2::

    # Portion of freedm.cfg for zapdos.freedm DGI #2

    add-host=zapdos.freedm:50000
    add-host=raikou.freedm:50000

    address=0.0.0.0
    port=50001

Raikou DGI::

    # Portion of freedm.cfg for DGI on raikou.freedm

    add-host=zapdos.freedm:50000
    add-host=zapdos.freedm:50001

    address=0.0.0.0
    port=50000

Since the hostname you choose is the unique identifier of the DGI in its group, it has to be specified exactly the same in each DGI's configuration file and each DGI in the group must be able to resolve the hostname to the same host. This implies that localhost is NEVER a valid hostname in an add-host directive. It won't work for groups on multiple machines, and it won't even work for groups where each DGI is on the same machine since you don't get to specify the hostname that the DGI uses for itself.