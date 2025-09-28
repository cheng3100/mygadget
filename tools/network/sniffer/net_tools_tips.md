# Network Tools Tips

## tcpdump Tool

### Basic Capture Commands

#### 1. Capture from Specific Interface
```bash
# Capture from wireless interface
tcpdump -i wlan0

# Capture from ethernet interface
tcpdump -i eth0

# Capture from any interface
tcpdump -i any
```

#### 2. Save to File
```bash
# Save to file
tcpdump -i wlan0 -w capture.pcap

# Save with timestamp
tcpdump -i wlan0 -w capture_$(date +%Y%m%d_%H%M%S).pcap
```

### Protocol-Specific Filters

#### 3. ARP Filtering
```bash
# Capture only ARP packets
tcpdump -i wlan0 arp

# ARP requests only
tcpdump -i wlan0 "arp[6:2] = 1"

# ARP responses only
tcpdump -i wlan0 "arp[6:2] = 2"

# ARP from specific MAC
tcpdump -i wlan0 "arp and ether src 00:11:22:33:44:55"

# ARP to specific IP
tcpdump -i wlan0 "arp and arp[24:4] = 192.168.1.1"
```

#### 4. DHCP Filtering
```bash
# Capture only DHCP packets
tcpdump -i wlan0 port 67 or port 68

# DHCP Discover (client to server)
tcpdump -i wlan0 "port 67 or port 68" and "udp[8:1] = 1"

# DHCP Offer (server to client)
tcpdump -i wlan0 "port 67 or port 68" and "udp[8:1] = 2"

# DHCP Request
tcpdump -i wlan0 "port 67 or port 68" and "udp[8:1] = 3"

# DHCP ACK
tcpdump -i wlan0 "port 67 or port 68" and "udp[8:1] = 5"
```

#### 5. ICMP Filtering
```bash
# Capture only ICMP packets
tcpdump -i wlan0 icmp

# ICMP Echo Request (ping)
tcpdump -i wlan0 "icmp[0] = 8"

# ICMP Echo Reply (pong)
tcpdump -i wlan0 "icmp[0] = 0"

# ICMP Destination Unreachable
tcpdump -i wlan0 "icmp[0] = 3"

# ICMP Time Exceeded
tcpdump -i wlan0 "icmp[0] = 11"

# ICMP from specific host
tcpdump -i wlan0 "icmp and src host 192.168.1.1"
```

#### 6. UDP Filtering
```bash
# Capture only UDP packets
tcpdump -i wlan0 udp

# UDP on specific port
tcpdump -i wlan0 "udp port 53"        # DNS
tcpdump -i wlan0 "udp port 123"      # NTP
tcpdump -i wlan0 "udp port 161"      # SNMP
tcpdump -i wlan0 "udp port 5004"     # RTP

# UDP from specific host
tcpdump -i wlan0 "udp and src host 192.168.1.100"

# UDP to specific host
tcpdump -i wlan0 "udp and dst host 192.168.1.1"

# UDP with specific payload size
tcpdump -i wlan0 "udp and len > 1000"
```

#### 7. TCP Filtering
```bash
# Capture only TCP packets
tcpdump -i wlan0 tcp

# TCP on specific port
tcpdump -i wlan0 "tcp port 80"       # HTTP
tcpdump -i wlan0 "tcp port 443"      # HTTPS
tcpdump -i wlan0 "tcp port 22"       # SSH
tcpdump -i wlan0 "tcp port 23"       # Telnet

# TCP connection states
tcpdump -i wlan0 "tcp[tcpflags] & tcp-syn != 0"     # SYN packets
tcpdump -i wlan0 "tcp[tcpflags] & tcp-ack != 0"     # ACK packets
tcpdump -i wlan0 "tcp[tcpflags] & tcp-fin != 0"     # FIN packets
tcpdump -i wlan0 "tcp[tcpflags] & tcp-rst != 0"     # RST packets

# TCP three-way handshake
tcpdump -i wlan0 "tcp[tcpflags] & tcp-syn != 0 and tcp[tcpflags] & tcp-ack = 0"

# TCP data packets
tcpdump -i wlan0 "tcp[tcpflags] & tcp-push != 0"
```

