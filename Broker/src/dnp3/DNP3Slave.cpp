#include "DNP3Slave.hpp"
#include "LogToClog.hpp"
#include "CLogger.hpp"

namespace freedm {
namespace broker {

namespace {
/// This file's logger.
CLocalLogger Logger(__FILE__);
}

DNP3Slave::DNP3Slave()
    : LOG_LEVEL(apl::LEV_INFO)
    , m_logger()
    , m_stack_mgr(m_logger.GetLogger(LOG_LEVEL, "dnp3"))
    , m_observer(NULL)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    using namespace apl;
    using namespace apl::dnp;

    // default values for test slave
    boost::uint16_t remote_dnp3 = 100;
    boost::uint16_t local_dnp3  = 1;
    std::string     local_ip    = "192.168.1.56";
    boost::uint16_t local_port  = 4999;

    // slave configuration objects
    DeviceTemplate format(
        0,  // binary
        13,  // analog
        0,  // counter
        0,  // control status
        0,  // setpoint status
        0,  // control
        0   // setpoint
    );
    SlaveStackConfig config;
    config.device           = format;
    config.link.LocalAddr   = local_dnp3;
    config.link.RemoteAddr  = remote_dnp3;

    m_logger.AddLogSubscriber(LogToClog::Inst());

    // initialize the slave communication stack
    m_stack_mgr.AddTCPServer("tcpserver", PhysLayerSettings(LOG_LEVEL, 3000), local_ip, local_port);
    m_slave = boost::shared_ptr<SlaveDemoApp>(new SlaveDemoApp(m_logger.GetLogger(LOG_LEVEL, "dnp3")));
    m_observer = m_stack_mgr.AddSlave("tcpserver", "Slave", LOG_LEVEL, m_slave->GetCmdAcceptor(), config);
    m_slave->SetDataObserver(m_observer);

    m_thread = boost::thread(boost::bind(&DNP3Slave::RunService, this));
}

DNP3Slave::~DNP3Slave()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    m_observer = NULL;
}

DNP3Slave & DNP3Slave::Instance()
{
    static DNP3Slave instance;
    return instance;
}

void DNP3Slave::RunService()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    m_slave->Run();
}

void DNP3Slave::Update(std::size_t index, double value)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    if( !m_lock )
    {
        Logger.Info << "Locked the DNP3 slave buffer." << std::endl;
        m_lock.reset(new apl::Transaction(m_observer));
    }
    m_observer->Update(apl::Analog(value, apl::AQ_ONLINE), index);
}

void DNP3Slave::Flush()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    m_lock.reset();
    Logger.Info << "Flushed the DNP3 slave buffer." << std::endl;
}

} // namespace broker
} // namespace freedm
