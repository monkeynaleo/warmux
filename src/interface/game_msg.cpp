/******************************************************************************
 *  Wormux, a free clone of the game Worms from Team17.
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
 * Messages s'affichant en haut de l'ecran (et écrit dans la console).
 *****************************************************************************/

#include "game_msg.h"
#include <iostream>
#include "../game/time.h"
#include "../graphic/video.h"
#include "../graphic/font.h"
#include "../include/app.h"
#include "game_msg.h"

// Hauteur de la police de caractere "mini"
#define HAUT_POLICE_MINI 12 // pixels

// Interligne police "mini" (pour les messages)
#define INTERLIGNE_MINI 3 // pixels

// Duree de vie d'un message
#define DUREE_VIE_MSG 7000 // ms

const uint NBR_MSG_MAX = 14;

GameMessages * GameMessages::singleton = NULL;

GameMessages * GameMessages::GetInstance() {
  if (singleton == NULL) {
    singleton = new GameMessages();
  }
  return singleton;
}

GameMessages::GameMessages() {
}

// Remise a zéro
void GameMessages::Reset(){
  liste.clear();
}

void GameMessages::Draw(){
  // Affichage des messages
  uint msgy = 50;

  for( iterator i=liste.begin(); i != liste.end(); ++i ){
    (*i)->DrawCenterTop(AppWormux::GetInstance()->video.window.GetWidth()/2, msgy);
    msgy += HAUT_POLICE_MINI + INTERLIGNE_MINI;
  }
}

// Actualisation : Supprime les anciens messages
void GameMessages::Refresh(){
  iterator i;
  for( i=liste.begin(); i != liste.end(); ){
    Message * msg = *i;
    if( DUREE_VIE_MSG < Time::GetInstance()->Read() - msg->get_time() ){
      delete (msg);
      i = liste.erase (i);
    }
    else
      i++;
  }
}

// Ajoute un message
void GameMessages::Add(const std::string &message){
  // Affiche le message dans la console
  std::cout << "o MSG: " << message << std::endl;

  // Ajoute le message à la liste (avec son heure d'arrivée)
  Message * newMessage = new Message(message, white_color, Font::GetInstance(Font::FONT_SMALL), Time::GetInstance()->Read());
  liste.push_back (newMessage);
  while( NBR_MSG_MAX < liste.size() )
    liste.pop_front();
}

