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
 * Mouse management
 *****************************************************************************/

#include "mouse.h"

#include "cursor.h"
#include "interface.h"
#include "../game/game_mode.h"
#include "../game/game_loop.h"
#include "../game/time.h"
#include "../graphic/video.h"
#include "../include/app.h"
#include "../include/action_handler.h"
#include "../map/camera.h"
#include "../map/map.h"
#include "../team/macro.h"
#include "../tool/point.h"
#include "../weapon/weapon.h"

// Vitesse du definalement �la souris
const uint SCROLL_MOUSE = 20;

// Largeur de la zone de sensibilite au camera a la souris
const uint SENSIT_SCROLL_MOUSE = 40; // pixels

Mouse * Mouse::singleton = NULL;

Mouse * Mouse::GetInstance() {
  if (singleton == NULL) {
    singleton = new Mouse();
  }
  return singleton;
}

Mouse::Mouse(){
  scroll_actif = false;

  // Load the different pointers
  Profile *res = resource_manager.LoadXMLProfile("graphism.xml", false);
  pointer_select = resource_manager.LoadImage(res, "mouse/pointer_select");
  pointer_move = resource_manager.LoadImage(res, "mouse/pointer_move");
  pointer_aim = resource_manager.LoadImage(res, "mouse/pointer_aim");
  //resource_manager.UnLoadXMLProfile( res);
  current_pointer = POINTER_STANDARD;
  delete res;
}

void Mouse::Reset(){
}

bool Mouse::ActionClicD(){ 
  if( ActiveTeam().GetWeapon().CanChangeWeapon() )
    Interface::GetInstance()->weapons_menu.SwitchDisplay();

  return true;
}

//WORKING=============================================
bool Mouse::ActionWhellUp(){
  if (GameLoop::GetInstance()->ReadState() == GameLoop::PLAYING) {
    ActiveTeam().AccessWeapon().ActionUp(); 
    return true ;
    }
    
  return true;
}

bool Mouse::ActionWhellDown(){
  if (GameLoop::GetInstance()->ReadState() == GameLoop::PLAYING) {
    ActiveTeam().AccessWeapon().ActionDown();
    return true ;
    }
    
  return true;
}
//==========================================================

bool Mouse::ActionClicG()
{
  if(!ActiveTeam().is_local)
    return false;

  const Point2i pos_monde = GetWorldPosition();

  // Action dans le menu des armes ?
  if( Interface::GetInstance()->weapons_menu.ActionClic( GetPosition() ) )
    return true;

  // On peut changer de ver ?
  //Change character by mouse click only if the choosen weapon allows it
  if( GameMode::GetInstance()->AllowCharacterSelection() && ActiveTeam().GetWeapon().MOUSE_CHARACTER_SELECTION){
    
    // S�ection d'un ver se son �uipe ?
    bool ver_choisi=false;
    Team::iterator it=ActiveTeam().begin(),
	                 fin=ActiveTeam().end();

    for( ; it != fin; ++it) {
      if( &(*it) != &ActiveCharacter()
        && !it -> IsDead()
        && it->GetRect().Contains( pos_monde ) ){
	
	ver_choisi = true;
        break;
      }
    }

    if( ver_choisi ){
      while ( &(*it) != &ActiveCharacter() )
	ActiveTeam().NextCharacter ();
      return true;
    }

    if( ActiveCharacter().GetRect().Contains( pos_monde ) ){
      CharacterCursor::GetInstance()->FollowActiveCharacter();
      return true;
    }
  }

/*  // Action dans le menu des armes ?
  if( Interface::GetInstance()->weapons_menu.ActionClic(GetPosition()) )
    return true; */

  // Choosing target for a weapon, many posibilities :
  // - Do nothing
  // - Choose a target but don't fire
  // - Choose a target and fire it !
  if (GameLoop::GetInstance()->ReadState() == GameLoop::PLAYING) {
    Action* a = new Action(ACTION_SET_TARGET);
    a->Push(GetWorldPosition().x);
    a->Push(GetWorldPosition().y);
    ActionHandler::GetInstance()->NewAction (a);
    return true ;
  }

  return false;
}

void Mouse::ChoixVerPointe(){
  if (GameLoop::GetInstance()->ReadState() != GameLoop::PLAYING)
    return;

  const Point2i pos_monde = GetWorldPosition();

  // Quel ver est point�par la souris ? (en dehors du ver actif)
  Interface::GetInstance()->character_under_cursor = NULL;
  FOR_ALL_LIVING_CHARACTERS(equipe,ver){
    if ((&(*ver) != &ActiveCharacter())
       && ver->GetRect().Contains(pos_monde) ){
      Interface::GetInstance()->character_under_cursor = &(*ver);
    }
  }

  // Aucun ver n'est point�... et le ver actif alors ?
  if ((Interface::GetInstance()->character_under_cursor == NULL)
      && ActiveCharacter().GetRect().Contains( pos_monde)){
      Interface::GetInstance()->character_under_cursor = &ActiveCharacter();
  }

  // Dessine le curseur autour du ver point�s'il y en a un
//  if (interface.character_under_cursor != NULL) {
//    curseur_ver.PointeObj (interface.character_under_cursor);
//  } else {
//    curseur_ver.PointeAucunObj();
//  }
}

