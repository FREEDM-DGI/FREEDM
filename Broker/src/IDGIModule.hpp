////////////////////////////////////////////////////////////////////////////////
/// @file         IDGIModule.hpp
///
/// @author       Michael Catanzaro <michael.catanzaro@mst.edu>
///
/// @project      FREEDM DGI
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

#ifndef IDGIMODULE_HPP
#define IDGIMODULE_HPP

#include "CPeerNode.hpp"

#include "messages/ModuleMessage.pb.h"

#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

namespace freedm {

namespace broker {

///An interface for an object which can handle recieving incoming messages
class IDGIModule
    : private boost::noncopyable
{
///////////////////////////////////////////////////////////////////////////////
/// @class IDGIModule
///
/// @description Provides common interfaces and boilerplate for DGI modules
///////////////////////////////////////////////////////////////////////////////
public:
    /// Constructor, initializes the reference to self
    IDGIModule();
    
    /// Virtual destructor for inhertiance
    virtual ~IDGIModule() {};

    /// Handles received messages
    virtual void HandleIncomingMessage(
        boost::shared_ptr<const ModuleMessage> msg, CPeerNode peer) = 0;

protected:
    /// Gets the UUID of this process.
    std::string GetUUID() const;
    
    /// Gets a CPeerNode representing this process.
    CPeerNode GetMe();

private: 
    /// The CPeerNode this represents.
    CPeerNode m_me;
};

} // namespace freedm

} // namespace broker

#endif // IHANDLER_HPP
