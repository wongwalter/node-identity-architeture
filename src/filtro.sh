#!/bin/sh
iptables -A OUTPUT -o nid0 -s 1.0.0.0/8 -d ! localhost -j QUEUE
