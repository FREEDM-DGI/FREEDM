#ifndef FREEDM_BROKER_CUUIDHandler
#define FREEDM_BROKER_CUUIDHandler
#include <boost/property_tree/ptree.hpp>

#include "IHandler.hpp"
#include "uuid.hpp"

namespace freedm {
namespace broker {

class CUUIDHandler : public IWriteHandler
{
private:
    freedm::uuid m_uuid;
public:
    CUUIDHandler( freedm::uuid p_u ) :
        m_uuid( p_u )
    { }

    virtual ~CUUIDHandler()
    { }

    virtual void HandleWrite( ptree& pt )
    {
        pt.put("message.source", m_uuid );
    }
};

}
}


#endif // FREEDM_BROKER_CUUIDHandler
