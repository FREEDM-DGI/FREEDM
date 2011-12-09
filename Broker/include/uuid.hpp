#ifndef FREEDM_UUID_HPP
#define FREEDM_UUID_HPP

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

namespace freedm {
/// A rarely used type for uuids, used for initial generation of uuids
class uuid : public boost::uuids::uuid
{
private:

public:
    /// Intializes the uuid and the uuid generator
    uuid()
        : boost::uuids::uuid(boost::uuids::random_generator()())
    {}

    /// Copy constructor for the UUID generator
    explicit uuid(boost::uuids::uuid const& u)
        : boost::uuids::uuid(u)
    {}

    /// Generates a UUID from an input string
    explicit uuid ( const std::string &s )
		: boost::uuids::uuid(boost::uuids::string_generator()(s))
    {}

    /// Returns a UUID in the DNS namespace for the given hostname.
    static uuid from_dns( const std::string &s )
	{
    	boost::uuids::uuid dns_namespace =
    		boost::uuids::string_generator()(
    				"{6ba7b810-9dad-11d1-80b4-00c04fd430c8}"
    		);
    	return uuid(boost::uuids::name_generator(dns_namespace)(s));
	}

    /// Casts this to the boost uuid type
    operator boost::uuids::uuid() {
        return static_cast<boost::uuids::uuid&>(*this);
    }

    /// Casts this to the boost uuid type
    operator boost::uuids::uuid() const {
        return static_cast<boost::uuids::uuid const&>(*this);
    }
};

}

#endif // FREEDM_UUID_HPP
