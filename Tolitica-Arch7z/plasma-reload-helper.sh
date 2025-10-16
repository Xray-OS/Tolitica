#!/bin/bash
# Plasma Theme Reload Helper Script

# Kill plasmashell
killall plasmashell

# Clear all plasma caches
rm -rf ~/.cache/plasma*
rm -rf ~/.cache/ksvg*
rm -rf ~/.cache/icon*

# Force KDE configuration reload
kbuildsycoca6

# Restart plasmashell
nohup plasmashell > /dev/null 2>&1 &

echo "Plasma reloaded successfully"