////////////////////////////////////////////////////////////////////////////////
/// @file       IPhysicalAdapter.hpp
///
/// @author     Thomas Roth <tprfh7@mst.edu>,
///             Yaxi Liu <ylztf@mst.edu>,
///             Mark Stanovich <stanovic@cs.fsu.edu>,
///             Michael Catanzaro <michael.catanzaro@mst.edu>
///
/// @project    FREEDM DGI
///
/// @description
///     Physical device adapter interface. Each device contains a reference to
///     an adapter that it uses to perform all operations. The adapter is, in
///     effect, the device's "driver". However, the same adapter can (and
///     currently should) be used for all devices in the simulation.
///
/// @copyright
///     These source code files were created at Missouri University of Science
///     and Technology, and are intended for use in teaching or research. They
///     may be freely copied, modified, and redistributed as long as modified
///     versions are clearly marked as such and this notice is not removed.
///     Neither the authors nor Missouri S&T make any warranty, express or
///     implied, nor assume any legal responsibility for the accuracy,
///     completeness, or usefulness of these files or any information
///     distributed with these files. 
///     
///     Suggested modifications or questions about these files can be directed
///     to Dr. Bruce McMillin, Department of Computer Science, Missouri
///     University of Science and Technology, Rolla, MO 65409 <ff@mst.edu>.
////////////////////////////////////////////////////////////////////////////////

#ifndef IPHYSICALADAPTER_HPP
#define	IPHYSICALADAPTER_HPP

class IPhysicalAdapter : private boost::noncopyable {
    /// pointer to an adapter
    typedef boost::shared_ptr<CRtdsAdapter> RTDSPointer;

    /// handles connection to FPGA
    virtual void Connect(const std::string p_hostname, const std::string p_port);

    /// updates command table
    virtual void Set(const std::string p_device, const std::string p_key,
            const double p_value);

    /// retrieve data from state table
    virtual double Get(const std::string p_device, const std::string p_key);

    /// shut down communicaiton to FPGA
    virtual void Quit();
};

#endif	// IPHYSICALADAPTER_HPP
