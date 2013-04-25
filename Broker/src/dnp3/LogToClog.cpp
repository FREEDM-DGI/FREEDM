#include "LogToClog.hpp"
#include "CLogger.hpp"

namespace freedm {
namespace broker {

namespace {
/// This file's logger.
CLocalLogger Logger(__FILE__);
}

/// Creates the static instance.
LogToClog::LogToClog()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
}

/// Gets the static instance.
LogToClog * LogToClog::Inst()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    static LogToClog instance;
    return &instance;
}

/// Writes some data to the log.
void LogToClog::Log(const apl::LogEntry & arEntry)
{
    Logger.Info << arEntry.LogString() << std::endl;
}

/// Required implementation of a pure virtual function.
void LogToClog::SetVar(const std::string & aSource, const std::string & aVarName, int aValue)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
}

} // namespace freedm
} // namespace broker

