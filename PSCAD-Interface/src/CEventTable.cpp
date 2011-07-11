#include "CEventTable.hpp"

void CEventTable::Insert( double p_time, size_t p_device, double p_value )
{
    // create and store new event entry
    boost::mutex::scoped_lock lock(m_TableMutex);
    m_EventTable.insert(CEvent(p_time,p_device,p_value));
}

void CEventTable::Update( double p_time, double * p_data, size_t p_size )
{
    boost::mutex::scoped_lock lock(m_TableMutex);
    std::set<CEvent>::iterator it = m_EventTable.begin();

    // iterate through all due events (time <= simulation time)
    for( ; it != m_EventTable.end() && (*it).GetTime() <= p_time; it++ )
    {
        // prevent segmentation faults
        if( (*it).GetDevice() >= p_size )
        {
            throw std::runtime_error("unrecognized device index");
        }
            
        // update device value in data array
        p_data[(*it).GetDevice()] = (*it).GetValue();
    }

    // delete handled events
    m_EventTable.erase(m_EventTable.begin(),it);
}

void CEventTable::Clear()
{
    // erase all events
    m_EventTable.clear();
}

CEventTable::CEvent::CEvent( double p_time, size_t p_device, double p_value )
    : m_Time(p_time), m_Device(p_device), m_Value(p_value)
{
    if( p_time < 0 ) throw std::logic_error("negative event time");
}

bool CEventTable::CEvent::operator<( const CEvent & p_event ) const
{
    // device index used as tie breaker
    if( m_Time < p_event.m_Time )
    {
        return true;
    }
    if( m_Time == p_event.m_Time && m_Device < p_event.m_Device )
    {
        return true;
    }

    return false;
}

bool CEventTable::CEvent::operator==( const CEvent & p_event ) const
{
    // equivalence defined as same event time for same device index
    return( m_Time == p_event.m_Time && m_Device == p_event.m_Device );
}
