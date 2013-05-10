////////////////////////////////////////////////////////////////////////////////
/// @file           DNP3Slave.hpp
///
/// @author         Thomas Roth <tprfh7@mst.edu>
///
/// @project        FREEDM DGI
///
/// @description    Singleton that contains the DNP3 communication stack.
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

#ifndef DNP3_SLAVE_HPP
#define DNP3_SLAVE_HPP

#include "SlaveDemo.h"

#include <APL/Log.h>
#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <DNP3/AsyncStackManager.h>

namespace freedm {
namespace broker {

////////////////////////////////////////////////////////////////////////////////
/// @limitations The DNP3 has its own logger class which is not compatible with
/// the FREEDM-DGI. 
////////////////////////////////////////////////////////////////////////////////
class DNP3Slave
{
public:
    /// Gets the singleton instance.
    static DNP3Slave & Instance();

    /// Safely destroys the instance.
    ~DNP3Slave();

    /// Sets the value of an element in the data buffer.
    void Update(std::size_t index, double value);

    /// Flushes the buffered data to the DNP3 stack.
    void Flush();
private:
    /// Initializes the DNP3 communication stack.
    DNP3Slave();

    /// Runs the dedicated DNP3 I/O service.
    void RunService();

    /// Set the amount of DNP3 output.
    const apl::FilterLevel LOG_LEVEL;

    /// DNP3 output log.
    apl::EventLog m_logger;

    /// Main DNP3 interface.
    apl::dnp::AsyncStackManager m_stack_mgr;

    /// DNP3 data interface tied to the stack manager lifeline.
    apl::IDataObserver * m_observer;

    /// Slave DNP3 application.
    boost::shared_ptr<apl::dnp::SlaveDemoApp> m_slave;

    /// Lock for the DNP3 application.
    boost::shared_ptr<apl::Transaction> m_lock;

    /// Thread for the DNP3 I/O service.
    boost::thread m_thread;
};

} // namespace broker
} // namespace freedm

#endif // DNP3_SLAVE_HPP

