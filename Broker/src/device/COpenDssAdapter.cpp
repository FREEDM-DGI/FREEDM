////////////////////////////////////////////////////////////////////////////////
/// @file         COpenDssAdapter.cpp
///
/// @author       Yaxi Liu <ylztf@mst.edu>
/// @author       Thomas Roth <tprfh7@mst.edu>
/// @author       Mark Stanovich <stanovic@cs.fsu.edu>
/// @author       Michael Catanzaro <michael.catanzaro@mst.edu>
/// @author       Manish jaisinghani <mjkhf@mst.edu>
///
/// @project      FREEDM DGI
///
/// @description  Opendss simulation adapter.
///
/// @functions    EndianSwap
///               EndianSwapIfNeeded
///               COpenDssAdapter::Create
///               COpenDssAdapter::Start
///               COpenDssAdapter::~COpenDssAdapter
///               COpenDssAdapter::Run
///               COpenDssAdapter::COpenDssAdapter
///               COpenDssAdapter::Quit
///               COpenDssAdapter::Connect
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

#include "COpenDssAdapter.hpp"
#include "CLogger.hpp"
#include "CTimings.hpp"
#include "SynchronousTimeout.hpp"

#include <sys/param.h>

#include <cmath>
#include <vector>
#include <cstring>
#include <csignal>
#include <stdexcept>
#include <algorithm>

#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/foreach.hpp>
#include <boost/static_assert.hpp>
#include <boost/property_tree/ptree.hpp>

namespace freedm {
    namespace broker {
        namespace device {

// FPGA is expecting 4-byte floats.
            BOOST_STATIC_ASSERT(sizeof(SignalValue) == 4);

