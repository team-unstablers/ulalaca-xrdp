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
  XrdpEvent.cpp \
  XrdpTransport.hpp \
  XrdpTransport.cpp \
  XrdpStream.hpp \
  XrdpStream.cpp \
  UlalacaMessages.hpp \
  IPCConnection.hpp \
  IPCConnection.cpp \
  SessionBrokerClient.hpp \
  SessionBrokerClient.cpp \
  ProjectorClient.hpp \
  ProjectorClient.cpp \
  KeycodeMap.cpp \
  ULCGTextRenderer.cpp \
  ULCGTextRenderer.hpp \
  ULSurface.cpp \
  ULSurface.hpp \
  XrdpUlalacaPrivate.cpp \
  XrdpUlalacaPrivate.xrdpmodule.cpp \
  XrdpUlalacaPrivate.debug.cpp \
  XrdpUlalacaPrivate.hpp \
  ulalaca.cpp \
  ulalaca.hpp

AM_LDFLAGS = \
  -ObjC \
  -all_load \
  -framework CoreFoundation \
  -framework Foundation \
  -framework CoreGraphics \
  -framework CoreText

libulalaca_la_LIBADD = \
  $(top_builddir)/common/libcommon.la

# SUBDIRS = tests
