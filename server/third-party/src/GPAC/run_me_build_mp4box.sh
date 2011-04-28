#!/bin/bash
# this will unpack, build and install in the correct location

linkerPathFile="/home/OpenQWAQ/server/etc/OpenQWAQ-mp4box.conf"
prog=gpac-0.4.5.tar.gz
vers=gpac

SCRIPT_DIR=$(cd `dirname $0` && pwd)
cd $SCRIPT_DIR
tar zxvf $prog
cd $vers

sh ./configure --disable-oss-audio --disable-x11xv --disable-x11-shm --disable-wx --disable-opengl --use-js=no --use-ft=no --use-xvid=no --use-ogg=no --use-vorbis=no --use-theora=no --open-jpeg=no --prefix=$SCRIPT_DIR/../.. && make &&  make install
