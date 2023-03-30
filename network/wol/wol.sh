#! /bin/bash
python wol_payload.py | netcat -b -u 255.255.255.255 7
