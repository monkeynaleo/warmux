/******************************************************************************
 *  Wormux is a convivial mass murder game.
 *  Copyright (C) 2001-2004 Lawrence Azzoug.
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
 * Affiche un message dans le jeu, puis pose une question dans le jeu ou
 * attend au moins la pression d'une touche.
 *****************************************************************************/

#ifndef QUESTION_H
#define QUESTION_H
//-----------------------------------------------------------------------------
#include "../include/base.h"
#include "../graphic/sprite.h"
#include <string>
#include <list>
#include <SDL_events.h>

//-----------------------------------------------------------------------------

class Question
{
  Sprite* background;

  // A choice = a key return a value
  class choice_t
  {
    private:
      int m_key;
      int m_val;
    public:
      choice_t (int key, int value)
      { m_key = key; m_val = value; };
      inline const int & key() const { return m_key; };
      inline const int & val() const { return m_val; };
  };

  // Choices list
  std::list<choice_t> choices;
  typedef std::list<choice_t>::iterator choice_iterator;
  // Default choice used when another key is pressed
  struct s_default_choice
  {
    bool active;
    int value;
  } default_choice;

  int TreatsKey (SDL_Event &event);
  void Draw();
  // Message to display
  std::string message;

public:
  Question();
  ~Question();

  void Set(const std::string &message,
	   bool default_active,
	   int default_value,
      const std::string &bg_sprite="");
  int AskQuestion ();
  inline void add_choice(int key, int value)
  {
    return this->choices.push_back(choice_t(key,value));
  }

};

//-----------------------------------------------------------------------------
#endif
