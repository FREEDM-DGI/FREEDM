////////////////////////////////////////////////////////////////////////////////
/// @file         IDGIModule.cpp
///
/// @author       Michael Catanzaro <michael.catanzaro@mst.edu>
///
/// @project      FREEDM DGI
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

#include "IDGIModule.hpp"
#include "CGlobalConfiguration.hpp"

namespace freedm {

namespace broker {

///////////////////////////////////////////////////////////////////////////////
/// IDGIModule
/// @description Constructor for an IDGIModule. Gets the uuid from
///  CGlobalConfiguration and makes a CPeerNode referencing it.
/// @pre CGlobalConfiguration is loaded.
/// @post m_me is created.
/////////////////////////////////////////////////////////////////////////////// 
IDGIModule::IDGIModule()
    : m_me(CGlobalConfiguration::Instance().GetUUID())
{
    //Pass
}
///////////////////////////////////////////////////////////////////////////////
/// GetUUID
/// @description Gets this process's UUID.
/// @return This process's UUID
///////////////////////////////////////////////////////////////////////////////
std::string IDGIModule::GetUUID() const 
{
    return m_me.GetUUID();
}
///////////////////////////////////////////////////////////////////////////////
/// GetMe
/// @description Gets a CPeerNode that refers to this process.
/// @return A CPeerNode referring to this process.
///////////////////////////////////////////////////////////////////////////////
CPeerNode IDGIModule::GetMe()
{
    return m_me;
}

} // namespace freedm

} // namespace broker

