////////////////////////////////////////////////////////////////////
/// @file      CDispatcher.cpp
///
/// @author    Derek Ditch <derek.ditch@mst.edu>
///
/// @compiler  C++
///
/// @project   FREEDM DGI
///
/// @description Implementation of CDispatcher class. This class
///   models the "Broker"
/// pattern as described in POSA1.
/// @sa CDispatcher, CServer
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
/// Computer Science, Missouri University of Science and
/// Technology, Rolla, MO  65409 (ff@mst.edu).
////////////////////////////////////////////////////////////////////

#include <boost/thread/locks.hpp>

#include "CDispatcher.hpp"
#include "CLogger.hpp"

#define UNUSED_ARGUMENT(x) (void)x

namespace freedm {
    namespace broker {
        
static CLocalLogger Logger(__FILE__);

///////////////////////////////////////////////////////////////////////////////
/// @fn CDispatcher::CDispatcher
/// @description Dispatcher constructor
/// @pre None
/// @post None
///////////////////////////////////////////////////////////////////////////////
CDispatcher::CDispatcher()
{
    Logger.Debug << __PRETTY_FUNCTION__ << std::endl;

}

///////////////////////////////////////////////////////////////////////////////
/// @fn CDispatcher::HandleRequest
/// @description Given an input property tree determine which handlers should
///   be given the message out of a pool of modules and deliever the message
///   as appropriate.
/// @pre Modules have registered their read handlers.
/// @post Message delievered to a module
/// @param msg The message to distribute to modules
///////////////////////////////////////////////////////////////////////////////
void CDispatcher::HandleRequest(CBroker &broker, CMessage msg)
{
    Logger.Debug << __PRETTY_FUNCTION__ << std::endl;
    ptree sub_;
    ptree::const_iterator it_;
    std::map< std::string, IReadHandler *>::const_iterator mapIt_;
    std::string key_;
    ptree p_mesg = static_cast<ptree>(msg);

    try
    {
        // Scoped lock, will release mutex at end of try {}
        //boost::lock_guard< boost::mutex > scopedLock_( m_rMutex );

        // This allows for a handler to process all messages using
        // the special keyword "any"
        for( mapIt_ = m_readHandlers.lower_bound( "any" );
             mapIt_ != m_readHandlers.upper_bound( "any" );
             ++mapIt_ )
        {
            try
            {
                CBroker::BoundScheduleable x = boost::bind(&CDispatcher::ReadHandlerCallback,
                    this, mapIt_->second, msg);
                broker.Schedule(m_handlerToModule[mapIt_->second],x);
            }
            catch( boost::property_tree::ptree_bad_path &e )
            {
                Logger.Warn<<"Module failed to read message"<<std::endl;
            }
        }
	        
	      // Loop through all submessages of this message to call its
        // handler
        ptree sub_ = p_mesg.get_child("message.submessages");
        for( it_ = sub_.begin(); it_ != sub_.end(); ++it_ )
        {
            Logger.Debug << "Processing " << it_->first
                    << std::endl;

            // Retrieve current key and iterate through all matching
            // handlers of that key. If the key doesn't exist in the
            // map, lower_bound(key) == upper_bound(key).
            key_ = it_->first;
            // Special keyword any which gives the submessage to all modules.
            if( key_ == "any")
            {
                for( mapIt_ =  m_readHandlers.begin(); 
                     mapIt_ != m_readHandlers.end();
                     ++mapIt_)
                {
                    try
                    {
                        CBroker::BoundScheduleable x = boost::bind(&CDispatcher::ReadHandlerCallback,
                            this, mapIt_->second, msg);
                        broker.Schedule(m_handlerToModule[mapIt_->second],x);
                    }
                    catch( boost::property_tree::ptree_bad_path &e )
                    {
                        Logger.Warn<<"Module failed to read message (any field)"<<std::endl;
                    }
                }
            }
            else
            {
                for( mapIt_ = m_readHandlers.lower_bound( key_ );
                        mapIt_ != m_readHandlers.upper_bound(key_);
                        ++mapIt_ )
                {
                    CBroker::BoundScheduleable x = boost::bind(&CDispatcher::ReadHandlerCallback,
                        this, mapIt_->second, msg);
                    broker.Schedule(m_handlerToModule[mapIt_->second],x);
                }

                if( m_readHandlers.lower_bound( key_ ) == 
                    m_readHandlers.upper_bound( key_)     )
                {
                    // Just log this for now
                    Logger.Debug << "Submessage '" << key_ << "' had no read handlers.";
                }
            }
        }
        // XXX Should anything be done if the message didn't
        // have any submessages? 
        if( sub_.begin() == sub_.end() )
        {
            // Just log this for now
            Logger.Debug << "Message had no submessages.";
        }
    }
    catch( boost::property_tree::ptree_bad_path &e )
    {
        Logger.Error
            << __PRETTY_FUNCTION__ << " (" << __LINE__ << "): "
            << "Malformed message. Does not contain 'submessages'."
            << std::endl << "\t" << e.what() << std::endl;
    }

}

void CDispatcher::ReadHandlerCallback(IReadHandler *h, CMessage msg)
{
    (h)->HandleRead(msg);
}

///////////////////////////////////////////////////////////////////////////////
/// @fn CDispatcher::HandleWrite
/// @description Handles calling modules write handlers which allows them to
///   touch messages before they are sent.
/// @pre Write handlers have been registered with the dispatcher
/// @post The outgoing message is touched before being delivered.
/// @param p_mesg The message to affect before sending them out.
///////////////////////////////////////////////////////////////////////////////
void CDispatcher::HandleWrite( ptree &p_mesg )
{
    Logger.Debug << __PRETTY_FUNCTION__ << std::endl;
    ptree sub_;
    ptree::const_iterator it_;
    std::map< std::string, IWriteHandler *>::const_iterator mapIt_;
    std::string key_;

    try
    {
        // Scoped lock, will release mutex at end of try {}
        boost::lock_guard< boost::mutex > scopedLock_( m_wMutex );

        // This allows for a handler to process all messages using
        // the special keyword "any". This happens before the other
        // sections in case a specific handler depends upon a
        // general handler
        for( mapIt_ = m_writeHandlers.lower_bound( "any" );
                mapIt_ != m_writeHandlers.upper_bound( "any" );
                ++mapIt_ )
        {
           Logger.Debug << "Processing 'any'" << std::endl;  
	  (mapIt_->second)->HandleWrite( p_mesg );
        }

        // Loop through all submessages of this message to call its
        // handler
        ptree sub_ = p_mesg.get_child("message.submessages");
        for( it_ = sub_.begin(); it_ != sub_.end(); ++it_ )
        {
            Logger.Debug << "Processing " << it_->first
                    << std::endl;

            // Retrieve current key and iterate through all matching
            // handlers of that key. If the key doesn't exist in the
            // map, lower_bound(key) == upper_bound(key).
            key_ = it_->first;
            for( mapIt_ = m_writeHandlers.lower_bound( key_ );
                    mapIt_ != m_writeHandlers.upper_bound(key_);
                    ++mapIt_ )
            {
                (mapIt_->second)->HandleWrite( p_mesg );
            }

            // XXX Should anything be done if the message didn't
            // have a handler?
            if( m_writeHandlers.lower_bound( key_ ) == 
                m_writeHandlers.upper_bound( key_)     )
            {
                // Just log this for now
                Logger.Debug << "Submessage '" << key_ << "' had no write handlers.";
            }
        }


    }
    catch( boost::property_tree::ptree_bad_path &e )
    {
        Logger.Error
            << __PRETTY_FUNCTION__ << " (" << __LINE__ << "): "
            << "Malformed message. Does not contain 'submessages'."
            << std::endl << "\t" << e.what() << std::endl;
    }
}

///////////////////////////////////////////////////////////////////////////////
/// @fn CDispatcher::RegisterReadHandler
/// @description Registers a module that provides a read handler with the
///   dispatcher.
/// @pre A module that inherits from IReadHandler is provided.
/// @post A module is registered with a read handler.
/// @param p_type the tree key used to identify which messages the module
///   would like to recieve.
/// @param p_handler The module which will be called to recieve the message.
///////////////////////////////////////////////////////////////////////////////
void CDispatcher::RegisterReadHandler(const std::string &module, const std::string &p_type,
        IReadHandler *p_handler)
{
    Logger.Debug << __PRETTY_FUNCTION__ << std::endl;

    {
        // Scoped lock, will release mutex at end of {}
        boost::lock_guard< boost::mutex > scopedLock_( m_rMutex );
        m_readHandlers.insert(
                std::pair< const std::string, IReadHandler *>
                    (p_type, p_handler));
        m_handlerToModule.insert(std::pair<IReadHandler *,
                const std::string>(p_handler,module));
    }
}

///////////////////////////////////////////////////////////////////////////////
/// @fn CDispatcher::RegisterWriteHandler
/// @description Registers a module that provides a write handler with the
///   dispatcher.
/// @pre A module that inhertis from IWriteHandler is provided
/// @post The module will be registered to touch outgoing messages that contain
///   the p_type key.
/// @param p_type A ptree key that will be used to identify which messages 
///   should be touched.
/// @param p_handler The module that will be invoked to perform the touch
///////////////////////////////////////////////////////////////////////////////
#pragma GCC diagnostic ignored "-Wunused-parameter"
void CDispatcher::RegisterWriteHandler(const std::string &module, const std::string &p_type,
        IWriteHandler *p_handler )
{
    Logger.Debug << __PRETTY_FUNCTION__ << std::endl;
    {
        // Scoped lock, will release mutex at end of {}
        boost::lock_guard< boost::mutex > scopedLock_( m_wMutex );
        m_writeHandlers.insert(
                std::pair< const std::string, IWriteHandler *>
                    (p_type, p_handler)
        );
    }
}

    } //namespace broker
} // namespace freedm