### Advanced Filtering

#### 8. IP Address Filtering
```bash
# From specific IP
tcpdump -i wlan0 "src host 192.168.1.100"

# To specific IP
tcpdump -i wlan0 "dst host 192.168.1.1"

# Between two IPs
tcpdump -i wlan0 "host 192.168.1.100 and host 192.168.1.1"

# From IP range
tcpdump -i wlan0 "src net 192.168.1.0/24"

# Exclude specific IP
tcpdump -i wlan0 "not host 192.168.1.1"
```

#### 9. MAC Address Filtering
```bash
# From specific MAC
tcpdump -i wlan0 "ether src 00:11:22:33:44:55"

# To specific MAC
tcpdump -i wlan0 "ether dst 00:11:22:33:44:55"

# Between two MACs
tcpdump -i wlan0 "ether host 00:11:22:33:44:55"

# Broadcast packets
tcpdump -i wlan0 "ether dst ff:ff:ff:ff:ff:ff"
```

#### 10. WLAN Specific Filtering
```bash
# WLAN management frames
tcpdump -i wlan0 "wlan type mgt"

# WLAN control frames
tcpdump -i wlan0 "wlan type ctrl"

# WLAN data frames
tcpdump -i wlan0 "wlan type data"

# Beacon frames
tcpdump -i wlan0 "wlan type mgt and wlan subtype beacon"

# Probe requests
tcpdump -i wlan0 "wlan type mgt and wlan subtype probereq"

# Probe responses
tcpdump -i wlan0 "wlan type mgt and wlan subtype proberesp"
```

### Useful Options

#### 11. Output Control
```bash
# Verbose output
tcpdump -i wlan0 -v

# More verbose
tcpdump -i wlan0 -vv

# Even more verbose
tcpdump -i wlan0 -vvv

# Don't resolve hostnames
tcpdump -i wlan0 -n

# Don't resolve port names
tcpdump -i wlan0 -nn

# Show packet contents in hex
tcpdump -i wlan0 -x

# Show packet contents in hex and ASCII
tcpdump -i wlan0 -X
```

#### 12. Capture Control
```bash
# Limit number of packets
tcpdump -i wlan0 -c 100

# Limit capture time
tcpdump -i wlan0 -G 60 -w capture_%Y%m%d_%H%M%S.pcap

# Limit file size
tcpdump -i wlan0 -C 10 -w capture.pcap

# Capture only first N bytes
tcpdump -i wlan0 -s 64
```

### Practical Examples

#### 13. Common Use Cases
```bash
# Monitor DHCP traffic
tcpdump -i wlan0 "port 67 or port 68" -v

# Monitor DNS queries
tcpdump -i wlan0 "port 53" -n

# Monitor HTTP traffic
tcpdump -i wlan0 "tcp port 80" -A

# Monitor WLAN beacons
tcpdump -i wlan0 "wlan type mgt and wlan subtype beacon" -v

# Monitor ARP requests
tcpdump -i wlan0 "arp[6:2] = 1" -n

# Monitor ICMP ping
tcpdump -i wlan0 "icmp[0] = 8" -n
```

#### 14. Complex Filters
```bash
# HTTP traffic from specific host
tcpdump -i wlan0 "tcp port 80 and src host 192.168.1.100"

# UDP traffic larger than 1000 bytes
tcpdump -i wlan0 "udp and len > 1000"

# TCP SYN packets to port 80
tcpdump -i wlan0 "tcp port 80 and tcp[tcpflags] & tcp-syn != 0"

# WLAN data frames from specific MAC
tcpdump -i wlan0 "wlan type data and wlan src 00:11:22:33:44:55"
```

### Key Takeaways

1. **Protocol filters**: `arp`, `icmp`, `udp`, `tcp`
2. **Port filters**: `port 80`, `port 53`
3. **Host filters**: `src host`, `dst host`
4. **MAC filters**: `ether src`, `ether dst`
5. **WLAN filters**: `wlan type`, `wlan subtype`
6. **Combined filters**: Use `and`, `or`, `not`