XCPPFLAGS=-pthread\
 -I$(topdir)/platforms/Cross/plugins/QwaqLib\
 -I$(topdir)/platforms/Cross/third-party/boost_1_40_0\
 -I../../third-party/MainConcept/AVCSDK-LX32_8.6.0_LINUX/include

# -Bsymbolic to get self-contained shared objects.  This should be in libtool
# construction but we don't control e.g. /usr/share/libtool/ltmain.sh. Sigh
PLIBS=-Wl,-Bsymbolic\
 -lpthread\
 -ldl\
 -L../../third-party/MainConcept/AVCSDK-LX32_8.6.0_LINUX/bin\
  -lmc_dec_avc\
  -lmc_std_enc_avc\
 -L../../libboost\
  -lboost_thread
