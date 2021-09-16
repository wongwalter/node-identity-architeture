#!/usr/sbin/setkey -f
#
# Flush SAD and SPD
flush;
spdflush;

# Create policies for racoon
spdadd 10.10.1.4 10.10.1.3 any -P out ipsec
           esp/transport//require;

spdadd 10.10.1.3 10.10.1.4 any -P in ipsec
           esp/transport//require;
