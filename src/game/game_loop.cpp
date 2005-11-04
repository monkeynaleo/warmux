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
 * Boucle de jeu : dessin et gestion des données.
 *****************************************************************************/

#include "game_loop.h"
//-----------------------------------------------------------------------------
#include "../graphic/fps.h"
#include "../graphic/graphism.h"
#include "../graphic/video.h"
#include "../include/constant.h"
#include "../include/action_handler.h"
#include "../interface/game_msg.h"
#include "../interface/cursor.h"
#include "../interface/interface.h"
#include "../interface/mouse.h"
#include "../object/bonus_box.h"
#include "../object/objects_list.h"
#include "../object/particle.h"
#include "../map/camera.h"
#include "../map/wind.h"
#include "../map/map.h"
#include "../map/maps_list.h"
#include "../sound/jukebox.h"
#include "../team/macro.h"
#include "../tool/i18n.h"
#include "../weapon/weapons_list.h"
#include "game.h"
#include "time.h"
#include "debug.h"
#include "game_mode.h"
#include "config.h"
#include "../interface/keyboard.h"

#ifdef CL
#include "../network/network.h"
#else
#include <SDL.h>
#include "../include/app.h"
#include "../tool/Distance.h"
#endif

#include <sstream>
#include <iostream>
using namespace Wormux;
//-----------------------------------------------------------------------------

#ifdef DEBUG

// Debogue la fin du tour ?
#define DBG_FIN_TOUR

// Afiche les changements d'état ?
#define DEBUG_CHG_ETAT

#endif

#define COUT_DEBUG std::cout << "[BoucleJeu] "

//-----------------------------------------------------------------------------
GameLoop game_loop;
//-----------------------------------------------------------------------------



// ***************************************************************************
// ***************************************************************************
//               FUNCTIONS USED TO INITIALIZE NETWORK GAME
// ***************************************************************************
// ***************************************************************************
#ifdef CL
void InitGameData_NetServer()
{
  //	action_handler.NewAction(Action(ACTION_ASK_TEAM));
  do
    {
      action_handler.NewAction(Action(ACTION_ASK_VERSION));
      std::string msg=_("Wait for clients");
      action_handler.ExecActions();
      std::cout << msg << std::endl;
      CL_Display::clear(CL_Color::black);
      police_grand.WriteCenterTop (video.GetWidth()/2, video.GetHeight()/2, msg);
      CL_Display::flip();
      CL_System::keep_alive(500);
    } while (network.state != Network::NETWORK_SERVER_INIT_GAME);
  std::cout << "Server init game." << std::endl;


	
  std::cout << "o " << _("Load map") << std::endl;
  action_handler.NewAction (ActionString(ACTION_SET_MAP, TerrainActif().name));
  monde.Reset();

  std::cout << "o " << _("Initialise teams") << std::endl;
  teams_list.Reset();

  // For cliens : Create teams
  action_handler.NewAction (Action(ACTION_CLEAR_TEAMS));
  
  TeamsList::iterator 
    it=teams_list.list.begin(),
    end=teams_list.list.end();

  for (; it != end; ++it)
    {
      Team& team = **it;
		
      // cliens : Create teams
      action_handler.NewAction (ActionString(ACTION_NEW_TEAM, team.GetId()));
		
      // cliens : Place characters
      action_handler.NewAction (ActionString(ACTION_CHANGE_TEAM, team.GetId()));
      Team::iterator
	tit = team.begin(),
	tend = team.end();
      int i=0;
      for (; tit != tend; ++tit, ++i)
	{
	  Character &character = *tit;
	  action_handler.NewAction (ActionInt(
					      ACTION_CHANGE_CHARACTER, i));
	  action_handler.NewAction (ActionInt2(
					       ACTION_MOVE_CHARACTER, 
					       character.GetX(), character.GetY()));
	  action_handler.NewAction (ActionInt(
					      ACTION_SET_CHARACTER_DIRECTION, 
					      character.GetDirection()));
	}

      // Select first character
      action_handler.NewAction (ActionInt(ACTION_CHANGE_CHARACTER, 0));
    }
	
  action_handler.NewAction (ActionString(ACTION_CHANGE_TEAM, ActiveTeam().GetId()));
  action_handler.NewAction (ActionInt(ACTION_CHANGE_CHARACTER, ActiveTeam().ActiveCharacterIndex()));

  // Create objects
  lst_objets.Reset();
   // @@@ TODO : send objects ... @@@@
	
  // Remise à zéro
  std::cout << "o " << _("Initialise data") << std::endl;
  curseur_ver.Reset();
  temps.Reset();
  mouse.Reset();
  image_par_seconde.Reset();
  interface.Reset();
  game_messages.Reset();
  
  action_handler.NewAction (Action(ACTION_START_GAME));
}

