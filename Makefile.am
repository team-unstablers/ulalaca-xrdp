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
  UnixSocket.cpp \
  UnixSocket.hpp \
  SocketStream.cpp \
  SocketStream.hpp \
  SystemCallException.cpp \
  SystemCallException.hpp \
  XrdpEvent.hpp \
  XrdpTransport.hpp \
  XrdpTransport.cpp \
  XrdpStream.hpp \
  XrdpStream.cpp \
  UlalacaMessages.hpp \
  ProjectionThread.hpp \
  ProjectionThread.cpp \
  KeycodeMap.cpp \
  ulalaca.cpp \
  ulalaca.hpp

libulalaca_la_LIBADD = \
  $(top_builddir)/common/libcommon.la

