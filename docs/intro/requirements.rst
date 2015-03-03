.. _system-requirements:

System Requirements
===================
This section lists system requirements for the FREEDM simulation.

DGI
---

The DGI is tested on recent versions of popular GNU/Linux distributions and require components that can generally be installed from your package manager:

* ISO-compliant C++98 compiler (such as recent versions of GCC or Clang)
* CMake 2.6 or higher
* Boost 1.47 or higher, including binaries
* Python 2.7.x (and not higher)
* Google Protocol Buffers 2.4.1 or higher (older versions may work)
* NTP daemon (not required if only running the PSCAD interface)

Boost
-----

The recommended method to install Boost is through your system's package manager. If your distribution does not package a sufficiently recent version of Boost, refer to the `Boost documentation` for instructions on compiling a newer version of Boost and installing it to `/usr/local/boost`. Some binary libraries are required: CMake will tell you which ones you are missing when you attempt to compile the DGI. 

Additionally, set the BOOST_ROOT environment variable to include the Boost directory::
 
    echo 'export BOOST_ROOT=/usr/local/boost/' >> ~/.bashrc

And restart your shell.
	
.. _Boost documentation: http://www.boost.org/doc/

Python
------

FREEDM DGI uses Python 2.7 for the time being; newer versions will not work. You must have a binary in your path named `python2`; this is generally `/usr/bin/python2`. If you don't have this binary then `/usr/bin/python` is probably python2 and you can safely create a symlink:: 

	ln -s /usr/bin/python /usr/bin/python2

NTP
---

All DGI nodes must run an NTP daemon (such as chrony or ntpd) to synchronize their clocks. The DGI runs its own fine-grained clock synchronizer designed to correct very small clock skews. However if two nodes' system clocks are off by a big amount, like one a minute or more, then there is no chance that the DGI's clock synchronizer will work and you will be unable to form groups.

PSCAD Simulation Requirements
-----------------------------

- PSCAD v4.4 Educational Edition
- GFortran compiler
- sys/socket.h
- netdb.h

Network
-------
Each computer that will run the DGI must have a unique hostname and each other computer that will run a DGI must be able to reach that machine by that hostname. You can check to see if the hostnames are properly configured with a simple ping test. On machine A::

	$ hostname
	raichu.freedm

On machine B::

	$ ping raichu.freedm
	PING raichu.freedm (216.229.90.108) 56(84) bytes of data.
	64 bytes from _________________________ (xxx.xxx.xxx.xxx): icmp_seq=1 ttl=64 time=0.348 ms
	64 bytes from _________________________ (xxx.xxx.xxx.xxx): icmp_seq=2 ttl=64 time=0.295 ms
	64 bytes from _________________________ (xxx.xxx.xxx.xxx): icmp_seq=3 ttl=64 time=0.264 ms
	^C
	--- raichu.freedm ping statistics ---
	6 packets transmitted, 6 received, 0% packet loss, time 5007ms
	rtt min/avg/max/mdev = 0.264/0.294/0.348/0.033 ms

The are many methods to achieve this. The easiest method is documented in :ref:`hostname-static-config`