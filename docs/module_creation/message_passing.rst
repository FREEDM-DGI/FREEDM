.. _message-passing:

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

    message VoltageDeltaMessage {
        required uint32 control_dactor = 1;
        required float phase_measurement = 2;
        optional string reading_location = 3;
    }
    
    message LineReadingsMessage
    {
        repeated float measurement = 1;
        required string capture_time = 2;
    }
    
We've created two messages, VoltageDeltaMessage and LineReadingsMessage and listed what these messages will carry.
The first message, VoltageDeltaMessage carries and integer "controlFactor" and float "phaseMeasurement" which are required parameters.
The VoltageDeltaMessage can't be sent without those parameters.
It also has an optional third parameter "readingLocation" that contains a string, and is not needed to send the message.
The second message, LineReadingsMessage, contains any number of measurement's (including zero!) as well as the time the measurements were captured.

Note the = <somenumber> item we have on each line.
This is to help the protocol buffers library pack each message.
When creating simple message for the DGI, the best practice is to simply use increasing numbers in each message.

You will also need to create a message type for your module that can hold any of the other messages you create.
We will refer to this as a module container message.
This will help the message delivery system get your message to your module, and make it easier to prepare your messages for sending::

    message VoltVarMessage
    {
        optional VoltageDeltaMessage voltage_delta_message = 1;
        optional LineReadingsMessage line_readings_message = 2;
    }
    
Each message type I created previously is now an optional value in my `VoltVarMessage`.
My complete `VoltVar.proto` is below::

    message VoltageDeltaMessage {
        required uint32 control_factor = 1;
        required float phase_measurement = 2;
        optional string reading_location = 3;
    }
    
    message LineReadingsMessage
    {
        repeated float measurement = 1;
        required string capture_time = 2;
    }
    
    message VoltVarMessage
    {
        optional VoltageDeltaMessage voltage_delta_message = 1;
        optional LineReadingsMessage line_readings_message = 2;
    }
    
The last thing to do is to register your VoltVar messages with the DGI.
Doing so is simple: open ModuleMessage.proto and append your module container message (VoltVarMessage) to the existing list.
You will need to use your short name prefix to access your message::

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

Preparing Messages
------------------

In order to send a message, you must first create your module container message (VoltVarMessage), then add the values to the specific message you are sending, and lastly pack the module container message in a ModuleMessage.
The DGI team recommends creating methods for your module for each message type you wish to send as well as a method that packs the module container message into the ModuleMessage. 
Let's make some methods for the messages we created previously::

    ModuleMessage VVAgent::VoltageDelta(unsigned int cf, float pm, std::string loc) {
        VoltVarMessage vvm;
        VoltageDeltaMessage *vdm = vvm.mutable_voltage_delta_message();
        vdm->set_control_factor(cf);
        vdm->set_phase_measurement(pm);
        vdm->set_reading_location(loc);
        return PrepareForSending(vvm,"vv");
    }

    ModuleMessage VVAgent::LineReadings(std::vector<float> vals)
    {
        VoltVarMessage vvm;
        std::vector<float>::iterator it;
        LineReadingsMessage *lrm = vvm.mutable_line_readings_message();
        for(it = vals.begin(); it != vals.end(); it++)
        {
            lrm->add_measurement(*it);
        }
        lrm->set_capture_time = boost::posix_time::to_simple_string(boost::posix_time::microsec_clock::universal_time());
        return PrepareForSending(vvm,"vv");
    }

These methods prepare our two messages and return them as a ModuleMessage.
The messages are first created as our module container message, VoltVarMessage.
We access the specific child message we want to populate and fill in it's contents.
Send functions expect to receive ModuleMessages, so we have to do a little legwork to convert our VoltVarMessages to a ModuleMessage.
PrepareForSending is a method you'll need to add to your module.
Fortunately, it's pretty simple to make::

    ModuleMessage VVAgent::PrepareForSending(
        const VoltVarMessage& message, std::string recipient)
    {
        Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
        ModuleMessage mm;
        mm.mutable_volt_var_message()->CopyFrom(message);
        mm.set_recipient_module(recipient);
        return mm;
    }

To make this for your own module, you'll need to change the type of the message parameter, and change the "mutable_volt_var_message" to be the correct message type.
PrepareForSending expects two arguments, the original message you wish to convert and a recipient, which is the shortname of the module you wish to receive the message.

Sending Messages
----------------

Messages can be sent to other DGI using the Send method of a CPeerNode.
This method expects one parameter of type ModuleMessage and sends that method the peer the object represents.
The best way to get some CPeerNode's is to use the peers from the PeerList message from group managemnt.
Assuming you have access to `m_peers` from that example, you can sent a message to each peer in the PeerSet like so::

    void VVAgent::MyScheduledMethod(const boost::system::error_code& err)
    {
        if(!err)
        {
            BOOST_FOREACH(CPeerNode peer, m_peers | boost::adaptors::map_values)
            {
                ModuleMessage mm = VoltageDelta(2, 3.0, "S&T");
                peer->Send(mm);
            }
            Logger.Error<<"Schedule!"<<std::endl;
            CBroker::Instance().Schedule(m_timer, boost::posix_time::not_a_date_time,
                boost::bind(&VVAgent::MyScheduledMethod, this, boost::asio::placeholders::error));
        }
        else
        {
            /* An error occurred or timer was canceled */
            Logger.Error << err << std::endl;
        }

    }

And that's it!
PeerSet's have several methods that can help you select individual peers from the set, but in general, sending messages to the entire list, or to the leader is sufficient.

Processing Messages
-------------------

To handle the messages you create, you'll need to add them to your module's HandleIncomingMessage method as well as create handlers for each type of message you'll receive.
Let's do that::

    void VVAgent::HandleIncomingMessage(boost::shared_ptr<const ModuleMessage> msg, CPeerNode peer)
    {
        if(m->has_volt_var_message())
        {
            VoltVarMessage vvm = m->volt_var_message();
            if(vvm.has_voltage_delta_message())
            {
                HandleVoltageDelta(m, peer);
            }
            else if(vvm.has_line_readings_message())
            {
                HandleLineReadings(m, peer);
            }
            else
            {
                Logger.Warn << "Dropped unexpected volt var message: \n" << m->DebugString();
            }
        }
        else if(m->has_group_management_message())
        {
            gm::GroupManagementMessage gmm = m->group_management_message();
            if(gmm.has_peer_list_message())
            {
                HandlePeerList(gmm.peer_list_message(), peer);
            }
            else
            {
                Logger.Warn << "Dropped unexpected group management message:\n" << m->DebugString();
            }
        }
        else
        {
            Logger.Warn<<"Dropped message of unexpected type:\n" << msg->DebugString();
        }
    }

    void VVAgent::HandleVoltageDelta(const gm::PeerListMessage & m, CPeerNode peer)
    {
        Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
        Logger.Notice << "Got VoltageDelta from: " << peer.GetUUID() << std::endl;
        Logger.Notice << "CF "<<m.control_factor()<<" Phase "<<m.phase_measurement()<<std::endl;
    }

    void VVAgent::HandleLineReadings(const gm::PeerListMessage & m, CPeerNode peer)
    {
        Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
        Logger.Notice << "Got Line Readings from "<< peer.GetUUID() << std::endl;
    }

That's it!

