SUBDIRS = .

noinst_LIBRARIES = libwormux.a

libwormux_a_SOURCES = \
	./include/WORMUX_error.h \
	./include/WORMUX_i18n.h \
	./include/WORMUX_network.h \
	./include/WORMUX_socket.h \
	./include/WORMUX_types.h \
	./base/error.cpp \
	./base/i18n.cpp \
	./net/network.cpp \
	./net/socket.cpp

AM_CXXFLAGS += -I$(top_srcdir)/lib/wormux/include