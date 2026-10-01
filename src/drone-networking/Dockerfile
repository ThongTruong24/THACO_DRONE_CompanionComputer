FROM alpine:3.20

# Cài đặt hostapd, dnsmasq, iproute2, util-linux (chứa rfkill), iptables, iw, bash
RUN apk add --no-cache \
    hostapd \
    dnsmasq \
    util-linux \
    iproute2 \
    iptables \
    iw \
    wpa_supplicant \
    bash \
    avahi \
    python3

WORKDIR /app

COPY hostapd.conf /etc/hostapd/hostapd.conf
COPY dnsmasq.conf /etc/dnsmasq.d/drone.conf
COPY 50-drone.yaml /etc/netplan/50-drone.yaml
COPY entrypoint.sh /app/entrypoint.sh

RUN chmod +x /app/entrypoint.sh

ENTRYPOINT ["/app/entrypoint.sh"]
