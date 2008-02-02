#include "include/singleton.h"
#include "tool/debug.h"

SingletonList singletons;

BaseSingleton::BaseSingleton()
{
  singletons.push_back(this);
  MSG_DEBUG("singleton", "Added %p\n", this);
}

BaseSingleton::~BaseSingleton()
{
  singletons.remove(this);
  MSG_DEBUG("singleton", "Removed %p\n", this);
}

void BaseSingleton::ReleaseSingletons()
{
  SingletonList copy(singletons);
  for (SingletonList::iterator it = copy.begin();
       it != copy.end();
       ++it)
  {
    MSG_DEBUG("singleton", "Releasing %p\n", *it);
    delete (*it);
  }
}
