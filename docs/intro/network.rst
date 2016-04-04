.. _hostname-static-config:

Network Configuration
=====================

In order for clients to correctly connect to each other, each client must have a `single, globally unique` name. To accomplish this in DGI, we use a combination of the hostname of the node and port the DGI instance listens on. It is critical that each DGI has only one name globally that all nodes refer to. `This requires correct configuration on all machines.`

Assigning A Hostname To A Computer
----------------------------------

DGI loads the hostname automatically: there is no need to specify a hostname in ``freedm.cfg``. The hostname DGI loads can be found by running ``hostname``; here is an example from our testing system::

    $ hostname
    raichu.freedm

The hostname is specified in /etc/hostname as the only contents of that file. When you set the hostname you must restart the machine for it to take effect. 

Each machine in the system must have a unique host name specified in ``/etc/hostname``

You can confirm that the hostname has been loaded correctly by invoking `./PosixBroker -u` (again, on raichu.freedm)::

    $ ./PosixBroker -u
    raichu.freedm:1870

Making Other Nodes Reachable
-----------------------------

Each node must also know how to reach each other node by the name you assigned them in the ``/etc/hostname`` file. This is done using ``/etc/hosts``.  Be sure to take special care to ensure that each name in /etc/hosts is exactly as you specified in each of the ``/etc/hostname`` files.

Here is an example of a well done ``/etc/hosts`` file::

    TS7800-4:~# cat /etc/hosts
    127.0.0.1 localhost

    192.168.100.11 MAMBA1
    192.168.100.12 MAMBA2
    192.168.100.13 MAMBA3
    192.168.100.14 MAMBA4
    192.168.100.15 MAMBA5
    192.168.100.16 MAMBA6
    192.168.100.17 TS7800-1
    192.168.100.18 TS7800-2
    192.168.100.19 TS7800-3
    192.168.100.20 TS7800-4
    192.168.100.21 TS7800-5
    192.168.100.22 TS7800-6

* Each line is of the form "ipaddress hostname". 
* The first entry in the file should be "127.0.0.1 localhost"
* The machine should appear in its own hosts file. 
* Make sure that each machine you want to reach is defined in the hosts file.
* Make sure the hostname for each machine is **exactly** how you specified it in that machines ``/etc/hostname``

Restart the machine to make sure the changes are applied correctly.

Once the network has been configured, you can go on to :ref:`configuring-dgi`