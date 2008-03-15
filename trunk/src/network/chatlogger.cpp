#include <string>
#include <iostream>
#include <fstream>
#include "include/base.h"
#include "tool/error.h"
#include "tool/i18n.h"
#include "network/chatlogger.h"
#include "game/config.h"

ChatLogger::ChatLogger() :
        logfile("logs"PATH_SEPARATOR"chat.log"),
        logdir(Config::GetInstance()->GetPersonalDataDir())
{
  // FIXME: also add the game name to the filename
  //
  //  currently, it seems this info doesn't get to the client,
  //  and probably the game_name should be stored after selecting
  //  the game
  std::string fn = logdir + std::string(PATH_SEPARATOR) + logfile ;
  //std::string fn = std::string(".") + std::string(PATH_SEPARATOR) + logfile ; // we use . for debug


  m_logfilename.open(fn.c_str(), std::ios::out | std::ios::app);
  if(!m_logfilename)
  {
    std::string err = Format(_("Couldn't open file %s"), fn.c_str());
    throw err;
  }
}

ChatLogger::~ChatLogger()
{
  m_logfilename.close();
}

void ChatLogger::LogMessage(
        const std::string &author,
        const std::string &msg
    )
{
  std::string txt = author + std::string("> ") + msg;
  LogMessage(txt);
}

void ChatLogger::LogMessage(
        const std::string &msg
    )
{
  m_logfilename << msg << std::endl << std::flush;
}

ChatLogger *ChatLogger::GetInstance()
{
    if ( singleton == NULL ) {
        singleton = new ChatLogger();
        MSG_DEBUG("singleton", "Created singleton %p of type 'ChatLoogger'\n", singleton);
    }
    return singleton;
}
