///////////////////////////////////////////////////////////////////////////////
/// @file CLoadDevice.cpp
///
/// @author Yaxi Liu <ylztf@mst.edu>
///
/// @compiler C++
///
/// @project FREEDM DGI
///
/// @description Manages the solar panels
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

#include "CLoadDevice.hpp"

namespace freedm {
	namespace broker {

		///////////////////////////////////////////////////////////////////////////////
		/// @fn CLoadDevice
		/// @brief  constructor. The device type is always LOAD.
		/// @param phymanager The related physical device manager.
		/// @param deviceid The identifier for this generic device.
		/// @param lineClient  the client that connects to the PSCAD interface
		///////////////////////////////////////////////////////////////////////////////
		CLoadDevice::CLoadDevice(CLineClient::TPointer lineClient, CPhysicalDeviceManager& phymanager, IPhysicalDevice::Identifier deviceid)  
			: CPSCADDevice(lineClient, phymanager, deviceid, physicaldevices::LOAD)
		{};

		/////////////////////////////////////////////////////////////////////////////
		/// @fn GET_POWERLEVEL
		/// @brief get the power level of the load.  
		/// @return  the power level read from PSCAD simulation. 
		///////////////////////////////////////////////////////////////////////////////
		CLoadDevice:: SettingValue CLoadDevice::get_powerLevel()
		{
			CPSCADDevice::Get("powerLevel");
		}

		/////////////////////////////////////////////////////////////////////////////
		/// @fn turnOn
		/// @brief turn load on
		///////////////////////////////////////////////////////////////////////////////
		void CLoadDevice::turnOn()
		{
			CPSCADDevice::Set("onOffSwitch", 1);
		}

		/////////////////////////////////////////////////////////////////////////////
		/// @fn turnOff
		/// @brief turn load off
		///////////////////////////////////////////////////////////////////////////////
		void CLoadDevice::turnOff()
		{
			CPSCADDevice::Set("onOffSwitch", 0);
		}

    } // namespace broker
} // namespace freedm



