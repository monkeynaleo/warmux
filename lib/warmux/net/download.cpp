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
#include <cerrno>
#include <stdio.h>
#include <map>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <WARMUX_debug.h>
#include <WARMUX_download.h>
#include <WARMUX_error.h>
#include <WARMUX_i18n.h>
#include <WARMUX_file_tools.h>

static size_t dummy_callback(void*, size_t size, size_t nmemb, void*)
{
  return size*nmemb;
}
#ifdef HAVE_LIBCURL
# include <curl/curl.h>

static size_t download_callback(void* buf, size_t size, size_t nmemb, void* str)
{
  std::string* out = (std::string*)str;
  size_t       sz  = size*nmemb;

  out->append((char*)buf, sz);
  return sz;
}

Downloader::Downloader()
{
#ifdef HAVE_FACEBOOK
  logged = false;
  curl_global_init(CURL_GLOBAL_ALL);
#else
#  ifdef WIN32
  curl_global_init(CURL_GLOBAL_WIN32);
#  else
  curl_global_init(CURL_GLOBAL_NOTHING);
#  endif
#endif

  curl = curl_easy_init();
  curl_error_buf = new char[CURL_ERROR_SIZE];
  curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, curl_error_buf);
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);
  curl_easy_setopt(curl, CURLOPT_CAINFO, NULL);
  curl_easy_setopt(curl, CURLOPT_CAPATH, NULL); 
  curl_easy_setopt(curl, CURLOPT_COOKIEFILE, "");
  curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0 (Windows; U; Windows NT 5.1; en-US; rv:1.8.1.3) Gecko/20070309 Firefox/2.0.0.3");
}

Downloader::~Downloader()
{
  curl_easy_cleanup(curl);

  curl_global_cleanup();
  delete[] curl_error_buf;
}

void Downloader::FillCurlError(int r)
{
  error = std::string(curl_error_buf);
  if (error.empty()) {
    snprintf(curl_error_buf, CURLOPT_ERRORBUFFER-1, "Unknown Curl error no.%i", r);
    error = std::string(curl_error_buf);
  }
}

bool Downloader::GetUrl(const char* url, std::string* out)
{
  curl_easy_setopt(curl, CURLOPT_HTTPGET, 1);
  if (out) {
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, out);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, download_callback);
  } else {
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, NULL);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, dummy_callback);
  }
  curl_easy_setopt(curl, CURLOPT_URL, url);
  int r = curl_easy_perform(curl);
  if (CURLE_OK == r)
    return true;

  FillCurlError(r);
  return false;
}

bool Downloader::Post(const char* url, std::string* out, const std::string& fields)
{
  if (!fields.empty())
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, fields.c_str());
  if (out) {
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, out);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, download_callback);
  } else {
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, NULL);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, dummy_callback);
  }
  curl_easy_setopt(curl, CURLOPT_POST, 1);
  curl_easy_setopt(curl, CURLOPT_URL, url);
  int r = curl_easy_perform(curl);
  if (CURLE_OK == r)
    return true;

  FillCurlError(r);
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

static JavaVM    *vm        = NULL;

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *_vm, void *reserved)
{
  vm = _vm;
  return JNI_VERSION_1_2;
}

JNIEXPORT void JNICALL JNI_OnUnload(JavaVM *_vm, void *reserved)
{
  vm = _vm;
}

static JNIEnv    *env       = NULL;
static jmethodID  FetchURL  = NULL;
static jobject    dler      = NULL;

void
JAVA_EXPORT_NAME(URLDownloader_nativeInitCallbacks)(JNIEnv * libEnv, jobject thiz)
{
  env  = libEnv;
  dler = env->NewGlobalRef(thiz); //Never freed!
  jclass dlerClass = env->GetObjectClass(dler);
  FetchURL = env->GetMethodID(dlerClass, "FetchURL", "(Ljava/lang/String;)[B");
}

};

Downloader::Downloader() { }
Downloader::~Downloader() { }
bool Downloader::GetUrl(const char* url, std::string& out)
{
  bool       ret    = false;
  jboolean   isCopy = JNI_FALSE;
  int        written;

  // Attach to avoid: "JNI ERROR: non-VM thread making JNI calls"
  // Now make sure to properly detach even in case of error
  vm->AttachCurrentThread(&env, NULL);

  jstring    jurl   = env->NewStringUTF(url);
  jbyteArray buffer = (jbyteArray)env->CallObjectMethod(dler, FetchURL, jurl);
  int        len    = env->GetArrayLength(buffer);
  jbyte     *ptr;

  if (!len) {
    error = Format(_("Read only %i bytes"), len);
    goto out;
  }

  ptr = env->GetByteArrayElements(buffer, &isCopy);
  if (!ptr) {
    error = _("No pointer");
    goto out;
  }

  if (len) {
    out.append((char*)ptr, sizeof(jbyte)*len);
    if (isCopy == JNI_TRUE)
      env->ReleaseByteArrayElements(buffer, ptr, 0);
    ret = true;
  } else {
    error = Format(_("Wrote %i/%i bytes"), written, len);
  }

out:
  // Done with JNI calls, detach
  vm->DetachCurrentThread();
  return ret;
}
#else  // waiting for an alternate implementation
Downloader::Downloader() { }
Downloader::~Downloader() { }
bool Downloader::GetUrl(const char* /*url*/, std::string& /*file*/) { return false; }
#endif

