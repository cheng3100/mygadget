import socket

def wol(lunaMacAddress: bytes):
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    s.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)

    magic = b'\xff' * 6 + lunaMacAddress * 16
    s.sendto(magic, ('<broadcast>', 7))

if __name__ == '__main__':
    # pass to wol the mac address of the ethernet port of the appliance to wakeup
    wol(b'\x00\x15\xB2\xAA\x5B\x00')
