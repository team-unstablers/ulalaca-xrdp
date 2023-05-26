AM_CPPFLAGS = \
  -DXRDP_CFG_PATH=\"${sysconfdir}/xrdp\" \
  -DXRDP_SBIN_PATH=\"${sbindir}\" \
  -DXRDP_SHARE_PATH=\"${datadir}/xrdp\" \
  -DXRDP_PID_PATH=\"${localstatedir}/run\" \
  -I$(top_srcdir)/common

AM_CXXFLAGS = \
  --std=c++17

PACKAGE_STRING = "ulalaca"

TESTS = test_ulalaca_xrdp
check_PROGRAMS = test_ulalaca_xrdp

test_ulalaca_xrdp_SOURCES = \
    catch_amalgamated.cpp \
    catch_amalgamated.hpp \
    test_copyrects.cpp

test_ulalaca_xrdp_CFLAGS = \
    @CHECK_CFLAGS@

test_ulalaca_xrdp_LDFLAGS = -Wl

test_ulalaca_xrdp_LDADD = \
    ../libulalaca.la

