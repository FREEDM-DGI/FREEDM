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

CSimulationServer::CSimulationServer( const std::string & p_xml, unsigned short p_port )
    : m_port(p_port), m_command(p_xml,"command"), m_state(p_xml,"state")
{
    // read the xml input file
    boost::property_tree::ptree xmlTree;
    boost::property_tree::read_xml( p_xml, xmlTree );
    size_t interfaces = xmlTree.get<size_t>("SSTCount");

    // start each interface with a unique port / identifier
    for( size_t i = 1; i <= interfaces; i++ )
    {
        m_interface.push_back( CSimulationInterface::Create( m_service, m_state, m_command, (p_port + i), i ) );
    }

    // start the simulation server
    m_thread = boost::thread( &CSimulationServer::Run, this );
}

CSimulationServer::~CSimulationServer()
{
    // wait on the worker
    m_thread.join();
}

void CSimulationServer::Stop()
{
    m_quit = true;
    m_service.stop();
}

void CSimulationServer::Run()
{
    boost::array<char,HEADER_SIZE> header;

    // create an acceptor on the shared I/O service
    boost::asio::ip::tcp::acceptor acceptor( m_service );
    
    // create an endpoint for IPv4 with m_port as a port number
    boost::asio::ip::tcp::endpoint endpoint( boost::asio::ip::tcp::v4(), m_port );
    
    // open the acceptor at the endpoint
    acceptor.open( endpoint.protocol() );
    acceptor.set_option( boost::asio::ip::tcp::acceptor::reuse_address(true) );
    acceptor.bind( endpoint );
    acceptor.listen();

    // start the shared service
    m_service.run();
    m_quit = false;

    while( !m_quit )
    {
        // TODO: this blocks - makes m_quit rather pointless
        // create and accept the client connection
        boost::asio::ip::tcp::socket socket( m_service );
        acceptor.accept(socket);
        
        // read the header of the next received packet
        boost::asio::read( socket, boost::asio::buffer(header) );
        
        // message handler based on header type
        if( strcmp( header.data(), "GET" ) == 0 )
        {
            boost::shared_lock<boost::shared_mutex> lock(m_command.m_mutex);
            size_t bytes = m_command.m_length * sizeof(double);
            
            // write the command table as a response
            boost::asio::write( socket, boost::asio::buffer(m_command.m_data, bytes) );
        }
        else if( strcmp( header.data(), "SET" ) == 0)
        {
            boost::unique_lock<boost::shared_mutex> lock(m_state.m_mutex);
            size_t bytes = m_state.m_length * sizeof(double);
            
            // read the message body into the state table
            boost::asio::read( socket, boost::asio::buffer(m_state.m_data, bytes) );
        }
        else if( strcmp( header.data(), "QUIT" ) == 0 )
        {
            Stop();
        }
        else
        {
            // bad header
        }
        
        socket.close();
    }
}
