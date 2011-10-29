////////////////////////////////////////////////////////////////////
/// @file CPSCADDevice.hpp
///
/// @author Yaxi Liu <ylztf@mst.edu>
///
/// @compiler C++
///
/// @project FREEDM DGI
///
/// @description Implements the CPSCADDevice class. 
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
/// Computer Science, Missouri University of Science and
/// Technology, Rolla, MO 65409 (ff@mst.edu).
///
////////////////////////////////////////////////////////////////////
#ifndef CPSCADDEVICE_HPP
#define CPSCADDEVICE_HPP

#include <string>
#include <boost/noncopyable.hpp>
#include "IPhysicalDevice.hpp"
#include "CPhysicalDeviceManager.hpp"
#include "CLineClient.hpp"
#include "PhysicalDeviceTypes.hpp"

namespace freedm {
    namespace broker {

		class CPSCADDevice
			: public IPhysicalDevice
		{
		protected:
			/// Constructor
			CPSCADDevice(CLineClient::TPointer lineClient, CPhysicalDeviceManager& phymanager, Identifier deviceid = "pscad", DeviceType devtype =  physicaldevices::FREEDM_GENERIC);  
         
			/// Pulls the setting of some key from PSCAD.
			SettingValue Get(SettingKey key);

			/// Sets the value of some key to PSCAD.
			void Set(SettingKey key, SettingValue value);
    
		private:
			///pointer to lineClient
			CLineClient::TPointer m_lineClient;
		};

	} // namespace broker
} // namespace freedm

#endif // CPSCADDEVICE_HPP


