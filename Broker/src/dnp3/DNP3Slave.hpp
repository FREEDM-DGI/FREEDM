#ifndef DNP3_SLAVE_HPP
#define DNP3_SLAVE_HPP

#include "SlaveDemo.h"

#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>

namespace freedm {
namespace broker {

class DNP3Slave
{
public:
    static DNP3Slave & Instance();
private:
    DNP3Slave();

    void Work();

    apl::EventLog log;
    boost::shared_ptr<apl::dnp::AsyncStackManager> mgr;
    apl::IDataObserver * pDataObserver;
    boost::shared_ptr<apl::dnp::SlaveDemoApp> app;
    boost::thread m_thread;
};

} // namespace broker
} // namespace freedm

#endif // DNP3_SLAVE_HPP