//-----------------------------------------------------------------------------

void InitGameData_NetClient()
{
  do
    {
      std::string msg=_("Wait for server informations");
      switch(network.state)
	{
	case Network::NETWORK_WAIT_SERVER:
	  msg = _("Wait for server");
	  break;
	case Network::NETWORK_WAIT_MAP:
	  msg = _("Wait for map");
	  break;
	case Network::NETWORK_WAIT_TEAMS:
	  msg = _("Wait teams");
	  break;
	default:
	  msg = _("Unknow action");
	  std::cout << "Unknow action for network in game_loop.cpp" << std::endl;
	  break;
	}
      action_handler.ExecActions();
      std::cout << network.state << std::endl;
      std::cout << msg << std::endl;
      CL_Display::clear(CL_Color::black);
      police_grand.WriteCenterTop (video.GetWidth()/2, video.GetHeight()/2, msg);
      CL_Display::flip();
      CL_System::keep_alive(300);
    } while (network.state != Network::NETWORK_PLAYING);
  std::cout << network.state << " : Run game !" << std::endl;
 
  // @@@ TODO @@@
  lst_objets.Reset();
 
  //Set the second team as a team played from the client
  ActiveTeam().is_local = false;
}

#endif // CL defined

//-----------------------------------------------------------------------------

void InitGameData_Local()
{
  // Placement des vers
  std::cout << "o " << _("Find a random position for worms") << std::endl;
  monde.Reset();
  teams_list.Reset();

  // Remise à zéro
  std::cout << "o " << _("Initialise data") << std::endl;
  lst_objets.Reset();
}

//-----------------------------------------------------------------------------

void InitGameData()
{
  temps.Reset();
  
#ifdef CL
  if (network.is_server())
    InitGameData_NetServer();
  else if (network.is_client())
    InitGameData_NetClient();
  else	
#endif
    InitGameData_Local();

  curseur_ver.Reset();
  mouse.Reset();
  clavier.Reset();
   
  image_par_seconde.Reset();
  interface.Reset();
  game_messages.Reset();
}

//-----------------------------------------------------------------------------


// ***************************************************************************
// ***************************************************************************
//               
// ***************************************************************************
// ***************************************************************************
void InitGame ()
{
  jeu.MsgChargement();

  // =============================================
  // Init all needed data
  // =============================================
  if (!jeu.initialise)
  {
    std::cout << "o " << _("Initialisation") << std::endl;
    interface.Init();
    curseur_ver.Init();
    lst_objets.Init();	
    jeu.initialise = true;
  }

  InitGameData();

  // =============================================
  // Init teams
  // =============================================

  // Teams' creation
   std::cout << "team nb = " << teams_list.list.size() << std::endl;
  if (teams_list.list.size() < 2)
    Erreur(_("You need at least two teams to play: "
	     "change this in 'Options menu' !"));
  assert (teams_list.list.size() <= game_mode.max_teams);

  // Initialization of teams' energy
  teams_list.InitEnergy();

  // Load teams' sound profiles
#ifdef CL
  jukebox.Load("default");
  POUR_CHAQUE_EQUIPE(equipe) 
    if ( (**equipe).GetSoundProfile() != "default" )
      jukebox.Load((**equipe).GetSoundProfile()) ;  
#else
  jukebox.LoadXML("default");
  POUR_CHAQUE_EQUIPE(equipe) 
    if ( (**equipe).GetSoundProfile() != "default" )
      jukebox.LoadXML((**equipe).GetSoundProfile()) ; 
#endif
   
  // =============================================
  // Begin to play !!
  // =============================================
  // Music -> sound should be choosed in map.Init and then we just have to call jukebox.PlayMusic()
#ifdef CL
   if (jukebox.UseMusic()) jukebox.Play ("ambiance/grenouilles", true);
#else
   if (jukebox.UseMusic()) jukebox.Play ("share", "music/grenouilles", -1);
#endif
  jeu.fin_partie = false;
  game_loop.SetState (gamePLAYING, true);
}

// ***************************************************************************
// ***************************************************************************
//   GAMELOOP METHODS
// ***************************************************************************
// ***************************************************************************

//-----------------------------------------------------------------------------

