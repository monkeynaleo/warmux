/******************************************************************************
 *  Wormux is a convivial mass murder game.
 *  Copyright (C) 2001-2010 Wormux Team.
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
 * Generic menu
 *****************************************************************************/

#include <iostream>

#include "menu/menu.h"
#include "include/app.h"
#include "graphic/sprite.h"
#include "graphic/video.h"
#include "gui/button.h"
#include "gui/grid_box.h"
#include "gui/vertical_box.h"
#include "gui/horizontal_box.h"
#include "gui/question.h"
#include "interface/mouse.h"
#include "sound/jukebox.h"
#include "tool/resource_manager.h"
#include "gui/big/button_pic.h"
#include "gui/label.h"
#include "gui/check_box.h"
#include "gui/picture_text_cbox.h"
#include "gui/text_box.h"
#include "gui/list_box.h"

Menu::Menu(const std::string& bg, t_action _actions) :
  actions(_actions),
  selected_widget(NULL)
{
  close_menu = false ;
  AppWormux * app = AppWormux::GetInstance();

  uint x = app->video->window.GetWidth() / 2;
  uint y = app->video->window.GetHeight() - 50;

  Profile *res = GetResourceManager().LoadXMLProfile( "graphism.xml", false);
  background = new Sprite( GetResourceManager().LoadImage( res, bg), true);
  background->cache.EnableLastFrameCache();

  b_ok = NULL;
  b_cancel = NULL;

  if (actions == vNo) {
    actions_buttons = NULL;
  } else {
    actions_buttons = new HBox(50, false);

    if (actions == vOk || actions == vOkCancel) {
      b_ok = new Button(res, "menu/valider");
      actions_buttons->AddWidget(b_ok);
    }

    if (actions == vCancel  || actions == vOkCancel) {
      b_cancel = new Button(res, "menu/annuler");
      actions_buttons->AddWidget(b_cancel);
    }

    actions_buttons->SetPosition(x, y);
    widgets.AddWidget(actions_buttons);
    widgets.Pack();
  }

  widgets.SetContainer(this);
  GetResourceManager().UnLoadXMLProfile(res);
}

Menu::Menu(void) :
  widgets(),
  actions(vNo),
  background(NULL),
  selected_widget(NULL),
  b_cancel(NULL),
  b_ok(NULL),
  close_menu(false),
  actions_buttons(NULL)
{
}

Menu::~Menu()
{
  AppWormux::GetInstance()->SetCurrentMenu(NULL);
  if (NULL != background) {
    delete background;
  }
}

void Menu::LoadMenu(Profile * profile,
                    const xmlNode * rootMenuNode) 
{
  LoadBackground(profile, rootMenuNode);
  LoadWidget(profile, rootMenuNode, &widgets);
  widgets.Pack();
}

void Menu::LoadBackground(Profile * profile,
                          const xmlNode * rootMenuNode)
{
  XmlReader * xmlFile = profile->GetXMLDocument();
  std::string file("menu/pic_not_found.png");
  xmlFile->ReadStringAttr(rootMenuNode, "backgroundPicture", file);
  file = profile->relative_path + file;
  Surface surface(file.c_str());
  background = new Sprite(surface, true);
  background->cache.EnableLastFrameCache();
}

void Menu::LoadWidget(Profile * profile,
                      const xmlNode * rootMenuNode,
                      WidgetList * container) 
{
  XmlReader * xmlFile = profile->GetXMLDocument();
  unsigned int widgetCount = xmlFile->GetNbChildren(rootMenuNode);
  const xmlNode * currentNode = xmlFile->GetFirstChild(rootMenuNode);
  std::string currentNodeName;

  // For each sub-node ...
  for ( ; widgetCount > 0; --widgetCount) {

    currentNodeName = xmlFile->GetNodeName(currentNode);
    Widget * newWidget = CreateWidget(profile, currentNode, currentNodeName);

    if (NULL != newWidget) {

      if ("GridBox" == currentNodeName ||
          "HorizontalBox" == currentNodeName ||
          "VerticalBox" == currentNodeName) {
        LoadWidget(profile, currentNode, (WidgetList*)newWidget);
      }
      container->AddWidget(newWidget);
    }
    currentNode = xmlFile->GetNextSibling(currentNode);
  }

  if (NULL != container) {
    container->Pack();
  }
}

