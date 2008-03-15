#include <string>
#include <iostream>
#include <fstream>
#include "include/base.h"
#include "tool/error.h"
#include "tool/i18n.h"
#include "network/chatlogger.h"
#include "game/config.h"
#include <time.h>
#ifdef DEBUG
#  include <stdlib.h>
#endif


// the year that time has as base; although time(2)
// says the reference is 1970, tests show is 1900. WHY?
#define TIME_BASE_YEAR 1900 //FIXME: find out why this is not 1970


ChatLogger::ChatLogger() :
        logdir(Config::GetInstance()->GetPersonalDataDir() + std::string(PATH_SEPARATOR"logs")),
        logfile(logdir.c_str() + std::string(PATH_SEPARATOR"chat.log"))
{
  // FIXME: also add the game name to the filename
  //
  //  currently, it seems this info doesn't get to the client,
  //  and probably the game_name should be stored after selecting
  //  the game
  time_t t;
  struct tm lt, *plt;
  std::string timestamp;

  if ( ((time_t) -1) == time(&t) ) {
      timestamp = std::string (_("(unknown time)")) ;
  }
  else {
      // convert to local time
      plt = localtime(&t);
      memcpy(&lt, plt, sizeof(struct tm));

      timestamp = Format ( "%.4d-%.2d-%.2d-%.2d:%.2d:%.2d" ,
        lt.tm_year + TIME_BASE_YEAR, lt.tm_mon+1, lt.tm_mday+1,
        lt.tm_hour, lt.tm_min, lt.tm_sec ) ;

#ifndef DEBUG
      logfile = Format ( "%s.log" , timestamp.c_str() );
#else // DEBUG
      logfile = Format ( "%s-%c.log" , timestamp.c_str(), (char)((random() % 10)+'a') );
#endif // DEBUG

  }

  // TRANSLATORS: after this string there will be a time stamp or the string '(unknown time)'
  timestamp = std::string(_("New network game at ")) + timestamp ;

  std::string fn = logdir + std::string(PATH_SEPARATOR) + logfile ;


  m_logfilename.open(fn.c_str(), std::ios::out | std::ios::app);
  if(!m_logfilename)
  {
    std::string err = Format(_("Couldn't open file %s"), fn.c_str());
    throw err;
  }

  this->LogMessage(timestamp);

}

ChatLogger::~ChatLogger()
{
  m_logfilename.close();
}

void ChatLogger::LogMessage(
        const std::string &msg
    )
{
  m_logfilename << msg << std::endl << std::flush;
}

void ChatLogger::LogMessageIfOpen(
        const std::string &msg
    )
{
  if ( singleton ) ChatLogger::GetInstance()->LogMessage(msg);
}

ChatLogger *ChatLogger::GetInstance()
{
    if ( singleton == NULL ) {
        singleton = new ChatLogger();
        MSG_DEBUG("singleton", "Created singleton %p of type 'ChatLoogger'\n", singleton);
    }
    return singleton;
}

void ChatLogger::CloseIfOpen()
{
    if ( singleton ) ChatLogger::GetInstance()->CleanUp();
}
