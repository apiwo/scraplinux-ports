#!/bin/sh
# i3wl session autostart — mirrors apiwo's niri spawn-at-startup list.
# Run via `dwl -s`; dwl forks this in its own session and kills the whole
# process group on exit.

swaybg -o '*' -i /home/apiwo/Downloads/Wallpaper.png -m fill &
mako &
xsettingsd &
xwayland-satellite &

# lock on idle, matching the existing swayidle timeouts (no monitor
# power-off/on step here — that used `niri msg action power-*-monitors`,
# which has no i3wl equivalent since wlopm isn't installed)
swayidle -w \
	timeout 300 /home/apiwo/.config/sway/scripts/lock.sh \
	before-sleep /home/apiwo/.config/sway/scripts/lock.sh &

wait
