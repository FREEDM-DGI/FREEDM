////////////////////////////////////////////////////////////////////////////////
/// @file         CProtocolSU.cpp
///
/// @author       Derek Ditch <derek.ditch@mst.edu>
/// @author       Stephen Jackson <scj7t4@mst.edu>
///
/// @project      FREEDM DGI
///
/// @description  Declare CProtocolSU class
///
/// These source code files were created at Missouri University of Science and
/// Technology, and are intended for use in teaching or research. They may be
/// freely copied, modified, and redistributed as long as modified versions are
/// clearly marked as such and shared_from_this() notice is not removed. Neither the authors
/// nor Missouri S&T make any warranty, express or implied, nor assume any legal
/// responsibility for the accuracy, completeness, or usefulness of these files
/// or any information distributed with these files.
///
/// Suggested modifications or questions about these files can be directed to
/// Dr. Bruce McMillin, Department of Computer Science, Missouri University of
/// Science and Technology, Rolla, MO 65409 <ff@mst.edu>.
////////////////////////////////////////////////////////////////////////////////

#include "CProtocolSU.hpp"

#include "CLogger.hpp"
#include "CTimings.hpp"
#include "CBroker.hpp"
#include "messages/ModuleMessage.pb.h"
#include "messages/ProtocolMessage.pb.h"

#include <boost/asio.hpp>
#include <boost/bind.hpp>

namespace freedm {
    namespace broker {

namespace {

/// This file's logger.
CLocalLogger Logger(__FILE__);

}

CProtocolSU::CProtocolSU(std::string uuid,boost::asio::ip::udp::endpoint endpoint)
    : IProtocol(uuid,endpoint),
      m_timeout(CBroker::Instance().GetIOService())
{
    m_outseq = 0;
    m_inseq = 0;
    m_acceptmod = SEQUENCE_MODULO/WINDOW_SIZE;
}

void CProtocolSU::Send(const ModuleMessage& msg)
{
    ProtocolMessage pm;
    pm.mutable_module_message()->CopyFrom(msg);

    pm.set_sequence_num(m_outseq);
    m_outseq = (m_outseq+1) % SEQUENCE_MODULO;

    QueueItem q;

    q.ret = MAX_RETRIES;
    q.msg = pm;

    m_window.push_back(q);

    if(m_window.size() < WINDOW_SIZE)
    {
        Write(pm);
        m_timeout.cancel();
        m_timeout.expires_from_now(boost::posix_time::milliseconds(CTimings::CSUC_RESEND_TIME));
        m_timeout.async_wait(boost::bind(&CProtocolSU::Resend,
            boost::static_pointer_cast<CProtocolSU>(shared_from_this()),
            boost::asio::placeholders::error));
    }
}

void CProtocolSU::Resend(const boost::system::error_code& err)
{
    if(!err)
    {
        int ws = m_window.size();
        int writes = 0;
        for(int i=0; i < ws; i++)
        {
            QueueItem f = m_window.front();
            m_window.pop_front();
            if(f.ret > 0 && writes < static_cast<int>(WINDOW_SIZE))
            {
                Write(f.msg);
                writes++;
                f.ret--;
            }
            if(f.ret > 0)
            {
                 m_window.push_back(f);
            }
            else
            {
                Logger.Notice<<"Gave Up Sending (No Retries) "<<f.msg.DebugString();
            }
        }
        if(m_window.size() > 0)
        {
            m_timeout.cancel();
            m_timeout.expires_from_now(boost::posix_time::milliseconds(CTimings::CSUC_RESEND_TIME));
            m_timeout.async_wait(boost::bind(&CProtocolSU::Resend,
                boost::static_pointer_cast<CProtocolSU>(shared_from_this()),
                boost::asio::placeholders::error));
        }
    }
}

void CProtocolSU::ReceiveACK(const ProtocolMessage& msg)
{
    unsigned int seq = msg.sequence_num();
    while(m_window.size() > 0)
    {
        unsigned int fseq = m_window.front().msg.sequence_num();
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
        boost::system::error_code x;
        Resend(x);
    }
}

bool CProtocolSU::Receive(const ProtocolMessage& msg)
{
    //Consider the window you expect to see
    unsigned int bounda = m_inseq;
    unsigned int boundb = (m_inseq+WINDOW_SIZE*m_acceptmod)%SEQUENCE_MODULO;
    unsigned int seq = msg.sequence_num();
    if(bounda <= seq || (seq < boundb and boundb < bounda))
    {
        m_acceptmod = 1;
        m_inseq = seq+1;
        return true;
    }
    if(m_acceptmod <= SEQUENCE_MODULO/WINDOW_SIZE)
    {
        // Expand the accept window to try and prevent bad things from happening.
        m_acceptmod *= 2;
    }
    return false;
}

void CProtocolSU::SendACK(const ProtocolMessage& msg)
{
    ProtocolMessage outmsg;
    // Presumably, if we are here, the connection is registered
    outmsg.set_status(ProtocolMessage::ACCEPTED);
    outmsg.set_sequence_num(msg.sequence_num());
    Write(outmsg);
}

    }
}
