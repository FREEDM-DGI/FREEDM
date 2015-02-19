.. _message_passing:

Message Passing
===============

The message passing allows messages to be exchanged between DGI.
Messages are creating using Google's Protocol Buffers and are sent with a few simple commands inside your DGI module.
Messages are defined in .proto files which are kept in the `Broker/src/messages` directory.
Each module has it's own .proto file.

Creating Your Protocol File
---------------------------

First create a new .proto file in the `Broker/src/messages` folder.
For our VoltVar example, I'll be making `VoltVar.proto`.
The first thing to do in our proto file is to define what package the messages belong to::

    package freedm.broker.vv;
    
Once again, we are using "vv" as the short name for our module.
It's best to try and keep your short name consistent across the system.
Next, we can start creating messages.
Here's a couple::

    message VoltageDelta {
        required uint32 controlFactor = 1;
        required float phaseMeasurement = 2;
        optional string readingLocation = 3;
    }
    
    message LineReadings
    {
        repeated VoltageDelta measurement = 1;
        required string captureTime = 2;
    }
    
We've created two messages, VoltageDelta and LineReadings and listed what these messages will carry.
The first message, VoltageDelta carries and integer "controlFactor" and float "phaseMeasurement" which are required parameters.
The VoltageDelta message can't be sent without those parameters.
It also has an optional third parameter "readingLocation" that contains a string, and is not needed to send the message.
The second message, LineReadings, actually contains any number of VoltageDelta's (including zero!) as well as the time the measurements were captured.

Note the = <somenumber> item we have on each line.
This is to help the protocol buffers library pack each message.
When creating simple message for the DGI, the best practice is to simply use increasing numbers in each message.

You will also need to create a message type for your module that can hold any of the other messages you create.
We will refer to this as a module container message.
This will help the message delivery system get your message to your module, and make it easier to prepare your messages for sending::

    message VoltVarMessage
    {
        optional VoltageDelta voltage_delta_message = 1;
        optional VoltageDelta line_readings_message = 2;
    }
    
Each message type I created previously is now an optional value in my `VoltVarMessage`.
My complete `VoltVar.proto` is below::

    message VoltageDelta {
        required uint32 control_factor = 1;
        required float phase_measurement = 2;
        optional string reading_location = 3;
    }
    
    message LineReadings
    {
        repeated float measurement = 1;
        required string capture_time = 2;
    }
    
    message VoltVarMessage
    {
        optional VoltageDelta voltage_delta_message = 1;
        optional VoltageDelta line_readings_message = 2;
    }
    
The last thing to do is to register your VoltVar messages with the DGI.
Doing so is simple: open ModuleMessage.proto and append your module container message (VoltVarMessage) to the existing list.
You will need to use your shortname prefix to access your message::

    message ModuleMessage
    {
        required string recipient_module = 1;
        optional gm.GroupManagementMessage group_management_message = 2;
        optional sc.StateCollectionMessage state_collection_message = 3;
        optional lb.LoadBalancingMessage load_balancing_message = 4;
        optional ClockSynchronizerMessage clock_synchronizer_message = 5;
        
        /// My New Message!
        optional vv.VoltVarMessage volt_var_message = 6;
    }

Make sure the number you select is not repeated anywhere else.

Protocol Buffers are a powerful library.
This covered basic creation of protocol buffers messages, which should be sufficient to create any module, however, additional documentation for protocol buffers can be found in their official manual. LIIIIIIIIINK

Sending Messages
----------------

In order to send a message, you must first create your module container message (VoltVarMessage), then add the values to the specific message you are sending, and lastly pack the module container message in a ModuleMessage.
The DGI team recommends creating methods for your module for each message type you wish to send as well as a method that packs the module container message into the ModuleMessage. 
Let's make some methods for the messages we created previously::

    ModuleMessage VoltageDelta(unsigned int cf, float pm, std::string loc) {
        VoltVarMessage vvm;
        VoltageDeltaMessage *vdm = vvm.mutable_voltage_delta_message();
        vdm->set_control_factor(cf);
        vdm->set_phase_measurement(