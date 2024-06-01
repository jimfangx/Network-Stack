# Resources

## Tun/Tap:
There was some initial debate on the environment to write this on. 

A large part of the decision came down to whether it was possible grab L2 data from the kernel. I wanted to grab L2 data as that would've meant the ability to implement another (lower!) layer.

<img src="https://upload.wikimedia.org/wikipedia/commons/a/af/Tun-tap-osilayers-diagram.png" width="500px">

(https://en.wikipedia.org/wiki/TUN/TAP)


The following is some discoveries made along the way of making that decision:

* Turns out MacOS made it impossible (since Catalina) to grab L2 data from a TAP IF, and all applications have to use L3 data from a kernel TUN.
    * https://tunnelblick.net/cTunTapConnections.html
    * The old Tun Tap tool for MacOS became deprecated in 2015 (so anything beyond that was a hack and could lead to undefined/weird behavior): https://github.com/ntop/n2n/issues/773
        * This was the hack: a signed version of a system extension that allowed us to TAP: https://github.com/Tunnelblick/Tunnelblick/tree/master/third_party
        * The other hack (even more hacky) was to disable system integrity on macOS and allow the unsigned TunTapposx to run (this would not be ideal both for the sake of system security & future changes from apple).

* Windows was out of the picture for the sake of weird documentation and not having to deal with potentially antiquated windows behavior.

* Linux & BSD kernels have a [Universal Tun/Tap driver](https://en.wikipedia.org/wiki/TUN/TAP) that allows us to Tap into L2 data. Since macOS got rid of this (basically), our only other choice was Linux.
    * Linux's tun file descriptor (`/dev/net/tun`) acts as both a Tun & Tap IF!
    * Some kernel docs on the universal tun/tap driver: https://docs.kernel.org/networking/tuntap.html which includes essentially all the code one needs to interface with the tun/tap file descriptor
    * If you don't add the right flags you can have 4 bytes of data missing: https://stackoverflow.com/questions/43449664/why-the-leading-4bytes-data-missing-when-sending-raw-bytes-data-to-a-tap-device - see `tuntap_interface.c`

## Ethernet
* Original DEC, Intel Xerox spec: https://ethernethistory.typepad.com/papers/EthernetSpec.pdf
* Guide with a helpful visualizatoin of a raw ethernet packet: https://medium.com/kernel-space/unpacking-a-raw-packet-ethernet-frame-part-1-e91033e745a4
* Berkeley CS168 Lec 20: https://cs168.io/assets/lectures/lecture20.pdf

## Sk_buff
This was the carrier for each packet throughout our stack. Inspired by the Linux implementation of sk_buff: https://docs.kernel.org/networking/kapi.html#c.sk_buff

## ARP
* General desc: https://www.geeksforgeeks.org/how-address-resolution-protocol-arp-works/
    * Particularly this gif was particularly helpful (for ARP & ICMP):

        <img src="https://media.geeksforgeeks.org/wp-content/uploads/20240212121108/How-Address-Resolution-Protocol-ARP-works---gif-opt-(1).gif" width="500px">

    * ARP algorithm: RFC 826: https://datatracker.ietf.org/doc/html/rfc826
    * See `arp.c`

## IPv4
* Original standard: RFC 791: https://datatracker.ietf.org/doc/html/rfc791#section-3.1 
* The original RFC has 6 unused bits, however, some of those bits have been taken up by congestion control (DSCP & ECN). These are not in the original standard. Reference here: https://en.wikipedia.org/wiki/IPv4#Packet_structure
* What to include in a routing table: https://www.baeldung.com/cs/routing-table-entry
* Also see `route.c`

## ICMP
* Main standard RFC 792: https://datatracker.ietf.org/doc/html/rfc792
* Computing the checksum: RFC 1071: https://datatracker.ietf.org/doc/html/rfc1071

## TCP
* Main: RFC 793: https://datatracker.ietf.org/doc/html/rfc793
* Some additional TCP features: https://datatracker.ietf.org/doc/html/rfc7414
* This simplified the dense RFC a lot: https://www.geeksforgeeks.org/services-and-segment-structure-in-tcp/#

## Sockets API
* Socket states & state diagram: http://www.diranieh.com/SOCKETS/SocketStates.htm#:~:text=The%20state%20of%20a%20socket,even%20determines%20the%20error%20code).
* Linux socket API man: https://man7.org/linux/man-pages/man2/socket.2.html
* Project with existing socket API that might be a good inspiration of what to do: https://github.com/chobits/tapip

## Curl & HTTP
* Hooking up a Tun/Tap IF to the internet:
    * https://stackoverflow.com/questions/67123324/sending-tun-tap-packets-to-destination-server
    * https://unix.stackexchange.com/questions/588938/how-to-relay-traffic-from-tun-to-internet
    * search "connect tun device to internet"

## Helper functions:
* Host Byte Order <---> Network Byte Order: https://linux.die.net/man/3/ntohl
* tcpdump debugging: https://stackoverflow.com/questions/9334428/how-to-send-a-ping-packet