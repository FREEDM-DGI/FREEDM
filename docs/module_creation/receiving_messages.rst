.. _receiving-messages:

Receiving Messages
==================

`GMAgent::ProcessPeerList is available by including gm/GroupManagement.hpp`

Messages arrive at your module as "ModuleMessages."
When a message is received, your module's HandleIncomingMessage method is invoked.
This method, which you must implement, should take the ModuleMessage, extract the contents you are interested in, and operate on those contents.
We recommend writing a Handler for each type of message you expect to receive.
Since our VoltVar example doesn't do anything yet, let's just set up the module to process GroupManagement's PeerList message.
This message is sent by the GroupManagement module occasionally to announce when other DGIs are started or stopped.
Let's expand our HandleIncomingMessage function to handle the PeerList message::

    void VVAgent::HandleIncomingMessage(boost::shared_ptr<const ModuleMessage> msg, CPeerNode peer)
    {
        if(m->has_group_management_message())
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
    
When we receive a message, we check to see if it contains a group management message.
If it does, we see if that message is a peer list.
If it is, then we invoke a HandlePeerList method (that we are about to write) to handle that message::

    //Class definition
    class VVAgent
    {
        ...
        private:
        PeerSet m_peers;
        std::string m_leader;
    };

    // HandlePeerList Implementation
    void VVAgent::HandlePeerList(const gm::PeerListMessage & m, CPeerNode peer)
    {
        Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
        Logger.Notice << "Updated peer list received from: " << peer.GetUUID() << std::endl;

        m_peers.clear();

        PeerSet m_peers = gm::GMAgent::ProcessPeerList(m);
        m_leader = peer.GetUUID();
    }

When the peerlist message is received, the HandlePeerList message will be invoked.
This method will update m_peers with the new list of running DGI.
It will also set m_leader, which contains the UUID of the process that is currently managing the active process list.
If needed, you can initialize m_leader in the constructor to GetUUID() as that is a safe startup value.

At this point you can run your module and see that it is receiving peer lists.
It will only receive a peer list when the list of active DGIs change.

Now that you've seen how receiving message works, you can move on creating and handling your own messages: :ref:`message-passing`
