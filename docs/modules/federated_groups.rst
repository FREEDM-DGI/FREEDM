.. _federated-groups:

Federated Groups Module
=======================

The federated groups module implements an approach for managing power resources over a large area. The federated groups module observes the interactions of the group management and load balancing modules to automatically tailor its behavior to the current configuration of the FREEDM system. The federated module exchanges topology, physical load, and group configuration information between coordinators. Coordinators detect when a group has exhausted its local generation resources, or has generation capabilities beyond its own need. Federated groups module then facilitates power transfers between these groups.

The federated groups DGI module will allow the DGI to relax some of the constraints of the hard real-time compared to a DGI without this module. Without a federation module the time requirements for organizing a large number of DGI processes grows quadratically, making the DGI impractical for very large groups. The federation behavior allows the system to be organized into groups that have hard real-time properties along side a soft real-time wide-area layer. Using the federated DGI module, coordinators are presented with an interface that allows them to manage power between groups. By doing so, the DGI system can organize and manage large numbers of groups.
The federated groups module assumes the following properties are present or desirable for the smart grid system.

#.	Fault conditions can be handled within the local groups without a hard real-time reaction from the rest of the system. FIDs and other similar devices can constrain failure within a group to mitigate faults.
#.	Groups should exhaust all resources inside the group before requesting resources from other groups. Similarly, a group with excess resources will immediately offer them to other groups.
#.	The shared bus can tolerate a reasonable amount of excess power (1 quantum/process) being placed on the wide area grid without fault.
#.	Physical topology information is available (See: :ref:`physical-topology`). Additionally, the smart grid can be isolated into logical or physical regions through the placement of “grid ties” to enforce grouping constraints.
#.	The related power management algorithm produces a broadcasted message when a DGI detects it cannot facilitate its local demand. In the Load Balancing module, this is the demand message.

In addition to this, the federated groups also relies on the default behavior of group management to provide information about the current coordinators and the state of fault isolation devices. 
DGI ships with federated grouping behavior implemented for our default load balancing behavior. Other algorithms can be supported with small modifications to the FG module.

General Theory
--------------

#.	The federated groups module collects “Are You Coordinator” response messages and distributes them epidemically to all other coordinators. These messages allow other processes to identify other coordinators in the wide area system. Additionally, these message carry information about the state of the FIDs in the system. FID state is lazily updated at each process running the federated groups algorithm. Unlike Group Management, which assumes an FID is open when its state has not been reported recently, Federated groups assumes the FID remains in its last known state.

#.	The coordinator collects information from the power management algorithm. In particular, it looks for a message indicating a particular process is in a “Demand” state. This message indicates to the coordinator that a process requires additional power.  The coordinator will create a set of processes that have recently generated this message and determine the cardinality of that set. If the cardinality is non-zero the process will attempt to acquire power from other groups. If the cardinality is zero, the process will attempt to acquire power to give to other groups.

#.	If the process is supplying power, is a coordinator, and the group has a zero demand score, it will change its device settings to put power onto the shared bus between groups and increase the “outgoing” power setting on the virtual device to reflect the amount of power being provided to the shared bus. The process will announce it is in a supply state to other coordinators.

#.	If it has at least one demand process in its group, and the “incoming” value on the virtual device is 0, the coordinator will send a “Take” message to another coordinator that has announced it is in the supply state. If that process responds in the positive to that message, the “incoming” value on the virtual device will increase by the migration step size. The responding coordinator will decrease the outgoing value.

#.	In the demand group, once the coordinator has exhausted its local generation, it will decrease the “incoming” value and change a device value to consume power from the shared bus.

#.	If a group changes from the Demand to the Supply state, the amount of Incoming power will be added to the Outgoing power and the Incoming power will be set to 0. Similarly, when a process goes from Supply to demand, the Outgoing power will be added to the Incoming power and the Outgoing power will be set to 0. Both of these interactions allow power that is being placed on the shared bus, but not yet consumed to be used effectively.

