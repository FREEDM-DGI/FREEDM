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
#include "CDeviceManager.hpp"
#include "CGlobalConfiguration.hpp"
#include "CLogger.hpp"

#include <sstream>
#include <stdexcept>

#include <boost/foreach.hpp>
#include <boost/optional.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/pointer_cast.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

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

    if(id.size() > 23)
        throw std::runtime_error("MQTT Client ID contains more than 23 characters");
    std::size_t length = id.copy(m_ID, 23);
    m_ID[length] = '\0';

    if(MQTTClient_create(&m_Client, address.c_str(), id.c_str(), MQTTCLIENT_PERSISTENCE_NONE, NULL) != MQTTCLIENT_SUCCESS)
        throw std::runtime_error("Failed to create the MQTT Client object");

    if(MQTTClient_setCallbacks(m_Client, &m_ID, ConnectionLost, HandleMessage, DeliveryComplete) != MQTTCLIENT_SUCCESS)
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
    // TODO - subscribe function for strings
    if(MQTTClient_subscribe(m_Client, "join/#", 2) != MQTTCLIENT_SUCCESS)
        throw std::runtime_error("MQTT failed to subscribe to join/#");
    if(MQTTClient_subscribe(m_Client, "leave/#", 2) != MQTTCLIENT_SUCCESS)
        throw std::runtime_error("MQTT failed to subscribe to leave/#");
    BOOST_FOREACH(std::string subscription, CGlobalConfiguration::Instance().GetMQTTSubscriptions())
    {
        std::string topic;
        topic = subscription + "/+/JSON";
        MQTTClient_subscribe(m_Client, topic.c_str(), 2);
        Logger.Notice << "Subscribed to MQTT topic " << topic << std::endl;
        topic = subscription + "/+/AOUT";
        MQTTClient_subscribe(m_Client, topic.c_str(), 2);
        Logger.Notice << "Subscribed to MQTT topic " << topic << std::endl;
        topic = subscription + "/+/DOUT";
        MQTTClient_subscribe(m_Client, topic.c_str(), 2);
        Logger.Notice << "Subscribed to MQTT topic " << topic << std::endl;
    }
}

void CMqttAdapter::Stop()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    MQTTClient_disconnect(m_Client, 2000);
}

SignalValue CMqttAdapter::GetState(const std::string device, const std::string key) const
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    boost::lock_guard<boost::mutex> lock(m_DeviceDataLock);
    if(m_DeviceData.count(device) != 1)
    {
        Logger.Error << "Device " << device << " does not exist as an MQTT device" << std::endl;
        throw std::runtime_error("Invalid Device Name");
    }
    if(m_DeviceData.at(device).s_SignalToValue.count(key) != 1)
    {
        Logger.Error << "Device " << device << " does not have the signal " << key << std::endl;
        throw std::runtime_error("Invalid Device Signal");
    }
    SignalValue value = m_DeviceData.at(device).s_SignalToValue.at(key);
    Logger.Debug << device << " " << key << ": " << value << std::endl;
    return value;
}

void CMqttAdapter::SetCommand(const std::string device, const std::string key, const SignalValue value)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    boost::lock_guard<boost::mutex> lock(m_DeviceDataLock);
    if(m_DeviceData.count(device) != 1)
    {
        Logger.Error << "Device " << device << " does not exist as an MQTT device" << std::endl;
        throw std::runtime_error("Invalid Device Name");
    }
    if(m_DeviceData[device].s_SignalToValue.count(key) != 1)
    {
        Logger.Error << "Device " << device << " does not have the signal " << key << std::endl;
        throw std::runtime_error("Invalid Device Signal");
    }
    m_DeviceData[device].s_SignalToValue[key] = value;
    std::string strIndex = m_DeviceData[device].s_IndexReference.at(key);
    Publish(device + "/" + strIndex, boost::lexical_cast<std::string>(value));
    Logger.Info << "Sent Command " << device << "/" << strIndex << " = " << value << std::endl;
}

