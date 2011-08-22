////////////////////////////////////////////////////////////////////////////////
/// @file           CSimulationInterface.hpp
///
/// @author         Thomas Roth <tprfh7@mst.edu>
///
/// @compiler       C++
///
/// @project        Missouri S&T Power Research Group
///
/// @see            CSimulationInterface.hpp
///
/// These source code files were created at the Missouri University of Science
/// and Technology, and are intended for use in teaching or research. They may
/// be freely copied, modified and redistributed as long as modified versions
/// are clearly marked as such and this notice is not removed.
///
/// Neither the authors nor Missouri S&T make any warranty, express or implied,
/// nor assume any legal responsibility for the accuracy, completeness or
/// usefulness of these files or any information distributed with these files.
///
/// Suggested modifications or questions about these files can be directed to
/// Dr. Bruce McMillin, Department of Computer Science, Missouri University of
/// Science and Technology, Rolla, MO 65401 <ff@mst.edu>.
///
////////////////////////////////////////////////////////////////////////////////

#include "CSimulationInterface.hpp"

CSimulationInterface::CSimulationInterface( boost::asio::io_service & p_service,
    CDeviceTable & p_command, CDeviceTable & p_state, unsigned short p_port, size_t p_index )
    : m_command(p_command), m_state(p_state), m_index(p_index)
{
    m_server = CLineServer::Create( p_service, p_port,
        boost::bind(&CSimulationInterface::Set, boost::ref(*this), _1, _2, _3),
        boost::bind(&CSimulationInterface::Get, boost::ref(*this), _1, _2) );
}

void CSimulationInterface::Set( const std::string & p_device, const std::string & p_key, const std::string & p_value )
{
    double value = boost::lexical_cast<double>(p_value);
    m_command.SetValue( CDeviceKey(p_device,p_key), m_index, value );
}

std::string CSimulationInterface::Get( const std::string & p_device, const std::string & p_key )
{
    double value = m_state.GetValue( CDeviceKey(p_device,p_key), m_index );
    return boost::lexical_cast<std::string>(value);
}
