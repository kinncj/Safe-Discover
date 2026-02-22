#!/bin/bash
# Safe Discover privileged helper script
# Only whitelisted commands are allowed to run as root.

set -euo pipefail

if [ $# -lt 1 ]; then
    echo "Error: No command specified" >&2
    exit 1
fi

COMMAND="$1"
shift

case "$COMMAND" in
    pacman)
        # Only allow specific pacman operations
        case "${1:-}" in
            -S|--sync|-Syu|-Rns)
                exec /usr/bin/pacman "$@"
                ;;
            *)
                echo "Error: Disallowed pacman operation: ${1:-}" >&2
                exit 1
                ;;
        esac
        ;;
    fwupdmgr)
        # Only allow specific fwupdmgr operations
        case "${1:-}" in
            refresh|update|get-devices|get-updates)
                exec /usr/bin/fwupdmgr "$@"
                ;;
            *)
                echo "Error: Disallowed fwupdmgr operation: ${1:-}" >&2
                exit 1
                ;;
        esac
        ;;
    *)
        echo "Error: Disallowed command: $COMMAND" >&2
        exit 1
        ;;
esac