The basic operation then when implementing an interaction with the FG module can be summarized as following:

#.	When you have excess generation, increase the “outgoing” value on the virtual device, and supply that power to a shared bus.

#.	Have each process that runs your power management algorithm announce when it has excess demand to the Federated Groups module. This can be done by having FG listen for a message your algorithm already exchanges or it can send its coordinator’s FG process a federated_demand message.

#.	When the “Incoming” value on the virtual device changes, decrease the value and change your device settings to take power from the shared bus.

Interaction With The Virtual Device
-----------------------------------

Federated Groups creates a Virtual Device at each process running the module. This device, of type “Virtual” tracks the state of the algorithm so it can be shared between the module and power algorithms that interact with it. The virtual device can be accessed using any of the methods documented in :ref:`using-devices`. Device values are only changed by Federated Groups when the process is a coordinator. However, values may persist while a process is not a coordinator. Your algorithm can leave these values, or manipulate them while maintaining the invariant depending on the requirements of the model or algorithm.
The virtual device has the following signals:

#.	“incoming” – The value of this signal is the number of quantum of power that have been transferred from one group to this group. Power algorithms decrease this value as they consume power.

#.	“outgoing” – The value of this signal is the number of quantum of power that this group is offering to other groups. Power algorithms increase this value when they have extra power to supply to other groups.

#.	“demand” – The value of this signal indicates the state that the Federated Groups module considers this group to be in. If the value is 1.0, Federated Groups believes at least one process is in the demand state. This value only changes when the process is a coordinator during the Federated Groups module’s execution time.

Load Balancing Module Implementation
------------------------------------

As an example implementation of interaction with the Load Balancing module, the Federated groups module extends the behavior of Load Balancing to allow processes to cooperate between groups. Assume that local resources are consumed first in all cases.

#.	The load balance state is first determined by the attached devices and then the state of the virtual device. If the actual devices are determined to be in the normal state, a second round of consideration with the virtual device is used. If the outgoing power on the virtual device is 0 and the group is not in a demand state, the coordinator will enter a virtual demand state to take power from supply process in its group. If the incoming power is non-zero the coordinator will enter a virtual supply state.

#.	When distributing demand messages, there is now a virtual flag on the state message which indicates if the state is “virtual” because it has been decided using the state of the virtual device. This is later used by the Federated Groups module to determine if the message should be ignored. Virtual demand messages are ignored by the federated groups module for determining the demand score.

#.	If a coordinator is in the demand state, and the incoming power value is non-zero on the virtual device, the coordinator will decrease its gateway and decrease the incoming power value. (Development cases 1, 2, and 3)

#.	If a coordinator is in the supply state, after sending draft request messages to its group, if the leader receives no responses, it will supply some of its generation to be shared using the federated groups module. If the outgoing value is 0, it will increase its gateway and increase the amount of outgoing power. (Development cases 25 and 26)

#.	If a coordinator receives a Draft Request and they are in a virtual demand state they will use a migration step as the “age” for their “Draft Age” message to ensure they have a low priority. On receiving a draft select when the process is in a “virtual demand” state, increase the outgoing value on the virtual device. (Case 16)

#.	When selecting a process to migrate with when a coordinator is in Virtual Supply, the coordinator will decrease its virtual device’s “incoming” value. (Cases 13 and 21)

The following table summarizes the development cases:

