#!/bin/sh

export XDG_CURRENT_DESKTOP=XFCE5
export XDG_SESSION_TYPE=wayland

export WLR_RENDERER=pixman
export WLR_NO_HARDWARE_CURSORS=1
export LIBGL_ALWAYS_SOFTWARE=1

export GDK_BACKEND=wayland,x11
export QT_QPA_PLATFORM="wayland;xcb"

/usr/bin/xfce5-autoscale-daemon
if [ -f /tmp/xfce5_env ]; then
    . /tmp/xfce5_env
fi

/usr/bin/xfce5-panel &

exec labwc
