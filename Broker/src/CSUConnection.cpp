////////////////////////////////////////////////////////////////////
/// @file      CSUConnection.cpp
///
/// @author    Derek Ditch <derek.ditch@mst.edu>, Stephen Jackson <scj7t4@mst.edu>
///
/// @compiler  C++
///
/// @project   FREEDM DGI
///
/// @description Declare CConnection class
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
/// Technology, Rolla, MO 65409 (ff@mst.edu).
////////////////////////////////////////////////////////////////////

#include "CSUConnection.hpp"
#include "CMessage.hpp"

#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#include <iomanip>
#include <set>

namespace freedm {
    namespace broker {

CSUConnection::CSUConnection(CConnection *  conn)
    : IProtocol(conn), m_timeout(conn->GetSocket())
{
    m_outseq = 0;
    m_inseq = 0;
    m_acceptmod = SEQUENCE_MODULO/WINDOW_SIZE;
}

void CSUConnection::Send(CMessage msg)
{
    ptree x = static_cast<ptree>(msg);
    unsigned int msgseq;

    CMessage outmsg(x);
    
    msgseq = m_outseq;
    outmsg.SetSequenceNumber(msgseq);
    m_outseq = (m_outseq+1) % SEQUENCE_MODULO;

    outmsg.SetSourceUUID(GetConnectionManager().GetUUID());
    outmsg.SetSourceHostname(GetConnectionManager().GetHostname());

    QueueItem x;

    x.ret = MAX_RETRIES;
    x.msg = outmsg;

    m_window.push_back(x);
    
    if(m_queue.size() <= WINDOW_SIZE)
    {
        Write(outmsg);
        m_timeout.cancel();
        m_timeout.expires_from_now(boost::posix_time::milliseconds(50));
        m_timeout.async_wait(boost::bind(&CSUConnection::Resend,this,
            boost::asio::placeholders.error)); 
    }
}

void CSUConnection::Resend(boost::system::error_code& err)
{
    if(!err)
    {
        int ws = m_window.size();
        int writes = 0;
        for(int i=0; i < ws; i++)
        {
            QueueItem f = m_window.pop_front();
            if(f.ret > 0 && writes < WINDOW_SIZE)
            {        
                Write(f.msg);
                writes++;
                f.ret--;
            }
            if(f.ret > 0) m_window.push_back(f);
        }
        if(m_window.size() > 0)
        {
            m_timeout.cancel();
            m_timeout.expires_from_now(boost::posix_time::milliseconds(50));
            m_timeout.async_wait(boost;:bind(&CSUConnection::Resend,this
                boost::asio::placeholders.error));
        }
    }
}

void CSUConnection::RecieveACK(const CMessage &msg)
{
    unsigned int seq = msg.GetSequenceNumber();
    while(m_window.size() > 0)
    {
        unsigned int fseq = m_window.front().msg.GetSequenceNumber();
        unsigned int bounda = fseq;
        unsigned int boundb = (fseq+WINDOW_SIZE)%SEQUENCE_MODULO;
        if(bounda <= seq || (seq < boundb and boundb < bounda))
        {
            m_window.pop_front();
        }
        else
        {
            break;
        }
    }
    if(m_window.size() > 0)
    {
        m_timeout.cancel();
        m_timeout.expires_from_now(boost::posix_time::milliseconds(50));
        m_timeout.async_wait(boost;:bind(&CSUConnection::Resend,this
            boost::asio::placeholders.error));
    }
}

void CSUConnection::Recieve(const CMessage &msg)
{
    //Consider the window you expect to see
    unsigned int bounda = m_inseq;
    unsigned int boundb = (fseq+WINDOW_SIZE*m_acceptmod)%SEQUENCE_MODULO;
    unsigned int seq = msg.GetSequenceNumber();
    if(bounda <= seq || (seq < boundb and boundb < bounda))
    {
        m_acceptmod = 1;
        m_inseq = seq;
        return true;
    }
    if(m_acceptmod <= SEQUENCE_MODULO/WINDOW_SIZE)
    {
        // Expand the accept window to try and prevent bad things from happening.
        m_acceptmod *= 2;
    }
    return false;
}

void CSUConnection::SendACK(const CMessage &msg)
{
    unsigned int seq = msg.GetSequenceNumber();
    freedm::broker::CMessage outmsg;
    // Presumably, if we are here, the connection is registered 
    outmsg.SetSourceUUID(GetConnectionManager().GetUUID());
    outmsg.SetSourceHostname(GetConnectionManager().GetHostname());
    outmsg.SetStatus(freedm::broker::CMessage::Accepted);
    outmsg.SetSequenceNumber(sequenceno);
    Write(outmsg);
}

    }
}
