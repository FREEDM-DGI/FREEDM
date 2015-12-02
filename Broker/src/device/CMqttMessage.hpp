////////////////////////////////////////////////////////////////////////////////
/// @file         CMqttMessage.hpp
///
/// @author       Thomas Roth <tprfh7@mst.edu>
///
/// @project      FREEDM DGI
///
/// @description  Stores data associated with a single MQTT message.
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

#ifndef C_MQTT_MESSAGE_HPP
#define C_MQTT_MESSAGE_HPP

#include <string>

#include <MQTTClient.h>
#include <boost/shared_ptr.hpp>

namespace freedm {
namespace broker {
namespace device {

class CMqttMessage
{
public:
    typedef boost::shared_ptr<CMqttMessage> Pointer;

    static Pointer Create(std::string content);

    MQTTClient_message & GetMessage() { return m_Message; }

    MQTTClient_deliveryToken & GetToken() { return m_Token; }

    ~CMqttMessage();
private:
    CMqttMessage(std::string content);

    char * m_Payload;

    MQTTClient_message m_Message;

    MQTTClient_deliveryToken m_Token;
};

} //namespace device
} //namespace broker
} //namespace freedm

#endif // C_MQTT_MESSAGE_HPP


