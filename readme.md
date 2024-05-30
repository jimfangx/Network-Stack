# TCP



## C Include path
```
${workspaceFolder}/**
/usr/include/x86_64-linux-gnu
${workspaceFolder}/include
/usr/include
/usr/lib/gcc/x86_64-linux-gnu/13/include
```

## Bringing up custom tun/tap interface on linux:
https://stackoverflow.com/questions/1003684/how-to-interface-with-the-linux-tun-driver

## Ensure that ARPing updates ARP Table in kernel
https://www.spinics.net/lists/netdev/msg314093.html
https://askubuntu.com/questions/783017/bash-proc-sys-net-ipv4-ip-forward-permission-denied
- see `sudo bash` cmd

## debugging, tcpdumping your interface:
`tcpdump -i <tun/tap name> icmp` - dumps icmp related things

https://stackoverflow.com/questions/9334428/how-to-send-a-ping-packet

## hooking tun/tap to internet
https://stackoverflow.com/questions/67123324/sending-tun-tap-packets-to-destination-server
https://unix.stackexchange.com/questions/588938/how-to-relay-traffic-from-tun-to-internet
"connect tun device to internet"
