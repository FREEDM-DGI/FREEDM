#include "DNP3Slave.hpp"
#include "CLogger.hpp"

#include <string>

#include <APL/Log.h>
#include <APL/LogToFile.h>
#include <boost/bind.hpp>

namespace freedm {
namespace broker {

namespace {
CLocalLogger Logger(__FILE__);
}

DNP3Slave::DNP3Slave()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    using namespace apl;
    using namespace apl::dnp;

    // default values for test slave
    boost::uint16_t remote_dnp3 = 100;
    boost::uint16_t local_dnp3  = 1;
    std::string     local_ip    = "127.0.0.1";
    boost::uint16_t local_port  = 4999;

    // Create a log object for the stack to use and configure it
    // with a subscriber that print alls messages to the stdout
    LogToFile(&log, "dnp3.log");

    // Specify a FilterLevel for the stack/physical layer to use.
    // Log statements with a lower priority will not be logged.
    const FilterLevel LOG_LEVEL = LEV_INFO;

    // create our demo application that handles commands and
    // demonstrates how to publish data give it a loffer with a
    // unique name and log level
    app = boost::shared_ptr<SlaveDemoApp>(new SlaveDemoApp(log.GetLogger(LOG_LEVEL, "demoapp")));

    // This is the main point of interaction with the stack. The
    // AsyncStackManager object instantiates master/slave DNP
    // stacks, as well as their physical layers
    mgr = boost::shared_ptr<AsyncStackManager>(new AsyncStackManager(log.GetLogger(LOG_LEVEL, "dnp")));

    // Add a TCPServer to the manager with the name "tcpserver".
    // The server will wait 3000 ms in between failed bind calls.
    mgr->AddTCPServer(
        "tcpserver",
        PhysLayerSettings(LOG_LEVEL, 3000),
        local_ip,
        local_port
    );

    // The master config object for a slave. The default are
    // useable, but understanding the options are important.
    SlaveStackConfig stackConfig;

    // Override the default link addressing
    stackConfig.link.LocalAddr  = local_dnp3;
    stackConfig.link.RemoteAddr = remote_dnp3;

    // The DeviceTemplate struct specifies the structure of the
    // slave's database, as well as the index range of controls and
    // setpoints it accepts.
    DeviceTemplate device(5, 5, 5, 5, 5, 5, 5);
    stackConfig.device = device;

    // Create a new slave on a previously declared port, with a
    // name, log level, command acceptor, and config info This
    // returns a thread-safe interface used for updating the slave's
    // database.
    pDataObserver = mgr->AddSlave("tcpserver", "slave", LOG_LEVEL, app->GetCmdAcceptor(), stackConfig);

    // Tell the app where to write opdates
    app->SetDataObserver(pDataObserver);

    m_thread = boost::thread(boost::bind(&DNP3Slave::Work, this));
}

DNP3Slave & DNP3Slave::Instance()
{
    static DNP3Slave instance;
    return instance;
}

void DNP3Slave::Work()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    app->Run();
}

} // namespace broker
} // namespace freedm
