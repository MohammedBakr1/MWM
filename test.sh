#!/bin/sh

DISPLAY_NUM=2
RES=1280x800

echo "Stopping any running mwm and Xephyr instances..."
pkill -9 mwm 2>/dev/null
pkill -9 Xephyr 2>/dev/null
sleep 1

echo "Removing stale X lock files..."
rm -f /tmp/.X${DISPLAY_NUM}-lock
rm -f /tmp/.X11-unix/X${DISPLAY_NUM}

if [ ! -f ./mwm ]; then
    echo "mwm binary not found in current directory. Building..."
    make
fi

echo "Starting Xephyr on display :${DISPLAY_NUM}..."
Xephyr -screen ${RES} :${DISPLAY_NUM} &
XEPHYR_PID=$!

sleep 2

if ! kill -0 ${XEPHYR_PID} 2>/dev/null; then
    echo "Xephyr failed to start. Check the error above."
    exit 1
fi

echo "Xephyr is running. Starting mwm on display :${DISPLAY_NUM}..."
DISPLAY=:${DISPLAY_NUM} ./mwm &
MWM_PID=$!

sleep 1

if ! kill -0 ${MWM_PID} 2>/dev/null; then
    echo "mwm failed to start. Check the error above."
    exit 1
fi

echo "mwm is running inside Xephyr (display :${DISPLAY_NUM})."
echo "Opening a terminal inside the session..."
DISPLAY=:${DISPLAY_NUM} st &

echo ""
echo "Ready. Super+Return opens a terminal, Super+Shift+e quits mwm."
echo "To stop everything: pkill Xephyr"