            namespace {

/// This file's logger.
                CLocalLogger Logger(__FILE__);

            } // unnamed namespace

///////////////////////////////////////////////////////////////////////////////
/// Creates an Opendss client on the given io_service.
///
/// @Shared_Memory Uses the passed io_service
///
/// @pre None.
/// @post COpenDssAdapter object is returned for use.
///
///
/// @return Shared pointer to the new COpenDssAdapter object.
///
/// @limitations None
///////////////////////////////////////////////////////////////////////////////
            IAdapter::Pointer COpenDssAdapter::Create(boost::asio::io_service & service,
                                                      const boost::property_tree::ptree & ptree)
            {
                Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
                return COpenDssAdapter::Pointer(new COpenDssAdapter(service, ptree));
            }

////////////////////////////////////////////////////////////////////////////////
/// Constructs an Opendss client.
///
/// @Shared_Memory Uses the passed io_service.
///
/// @pre None.
/// @post COpenDssAdapter created.
///
///
/// @limitations None
////////////////////////////////////////////////////////////////////////////////
            COpenDssAdapter::COpenDssAdapter(boost::asio::io_service & io_service,
                                             const boost::property_tree::ptree & ptree)
                    : m_runTimer(io_service)
                    , m_socket(io_service)
                    , m_host(ptree.get<std::string>("host"))
                    , m_port(ptree.get<std::string>("port"))
            {
                Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
            }

////////////////////////////////////////////////////////////////////////////////
/// Starts sending and receiving data with the adapter.
///
/// @pre The adapter has not yet been started.
/// @post COpenDssAdapter::Run is called to start the adapter.
///
/// @limitations All devices must be added to the adapter before this call.
////////////////////////////////////////////////////////////////////////////////
            void COpenDssAdapter::Start()
            {
                Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

                IBufferAdapter::Start();
                Connect();
                m_runTimer.expires_from_now(
                        boost::posix_time::milliseconds(CTimings::Get("DEV_RTDS_DELAY")));
                m_runTimer.async_wait(boost::bind(&COpenDssAdapter::Run, shared_from_this(),
                                                  boost::asio::placeholders::error));
            }

////////////////////////////////////////////////////////////////////////////////
/// This is the main communication engine.
///
/// @IO Opendss csv data
///
/// @Error_Handling
///     Throws std::runtime_error if reading from or writing to socket fails.
///
/// @pre Connection with FPGA is established.
///
/// @post All values in the receive buffer are sent to the FPGA.  All values in
/// the send buffer are updated with data from the FPGA.
///
/// @limitations This function uses synchronous communication.
////////////////////////////////////////////////////////////////////////////////
            void COpenDssAdapter::Run(const boost::system::error_code & e)
            {
                Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

                if( e )
                {
                    if (e == boost::asio::error::operation_aborted)
                    {
                        return;
                    }
                    else
                    {
                        Logger.Fatal << "Run called with error: " << e.message()
                                     << std::endl;
                        throw boost::system::system_error(e);
                    }
                }
                bzero(buffer,BUFFER_SIZE-1);
                if(!(read(sd,buffer, BUFFER_SIZE-1))){
                    Logger.Status<<"Error reading socket!?"<<std::endl;
                }
                Logger.Status << "opendss data: " << buffer << std::endl;
                std::string command = "Bus : 1,Node1 : 2,Basekv : 88.88,Magnitude1 : 8088.8,Angle1 : 88.8, pu1 : 1.088"; // generic command should be changed
                sendCommand(sd,command);    //test sendop
                Logger.Status<<"command sent to openDss device"<<std::endl;
            }

////////////////////////////////////////////////////////////////////////////
/// Stops the adapter. Thread-safe.
///
/// @pre None.
/// @post Adapter is stopped and can be freed
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////
            void COpenDssAdapter::Stop()
            {
                Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

                try
                {
                    m_runTimer.cancel();
                }
                catch( boost::system::system_error& e)
                {
                    Logger.Error << "Error cancelling timer: " << e.what() << std::endl;
                }

                if( m_socket.is_open() )
                {
                    m_socket.close();
                }
            }
////////////////////////////////////////////////////////////////////////////
/// Sends a command to an openDss device
///
/// @pre connection with a device
/// @post command sent to an openDss device
///
/// @limitations Still under construction.
////////////////////////////////////////////////////////////////////////////
            void COpenDssAdapter::sendCommand(int sd,std::string command)
            {

                strcpy(buffer,command.c_str());
                n = write(sd,buffer,command.size());

                if(!n) {
                    Logger.Error<<"Error writing to socket"<<std::endl;
                }
            }
////////////////////////////////////////////////////////////////////////////
/// Closes the socket before destroying an object instance.
///
/// @pre None.
/// @post Closes m_socket.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////
            COpenDssAdapter::~COpenDssAdapter()
            {
                Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
            }

////////////////////////////////////////////////////////////////////////////////
/// A utility function for converting byte order from big endian to little
/// endian and vice versa. This needs to be called on a SINGLE WORD of the data
/// since it actually just reverses the bytes.
///
/// @pre None
/// @post The bytes in the buffer are now reversed
/// @param buffer the data to be reversed
/// @param numBytes the number of bytes in the buffer
////////////////////////////////////////////////////////////////////////////////
            void COpenDssAdapter::ReverseBytes( char * buffer, const int numBytes )
            {
                Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

                for( std::size_t i = 0, j = numBytes-1; i < j; i++, j-- )
                {
                    char temp = buffer[i];
                    buffer[i] = buffer[j];
                    buffer[j] = temp;
                }
            }

///////////////////////////////////////////////////////////////////////////////
/// Converts the SignalValues in the passed vector from big-endian to
/// little-endian, or vice-versa, if the DGI is running on a little-endian
/// system.
///
/// @pre None.
/// @post The elements of data are converted in endianness if the DGI is
///  running on a little-endian system.  Otherwise, nothing happens.
/// @param v The vector of SignalValues to be endian-swapped.
///
/// @limitations Assumes the existence of UNIX byte order macros.
///////////////////////////////////////////////////////////////////////////////
            void COpenDssAdapter::EndianSwapIfNeeded(std::vector<SignalValue> & v)
            {
                Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

// check endianess at compile time.  Middle-Endian not allowed
// The parameters __BYTE_ORDER, __LITTLE_ENDIAN, __BIG_ENDIAN should
// automatically be defined and determined in sys/param.h, which exists
// in most Unix systems.
#if __BYTE_ORDER == __LITTLE_ENDIAN
                for( std::size_t i = 0; i < v.size(); i++ )
                {
                    ReverseBytes((char*)&v[i], sizeof(SignalValue));
                }

#elif __BYTE_ORDER == __BIG_ENDIAN
                Logger.Debug << "Endian swap skipped: host is big-endian." << std::endl;
#else
#error "unsupported endianness or __BYTE_ORDER not defined"
#endif
            }

////////////////////////////////////////////////////////////////////////////////
/// Creates a TCP socket connection to the adapter's target host and port.
///
/// @ErrorHandling Throws a std::runtime_error for connection errors.
/// @pre hostname and service specify a valid endpoint.
/// @post m_socket is connected to the passed service.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
            void COpenDssAdapter::Connect()
            {
                Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

                boost::asio::ip::tcp::resolver resolver(m_socket.get_io_service());
                boost::asio::ip::tcp::resolver::query query(m_host, m_port);
                boost::asio::ip::tcp::resolver::iterator it = resolver.resolve(query);
                boost::asio::ip::tcp::resolver::iterator end;

                // attempt to connect to one of the resolved endpoints
                boost::system::error_code error = boost::asio::error::host_not_found;

                while( error && it != end )
                {
                    m_socket.close();
                    m_socket.connect(*it, error);
                    ++it;
                }

                if( error )
                {
                    throw std::runtime_error("Failed to connect to " + m_host + ":"
                                             + m_port + " because: "
                                             + std::string(boost::system::system_error(error).what()));
                }

                Logger.Status << "Opened a TCP socket connection to host " << m_host
                              << ":" << m_port << "." << std::endl;
            }

        }//namespace broker
    }//namespace freedm
}//namespace device
