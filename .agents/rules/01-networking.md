# Networking rules

- `drone-networking` is the only service allowed to create or reconfigure
  `wlan0` / `uap0` and DHCP/AP processes.
- AP gateway is `192.168.10.1/24`; DHCP allocation is defined in `dnsmasq.conf`.
- `50-drone.yaml` is a host netplan template. Apply it explicitly with
  `make apply-netplan`, never as an incidental side effect of another service.
- See `src/drone-networking/README.md` for the runtime contract and diagnostics.
