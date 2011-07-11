// TODO: check packet for correct data size
// TODO: handle loss of connection / DGI failure
// TODO: figure out what QUIT should do (if anything)
// TODO: SetValue should probably include an event time
// TODO: expand CSimulationThread state and GetValue

#include "CSimulationServer.hpp"

static double g_SimulationTime;
static CEventTable g_EventTable;
static boost::mutex g_StateMutex;

CSimulationServer::CSimulationServer( size_t p_devices, unsigned short p_port )
    : m_DeviceCount(p_devices), m_SimulationPort(p_port), m_Terminate(false)
{
    try
    {
        // last packet received from simulation
        m_StateSize = PSCAD_PACKET_SIZE;
        m_LatestState = new double[m_StateSize];
        
        // last packet sent to simulation
        m_EventSize = m_DeviceCount;
        m_LatestEvent = new double[m_EventSize];
        
        // create individual threads for each DGI interface
        m_DeviceInterface = new CSimulationThread[m_DeviceCount];
        
        // create individual thread for the PSCAD interface
        m_SimulationInterface = boost::thread( &CSimulationServer::Run, this );
    }
    catch(...)
    {
        delete [] m_DeviceInterface;
        delete [] m_LatestState;
        delete [] m_LatestEvent;
    }
}

CSimulationServer::~CSimulationServer()
{
    // wait for PSCAD interface
    m_SimulationInterface.join();
    
    // wait for DGI interfaces
    for( size_t i = 0; i < m_DeviceCount; i++ )
    {
        m_DeviceInterface[i].Join();
    }

    delete [] m_DeviceInterface;
    delete [] m_LatestState;
    delete [] m_LatestEvent;
}

void CSimulationServer::Stop()
{
    // set terminate flag
    m_Terminate = true;
}

void CSimulationServer::Run()
{
    char buffer[BUFFER_SIZE];
    char header[HEADER_SIZE];
    int client = -1, bytes;
    std::string port;

    // create PSCAD interface
    port = boost::lexical_cast<std::string>(m_SimulationPort);
    m_Socket = TCPSocket::Create(port);
    
    // create DGI interfaces
    for( size_t i = 0; i < m_DeviceCount; i++ )
    {
        m_DeviceInterface[i].Start(i,m_SimulationPort+i+1);
    }

    // main simserv loop
    while( !m_Terminate )
    {
        while( client == -1 && !m_Terminate )
        {
            // wait for connection from simulation
            client = TCPSocket::Accept( m_Socket, m_ClientAddress );
        }
        
        if( !m_Terminate )
        {
            // read simulation packet into temporary buffer
            bytes = TCPSocket::Read( client, buffer, BUFFER_SIZE );
            
            // extract packet header
            memcpy( header, buffer, HEADER_SIZE );
            
            // message handler
            if( strcmp( header, "RST" ) == 0 )
            {
                // clear local state and event arrays
                memset( m_LatestState, 0, m_StateSize*sizeof(double) );
                memset( m_LatestEvent, 0, m_EventSize*sizeof(double) );
                
                // clear global state
                UpdateState();
                
                // clear event table
                g_EventTable.Clear();
            }
            else if( strcmp( header, "GET" ) == 0 )
            {
                // update and dispatch event
                UpdateEvent();
                bytes = m_EventSize * sizeof(double);
                TCPSocket::Write( client, m_LatestEvent, bytes );
            }
            else if( strcmp( header, "SET" ) == 0 )
            {
                // store and update state
                bytes = m_StateSize * sizeof(double);
                memcpy( m_LatestState, (buffer + HEADER_SIZE), bytes );
                UpdateState();
            }
            else if( strcmp( header, "QUIT" ) == 0 )
            {
                // unimplemented
            }

            close(client);
            client = -1;
        }
    }
    
    // stop DGI interfaces
    for( size_t i = 0; i < m_DeviceCount; i++ )
    {
        m_DeviceInterface[i].Stop();
    }
}

void CSimulationServer::UpdateState()
{
    boost::mutex::scoped_lock lock(g_StateMutex);
    std::string gateway;
    
    // set simulation time
    g_SimulationTime = m_LatestState[0];
    
    // update DGI interfaces
    for( size_t i = 0; i < m_DeviceCount; i++ )
    {
        gateway = boost::lexical_cast<std::string>(m_LatestState[i+1]);
        m_DeviceInterface[i].SetValue( "gateway", gateway );
    }
}

void CSimulationServer::UpdateEvent()
{
    // repopulate m_LatestEvent with most recent data
    g_EventTable.Update( g_SimulationTime, m_LatestEvent, m_EventSize );
}

void CSimulationThread::Start( size_t p_device, unsigned short p_port )
{
    m_DeviceIndex = p_device;
    m_ListenPort = p_port;
    
    // start interface on new thread
    m_Thread = boost::thread( &CSimulationThread::Run, this );
}

void CSimulationThread::Join()
{
    // wait for termination
    m_Thread.join();
}

void CSimulationThread::Run()
{
    // start extensible service with GetValue and SetValue functions
    freedm::CExtensibleService my_service( m_Service, m_ListenPort,
        boost::bind(&CSimulationThread::GetValue, boost::ref(*this), _1),
        boost::bind(&CSimulationThread::SetValue, boost::ref(*this), _1, _2) );

    m_Service.run();
}

void CSimulationThread::Stop()
{
    // cause termination
    m_Service.stop();
}

const std::string & CSimulationThread::GetValue( const std::string & p_key )
{
    boost::mutex::scoped_lock lock(g_StateMutex);
    
    if( p_key == "gateway" )
    {
        return m_Value;
    }
    else
    {
        throw std::logic_error("received unknown key value");
    }
}

void CSimulationThread::SetValue( const std::string & p_key,
        const std::string & p_value )
{
    if( p_key == "pstar" )
    {
        double value = boost::lexical_cast<double>(p_value);
        g_EventTable.Insert( g_SimulationTime, m_DeviceIndex, value );
    }
    else if( p_key == "gateway" )
    {
        m_Value = p_value;
    }
    else
    {
        throw std::logic_error("received unknown key value");
    }
}
