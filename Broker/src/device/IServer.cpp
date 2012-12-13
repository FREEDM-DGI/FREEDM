////////////////////////////////////////////////////////////////////////////////
/// @file           IServer.cpp
///
/// @author         Thomas Roth <tprfh7@mst.edu>
///
/// @project        FREEDM DGI
///
/// @description    Defines the interface for a server.
///
/// @functions
///     IServer::~IServer
///     IServer::RegisterHandler
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

#include "IServer.hpp"
#include "CLogger.hpp"

#include <stdexcept>

namespace freedm {
namespace broker {
namespace device {

namespace {
/// This file's logger.
CLocalLogger Logger(__FILE__);
}

////////////////////////////////////////////////////////////////////////////////
/// Virtual destructor for derived classes.
///
/// @pre None.
/// @post Destroys the base server class.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
IServer::~IServer()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
}

////////////////////////////////////////////////////////////////////////////////
/// Registers a client connection handler with the server.
///
/// @ErrorHanlding Throws a std::runtime_error if either m_handler has already
/// been initialized or the passed function is null.
/// @pre m_handler must not be initialized.
/// @post Assigns the passed function to m_handler.
/// @param h The callback function to handle client connections.
///
/// @limitations This function can only be called once.
////////////////////////////////////////////////////////////////////////////////
void IServer::RegisterHandler( ConnectionHandler h )
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    if( !m_handler.empty() )
    {
        throw std::runtime_error(
            "Attempted to override an IServer connection handler.");
    }

    if( h.empty() )
    {
        throw std::runtime_error(
            "IServer received an empty connection handler.");
    }

    m_handler = h;
}

} // namespace device
} // namespace broker
} // namespace freedm
