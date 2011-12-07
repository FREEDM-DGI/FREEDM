

#ifndef UTILITY_HPP_
#define UTILITY_HPP_

#include <map>


#include <boost/utility.hpp>
#include <boost/thread/once.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/noncopyable.hpp>

namespace Templates {
template <class K, class V>
/// function object to check the value of a map element
class set_value {
  private:
    V value;
  public:
    /// constructor (initialize value to compare with)
    set_value (const V& v)
     : value(v) {
    }
    /// Checks the value against the item in the map
    void operator() ( std::pair<const K, V> elem) {
        elem.second = value;
        return;
    }
};



template<class T>
/// A Singleton Class; Allows a single instance to be shared in global namespace
class Singleton : private boost::noncopyable
{

public:
    /// Fetches the instance of the object being tracked by the singleton
    static T& instance()
    {
        // Warning: If T's constructor throws, instance() will return a null reference.
        boost::call_once(init, flag);
        return *t;
    }
    /// Creates a new instance of the class, which is tracked by the singleton
    static void init() // never throws
    {
        t.reset(new T());
    }

protected:
    ~Singleton() {}
     Singleton() {}

private:
     static boost::scoped_ptr<T> t;
     static boost::once_flag flag;

};

}

template<class T> boost::scoped_ptr<T> Templates::Singleton<T>::t(0);
template<class T> boost::once_flag Templates::Singleton<T>::flag
									= BOOST_ONCE_INIT;

template <class T>
/// Uses a stream to a convert a string to type T
bool from_string(T& t,
                 const std::string& s,
                 std::ios_base& (*f)(std::ios_base&))
{
  std::istringstream iss(s);
  return !(iss >> f >> t).fail();
}

template <class T>
/// Uses a stream to convert from T to string
std::string to_string(const T& value)
{
    std::stringstream oss;
    oss << value;
    return oss.str();
}


#endif /* UTILITY_HPP_ */