bool Downloader::GetLatestVersion(std::string& line)
{
  static const char url[] = "http://www.warmux.org/last";
  error.clear();
  if (!GetUrl(url, &line)) {
    if (error.empty())
      error = Format(_("Couldn't fetch last version from %s"), url);
    fprintf(stderr, "%s\n", error.c_str());
    return false;
  }

  return true;
}

bool Downloader::GetServerList(std::map<std::string, int>& server_lst, const std::string& list_name)
{
  MSG_DEBUG("downloader", "Retrieving server list: %s", list_name.c_str());

  // Download the list of server
  const std::string list_url = "http://www.warmux.org/" + list_name;
  std::string       list_line;

  error.clear();

  if (!GetUrl(list_url.c_str(), &list_line))
    return false;
  MSG_DEBUG("downloader", "Received '%s'", list_line.c_str());

  // Parse the file
  std::stringstream list(list_line);
  std::string       line;

  while (std::getline(list, line)) {
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
    MSG_DEBUG("downloader", "Received %s:%i", hostname.c_str(), port);

    server_lst[ hostname ] = port;
  }

  MSG_DEBUG("downloader", "Server list retrieved. %u servers are running",
            (uint)server_lst.size());

  return true;
}

bool Downloader::FindPair(std::string& value, const std::string& n, const std::string& html)
{
  size_t s = html.find(n);
  if (!s)
    return false;
  s += n.size();
  size_t e = html.find('"', s);
  if (e <= s)
    return false;
  value = html.substr(s, e-s);
  return true;
}

bool Downloader::FindNameValue(std::string& value, const std::string& name, const std::string& html)
{
  return FindPair(value, "name=\"" + name + "\" value=\"", html);
}

#ifdef HAVE_FACEBOOK
bool Downloader::InitFaceBook(const std::string& semail, const std::string& spwd)
{
  if (logged)
    return true;
  std::string html, fields;

  logged = false;
#ifdef HAVE_LIBCURL
  char *email = curl_easy_escape(curl, semail.c_str(), semail.size());
  char *pass  = curl_easy_escape(curl, spwd.c_str(), spwd.size());
#endif
  if (!GetUrl("http://m.facebook.com/login.php?http&refsrc=http%3A%2F%2Fm.facebook.com%2F&no_next_msg&refid=8", &html)) {
    error = _("Can't find login connect");
    goto end;
  }
  MSG_DEBUG("downloader", "Login connect success!");
  
  // Find m_ts value
  if (!FindNameValue(m_ts, "m_ts", html)) {
    error = "Can't find m_ts";
    goto end;
  }
  MSG_DEBUG("downloader", "m_ts=%s", m_ts.c_str());

  // Find post_form_id value
  if (!FindNameValue(post_form_id, "post_form_id", html)) {
    error = "Can't find post_form_id";
    goto end;
  } 
   MSG_DEBUG("downloader", "post_form_id=%s", post_form_id.c_str());
  
  // Find form
  if (!FindPair(form, "id=\"login_form\" action=\"", html)) {
    error = "Can't find form";
    goto end;
  } 
  MSG_DEBUG("downloader", "form=%s", form.c_str());

  html.clear();

  form = "http://m.facebook.com" + form;
  fields = "lsd=&post_form_id=" + post_form_id +
           "&version=1&ajax=0&pxr=0&gps=0&email=" + email + "&pass=" + pass + "&m_ts=" + m_ts + "&login=Login";
  MSG_DEBUG("downloader", "Fields: %s\n", fields.c_str());
  if (!Post(form.c_str(), &html, fields)) {
    error = _("Login failed");
    goto end;
  }
  if (html.find("abb acr aps") != std::string::npos) {
    error = _("Login error, probably invalid email or password");
    goto end;
  }
  MSG_DEBUG("downloader", "Login success!\n");

  // Find form
  if (!FindPair(form, "id=\"composer_form\" action=\"", html)) {
    error = "Can't find form";
    goto end;
  } 
  MSG_DEBUG("downloader", "form=%s", form.c_str());
  
  // Find fb_dtsg
  if (!FindNameValue(fb_dtsg, "fb_dtsg", html)) {
    error = "Can't find fb_dtsg";
    goto end;
  } 
  MSG_DEBUG("downloader", "fb_dtsg=%s", fb_dtsg.c_str());

  // Find post_form_id value
  if (!FindNameValue(post_form_id, "post_form_id", html)) {
    error = "Can't find post_form_id";
    goto end;
  } 
  MSG_DEBUG("downloader", "post_form_id=%s", post_form_id.c_str());
  
  form = "http://m.facebook.com" + form;
  html.clear();

  logged = true;

end:
  free(pass);
  free(email);
#ifdef HAVE_LIBCURL
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, dummy_callback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, NULL);
#endif

  if (!logged && IsLOGGING("download")) {
    FILE *f = fopen("out.htm", "wt");
    fwrite(html.c_str(), html.size(), 1, f);
    fclose(f);
    MSG_DEBUG("downloader", "Login failed: %s\n", error.c_str());
  }

  return logged;
}

bool Downloader::FBStatus(const std::string& text)
{
  if (!logged)
    return false;
  std::string txt;
#ifdef HAVE_LIBCURL
  char *msg = curl_easy_escape(curl, text.c_str(), text.size());
  txt = "fb_dtsg=" + fb_dtsg + "&post_form_id=" + post_form_id + "&status=" + msg + "&update=Update&target=";
  free(msg);
#endif
  MSG_DEBUG("downloader", "fields=%s", txt.c_str());
  return Post(form.c_str(), NULL, txt);
}
#endif
