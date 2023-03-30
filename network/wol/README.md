# wol magic packet sender
wakeonlan is a method to trigger the suspend or poweroff machine to wakeup by sending a special `magic packet` to the eth card.
The wlan doesn't not support wol now.

The `magic packet` is just:
`0xff` * 6 + `<target_mac>` * 16 = 102 raw bytes
> note that the eth card only search for a if there is a `magic packet` pattern in the raw receive data. It doesn't parse the protocol.
> So any packet is ok. For example the udp port can be arbitrary.

# test
In the target machine:
`systemctl suspend` to suspend the machine. Then invoke the `wol.sh` in another machin inside the same LAN. The target will be waked up.
