#ifndef C_DATA_MANAGER_HPP
#define C_DATA_MANAGER_HPP

#include <map>
#include <string>

#include <boost/noncopyable.hpp>
#include <boost/property_tree/ptree.hpp>

namespace freedm {
namespace broker {

class CDataManager
    : private boost::noncopyable
{
public:
    static CDataManager & Instance();
    void AddData(std::string key, float value);
    float GetData(std::string key, float time);
    void AddFIDState(const std::map<std::string, bool> & fidstate);
    std::map<std::string, bool> GetFIDState(float time);
private:
    static const std::size_t MAX_DATA_ENTRIES = 100;
    boost::property_tree::ptree m_data;
    std::map<float, std::map<std::string, bool> > m_fidstate;
};

} // namespace broker
} // namespace freedm

#endif // C_DATA_MANAGER_HPP