void Mouse::ScrollCamera() {
  bool scroll = false;

  Point2i mousePos = GetPosition();
  Point2i sensitZone(SENSIT_SCROLL_MOUSE, SENSIT_SCROLL_MOUSE);
  Point2i winSize = AppWormux::GetInstance()->video.window.GetSize();
  Point2i tstVector;

  tstVector = mousePos.inf(sensitZone);
  if( !tstVector.IsNull() ){
    camera.SetXY( tstVector * (mousePos - sensitZone)/2 );
    camera.autorecadre = false;
    scroll = true;
  }

  tstVector = winSize.inf(mousePos + sensitZone);
  if( !tstVector.IsNull() ){
    camera.SetXY( tstVector * (mousePos + sensitZone - winSize)/2 );
    camera.autorecadre = false;
    scroll = true;
  }

}

void Mouse::TestCamera(){
  Point2i mousePos = GetPosition();

  //Move camera with mouse holding Ctrl key down
  const bool demande_scroll = SDL_GetModState() & KMOD_CTRL;

  if( demande_scroll ){
    if( scroll_actif ){
	  Point2i offset = savedPos - mousePos;
      camera.SetXY(offset);
      camera.autorecadre = false;
    }else{
      scroll_actif = true;
    }
	savedPos = mousePos;
    return;
  }else{
    scroll_actif = false;
  }

  if(!Interface::GetInstance()->weapons_menu.IsDisplayed())
    ScrollCamera();
}

void Mouse::Refresh(){
  if (!scroll_actif)
    ChoixVerPointe();
}

Point2i Mouse::GetPosition() const{
	int x, y;

	SDL_GetMouseState( &x, &y);
	return Point2i(x, y);
}

Point2i Mouse::GetWorldPosition() const{
   return GetPosition() + camera.GetPosition();
}

void Mouse::TraiteClic (const SDL_Event *event){
  if( event->type == SDL_MOUSEBUTTONDOWN ){
  
    if( event->button.button == SDL_BUTTON_RIGHT ){
      ActionClicD();
      return;
    }

    // Clic gauche de la souris ?
    if( event->button.button == SDL_BUTTON_LEFT ){
      ActionClicG();
      return;
    }
    

    if (event->button.button == SDL_BUTTON_WHEELDOWN){
      ActionWhellDown(); 
      return;
    }
       
    if (event->button.button == SDL_BUTTON_WHEELUP){
      ActionWhellUp(); 
      return;
    }
    
  }
}

void Mouse::SetPointer(pointer_t pointer)
{
  if (current_pointer == pointer) return;

  current_pointer = pointer;

  if (pointer == POINTER_STANDARD) SDL_ShowCursor(true);
  else SDL_ShowCursor(false);
}

bool Mouse::ScrollPointer()
{

  Point2i mousePos = GetPosition();
  Point2i winSize = AppWormux::GetInstance()->video.window.GetSize();
  Point2i cameraPos = camera.GetPosition();

  // tries to go on the left
  if ( (mousePos.x > 0 && mousePos.x < (int)SENSIT_SCROLL_MOUSE)
       && (cameraPos.x > 0) )
      return true;

  // tries to go on the right
  if ( (mousePos.x > winSize.x - (int)SENSIT_SCROLL_MOUSE)
       && ( cameraPos.x + winSize.x < world.GetWidth() ))
      return true;

  // tries to go up
  if ( (mousePos.y > 0 && mousePos.y < (int)SENSIT_SCROLL_MOUSE)
       && (cameraPos.y > 0) )
      return true;

  // tries to go down
  if ( (mousePos.y > winSize.y - (int)SENSIT_SCROLL_MOUSE)
       && (cameraPos.y + winSize.y < world.GetHeight()) )
    return true;


  return false;
}

bool Mouse::DrawMovePointer()
{
  if (ScrollPointer() ) {
    AppWormux::GetInstance()->video.window.Blit( pointer_move, GetPosition() );
    world.ToRedrawOnScreen(Rectanglei(GetPosition().x, GetPosition().y , pointer_move.GetWidth(), pointer_move.GetHeight()));
    return true;
  }
  return false;
}

void Mouse::Draw()
{
  if (current_pointer == POINTER_STANDARD)
    return; // use standard SDL cursor

  if ( DrawMovePointer() )
    return;

  switch (current_pointer)
    {
    case POINTER_SELECT:
      AppWormux::GetInstance()->video.window.Blit( pointer_select, GetPosition() );
      world.ToRedrawOnScreen(Rectanglei(GetPosition().x, GetPosition().y , pointer_select.GetWidth(), pointer_select.GetHeight()));
      break;
    case POINTER_MOVE:
      AppWormux::GetInstance()->video.window.Blit( pointer_move, GetPosition() );
      world.ToRedrawOnScreen(Rectanglei(GetPosition().x, GetPosition().y , pointer_move.GetWidth(), pointer_move.GetHeight()));
      break;
    case POINTER_AIM:
      if(ActiveTeam().is_local) {
	AppWormux::GetInstance()->video.window.Blit( pointer_aim, Point2i(GetPosition().x-7, GetPosition().y-10 ));
	world.ToRedrawOnScreen(Rectanglei(GetPosition().x-7, GetPosition().y-10, pointer_aim.GetWidth(), pointer_aim.GetHeight()));
      } else {
	AppWormux::GetInstance()->video.window.Blit( pointer_select, GetPosition() );
	world.ToRedrawOnScreen(Rectanglei(GetPosition().x, GetPosition().y , pointer_select.GetWidth(), pointer_select.GetHeight()));
      }
      break;
    default:
      break;
    };
}
  

// Center the pointer on the screen
void Mouse::CenterPointer()
{
  SDL_WarpMouse(AppWormux::GetInstance()->video.window.GetWidth()/2,
		AppWormux::GetInstance()->video.window.GetHeight()/2);
}
