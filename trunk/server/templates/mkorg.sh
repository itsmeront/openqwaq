#/bin/bash
###############################################
BASE=/home/openqwaq/server/templates
mkdir -p "$1"
(cd "$1"; tar -xf $BASE/neworg.tar)

