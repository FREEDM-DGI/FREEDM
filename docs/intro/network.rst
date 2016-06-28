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

Explict Congestion Notification Support
---------------------------------------

The DGI includes experimental support for an explicit congestion notification protocol. This protocol is intended to allow network devices to identify current or impending congestion and then notify connected DGI of that congestion. These notifications allow the DGI to adjust its behavior to protect the physical network and to provide congestion relief to the communication network.

Congestion notifications are transmitted to the DGI via UDP multicast. The DGI listens on multicast group 224.0.0.1. The DGI uses the port 51871 by default, although the port can be changed via a command line argument when starting the DGI. DGI accepts packets of the following form as ECN:

+-----------+-----------------------------------------------------------------+
| Bytes     | Contents                                                        |
+===========+=================================================================+
| 0x00-0x07 | should contain "ECNDGI00"                                       |
+-----------+-----------------------------------------------------------------+
| 0x08-0x08 | should contain 0 if the packet that caused the ECN would be a   |
|           | soft drop, 1 for a hard drop.                                   |
+-----------+-----------------------------------------------------------------+
| 0x09-0x12 | should contain the four byte ip address of the device that      |
|           | originated the packet                                           |
+-----------+-----------------------------------------------------------------+
| 0x13-0x16 | should contain the four byte ip address of the destination for  |
|           | the packet                                                      |
+-----------+-----------------------------------------------------------------+
| 0x17-0x18 | should contain the destination port expressed as two bytes.     |
+-----------+-----------------------------------------------------------------+
| 0x19-0x22 | should contain the current average queue size for the queueing  |
|           | algorithm.                                                      |
+-----------+-----------------------------------------------------------------+

Numerical values and IP addresses are expected to be network byte order.

Individual modules are responsible for their reaction to congestion notifications. Currently, only the Group Management and Load Balancing modules react to congestion notification messages. See the documentation of those modules for the individual reactions to those messages.
