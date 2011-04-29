XCPPFLAGS=-I$(topdir)/platforms/Cross/plugins/PyBridge -I$(topdir)/platforms/Cross/third-party/Python-2.4.6/Include
XCFLAGS=-pthread
XLDFLAGS=-L../../python2.4-2.4.6 -lpthread
