# See unixbuild/HowToBuildClient.
# boost speex and openal are all built by us instead of taken from linux.

XCPPFLAGS=-pthread\
 -DEXCLUDE_IAX=1 -DEXCLUDE_PORTAUDIO=1\
 -I$(topdir)/platforms/Cross/third-party\
 -I$(topdir)/platforms/Cross/third-party/boost_1_40_0\
 -I$(topdir)/platforms/Cross/plugins/QwaqLib\
 -I$(topdir)/platforms/unix/third-party/openal-soft-1.10.622/include\
 -I../../libspeex/include\
 -I../../third-party/MainConcept/AACSDK-LX32_8.6.0_LINUX/include

# -Bsymbolic to get self-contained shared objects.  This should be in libtool
# construction but we don't control e.g. /usr/share/libtool/ltmain.sh. Sigh
PLIBS=-Wl,-Bsymbolic\
 -lpthread\
 -lasound\
 -L../../libopenal\
  -lopenal\
 -L../../third-party/MainConcept/AACSDK-LX32_8.6.0_LINUX/bin\
  -lmc_dec_aac\
  -lmc_enc_aac\
 -L../../libspeex/lib\
  -lspeex\
  -lspeexdsp\
 -L../../libboost\
  -lboost_thread
