#include <boost/property_tree/ptree.hpp>
#include <sstream>
#include <iostream>

#ifndef PTREEPACKER_H
#define PTREEPACKER_H

#define LENGTHFIELD 5
#define STORETYPELEN 1

std::string encode_tree(const boost::property_tree::ptree &tree)
{
    if(tree.empty())
    {
        // if there isn't a subtree, the length of the data as a string and
        // Then the data as a string.
        std::stringstream out;
        std::string encoded;
        std::stringstream conv;
        conv<<tree.data();
        encoded = conv.str();
        out<<'v'<<std::setw(LENGTHFIELD)<<std::setfill('0')<<encoded.length();
        return out.str()+encoded;
    }
    else
    {
        std::string r;
        boost::property_tree::ptree::const_iterator it = tree.begin();
        for( ; it != tree.end(); ++it)
        {
            std::stringstream out;
            std::stringstream subt;
            std::string subr;
            // Encode the length of the key and then the name of the key.
            out<<'k'<<std::setw(LENGTHFIELD)<<std::setfill('0')<<it->first.length();
            out<<it->first;
            // Encode the subtree
            subr = encode_tree(it->second);
            subt<<'s'<<std::setw(LENGTHFIELD)<<std::setfill('0')<<subr.length();
            r += out.str() + subt.str() + subr;
        }
        return r;
    }
}

boost::property_tree::ptree decode_tree(const std::string &encoded)
{
    boost::property_tree::ptree decoded;
    size_t s = 0;
    while(s < encoded.length())
    {
        size_t storet = s;
        std::stringstream conv(encoded.substr(s+STORETYPELEN,LENGTHFIELD));
        s += STORETYPELEN + LENGTHFIELD;
        size_t fieldlen;
        conv>>fieldlen;
        // Check to see if you're unpacking a value or a key.
        if(encoded[storet] == 'k')
        {
            std::string key;
            key = encoded.substr(s,fieldlen);
            s += fieldlen;
            if(encoded[s] == 's')
            {
                std::stringstream subconv(encoded.substr(s+STORETYPELEN,LENGTHFIELD));
                s += STORETYPELEN + LENGTHFIELD;
                size_t fieldlen;
                conv>>fieldlen;
                // Take the subtree and add it based on the key you just decoded
                decoded.add_child( key, decode_tree(encoded.substr(s,fieldlen)) );
                s += fieldlen;
            }
        }
        else if(encoded[storet] == 'v')
        {
            // If the tree has no key, store the value in the root.
            decoded.put_value(encoded.substr(s,fieldlen));
            s+=fieldlen;
        }
    }
    return decoded;
}

#endif
