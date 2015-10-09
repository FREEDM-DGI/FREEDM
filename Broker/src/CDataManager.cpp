#include "CDataManager.hpp"
#include "CDeviceManager.hpp"
#include "CLogger.hpp"

#include <stdexcept>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/range/adaptor/map.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/property_tree/xml_parser.hpp>

namespace freedm {
namespace broker {

namespace {
CLocalLogger Logger(__FILE__);
}

CDataManager & CDataManager::Instance()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    static CDataManager instance;
    return instance;
}

void CDataManager::AddData(std::string key, float time, float value)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    m_data[key].insert(std::pair<float, float>(time, value));
    if(m_data[key].size() > MAX_DATA_ENTRIES)
    {
        std::map<float, float>::iterator it = m_data[key].begin();
        std::advance(it, m_data[key].size() - MAX_DATA_ENTRIES);
        m_data[key].erase(m_data[key].begin(), it);
        Logger.Info << "Deleted historic data for " << key << " before " << m_data[key].begin()->first() << std::endl;
    }
}

float CDataManager::GetData(std::string key, float time)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    std::map<float, float>::iterator it = m_data.at(key).find(time);
    if(it == m_data[key].end())
    {
        Logger.Notice << "No historic data for " << key << " at " << time << std::endl;
        for(it = m_data[key].begin(); it != m_data[key].end() && it->first < time; it++);
        if(it == m_data[key].begin() && it->first > time)
        {
            Logger.Error << "No historic data for " << key << " before requested time " << time << std::endl;
            throw std::runtime_error("Missing Historic Data");
        }
        else if(it->first > time)
        {
            it--;
        }
    }
    return it->second;
}

void CDataManager::AddFIDState(const std::map<std::string, bool> & fidstate)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    
    device::CDevice::Pointer clock = device::CDeviceManager::Instance().GetClock();

    if(clock)
    {
        float time = clock->GetState("time");
        m_fidstate[time] = fidstate;

        while(m_fidstate.size() > MAX_DATA_ENTRIES)
        {
            Logger.Info << "Deleted historic data for fidstate at time " << m_fidstate.begin()->first << std::endl;
            m_fidstate.erase(m_fidstate.begin());
        }
    }
    else
    {
        // because fidstate does not update each clock tick
        // the initial topology might be lost without this line
        m_fidstate[0] = fidstate;
    }
}

std::map<std::string, bool> CDataManager::GetFIDState(float time)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    
    if(time < m_fidstate.begin()->first)
    {
        Logger.Notice << "No FID state for time " << time << std::endl;
        throw std::runtime_error("Invalid FID State");
    }

    float matched_time = m_fidstate.begin()->first;

    BOOST_FOREACH(float t, m_fidstate | boost::adaptors::map_keys)
    {
        if(time >= t)
        {
            matched_time = t;
        }
    }
    return m_fidstate[matched_time];
}

} // namespace freedm
} // namespace broker

