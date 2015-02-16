#include "CTimings.hpp"
#include "CLogger.hpp"
#include "FreedmExceptions.hpp"

#include <fstream>

#include <boost/program_options.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/foreach.hpp>
#include <boost/range/adaptor/map.hpp>

namespace po = boost::program_options;

namespace freedm {
namespace broker {

namespace {

CLocalLogger Logger(__FILE__);

}

static std::map<std::string, unsigned int> CTimings::timing_values;

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
    RegisterTimingValue(opts,"CSUC_RESEND_TIME");
    /////////////////////////////////////////////
    // ADD YOUR TIMING PARAMETERS BELOW HERE
    /////////////////////////////////////////////

    /////////////////////////////////////////////
    // ADD YOUR TIMING PARAMETERS ABOVE HERE
    ////////////////////////////////////////////
}

unsigned int CTimings::Get(std::string param) const
{
    std::map<std::string, unsigned int>::iterator it;
    it = timing_values.find(param);
    if(it == timing_values.end())
        throw std::runtime_error("CTimings:: Requested timing parameter does not exist");
    return *it.second;  
}

void CTimings::RegisterTimingValue(po::options_description&, const std::string param)
{
    desc = "The timing value "+param;
    opts.add_options()
        (param,
        po::value<unsigned int>( ),
        desc.c_str() );
    timing_values[param] = 0;    
}

void CTimings::SetTimings(const std::string timingsFile)
{
    std::ifstream ifs;

    po::options_description opts("Timing Parameters");
    po::variables_map vm;
    std::string desc;
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

    BOOST_FOREACH(timing_values, param | boost::adaptors::map_keys)
    {
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