+------+----------+----------+-------------------+-------------+--------------------------------------------------------------------------------------------------------------------+
| Case | Incoming | Outgoing | Coordinator State | Group State | How this state can happen                                                                                          |
+======+==========+==========+===================+=============+====================================================================================================================+
| 1    | 1+       | 0        | Demand            | Supply      | "FG has found power from another group. Coord. needs power, members have SUPPLY to give"                           |
+------+----------+----------+-------------------+-------------+--------------------------------------------------------------------------------------------------------------------+
| 2    | 1+       | 0        | Demand            | Normal      | "FG has found power from another group. Coord. needs power, members don't have SUPPLY to give"                     |
+------+----------+----------+-------------------+-------------+--------------------------------------------------------------------------------------------------------------------+
| 3    | 1+       | 0        | Demand            | Demand      | FG has found power from another group. Everyone in this group needs power.                                         |
+------+----------+----------+-------------------+-------------+--------------------------------------------------------------------------------------------------------------------+
| 4    | 0        | 0        | Demand            | Supply      | "A federated transaction has happened, but more will need to occur"                                                |
+------+----------+----------+-------------------+-------------+--------------------------------------------------------------------------------------------------------------------+
| 5    | 0        | 0        | Demand            | Normal      | "A federated transaction has happened, but more will need to occur"                                                |
+------+----------+----------+-------------------+-------------+--------------------------------------------------------------------------------------------------------------------+
| 6    | 0        | 0        | Demand            | Demand      | "A federated transaction has happened, but more will need to occur"                                                |
+------+----------+----------+-------------------+-------------+--------------------------------------------------------------------------------------------------------------------+
| 7    | 0        | 1+       | Demand            | Supply      | Shouldn't be possible -- The demand score should be > 0 which puts this into the virtual demand state              |
+------+----------+----------+-------------------+-------------+--------------------------------------------------------------------------------------------------------------------+
| 8    | 0        | 1+       | Demand            | Normal      | Shouldn't be possible -- The demand score should be > 0 which puts this into the virtual demand state              |
+------+----------+----------+-------------------+-------------+--------------------------------------------------------------------------------------------------------------------+
| 9    | 0        | 1+       | Demand            | Demand      | Shouldn't be possible -- The demand score should be > 0 which puts this into the virtual demand state              |
+------+----------+----------+-------------------+-------------+--------------------------------------------------------------------------------------------------------------------+
| 10   | 1+       | 0        | Normal            | Supply      | Shouldn't be possible -- the group's demand score is 0 so take messages shouldn't be sent. (Group is SUPPLY to FG) |
+------+----------+----------+-------------------+-------------+--------------------------------------------------------------------------------------------------------------------+
| 11   | 1+       | 0        | Normal            | Normal      | Shouldn't be possible -- the group's demand score is 0 so take messages shouldn't be sent. (Group is SUPPLY to FG) |
+------+----------+----------+-------------------+-------------+--------------------------------------------------------------------------------------------------------------------+
| 12   | 1+       | 0        | Normal            | Demand      | "FG has found power from another group. Coord. doesn't need power, members need power"                             |
+------+----------+----------+-------------------+-------------+--------------------------------------------------------------------------------------------------------------------+
| 13   | 0        | 0        | Normal            | Supply      | Group has sold power but nothing else has changed                                                                  |
+------+----------+----------+-------------------+-------------+--------------------------------------------------------------------------------------------------------------------+
| 14   | 0        | 0        | Normal            | Normal      | Group is in a normal state                                                                                         |
+------+----------+----------+-------------------+-------------+--------------------------------------------------------------------------------------------------------------------+
| 15   | 0        | 0        | Normal            | Demand      | "A federated transaction has happened but more will need to occur"                                                 |
+------+----------+----------+-------------------+-------------+--------------------------------------------------------------------------------------------------------------------+
| 16   | 0        | 1+       | Normal            | Supply      | The coordinator cannot sell (they should go into VIRTUAL demand) and the member should migrate power out           |
+------+----------+----------+-------------------+-------------+--------------------------------------------------------------------------------------------------------------------+
| 17   | 0        | 1+       | Normal            | Normal      | Power is balanced and the group doesn't have anything else to offer.                                               |
+------+----------+----------+-------------------+-------------+--------------------------------------------------------------------------------------------------------------------+
| 18   | 0        | 1+       | Normal            | Demand      | Shouldn't be possible -- The demand score should be > 0 which puts this into the virtual demand state              |
+------+----------+----------+-------------------+-------------+--------------------------------------------------------------------------------------------------------------------+
| 19   | 1+       | 0        | Supply            | Supply      | Shouldn't be possible -- the group's demand score is 0 so take messages shouldn't be sent. (Group is SUPPLY to FG) |
+------+----------+----------+-------------------+-------------+--------------------------------------------------------------------------------------------------------------------+
| 20   | 1+       | 0        | Supply            | Normal      | Shouldn't be possible -- the group's demand score is 0 so take messages shouldn't be sent. (Group is SUPPLY to FG) |
+------+----------+----------+-------------------+-------------+--------------------------------------------------------------------------------------------------------------------+
| 21   | 1+       | 0        | Supply            | Demand      | "FG has found power from another group. Coord. has supply and members need power."                                 |
+------+----------+----------+-------------------+-------------+--------------------------------------------------------------------------------------------------------------------+
| 22   | 0        | 0        | Supply            | Supply      | "The group has sold some power to federation but hasn't sold enough to change state."                              |
+------+----------+----------+-------------------+-------------+--------------------------------------------------------------------------------------------------------------------+
| 23   | 0        | 0        | Supply            | Normal      | "The group has sold some power to federation but hasn't sold enough to change state."                              |
+------+----------+----------+-------------------+-------------+--------------------------------------------------------------------------------------------------------------------+
| 24   | 0        | 0        | Supply            | Demand      | "A federated transaction has happened but more may need to occur."                                                 |
+------+----------+----------+-------------------+-------------+--------------------------------------------------------------------------------------------------------------------+
| 25   | 0        | 1+       | Supply            | Supply      | "The whole group can sell power to the grid. The coordinator will sell first then the peers"                       |
+------+----------+----------+-------------------+-------------+--------------------------------------------------------------------------------------------------------------------+
| 26   | 0        | 1+       | Supply            | Normal      | The coordinator has fulfilled his group's demand and can sell power to the grid.                                   |
+------+----------+----------+-------------------+-------------+--------------------------------------------------------------------------------------------------------------------+
| 27   | 0        | 1+       | Supply            | Demand      | Shouldn't be possible -- The demand score should be > 0 which puts this into the virtual demand state              |
+------+----------+----------+-------------------+-------------+--------------------------------------------------------------------------------------------------------------------+