GameLoop::GameLoop()
{
  state = gamePLAYING;
  interaction_enabled = true;
}




//-----------------------------------------------------------------------------

void GameLoop::Refresh()
{  
  // Refresh the clock
  RefreshClock();

  // Refresh game messages
  game_messages.Refresh();

  // Camera Refresh 
  camera.Refresh();

  // Mise à jeu des entrées (clavier / mouse)
#ifdef CL
#if CL_CURRENT_VERSION <= 0x0708
  CL_System::keep_alive(sleep_fps);
#else
  CL_System::keep_alive();
#endif
#endif
   
#ifndef CL //////////////////// TREAT EVENTS in the SDL fashion /////////////////////
 
   // Poll and treat events
	
   SDL_Event event;
   
   while( SDL_PollEvent( &event) ) 
     {      
	if ( event.type == SDL_QUIT) 
	  {  
	     std::cout << "SDL_QUIT received ===> exit TODO" << std::endl;
	     SDL_Quit();
	     exit(0);
	  }
	if ( event.type == SDL_MOUSEBUTTONDOWN )
	  {
	     mouse.TraiteClic( &event);
	  }
	if ( event.type == SDL_KEYDOWN 
	||   event.type == SDL_KEYUP)
	  {	       
	     switch ( event.key.keysym.sym)
	       { 
		case SDLK_ESCAPE:SDL_Quit();exit(0);break;
		case SDLK_F1: break;
		case SDLK_p: break;
		case SDLK_UP: break;
		case SDLK_DOWN: break;
		case SDLK_LEFT: break;
		case SDLK_RIGHT: break;
		case SDLK_SPACE: break;
		default:break;
	       }
	     
	     clavier.HandleKeyEvent( &event);
	  }
     }
   

#endif ////////////////////////////////////////
   
  // How many frame by seconds ?
  image_par_seconde.Refresh();

  //--- D'abord ce qui pourrait modifier les données d'un ver ---

  if (!temps.EstPause())
  {
     
    // Keyboard and mouse refresh
    if ( 
	(interaction_enabled && state != gameEND_TURN)
	|| (ActiveTeam().GetWeapon().IsActive() && ActiveTeam().GetWeapon().override_keys) // for driving supertux for example
	)
    {
      mouse.Refresh();
      clavier.Refresh();
    }
   
    // Execute actions
    action_handler.ExecActions();
 
    // Refresh des vers
    POUR_TOUS_VERS(equipe,ver) ver -> Refresh();

    // Recalcule l'energie des equipes
    POUR_CHAQUE_EQUIPE(team) (**team).Refresh();
    teams_list.RefreshEnergy();

    //--- Ensuite, actualise le reste du jeu ---

    // Refresh weapons
    ActiveTeam().AccessWeapon().Manage();

    // Refresh objects
    lst_objets.Refresh();

    // Refresh particles
    global_particle_engine.Refresh();

    // Refresh cursor
    curseur_ver.Refresh();
  }
  
  // Refresh the map
  monde.Refresh();

#ifdef CL
#ifdef DEBUG
  // Draw les messages de debug
  debug.Refresh();
#endif
#endif

}

//-----------------------------------------------------------------------------

void GameLoop::Draw ()
{
  std::ostringstream txt;

#ifdef CL
  CL_Display::push_cliprect( CL_Rect(FOND_X, FOND_Y, 
  					  FOND_X+camera.GetWidth(), 
					  FOND_Y+camera.GetHeight()));
#endif
   
  // Draw the sky 
  monde.DrawSky();

#ifdef CL
  CL_Display::push_translate((int)FOND_X-camera.GetX(), 
			     (int)FOND_Y-camera.GetY()); // needed for differential scrolling
#endif
   
  // Draw the map
  monde.Draw();

  // Draw the characters 
  POUR_TOUS_VERS(equipe,ver) {
    ver -> Draw();

    if (&(*ver) == &ActiveCharacter() && !ActiveCharacter().IsDead()) {
      if (state == gamePLAYING) {
	ActiveTeam().crosshair.Draw();
	ActiveTeam().AccessWeapon().Draw();
      }
    }
  }

  lst_objets.Draw();
  global_particle_engine.Draw();
  curseur_ver.Draw();

  // Draw water
  monde.DrawWater();

  // Draw teams' information
  POUR_CHAQUE_EQUIPE(team) (**team).Draw();

#ifdef CL
  // Supprime le decalage
  CL_Display::pop_modelview();
#endif
   
  // Display game messages
  game_messages.Draw();

  // Display the name of map's author
  monde.DrawAuthorName();

  // Display number of frames by second
  image_par_seconde.Draw();

#ifdef CL
  // Remove clipping
  CL_Display::pop_cliprect();
#endif
   
#ifdef CL
#ifdef DEBUG
  // Draw les messages de debug
  debug.Draw();
#endif
#endif

  // Draw the interface (current team's information, weapon's ammo)
  interface.Draw ();

  // Display game clock
  temps.Draw();

  // Display wind bar
  wind.Draw();

  // Add one frame to the fps counter ;-)
  image_par_seconde.AjouteUneImage();
}

