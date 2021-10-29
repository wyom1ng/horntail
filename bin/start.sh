#!/bin/sh

set -e
set -o pipefail

SCRIPT=$(readlink -f "$0")
SCRIPTPATH=$(dirname "$SCRIPT")

(cd $SCRIPTPATH/.. && cmake-build-debug/horntail "$@" | cmake-build-debug/horntail_log)
