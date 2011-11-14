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
 ******************************************************************************
 * Download a file using libcurl
 *****************************************************************************/

#ifndef DOWNLOAD_H
#define DOWNLOAD_H

#include <stdio.h> // for FILE...
#include <map>
#include <string>

// Load config about download librart (curl, ...)
#include "WARMUX_config.h"

#include <WARMUX_singleton.h>

#define HAVE_FACEBOOK 1

class Downloader : public Singleton<Downloader>
{
  std::string error;

#ifdef HAVE_LIBCURL
  void* curl;
  char* curl_error_buf;
  void FillCurlError(int r);
#endif

#ifdef HAVE_FACEBOOK
  bool        logged;
  std::string fb_dtsg, post_form_id, form, m_ts;
  static bool FindPair(std::string& value, const std::string& n, const std::string& html);
  static bool FindNameValue(std::string& value, const std::string& name, const std::string& html);
#endif

  // Return true if the download was successful
  bool GetUrl(const char* url, std::string* out);
  bool Post(const char* url, std::string* out, const std::string& fields = "");

protected:
  friend class Singleton<Downloader>;
  Downloader();
  ~Downloader();

public:
#ifdef HAVE_FACEBOOK
  bool InitFaceBook(const std::string& email, const std::string& pwd);
  bool FBStatus(const std::string& text);
#endif
  bool GetLatestVersion(std::string& line);
  bool GetServerList(std::map<std::string, int>& server_lst, const std::string& list_name);

  const std::string& GetLastError() const { return error; };
};

#endif

