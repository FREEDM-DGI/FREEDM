#ifndef C_SIMULATION_SERVER_HPP
#define C_SIMULATION_SERVER_HPP

#include <string>
#include <stdexcept>

#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>

#include "TCPSocket.hpp"
#include "CEventTable.hpp"
#include "ExtensibleLineProtocol.hpp"

const int PSCAD_PACKET_SIZE = 4;
const int BUFFER_SIZE = 128;
const int HEADER_SIZE = 5;

class CSimulationThread
{
public:
    void Start( size_t p_device, unsigned short p_port );
    const std::string & GetValue( const std::string & p_key );
    void SetValue( const std::string & p_key, const std::string & p_value );
    void Stop();
    void Join();
private:
    void Run();
    
    boost::asio::io_service m_Service;
    boost::thread m_Thread;

    unsigned short m_ListenPort;
    size_t m_DeviceIndex;
    std::string m_Value;
};

class CSimulationServer
{
public:
    CSimulationServer( size_t p_devices, unsigned short p_port );
    ~CSimulationServer();
    void Stop();
private:
    void Run();
    void UpdateState();
    void UpdateEvent();
    
    CSimulationThread * m_DeviceInterface;
    boost::thread m_SimulationInterface;
    double * m_LatestState;
    double * m_LatestEvent;
    size_t m_DeviceCount;
    int m_StateSize;
    int m_EventSize;
    int m_Socket;
    
    unsigned short m_SimulationPort;
    std::string m_ClientAddress;

    bool m_Terminate;
};

#endif // C_SIMULATION_SERVER_HPP
