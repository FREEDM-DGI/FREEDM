#ifndef C_EVENT_TABLE_HPP
#define C_EVENT_TABLE_HPP

#include <set>
#include <stdexcept>
#include <boost/thread/mutex.hpp>

class CEventTable
{
public:
    void Insert( double p_time, size_t p_device, double p_value );
    void Update( double p_time, double * p_data, size_t p_size );
    void Clear();
private:
    class CEvent
    {
    public:
        CEvent( double p_time, size_t p_device, double p_value );

        double GetTime() const { return m_Time; }
        size_t GetDevice() const { return m_Device; }
        double GetValue() const { return m_Value; }

        bool operator<( const CEvent & p_event ) const;
        bool operator==( const CEvent & p_event ) const;
    private:
        double m_Time;
        size_t m_Device;
        double m_Value;
    };

    std::set<CEvent> m_EventTable;
    boost::mutex m_TableMutex;
};

#endif // C_EVENT_TABLE_HPP
