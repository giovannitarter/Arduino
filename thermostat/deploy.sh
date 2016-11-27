#!/bin/sh


VERSION_FILE="VERSION"

VERSION=$(cat $VERSION_FILE)
NEXT_VERSION=$(expr $VERSION + 1)
echo $NEXT_VERSION > $VERSION_FILE

make clean
FW_VERSION="$NEXT_VERSION" make
cp out/thermostat.bin httpserver/
echo $NEXT_VERSION > httpserver/thermostat.bin.version
