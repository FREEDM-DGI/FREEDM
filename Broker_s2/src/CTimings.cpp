////////////////////////////////////////////////////////////////////////////////
/// @file         CTimings.cpp
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
#include "CTimings.hpp"
#include "CLogger.hpp"
#include "FreedmExceptions.hpp"

#include <fstream>

#include <boost/foreach.hpp>
#include <boost/range/adaptor/map.hpp>


namespace freedm {
namespace broker {

namespace {

CLocalLogger Logger(__FILE__);

}

CTimings::TimingMap CTimings::timing_values;

///////////////////////////////////////////////////////////////////////////////
/// CTimings::TimingParameters
/// @description Registers all the expected timing values with the
///     configuration file loader. If a programmer needs to add timing values,
///     they should make additional calls to RegisterTimingValue in this
///     function, which will add their value to the DGI.
/// @pre None
/// @post The program options have been modified to include the new timings
///     options added by this function
/// @param opts The program options, configuration file processor, that will
///     be used to load this values.
///////////////////////////////////////////////////////////////////////////////
void CTimings::TimingParameters(po::options_description& opts)
{

    RegisterTimingValue(opts,"GM_AYC_RESPONSE_TIMEOUT");
    RegisterTimingValue(opts,"GM_PREMERGE_MAX_TIMEOUT");
    RegisterTimingValue(opts,"GM_INVITE_RESPONSE_TIMEOUT");
    RegisterTimingValue(opts,"GM_AYT_RESPONSE_TIMEOUT");
    RegisterTimingValue(opts,"GM_PREMERGE_MIN_TIMEOUT");
    RegisterTimingValue(opts,"GM_PREMERGE_GRANULARITY");
    RegisterTimingValue(opts,"GM_PHASE_TIME");
    RegisterTimingValue(opts,"LB_PHASE_TIME");
    RegisterTimingValue(opts,"LB_ROUND_TIME");
    RegisterTimingValue(opts,"LB_REQUEST_TIMEOUT");
    RegisterTimingValue(opts,"SC_PHASE_TIME");
    RegisterTimingValue(opts,"DEV_PNP_HEARTBEAT");
    RegisterTimingValue(opts,"DEV_RTDS_DELAY");
    RegisterTimingValue(opts,"DEV_SOCKET_TIMEOUT");
    RegisterTimingValue(opts,"CSRC_RESEND_TIME");
    RegisterTimingValue(opts,"CSRC_DEFAULT_TIMEOUT");
    /////////////////////////////////////////////
    // ADD YOUR TIMING PARAMETERS BELOW HERE
    /////////////////////////////////////////////

    /////////////////////////////////////////////
    // ADD YOUR TIMING PARAMETERS ABOVE HERE
    ////////////////////////////////////////////
}

///////////////////////////////////////////////////////////////////////////////
/// CTimings::Get
/// @description Returns the value of the specified timing parameter, in
///     milliseconds, or throws an exception if the timing parameter does not
///     exist in the timings set.
/// @pre None
/// @post Throws exception if timing parameter has not been registered.
/// @return The requested timing parameter in ms.
///////////////////////////////////////////////////////////////////////////////
unsigned int CTimings::Get(std::string param)
{
    TimingMapIterator it;
    it = timing_values.find(param);
    if(it == timing_values.end())
        throw std::runtime_error("CTimings:: Requested timing parameter, "+param+", does not exist");
    return it->second;  
}

///////////////////////////////////////////////////////////////////////////////
/// CTimings::RegisterTimingValue
/// @description Registers a timing parameter with the configuration file
///     parser.
/// @pre None
/// @post The configuration file parser expects a new parameter when loading
///     the file. The timing value for the parameter is defaulted to zero.
/// @param opts The options parser that will parse the timings config
/// @param param the name of the timing parameter being added.
///////////////////////////////////////////////////////////////////////////////
void CTimings::RegisterTimingValue(po::options_description& opts, const std::string param)
{
    std::string desc = "The timing value "+param;
    opts.add_options()
        (param.c_str(),
        po::value<unsigned int>( ),
        desc.c_str() );
    timing_values[param] = 0;    
}

///////////////////////////////////////////////////////////////////////////////
/// CTimings::SetTimings
/// @description Loads the specified timing configuration file and sets all
///     the timing values from that file. If a timing value is missing from
///     that file, an exception is thrown.
/// @pre None
/// @post The timings values are loaded from the specified file, or an
///     exception is thrown because the file was missing one or more timing
///     parameters.
/// @param timingsFile The name of the file that contains the timings config.
///////////////////////////////////////////////////////////////////////////////
void CTimings::SetTimings(const std::string timingsFile)
{
    std::ifstream ifs;

    po::options_description opts("Timing Parameters");
    po::variables_map vm;
    std::string param;
	
    TimingParameters(opts);

    ifs.open(timingsFile.c_str());
    if (!ifs)
    {
        throw EDgiConfigError("Unable to open timings config " + timingsFile);
    }
    else
    {
        // Process the config
        po::store(parse_config_file(ifs, opts), vm);
        po::notify(vm);
        Logger.Info << "timer config file " << timingsFile <<
                " successfully loaded." << std::endl;
    }
    ifs.close();

    TimingMapIterator it;
    for(it=timing_values.begin(); it!=timing_values.end(); it++)
    {
        param = it->first;
        try
        {
            timing_values[param] = vm[param].as<unsigned int>();
        }
        catch (boost::bad_any_cast& e)
        {
            throw EDgiConfigError(
                    param+" is missing, please check your timings config");
        }
    } 
}

}
}

