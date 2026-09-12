#!/bin/bash

source .env

# Base mounts
MOUNTS="-v $PWD:/app \
    -v $SDK_PATH:/sdk"

# Add TBG_DISC_ROOT mount if the variable is set
if [ -n "$TBG_DISC_ROOT" ]; then
    MOUNTS="$MOUNTS -v $TBG_DISC_ROOT:/tbgdisc"
fi

# The container outlives the client when this script is killed (e.g. by
# `timeout`), and a build left running behind everyone's back collides with
# every later one. Name it so it can be torn down.
NAME="tbg-run-$$"
trap 'docker rm -f "$NAME" >/dev/null 2>&1' EXIT INT TERM

# Backgrounded so a signal reaches the trap right away; bash defers traps until
# a foreground child returns.
docker run --rm --name "$NAME" \
    $MOUNTS \
    -w /app \
    -u 1000 \
    -e TERM=$TERM \
    lhsazevedo/tbg-decomp "$@" &

wait $!
