///////////////////////////////////////////////////////////////////////////////
/// @file      UnitTest.hpp
///
/// @author    Derek Ditch <derek.ditch@mst.edu>
///
/// @compiler  C++
///
/// @project   FREEDM DGI
///
/// @description Sets up testing environment for unit tests. Based upon
/// unit tests for Boost.Asio.
///
/// These source code files were created at as part of the
/// FREEDM DGI Subthrust, and are
/// intended for use in teaching or research.  They may be 
/// freely copied, modified and redistributed as long
/// as modified versions are clearly marked as such and
/// this notice is not removed.
/// 
/// Neither the authors nor the FREEDM Project nor the
/// National Science Foundation
/// make any warranty, express or implied, nor assumes
/// any legal responsibility for the accuracy,
/// completeness or usefulness of these codes or any
/// information distributed with these codes.
///
/// Suggested modifications or questions about these codes 
/// can be directed to Dr. Bruce McMillin, Department of 
/// Computer Science, Missour University of Science and
/// Technology, Rolla, /// MO  65409 (ff@mst.edu).
///
///////////////////////////////////////////////////////////////////////////////
#ifndef UNIT_TEST_HPP
#define UNIT_TEST_HPP

#include <boost/test/unit_test.hpp>
#include <boost/test/included/unit_test_framework.hpp>
using boost::unit_test::test_suite;

#include <boost/property_tree/ptree.hpp>
using boost::property_tree::ptree;

#define UNUSED_ARGUMENT(x) (void)x

#include "logger.hpp"
CREATE_STD_LOGS()

#include <exception>

struct FreedmTestException : public std::exception
{
    virtual const char* what() const throw()
    {
        return "Freedm Test Exception was thrown.";
    }
};

/// This test is useful for checking if a header can pass a parser.
inline void null_test()
{
}

#endif // UNIT_TEST_HPP
