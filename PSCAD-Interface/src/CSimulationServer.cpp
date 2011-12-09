////////////////////////////////////////////////////////////////////////////////
/// @file           CSimulationServer.hpp
///
/// @author         Thomas Roth <tprfh7@mst.edu>
///
/// @compiler       C++
///
/// @project        Missouri S&T Power Research Group
///
/// @see            CSimulationServer.hpp
///
/// These source code files were created at the Missouri University of Science
/// and Technology, and are intended for use in teaching or research. They may
/// be freely copied, modified and redistributed as long as modified versions
/// are clearly marked as such and this notice is not removed.
///
/// Neither the authors nor Missouri S&T make any warranty, express or implied,
/// nor assume any legal responsibility for the accuracy, completeness or
/// usefulness of these files or any information distributed with these files.
///
/// Suggested modifications or questions about these files can be directed to
/// Dr. Bruce McMillin, Department of Computer Science, Missouri University of
/// Science and Technology, Rolla, MO 65401 <ff@mst.edu>.
///
////////////////////////////////////////////////////////////////////////////////

#include "CSimulationServer.hpp"

namespace freedm {
namespace simulation {

CSimulationServer::CSimulationServer( const std::string & p_xml, unsigned short p_port )
    : m_port(p_port), m_command(p_xml,"command"), m_state(p_xml,"state"), m_quit(false)
{
    Logger::Info << __PRETTY_FUNCTION__ << std::endl;
    
    // read the xml input file
    boost::property_tree::ptree xmlTree;
    boost::property_tree::read_xml( p_xml, xmlTree );
    size_t interfaces = xmlTree.get<size_t>("SSTCount");
    std::list<boost::shared_ptr<boost::thread> > thread_pool;

    // start each interface with a unique port / identifier
    for( size_t i = 1; i <= interfaces; i++ )
    {
        thread_pool.push_back(boost::make_shared<boost::thread>(&CSimulationServer::StartDGIProcess, this, p_port+i, i
));

        Logger::Notice << "Initialized DGI-Interface " << i << std::endl;
    }
    
    // start the simulation server
    m_thread = boost::thread( &CSimulationServer::Run, this );
    Logger::Notice << "Running PSCAD Interface" << std::endl;
    
    // start i/o service
    m_service.run();
}

void CSimulationServer::StartDGIProcess(unsigned short p_port,size_t p_index)
{
  boost::asio::io_service dgi_service;
  
  CSimulationInterface::TPointer dgi=CSimulationInterface::Create(dgi_service, m_command, m_state, p_port, p_index);
  dgi_service.run();
}



CSimulationServer::~CSimulationServer()
{
    Logger::Info << __PRETTY_FUNCTION__ << std::endl;
    
    // wait on the worker
    m_thread.join();
}

void CSimulationServer::Stop()
{
    Logger::Info << __PRETTY_FUNCTION__ << std::endl;
    
    m_quit = true;
    m_service.stop();
}

void CSimulationServer::Run()
{
    Logger::Info << __PRETTY_FUNCTION__ << std::endl;
    
    boost::array<char,HEADER_SIZE> header;

    // create an acceptor on the shared I/O service
    boost::asio::ip::tcp::acceptor acceptor( m_service );
    
    // create an endpoint for IPv4 with m_port as a port number
    boost::asio::ip::tcp::endpoint endpoint( boost::asio::ip::tcp::v4(), m_port );
    Logger::Notice << "PSCAD will use port " << m_port << std::endl;
    
    // open the acceptor at the endpoint
    acceptor.open( endpoint.protocol() );
    acceptor.set_option( boost::asio::ip::tcp::acceptor::reuse_address(true) );
    acceptor.bind( endpoint );
    acceptor.listen();

    while( !m_quit )
    {
        // TODO: this blocks - makes m_quit rather pointless
        // create and accept the client connection
        boost::asio::ip::tcp::socket socket( m_service );
        acceptor.accept(socket);
        
        // read the header of the next received packet
        boost::asio::read( socket, boost::asio::buffer(header) );
        Logger::Debug << "PSCAD - received " << header.data() << std::endl;
        
        // message handler based on header type
        if( strcmp( header.data(), "RST" ) == 0 )
        {
            boost::unique_lock<boost::shared_mutex> lockA(m_state.m_mutex);
            boost::unique_lock<boost::shared_mutex> lockB(m_command.m_mutex);
            Logger::Debug << "PSCAD - obtained mutex as writer" << std::endl;
            size_t bytes = m_state.m_length * sizeof(double);
            
            // read the message body into the state table
            boost::asio::read( socket, boost::asio::buffer(m_state.m_data, bytes) );
            
            // copy the message body into the command table
            if( m_state.m_length != m_command.m_length )
            {
                // this implementation requires m_state == m_command
                Logger::Error << "Failed to handle RST message: " << 
                    "state and command are not uniform" << std::endl;
            }
            else
            {
                for( size_t i = 0; i < m_state.m_length; i++ )
                {
                    m_command.m_data[i] = m_state.m_data[i];
                }
            }
            
            Logger::Debug << "PSCAD - released writer mutex" << std::endl;
        }
        else if( strcmp( header.data(), "GET" ) == 0 )
        {
            boost::shared_lock<boost::shared_mutex> lock(m_command.m_mutex);
            Logger::Debug << "PSCAD - obtained mutex as reader" << std::endl;
            size_t bytes = m_command.m_length * sizeof(double);
            
            // write the command table as a response
            boost::asio::write( socket, boost::asio::buffer(m_command.m_data, bytes) );
            Logger::Debug << "PSCAD - released reader mutex" << std::endl;
        }
        else if( strcmp( header.data(), "SET" ) == 0)
        {
            boost::unique_lock<boost::shared_mutex> lock(m_state.m_mutex);
            Logger::Debug << "PSCAD - obtained mutex as writer" << std::endl;
            size_t bytes = m_state.m_length * sizeof(double);
            
            // read the message body into the state table
            boost::asio::read( socket, boost::asio::buffer(m_state.m_data, bytes) );
            Logger::Debug << "PSCAD - released writer mutex" << std::endl;
        }
        else if( strcmp( header.data(), "QUIT" ) == 0 )
        {
            Stop();
        }
        else
        {
            Logger::Warn << "PSCAD - received unhandled message" << std::endl;
        }
        
        socket.close();
    }
}

} // namespace simulation
} // namespace freedm
