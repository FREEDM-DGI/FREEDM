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

#include "CMessage.hpp"
#include "IHandler.hpp"

#include <map>

#include <boost/asio.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/noncopyable.hpp>

namespace freedm {
    namespace broker {

class CBroker;
class IPeerNode;

class CClockSynchronizer
    : public IReadHandler
    , private boost::noncopyable
{
    public:
    /// PeerNodePtr
    typedef boost::shared_ptr<IPeerNode> PeerNodePtr; 
    /// Initialize module
    CClockSynchronizer(CBroker &broker);
    /// Destructor for module
    ~CClockSynchronizer();
    /// Reciever
    void HandleExchangeResponse(MessagePtr msg, PeerNodePtr peer);
    /// Reciever
    void HandleExchange(MessagePtr msg, PeerNodePtr peer);
    /// Broadcaster    
    void Exchange(const boost::system::error_code& err );    
    /// Generate the exchange message
    CMessage ExchangeMessage(unsigned int k);
    /// Generate the exchange response message
    CMessage ExchangeResponse(unsigned int k);
    /// Returns the synchronized time
    boost::posix_time::ptime GetSynchronizedTime();
    /// Starts the stuff.
    void Run();
    /// Stops the stuff
    void Stop();

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
    /// The broker
    CBroker& m_broker;
    /// The UUID
    std::string m_uuid;
   
    /// Gets the weight with a decay 
    double GetWeight(MapIndex i);
    
    /// Sets the weight
    void SetWeight(MapIndex i, double w);
    
    ///Turn a time duration into a double
    double TDToDouble(boost::posix_time::time_duration td);

    ///Turn a double into a time duration
    boost::posix_time::time_duration DoubleToTD(double td);

};


}
}
#endif
