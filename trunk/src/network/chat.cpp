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

#include <SDL_events.h>
#include "game/time.h"
#include "graphic/text.h"
#include "graphic/text_list.h"
#include "include/action.h"
#include "include/action_handler.h"
#include "include/app.h"
#include "network/chat.h"
#include "network/admin_commands.h"
#include "network/network.h"
#include "tool/text_handling.h"

const uint HEIGHT=15;
const uint XPOS=25;
const uint YPOS=130;
const uint MAXLINES=10; //Fidel's advise
const uint MAXSECONDS=40;

Chat::~Chat()
{
  delete msg;
  delete input;
}

Chat::Chat():
  input(NULL),
  msg(NULL),
  cursor_pos(0),
  check_input(false),
  last_time(0)
{
}

void Chat::Clear()
{
  chat.Clear();
}

void Chat::Show()
{
  uint now = Time::GetInstance()->ReadSec();

  if((now - last_time) >= MAXSECONDS){
    chat.DeleteLine();
    last_time = now;
  }

  chat.Draw(XPOS, YPOS, HEIGHT);

  if(check_input)
    ShowInput();
}

void Chat::ShowInput()
{
  if (!check_input) {
    check_input = true;

    /* Enable key repeat when chatting :) */
    SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);
  }

  if (input == NULL){
    input = new Text("", c_white);
    msg = new Text(_("Say: "), c_red);
  }

  /* FIXME where do those constants come from ?*/
  msg->DrawLeftTop(Point2i(25, 400));
  if (input->GetText() != "") {
    input->DrawLeftTop(Point2i(25 + msg->GetWidth() + 5, 400));
    input->DrawCursor(Point2i(25 + msg->GetWidth() + 5, 400), cursor_pos);
  }
}

bool Chat::CheckInput() const
{
  return check_input;
}

void Chat::NewMessage(const std::string &msg)
{
  if (!chat.Size()){
    uint now = Time::GetInstance()->ReadSec();
    last_time = now;
  }

  chat.AddText(msg, MAXLINES);
}

void Chat::SendMessage(const std::string &msg)
{
  if (msg.size() == 0)
    return;

  Action* a = new Action(Action::ACTION_CHAT_MESSAGE);
  a->Push(Network::GetInstance()->GetPlayer().GetNickname());
  a->Push(msg);
  ActionHandler::GetInstance()->NewAction(a);
}

void Chat::CloseInput()
{
  check_input = false; //Hide input widget

  input->SetText("");
  cursor_pos = 0;

  // Disable key repeat during the game!
  SDL_EnableKeyRepeat(0, 0);
}

void Chat::HandleKeyPressed(const SDL_Event& evnt)
{
  SDL_KeyboardEvent kbd_event = evnt.key;
  SDL_keysym key = kbd_event.keysym;
  std::string txt = input->GetText();

  if (TextHandle(txt, cursor_pos, key))
    input->SetText(txt);
}

void Chat::HandleKeyReleased(const SDL_Event& evnt)
{
  SDL_KeyboardEvent kbd_event = evnt.key;
  SDL_keysym key = kbd_event.keysym;
  std::string txt = input->GetText();

  switch (key.sym) {

  case SDLK_RETURN:
  case SDLK_KP_ENTER:
    if (txt[0] == '/')
      ProcessCommand(txt);
    else if (txt != "")
      SendMessage(txt);

    CloseInput();
    break;
  case SDLK_ESCAPE:
    CloseInput();
    break;
  default:
    break;
  }
}