void CMqttAdapter::ConnectionLost(void * id, char * reason)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    Logger.Error << "MQTT Client " << (char *)id << " lost connection to broker: " << reason << std::endl;
    throw std::runtime_error("Lost Connection to the MQTT Broker");
}

int CMqttAdapter::HandleMessage(void * id, char * topic, int topicLen, MQTTClient_message * msg)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    std::string strId((char *)id);
    std::string strTopic(topic);
    std::string message((char *)msg->payload, msg->payloadlen);
    if(topicLen != 0)
    {
        Logger.Warn << "Dropped byte array topic for MQTT adapter with identifier " << strId << std::endl;
    }
    else if(CAdapterFactory::Instance().m_adapters.count(strId) > 0)
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

void CMqttAdapter::DeliveryComplete(void * id, MQTTClient_deliveryToken token)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    std::string strId((char *)id);
    std::list<CMqttMessage::Pointer>::iterator it;
    Pointer client = boost::dynamic_pointer_cast<CMqttAdapter>(CAdapterFactory::Instance().m_adapters.at(strId));
    for(it = client->m_MessageQueue.begin(); it != client->m_MessageQueue.end() && (*it)->GetToken() != token; it++);
    if(it == client->m_MessageQueue.end())
    {
        Logger.Error << "MQTT client " << strId << " does not recognize the token " << token << std::endl;
        throw std::runtime_error("Unrecognized Delivery Token");
    }
    else
    {
        Logger.Info << "MQTT client " << strId << " has delivered message " << token << std::endl;
        client->m_MessageQueue.erase(it);
    }
}

void CMqttAdapter::HandleMessage(std::string topic, std::string message)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    std::size_t index;

    if(topic.compare(0,4,"join") == 0)
    {
        std::string deviceName = topic.substr(5);
        Logger.Status << "Received a join message for device: " << deviceName << std::endl;
        boost::lock_guard<boost::mutex> lock(m_DeviceDataLock);
        if(m_DeviceData.count(deviceName) == 0)
        {
/*
            std::string subscription = deviceName + "/#";
            if(MQTTClient_subscribe(m_Client, subscription.c_str(), 2) != MQTTCLIENT_SUCCESS)
            {
                Logger.Error << "Failed to subscribe to the topic " << subscription << std::endl;
                throw std::runtime_error("MQTT Subscription Failure");
            }
*/
            Publish(deviceName + "/ACK", "ACK");
            m_DeviceData[deviceName];
        }
        else
        {
            Logger.Status << "Dropped duplicate join message for device " << deviceName << std::endl;
        }
    }
    else if(topic.compare(0,5,"leave") == 0)
    {
        std::string deviceName = topic.substr(6);
        Logger.Status << "Received a leave message for device: " << deviceName << std::endl;
        boost::lock_guard<boost::mutex> lock(m_DeviceDataLock);
        if(m_DeviceData.count(deviceName) > 0 )
        {
/*
            std::string subscription = deviceName + "/#";
            if(MQTTClient_unsubscribe(m_Client, subscription.c_str()) != MQTTCLIENT_SUCCESS)
            {
                Logger.Error << "Failed to unsubscribe to the topic " << subscription << std::endl;
                throw std::runtime_error("MQTT Subscription Failure");
            }
*/
            CDeviceManager::Instance().RemoveDevice(deviceName);
            m_DeviceData.erase(deviceName);
        }
        else
        {
            Logger.Status << "Dropped leave message for unknown device " << deviceName << std::endl;
        }
    }
    else if(topic.compare(topic.size()-4,4,"JSON-DGI") == 0)
    {
        std::string deviceName = topic.substr(0, topic.size()-5);
        Logger.Status << "Received JSON for device " << deviceName << ":\n" << message << std::endl;
        CreateDevice(deviceName, message);
    }
    else if((index = topic.find("/AOUT/")) != std::string::npos || (index = topic.find("/DOUT/")) != std::string::npos)
    {
        std::string device = topic.substr(0, index);
        std::string signal = topic.substr(index+1);
        float value = boost::lexical_cast<float>(message);

        try
        {
            boost::lock_guard<boost::mutex> lock(m_DeviceDataLock);
            signal = m_DeviceData.at(device).s_IndexReference.at(signal);
            m_DeviceData.at(device).s_SignalToValue.at(signal) = value;
        }
        catch(std::exception & e)
        {
            Logger.Warn << "Device Signal (" << device << "," << signal << ") does not exist" << std::endl;
        }
    }
    else
    {
        Logger.Warn << "Dropped MQTT Message:\n" << topic << "\n" << message << std::endl;
    }
}

