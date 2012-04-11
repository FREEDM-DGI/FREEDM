////////////////////////////////////////////////////////////////////////////////
/// @file           CLogger.hpp
///
/// @author         Stephen Jackson <scj7t4@mst.edu>
///
/// @project        FREEDM DGI
///
/// @description    A logger to be included by Broker files.
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
/// Science and Technology, Rolla, MO 65409 <ff@mst.edu>.
////////////////////////////////////////////////////////////////////////////////

#ifndef CLOGGER_HPP
#define CLOGGER_HPP

#include <boost/iostreams/concepts.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/program_options.hpp>
#include <boost/shared_ptr.hpp>

#include <cstdlib>
#include <fstream>
#include <string>

#include "Utility.hpp"

namespace po = boost::program_options;

using namespace boost::posix_time;

class CLocalLogger;

typedef CLocalLogger* CLoggerPointer;

/// Tracks the global logging configuration
class CGlobalLogger : public Templates::Singleton<CGlobalLogger>
{
    ///////////////////////////////////////////////////////////////////////////
    /// @description The GlobalLogger is responsible for tracking a table which
    ///     lists the names of the different loggers and their current output
    ///     levels.
    /// @limitations: Singleton. Should not be copied.
    ///////////////////////////////////////////////////////////////////////////
    public:
        /// Sets the logging level of a specific logger.
        void SetOutputLevel(std::string logger, int level);
        /// Fetch the logging level of a specific logger.
        int GetOutputLevel(std::string logger);
        /// Sets the logging level of all loggers.
        void SetGlobalLevel(int level);
        /// Reads the logging levels of all loggers from the config file.
        void SetInitialLoggerLevels(const std::string loggerCfgFile, 
                const unsigned int globalVerbosity);
    private:
        /// What the output level is if not set specifically.
        int m_default;
        /// Type of container for the output levels.
        typedef std::map< std::string, int > OutputMap;
        /// The map of loggers to logger levels.
        OutputMap m_loggers;
};

/// Logging Output Software
class CLog : public boost::iostreams::sink
{
    ///////////////////////////////////////////////////////////////////////////
    /// @description: A specific logging stream interface.
    /// @limitations: Relies on having a parent LocalLogger to ask for its
    ///     output level
    ///////////////////////////////////////////////////////////////////////////
    public:
        /// Constructor; prepares a log of a specified level.
        CLog(CLoggerPointer p, int level_, const char * name_,
            std::ostream *out_= &std::clog );
        /// Writes from a character array into the logger stream
        std::streamsize write( const char* s, std::streamsize n);
        /// Determine the level of this logger
        int GetOutputLevel();
    private:
        /// The local logger managing this logger
        CLoggerPointer m_parent;
        /// The level of this logger
        const int m_level;
        /// String name of this logger
        const std::string m_name;
        /// Output stream to use.
        std::ostream *m_ostream;
};

class CLocalLogger : private boost::noncopyable
{
    ///////////////////////////////////////////////////////////////////////////
    /// @description: Tracks the loggers available to the namespace the logger
    ///     is instantiated in. Loggers can share names. Loggers are typically
    ///     declared as static members of the files that the log and are named
    ///     after those files.
    /// @limitations: Logging in header files is kind of hacky: the logger
    ///     must have a globally unique name, where the those in cpps can share
    ///     one name since they are declared statically.
    ///////////////////////////////////////////////////////////////////////////
    public:
        ///Initializes the local statics
        CLocalLogger(std::string loggername);
        ///Logger
	boost::iostreams::stream<CLog> Debug;
        ///Logger
        boost::iostreams::stream<CLog> Info;
        ///Logger
        boost::iostreams::stream<CLog> Notice;
        ///Logger
        boost::iostreams::stream<CLog> Status;
        ///Logger
        boost::iostreams::stream<CLog> Warn;
        ///Logger
        boost::iostreams::stream<CLog> Error;
        ///Logger
        boost::iostreams::stream<CLog> Alert;
        ///Logger
        boost::iostreams::stream<CLog> Fatal;
        /// Returns the name of this logger
        std::string GetName();
        /// Returns the filtering level for this set of loggers.
        int GetOutputLevel();
        /// Sets the output level for this set of loggers. 
        void SetOutputLevel(int level);
        /// Load the logger settings
        
    private:
        /// The name of this logger
        std::string m_name;
};

#endif
