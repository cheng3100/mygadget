import sys
#  print(b'\xff'*6)
wol_prefix = b'\xff'*6
wol_mac = b'\x64\x51\x06\x34\xf4\x3e' * 16
sys.stdout.buffer.write(wol_prefix + wol_mac)

# pipe the output to netcat to send a wol broadcast udp packet
# python wol.py | netcat -b -u 255.255.255.255 7
# or send by give the target ip
# python wol.py | netcat -u <targetip> 7

# it is found that any port number is ok, the wol just a magic byte array

