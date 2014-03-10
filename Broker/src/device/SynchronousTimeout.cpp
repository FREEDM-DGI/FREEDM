////////////////////////////////////////////////////////////////////////////////
/// @file         SynchronousTimeout.cpp
///
/// @author       Thomas Roth <tprfh7@mst.edu>
///
/// @project      FREEDM DGI
///
/// @description  Synchronous i/o operations with timeouts.
///
/// @functions
///     SetResult
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

#include "SynchronousTimeout.hpp"
#include "CLogger.hpp"

namespace freedm {
namespace broker {
namespace device {

namespace {
/// This file's logger.
CLocalLogger Logger(__FILE__);
}

////////////////////////////////////////////////////////////////////////////////
/// This function is meant to be used in conjunction with asynchronous calls to
/// determine when a call has finished execution. Bind an asynchronous call to
/// this function using an uninitialized boost::optional, and poll the first
/// argument to test for completion.
///
/// @pre The first argument should be uninitialized.
/// @post The first argument will be set to the error code.
/// @param status The optional variable that will store the error code.
/// @param error The error code of an asynchronous call bound to this function.
///
/// @limitations The optional variable will also be set in error cases when the
/// asynchronous call fails. Check the value of the first argument to determine
/// whether an error has occured.
////////////////////////////////////////////////////////////////////////////////
void SetResult(boost::shared_ptr<OptionalError> status,
        const boost::system::error_code & error)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    *status = error;
}

} // namespace device
} // namespace broker
} // namespace freedm