Integration With Other Power Algorithms
---------------------------------------

Other algorithms interacting with the Federated Groups module will need to implement cases 1, 2, 3, 13, 16, 21, 25 and 26. Other cases are handled automatically by Federated Groups to arrive at those cases or represent an unreachable state during normal operation.

If the “incoming” value of the virtual device is greater than zero, some other process in another group has made a change to their device settings (through your algorithm) to give power on the shared bus. 

The “outgoing” value is used to indicate when your algorithm has given power on the shared bus.  When you do so, you should increase the value of the “outgoing” command on the virtual device.

Incoming device values may be larger than a single migration step if multiple processes supplied power to the demand process. Federated groups will automatically manage this power not all of it is consumed by the process that acquires it.

Your algorithm should generate a message to indicate when a process needs power which is consistently delivered to the coordinator of your group each round. Algorithms interacting with Federated Groups can modify federated groups to expect this message, or use the static FGAgent::Demand() method to generate a demand message that Federated Groups will automatically understand.

If you choose to modify Federated Groups for a custom message:

#.	Register a new read handler in PosixMain.cpp for Federated Groups so Federated Groups receives messages from your module.

#.	Create a new message handler in Federated Groups for your demand message. You can do any processing on this message you desire, but if the Federated Groups module should interpret it as a demand message the m_demandscore member variable should be incremented.

#.	Add your new handler to the HandleMessage method.

FGAgent Reference
-----------------

.. doxygenclass:: freedm::broker::fg::FGAgent
    :members:
    :protected-members:
    :private-members:
    :no-link:
