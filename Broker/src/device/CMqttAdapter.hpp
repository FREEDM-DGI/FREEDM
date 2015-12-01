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

#include <map>
#include <list>
#include <string>

#include <MQTTClient.h>
#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>
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
    /// Store data relevant to published messages between function calls.
    struct MQTTData
    {
        /// The command sent to the device.
        float s_payload;
        /// The message sent over MQTT.
        MQTTClient_message s_message;
        /// The token associated with the message.
        MQTTClient_deliveryToken s_token;
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
    void Publish(std::string topic, float value);

    /// Map from a device signal name to its stored value.
    typedef std::map<std::string, SignalValue> TSignalToValue;

    /// Map from a device name to its stored signal values.
    typedef std::map<std::string, TSignalToValue> TDeviceToData;

    /// Map of device data expected over MQTT.
    mutable TDeviceToData m_DeviceData;

    /// Storage for MQTT messages
    std::list<MQTTData> m_MessageQueue;

    /// Protect the device data map.
    boost::mutex m_DeviceDataLock;

    /// MQTT client object.
    MQTTClient m_Client;

    /// MQTT client identifier.
    char m_ID[24];
};

} //namespace device
} //namespace broker
} //namespace freedm

#endif // C_MQTT_ADAPTER_HPP

