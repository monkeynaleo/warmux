SUBDIRS = .

noinst_LIBRARIES = libwormux.a

libwormux_a_SOURCES = \
	./include/WORMUX_debug.h \
	./include/WORMUX_distant_cpu.h \
	./include/WORMUX_error.h \
	./include/WORMUX_i18n.h \
	./include/WORMUX_network.h \
	./include/WORMUX_player.h \
	./include/WORMUX_point.h \
	./include/WORMUX_rectangle.h \
	./include/WORMUX_singleton.h \
	./include/WORMUX_socket.h \
	./include/WORMUX_team_config.h \
	./include/WORMUX_types.h \
	./include/WORMUX_vector2.h \
	./base/error.cpp \
	./base/i18n.cpp \
	./base/singleton.cpp \
	./net/distant_cpu.cpp \
	./net/network.cpp \
	./net/player.cpp \
	./net/socket.cpp \
	./tools/debug.cpp

AM_CXXFLAGS += -I$(top_srcdir)/lib/wormux/include