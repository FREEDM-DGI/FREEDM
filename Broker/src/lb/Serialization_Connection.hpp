//
// connection.hpp
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2008 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef SERIALIZATION_CONNECTION_HPP
#define SERIALIZATION_CONNECTION_HPP

#include <boost/asio.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>

#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/tuple/tuple.hpp>

#include <iomanip>
#include <string>
#include <sstream>
#include <vector>

#define MAXBUF 1024
namespace freedm
{

class CResponse;

using namespace boost::asio::ip;
/// The connection class provides serialization primitives on top of a socket.
/**
 * Each message sent using this class consists of:
 * @li An 8-byte header containing the length of the serialized data in
 * hexadecimal.
 * @li The serialized data.
 */
class udp_connection
{
    public:
        /// Constructor.
        udp_connection(boost::asio::io_service& io_service, unsigned short port)
            : socket_(io_service, udp::endpoint(udp::v4(), port ))
        {
            inbound_data_.resize(MAXBUF);
        }
        
        /// Get the underlying socket. Used for making a connection or for accepting
        /// an incoming connection.
        boost::asio::ip::udp::socket& socket()
        {
            return socket_;
        }
        
        /// Asynchronously write a data structure to the socket.
        template <typename T, typename Handler>
        void async_send_to(const T& t, const udp::endpoint &dest, Handler handler)
        {
            // Serialize the data first so we know how large it is.
            std::ostringstream archive_stream;
            boost::archive::xml_oarchive archive(archive_stream);
            archive << BOOST_SERIALIZATION_NVP(t);
            outbound_data_ = archive_stream.str();
            // Format the header.
            std::ostringstream header_stream;
            header_stream << std::setw(header_length)
                          << std::hex << outbound_data_.size();
                          
            if (!header_stream || header_stream.str().size() != header_length)
            {
                // Something went wrong, inform the caller.
                boost::system::error_code error(boost::asio::error::invalid_argument);
                socket_.io_service().post(boost::bind(handler, error));
                return;
            }
            
            outbound_header_ = header_stream.str();
            // Write the serialized data to the socket. We use "gather-write" to send
            // both the header and the data in a single write operation.
            std::vector<boost::asio::const_buffer> buffers;
            buffers.push_back(boost::asio::buffer(outbound_header_));
            buffers.push_back(boost::asio::buffer(outbound_data_));
            socket_.async_send_to(buffers, dest, handler);
        }
        
        /// Asynchronously read a data structure from the socket.
        template <typename T, typename Handler>
        void async_receive_from(T& t, udp::endpoint &sender_endpoint, Handler handler)
        {
            // Issue a read operation to read exactly the number of bytes in a header.
            void (udp_connection::*f)(
                const boost::system::error_code&, udp::endpoint&,
                T&, boost::tuple<Handler>)
                = &udp_connection::handle_read_data<T, Handler>;
            socket_.async_receive_from(
                boost::asio::buffer(inbound_data_),
                sender_endpoint,
                boost::bind(
                    f, this, boost::asio::placeholders::error, sender_endpoint,
                    boost::ref(t), boost::make_tuple(handler)));
        }
        
        /// Handle a completed read of a message header. The handler is passed using
        /// a tuple since boost::bind seems to have trouble binding a function object
        /// created using boost::bind as a parameter.
        template <typename T, typename Handler>
        void handle_read_data(const boost::system::error_code& e,
                              udp::endpoint& sender_endpoint, T& t, boost::tuple<Handler> handler)
        {
            if (e)
            {
                boost::get<0>(handler)(e);
            }
            else
            {
                std::istringstream is(std::string(inbound_data_.begin(),
                                                  inbound_data_.begin() + header_length));
                std::size_t inbound_data_size = 0;
                
                if (!(is >> std::hex >> inbound_data_size))
                {
                    // Header doesn't seem to be valid. Inform the caller.
                    std::clog << "Invalid header." << std::endl;
                    boost::system::error_code error(boost::asio::error::invalid_argument);
                    boost::get<0>(handler)(error);
                    return;
                }
                
                std::string archive_data(inbound_data_.begin() + header_length,
                                         (inbound_data_.end()));
                                         
                // Extract the data structure from the data just received.
                try
                {
                    std::istringstream archive_stream(archive_data);
                    boost::archive::xml_iarchive archive(archive_stream);
                    archive >> BOOST_SERIALIZATION_NVP(t);
                }
                catch (std::exception& e)
                {
                    // Unable to decode data.
                    std::clog << "Unable to decode data" << std::endl;
#if DEBUG > 7
                    std::cerr << "Incoming message" << std::endl
                              << archive_data << std::endl;
#endif
                    boost::system::error_code error(boost::asio::error::invalid_argument);
                    boost::get<0>(handler)(error);
                    return;
                }
                
                // Inform caller that data has been received ok.
                boost::get<0>(handler)(e);
            }
        }
#if 0
        /// Handle a completed read of message data.
        template <typename T, typename Handler>
        void handle_read_data(const boost::system::error_code& e,  std::size_t size,
                              udp::endpoint& sender_endpoint, T& t, boost::tuple<Handler> handler)
        {
            if (e)
            {
                std::cerr << __PRETTY_FUNCTION__ << std::endl << "ERROR" << std::endl;
                boost::get<0>(handler)(e);
            }
            else
            {
                // Extract the data structure from the data just received.
                try
                {
                    std::string archive_data(&inbound_data_[header_length],
                                             (inbound_data_.size() - header_length));
                    std::istringstream archive_stream(archive_data);
                    boost::archive::xml_iarchive archive(archive_stream);
                    archive >> BOOST_SERIALIZATION_NVP(t);
                }
                catch (std::exception& e)
                {
                    // Unable to decode data.
                    boost::system::error_code error(boost::asio::error::invalid_argument);
                    boost::get<0>(handler)(error);
                    return;
                }
                
                // Inform caller that data has been received ok.
                boost::get<0>(handler)(e);
            }
        }
#endif
    private:
        /// The underlying socket.
        boost::asio::ip::udp::socket socket_;
        
        /// The size of a fixed length header.
        enum { header_length = 8 };
        
        /// Holds an outbound header.
        std::string outbound_header_;
        
        /// Holds the outbound data.
        std::string outbound_data_;
        
        /// Holds the inbound data.
        std::vector<char> inbound_data_;
};

typedef boost::shared_ptr<udp_connection> udp_connection_ptr;

}

#endif // SERIALIZATION_CONNECTION_HPP
