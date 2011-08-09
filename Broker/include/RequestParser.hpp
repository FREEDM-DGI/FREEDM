//
// RequestParser.hpp
// ~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2010 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef HTTP_CRequestParser_HPP
#define HTTP_CRequestParser_HPP

#include "CMessage.hpp"
#include "logger.hpp"
CREATE_EXTERN_STD_LOGS()


#include <algorithm>
#include <stdexcept>
#include <iostream>
#include <iterator>
#include <sstream>

#include <boost/logic/tribool.hpp>
#include <boost/tuple/tuple.hpp>

namespace freedm {
namespace broker {

/// Parse some data. The tribool return value is true when a complete request
/// has been parsed, false if the data is invalid, indeterminate when more
/// data is required. The InputIterator return value indicates how much of the
/// input has been consumed.
template <typename InputIterator>
boost::tuple<boost::tribool, InputIterator> Parse(CMessage &req,
        InputIterator begin, InputIterator end)
{
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;

    std::stringstream ss_;
    std::ostreambuf_iterator<char> ss_buf( ss_ );
    std::copy( begin, end, ss_buf );
    boost::tribool result = boost::tribool::indeterminate_value;

    try 
    {
        Logger::Debug << "Loading xml: " << std::endl
                << ss_.str() << std::endl;
        result = req.Load( ss_ );
    }
    catch ( std::exception &e )
    {
        // Perhaps incomplete message.
        Logger::Error << "Exception: " << e.what() << std::endl;
    }

    return boost::make_tuple(result, begin);
}

template <typename OutputIterator>
boost::tuple< boost::tribool, OutputIterator> Synthesize( CMessage &msg,
    OutputIterator begin, size_t p_outMaxLength )
{
    Logger::Debug << __PRETTY_FUNCTION__ << std::endl;
    std::stringstream ss_;
    std::string str_;
    boost::tribool result = boost::indeterminate;
    OutputIterator last;

    try
    {
        msg.Save( ss_ );
        Logger::Debug << "Saved xml: " << std::endl
                << ss_.str() << std::endl;
        result = true;
    }
    catch( std::exception &e )
    {
        Logger::Error
            << "Exception: " << e.what() << std::endl;
    }

    str_ = ss_.str();

    if( str_.length() <= p_outMaxLength )
    {
        last = std::copy( str_.begin(), str_.end(), begin  );
    }
    else
    {
        // This might be better handled with some
        // sort of iterative call
        last = begin;
        throw (
            std::length_error
                ( "Output stream too short for message size.")
        );
    }

    return boost::make_tuple( result, last );
}

} // namespace broker
} // namespace freedm

#endif // REQUESTPARSER_HPP
