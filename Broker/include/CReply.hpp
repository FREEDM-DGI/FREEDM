////////////////////////////////////////////////////////////////////
/// @file      CReply.hpp
///
/// @author    Derek Ditch <derek.ditch@mst.edu>
///
/// @compiler  C++
///
/// @project   FREEDM DGI
///
/// @description <FILE DESCRIPTION> 
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

#include "CMessage.hpp"
#include "logger.hpp"
CREATE_EXTERN_STD_LOGS()

namespace freedm {
    namespace broker {

class CReply : public CMessage
{
public:
    // Status codes are modeled after HTTP/1.0 will add/remove as necessary.
    /// The status of the reply.
    enum StatusType
    {
        OK = 200,
        Created = 201,
        Accepted = 202,
        NoContent = 204,
        MultipleChoices = 300,
        MovedPermanently = 301,
        MovedTemporarily = 302,
        NotModified = 304,
        BadRequest = 400,
        Unauthorized = 401,
        Forbidden = 403,
        NotFound = 404,
        InternalServerError = 500,
        NotImplemented = 501,
        BadGateway = 502,
        ServiceUnavailable = 503
    };

private:
    StatusType     m_status;

public:
    virtual bool Load( std::istream &p_is )
        throw ( boost::property_tree::file_parser_error );
    virtual void Save( std::ostream &p_os );

    static CReply StockReply( StatusType p_status )
    {
        Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
        CReply reply_;
        reply_.m_status = p_status;

        return reply_;
    }

    // Casting constructor
    virtual operator ptree ();
    explicit CReply( const ptree &pt );

};


    } // namespace broker
} // namespace freedm
