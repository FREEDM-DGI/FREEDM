////////////////////////////////////////////////////////////////////////////////
/// @file         CTimings.hpp
///
/// @author       Stephen Jackson <scj7t4@mst.edu>
///
/// @project      FREEDM DGI
///
/// @description  Global list of timer values for modules, loaded from a 
///     configuration file.
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
#include <string>
#include <map>

#include <boost/program_options.hpp>
#include <boost/program_options/options_description.hpp>

#ifndef CTIMINGS_HPP
#define CTIMINGS_HPP
namespace po = boost::program_options;

namespace freedm {
namespace broker {

class CTimings
{
public:
    /// Loads timings values from the specified file
    static void SetTimings(const std::string timingsFile);
    /// Returns the value of the specified timing parameter
    static unsigned int Get(const std::string param);
private:
    /// Typedef for timing datastore.
    typedef std::map<std::string, unsigned int> TimingMap;
    /// Typedef for timing datastore iterator
    typedef TimingMap::iterator TimingMapIterator;
    /// Registers all the expected timing parameters
    static void TimingParameters(po::options_description& opts);
    /// Adds individual parameter to the expected options
	static void RegisterTimingValue(po::options_description&, const std::string param);
    /// Data store for the timing parameter values
    static TimingMap timing_values;

};


}
}

#endif
