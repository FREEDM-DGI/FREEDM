#ifndef FREEDM_UUID_HPP
#define FREEDM_UUID_HPP

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

namespace freedm {
/// A rarely used type for uuids, used for initial generation of uuids
class CUuid : public boost::uuids::uuid
{
private:

public:
    /// Intializes the uuid and the uuid generator
    CUuid()
        : boost::uuids::uuid(boost::uuids::random_generator()())
    {}

    /// Copy constructor for the UUID generator
    explicit CUuid(boost::uuids::uuid const& u)
        : boost::uuids::uuid(u)
    {}

    /// Generates a UUID from an input string
    explicit CUuid ( const std::string &s )
		: boost::uuids::uuid(boost::uuids::string_generator()(s))
    {}

    /// Returns a UUID in the DNS namespace for the given hostname.
    static CUuid from_dns( const std::string &s, const std::string &p )
	{
    	boost::uuids::uuid dns_namespace =
    		boost::uuids::string_generator()(
    				"{6ba7b810-9dad-11d1-80b4-00c04fd430c8}"
    		);
    	return CUuid(boost::uuids::name_generator(dns_namespace)(s+":"+p));
	}
};

}

#endif // FREEDM_UUID_HPP