Widget * Menu::CreateWidget(Profile * profile,
                            const xmlNode * widgetNode,
                            std::string & widgetName)
{
  Widget * widget = NULL;

  if ("Picture" == widgetName) {
    widget = new PictureWidget(profile, widgetNode);
  } else if ("GridBox" == widgetName) {
    widget = new GridBox(profile, widgetNode);
  } else if ("ButtonPic" == widgetName) {
    widget = new ButtonPic(profile, widgetNode);
  } else if ("Label" == widgetName) {
    widget = new Label(profile, widgetNode);
  } else if ("CheckBox" == widgetName) {
    widget = new CheckBox(profile, widgetNode);
  } else if ("PicTxtCheckBox" == widgetName ) {
    widget = new PictureTextCBox(profile, widgetNode);
  } else if ("VerticalBox" == widgetName) {
    widget = new VBox(profile, widgetNode);
  } else if ("HorizontalBox" == widgetName) {
    widget = new HBox(profile, widgetNode);
  } else if ("SpinButton" == widgetName) {
    return NULL;
  } else if ("PasswordBox" == widgetName) {
    widget = new PasswordBox(profile, widgetNode);
  } else if ("TextBox" == widgetName) {
    widget = new TextBox(profile, widgetNode);
  } else if ("Button" == widgetName) {
    widget = new Button(profile, widgetNode);
  } else if ("ListBox" == widgetName) {
    widget = new ListBox(profile, widgetNode);
  }

  if (NULL != widget) { 
    return widget->LoadXMLConfiguration() ? widget : NULL;
  }
  return NULL;
}

void Menu::play_ok_sound()
{
  JukeBox::GetInstance()->Play("default", "menu/ok");
}

void Menu::play_cancel_sound()
{
  JukeBox::GetInstance()->Play("default", "menu/cancel");
}

void Menu::play_error_sound()
{
  JukeBox::GetInstance()->Play("default", "menu/error");
}

void Menu::mouse_ok()
{
  if (signal_ok()) {
    play_ok_sound();
    close_menu = true;
  }
}

void Menu::mouse_cancel()
{
  if (signal_cancel()) {
    play_cancel_sound();
    close_menu = true;
  }
}

bool Menu::BasicOnClickUp(const Point2i &mousePosition)
{
  if (b_ok != NULL &&  b_ok->Contains(mousePosition)) {
    mouse_ok();
  } else if (b_cancel != NULL && b_cancel->Contains(mousePosition)) {
    mouse_cancel();
  } else {
    return false;
  }
  return true;
}

void Menu::key_ok()
{
  if (signal_ok()) {
    play_ok_sound();
    close_menu = true;
  }
}

void Menu::key_cancel()
{
  if (signal_cancel()) {
    play_cancel_sound();
    close_menu = true;
  }
}

void Menu::key_up()
{
  widgets.SetFocusOnPreviousWidget();
  RedrawMenu();
}

void Menu::key_down()
{
  widgets.SetFocusOnNextWidget();
  RedrawMenu();
}

void Menu::key_left()
{
}

void Menu::key_right()
{
}

void Menu::key_tab()
{
  widgets.SetFocusOnNextWidget();
}

void Menu::DisplayError(const std::string & msg)
{
  play_error_sound();

  std::cerr << msg << std::endl;

  Question question(Question::WARNING);
  question.Set(msg, true, 0);
  question.Ask();
}

void Menu::DrawBackground()
{
  if (NULL == background) {
    return;
  }
  background->ScaleSize(GetMainWindow().GetSize());
  background->Blit(GetMainWindow(), 0, 0);
}

void Menu::RedrawBackground(const Rectanglei & rect)
{
  if (NULL == background) {
    return;
  }
  background->Blit(GetMainWindow(), rect, rect.GetPosition());
}

void Menu::RedrawMenu()
{
  DrawBackground();
  widgets.NeedRedrawing();
}

