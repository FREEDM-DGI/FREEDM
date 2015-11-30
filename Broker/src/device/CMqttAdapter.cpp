////////////////////////////////////////////////////////////////////////////////
/// @file         CMqttAdapter.cpp
///
/// @author       Thomas Roth <tprfh7@mst.edu>
///
/// @project      FREEDM DGI
///
/// @description  Implementation of an asynchronous PAHO MQTT client in DGI.
///
/// @functions    CMqttAdapter::CMqttAdapter
///               CMqttAdapter::~CMqttAdapter
///               CMqttAdapter::Create
///               CMqttAdapter::Start
///               CMqttAdapter::Stop
///               CMqttAdapter::GetState
///               CMqttAdapter::SetCommand
///               CMqttAdapter::ConnectionLost
///               CMqttAdapter::HandleMessage
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

#include "CMqttAdapter.hpp"
#include "CAdapterFactory.hpp"
#include "CLogger.hpp"

#include <stdexcept>

#include <boost/pointer_cast.hpp>

namespace freedm {
namespace broker {
namespace device {

namespace {
/// This file's logger.
CLocalLogger Logger(__FILE__);
} // unnamed namespace

CMqttAdapter::CMqttAdapter(std::string id, std::string address)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    // TODO - put a known prefix in front of the specified ID so devices know it is a DGI

    if(id.size() > 23)
        throw std::runtime_error("MQTT Client ID contains more than 23 characters");
    std::size_t length = id.copy(m_ID, 23);
    m_ID[length] = '\0';

    if(MQTTClient_create(&m_Client, address.c_str(), id.c_str(), MQTTCLIENT_PERSISTENCE_NONE, NULL) != MQTTCLIENT_SUCCESS)
        throw std::runtime_error("Failed to create the MQTT Client object");

    if(MQTTClient_setCallbacks(m_Client, &m_ID, ConnectionLost, HandleMessage, NULL) != MQTTCLIENT_SUCCESS)
        throw std::runtime_error("Failed to set the MQTT client callback functions");
}

CMqttAdapter::~CMqttAdapter()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    MQTTClient_destroy(&m_Client);
}

IAdapter::Pointer CMqttAdapter::Create(std::string id, std::string address)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    return CMqttAdapter::Pointer(new CMqttAdapter(id, address));
}

void CMqttAdapter::Start()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    MQTTClient_connectOptions connectOptions = MQTTClient_connectOptions_initializer;
    connectOptions.keepAliveInterval = 60;
    connectOptions.cleansession = 1;

    int returnCode = MQTTClient_connect(m_Client, &connectOptions);
    if(returnCode != MQTTCLIENT_SUCCESS)
    {
        Logger.Error << "MQTT Client Connection Failed with Return Code = " << returnCode << std::endl;
        throw std::runtime_error("Failed to connect to the MQTT Broker");
    }

    // TODO - subscribe the join and leave channels
}

void CMqttAdapter::Stop()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    MQTTClient_disconnect(m_Client, 2000);
}

SignalValue CMqttAdapter::GetState(const std::string device, const std::string key) const
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    return 0;
}

void CMqttAdapter::SetCommand(const std::string device, const std::string key, const SignalValue value)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
}

void CMqttAdapter::ConnectionLost(void * id, char * reason)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    Logger.Error << "MQTT Client " << (char *)id << " lost connection to broker" << std::endl;
    throw std::runtime_error("Lost Connection to the MQTT Broker");
}

int CMqttAdapter::HandleMessage(void * id, char * topic, int topicLen, MQTTClient_message * msg)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    std::string strId((char *)id);
    std::string strTopic(topic, topicLen);
    std::string message((char *)msg->payload, msg->payloadlen);
    if(CAdapterFactory::Instance().m_adapters.count(strId) > 0)
    {
        Pointer client = boost::dynamic_pointer_cast<CMqttAdapter>(CAdapterFactory::Instance().m_adapters[strId]);
        client->HandleMessage(strTopic, message);
    }
    else
    {
        Logger.Warn << "Dropped message for missing MQTT adapter with identifier " << strId << std::endl;
    }
    MQTTClient_freeMessage(&msg);
    MQTTClient_free(topic);
    return 1;
}

void CMqttAdapter::HandleMessage(std::string topic, std::string message)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
}

}//namespace broker
}//namespace freedm
}//namespace device

