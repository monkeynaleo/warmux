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
 ******************************************************************************
 * Download a file using libcurl
 *****************************************************************************/

#ifndef DOWNLOAD_H
#define DOWNLOAD_H

#include <map>

// Load config about download librart (curl, ...)
#ifdef _MSC_VER
#  include "msvc/config.h"
#else
#  include "config.h"
#endif

#ifdef HAVE_LIBCURL
// Actual definition from curl.h
typedef void CURL;
#endif

#include <WORMUX_singleton.h>

class Downloader : public Singleton<Downloader>
{
  /* If you need this, implement it (correctly) */
  Downloader(const Downloader&);
  const Downloader& operator=(const Downloader&);
  /**********************************************/

#ifdef HAVE_LIBCURL
  CURL* curl;
#endif

  // Return true if the download was successful
  bool Get(const char* url, FILE* file) const;

protected:
  friend class Singleton<Downloader>;
  Downloader();
  ~Downloader();

public:
  std::string GetLatestVersion() const;
  std::map<std::string, int> GetServerList(std::string list_name) const;
};

#endif

