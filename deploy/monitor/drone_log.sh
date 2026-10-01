#!/bin/sh
# drone_log.sh -- Shell wrapper and CLI for drone logging
_DRONE_LOG_PY="${DRONE_LOG_PY:-/tmp/drone_log.py}"
_DRONE_LOG_SOCK="${DRONE_LOG_SOCK:-/run/drone/log.sock}"

drone_log() {
    _sev="$1"
    _src="$2"
    _msg="$3"

    printf "[%s][%s] %s\n" "$_sev" "$_src" "$_msg"

    _num=6
    case "$_sev" in
        CRITICAL|critical|2) _num=2 ;;
        ERROR|error|3)       _num=3 ;;
        WARN|WARNING|warn|warning|4) _num=4 ;;
        INFO|info|6)         _num=6 ;;
    esac

    if command -v python3 > /dev/null 2>&1 && [ -f "$_DRONE_LOG_PY" ]; then
        python3 "$_DRONE_LOG_PY" --src "$_src" --sev "$_num" --msg "$_msg" --sock "$_DRONE_LOG_SOCK" 2>/dev/null || true
    elif command -v perl > /dev/null 2>&1; then
        perl -MSocket -e '
            my ($sock_path, $src, $sev, $msg) = @ARGV;
            socket(my $s, PF_UNIX, SOCK_DGRAM, 0) or exit 0;
            connect($s, sockaddr_un($sock_path)) or exit 0;
            $msg =~ s/"/\\"/g;
            my $payload = sprintf("{\"src\":\"%s\",\"sev\":%d,\"msg\":\"%s\"}", $src, $sev, $msg);
            send($s, $payload, 0);
        ' "$_DRONE_LOG_SOCK" "$_src" "$_num" "$_msg" 2>/dev/null || true
    fi
}

drone_info()     { drone_log INFO     "$1" "$2"; }
drone_warn()     { drone_log WARNING  "$1" "$2"; }
drone_error()    { drone_log ERROR    "$1" "$2"; }
drone_critical() { drone_log CRITICAL "$1" "$2"; }

# Cho phep chay truc tiep: /tmp/drone_log.sh INFO "drone-container" "message"
if [ "$#" -ge 3 ]; then
    drone_log "$1" "$2" "$3"
fi
