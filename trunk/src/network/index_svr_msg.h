/******************************************************************************
 *  Wormux is a convivial mass murder game.
 *  Copyright (C) 2001-2004 Lawrence Azzoug.
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
 ******************************************************************************
 * Notify a top server of an opened wormux server
 * Obtain information about running games from a top server
 *****************************************************************************/

#ifndef TOP_SERVER_MSG_H
#define TOP_SERVER_MSG_H

enum IndexServerMsg
{
  TS_NO_MSG, // Don't send this
  TS_MSG_VERSION, // Send this to identify the server
  TS_MSG_HOSTING, // Send this if we are hosting a game
  TS_MSG_GET_LIST, // Send this to obtain the list of the current hosting games
  TS_LAST_MSG // Don't send this
};

#endif

