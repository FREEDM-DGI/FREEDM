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
///     TimedRead
///     TimedReadUntil
///     TimedWrite
///     
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

#include "CLogger.hpp"

#include <stdexcept>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/optional.hpp>

namespace freedm {
namespace broker {
namespace device {

namespace {
/// This file's logger.
CLocalLogger SyncTimeoutLogger(__FILE__);
}

/// Callback function for the timed socket operations.
void SetResult(boost::optional<boost::system::error_code> * status,
        const boost::system::error_code & error);

////////////////////////////////////////////////////////////////////////////////
/// A read that blocks until the buffer is full or the time duration expires.
///
/// @ErrorHandling Throws std::runtime_error if the time duration expires.
/// @pre The stream must receive data within the given time duration.
/// @post The call will block until data is read from the stream.
/// @post The result of the read will be stored in the passed buffer.
/// @param stream The read stream that will receive data.
/// @param buffer The buffer to store the result of the read.
/// @param duration The amount of time in milliseconds before a timeout.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
template <typename ReadStream, typename BufferSequence>
void TimedRead(ReadStream & stream, const BufferSequence & buffer, int duration)
{
    SyncTimeoutLogger.Trace << __PRETTY_FUNCTION__ << std::endl;

    boost::optional<boost::system::error_code> result, timeout;
    boost::asio::deadline_timer timer(stream.get_io_service());

    timer.expires_from_now(boost::posix_time::milliseconds(duration));
    timer.async_wait(boost::bind(SetResult, &timeout,
            boost::asio::placeholders::error));
    boost::asio::async_read(stream, buffer, boost::bind(SetResult, &result,
            boost::asio::placeholders::error));

    SyncTimeoutLogger.Info << "Blocking for synchronous read." << std::endl;

    while( !result && !timeout )
    {
        stream.get_io_service().run_one();
    }

    if( timeout )
    {
        throw std::runtime_error("Synchronous Read Timeout");
    }
    else
    {
        timer.cancel();
    }
    SyncTimeoutLogger.Info << "Synchronous read complete." << std::endl;
}

////////////////////////////////////////////////////////////////////////////////
/// A read that blocks until a delimiter is read or the time duration expires.
///
/// @ErrorHandling Throws std::runtime_error if the time duration expires.
/// @pre The stream must receive delimited data within the time duration.
/// @post The call will block until the delimiter is read from the stream.
/// @post The result of the read will be stored in the passed buffer.
/// @param stream The read stream that will receive delimited data.
/// @param buffer The buffer to store the reuslt of the read.
/// @param delim The delimiter that marks the end of the data.
/// @param duration The amount of time in milliseconds before a timeout.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
template <typename ReadStream, typename Allocator>
void TimedReadUntil(ReadStream & stream,
        boost::asio::basic_streambuf<Allocator> & buffer,
        const std::string & delim, int duration)
{
    SyncTimeoutLogger.Trace << __PRETTY_FUNCTION__ << std::endl;

    boost::optional<boost::system::error_code> result, timeout;
    boost::asio::deadline_timer timer(stream.get_io_service());

    timer.expires_from_now(boost::posix_time::milliseconds(duration));
    timer.async_wait(boost::bind(SetResult, &timeout,
            boost::asio::placeholders::error));
    boost::asio::async_read_until(stream, buffer, delim, boost::bind(SetResult,
            &result, boost::asio::placeholders::error));

    SyncTimeoutLogger.Info << "Blocking for synchronous read." << std::endl;

    while( !result && !timeout )
    {
        stream.get_io_service().run_one();
    }

    if( timeout )
    {
        throw std::runtime_error("Synchronous Read Until Timeout");
    }
    else
    {
        timer.cancel();
    }
    SyncTimeoutLogger.Info << "Synchronous read complete." << std::endl;
}

////////////////////////////////////////////////////////////////////////////////
/// A write that blocks until either successful or the time duration expires.
///
/// @ErrorHandling Throws std::runtime_error if the time duration expires.
/// @pre The write must complete within the given time duration.
/// @post The call will block until the write is successful.
/// @param stream The write stream to receive the data.
/// @param buffer The buffered data to write to the stream.
/// @param duration The amount of time in milliseconds before a timeout.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
template <typename WriteStream, typename BufferSequence>
void TimedWrite(WriteStream & stream, const BufferSequence & buffer,
        int duration)
{
    SyncTimeoutLogger.Trace << __PRETTY_FUNCTION__ << std::endl;

    boost::optional<boost::system::error_code> result, timeout;
    boost::asio::deadline_timer timer(stream.get_io_service());

    timer.expires_from_now(boost::posix_time::milliseconds(duration));
    timer.async_wait(boost::bind(SetResult, &timeout, _1));
    boost::asio::async_write(stream, buffer, boost::bind(SetResult, &result, _1));

    SyncTimeoutLogger.Info << "Blocking for synchronous write." << std::endl;

    while( !result && !timeout )
    {
        stream.get_io_service().run_one();
    }

    if( timeout )
    {
        throw std::runtime_error("Synchronous Write Timeout");
    }
    else
    {
        timer.cancel();
    }
    SyncTimeoutLogger.Info << "Synchronous write complete." << std::endl;
}

////////////////////////////////////////////////////////////////////////////////
/// A write that blocks until either successful or the time duration expires.
///
/// @ErrorHandling Throws std::runtime_error if the time duration expires.
/// @pre The write must complete within the given time duration.
/// @post The call will block until the write is successful.
/// @param stream The write stream to receive the data.
/// @param buffer The buffered data to write to the stream.
/// @param duration The amount of time in milliseconds before a timeout.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
template <typename WriteStream, typename Allocator>
void TimedWrite(WriteStream & stream,
        boost::asio::basic_streambuf<Allocator> & buffer, int duration)
{
    SyncTimeoutLogger.Trace << __PRETTY_FUNCTION__ << std::endl;

    boost::optional<boost::system::error_code> result, timeout;
    boost::asio::deadline_timer timer(stream.get_io_service());

    timer.expires_from_now(boost::posix_time::milliseconds(duration));
    timer.async_wait(boost::bind(SetResult, &timeout,
            boost::asio::placeholders::error));
    boost::asio::async_write(stream, buffer, boost::bind(SetResult, &result,
            boost::asio::placeholders::error));

    SyncTimeoutLogger.Info << "Blocking for synchronous write." << std::endl;

    while( !result && !timeout )
    {
        stream.get_io_service().run_one();
    }

    if( timeout )
    {
        throw std::runtime_error("Synchronous Write Timeout");
    }
    else
    {
        timer.cancel();
    }
    SyncTimeoutLogger.Info << "Synchronous write complete." << std::endl;
}

} // namespace device
} // namespace broker
} // namespace freedm

#endif // SYNCHRONOUS_TIMEOUT_HPP