//-----------------------------------------------------------------------------

void GameLoop::CallDraw()
{
  Draw();
#ifdef CL
   CL_Display::flip (false);
#else
   SDL_Flip( app.sdlwindow);
#endif
}


//-----------------------------------------------------------------------------

void GameLoop::Run()
{
  sleep_fps = 0;

  // boucle until game is finished
  do
  {
#ifdef CL
    unsigned int start = CL_System::get_time();
#else
    unsigned int start = SDL_GetTicks();
#endif
     
    jeu.fin_partie = false;

    // the REAL loop
    Refresh();
    CallDraw ();

    // try to adjust to max Frame by seconds
#ifdef CL
    unsigned int delay = CL_System::get_time()-start;
#else
    unsigned int delay = SDL_GetTicks()-start;
#endif
     
# ifdef BUGGY_CODE
    if (delay < video.GetSleepMaxFps())
      sleep_fps = video.GetSleepMaxFps() - delay;
    else
      sleep_fps = 0;
#endif

  } while (!jeu.fin_partie); 

  global_particle_engine.Stop();
}


//-----------------------------------------------------------------------------

void GameLoop::RefreshClock()
{
  if (temps.EstPause()) return;

  if (1000 < temps.Lit() - pause_seconde) 
    {
      pause_seconde = temps.Lit();

      switch (state) {

      case gamePLAYING:
	if (duration == 0) {
#ifdef CL
	  jukebox.Play("end_turn");
#else
	  jukebox.Play("share", "end_turn");
#endif
	   SetState (gameEND_TURN);
	} else {
	  duration--;
	  interface.chrono = duration;
	}
	break;

      case gameHAS_PLAYED:
	if (duration == 0) {
	  SetState (gameEND_TURN);
	} else {
	  duration--;
	  interface.chrono = duration;
	}
	break;

      case gameEND_TURN:
	if (duration == 0) {

	  if (IsAnythingMoving()) break;

	  if (jeu.EstFinPartie()) 
	    jeu.fin_partie = true;
	  else { 
	    ActiveTeam().AccessWeapon().Deselect();    
	    caisse.FaitApparaitre();
	    SetState (gamePLAYING);
	    break;
	  }
	} else {
	  duration--;
	}
	break;
      } // switch
    }// if
}

//-----------------------------------------------------------------------------

void GameLoop::SetState(game_state new_state, bool begin_game)
{
  // already in good state, nothing to do 
  if ((state == new_state) && !begin_game) return;

  state = new_state;
  action_handler.ExecActions();

  switch (state)
  {
  // Début d'un tour
  case gamePLAYING:
#ifdef DEBUG_CHG_ETAT
    COUT_DEBUG << "PLAYING" << std::endl;
#endif
    // Init. le compteur
    duration = game_mode.duration_turn;
    interface.chrono = duration;
    pause_seconde = temps.Lit();

#ifdef CL
    if (network.is_server() || network.is_local())
#endif
     wind.ChooseRandomVal();
    
     character_already_chosen = false;

    // Prépare un tour pour un ver
    POUR_TOUS_VERS_VIVANTS(equipe,ver) ver -> PrepareTour();

    // Changement d'équipe
    teams_list.NextTeam (begin_game);
    if( game_mode.allow_character_selection==GameMode::CHANGE_ON_END_TURN
     || game_mode.allow_character_selection==GameMode::BEFORE_FIRST_ACTION_AND_END_TURN)
    {
	    action_handler.NewAction(ActionInt(ACTION_CHANGE_CHARACTER,
				     ActiveTeam().NextCharacterIndex()));
    }

    action_handler.ExecActions();

    assert (!ActiveCharacter().IsDead());
    camera.ChangeObjSuivi (&ActiveCharacter(), true, true);

    interaction_enabled = true; // Be sure that we can play !
    break;

  // Un ver a joué son arme, mais peut encore se déplacer
  case gameHAS_PLAYED:
#ifdef DEBUG_CHG_ETAT
    COUT_DEBUG << "HAS_PLAYED" << std::endl;
#endif
    duration = 5;
    interface.chrono = duration;

    break;

  // Fin du tour : petite pause
  case gameEND_TURN:
#ifdef DEBUG_CHG_ETAT
    COUT_DEBUG << "END_TURN" << std::endl;
#endif
    ActiveTeam().AccessWeapon().SignalTurnEnd();
    duration = game_mode.duration_turn_end;
    interface.chrono = -1 ;
    pause_seconde = temps.Lit();

    interaction_enabled = false; // Be sure that we can NOT play !

    if (!IsAnythingMoving()) ActiveTeam().AccessWeapon().Deselect();
    break;
  }
}

