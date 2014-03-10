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
#include <boost/shared_ptr.hpp>

namespace freedm {
namespace broker {
namespace device {

namespace {
/// This file's logger.
CLocalLogger SyncTimeoutLogger(__FILE__);
}

/// Convenience typedef for the SetResult function.
typedef boost::optional<boost::system::error_code> OptionalError;

/// Callback function for the timed synchronous operations.
void SetResult(boost::shared_ptr<OptionalError> status,
        const boost::system::error_code & error);

////////////////////////////////////////////////////////////////////////////////
/// A read that blocks until the buffer is full or a time duration expires.
///
/// @ErrorHandling Throws std::runtime_error if either the read operation fails
/// or the time duration expires.
/// @pre The stream must receive data within the given time duration.
/// @post The call will block until data is read from the stream.
/// @post The result of the read will be stored in the passed buffer.
/// @param stream The read stream that will receive data.
/// @param buffer The buffer to store the result of the read.
/// @param duration The amount of time in milliseconds for the timeout.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
template <typename ReadStream, typename BufferSequence>
void TimedRead(ReadStream & stream, const BufferSequence & buffer,
        unsigned int duration)
{
    SyncTimeoutLogger.Trace << __PRETTY_FUNCTION__ << std::endl;

    boost::asio::io_service & ios = stream.get_io_service();
    boost::shared_ptr<OptionalError> result(new OptionalError);
    boost::shared_ptr<OptionalError> timeout(new OptionalError);
    boost::asio::deadline_timer timer(ios);

    SyncTimeoutLogger.Info << "Blocking for synchronous read." << std::endl;

    boost::asio::async_read(stream, buffer,
            boost::bind(SetResult, result, boost::asio::placeholders::error));
    timer.expires_from_now(boost::posix_time::milliseconds(duration));
    timer.async_wait(boost::bind(SetResult, timeout,
            boost::asio::placeholders::error));

    while( !(*result) && !(*timeout) )
    {
        ios.poll();
    }

    if( *result && (*result)->value() == boost::system::errc::success )
    {
        SyncTimeoutLogger.Info << "Synchronous read complete." << std::endl;
    }
    else if( *timeout && (*timeout)->value() == boost::system::errc::success )
    {
        throw std::runtime_error("Synchronous Read Timeout");
    }
    else
    {
        throw std::runtime_error("Synchronous Read Failed");
    }
}

////////////////////////////////////////////////////////////////////////////////
/// A read that blocks until a delimiter is read or a time duration expires.
///
/// @ErrorHandling Throws std::runtime_error if either the read operation fails
/// or the time duration expires.
/// @pre The stream must receive delimited data within the time duration.
/// @post The call will block until the delimiter is read from the stream.
/// @post The result of the read will be stored in the passed buffer.
/// @param stream The read stream that will receive delimited data.
/// @param buffer The buffer to store the reuslt of the read.
/// @param delim The delimiter that marks the end of the data.
/// @param duration The amount of time in milliseconds for the timeout.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
template <typename ReadStream, typename Allocator>
void TimedReadUntil(ReadStream & stream,
        boost::asio::basic_streambuf<Allocator> & buffer,
        const std::string & delim,
        unsigned int duration)
{
    SyncTimeoutLogger.Trace << __PRETTY_FUNCTION__ << std::endl;

    boost::asio::io_service & ios = stream.get_io_service();
    boost::shared_ptr<OptionalError> result(new OptionalError);
    boost::shared_ptr<OptionalError> timeout(new OptionalError);
    boost::asio::deadline_timer timer(ios);

    SyncTimeoutLogger.Info << "Blocking for synchronous read." << std::endl;

    boost::asio::async_read_until(stream, buffer, delim,
            boost::bind(SetResult, result, boost::asio::placeholders::error));
    timer.expires_from_now(boost::posix_time::milliseconds(duration));
    timer.async_wait(boost::bind(SetResult, timeout,
            boost::asio::placeholders::error));

    while( !(*result) && !(*timeout) )
    {
        ios.poll();
    }

    if( *result && (*result)->value() == boost::system::errc::success )
    {
        SyncTimeoutLogger.Info << "Synchronous read complete." << std::endl;
    }
    else if( *timeout && (*timeout)->value() == boost::system::errc::success )
    {
        throw std::runtime_error("Synchronous Read Until Timeout");
    }
    else
    {
        throw std::runtime_error("Synchronous Read Until Failed");
    }
}

////////////////////////////////////////////////////////////////////////////////
/// A write that blocks until either completion or a time duration expires.
///
/// @ErrorHandling Throws std::runtime_error if the write operation fails or
/// the time duration expires.
/// @pre The write must complete within the given time duration.
/// @post The call will block until the write is successful.
/// @param stream The write stream to receive the data.
/// @param buffer The buffered data to write to the stream.
/// @param duration The amount of time in milliseconds for the timeout.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
template <typename WriteStream, typename BufferSequence>
void TimedWrite(WriteStream & stream, const BufferSequence & buffer,
        unsigned int duration)
{
    SyncTimeoutLogger.Trace << __PRETTY_FUNCTION__ << std::endl;

    boost::asio::io_service & ios = stream.get_io_service();
    boost::shared_ptr<OptionalError> result(new OptionalError);
    boost::shared_ptr<OptionalError> timeout(new OptionalError);
    boost::asio::deadline_timer timer(ios);

    SyncTimeoutLogger.Info << "Blocking for synchronous write." << std::endl;

    boost::asio::async_write(stream, buffer,
            boost::bind(SetResult, result, boost::asio::placeholders::error));
    timer.expires_from_now(boost::posix_time::milliseconds(duration));
    timer.async_wait(boost::bind(SetResult, timeout,
            boost::asio::placeholders::error));

    while( !(*result) && !(*timeout) )
    {
        ios.poll();
    }

    if( *result && (*result)->value() == boost::system::errc::success )
    {
        SyncTimeoutLogger.Info << "Synchronous write complete." << std::endl;
    }
    else if( *timeout && (*timeout)->value() == boost::system::errc::success )
    {
        throw std::runtime_error("Synchronous Write Timeout");
    }
    else
    {
        throw std::runtime_error("Synchronous Write Failed");
    }
}

////////////////////////////////////////////////////////////////////////////////
/// A write that blocks until either completion or a time duration expires.
///
/// @ErrorHandling Throws std::runtime_error if the write opertaion fails or
/// the time duration expires.
/// @pre The write must complete within the given time duration.
/// @post The call will block until the write is successful.
/// @param stream The write stream to receive the data.
/// @param buffer The buffered data to write to the stream.
/// @param duration The amount of time in milliseconds for the timeout.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
template <typename WriteStream, typename Allocator>
void TimedWrite(WriteStream & stream,
        boost::asio::basic_streambuf<Allocator> & buffer,
        unsigned int duration)
{
    SyncTimeoutLogger.Trace << __PRETTY_FUNCTION__ << std::endl;

    boost::asio::io_service & ios = stream.get_io_service();
    boost::shared_ptr<OptionalError> result(new OptionalError);
    boost::shared_ptr<OptionalError> timeout(new OptionalError);
    boost::asio::deadline_timer timer(ios);

    SyncTimeoutLogger.Info << "Blocking for synchronous write." << std::endl;

    boost::asio::async_write(stream, buffer,
            boost::bind(SetResult, result, boost::asio::placeholders::error));
    timer.expires_from_now(boost::posix_time::milliseconds(duration));
    timer.async_wait(boost::bind(SetResult, timeout,
            boost::asio::placeholders::error));

    while( !(*result) && !(*timeout) )
    {
        ios.poll();
    }

    if( *result && (*result)->value() == boost::system::errc::success )
    {
        SyncTimeoutLogger.Info << "Synchronous write complete." << std::endl;
    }
    else if( *timeout && (*timeout)->value() == boost::system::errc::success )
    {
        throw std::runtime_error("Synchronous Write Timeout");
    }
    else
    {
        throw std::runtime_error("Synchronous Write Failed");
    }
}

} // namespace device
} // namespace broker
} // namespace freedm

#endif // SYNCHRONOUS_TIMEOUT_HPP

