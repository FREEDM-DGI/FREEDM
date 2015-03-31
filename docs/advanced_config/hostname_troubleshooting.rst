Troubleshooting Hostnames
=========================

"Are Your Hostnames Configured Correctly Error"
------------------------------------------------

If you observe this error::

    2013-Mar-19 11:39:37.207802 : PosixMain.cpp : Error(2):
        Exception caught in Broker: Could not resolve the endpoint victory-road:1870 (victory-road:1870) Are your hostnames configured correctly?

By default, most systems cannot resolve their own hostnames, unless, of course, you get your hostname from DHCP, in which case it could (potentially) be resolvable via DNS. But local hostname resolution is mandatory for FREEDM (and many other programs) to work. You have two options:

* Specify your own hostname in `/etc/hosts`. Some distros (e.g. Debian-based) do this for you automatically. This will suffice for static hostnames, but it will not work if you get your transient hostname from DHCP unless you add your transient hostname to `/etc/hosts` AND know that it will never change. (Alternatively, older versions of NetworkManager modify `/etc/hosts` for you.)
* Enable local hostname resolution using nss-myhostname. nss-myhostname is available on all modern distros, but not necessarily installed or enabled by default. To enable, you will have to add the module "myhostname" to the end of the hosts line in `/etc/nsswitch.conf`. (nss-myhostname has been merged into systemd 197, so on newer distros you might not need a separate package for this to work.)

Using nss-myhostname will allow you to seamlessly switch networks without losing resolvability, so we strongly recommend this approach.

All of my DGI are on the same machine, and I can't form groups
--------------------------------------------------------------

First, a bit of background info. You actually have a few different types of hostnames. If you have systemd, run `hostnamectl` to see them all.

* static hostname, stored in `/etc/hostname` by systemd and also on Debian-based systems.
* pretty hostname, which doesn't matter. (If you have GNOME 3.6 or higher, this is what you see on the Details panel in System Settings.)
* transient/dynamic/kernel hostname, may be assigned by your network configuration (e.g. DHCP) but will otherwise be the same as your static hostname (in which case `hostnamectl` will not display it). This is the most important hostname since it is what can be resolved by Linux system calls. You can also check this with the `hostname` command or `cat /proc/sys/kernel/hostname`

Other computers on your network may be able to resolve your transient hostname via DNS provided it is distinct from your static hostname, but they cannot resolve your static hostname unless you purposefully set it to something they can resolve via DNS or else manually modify `/etc/hosts` on those machines. If you're reading this page, then you can already form groups of DGI with one DGI per host, so you've already figured out how this works and know that your transient hostname is what goes in the add-host directive in `freedm.cfg`, so that's not a problem for you. And since you read the page up to this point, you've also made sure that this hostname is resolvable locally.

So having considered all of that, you're using a static hostname (by far the most common out-of-the-box configuration) that's locally resolvable, but you can't form groups with two DGI on the same machine. This is a known bug, but we're not quite sure what's wrong. It's most likely a DGI bug, but it could very well be a Boost bug or even an operating system bug. Try manually adding your hostname to /etc/hosts.