//-----------------------------------------------------------------------------

PhysicalObj* GameLoop::GetMovingObject()
{
  if (!ActiveCharacter().IsReady()) return &ActiveCharacter();

  POUR_TOUS_VERS(equipe,ver)
  {
    if (!ver -> IsReady() && !ver -> IsGhost())
    {
#ifdef DBG_FIN_TOUR
      std::cout << (*ver).m_name << " n'est pas prêt" << std::endl;
#endif

      return &(*ver);
    }
  }

  POUR_CHAQUE_OBJET(objet)
  {
    if (!objet -> ptr -> IsReady())
    {
#ifdef DBG_FIN_TOUR
      std::cout << objet -> ptr -> m_name << " n'est pas prêt" << std::endl;
#endif
      return objet -> ptr;
    }
  }

  return NULL;
}

//-----------------------------------------------------------------------------

bool GameLoop::IsAnythingMoving()
{
  // Is the weapon still active or an object still moving ??
  bool object_still_moving = false;

  if (ActiveTeam().GetWeapon().IsActive()) object_still_moving = true;
  
  if (!object_still_moving)
  {
    PhysicalObj *obj = GetMovingObject();
    if (obj != NULL)
    {
      camera.ChangeObjSuivi (obj, true, true);
      object_still_moving = true;
    } 
  }

  return object_still_moving;
}

//-----------------------------------------------------------------------------

// Signal death of a character
void GameLoop::SignalCharacterDeath (Character *character)
{
  std::string txt;

  if (!jeu.JeuEstLance()) return;

  if (character -> IsDrowned()) {
    txt = Format(_("%s has fallen in water."), character -> m_name.c_str());
    
  } else if (&ActiveCharacter() == character) { // Active Character is dead 
    curseur_ver.Cache();

    // Is this a suicide ?
    if (ActiveTeam().GetWeaponType() == WEAPON_SUICIDE) {
      txt = Format(_("%s commits suicide !"), character -> m_name.c_str());
      
      // Dead in moving ?
    } else if (state == gamePLAYING) {
      txt = Format(_("%s has fallen off the map!"),
		   character -> m_name.c_str());
#ifdef CL
       jukebox.PlayProfile(ActiveTeam().GetSoundProfile(), "out");
#else
       jukebox.Play(ActiveTeam().GetSoundProfile(), "out");
#endif
       
      // Mort en se faisant toucher par son arme / la mort d'un ennemi ?
    } else {
      txt = Format(_("%s is dead because he is clumsy!"), 
		   character -> m_name.c_str());
    }


  } else if ((!ActiveCharacter().IsDead())
	     && (&character -> GetTeam() == &ActiveTeam())) {
    txt = Format(_("%s is a psychopath, he has killed a member of %s team!"),
		 ActiveCharacter().m_name.c_str(), character -> m_name.c_str());
    
  } else if (ActiveTeam().GetWeaponType() == WEAPON_GUN) {
    txt = Format(_("What a shame for %s - he was killed by a simple gun!"),
		 character -> m_name.c_str());
    
  } else {
    // Affiche la mort du ver
    txt = Format(_("%s (%s team) has died."),
		 character -> m_name.c_str(), 
		 character -> GetTeam().GetName().c_str());
  }
  
  game_messages.Add (txt);
  
  // Si c'est le ver actif qui est mort, fin du tour
  if (character == &ActiveCharacter()) SetState (gameEND_TURN);
}

//-----------------------------------------------------------------------------

// Signal falling (with damage) of a character
void GameLoop::SignalCharacterDamageFalling (Character *character)
{
  if (character == &ActiveCharacter())
    {
      SetState (gameEND_TURN);
    }
}

//-----------------------------------------------------------------------------

