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
#include "logger.hpp"
CREATE_EXTERN_STD_LOGS()

#define UNUSED_ARGUMENT(x) (void)x

namespace freedm {
    namespace broker {

CDispatcher::CDispatcher()
{
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;

}

// Called upon incoming message
void CDispatcher::HandleRequest( const ptree &p_mesg )
{
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
    ptree sub_;
    ptree::const_iterator it_;
    std::map< std::string, IReadHandler *>::const_iterator mapIt_;
    std::string key_;

    try
    {
        // Scoped lock, will release mutex at end of try {}
        boost::lock_guard< boost::mutex > scopedLock_( m_rMutex );

        // This allows for a handler to process all messages using
        // the special keyword "any"
        for( mapIt_ = m_readHandlers.lower_bound( "any" );
             mapIt_ != m_readHandlers.upper_bound( "any" );
             ++mapIt_ )
        {
            (mapIt_->second)->HandleRead( p_mesg );
        }
	        
	      // Loop through all submessages of this message to call its
        // handler
        ptree sub_ = p_mesg.get_child("message.submessages");
        for( it_ = sub_.begin(); it_ != sub_.end(); ++it_ )
        {
            Logger::Debug << "Processing " << it_->first
                    << std::endl;

            // Retrieve current key and iterate through all matching
            // handlers of that key. If the key doesn't exist in the
            // map, lower_bound(key) == upper_bound(key).
            key_ = it_->first;
            for( mapIt_ = m_readHandlers.lower_bound( key_ );
                    mapIt_ != m_readHandlers.upper_bound(key_);
                    ++mapIt_ )
            {
                // XXX Not sure if the handler needs the full message
                (mapIt_->second)->HandleRead( p_mesg.get_child("message.submessages"));
            }

            if( m_readHandlers.lower_bound( key_ ) == 
                m_readHandlers.upper_bound( key_)     )
            {
                // Just log this for now
                Logger::Debug << "Submessage '" << key_ << "' had no read handlers.";
            }

        }
        // XXX Should anything be done if the message didn't
        // have any submessages? 
        if( sub_.begin() == sub_.end() )
        {
            // Just log this for now
            Logger::Debug << "Message had no submessages.";
        }
    }
    catch( boost::property_tree::ptree_bad_path &e )
    {
        Logger::Error
            << __PRETTY_FUNCTION__ << " (" << __LINE__ << "): "
            << "Malformed message. Does not contain 'submessages'."
            << std::endl << "\t" << e.what() << std::endl;
    }

}


// Called prior to sending a message
void CDispatcher::HandleWrite( ptree &p_mesg )
{
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
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
           Logger::Debug << "Processing 'any'" << std::endl;  
	  (mapIt_->second)->HandleWrite( p_mesg );
        }

        // Loop through all submessages of this message to call its
        // handler
        ptree sub_ = p_mesg.get_child("message.submessages");
        for( it_ = sub_.begin(); it_ != sub_.end(); ++it_ )
        {
            Logger::Debug << "Processing " << it_->first
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
                Logger::Debug << "Submessage '" << key_ << "' had no write handlers.";
            }
        }


    }
    catch( boost::property_tree::ptree_bad_path &e )
    {
        Logger::Error
            << __PRETTY_FUNCTION__ << " (" << __LINE__ << "): "
            << "Malformed message. Does not contain 'submessages'."
            << std::endl << "\t" << e.what() << std::endl;
    }
}

void CDispatcher::RegisterReadHandler( const std::string &p_type,
        IReadHandler *p_handler )
{
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;

    {
        // Scoped lock, will release mutex at end of {}
        boost::lock_guard< boost::mutex > scopedLock_( m_rMutex );
        m_readHandlers.insert(
                std::pair< const std::string, IReadHandler *>
                    (p_type, p_handler)
        );
    }
}

void CDispatcher::RegisterWriteHandler( const std::string &p_type,
        IWriteHandler *p_handler )
{
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;

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

