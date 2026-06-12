#!/bin/sh

cnt=`ps aux | grep wpa_supplicant | grep -v grep  | wc -l`
if [ "${cnt}" != "0" ];then
	killall wpa_supplicant > /dev/null
fi

cnt1=`ps aux | grep hostapd | grep -v grep  | wc -l`
if [ "${cnt1}" != "0" ];then
	killall hostapd > /dev/null
fi

/etc/init.d/S80dnsmasq stop

echo 1 > /proc/sys/net/ipv4/ip_forward
iptables -t nat -A POSTROUTING -o eth0 -j MASQUERADE

sleep 1
ifconfig mlan0 up
ifconfig uap0 up
ifconfig uap0 192.168.2.1

hostapd /etc/hostapd-2.4g.conf &
#hostapd /etc/hostapd-5g.conf &
/etc/init.d/S80dnsmasq start

