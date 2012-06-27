///////////////////////////////////////////////////////////////////////////////
/// @file         CAdapter.cpp
///
/// @author       Thomas Roth <tprfh7@mst.edu>
///
/// @project      FREEDM DGI
///
/// @description  Encapsulates the XML specification of an adapter
///
/// @copyright
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
/// Science and Technology, Rolla, MO 65409 <ff@mst.edu>.
///////////////////////////////////////////////////////////////////////////////

#include "CAdapter.hpp"
#include "DeviceTable.hpp"
#include "CTableManager.hpp"

#include <set>
#include <string>
#include <stdexcept>

#include <boost/foreach.hpp>
#include <boost/optional/optional.hpp>
#include <boost/property_tree/ptree.hpp>

namespace freedm {
namespace simulation {
namespace adapter {

namespace // unnamed
{
    /// local logger for this file
    CLocalLogger Logger(__FILE__);
}

///////////////////////////////////////////////////////////////////////////////
/// Constructs a base adapter from the property tree specification.
/// @pre The property tree must follow the format in CAdapter::ReadDetails().
/// @post Parses the property tree to construct its member variables.
/// @limitations None.
///////////////////////////////////////////////////////////////////////////////
CAdapter::CAdapter( const boost::property_tree::ptree & tree )
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    
    ReadDetails(tree,STATE_TABLE,m_StateDetails);
    ReadDetails(tree,COMMAND_TABLE,m_CommandDetails);
}

///////////////////////////////////////////////////////////////////////////////
/// Parses a subtree of the property tree to construct the passed vector.
/// @Peers This call will obtain a unique write lock on the device table with
/// the passed name argument.
/// @ErrorHandling This call will throw a std::logic_error when any of the
/// stated preconditions are violated.
/// @pre The property tree must contain a subtree of the given name.
/// @pre The subtree must have the children <device> and <signal>.
/// @pre The subtree must have an attribute for its index.
/// @pre The index attributes must be unique, consecutive, and start with 1.
/// @pre The <device> and <signal> attributes must form a unique pair.
/// @post Obtains a write lock on the device table with the passed identifier.
/// @post Inserts elements into the device table based on the specification.
/// @post Modifies the passed vector to contain the specification details.
/// @param tree The property tree that contains the adapter specification.
/// @param name The name of the subtree to parse and the table identifier.
/// @param details The vector to store the specifications details.
/// @limitations The name argument is used both for the XML tag that stores the
/// specification as well as the name of the device table associated with this
/// adapter.  An alternative would be to read the table name from an attribute
/// in the specification, but we already use that tag to refer to the table and
/// there is no present reason to support that flexibility.
///////////////////////////////////////////////////////////////////////////////
void CAdapter::ReadDetails( const boost::property_tree::ptree & tree,
        const char name[], std::vector<CDeviceSignal> & details )
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    typedef std::vector<CDeviceSignal>::size_type TIndex;
    
    boost::property_tree::ptree subtree;
    std::set<CDeviceSignal> devsigset;
    TSignalValue currentValue;
    CDeviceSignal blank;
    
    TIndex index;
    std::string device, signal;
    boost::optional<TSignalValue> value;
    
    try
    {
        Logger.Info << "Reading the " << name << " subtree." << std::endl;
        subtree = tree.get_child(name);
        details.resize(subtree.size());
    }
    catch( std::exception & e )
    {
        Logger.Warn << "Failed to parse the XML subtree " << name << ": "
                    << e.what() << std::endl;
        throw std::logic_error("Bad XML Specification");
    }
    
    CTableManager::TWriter lock = CTableManager::AsWriter(name);
    
    // this needs a proof to show that each index is certain to be used
    BOOST_FOREACH( boost::property_tree::ptree::value_type & child, subtree )
    {
        Logger.Info << "Parsing the next child of the subtree." << std::endl;
        try
        {
            index   = child.second.get<TIndex>("<xmlattr>.index");
            device  = child.second.get<std::string>("device");
            signal  = child.second.get<std::string>("signal");
            value   = child.second.get_optional<TSignalValue>("value");
        }
        catch( std::exception & e )
        {
            Logger.Warn << "Failed to parse child of " << name << " subtree:"
                        << e.what() << std::endl;
            throw std::logic_error("Bad XML Specification");
        }
        
        CDeviceSignal devsig(device,signal);
        Logger.Debug << "Index=" << index << ", DeviceSignal=" << devsig
                     << ", Value=" << (value ? value.get() : 0) << std::endl;
        
        if( index == 0 || index > details.size() )
        {
            Logger.Warn << "The specified table index " << index
                        << " is either 0 or larger than the expected size ("
                        << details.size() << ")." << std::endl;
            throw std::logic_error("Invalid Index");
        }
        if( details[index-1] < blank || blank < details[index-1] )
        {
            Logger.Warn << "The table index " << index << " appears more than"
                        << " once in the specification file." << std::endl;
            throw std::logic_error("Duplicate Index");
        }
        if( device.empty() || signal.empty() )
        {
            Logger.Warn << "At least one element of the specification file has"
                        << " an empty <device> or <signal> tag." << std::endl;
            throw std::logic_error("Invalid Device Signal");
        }
        if( devsigset.insert(devsig).second == false )
        {
            Logger.Warn << "The device signal " << devsig << " appears more"
                        << " than once in the specification file." << std::endl;
            throw std::logic_error("Duplicate Device Signal");
        }
        
        details[index-1] = devsig;
        lock->InsertDeviceSignal(devsig);
        currentValue = lock->GetValue(devsig);
        Logger.Info << "Added " << devsig << " to " << name
                    << " table." << std::endl;
        
        if( value )
        {
            if( currentValue != TSignalValue() && currentValue != value.get() )
            {
                Logger.Warn << "The initial value for " << devsig << " is set"
                            << " more than once to different values in the"
                            << " specification file." << std::endl;
                throw std::logic_error("Duplicate Initial Value");
            }
            lock->SetValue(devsig,value.get());
            Logger.Info << "Set the initial value " << devsig << "="
                        << lock->GetValue(devsig) << std::endl;
        }
    }
}

} // namespace adapter
} // namespace simulation
} // namespace freedm
