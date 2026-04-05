#!/bin/bash
# Wrapper to launch PluginScanServer.app via LaunchServices (open).
# AudioComponentInstanceNew requires the process to be launched as a proper
# macOS app, not via direct execvp. JUCE's launchWorkerProcess uses fork/execvp,
# so this script bridges the gap by using 'open' to start the .app bundle.
#
# 'open' does not reliably forward --args or --env to JUCE app bundles, so
# the IPC pipe name is written to a temp file that the server reads on startup.

SCRIPT_DIR="$(dirname "$0")"
ARGS_DIR="/tmp/syndicate_scan_args"
mkdir -p "$ARGS_DIR"

# Write the pipe args to a file named by this script's PID (unique per launch)
ARGS_FILE="$ARGS_DIR/$$.args"
printf '%s' "$*" > "$ARGS_FILE"

open -n -W "$SCRIPT_DIR/PluginScanServer.app"

# Clean up if the server didn't consume the file (e.g. it crashed)
rm -f "$ARGS_FILE"
