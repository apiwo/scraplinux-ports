# ScrapLinux - starts the pipewire/wireplumber user session once per
# login, since pipewire is a per-user daemon and nothing else on ScrapLinux
# starts it. Skipped outside a graphical session and if already running.
if [ -n "${DISPLAY:-}${WAYLAND_DISPLAY:-}" ]; then
	_pw_running=0
	for _pw_p in /proc/[0-9]*; do
		[ "$(readlink "$_pw_p/exe" 2>/dev/null)" = /usr/bin/pipewire ] && { _pw_running=1; break; }
	done
	if [ "$_pw_running" = 0 ]; then
		mkdir -p "${XDG_RUNTIME_DIR:-/run/user/$(id -u)}" 2>/dev/null
		( pipewire >/dev/null 2>&1 & )
		( pipewire-pulse >/dev/null 2>&1 & )
		( wireplumber >/dev/null 2>&1 & )
	fi
	unset _pw_running _pw_p
fi
