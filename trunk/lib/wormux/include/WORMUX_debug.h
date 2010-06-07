/******************************************************************************
 *  Wormux is a convivial mass murder game.
 *  Copyright (C) 2001-2010 Wormux Team.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 ******************************************************************************/

#ifndef WORMUX_DEBUG_H
#define WORMUX_DEBUG_H

#include <string>

#ifdef _MSC_VER
#  define __PRETTY_FUNCTION__  __FUNCTION__
#endif

/** Usage example :
 *
 * MSG_DEBUG( "game.pause", "Salut %s", "Truc" )
 *
 * MSG_DEBUG use standart printf style for the message.
 *
 * A debug will be printed only if there is a mode game or game.pause. A mode can be added by running wormux with:
 * ./wormux -d game        # print all messages in game section
 * ./wormux -d ""          # print all debug messages
 *
 */
#ifdef WMX_LOG
#  define MSG_DEBUG(LEVEL, MESSAGE, ...) \
   PrintDebug( __FILE__, __FUNCTION__, __LINE__, LEVEL, MESSAGE, ## __VA_ARGS__)
#else
#  define MSG_DEBUG(...) do {} while (0)
#endif

// RTTI is only based on debug, not just WMX_LOG
#ifdef DEBUG
#  define MSG_DBG_RTTI  MSG_DEBUG
#else
#  define MSG_DBG_RTTI(...) do {} while (0)
#endif

extern bool debug_all;

void PrintDebug (const char *filename, const char *function, unsigned long line,
                 const char *level, const char *message, ...);
void AddDebugMode(const std::string& mode);

#ifdef WMX_LOG
bool IsLOGGING(const char* mode);
#else
#  define IsLOGGING(a) false
#endif

#endif
