#!/bin/bash

# This is meant to document how to unpack
# the SVN checkout and make it a runnable VM.

# Copy to /Applications, excluding the .svn subdirs.
tar cf - --exclude .svn ./OpenQwaq.app | (cd /Applications; tar xf -)

# All the sub-content frameworks and the exe need to be executable;
# we just do everything so we don't miss anything.
cd /Applications/OpenQwaq.app
chmod -R a+x ./*

