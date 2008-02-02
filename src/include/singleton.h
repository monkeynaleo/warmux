/******************************************************************************
 *  Wormux is a convivial mass murder game.
 *  Copyright (C) 2001-2008 Wormux Team.
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
 * Declare base classes for singleton pattern design implementation.
 *****************************************************************************/

#ifndef SINGLETON_H
#define SINGLETON_H

#include <list>

// Forward declarations
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

