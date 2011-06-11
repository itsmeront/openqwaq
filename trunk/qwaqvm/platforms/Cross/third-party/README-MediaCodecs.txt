# BUILDING ON OS X:

# Building X264 ------------------------------------------------------
export CFLAGS="-m32 -isysroot /Developer/SDKs/MacOSX10.5.sdk -mmacosx-version-min=10.5"
./configure --host=i686-darwin 

# The previous step will fail to build the x264 executable, because
# for some reason it still tries to build a 64-bit exe, even though
# we jumped through hoops to build 32-bit libs.  We don't care; 
# we just need the lib anyway.  
make install-lib-static

# Finally, copy the static lib to macbuild/third-party


# Building LIBAVCODEC ------------------------------------------------
mkdir install-dir

./configure --prefix=./install-dir --extra-cflags="-arch i386" --extra-ldflags='-arch i386' --arch=x86_32 --target-os=darwin --enable-cross-compile --enable-libx264 --enable-gpl

make install

# then copy the newly-built libs somewhere
