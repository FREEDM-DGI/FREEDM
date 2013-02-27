////////////////////////////////////////////////////////////////////////////////
/// @file         SynchronousTimeout.hpp
///
/// @author       Thomas Roth <tprfh7@mst.edu>
///
/// @project      FREEDM DGI
///
/// @description  Synchronous i/o operations with timeouts.
///
/// @functions
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

#ifndef SYNCHRONOUS_TIMEOUT_HPP
#define SYNCHRONOUS_TIMEOUT_HPP

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/optional.hpp>

#include <stdexcept>

namespace freedm {
namespace broker {
namespace device {

void SetResult(boost::optional<int> * status, int value)
{
    status->reset(value);
}

template <typename ReadStream, typename BufferSequence>
void TimedRead(ReadStream & s, const BufferSequence & b, int duration)
{
    boost::optional<int> result, timeout;
    boost::asio::deadline_timer timer(s.get_io_service());

    timer.expires_from_now(boost::posix_time::milliseconds(duration));
    timer.async_wait(boost::bind(SetResult, &timeout, 1));
    boost::asio::async_read(s, b, boost::bind(SetResult, &result, 1));

    while( !result && !timeout );

    if( timeout )
    {
        throw std::runtime_error("Synchronous Read Timeout");
    }    
}

template <typename ReadStream, typename BufferSequence>
void TimedReadUntil(ReadStream & s, const BufferSequence & b,
        const std::string & delim, int duration)
{
    boost::optional<int> result, timeout;
    boost::asio::deadline_timer timer(s.get_io_service());

    timer.expires_from_now(boost::posix_time::milliseconds(duration));
    timer.async_wait(boost::bind(SetResult, &timeout, 1));
    boost::asio::async_read_until(s, b, delim, boost::bind(SetResult, &result, 1));

    while( !result && !timeout );

    if( timeout )
    {
        throw std::runtime_error("Synchronous Read Until Timeout");
    }
}

template <typename WriteStream, typename BufferSequence>
void TimedWrite(WriteStream & s, const BufferSequence & b, int duration)
{
    boost::optional<int> result, timeout;
    boost::asio::deadline_timer timer(s.get_io_service());

    timer.expires_from_now(boost::posix_time::milliseconds(duration));
    timer.async_wait(boost::bind(SetResult, &timeout, 1));
    boost::asio::async_write(s, b, boost::bind(SetResult, &result, 1));

    while( !result && !timeout );

    if( timeout )
    {
        throw std::runtime_error("Synchronous Write Timeout");
    }
}

template <typename ReadStream, typename Allocator>
void TimedRead(ReadStream & s, boost::asio::basic_streambuf<Allocator> & b, int duration)
{
    boost::optional<int> result, timeout;
    boost::asio::deadline_timer timer(s.get_io_service());

    timer.expires_from_now(boost::posix_time::milliseconds(duration));
    timer.async_wait(boost::bind(SetResult, &timeout, 1));
    boost::asio::async_read(s, b, boost::bind(SetResult, &result, 1));

    while( !result && !timeout );

    if( timeout )
    {
        throw std::runtime_error("Synchronous Read Timeout");
    }    
}

template <typename ReadStream, typename Allocator>
void TimedReadUntil(ReadStream & s, boost::asio::basic_streambuf<Allocator> & b,
        const std::string & delim, int duration)
{
    boost::optional<int> result, timeout;
    boost::asio::deadline_timer timer(s.get_io_service());

    timer.expires_from_now(boost::posix_time::milliseconds(duration));
    timer.async_wait(boost::bind(SetResult, &timeout, 1));
    boost::asio::async_read_until(s, b, delim, boost::bind(SetResult, &result, 1));

    while( !result && !timeout );

    if( timeout )
    {
        throw std::runtime_error("Synchronous Read Until Timeout");
    }
}

template <typename WriteStream, typename Allocator>
void TimedWrite(WriteStream & s, boost::asio::basic_streambuf<Allocator> & b, int duration)
{
    boost::optional<int> result, timeout;
    boost::asio::deadline_timer timer(s.get_io_service());

    timer.expires_from_now(boost::posix_time::milliseconds(duration));
    timer.async_wait(boost::bind(SetResult, &timeout, 1));
    boost::asio::async_write(s, b, boost::bind(SetResult, &result, 1));

    while( !result && !timeout );

    if( timeout )
    {
        throw std::runtime_error("Synchronous Write Timeout");
    }
}

} // namespace device
} // namespace broker
} // namespace freedm

#endif // SYNCHRONOUS_TIMEOUT_HPP

