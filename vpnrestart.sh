#!/bin/bash

VPNNAME=$(cat /etc/ethudp/SITE/VPNNAME)
HOSTNAME=$(cat /etc/ethudp/HOSTNAME)
INDEX=$(cat /etc/ethudp/SITE/INDEX)
BASEPORT=$(cat /etc/ethudp/SITE/BASEPORT)
PORT=`expr $BASEPORT + $INDEX`
IP=$(cat /etc/ethudp/IP)
MASK=$(cat /etc/ethudp/MASK)
GATE=$(cat /etc/ethudp/GATE)
MASTER=$(cat /etc/ethudp/MASTER)
SLAVE=$(cat /etc/ethudp/SLAVE)
PREFIX=$(cat /etc/ethudp/SITE/PREFIX)
IPV6=$(cat /etc/ethudp/IPV6)

sed -i -e "s/HOSTNAME=.*$/HOSTNAME=$HOSTNAME/" /etc/sysconfig/network
hostname $HOSTNAME

killall -9 EthUDP
killall -9 sendstat

/usr/src/vpnsetup/sendstat

ip link set eth0 up
ip link set eth1 up
ip add flush dev eth0
ip add add $IP/$MASK dev eth0
ip route add 0/0 via $GATE

ip add flush dev eth3
ip add add 100.64.0.1/24 dev eth3
ip link set eth3 up

ethtool -K eth1 gro off

OPT=$(cat /etc/ethudp/SITE/OPT)

if [ -z $MASTER ] ; then
	MASTER="CT"
fi

REMOTE=$(cat /etc/ethudp/SITE/$MASTER)

MPORT=`expr $PORT + 100`

if [ -z $SLAVE ]; then
	SLAVE="NONE"
fi

if [ $MASTER == $SLAVE ]; then
	SLAVE="NONE"
fi

if [ -z $IPV6 ]; then
	IPV6="NO"
fi

if [ $IPV6 == "YES" ]; then
	ip link set eth2 up
	ethtool -K eth2 gro off
fi

if [ $SLAVE = "NONE" ] ; then
	/usr/src/ethudp/EthUDP -n NET -e $OPT $IP $PORT $REMOTE $PORT eth1
	/usr/src/ethudp/EthUDP -n MGT -i $OPT $IP $MPORT $REMOTE $MPORT $PREFIX$INDEX.2 24
	if [ $IPV6 = "YES" ]; then
		PORT=`expr $PORT + 500`
		/usr/src/ethudp/EthUDP -n IPV6 -e $OPT $IP $PORT $REMOTE $PORT eth2
	fi
else
	REMOTE2=$(cat /etc/ethudp/SITE/$SLAVE)
	PORT2=`expr $PORT + 1000`
	MPORT2=`expr $PORT2 + 100`
	/usr/src/ethudp/EthUDP -n NET -e $OPT $IP $PORT $REMOTE $PORT eth1 $IP $PORT2 $REMOTE2 $PORT
	/usr/src/ethudp/EthUDP -n MGT -i $OPT $IP $MPORT $REMOTE $MPORT $PREFIX$INDEX.2 24 $IP $MPORT2 $REMOTE2 $MPORT
	if [ $IPV6 = "YES" ]; then
		PORT=`expr $PORT + 500`
		PORT2=`expr $PORT + 1000`
		/usr/src/ethudp/EthUDP -n IPV6 -e $OPT $IP $PORT $REMOTE $PORT eth2 $IP $PORT2 $REMOTE2 $PORT
	fi
fi
