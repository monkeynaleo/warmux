#ifndef CHATLOGGER_H
#define CHATLOGGER_H

#include <iostream>
#include <fstream>
#include "include/singleton.h"

class ChatLogger: public Singleton<ChatLogger>
{
 public:
  ChatLogger();
  ~ChatLogger();
  void LogMessage(const std::string& author, const std::string& msg);
  void LogMessage(const std::string& msg);
  static ChatLogger * GetInstance();
 private:
  friend class Singleton<ChatLogger>;
  friend class DistantComputer;
  friend class NetworkClient;

  std::string logfile;
  std::string logdir;
  std::ofstream m_logfilename;
};

#endif //CHATLOGGER_H
