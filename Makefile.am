AM_CPPFLAGS = \
  -DXRDP_CFG_PATH=\"${sysconfdir}/xrdp\" \
  -DXRDP_SBIN_PATH=\"${sbindir}\" \
  -DXRDP_SHARE_PATH=\"${datadir}/xrdp\" \
  -DXRDP_PID_PATH=\"${localstatedir}/run\" \
  -I$(top_srcdir)/common

AM_CXXFLAGS = \
  --std=c++17

module_LTLIBRARIES = \
  libulalaca.la

libulalaca_la_SOURCES = \
  ulalaca.cpp \
  ulalaca.hpp \
  XrdpStream.cpp

libulalaca_la_LIBADD = \
  $(top_builddir)/common/libcommon.la

libulalaca_la_LDFLAGS = -avoid-version -module
