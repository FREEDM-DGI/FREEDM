#include <string>
#include <map>

#ifndef CTIMINGS_HPP
#define CTIMINGS_HPP

namespace freedm {
namespace broker {

class CTimings
{
public:
    static void SetTimings(const std::string timingsFile);
    static unsigned int Get(const std::string param);
private:
    static void TimingParameters();
	static void RegisterLoggerOption(po::options_description&, const std::string param);
	static void AssignTimingValue(unsigned int& var, const std::string param);
    static std::map<std::string, unsigned int> timing_values;
};


}
}

#endif
