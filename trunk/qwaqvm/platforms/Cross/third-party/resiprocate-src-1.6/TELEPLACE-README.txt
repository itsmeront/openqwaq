These are notes about how I built resiprocate libraries on OS X.

See TELEPLACE-CONFIG.txt for info about the options I gave to ./configure.  I built and checked in debug-binaries, but these should eventually be replaced with release builds (./configure allows you to make this choice interactively).  There's a dependency on libpopt that I satisfied with MacPorts (hence my use of /opt/local/include and /opt/local/lib in ./configure).

build/Makefile.tools was modified to force 32-bit libraries to be built, instead of the 64-bit default.

You either need to build as root, or specify the install path to be something other than /usr/local (some parts of the library seem to require that other parts already be installed in the right place, or build errors occur).

Have fun!
Josh
