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


boost::property_tree::ptree decode_tree(std::string encoded)
{
    boost::property_tree::ptree decoded;
    while(encoded.length() > 0)
    {
        std::stringstream conv( encoded.substr(STORETYPELEN,LENGTHFIELD) );
        size_t fieldlen;
        conv>>fieldlen;
        char token = encoded[0];
        encoded = encoded.substr(STORETYPELEN+LENGTHFIELD);
        // Check to see if you're unpacking a value or a key.
        if(token == 'k')
        {
            std::string key = encoded.substr(0,fieldlen);
            encoded = encoded.substr(fieldlen);
            token = encoded[0];
            if(token == 's')
            {
                std::stringstream subconv( encoded.substr(STORETYPELEN,LENGTHFIELD) );
                size_t fieldlen;
                subconv>>fieldlen;
                encoded = encoded.substr(STORETYPELEN+LENGTHFIELD);
                // Take the subtree and add it based on the key you just decoded
                std::string subtree = encoded.substr(0,fieldlen);
                decoded.add_child( key, decode_tree( subtree ) );
                encoded = encoded.substr( fieldlen );
            }
        }
        else if(token == 'v')
        {
            std::string v = encoded.substr(0,fieldlen);
            // If the tree has no key, store the value in the root.
            decoded.put_value( v );
            encoded = encoded.substr( fieldlen );
        }
    }
    return decoded;
}

#endif
