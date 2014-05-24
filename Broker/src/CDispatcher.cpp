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
#include "IMessageHandler.hpp"
#include "CStopwatch.hpp"

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
/// Access the singleton instance of the Dispatcher
///////////////////////////////////////////////////////////////////////////////
CDispatcher& CDispatcher::Instance()
{
    static CDispatcher dispatcher;
    return dispatcher;
}

///////////////////////////////////////////////////////////////////////////////
/// @fn CDispatcher::HandleRequest
/// @description Given an input property tree determine which handlers should
///   be given the message out of a pool of modules and deliever the message
///   as appropriate.
/// @pre Modules have registered their read handlers.
/// @post Message delievered to a module
/// @param msg The message to distribute to modules
/// @param uuid The UUID of the DGI that sent the message
///////////////////////////////////////////////////////////////////////////////
void CDispatcher::HandleRequest(const ModuleMessage& msg, const std::string& uuid)
{
    CStopwatch me(__PRETTY_FUNCTION__);
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    Logger.Debug << "Processing message addressed to: " << msg.recipient_module() << std::endl;

    bool processed = false;

    RegistrationsMap::const_iterator it;
    RegistrationsMap::const_iterator itend;
    CBroker& broker = CBroker::Instance();
    if(msg.recipient_module() == "all")
    {
        it = m_registrations.begin();
        itend = m_registrations.end();
    }
    else
    {
        std::pair<RegistrationsMap::const_iterator, RegistrationsMap::const_iterator> r;
        r = m_registrations.equal_range(msg.recipient_module());
        it = r.first;
        itend = r.second;
    }
    if(it != itend)
        processed = true;
    for(; it != itend; ++it)
    {
        // Scheduled modules receive messages only during that module's phase.
        // Unscheduled modules receive messages immediately.
        if (broker.IsModuleRegistered(it->first))
        {
            CStopwatch me2(std::string(__PRETTY_FUNCTION__)+std::string(" LOOP BODY"));
            broker.Schedule(
                it->first,
                boost::bind(
                    &CDispatcher::ReadHandlerCallback, this, it->second, msg, uuid));
            std::cout<<"Loop for "<<it->first<<"\n";
        }
        else
        {
            ReadHandlerCallback(it->second, msg, uuid);
        }
    }
    if( processed == false )
    {
        Logger.Warn << "Message was not processed by any module:\n" << msg.DebugString();
    }

}

///////////////////////////////////////////////////////////////////////////////
/// Forward a received message to a local DGI module.
///
/// @param h the module to send the message to
/// @param msg the message to send
/// @param uuid the UUID of the DGI that sent
///////////////////////////////////////////////////////////////////////////////
void CDispatcher::ReadHandlerCallback(
    IMessageHandler* h, const ModuleMessage msg, std::string uuid)
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
/// Registers a module to receive messages addressed to id. Every registered
/// module will additionally receive messages addressed to "all". Call this
/// function multiple times if you want to promiscuously listen to messages
/// intended for other modules.
///
/// @param handler the module that will receive the message
/// @param id this module will receive messages addressed to id
///////////////////////////////////////////////////////////////////////////////
void CDispatcher::RegisterReadHandler(
    IMessageHandler* handler, std::string id)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    Logger.Debug << "Registered module listening on " << id << std::endl;
    m_registrations.insert(std::make_pair(id,handler));
}

    } //namespace broker
} // namespace freedm

