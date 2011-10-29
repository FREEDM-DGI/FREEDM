///////////////////////////////////////////////////////////////////////////////
/// @file CPSCADDevice.cpp
///
/// @author Yaxi Liu <ylztf@mst.edu>
///
/// @compiler C++
///
/// @project FREEDM DGI
///
/// @description implement the PSCAD device
///
/// @license
/// These source code files were created at as part of the
/// FREEDM DGI Subthrust, and are
/// intended for use in teaching or research. They may be
/// freely copied, modified and redistributed as long
/// as modified versions are clearly marked as such and
/// this notice is not removed.
///
/// Neither the authors nor the FREEDM Project nor the
/// National Science Foundation
/// make any warranty, express or implied, nor assumes
/// any legal responsibility for the accuracy,
/// completeness or usefulness of these codes or any
/// information distributed with these codes.
///
/// Suggested modifications or questions about these codes
/// can be directed to Dr. Bruce McMillin, Department of
/// Computer Science, Missour University of Science and
/// Technology, Rolla, MO 65409 (ff@mst.edu).
///////////////////////////////////////////////////////////////////////////////

#include "CPSCADDevice.hpp"
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/lexical_cast.hpp>

namespace freedm {
	namespace broker {
		///////////////////////////////////////////////////////////////////////////////
		/// @fn CPSCADDevice
		/// @brief constructor
		/// @param phymanager The related physical device manager.
		/// @param client The LineClient that connectsto PSCAD interface
		/// @param deviceid The identifier for this device.
		///////////////////////////////////////////////////////////////////////////////
		CPSCADDevice::CPSCADDevice(CLineClient::TPointer lineClient, CPhysicalDeviceManager& phymanager, Identifier deviceid, DeviceType devtype)
			: m_lineClient(lineClient), IPhysicalDevice(phymanager, deviceid, devtype) 
		{};

		///////////////////////////////////////////////////////////////////////////////
		/// @fn CPSCADDevice::Get
		/// @brief Returns the value of some key from readings from PSCAD
		/// @param key The key to retrieve from PSCAD
		/// @return The value from PSCAD
		///////////////////////////////////////////////////////////////////////////////
		CPSCADDevice::SettingValue CPSCADDevice::Get(SettingKey key)
		{
			std::string response = m_lineClient->Get(m_devid, key);
			return boost::lexical_cast<double>(response);
		};

		///////////////////////////////////////////////////////////////////////////////
		/// @fn CPSCADDevice::Set
		/// @brief Sets the value of some key to to a new value and send to PSCAD
		/// @param key The key to change.
		/// @param value The value to set the key to.
		///////////////////////////////////////////////////////////////////////////////
		void CPSCADDevice::Set(SettingKey key, SettingValue value)
		{
			std::string valueInString = boost::lexical_cast<std::string>(value);
			m_lineClient->Set(m_devid, key, valueInString);
		};

    } // namespace broker
} // namespace freedm

