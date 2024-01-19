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
  Utility.hpp \
  Utility.cpp \
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
  XrdpStream.template.cpp \
  channels/XrdpChannelBase.hpp \
  channels/XrdpChannelBase.cpp \
  channels/XrdpChannelEvent.hpp \
  channels/XrdpChannelEvent.cpp \
  channels/clipboard/ClipboardEntry.hpp \
  channels/clipboard/ClipboardEntry.cpp \
  channels/clipboard/ClipboardChannel.hpp \
  channels/clipboard/ClipboardChannel.cpp \
  UlalacaMessages.hpp \
  IPCConnection.hpp \
  IPCConnection.cpp \
  SessionBrokerClient.hpp \
  SessionBrokerClient.cpp \
  ProjectorClient.hpp \
  ProjectorClient.cpp \
  KeycodeMap.cpp \
  ULSurface.cpp \
  ULSurface.hpp \
  XrdpUlalacaPrivate.cpp \
  XrdpUlalacaPrivate.xrdpmodule.cpp \
  XrdpUlalacaPrivate.hpp \
  ulalaca.cpp \
  ulalaca.hpp

libulalaca_la_LIBADD = \
  $(top_builddir)/common/libcommon.la

# SUBDIRS = tests
