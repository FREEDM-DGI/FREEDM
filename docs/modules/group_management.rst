.. _group-management:


Group Management Reference
==========================

Group management provides consistent list of active DGI processes as well as identifies a primary process for initiating distributed algorithms.

Using Group Management
----------------------

`GMAgent::ProcessPeerList is available by including gm/GroupManagement.hpp`

Group management will send all modules in the system a PeerList message when the list of active processes in the system changes. This message will also contain the identity of the leader process. Group management provides a static method for to aid processing this message.

.. doxygenfunction:: freedm::broker::gm::GMAgent::ProcessPeerList

This method returns a `PeerList` of the processes in the group. The sender of the message will always be the group leader.

An example of processing the PeerList message is in :ref:`receiving-messages`

Physical Topology
-----------------

Group management can be configured to respect the topology of the physical network: it will not group two DGI unless a physical path exists between the two processes (as opposed to a cyber path).

Directions on configuring physical topology can be found in :ref:`physical-topology`

Embedding Group State In Simulation
-----------------------------------

The group state is by manipulating a "Logger" device on the FREEDM system. The value of device is updated each time the check or timeout procedure is called, that is, once at the beginning of the Group Management phase.

Data is stored in the device as a bit field. Because of the way data is passed to and from the RTDS and PSCAD the data is transported as a float, although the individual bits are unaffected by this process, it may be necessary to convert the float to an unsigned integer to be able to preform the bit fiddling needed to access the information.

This setup assumes that all DGI are aware of all other DGI in the system and that all the Group Management status tables are the same. That is, there is not a DGI some DGI is aware of that some other DGI isn't aware of. This is reasonable because of the experiments we are currently running, and the fact that the container for the other DGIs in the system is a map so when iterated the items are returned in alpha numeric order.

**UUIDS ARE ENTERED IN THE BITFIELD IN ASCENDING ALPHANUMERIC ORDER.**

## Bit Field Structure

Note that 2^0 is set based on the endianess of an x86_64 machine.

1. (2^0) - Set to 1 if the DGI editing the device is the coordinator.

1. (2^1) - Set to 1 if the DGI with the first UUID is in the same group as the DGI editing the bitfield

1. (2^2) - Set to 1 if the DGI with the second UUID is in the same group as the DGI editing the bitfield.

1. (2^n) - Set to 1 if the DGI with the nth UUID is in the same group as the DGI editing the bitfield.

Bits of non-members will be set to 0. The SGI will always set the bit relating to its own UUID to 1.

In order to use this feature, you need to define a `Logger` device with a "groupStatus" field. The DGI will write its group state to the first `Logger` device on the system.

Congestion Notifications
------------------------

The group management module supports congestion notifications. The group management module responds to two different type of congestion notifications: hard and soft. A soft notification is generated when a network device suspects network congestions are in a state where the DGI may not meet its real-time communication deadlines.

When the DGI receives a soft notification, the group management module will switch into a slower "maintenance" mode. In this mode, elections are disabled for any DGI that receives the notification. Additionally, the time window for the delivery for the AYC/AYT messages and their responses are extended to compensate for the expected congestion.

When a hard notification is received by a leader, the leader will perform a group division. The leader generates two new groups from its current group and distributes the new group information the processes in its group.

The DGI currently implements a random group split: the leader process will randomly assign processes to one of two groups, attempting to create two equally sized groups. This has been shown to reduce queue usage for large groups. An area for possible improvement it to target group division around the placement of resources and network congestion.

Hard notifications also enable the behavior of soft notifications.

Implementation Details
----------------------

Algorithm
^^^^^^^^^

Group Management is an implementation of the Garcia-Molina Invitation Election Algorithm found in "Elections in a Distributed System" published 1982 in IEEE Transactions on Computers.

GMAgent Reference
^^^^^^^^^^^^^^^^^^^^^^^^^^

.. doxygenclass:: freedm::broker::gm::GMAgent
    :members:
    :protected-members:
    :private-members:
    :no-link:
