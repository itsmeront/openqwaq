# BUILDING ON OS X:
# Prerequisites ------------------------------------------------------
# The following libraries need to be installed in /opt/local/lib:
#   - libboost_thread-mt.a
#   - libz.a
#   - libbz.a
#   - libspeex.a
#   - libportaudio.a
# They can most easily be installed via MacPorts.  Be sure to use the +universal parameter, since the plugin is built as 32-bit, not 64-bit.
# eg: sudo port install zlib +universal


# Building X264 (OS X)------------------------------------------------------
# (optional step; pre-compiled binaries are provided) ----------------------
# Install the following via MacPorts:
# - libyasm
# If you're using XCode 4.0, you'll need to use the 10.6 SDK instead of 10.5.  Unfortunately, this means that your plugin won't work for 10.5 users (still nearly a quarter of users).
export CFLAGS="-m32 -isysroot /Developer/SDKs/MacOSX10.5.sdk -mmacosx-version-min=10.5"
./configure --host=i686-darwin 

# The previous step will fail to build the x264 executable, because
# for some reason it still tries to build a 64-bit exe, even though
# we jumped through hoops to build 32-bit libs.  We don't care; 
# we just need the lib anyway.  
make install-lib-static

# Finally, copy the static lib to macbuild/third-party

# Building X264 (Windows)------------------------------------------------------
# (optional step; pre-compiled binaries are provided) -------------------------
# - Install MinGW and yasm (from mingw.org and yasm.tortall.net)
# - copy yasm.exe into your MinGW/bin directory
export CFLAGS="-m32"
./configure --enable-shared 
# use the resulting libx264.dll.a and libx264-115.dll 
# - the former is an "import library" that is statically linked with our plugin


# Building LIBAVCODEC (OS X) --------------------------------------------------
# (optional step; pre-compiled binaries are provided) -------------------------
mkdir install-dir

# We're using libx264 directly, so don't bother including it in libavcodec.
#./configure --prefix=./install-dir --extra-cflags="-arch i386" --extra-ldflags='-arch i386' --arch=x86_32 --target-os=darwin --enable-cross-compile --disable-everything --enable-decoder=h264 --enable-decoder=aac --enable-decoder=aac_latm --enable-encoder=aac --enable-encoder=libx264 --enable-libx264 --enable-gpl --enable-parser=aac --enable-parser=aac_latm --enable-parser=h264
./configure --prefix=./install-dir --extra-cflags="-arch i386" --extra-ldflags='-arch i386' --arch=x86_32 --target-os=darwin --enable-cross-compile --disable-everything --enable-decoder=h264 --enable-decoder=aac --enable-decoder=aac_latm --enable-encoder=aac --enable-parser=aac --enable-parser=aac_latm --enable-parser=h264

make install

# then copy the newly-built libs somewhere
