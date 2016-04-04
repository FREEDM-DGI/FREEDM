////////////////////////////////////////////////////////////////////////////////
/// @file           CLogger.cpp
///
/// @author         Stephen Jackson   <scj7t4@mst.edu>
/// @author         Michael Catanzaro <michael.catanzaro@mst.edu>
///
/// @project        FREEDM DGI
///
/// @description    A logger to be included by Broker files.
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

#include "CLogger.hpp"
#include "CGlobalConfiguration.hpp"

#include <boost/program_options/options_description.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/mutex.hpp>

using namespace boost::posix_time;

namespace freedm {

namespace broker {

namespace {

/// This file's logger.
CLocalLogger Logger(__FILE__);

/// Only one logger can write at a time.
boost::mutex mutex;

}

///////////////////////////////////////////////////////////////////////////////
/// basename
/// @description Return the last component of a path. That is, given a
///     path like "/root/freedm/example", this function will return 
///     "example"
/// @pre None
/// @post None
/// @param s The path to extract from.
/// @return The last component of the path.
///////////////////////////////////////////////////////////////////////////////
std::string basename(const std::string s)
{
    // This works for both Windows and UNIX-style paths
    size_t idx;
    idx = s.find_last_of("/\\");
    return s.substr(idx + 1);
}

///////////////////////////////////////////////////////////////////////////////
/// CLog::CLog
/// @description Log constructor
/// @param p The parent logger object that manages this log
/// @param level_ the output level of this log.
/// @param name_ the name of this log
/// @param out_ the stream that the messages sent to this log will be written.
/// @pre None
/// @post None
///////////////////////////////////////////////////////////////////////////////
CLog::CLog(const CLoggerPointer p, const unsigned int level_,
        std::string name_, std::ostream* const out_) :
m_parent(p), m_level(level_), m_name(name_), m_ostream(out_)
{
    //pass
}

///////////////////////////////////////////////////////////////////////////////
/// CLog::write
/// @description Given an input array of characters, write that stream to
///     the screen with information about the log that captured those messages
/// @param s An array of characters to write to the log's stream.
/// @param n The size of the array that will be written
/// @pre None
/// @post The array is written to the log's stream if the level of the message
///     is greater than the Log's OutputLevel.
/// @return the number of characters written to the stream. 
//////////////////////////////////////////////////////////////////////////////
std::streamsize CLog::write(const char* const s, std::streamsize n)
{
    if (GetOutputLevel() >= m_level)
    {
        boost::lock_guard<boost::mutex> lock(mutex);
        *m_ostream << microsec_clock::local_time() + CGlobalConfiguration::Instance().GetClockSkew() << " : "
                << m_name << "(" << m_level << "):\n\t";
        boost::iostreams::write(*m_ostream, s, n);
    }
    return n;
}
///////////////////////////////////////////////////////////////////////////////
/// CLog::GetOutputLevel
/// @description Get's the output level (which messages to show) from the
///     CLogger that owns this log.
/// @pre None
/// @post None
/// @return The output level of the Logger.
///////////////////////////////////////////////////////////////////////////////
unsigned int CLog::GetOutputLevel() const
{
    return m_parent->GetOutputLevel();
}

///////////////////////////////////////////////////////////////////////////////
/// CLocalLogger::CLocalLogger
/// @description Constructor for the CLocalLogger, an object that manages a
///     a group of CLogs of differing output levels.
/// @param loggername The name of this logger set.
/// @pre None
/// @post Creates log levels 0-8 for the logger specified by loggername.
///     Registers this local logger with the GlobalLogger.
///////////////////////////////////////////////////////////////////////////////
CLocalLogger::CLocalLogger(const std::string loggername)
: Trace(this, 8, basename(loggername) + " : Trace"),
Debug(this, 7, basename(loggername) + " : Debug"),
Info(this, 6, basename(loggername) + " : Info"),
Notice(this, 5, basename(loggername) + " : Notice"),
Status(this, 4, basename(loggername) + " : Status"),
Warn(this, 3, basename(loggername) + " : Warn"),
Error(this, 2, basename(loggername) + " : Error"),
Alert(this, 1, basename(loggername) + " : Alert"),
Fatal(this, 0, basename(loggername) + " : Fatal"),
m_name(basename(loggername))
{
    CGlobalLogger::instance().RegisterLocalLogger(m_name);
}
///////////////////////////////////////////////////////////////////////////////
/// CLocalLogger::GetName
/// @description Gets the name of this logger
/// @pre None
/// @post None
/// @return The name of the logger.
///////////////////////////////////////////////////////////////////////////////
std::string CLocalLogger::GetName() const
{
    return m_name;
}

///////////////////////////////////////////////////////////////////////////////
/// CLocalLogger::GetOutputLevel
/// @description Gets the logger's output level, which decides how verbose the
///     logger will be
/// @pre None
/// @post None
/// @return The output level of this logger.
///////////////////////////////////////////////////////////////////////////////
unsigned int CLocalLogger::GetOutputLevel() const
{
    return CGlobalLogger::instance().GetOutputLevel(m_name);
}
///////////////////////////////////////////////////////////////////////////////
/// CLocalLogger::SetOutputLevel
/// @description Sets the logger's output level, which describes how verbose
///     the logger is
/// @pre None
/// @post The level value is stored in the GlobalLogger.
///////////////////////////////////////////////////////////////////////////////
void CLocalLogger::SetOutputLevel(const unsigned int level)
{
    CGlobalLogger::instance().SetOutputLevel(m_name, level);
}
///////////////////////////////////////////////////////////////////////////////
/// CGlobalLogger::instance
/// @description Gets the global logger which stores the configuration info
///     for the other loggers.
/// @pre None
/// @post None
/// @return The instance of the global logger.
/////////////////////////////////////////////////////////////////////////////// 
CGlobalLogger& CGlobalLogger::instance()
{
    static CGlobalLogger singleton;
    return singleton;
}
///////////////////////////////////////////////////////////////////////////////
/// CGlobalLogger::RegisterLocalLogger
/// @description Registers a local logger with the global logger so it can
///     be enumerated for functions like SetGlobalLevel.
/// @pre None
/// @post The loggers name is added to the loggers table.
/// @param logger the name of the CLogger being added.
///////////////////////////////////////////////////////////////////////////////
void CGlobalLogger::RegisterLocalLogger(const std::string logger)
{
    m_loggers.insert(std::make_pair(logger, m_default));
}
///////////////////////////////////////////////////////////////////////////////
/// CGlobalLogger::SetGlobalLevel
/// @description For reach logger that the GlobalLogger is aware of set the
///     loggers level to the specified value
/// @pre None
/// @post Every logger's log level is changed to level.
/// @param level The new desired log level.
///////////////////////////////////////////////////////////////////////////////
void CGlobalLogger::SetGlobalLevel(const unsigned int level)
{
    // Iterate over the registered loggers and
    // set the filter levels of each logger
    OutputMap::iterator it;
    for (it = m_loggers.begin(); it != m_loggers.end(); it++)
    {
        ( *it ).second = level;
    }
    m_default = level;
}
///////////////////////////////////////////////////////////////////////////////
/// CGlobalLogger::SetOutputLevel
/// @description Sets the output level of the specified logger.
/// @pre The logger has been registered with the GlobalLogger.
/// @post The logger's level has been set to the specified value.
/// @param logger The name of the logger to modify.
/// @param level The level the the logger's output should be set to.
///////////////////////////////////////////////////////////////////////////////
void CGlobalLogger::SetOutputLevel(const std::string logger,
        const unsigned int level)
{
    //Fetch the specified logger and set its level to the one specified
    m_loggers[logger] = level;
}
///////////////////////////////////////////////////////////////////////////////
/// CGlobalLogger::GetOutputLevel
/// @description Gets the output level of the specified logger.
/// @pre None
/// @post Throws an exception if the specified logger does not exist.
/// @param logger the name of the logger to fetch the output level for.
/// @return The output level of the logger.
///////////////////////////////////////////////////////////////////////////////
unsigned int CGlobalLogger::GetOutputLevel(const std::string logger) const
{
    OutputMap::const_iterator it = m_loggers.find(logger);
    if (it == m_loggers.end())
    {
        throw std::runtime_error(
                "Requested output level of unregistered logger " + logger);
    }
    return m_loggers.find(logger)->second;
}
///////////////////////////////////////////////////////////////////////////////
/// CGlobalLogger::SetInitialLoggerLevels
/// @description Sets the logger verbosity levels to the values specified as
///     default or in one of the configuration files.
/// @pre None
/// @post Logger levels are set to the levels specified in the logger config
///     file. Exceptions may be thrown if the file is not correctly formatted
///     or contains invalid entries.
/// @param loggerCfgFile the name of the config file to process the values
///     from.
///////////////////////////////////////////////////////////////////////////////
void CGlobalLogger::SetInitialLoggerLevels(const std::string loggerCfgFile)
{
    std::ifstream ifs;

    po::options_description loggerOpts("Logger Verbosity Settings");
    po::variables_map vm;

    OutputMap::iterator it;
    for (it = m_loggers.begin(); it != m_loggers.end(); it++)
    {
        std::string desc = "The verbosity level for " + it->first;
        loggerOpts.add_options()
                ( it->first.c_str(),
                po::value<unsigned int>( )->default_value(m_default),
                desc.c_str() );
    }

    ifs.open(loggerCfgFile.c_str());
    if (!ifs)
    {
        Logger.Warn << "Unable to load logger config file: "
                << loggerCfgFile << std::endl;
        return;
    }
    else
    {
        // Process the config
        try
        {
            po::store(parse_config_file(ifs, loggerOpts), vm);
        }
        catch( po::unknown_option & e)
        {
            throw std::runtime_error( "Invalid logger name '"
                    + e.get_option_name() + "' in " + loggerCfgFile );
        }
        po::notify(vm);
        Logger.Info << "Logger config file " << loggerCfgFile <<
                " successfully loaded." << std::endl;
    }
    ifs.close();

    // Can't use a type with a comma in BOOST_FOREACH since it's a macro
    typedef std::pair<std::string, po::variable_value> VerbosityPair;
    BOOST_FOREACH(VerbosityPair pair, vm)
    {
        if (!vm[pair.first].defaulted())
        {
            m_loggers[pair.first] = pair.second.as<unsigned int>( );
        }
    }
}
///////////////////////////////////////////////////////////////////////////////
/// CGlobaLogger::ListLoggers
/// @description Prints a list of the loggers and their level to stdout.
/// @pre None
/// @post The registered loggers and their current level are printed to stdout.
///////////////////////////////////////////////////////////////////////////////
void CGlobalLogger::ListLoggers() const
{
    OutputMap::const_iterator it;
    for (it = m_loggers.begin(); it != m_loggers.end(); it++)
    {
        std::cout << ( *it ).first << "=" << ( *it ).second << std::endl;
    }
}

} // namespace broker

} // namespace freedm
