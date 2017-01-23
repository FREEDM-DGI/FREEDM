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

CMqttMessage::CMqttMessage(std::string topic, std::string content, int qos)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    m_Topic = topic;

    std::size_t size = content.size();
    m_Payload = new char[size];
    memcpy(m_Payload, content.c_str(), size);

    m_Message.struct_id[0] = 'M';
    m_Message.struct_id[1] = 'Q';
    m_Message.struct_id[2] = 'T';
    m_Message.struct_id[3] = 'M';
    m_Message.struct_version = 0;
    m_Message.payloadlen = size;
    m_Message.payload = m_Payload;
    m_Message.qos = qos;
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

CMqttMessage::Pointer CMqttMessage::Create(std::string topic, std::string content, int qos)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    return Pointer(new CMqttMessage(topic, content, qos));
}

const MQTTClient_deliveryToken & CMqttMessage::GetToken() const
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    return m_Token;
}

void CMqttMessage::Publish(MQTTClient client)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    if(MQTTClient_publishMessage(client, m_Topic.c_str(), &m_Message, &m_Token) != MQTTCLIENT_SUCCESS)
    {
        Logger.Error << "Message on topic " << m_Topic << " with value " << m_Payload << " rejected." << std::endl;
        throw std::runtime_error("Message Rejected for Publication");
    }
    Logger.Info << m_Topic << " " << m_Payload << " sent for delivery with token " << m_Token << std::endl;

}

}//namespace broker
}//namespace freedm
}//namespace device

