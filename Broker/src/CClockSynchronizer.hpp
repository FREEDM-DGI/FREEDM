////////////////////////////////////////////////////////////////////////////////
/// @file         CClockSynchronizer.hpp
///
/// @author       Stephen Jackson <scj7t4@mst.edu>
///
/// @project      FREEDM DGI
///
/// @description  Handles the exchanges and mathematics to synchronize clocks
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

#ifndef FREEDM_CLOCK_HPP
#define FREEDM_CLOCK_HPP

#include "IDGIModule.hpp"

#include <list>
#include <map>

#include <boost/asio.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/noncopyable.hpp>

namespace freedm {
    namespace broker {

class CBroker;
class CPeerNode;

class CClockSynchronizer
    : public IDGIModule
{
public:
    /// Initialize module
    explicit CClockSynchronizer(boost::asio::io_service& ios);
    /// Returns the synchronized time
    boost::posix_time::ptime GetSynchronizedTime() const;
    /// Starts the synchronization algorithm.
    void Run();
    /// Stops the syncrhonization algorithm.
    void Stop();
    /// Processes incoming messages from other modules.
    void HandleIncomingMessage(boost::shared_ptr<const ModuleMessage> msg, CPeerNode peer);

private:
    /// Does the i,j referencing
    typedef std::pair<std::string,std::string> MapIndex;
    /// Stores the relative offsets
    typedef std::map< MapIndex, boost::posix_time::time_duration > OffsetMap;
    /// Query Tuple
    typedef std::pair<unsigned int, boost::posix_time::ptime> QueryRecord;
    /// Stores the outstanding clock queries
    typedef std::map< MapIndex, QueryRecord > QueryMap;
    /// Tuples of challenge query/response stuff
    typedef std::pair<boost::posix_time::ptime, boost::posix_time::ptime> TimeTuple;
    /// Stores the response pairs
    typedef std::list< TimeTuple > ResponseList;
    /// Stores the challenge responses
    typedef std::map< MapIndex, ResponseList > ResponseMap;
    /// Type used by skews
    typedef std::map< MapIndex, double > SkewMap;
    /// Container for decaying weights
    typedef std::pair< double, boost::posix_time::ptime > DecayingWeight;
    /// Type used by the weights
    typedef std::map< MapIndex, DecayingWeight > WeightMap;
    /// Last responses type
    typedef std::map< MapIndex, unsigned int > LastResponseMap;

    /// Handler for clock exchange responses
    void HandleExchangeResponse(const ExchangeResponseMessage& msg, CPeerNode peer);
    /// Receiver for clock exchange requests
    void HandleExchange(const ExchangeMessage& msg, CPeerNode peer);
    /// Sends clock exchange requests to other processes
    void Exchange(const boost::system::error_code& err );

    /// Generate the exchange message
    ModuleMessage CreateExchangeMessage(unsigned int k);
    /// Generate the exchange response message
    ModuleMessage CreateExchangeResponse(unsigned int k);
    /// Wraps a clock synchronizer message in a ModuleMessage
    static ModuleMessage PrepareForSending(const ClockSynchronizerMessage& message);

    /// Relative offsets
    OffsetMap m_offsets;
    /// Relative skews
    SkewMap m_skews;
    /// Relative weights
    WeightMap m_weights;
    /// Outstanding Clock Queries
    QueryMap m_queries;
    /// Old responses
    ResponseMap m_responses;
    /// Time between interactions
    boost::posix_time::ptime m_lastinteraction;
    /// The current k for identifying the freshness
    unsigned int m_kcounter;
    /// The last time a node responded
    LastResponseMap m_lastresponse;

    /// My offset
    boost::posix_time::time_duration m_myoffset;
    /// My skew
    double m_myskew;

    ///Time for the exchange
    boost::asio::deadline_timer m_exchangetimer;

    /// Gets the weight with a decay.
    double GetWeight(MapIndex i) const;

    /// Sets the weight for a process.
    void SetWeight(MapIndex i, double w);

    ///Turn a time duration into a double
    static double TDToDouble(boost::posix_time::time_duration td);

    ///Turn a double into a time duration
    static boost::posix_time::time_duration DoubleToTD(double td);
};


}
}
#endif
