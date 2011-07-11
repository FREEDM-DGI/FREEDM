////////////////////////////////////////////////////////////////////
/// @file      CConnectionHeader.hpp
///
/// @author    Stephen Jackson <scj7t4@mst.edu>
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
/// Technology, Rolla, /// MO  65409 (ff@mst.edu).
///
////////////////////////////////////////////////////////////////////

#ifndef CCONNECTIONHEADER_HPP
#define CCONNECTIONHEADER_HPP

#include "CMessage.hpp"
#include "CDispatcher.hpp"

#include "concurrentqueue.hpp"

#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#include <iomanip>

namespace freedm {
    namespace broker {

class CConnectionHeader
{
    public:
        static const unsigned int HEADERSIZE = 23;
        static const unsigned int LENGTHFIELDSIZE = 11;
        static const unsigned int SEQUENCEFIELDSIZE = 11;
        static const unsigned int ACKFIELDSIZE = 1;
        CConnectionHeader(const std::string &m, unsigned int sequenceno);
        CConnectionHeader(unsigned int sequenceno);
        CConnectionHeader(const boost::array<char,8192> &buff, unsigned int len);
        CConnectionHeader(const CConnectionHeader &x);
        CConnectionHeader();
        unsigned int GetMessageSize() const;
        unsigned int GetSequenceNumber() const;
        std::string ToString() const;
        bool IsWellFormed() const;
        bool IsAck() const;
    private:
        unsigned int m_datagramsize;
        unsigned int m_sequenceno;
        unsigned int m_ack;
        bool m_malformed;
};


    } // namespace broker
} // namespace freedm

#endif // CCONNECTIONHEADER_HPP
