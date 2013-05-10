#ifndef LOG_TO_C_LOG
#define LOG_TO_C_LOG

#include <string>

#include <APL/LogBase.h>
#include <APL/LogEntry.h>

namespace freedm {
namespace broker {

class LogToClog
    : public apl::ILogBase
{
public:
    /// Gets the static instance.
    static LogToClog * Inst();

    /// Writes some data to the log.
    void Log(const apl::LogEntry & arEntry);

    /// Required implementation of a pure virtual function.
    void SetVar(const std::string & aSource, const std::string & aVarName, int aValue);
private:
    /// Creates the static instance.
    LogToClog();
};

} // namespace freedm
} // namespace broker

#endif // LOG_TO_C_LOG

