///////////////////////////////////////////////////////////////////////////////
/// @file      IHandler.cpp
///
/// @author    Derek Ditch <derek.ditch@mst.edu>
///            Stephen Jackson <scj7t4@mst.edu>
///
/// @compiler  C++
///
/// @project   FREEDM DGI
///
/// @description Provides handlers for module read/write operations 
///
/// @license
/// These source code files were created at as part of the
/// FREEDM DGI Subthrust, and are
/// intended for use in teaching or research.  They may be 
/// freely copied, modified and redistributed as long
/// as modified versions are clearly marked as such and
/// this notice is not removed.
/// 
/// Neither the authors nor the FREEDM Project nor the
/// National Science Foundation
/// make any warranty, express or implied, nor assumes
/// any legal responsibility for the accuracy,
/// completeness or usefulness of these codes or any
/// information distributed with these codes.
///
/// Suggested modifications or questions about these codes 
/// can be directed to Dr. Bruce McMillin, Department of 
/// Computer Science, Missour University of Science and
/// Technology, Rolla, MO  65409 (ff@mst.edu).
///
///////////////////////////////////////////////////////////////////////////////

#include "IHandler.hpp"
#include "CLogger.hpp"
#include "CGlobalPeerList.hpp"

#include <stdexcept>

#include <boost/foreach.hpp>

#define foreach         BOOST_FOREACH

namespace freedm {

namespace broker {

namespace {

/// This file's logger.
CLocalLogger Logger(__FILE__);

}

///////////////////////////////////////////////////////////////////////////////
/// IReadHandler::RegisterSubhandle
/// @description Inserts a handler into the function map.
/// @param key The submessage token to check for
/// @param f A functor which will be called when the key is found. Note that
///     only one functor can be registed for a key and only the first matched
///     key (based on FIFO order they are registered) will be used.
///////////////////////////////////////////////////////////////////////////////
void IReadHandler::RegisterSubhandle(std::string key, SubhandleFunctor f)
{
    m_handlers.push_back(SubhandleContainer::value_type(key,f));
}

///////////////////////////////////////////////////////////////////////////////
/// IReadHandler::HandleRead
/// @description Does some processing on the message before sending to the
/// functor
/// @param msg The CMessage Recieved.
///////////////////////////////////////////////////////////////////////////////
void IReadHandler::HandleRead(freedm::broker::CMessage msg)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    std::string source = msg.GetSourceUUID();
    ptree pt = msg.GetSubMessages();
    CGlobalPeerList::PeerNodePtr peer;
    try
    {
        peer = CGlobalPeerList::instance().GetPeer(source);
    } 
    catch(std::runtime_error &e)
    {
        if(CGlobalPeerList::instance().begin() == CGlobalPeerList::instance().end())

        {
            Logger.Info<<"Didn't have a peer to construct the new peer from (might be ok)"<<std::endl;
            return;
        }
        ConnManagerPtr connmgr = CGlobalPeerList::instance().begin()->second->GetConnectionManager();
        peer = CGlobalPeerList::instance().Create(source,connmgr);
    }
    if(msg.GetHandler() == "")
    {
        throw std::runtime_error("Message didn't specify a handler");
    }
    //Try to find the key in the map, unless its type is any:
    foreach(SubhandleContainer::value_type f, m_handlers)
    {
        try
        {
            if(f.first == "any" || msg.GetHandler() == f.first)
            {
                f.second(msg,peer);
                Logger.Debug<<"Found key "<<f.first<<" in message"<<std::endl;
                return;
            }
        }
        catch(boost::property_tree::ptree_bad_path &e)
        {
            continue;
        }
    }
    Logger.Warn<<"No handlers found for message."<<std::endl;
}

///////////////////////////////////////////////////////////////////////////////
/// IReadHandler::PrehandlerHelper
/// @description Helps create bindings with prehandlers, which a functions that
///     are applied before handlers. Typically this is for code that would be
///     run for several handlers and can do effects like dropping messages.
/// @param f1 The prehandle functor which takes a subhandle functor (and the message information)
///         as arguments. The code of f1 is run and then f1 can make a call to f2.
///         We use partial application to bind f1 down to the type of f2, Prehandlers
///         can be chained.
/// @param f2 The handlerfunctor which handles the actual processing of the message.
/// @return A partially bound f1 (bound to f2) that is of the type of f2
///////////////////////////////////////////////////////////////////////////////
IReadHandler::SubhandleFunctor IReadHandler::PrehandlerHelper(PrehandleFunctor f1,SubhandleFunctor f2)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    return boost::bind(f1, f2,  _1, _2);
}

}

}
