Load Balancing Module
=====================

Algorithm
---------

The Load Balancing algorithm is published in “Distributed Load Balancing for FREEDM system” by Akella.  
The algorithm labels each process as being in Supply, Demand, or Neutral state and distributes that state to other processes in the group. Processes in the supply state have more generation and storage than load, neutral have equal generation and load, and demand processes have more load than generation.
Each time the algorithm runs, a quantum of power is migrated from a supply process to a demand process. The amount that is migrated is a configurable but static value “Migration Step.”

Algorithm Implementation Overview
---------------------------------

A majority of the work is controlled by the LoadManage() function. The function performs the following actions to complete the algorithm.

#. Schedule Next Round – The first thing the algorithm does is schedule the next time it will run. This will either be in the same phase or in the next phase. This is done to prevent the algorithm from running over its allotted execution time.
#. Read Devices – The state of the attached devices is read. This is done once per round so that the DGI can react to sudden changes in device state during the course of a load balance phase.  Read devices measures the attached SST’s sum gateway value (which is the amount of power it is sending to other processes) and computes the difference between the power available and the attached load (generation + storage – load). This difference is the process’s “net generation”
#. Update State – This function uses the values measured after reading the devices to determine which load balancing state the DGI is in. If the amount of power the DGI is sending to other processes is less than the amount of net generation the SST has, and the DGI has sufficient net generation to send a migration to another process it is in the supply state. If the process is in a state where another quantum of power being drawn through the gateway would help it fulfill a deficit in net generation, the process is in a demand state. If neither of these conditions are met the process is in the normal state.
#. Load Table – This function prints the read device state and the state of the processes in the system to the screen.
#. At this point, the DGI checks to see if a “Logger” type device is attached. If the device is attached its “dgiEnable” value must be set to 1 for the DGI to actually perform a migration. If there is no device attached, the DGI will always attempt to perform a migration. This is useful for avoiding transients in some simulations. If the logger device is attached but the DGI is not enabled the DGI will write the gateway value it read in Read Devices back to the SST.
#. If the DGI process is in the demand state, the DGI sends an announcement to all other processes in its group inform them that it is in demand. 
#. If the DGI process is in the supply state, it will check to see if it has received a message from state collection for the current phase. The collected state is used to set the initial value of a predicted SST gateway value that predicts how the SST's real power injection will change in response to the DGI commands. This predicted value is used during the course of load balance instead of the measured SST real power injection since the power simulation may not respond immediately to DGI commands. If a message has not yet been received from state collection, then the supply node will do nothing for the current round.
#. After load balance receives the collected state message, the DGI will check the invariant. If the supply process determines the invariant hasn’t been violated, it will send a “Draft Request” message to all demand processes to offer them a migration. The DGI will then wait for responses to that message. Once this timeout expires the “Draft Standard” function will run.
#. On receipt of the Draft Request, the receiving process will note the sender is in a supply state and send a “Draft Age” message as a response. This message indicates the amount of demand the receiver has. This value is used by the sender (the supply process) to determine which processes have the greatest need.
#. On receipt of the “Draft Age” message, the supply node places the response in a table. When the timer expires and the “Draft Standard” function runs, the table is processed. Processes with an age of 0 are moved to the normal set. The process with the greatest age is selected. If that process’ age is greater than the size of a migration step, the supply process will send the selected demand process a “Draft Select” message. Additionally, the process will change its gateway value to send power to the selected demand process.
#. On receipt of the Draft Select message, the demand process will determine if it still need the offered power. If it does, it will send a Draft Accept message, and adjust its power levels to accept the incoming power. If it does not need the power it will respond with a Too Late message. If the Too Late message is received the supply node will roll back its half of the transaction with the demand node. If the Malicious flag is set, the DGI will drop the draft select message and not send the draft accept message.
#. On receipt of the Draft Accept message, the supply process notes that the transaction has been completed.
#. This process repeats a fixed number of times each round, determined by potential difference that DGI can accrue between the actual measurements and the predicted value while load balancing.

Invariant Checking
------------------
The invariant check included in DGI 2.0 is an older invariant based on the frequency stability of a microgrid with a single isochronous generator. This model assumed there would be no droop generator used to stabilize frequency, and that large frequency oscillations would occur when the imbalance between generation and load at each SST reached a certain point. As this model is no longer consistent with the HIL-Testbed, the invariant is disabled by default and it is recommended the invariant not be used.

The included invariant was coded for a specific 7-node PSCAD simulation, and the equation used in load balance has hard-coded constants that correspond to that particular simulation. During the course of the simulation, the DGI keeps track of two values: the current frequency, which is measured in PSCAD and sent to each of the DGI, and the current power imbalance. At the start of each phase, the power imbalance is set equal to the isochronous generator's real power as reported by state collection. Then over the course of several load balance rounds, this value is updated by keeping track of incomplete power migrations. A power migration is considered incomplete until Step 12 of the above load manage description where a supply node receives a draft accept message.

To enable the invariant, which again is not recommended, the check-invariant flag must be set to 1 in the DGI configuration file for all DGI processes. A demand process must also set the malicious-behavior flag to 1 to ensure that it does not increase its load during the course of a simulation, leading to the imbalance between generation and load that would lead to frequency oscillations. This flag works by causing a demand node to ignore the draft select message entirely. It neither increases it load, nor sends a response to the supply DGI that increased its generation, leading to an incomplete power migration that increases the imbalance between generation and load. Without the malicious flag, it is highly unlikely that the normal operation of the DGI will lead to potential violations of the invariant.

Migration Size
--------------
The migration step size is set in the FREEDM configuration file and defaults to 2.

Limitations
------------
* The Load Balancing algorithm needs to be able to predict how the devices will react to its commands or the device will need to react instantaneously to its commands. If the device does not react sufficiently quickly enough, Load Balancing will repeatedly issue the same commands to the device and appear to not be working.
* The Load Balancing algorithm will move all power values to within one migration step of perfectly balanced. Since the migration step is currently a static value, the algorithm cannot perfectly balance the system. Additionally, there are many computations that include a migration step in order to prevent the DGIs from oscillating when they are near the perfect balance.

LBAgent Reference
^^^^^^^^^^^^^^^^^^^^^^^^^^

.. doxygenclass:: freedm::broker::lb::LBAgent
    :members:
    :protected-members:
    :private-members:
    :no-link:



