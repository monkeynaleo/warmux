/******************************************************************************
 *  Warmux is a convivial mass murder game.
 *  Copyright (C) 2001-2011 Warmux Team.
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
 *****************************************************************************
 * Retrieve string pasted depending on OS mechanisms.
 *****************************************************************************/

#ifdef _WIN32
#  include <windows.h>
#  include "ansi_convert.h"

std::string ANSIToUTF8(const char* str)
{
  int len = strlen(str);
  LPWSTR buf = (LPWSTR)malloc(2*len+2);

  if (buf) {
    int wlen = MultiByteToWideChar(CP_ACP, 0, str, len, buf, 2*len+2);
    if (wlen) {
      int utf_len = WideCharToMultiByte(CP_UTF8, 0, buf, wlen, NULL, 0, NULL, NULL);
      if (utf_len > 0) {
        // Convert from UTF-16 to UTF-8
        std::string tmp;
        tmp.resize(utf_len);
        if (WideCharToMultiByte(CP_UTF8, 0, buf, wlen, (LPSTR)tmp.c_str(), utf_len, NULL, NULL)) {
          free(buf);
          return tmp;
        }
      }
      free(buf);
    }
  }

  return "";
}
#endif
