////////////////////////////////////////////////////////////////////////////////
/// @file         CMqttMessage.cpp
///
/// @author       Thomas Roth <tprfh7@mst.edu>
///
/// @project      FREEDM DGI
///
/// @description  Stores data associated with a single MQTT message.
///
/// @functions    CMqttMessage::CMqttMessage
///               CMqttMessage::~CMqttMessage
///               CMqttMessage::Create
///
/// These source code files were created at Missouri University of Science and
/// Technology, and are intended for use in teaching or research. They may be
/// freely copied, modified, and redistributed as long as modified versions are
/// clearly marked as such and this notice is not removed. Neither the authors
/// nor Missouri S&T make any warranty, express or implied, nor assume any legal
/// responsibility for the accuracy, completeness, or usefulness of these files
/// or any information distributed with these files.
///
/// Suggested modifications or questions about these files can be directed to
/// Dr. Bruce McMillin, Department of Computer Science, Missouri University of
/// Science and Technology, Rolla, MO 65409 <ff@mst.edu>.
////////////////////////////////////////////////////////////////////////////////

#include "CMqttMessage.hpp"
#include "CLogger.hpp"

#include <string.h>

namespace freedm {
namespace broker {
namespace device {

namespace {
/// This file's logger.
CLocalLogger Logger(__FILE__);
} // unnamed namespace

CMqttMessage::CMqttMessage(std::string content)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    std::size_t size = content.size() + 1;
    m_Payload = new char[size];
    memcpy(m_Payload, content.c_str(), size);

    m_Message.struct_id[0] = 'M';
    m_Message.struct_id[1] = 'Q';
    m_Message.struct_id[2] = 'T';
    m_Message.struct_id[3] = 'M';
    m_Message.struct_version = 0;
    m_Message.payloadlen = size;
    m_Message.payload = m_Payload;
    m_Message.qos = 1;
    m_Message.retained = 0;
    m_Message.dup = 0;
}

CMqttMessage::~CMqttMessage()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    
    if(m_Payload != NULL)
    {
        delete [] m_Payload;
    }
}

CMqttMessage::Pointer CMqttMessage::Create(std::string content)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    return Pointer(new CMqttMessage(content));
}

}//namespace broker
}//namespace freedm
}//namespace device

