#!/usr/bin/env python

import sys,getopt
from smbus2 import SMBus, i2c_msg
import time
from inspect import getframeinfo, stack
import pprint

i2c_buf_max_len = 256
fill_value = 0xaa

buf_pattern  = ["zero", "one", "incre", "decre", "alter"]
pattern_max = len(buf_pattern)

debug = 1
pp = pprint.PrettyPrinter(indent =4)

def dp(str):
    if debug==1:
        pp.pprint(str)

def debuginfo(message):
    if debug == 1:
        caller = getframeinfo(stack()[1][0])
        print("%s:%d - %s" % (caller.filename, caller.lineno, message)) # python3 syntax print

class RequiredOptions:
    '''Just something to keep track of required options'''
    def __init__(self, options=[]):
        self.required_options = options

    def add(self, option):
        if option not in self.required_options:
            self.required_options.append(option)

    def resolve(self, option):
        if option in self.required_options:
            self.required_options.remove(option)

    def optionsResolved(self):
        if len(self.required_options):
            return False
        else:
            return True

def fill_buf_by_pattern(pattern, size):
    buf = b''
    if pattern == "zero":
        buf = bytearray([0]) * size
    elif pattern == "one":
        buf = bytearray([0xFF]) * size
    elif pattern == "incre":
        buf = bytearray(size)
        for i in range(size):
            buf[i] = i & 0xFF
    elif pattern == "decre":
        buf = bytearray(size)
        for i in range(size):
            buf[i] = 0xFF - (i & 0xFF)
    elif pattern == "alter":
        buf = bytearray(size)
        val = fill_value
        for i in range(size):
            buf[i] = val
            val = (1 << 8) - 1 - val

    return buf

def check_buf_by_pattern(buf, pattern, size):
    if pattern == "zero":
        for i in range(size):
            if buf[i] != 0:
                return False
    elif pattern == "one":
        for i in range(size):
            if buf[i] != 0xFF:
                return False
    elif pattern == "incre":
        for i in range(size):
            if buf[i] != i & 0xFF:
                return False
    elif pattern == "decre":
        for i in range(size):
            if buf[i] != 0xFF - (i & 0xFF):
                return False
    elif pattern == "alter":
        val = fill_value
        for i in range(size):
            if buf[i] != val:
                return False
            val = (1 << 8) - 1 - val

    return True

def print_usage():
    print("usage: i2c_master.py -a <addr> -m <mode> -p <buf pattern> -n <size>")
    print("\tFor general call:\ni2c_master.py -m g -v <gncall cmd>\n")
    return

def main(argv):
    '''
    Main program function
    '''
    addr = 0
    mode = ""
    pattern = 0
    size = i2c_buf_max_len
    buf = b""
    gncall_cmd = 0

    try:
        opts, args = getopt.getopt(argv, "ha:m:p:n:v:")
    except getopt.GetoptError:
        print_usage()
        debuginfo("usage wrong")
        sys.exit(2)

    # required parameters
    req = RequiredOptions(["-a", "-m", "-p", "-n"])
    req_gn = RequiredOptions(["-m", "-v"])

    for opt, arg in opts:
        if opt == "-h":
            print_usage()
            sys.exit(2)

        if opt == "-a":
            addr = int(arg, base=0)
            dp("addr: 0x{:02x}".format(addr))
            req.resolve(opt)
        if opt == "-m":
            req.resolve(opt)
            req_gn.resolve(opt)
            if arg not in ("s", "r", "g"):
                print("error: <mode>: s -> send, r -> receive, g -> gncall")
                sys.exit(2)
            mode = arg
            dp("mode: {}".format(mode))
        elif opt == "-p":
            req.resolve(opt)
            p = int(arg)
            if p <0 or p > pattern_max:
                print("error: <buf pattern>: [0, 4]")
                sys.exit(2)
            pattern = p
            dp("pattern: {}".format(p))

        elif opt == "-n":
            req.resolve(opt)
            size = int(arg)
            dp("size: {}".format(size))
        elif opt == "-v":
            req.resolve(opt)
            req_gn.resolve(opt)
            gncall_cmd = int(arg, base=0)
            dp("gncall cmd: {}".format(gncall_cmd))

    if not req.optionsResolved() and not req_gn.optionsResolved():
        print_usage()
        debuginfo("missing parameters")
        sys.exit(2)


    i2cbus = SMBus(1)  # Create a new I2C bus
    #  time.sleep(1)

    if mode == "s":
        buf = fill_buf_by_pattern(buf_pattern[pattern], size)
        dp(buf)
        write = i2c_msg.write(addr, list(buf))
        i2cbus.i2c_rdwr(write)
    elif mode == "g":
        dp("sending gncall cmd:{}".format(gncall_cmd))
        write = i2c_msg.write(0x00, list(bytearray([gncall_cmd])))
        i2cbus.i2c_rdwr(write)
    elif mode == "r":
        read = i2c_msg.read(addr, size)
        i2cbus.i2c_rdwr(read)
        #  buf = bytearray(read)
        buf = list(read)
        dp("receive i2c buf: {}".format(buf))
        if check_buf_by_pattern(buf, buf_pattern[pattern], size) == True:
            print("receive {} bytes buf, pattern {},".format(len(buf), buf_pattern[pattern]))
            print("check pass")
        else:
            print("receive {} bytes buf, pattern {},".format(len(buf), buf_pattern[pattern]))
            print("check fail")

if __name__ == "__main__":
    main(sys.argv[1:])
(.venv) pi@raspberrypi:~/work/i2c_test $
(.venv) pi@raspberrypi:~/work/i2c_test $ fg
vim i2c_master.py
                print("error: <buf pattern>: [0, 4]")
                sys.exit(2)
            pattern = p
            dp("pattern: {}".format(p))

        elif opt == "-n":
            req.resolve(opt)
            size = int(arg)
            dp("size: {}".format(size))
        elif opt == "-v":
            req.resolve(opt)
            req_gn.resolve(opt)
            gncall_cmd = int(arg, base=0)
            dp("gncall cmd: {}".format(gncall_cmd))

    if not req.optionsResolved() and not req_gn.optionsResolved():
        print_usage()
        debuginfo("missing parameters")
        sys.exit(2)


    i2cbus = SMBus(1)  # Create a new I2C bus
    #  time.sleep(1)

    if mode == "s":
        buf = fill_buf_by_pattern(buf_pattern[pattern], size)
        dp(buf)
        write = i2c_msg.write(addr, list(buf))
        i2cbus.i2c_rdwr(write)
    elif mode == "g":
        dp("sending gncall cmd:{}".format(gncall_cmd))
        write = i2c_msg.write(0x00, list(bytearray([gncall_cmd])))
        i2cbus.i2c_rdwr(write)
    elif mode == "r":
        read = i2c_msg.read(addr, size)
        i2cbus.i2c_rdwr(read)
        #  buf = bytearray(read)
        buf = list(read)
        dp("receive i2c buf: {}".format(buf))
        if check_buf_by_pattern(buf, buf_pattern[pattern], size) == True:
            print("receive {} bytes buf, pattern {},".format(len(buf), buf_pattern[pattern]))
            print("check pass")
        else:
            print("receive {} bytes buf, pattern {},".format(len(buf), buf_pattern[pattern]))
            print("check fail")

if __name__ == "__main__":
    main(sys.argv[1:])