void CMqttAdapter::Publish(std::string topic, std::string content)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    CMqttMessage::Pointer msg = CMqttMessage::Create(topic, content);
    m_MessageQueue.push_back(msg);
    msg->Publish(m_Client);
}

void CMqttAdapter::CreateDevice(std::string deviceName, std::string json)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    boost::property_tree::ptree propertyTree;
    std::istringstream inputStream(json);
    read_json(inputStream, propertyTree);

    DeviceInfo devinfo;
    BOOST_FOREACH(boost::property_tree::ptree::value_type & property, propertyTree)
    {
        if(property.first == "DEV_CHAR" || property.first == "AOUT" || property.first == "DOUT")
        {
            AddSignals(deviceName, property, devinfo.s_state, devinfo.s_type);
        }
        else if(property.first == "AIN" || property.first == "DIN")
        {
            AddSignals(deviceName, property, devinfo.s_command, devinfo.s_type);
        }
        else
        {
            Logger.Info << "Skipped property " << property.first << std::endl;
        }
    }
    CDevice::Pointer device = CDevice::Pointer(new CDevice(deviceName, devinfo, shared_from_this()));
    CDeviceManager::Instance().AddDevice(device);
    CDeviceManager::Instance().RevealDevice(deviceName);
}

void CMqttAdapter::AddSignals(std::string device, boost::property_tree::ptree::value_type & ptree, std::set<std::string> & sigset, std::set<std::string> & type)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    boost::lock_guard<boost::mutex> lock(m_DeviceDataLock);
    Logger.Info << "Parsing the " << ptree.first << " field of the JSON for device " << device << std::endl;
    BOOST_FOREACH(boost::property_tree::ptree::value_type & signal, ptree.second)
    {
        std::string name, index;
        float value;
        boost::optional<float> min, max;

        try
        {
            name = signal.second.get<std::string>("name");
            if(name == "Dev_Name")
            {
                std::string strval = signal.second.get<std::string>("value");
                type.insert(strval);
                Logger.Info << "Classified device " << device << " as type " << strval << std::endl;
                continue;
            }
            name = ptree.first + "/" + name;
            index = ptree.first + "/" + signal.second.get<std::string>("index");
            value = signal.second.get<float>("value");
            min = signal.second.get_optional<float>("minimum");
            max = signal.second.get_optional<float>("maximum");

            sigset.insert(name);
            m_DeviceData[device].s_SignalToValue[name] = value;
            m_DeviceData[device].s_IndexReference[name] = index;
            m_DeviceData[device].s_IndexReference[index] = name;
            Logger.Info << "Stored (" << index << "," << name << ") = " << value << std::endl;
            if(min)
            {
                sigset.insert(name + "_minimum");
                m_DeviceData[device].s_SignalToValue[name + "_minimum"] = min.get();
                Logger.Info << "Set its minimum value to " << min.get() << std::endl;
            }
            if(max)
            {
                sigset.insert(name + "_maximum");
                m_DeviceData[device].s_SignalToValue[name + "_maximum"] = max.get();
                Logger.Info << "Set its maximum value to " << max.get() << std::endl;
            }
        }
        catch(boost::property_tree::ptree_bad_data & e)
        {
            // this happens when value cannot be set as the float conversion fails
            Logger.Warn << "Dropped field " << name << " due to non-numeric type" << std::endl;
        }
        catch(std::exception & e)
        {
            Logger.Error << "Unexpected format for field " << ptree.first << std::endl;
            throw std::runtime_error("Bad Device JSON");
        }
    }
}

}//namespace broker
}//namespace freedm
}//namespace device

