#!/bin/sh

export XDG_CURRENT_DESKTOP=XFCE5
export XDG_SESSION_TYPE=wayland
export GDK_BACKEND=wayland
export QT_QPA_PLATFORM=wayland
export MOZ_ENABLE_WAYLAND=1

/usr/bin/xfce5-autoscale-daemon
if [ -f /tmp/xfce5_env ]; then
    . /tmp/xfce5_env
fi

/usr/bin/xfce5-panel &

exec labwc
