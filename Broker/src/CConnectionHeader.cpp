////////////////////////////////////////////////////////////////////
/// @file      CConnection Header.cpp
///
/// @author    Derek Ditch <derek.ditch@mst.edu>
///            Christopher M. Kohlhoff <chris@kohlhoff.com> (Boost Example)
///            Stephen Jackson <scj7t4@mst.edu>
///
/// @compiler  C++
///
/// @project   FREEDM DGI
///
/// @description Class for generating message headers 
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

#include "CDispatcher.hpp"
#include "CConnectionHeader.hpp"
#include "CConnection.hpp"
#include "CConnectionManager.hpp"
#include "CMessage.hpp"
#include "RequestParser.hpp"
#include "logger.hpp"


#include <vector>

#include <boost/bind.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/algorithm/string.hpp>

using boost::property_tree::ptree;

namespace freedm {
    namespace broker {

CConnectionHeader::CConnectionHeader(const std::string& m, unsigned int sequenceno)
{
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
    m_datagramsize = m.length();
    m_sequenceno = sequenceno;
    m_malformed = false;
    m_ack = 0;
}

CConnectionHeader::CConnectionHeader(unsigned int sequenceno)
{   
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
    m_ack = 1;
    m_datagramsize = 0;
    m_sequenceno = sequenceno;
    m_malformed = false;
}

CConnectionHeader::CConnectionHeader()
{
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
    m_ack = 0;
    m_datagramsize = 0;
    m_sequenceno = 0;
    m_malformed = false;
}

CConnectionHeader::CConnectionHeader(const boost::array<char,8192> &buff, unsigned int len)
{
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
    if(len < HEADERSIZE)
        m_malformed = true;
    else
        m_malformed = false;
    int offset = 0;
    std::string bufferstring(buff.data(),0,len);
    Logger::Notice << "Loaded string okay" << std::endl;
    std::string size = bufferstring.substr(0,LENGTHFIELDSIZE);
    offset += LENGTHFIELDSIZE;
    std::string sequence = bufferstring.substr(offset,SEQUENCEFIELDSIZE);
    offset += SEQUENCEFIELDSIZE;
    std::string ack = bufferstring.substr(offset,ACKFIELDSIZE); 
    offset += ACKFIELDSIZE;
    m_datagramsize = boost::lexical_cast<unsigned int>(boost::trim_copy(size));
    m_sequenceno = boost::lexical_cast<unsigned int>(boost::trim_copy(sequence));
    m_ack = boost::lexical_cast<unsigned int>(boost::trim_copy(ack));
    Logger::Notice << "Finished parsing header" << std::endl;
}

CConnectionHeader::CConnectionHeader(const CConnectionHeader &x)
{
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
    m_datagramsize = x.m_datagramsize;
    m_sequenceno = x.m_sequenceno;
    m_malformed = x.m_malformed;
    m_ack = x.m_ack;
}

unsigned int CConnectionHeader::GetMessageSize() const
{
    return m_datagramsize;
}

unsigned int CConnectionHeader::GetSequenceNumber() const
{
    return m_sequenceno;
}

bool CConnectionHeader::IsWellFormed() const
{
    return !m_malformed;
}

bool CConnectionHeader::IsAck() const
{
    return (m_ack?true:false);
}

std::string CConnectionHeader::ToString() const
{
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
    std::stringstream ss;
    ss << std::setw(LENGTHFIELDSIZE);
    ss << m_datagramsize;
    ss << std::setw(SEQUENCEFIELDSIZE);
    ss << m_sequenceno;
    ss << std::setw(ACKFIELDSIZE);
    ss << (m_ack?1:0);
    return ss.str();
}

    } // namespace broker
} // namespace freedm
