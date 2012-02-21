#ifndef __LOGGER_HPP__
#define __LOGGER_HPP__

#include <boost/iostreams/concepts.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <iostream>
#include <string>

using namespace boost::posix_time;

namespace Logger {

/// Logging Output Software
class Log : public boost::iostreams::sink
{
public:
    /// Constructor; prepares a log of a specifed level.
    Log( int level_, const char * name_, std::ostream *out_= &std::clog ) :
        m_level(level_), m_name( name_ ), m_ostream( out_ )
    {
    };

    /// Wrtes from a character array into the logger stream
    std::streamsize write( const char* s, std::streamsize n)
    {
        if( m_filter >= m_level ){
            *m_ostream << microsec_clock::local_time() << " : "
                << m_name << "(" << m_level << "):\t";
                       
            boost::iostreams::write( *m_ostream, s, n);
        }
        
        return n;
    };
    
    /// Sets the output level, if the logger level is less, then it won't be output
    static void setLevel( const int p_level )
    {
        m_filter = p_level;
    }

private:
    static int m_filter; /// The level of logger to output
    const int m_level; /// The level of this logger
    const std::string m_name; /// String name of this logger
    std::ostream *m_ostream; // Output stream to use.
};

}

#define CREATE_LOG( level, name ) \
    boost::iostreams::stream<Logger::Log> name( level, #name )

#define CREATE_EXTERN_LOG( level, name ) \
    boost::iostreams::stream<Logger::Log> name

#define CREATE_STD_LOGS() \
	namespace Logger { \
	CREATE_LOG(7, Debug); \
	CREATE_LOG(6, Info); \
	CREATE_LOG(5, Notice); \
	CREATE_LOG(4, Status); \
	CREATE_LOG(3, Warn); \
	CREATE_LOG(2, Error); \
	CREATE_LOG(1, Alert); \
	CREATE_LOG(0, Fatal); \
	int Log::m_filter=0; \
	}

#ifndef CREATE_EXTERN_STD_LOGS
    #define CREATE_EXTERN_STD_LOGS() \
        namespace Logger { \
        extern CREATE_EXTERN_LOG(7, Debug); \
        extern CREATE_EXTERN_LOG(6, Info); \
        extern CREATE_EXTERN_LOG(5, Notice); \
        extern CREATE_EXTERN_LOG(4, Status); \
        extern CREATE_EXTERN_LOG(3, Warn); \
        extern CREATE_EXTERN_LOG(2, Error); \
        extern CREATE_EXTERN_LOG(1, Alert); \
        extern CREATE_EXTERN_LOG(0, Fatal); \
        }
#else
    #undef CREATE_EXTERN_STD_LOGS
    #define CREATE_EXTERN_STD_LOGS()
#endif


#endif // __LOGGER_HPP__
