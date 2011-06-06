#!/usr/bin/env python
# this code belongs to public domain
# requires nodes.dat filename passed as argument
 
import struct
import sys

version = 0
count = 0

# check number of command line arguments
if len(sys.argv) != 2:
    sys.exit("Please supply a nodes.dat file!")
 
nodefile = open(sys.argv[1], 'r')

(count,) = struct.unpack("<I", nodefile.read(4))
if (count == 0): 
    (version,) = struct.unpack("<I", nodefile.read(4))
    (count,)   = struct.unpack("<I", nodefile.read(4))

if (version >= 0 & version < 3):
    print 'Nodes.dat file version = %d' %(version)
    print 'Node count = %d' %(count)
    print ' '
    if (version == 0):
        print ' idx type  IP address      udp   tcp'
    else :
        print ' idx Ver IP address        udp   tcp kadUDPKey        verified'

    for i in xrange(count):
        if (version == 0):
            (clientid, ip1, ip2, ip3, ip4, udpport, tcpport, 
             type) = struct.unpack("<16s4BHHB", nodefile.read(25))
            ipaddr = '%d.%d.%d.%d' % (ip1, ip2, ip3, ip4)
            print '%4d %4d %-15s %5d %5d' % (i, type, ipaddr, 
                                             udpport, tcpport)
        else :
            (clientid, ip1, ip2, ip3, ip4, udpport, tcpport, type,  kadUDPkey, 
             verified) = struct.unpack("<16s4BHHBQB", nodefile.read(34))
            ipaddr = '%d.%d.%d.%d' % (ip1, ip2, ip3, ip4)
            if (verified == 0): verf='N'
            else : verf='Y'
            print '%4d %3d %-15s %5d %5d %16x %s' % (i, type, ipaddr, udpport,
                                                     tcpport, kadUDPkey, verf)

else :
    print 'Cannot handle nodes.dat version %d !' (version)

nodefile.close()
