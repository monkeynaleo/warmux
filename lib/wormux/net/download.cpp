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
#include <cerrno>
#include <stdio.h>
#include <map>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <WORMUX_debug.h>
#include <WORMUX_download.h>
#include <WORMUX_error.h>
#include <WORMUX_i18n.h>
#include <WORMUX_file_tools.h>

#ifdef HAVE_LIBCURL
# include <curl/curl.h>

static size_t download_callback(void* buf, size_t size, size_t nmemb, void* fd)
{
  return fwrite(buf, size, nmemb, (FILE*)fd);
}

Downloader::Downloader():
  curl(curl_easy_init())
{
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, download_callback);
  curl_error_buf = new char[CURL_ERROR_SIZE];
  curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, curl_error_buf);
}

Downloader::~Downloader()
{
  curl_easy_cleanup(curl);
}

bool Downloader::Get(const char* url, FILE* file)
{
  curl_easy_setopt(curl, CURLOPT_FILE, file);
  curl_easy_setopt(curl, CURLOPT_URL, url);
  CURLcode r = curl_easy_perform(curl);
  fflush(file);

  if (r == CURLE_OK)
    return true;

  error = std::string(curl_error_buf);
  return false;
}
#elif defined(ANDROID)
# include <jni.h>

#ifndef SDL_JAVA_PACKAGE_PATH
# error SDL_JAVA_PACKAGE_PATH undefined!
#endif
#define JAVA_EXPORT_NAME2(name,package) Java_##package##_##name
#define JAVA_EXPORT_NAME1(name,package) JAVA_EXPORT_NAME2(name,package)
#define JAVA_EXPORT_NAME(name) JAVA_EXPORT_NAME1(name,SDL_JAVA_PACKAGE_PATH)

extern "C" {

static JNIEnv    *env       = NULL;
static jmethodID  FetchURL  = NULL;
static jobject    dler      = NULL;

void
JAVA_EXPORT_NAME(URLDownloader_nativeInitCallbacks)(JNIEnv * libEnv, jobject thiz)
{
  env  = libEnv;
  dler = thiz;
  jclass dlerClass = env->GetObjectClass(thiz);
  FetchURL = env->GetMethodID(dlerClass, "FetchURL", "(Ljava/lang/String;)[B");
}

};

Downloader::Downloader() { }
Downloader::~Downloader() { }
bool Downloader::Get(const char* url, FILE* file)
{
  jboolean   isCopy = JNI_FALSE;
  jstring    jurl   = env->NewStringUTF(url);
  jbyteArray buffer = (jbyteArray)env->CallObjectMethod(dler, FetchURL, jurl);
  int        len    = env->GetArrayLength(buffer);

  if (!len)
    return false;

  jbyte *ptr = env->GetByteArrayElements(buffer, &isCopy);
  if (!ptr)
    return false;

  bool success = fwrite(ptr, len, sizeof(jbyte), file) == len;
  if (isCopy == JNI_TRUE)
    env->ReleaseByteArrayElements(buffer, ptr, 0);
  return success;
}
#else  // waiting for an alternate implementation
Downloader::Downloader() { }
Downloader::~Downloader() { }
bool Downloader::Get(const char* /*url*/, FILE* /*file*/) { return false; }
#endif

static ssize_t getline(std::string& line, FILE* file)
{
  line.clear();
  char buffer[1024];

  int r = fscanf(file, "%1024s\n", buffer);
  if (r == 1)
    line = buffer;

  return line.size();
}

bool Downloader::GetLatestVersion(std::string& line)
{
  static const char url[] = "http://www.wormux.org/last";
  int fd;
  const std::string last_file = CreateTmpFile("wormux_version", &fd);

  if (fd == -1) {
    error = Format(_("Fail to create temporary file: %s"), strerror(errno));
    fprintf(stderr, "%s\n", error.c_str());
    return false;
  }

  FILE* file = fdopen(fd, "r+");
  if (!file) {
    error = Format(_("Fail to open temporary file: %s"), strerror(errno));
    fprintf(stderr, "%s\n", error.c_str());
    return false;
  }

  if (!Get(url, file)) {
    error = Format(_("Couldn't fetch last version from %s"), url);
    fprintf(stderr, "%s\n", error.c_str());
    return false;
  }

  // Parse the file
  rewind(file);
  getline(line, file);
  fclose(file);

  // remove the file
  remove(last_file.c_str());

  return true;
}

bool Downloader::GetServerList(std::map<std::string, int>& server_lst, const std::string& list_name)
{
  MSG_DEBUG("downloader", "Retrieving server list: %s", list_name.c_str());

  // Download the list of server
  const std::string list_url = "http://www.wormux.org/" + list_name;
  int fd;
  const std::string server_file = CreateTmpFile("wormux_servers", &fd);

  if (fd == -1) {
    error = Format(_("Fail to create temporary file: %s"), strerror(errno));
    fprintf(stderr, "%s\n", error.c_str());
    return false;
  }

  FILE* file = fdopen(fd, "r+");
  if (!Get(list_url.c_str(), file))
    return false;

  // Parse the file
  std::string line;
  rewind(file);

  // GNU getline isn't available on *BSD and Win32, so we use a new function, see getline above
  while (getline(line, file) > 0) {
    if (line.at(0) == '#'
        || line.at(0) == '\n'
        || line.at(0) == '\0')
      continue;

    std::string::size_type port_pos = line.find(':', 0);
    if (port_pos == std::string::npos)
      continue;

    std::string hostname = line.substr(0, port_pos);
    std::string portstr = line.substr(port_pos+1);
    int port = atoi(portstr.c_str());

    server_lst[ hostname ] = port;
  }

  fclose(file);
  remove(server_file.c_str());

  MSG_DEBUG("downloader", "Server list retrieved. %i servers are running", server_lst.size());

  return true;
}
