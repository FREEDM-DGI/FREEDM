#include "CLogger.hpp"

CLog::CLog(CLoggerPointer p, int level_, const char * name_, std::ostream *out_) :
        m_parent(p), m_level(level_), m_name( name_ ), m_ostream( out_ )
{
    //pass
}

    /// Wrtes from a character array into the logger stream
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



CGlobalLogger::CGlobalLogger()
{
    //Pass?
    m_default = 0;
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
