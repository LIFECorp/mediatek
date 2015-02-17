#!/bin/sh

# follow for ipv6
v6_dft_dev='usb0'
rlt=`busybox ip -6 route | grep default | grep -v 'lo'`

if [ "" = "$rlt" ]
then
  echo "no ipv6 default info"
  busybox ip -6 route add default dev $v6_dft_dev
else
  echo "get ipv6 default info [$rlt]"
fi
