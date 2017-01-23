////////////////////////////////////////////////////////////////////////////////
/// @file         CMqttAdapter.hpp
///
/// @author       Thomas Roth <tprfh7@mst.edu>
///
/// @project      FREEDM DGI
///
/// @description  Implementation of an asynchronous PAHO MQTT client in DGI.
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

#ifndef C_MQTT_ADAPTER_HPP
#define C_MQTT_ADAPTER_HPP

#include "IAdapter.hpp"
#include "CMqttMessage.hpp"

#include <map>
#include <list>
#include <string>

#include <MQTTClient.h>
#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/enable_shared_from_this.hpp>

namespace freedm {
namespace broker {
namespace device {

/// Provides an interface for communicating with an MQTT broker.
////////////////////////////////////////////////////////////////////////////////
/// This class handles communication with physical devices through MQTT. It
/// implements an asynchronous MQTT client that subscribes to a join channel to
/// listen for the appearance of new plug-and-play devices. When a new device
/// joins, the adapter subscribes to that device to receive its state changes.
///
/// @peers The MQTT client will communicate with the MQTT broker specified in
///     the adapter configuration file. Through the broker, the client will
///     send and receive data with physical devices.
///
/// @limitations None.
////////////////////////////////////////////////////////////////////////////////
class CMqttAdapter
    : public IAdapter
    , public boost::enable_shared_from_this<CMqttAdapter>
{
public:
    /// Pointer to a CMqttAdapater object.
    typedef boost::shared_ptr<CMqttAdapter> Pointer;

    /// Create a CMqttAdapter object and return a point to it.
    static IAdapter::Pointer Create(std::string id, std::string address);

    /// Start the MQTT client.
    void Start();

    /// Stop the MQTT client.
    void Stop();

    /// Retrieves a value from a device.
    SignalValue GetState(const std::string device, const std::string key) const;

    /// Sets a value on a device.
    void SetCommand(const std::string device, const std::string key, const SignalValue value);

    /// Destructor.
    ~CMqttAdapter();

private:
    struct DeviceData
    {
        std::map<std::string, SignalValue> s_SignalToValue;
        std::map<std::string, std::string> s_IndexReference;
    };

    /// Constructor.
    CMqttAdapter(std::string id, std::string address);

    /// Callback function when the client cannot reach the broker.
    static void ConnectionLost(void * id, char * reason);

    /// Callback function when the client receives a message about a subscribed topic.
    static int HandleMessage(void * id, char * topic, int topicLen, MQTTClient_message * msg);

    /// Callback function when a message has been delivered to the broker.
    static void DeliveryComplete(void * id, MQTTClient_deliveryToken token);

    /// Handles messages received from subscribed topics.
    void HandleMessage(std::string topic, std::string message);
    
    /// Publish a message on a topic to the MQTT broker.
    void Publish(std::string topic, std::string content);

    /// does a full Subscribe //ceasar
    void SubscribeAll(std::string deviceName);

    /// does a full UnSubscribe //ceasar
    void UnsubscribeAll(std::string deviceName);

    /// Create a CDevice object from a JSON specification.
    void CreateDevice(std::string deviceName, std::string json);

    void AddSignals(std::string device, boost::property_tree::ptree::value_type & ptree, std::set<std::string> & sigset, std::set<std::string> & type);

    /// Map from a device name to its stored signal values.
    typedef std::map<std::string, DeviceData> TDeviceToData;

    /// Map of device data expected over MQTT.
    TDeviceToData m_DeviceData;

    /// Storage for MQTT messages
    std::list<CMqttMessage::Pointer> m_MessageQueue;

    /// Protect the device data map.
    mutable boost::mutex m_DeviceDataLock;

    /// MQTT client object.
    MQTTClient m_Client;
    MQTTClient_message pubmsg;
    MQTTClient_deliveryToken token;

    /// MQTT client identifier.
    char m_ID[24];
};

} //namespace device
} //namespace broker
} //namespace freedm

#endif // C_MQTT_ADAPTER_HPP

