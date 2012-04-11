////////////////////////////////////////////////////////////////////////////////
/// @file           CLogger.cpp
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

#include <boost/program_options/options_description.hpp>

#include "CLogger.hpp"


CLog::CLog(CLoggerPointer p, int level_, const char * name_, std::ostream *out_) :
        m_parent(p), m_level(level_), m_name( name_ ), m_ostream( out_ )
{
    //pass
}

std::streamsize CLog::write( const char* s, std::streamsize n)
{
    if( GetOutputLevel() >= m_level ){
        *m_ostream << microsec_clock::local_time() << " : "
            << m_name << "(" << m_level << "):\t";
                   
        boost::iostreams::write( *m_ostream, s, n);
    }
    return n;
}

int CLog::GetOutputLevel()
{
    return m_parent->GetOutputLevel();
}

CLocalLogger::CLocalLogger(std::string loggername)
    : Debug(this, 7,"Debug"),
      Info(this, 6,"Info"),
      Notice(this,5,"Notice"),
      Status(this,4,"Status"),
      Warn(this, 3,"Warn"),
      Error(this,2,"Error"),
      Alert(this,1,"Alert"),
      Fatal(this,0,"Fatal"),
      m_name(loggername)
{
    // Pass
}

std::string CLocalLogger::GetName()
{
    return m_name;
}

int CLocalLogger::GetOutputLevel()
{
    return CGlobalLogger::instance().GetOutputLevel(m_name);
}

void CLocalLogger::SetOutputLevel(int level)
{
    CGlobalLogger::instance().SetOutputLevel(m_name,level);
}



void CGlobalLogger::SetGlobalLevel(int level)
{
    //Iterate over the registered loggers and set the filter levels of each logger
    OutputMap::iterator it;
    for(it = m_loggers.begin(); it != m_loggers.end(); it++)
    {
        (*it).second = level;
    }
    m_default = level;
}

void CGlobalLogger::SetOutputLevel(std::string logger,int level)
{
    //Fetch the specified logger and set its level to the one specified
    m_loggers[logger] = level;
}

int CGlobalLogger::GetOutputLevel(std::string logger)
{
    OutputMap::iterator it = m_loggers.find(logger);
    if(it == m_loggers.end())
    {
        m_loggers[logger] = m_default;
    }
    return m_loggers[logger];
}

void CGlobalLogger::SetInitialLoggerLevels(const std::string loggerCfgFile,
        const unsigned int globalVerbosity)
{
    SetGlobalLevel(globalVerbosity);
 //       po::options_description loggerOpts("Logger Verbosity Settings");
    
/*        // Separate config file for logger verbosity settings.
        ifs.open(loggerCfgFile.c_str());
        if (!ifs)
        {
            if (!vm["logger-config"].defaulted())
            {   // User specified a config file, so we should let
                // them know that we can't load it
                Logger.Error << "Unable to load logger config file: "
                        << loggerCfgFile << std::endl;
                return -1;
            }
            else
            {
                // File doesn't exist or couldn't open it for read.
                Logger.Error << "Logger config file " << loggerCfgFile << 
                        " doesn't exist. Skipping." << std::endl;
            }
        }
        else
        {
            // Process the config
            po::store(parse_config_file(ifs, logOpts), vm);
            po::notify(vm);
            Logger.Info << "Logger config file " << loggerCfgFile << 
                    " successfully loaded." << std::endl;
        }
        ifs.close();
        
        CGlobalLogger::instance().SetGlobalLevel( globalVerbosity );
    
        ("verbose,v", 
                po::value<int>(&globalVerbosity)->
        implicit_value(6)->default_value(4),
                "The default global verbosity level");
 */
}