// Class method
// Push a stupid user event to make the menu exits for SDL_WaitEvent
void Menu::WakeUpOnCallback()
{
  SDL_Event event;
  event.type = SDL_USEREVENT;
  event.user.code = 0;
  event.user.data1 = NULL;
  event.user.data2 = NULL;
  SDL_PushEvent(&event);
}

bool Menu::HandleGlobalEvent(const SDL_Event & event)
{
  if (event.type != SDL_KEYDOWN) {
    return false;
  }

  // Emergency exit
  if (event.key.keysym.sym == SDLK_ESCAPE
      && (SDL_GetModState() & KMOD_CTRL)) {
    AppWormux::EmergencyExit();
    return true; // never reached
  }

  // Toggle fullscreen
  if (event.key.keysym.sym == SDLK_F10) {
    AppWormux::GetInstance()->video->ToggleFullscreen();
    return true;
  }

  return false;
}

void Menu::HandleEvent(const SDL_Event& event)
{
  if (event.type == SDL_QUIT) {
    key_cancel();
  } else if (event.type == SDL_KEYDOWN) {
    bool used_by_widget = false;

    if (event.key.keysym.sym != SDLK_ESCAPE &&
	event.key.keysym.sym != SDLK_RETURN &&
	event.key.keysym.sym != SDLK_KP_ENTER)
      used_by_widget = widgets.SendKey(event.key.keysym);

    if (!used_by_widget) {
      switch (event.key.keysym.sym)
	{
	case SDLK_ESCAPE:
	  key_cancel();
	  break;
	case SDLK_RETURN:
	case SDLK_KP_ENTER:
	  key_ok();
	  break;
	case SDLK_UP:
	  key_up();
	  break;
	case SDLK_DOWN:
	  key_down();
	  break;
	case SDLK_LEFT:
	  key_left();
	  break;
	case SDLK_RIGHT:
	  key_right();
	  break;
	case SDLK_TAB:
	  key_tab();
	  break;
	default:
	  // should have been handle upper!
	  break;
	}
    }
  } else if (event.type == SDL_MOUSEBUTTONUP) {
    Point2i mousePosition(event.button.x, event.button.y);
    
    if (!BasicOnClickUp(mousePosition)) {
      OnClickUp(mousePosition, event.button.button);
    }
  } else if (event.type == SDL_MOUSEBUTTONDOWN) {
    Point2i mousePosition(event.button.x, event.button.y);
    OnClick(mousePosition, event.button.button);
  }
}

void Menu::HandleEvents()
{
  // Poll and treat events
  SDL_Event event;

  if (!SDL_WaitEvent(&event)) {
    return;
  }

  do {
    if (!HandleGlobalEvent(event)) {
      HandleEvent(event);
    }
  } while (SDL_PollEvent(&event) && !close_menu);
}

void Menu::Run(bool skip_menu)
{
  signal_begin_run();

  if (skip_menu) {
    signal_ok();
    return;
  }

  Mouse::pointer_t old_pointer = Mouse::GetInstance()->SetPointer(Mouse::POINTER_STANDARD);
  int x = 0;
  int y = 0;
  close_menu = false;

  // Display the background
  DrawBackground();

  SDL_GetMouseState(&x, &y);
  Point2i mousePosition(x, y);
  Display(mousePosition);

  SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);

  do
  {
    // this is the current menu (here in case we had run a submenu)
    AppWormux::GetInstance()->SetCurrentMenu(this);

    // Poll and treat events
    HandleEvents();

    // Avoid to calculate redraw menu when comming back for closing.
    if (!close_menu) {
      SDL_GetMouseState( &x, &y );
      Point2i mousePosition(x, y);

      Display(mousePosition);
    }

  } while (!close_menu);

  Mouse::GetInstance()->SetPointer(old_pointer);
}

void Menu::Display(const Point2i& mousePosition)
{
  widgets.Update(mousePosition);
  Draw(mousePosition);
  AppWormux::GetInstance()->video->Flip();
}

void Menu::SetActionButtonsXY(int x, int y)
{
  if (actions_buttons != NULL) {
    actions_buttons->SetPosition(x, y);
  }
}
