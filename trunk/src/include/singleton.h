#ifndef SINGLETON_H
#define SINGLETON_H

#include <list>

class BaseSingleton;

typedef std::list<BaseSingleton*> SingletonList;
extern SingletonList singletons;

class BaseSingleton
{
protected:
  BaseSingleton();
  virtual ~BaseSingleton();

public:
  static void ReleaseSingletons();
};

template<typename T>
class Singleton : public BaseSingleton
{
protected:
  static T* singleton;
  ~Singleton() { singleton = NULL; }
public:
  static T* GetInstance()
  {
    if (!singleton) singleton = new T();
    return singleton;
  }
  static const T* GetConstInstance() { return GetInstance(); }

  static T& GetRef() { return *GetInstance(); }
  static const T& GetConstRef() { return *GetInstance(); }

  static void CleanUp() { if (singleton) delete singleton; }
};

template<typename T>
T* Singleton<T>::singleton = NULL;

#endif // SINGLETON_H

#ifndef SINGLETON_H
#define SINGLETON_H

#include <list>
#include "tool/debug.h"

class BaseSingleton;

typedef std::list<BaseSingleton*> SingletonList;
extern SingletonList singletons;

class BaseSingleton
{
public:
  BaseSingleton()
  {
    singletons.push_back(this);
    MSG_DEBUG("singleton", "Added %p\n", this);
  }
  virtual ~BaseSingleton()
  {
    singletons.remove(this);
    MSG_DEBUG("singleton", "Removed %p\n", this);
  }
  static void ReleaseSingletons()
  {
    SingletonList copy(singletons);
    for (SingletonList::iterator it = copy.begin();
         it != copy.end();
         ++it)
    {
      MSG_DEBUG("singleton", "Releasing %p\n", *it);
      delete (*it);
    }
  };
};

template<typename T>
class Singleton : public BaseSingleton
{
protected:
  static T* singleton;
  ~Singleton() { singleton = NULL; }
public:
  static T* GetInstance()
  {
    if (!singleton)
    {
      singleton = new T();
      MSG_DEBUG("singleton", "Singleton is %p\n", singleton);
    }
    return singleton;
  }
  static const T* GetConstInstance() { return GetInstance(); }

  static T& GetRef() { return *GetInstance(); }
  static const T& GetConstRef() { return *GetInstance(); }

  static void CleanUp() { if (singleton) delete singleton; }
};

template<typename T>
T* Singleton<T>::singleton = NULL;

#endif // SINGLETON_H

