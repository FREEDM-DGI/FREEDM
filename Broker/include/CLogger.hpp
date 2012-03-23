#ifndef CLOGGER_HPP
#define CLOGGER_HPP

#include <boost/iostreams/concepts.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/shared_ptr.hpp>

#include <iostream>
#include <string>

#include "Utility.hpp"

using namespace boost::posix_time;

class CLocalLogger;

typedef CLocalLogger* CLoggerPointer;

class CGlobalLogger : public Templates::Singleton<CGlobalLogger>
{
    public:
        CGlobalLogger();
        void SetGlobalLevel(int level);
        void SetOutputLevel(std::string logger,int level);
        int GetOutputLevel(std::string logger);
    private:
        int m_default;
        typedef std::map< std::string, int > OutputMap;
        OutputMap m_loggers;
};

/// Logging Output Software
class CLog : public boost::iostreams::sink
{
    public:
        /// Constructor; prepares a log of a specifed level.
        CLog(CLoggerPointer p, int level_, const char * name_, std::ostream *out_= &std::clog );
        /// Wrtes from a character array into the logger stream
        std::streamsize write( const char* s, std::streamsize n);
        /// Determine the level of this logger
        int GetOutputLevel();
    private:
        CLoggerPointer m_parent; /// The local logger managing this logger
        const int m_level; /// The level of this logger
        const std::string m_name; /// String name of this logger
        std::ostream *m_ostream; // Output stream to use.
};

class CLocalLogger : private boost::noncopyable
{
    public:
        //Intializes the local statics
        CLocalLogger(std::string loggername);
	    boost::iostreams::stream<CLog> Debug;
        boost::iostreams::stream<CLog> Info;
        boost::iostreams::stream<CLog> Notice;
        boost::iostreams::stream<CLog> Status;
        boost::iostreams::stream<CLog> Warn;
        boost::iostreams::stream<CLog> Error;
        boost::iostreams::stream<CLog> Alert;
        boost::iostreams::stream<CLog> Fatal;
        std::string GetName();
        int GetOutputLevel();
        void SetOutputLevel(int level);
    private:
        std::string m_name;
};

#endif
