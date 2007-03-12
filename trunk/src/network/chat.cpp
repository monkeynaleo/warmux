/******************************************************************************
 *  Wormux is a convivial mass murder game.
 *  Copyright (C) 2007 Jon de Andres
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
 * Chat in game session.
 * nefertum - Jon de Andres
 *****************************************************************************/

#include "chat.h"
#include "../graphic/text.h"
#include "../game/time.h"
#include <string>

Chat::~Chat()
{
  delete chat;
}

Chat::Chat()
{
  chat = NULL;
  input = NULL;
  msg = NULL;
  check_input = false;
}

void Chat::Reset()
{
  if(chat == NULL)
    chat = new TextList();
}

void Chat::Show()
{
  uint now = Time::GetInstance()->ReadSec();
  
  if((now - last_time) >= MAXSECONDS){
    chat->DeleteLine();
    last_time = now;
  }

  chat->Draw(XPOS, YPOS, HEIGHT);

  if(check_input)
    ShowInput();
}

void Chat::ShowInput()
{
  check_input = true;
  if (input == NULL){
    input = new Text("", c_white);
    msg = new Text(SAY, c_red);
  }
  input->DrawTopLeft(50,500);
  msg->DrawTopLeft(25,500);
}

bool Chat::CheckInput(){
  return check_input;
}

void Chat::NewMessage(const std::string &msg)
{
  if (!chat->Size()){
    uint now = Time::GetInstance()->ReadSec();
    last_time = now;
  }

  chat->AddText(msg, MAXLINES);
}


void Chat::HandleKey(const SDL_Event& event)
{
  SDL_KeyboardEvent kbd_event = event.key;
  SDL_keysym key = kbd_event.keysym;
  std::string txt = input->GetText();

  switch (key.sym){
    
  case SDLK_RETURN:
    check_input = false; //Hide input widget
    if (txt != "" )
      network.SendChatMessage(txt); //Send 'txt' to other players
    input->Set("");
    break;

  case SDLK_BACKSPACE:
    if (kbd_event.state == 1 && txt != "")
      txt = txt.substr(0, txt.size()-1);
    input->Set(txt);
    break;

  default:
    if (kbd_event.state == 1 && key.unicode > 0){
      if(key.unicode < 0x80) { // 1 byte char
	txt = txt + (char)key.unicode;
      }
      else if (key.unicode < 0x800) {// 2 byte char
        txt = txt + (char)(((key.unicode & 0x7c0) >> 6) | 0xc0);
        txt = txt + (char)((key.unicode & 0x3f) | 0x80);
      }
      else {// if (key.unicode < 0x10000) // 3 byte char
        txt = txt + (char)(((key.unicode & 0xf000) >> 12) | 0xe0);
        txt = txt + (char)(((key.unicode & 0xfc0) >> 6) | 0x80);
        txt = txt + (char)((key.unicode & 0x3f) | 0x80);
      }
      input->Set(txt);  
    }
  }
}
