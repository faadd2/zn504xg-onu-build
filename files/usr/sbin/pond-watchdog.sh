#!/bin/sh
# ppp-up watchdog — workaround for pppd child-script segfault
# (ppp-up/ppp6-up die with signal 11 on this image, so netifd never
# learns that PPPoE dialed successfully; interface holds the IP but
# stays "down". This script re-reports the state whenever it finds a
# pppoe-* interface with an address but protocol state down.)
# DNS values: Beijing Unicom resolvers (static; pppd's DNS1/DNS2 are
# lost with the crashed child script).
for IF in wan tr069; do
	PIF="pppoe-$IF"
	ip addr show "$PIF" 2>/dev/null | grep -q "inet " || continue
	ifstatus "$IF" 2>/dev/null | grep -q '"up": true' && continue
	LOCAL=$(ip -4 -o addr show "$PIF" | awk '{print $4}' | cut -d/ -f1 | head -1)
	PEER=$(ip -4 -o addr show "$PIF" | awk '{print $4}' | head -1 | cut -sd/ -f2)
	[ -z "$PEER" ] && PEER=$(ip route | awk -v p="$PIF" '$0 ~ p && /src/ {print $1; exit}')
	[ -z "$LOCAL" ] && continue
	IFNAME="$PIF" IPLOCAL="$LOCAL" IPREMOTE="$PEER" \
	DNS1=202.106.46.151 DNS2=202.106.195.68 \
	sh /lib/netifd/ppp-up "$PIF" /dev/null 0 "$LOCAL" "$PEER" "$IF" \
	&& logger -t pond-watchdog "re-reported $IF ($LOCAL) to netifd" \
	|| logger -t pond-watchdog "failed to re-report $IF"
done
exit 0
