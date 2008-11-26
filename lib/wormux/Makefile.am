SUBDIRS = .

noinst_LIBRARIES = libwormux.a

libwormux_a_SOURCES = \
	./include/WORMUX_debug.h \
	./include/WORMUX_error.h \
	./include/WORMUX_i18n.h \
	./include/WORMUX_network.h \
	./include/WORMUX_point.h \
	./include/WORMUX_rectangle.h \
	./include/WORMUX_singleton.h \
	./include/WORMUX_socket.h \
	./include/WORMUX_types.h \
	./include/WORMUX_vector2.h \
	./base/error.cpp \
	./base/i18n.cpp \
	./base/singleton.cpp \
	./net/network.cpp \
	./net/socket.cpp \
	./tools/debug.cpp

AM_CXXFLAGS += -I$(top_srcdir)/lib/wormux/include