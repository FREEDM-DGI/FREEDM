.. _configure-timings:

Configuring Timings
===================

Timing Settings
------------------

The real time scheduler uses a round robin approach. A module's individual processing time as its "Phase".
A round is composed of a GM Phase, an SC Phase and a LB Phase.

Every DGI instance must have the same timing profile.

Using The Excel Sheet To Create New Timings
---------------------------------------------------

There are three parameters which dictate how the sheet produces values. 
One (TProc) is computed using experimental data and the others (Nodes and TPTM) are adjustable by the user.
To generate a set of timing parameters one must first collect the requisite data.
Here are the parameters that must be collected and the techniques that can be used to find them.
The excel sheet can be found as *FREEDM-Timings.xlsx* in the ``OtherDocs`` folder.

Blue Box
^^^^^^^^^

In the blue shaded area you can change the user-tuneable parameters for the DGI.

No. of DGI -- This value is the size of the largest group you would be able to form.

In Channel Time (aka TPTM) -- You can collect this value by performing a ping, preferably with packets of at least the maximum size you intend to send. Messages tend to stay at less than 1 MTU so approximately 1400 bytes is sufficient. You can also use this to describe the maximum amount of latency the DGI will experience. A higher latency means a higher in channel time.

No. of Migrations -- This value is how many migrations should occur each load-balancing phase. Depending on your circumstances, you may want this to be small (so only one migration happens) or big (so that lots of migrations happen)

Premerge Intervals -- The number of intervals used to handle the race condition in the leader election algorithm. The idea is that the node with the lowest identifier should be the highest priority for becoming the leader. If that node is slow, the next lowest identifier should have priority and so on. The premerge slots them into an ordering for accomplishing this. Tau is the smallest premerge time of the nodes participating in the election. This parameter changes the number of premerge slots available.

Pink Box
^^^^^^^^^^

Values in the pink box are collected in a run of the system. To capture the requisite data, run a small slice of the system with ONLY two (More nodes will mislead the calculations because of how the processing queue works) nodes and capture a log with the Group Management log level set to 6. From there, grep out the lines containing the phrase "after query sent" and copy the timings from these lines into the excel sheet. There's an example of what you are looking for below. These values can go directly into the "AYC Resp. Time" column (it doesn't matter if they are AYC or AYT; the formula is the same). The Processing column computes how much time is spent by the DGI packing and unpacking the message. The more data you collect, the more accurate your timings may be::

    $ ./PosixBroker -v 6 &> timing.dat
    $ cat timing.dat | grep -i "after query sent"
    AYC response received 00:00:00.038365 after query sent
    AYT response received 00:00:00.054224 after query sent
    AYT response received 00:00:00.055469 after query sent
    AYT response received 00:00:00.056330 after query sent
    AYT response received 00:00:00.056489 after query sent
    AYT response received 00:00:00.056032 after query sent
    AYT response received 00:00:00.055191 after query sent
    AYT response received 00:00:00.055169 after query sent
    
Green Box
^^^^^^^^^^
The green box shows how the timings from the pink box are distributed. Q0 is the minimum observed time and Q4 is the maximum. Q2 is the median. If you use timings from Q4, then the processing time allotted is the maximum observed processing time from the values you put in the pink box.

Orange Box
^^^^^^^^^^

The orange box shows the individual events in the system. These events are composed together to create the timings parameters in the purple box.

Purple Box
^^^^^^^^^^

The purple box has each of the timing parameters. Select the desired quartile and transfer the values to the specified parameters.

Parameter Descriptions
-----------------------

Group Management Settings
^^^^^^^^^^^^^^^^^^^^^^^^^^

* GM_PHASE_TIME - The time allotted to Group Management as a whole should be greater than max(GM_AYC_RESPONSE_TIMEOUT + GM_PREMERGE_MAX_TIMEOUT + GM_INVITE_RESPONSE_TIMEOUT,GM_AYT_RESPONSE_TIMEOUT)
* GM_PREMERGE_MIN_TIMEOUT - The minimum amount of time that a node should wait before deciding the node with highest priority has crashed.
* GM_PREMERGE_GRANULARITY - The step size for the premerge timeout.
* GM_PREMERGE_MAX_TIMEOUT - The maximum amount of time that a node should wait before deciding all other nodes with higher priority have crashed.
* GM_AYC_RESPONSE_TIMEOUT - How long nodes have to respond to the coordinator before being removed from the group.
* GM_INVITE_RESPONSE_TIMEOUT - How long nodes have to respond to invitations from a coordinator If you have trouble establishing groups this is a good parameter to adjust.
* GM_AYT_RESPONSE_TIMEOUT - How long the coordinator has to respond to keep alive messages from the member nodes. If groups break often, try increasing this parameter.

Load Balancing Settings
^^^^^^^^^^^^^^^^^^^^^^^^^^^

* LB_PHASE_TIME - Length of the LB phase. Recommended to be set to (LB_GLOBAL_TIMER * Desired number of migrations). It is also recommended to keep the number of migrations per phase to be low.
* LB_ROUND_TIME - Length of time in between individual migrations. Should be set to be longer than the time required to do an individual migration.
* LB_REQUEST_TIMEOUT - The amount of time another process has to respond to a load balancing message.

State Collection Settings
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

* SC_PHASE_TIME - The amount of time needed for state collection to run. It should be set to a value large enough that the state can be collected each time.

Special Parameters
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

* CSRC_RESEND_TIME - The amount of time that the send and wait protocol should wait before considering a packet lost (No ACK). Should be greater than 1 RTT. If this is set too low it will appear that devices are not receiving messages.
* CSRC_DEFAULT_TIMEOUT - The time that a message should be considered to be worth sending if the module does not specify a TTL.

Some More Tips For Working Out Timings
---------------------------------------

Setting the CSRC_RESEND_TIME too low can make it look like all the modules are broken, when in fact, the receiver is being flooded by resent messages. A good diagnostic is to squelch all output except GM's and watch for the AYC queries and replies to be passed back and forth.

Group formation is highly dependent on the GM_INVITE_RESPONSE_TIMEOUT parameter. Setting this value too low will not allow for a sufficient amount of time to collect responses from coordinators and invites. If you see a lot of "Unexpected AYC responses" then this parameter should be increased.

Group stability is dependent on the GM_AYT_RESPONSE_TIMEOUT parameter. If this is set to low the member nodes won't give the coordinator enough time to respond to all their requests, and will leave the group. If you see the group membership changing frequently, or the group id constantly changing, try increasing this parameter.

There is no hard boundary between modules. If your groups are stable, but state collections are not finishing, then consider reducing the number of LB migrations per phase or increasing state collection's time.
Watch the output of the Broker module. It can report how long it is scheduling modules for. If you frequently see modules being schedule for less than their phase time then you should increase the time for other modules; they are plundering another module's time.
