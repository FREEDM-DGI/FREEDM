////////////////////////////////////////////////////////////////////////////////
/// @file           CLogger.cpp
///
/// @author         Stephen Jackson   <scj7t4@mst.edu>
///                 Michael Catanzaro <michael.catanzaro@mst.edu>
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
#include "Utility.hpp"

static CLocalLogger Logger(__FILE__);

CLog::CLog(const CLoggerPointer p, const unsigned int level_, 
        const char * name_, std::ostream *out_) :
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

unsigned int CLog::GetOutputLevel() const
{
    return m_parent->GetOutputLevel();
}

CLocalLogger::CLocalLogger(const std::string loggername)
    : Debug(this, 7,"Debug"),
      Info(this, 6,"Info"),
      Notice(this,5,"Notice"),
      Status(this,4,"Status"),
      Warn(this, 3,"Warn"),
      Error(this,2,"Error"),
      Alert(this,1,"Alert"),
      Fatal(this,0,"Fatal"),
      m_name(basename(loggername))
{
    CGlobalLogger::instance().RegisterLocalLogger(m_name);
}

std::string CLocalLogger::GetName() const
{
    return m_name;
}

unsigned int CLocalLogger::GetOutputLevel() const
{
    return CGlobalLogger::instance().GetOutputLevel(m_name);
}

void CLocalLogger::SetOutputLevel(const unsigned int level)
{
    CGlobalLogger::instance().SetOutputLevel(m_name,level);
}

CGlobalLogger& CGlobalLogger::instance()
{
    static CGlobalLogger singleton;
    return singleton;
}

void CGlobalLogger::RegisterLocalLogger(const std::string logger)
{
    m_loggers.insert(std::make_pair(logger, m_default));
}

void CGlobalLogger::SetGlobalLevel(const unsigned int level)
{
    // Iterate over the registered loggers and 
    // set the filter levels of each logger
    OutputMap::iterator it;
    for(it = m_loggers.begin(); it != m_loggers.end(); it++)
    {
        (*it).second = level;
    }
    m_default = level;
}

void CGlobalLogger::SetOutputLevel(const std::string logger, 
        const unsigned int level)
{
    //Fetch the specified logger and set its level to the one specified
    m_loggers[logger] = level;
}

unsigned int CGlobalLogger::GetOutputLevel(const std::string logger) const
{
    OutputMap::const_iterator it = m_loggers.find(logger);
    if (it == m_loggers.end())
    {
        throw std::string(
                "Requested output level of unregistered logger " + logger);
    }
    return m_loggers.find(logger)->second;
}

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
        Logger.Error << "Unable to load logger config file: "
                << loggerCfgFile << std::endl;
        std::exit(-1);
    }
    else
    {
        // Process the config
        po::store(parse_config_file(ifs, loggerOpts), vm);
        po::notify(vm);
        Logger.Info << "Logger config file " << loggerCfgFile <<
                " successfully loaded." << std::endl;
    }
    ifs.close();
    
    // Can't use a type with a comma in BOOST_FOREACH since it's a macro
    typedef std::pair<std::string, po::variable_value> VerbosityPair;
    
    foreach (VerbosityPair pair, vm)
    {
        if (!vm[pair.first].defaulted())
        {
            m_loggers[pair.first] = pair.second.as<unsigned int>();
        }
    }
}
