////////////////////////////////////////////////////////////////////////////////
/// @file         CDispatcher.cpp
///
/// @author       Derek Ditch <derek.ditch@mst.edu>
/// @author       Michael Catanzaro <michael.catanzaro@mst.edu>
///
/// @project      FREEDM DGI
///
/// @description  Implementation of CDispatcher class. This class models the
///               "Broker" pattern as described in POSA1.
///
/// These source code files were created at Missouri University of Science and
/// Technology, and are intended for use in teaching or research. They may be
/// freely copied, modified, and redistributed as long as modified versions are
/// clearly marked as such and this notice is not removed. Neither the authors
/// nor Missouri S&T make any warranty, express or implied, nor assume any legal
/// responsibility for the accuracy, completeness, or usefulness of these files
/// or any information distributed with these files.
///
/// Suggested modifications or questions about these files can be directed to
/// Dr. Bruce McMillin, Department of Computer Science, Missouri University of
/// Science and Technology, Rolla, MO 65409 <ff@mst.edu>.
////////////////////////////////////////////////////////////////////////////////

#include "CBroker.hpp"
#include "CDispatcher.hpp"
#include "CGlobalPeerList.hpp"
#include "CLogger.hpp"
#include "IDGIModule.hpp"

#include <boost/bind.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/thread/locks.hpp>

namespace freedm {
    namespace broker {

namespace {

/// This file's logger.
CLocalLogger Logger(__FILE__);

}

///////////////////////////////////////////////////////////////////////////////
/// CDispatcher::Instance
/// @description Access the singleton instance of the Dispatcher
/// @pre None
/// @post None
/// @return A reference to the dispatcher.
///////////////////////////////////////////////////////////////////////////////
CDispatcher& CDispatcher::Instance()
{
    static CDispatcher dispatcher;
    return dispatcher;
}

///////////////////////////////////////////////////////////////////////////////
/// CDispatcher::HandleRequest
/// @description Given an input property tree determine which handlers should
///   be given the message out of a pool of modules and schedule the delievery
///   of the message to those modules.
/// @pre Modules have registered their read handlers.
/// @post Message is scheduled to be delivered to the module.
/// @param msg The message to distribute to modules.
/// @param uuid The UUID of the DGI that sent the message.
///////////////////////////////////////////////////////////////////////////////
void CDispatcher::HandleRequest(boost::shared_ptr<const ModuleMessage> msg, std::string uuid)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    Logger.Debug << "Processing message addressed to: " << msg->recipient_module() << std::endl;

    bool processed = false;

    for(std::multimap<boost::shared_ptr<IDGIModule>, const std::string>::const_iterator it
            = m_registrations.begin();
        it != m_registrations.end(); ++it)
    {
        if (it->second == msg->recipient_module() || msg->recipient_module() == "all")
        {
            // Scheduled modules receive messages only during that module's phase.
            // Unscheduled modules receive messages immediately.
            if (CBroker::Instance().IsModuleRegistered(it->second))
            {
                CBroker::Instance().Schedule(
                    it->second,
                    boost::bind(
                        &CDispatcher::ReadHandlerCallback, this, it->first, msg, uuid));
            }
            else
            {
                ReadHandlerCallback(it->first, msg, uuid);
            }
            processed = true;
        }
    }

    if( processed == false )
    {
        Logger.Warn << "Message was not processed by any module:\n" << msg->DebugString();
    }

}

///////////////////////////////////////////////////////////////////////////////
/// CDispatcher::ReadHandlerCallback
/// @description Calls the receiving module's message handler for the received
///		message. 
/// @param h The module that will receive the message.
/// @param msg The message to deliver to that module.
/// @param uuid the UUID of the peer that sent the message.
///////////////////////////////////////////////////////////////////////////////
void CDispatcher::ReadHandlerCallback(
    boost::shared_ptr<IDGIModule> h, boost::shared_ptr<const ModuleMessage> msg, std::string uuid)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    CPeerNode peer;
    try
    {
        peer = CGlobalPeerList::instance().GetPeer(uuid);
    }
    catch(std::runtime_error& e)
    {
        if(CGlobalPeerList::instance().begin() == CGlobalPeerList::instance().end())
        {
            Logger.Info<<"Didn't have a peer to construct the new peer from (might be ok)"<<std::endl;
            return;
        }
        peer = CGlobalPeerList::instance().Create(uuid);
    }
    h->HandleIncomingMessage(msg, peer);
}

///////////////////////////////////////////////////////////////////////////////
/// CDispatcher::RegisterReadHandler
/// @description Registers a module to receive messages addressed to a uuid.
/// Every registeredmodule will additionally receive messages addressed to 
///	"all". Modules can register as "all" to recieve all messages from all
///	modules.
/// @param handler the module that will receive the message
/// @param id this module will receive messages addressed to id. If id is "all"
///		the module will receiver every message from every other module.
///////////////////////////////////////////////////////////////////////////////
void CDispatcher::RegisterReadHandler(
    boost::shared_ptr<IDGIModule> handler, std::string id)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    Logger.Debug << "Registered module listening on " << id << std::endl;
    m_registrations.insert(std::make_pair(handler,id));
}

    } //namespace broker
} // namespace freedm

