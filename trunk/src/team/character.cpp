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
 * Refresh d'un ver de terre.
 *****************************************************************************/

#include <SDL.h>

#include "character.h"
//-----------------------------------------------------------------------------
#include <sstream>
#include <iostream>
#include "../game/game.h"
#include "../game/game_mode.h"
#include "../game/game_loop.h"
#include "../game/time.h"
#include "../game/config.h"
#include "../graphic/graphism.h"
#include "../include/constant.h"
#include "../map/camera.h"
#include "../map/map.h"
#include "../map/water.h"
#include "../sound/jukebox.h"
#include "../tool/random.h"
#include "../tool/math_tools.h"
#include "../weapon/suicide.h"
#include "../weapon/crosshair.h"
#include "../weapon/weapon_tools.h"
#include "../include/action_handler.h"
#include "move.h"
#include "macro.h"

#ifdef CL
#include <ClanLib/core.h>
#include "../tool/geometry_tools.h"
#else
#include <SDL.h>
#include "../tool/Distance.h"
#include "../include/app.h"
#endif
#include <iostream>

using namespace std;
using namespace Wormux;
//-----------------------------------------------------------------------------

const uint HAUT_FONT_MIX = 13;

// Space between the name, the skin and the energy bar
const uint ESPACE = 3; // pixels

#ifdef DEBUG

//#define ANIME_VITE

//#define DEBUG_CHG_ETAT

//#define DEBUG_CADRE_AUTOUR_VER

//#define DEBUG_PLACEMENT

//#define NO_POSITION_CHECK

//#define DRAW_HAND_POSITION

#define COUT_DBG0 cout << "[Character " << m_name << "]"
#define COUT_DBG COUT_DBG0 " "
#define COUT_PLACEMENT COUT_DBG0 "[Init bcl=" << bcl << "] "
#endif

// Pause for the animation
#ifdef ANIME_VITE
  const uint ANIM_PAUSE_MIN = 0*1000;
  const uint ANIM_PAUSE_MAX = 3*1000;
#else
  const uint ANIM_PAUSE_MIN = 5*1000;
  const uint ANIM_PAUSE_MAX = 60*1000;
#endif

// Barre d'énergie
const uint LARG_ENERGIE = 40;
const uint HAUT_ENERGIE = 6;

//-----------------------------------------------------------------------------

Character::Character () : PhysicalObj("Soldat inconnu", 0.0)
{
  pause_bouge_dg = 0;
  m_team = NULL;
  energy = 100;
  losted_energy = 0;
  desactive = false;
  skin = NULL;
  walk_skin = NULL;
  m_wind_factor = 0.0;
  m_rebound_factor = 0.25;
  m_rebounding = true;
  is_walking = true;
  current_skin = "";
  channel_step = -1;

#ifndef CL
  name_surface = NULL;
#endif
}

//-----------------------------------------------------------------------------

// Signale la mort d'un ver
void Character::SignalDeath()
{
#ifdef DEBUG_CHG_ETAT
  COUT_DBG << "Meurt." << endl;
#endif

  // No more energy ...
  energy = 0;

#ifdef CL
  jukebox.PlayProfile(GetTeam().GetSoundProfile(),"death");
#else
  jukebox.Play(GetTeam().GetSoundProfile(),"death");
#endif
   
  SetSkin("dead");

  //--- Do the skin explosion :-) ---
#ifdef CL
  CL_Surface img_trou = suicide.hole_image;
#else
  SDL_Surface *img_trou = suicide.hole_image;
#endif
   
  // The hole is at the center
#ifdef CL
  CL_Point trou(GetCenter());
#else
  Point2i trou(GetCenter());
#endif
   
  // But the explosion is on the foots :)
#ifdef CL
  CL_Point explosion(GetCenterX(), GetY()+ GetHeight());
#else
   Point2i explosion(GetCenterX(), GetY()+ GetHeight());
#endif
   
  AppliqueExplosion (explosion, trou, img_trou, suicide.cfg(), NULL);

  // Change test rectangle
  int x = GetCenterX(), y=GetCenterY();
#ifdef CL
  SetSize (image.get_width(), image.get_height());
  SetXY (x - GetWidth()/2, y - GetHeight()/2);
#else
  SetSize (image->GetWidth(), image->GetHeight());
  SetXY (x - GetWidth()/2, y - GetHeight()/2);
#endif

  assert (m_alive == DEAD);
  assert (IsDead());
  
  // Signal the death
  game_loop.SignalCharacterDeath (this);
}

//-----------------------------------------------------------------------------

void Character::SignalDrowning()
{
  energy = 0;
  //Desactive();
  //m_type = objINSENSIBLE;
  SetSkin("drowned");
#ifdef CL
  jukebox.PlayProfile(GetTeam().GetSoundProfile(),"sink");
  game_loop.SignalCharacterDeath (this);
#else
  jukebox.Play (GetTeam().GetSoundProfile(),"sink");
  game_loop.SignalCharacterDeath (this);
#endif
  
}

//-----------------------------------------------------------------------------

// Si un ver devient un fantome, il meurt ! Signale sa mort
void Character::SignalGhostState (bool was_dead)
{
  // Désactive le ver
  Desactive();

#ifdef DEBUG_CHG_ETAT
  COUT_DBG << "Fantome." << endl;
#endif

  // Signal the death
  if (!was_dead) game_loop.SignalCharacterDeath (this);
}

//-----------------------------------------------------------------------------

void Character::SetDirection (int nv_direction)
{ 
#ifdef CL
   image.set_scale (nv_direction, 1); 
#else
   image->Scale (nv_direction, 1);
#endif
   
   if (GetSkin().anim.utilise) 
#ifdef CL
     anim.image.set_scale (nv_direction, 1);
#else
     anim.image->Scale (nv_direction, 1);
#endif
}

//-----------------------------------------------------------------------------

void Character::DrawEnergyBar(int dy) const
{
  energy_bar.DrawXY ( GetCenterX()-energy_bar.GetWidth()/2-camera.GetX(), 
		      GetY()+dy-camera.GetY());
}

//-----------------------------------------------------------------------------

void Character::DrawName (int dy) const
{
  const int x =  GetCenterX();
  const int y = GetY()+dy;
#ifdef CL
   police_mix.WriteCenterTop (x,y,m_name);
#else
   //small_font.WriteCenterTop (x-camera.GetX(),y-camera.GetY(),m_name,white_color);
   SDL_Rect dst_rect;
   dst_rect.x = x-camera.GetX() - small_font.GetWidth(m_name)/2; 
   dst_rect.y = y-camera.GetY();
   dst_rect.h = name_surface->h;
   dst_rect.w = name_surface->w;

   SDL_BlitSurface(name_surface, NULL, app.sdlwindow, &dst_rect);
#endif
}

//-----------------------------------------------------------------------------

void Character::SetEnergyDelta (int delta)
{
  // If already dead, do nothing
  if (IsDead()) return;

  uint sauve_energie = energy;
  uchar R,V,B;

  // Change energy
  energy = BorneLong((int)energy +delta, 0, game_mode.character.max_energy);
  energy_bar.Actu (energy);
    
  // Energy bar color
  if (70 < energy) {
    V = 255;
    R = 0;
  } else if (50 < energy) {
    V = 255;
    R = 255;
  } else if (20 < energy) {
    V = 128;
    R = 255;
  } else {
    V = 0;
    R = 255;
  }
  B = 0;

#ifdef CL
  energy_bar.value_color = CL_Color(R,V,B);
#else
  energy_bar.SetValueColor( R,V,B);
#endif
   
  // Compute energy lost
  if (delta < 0)
  {

    losted_energy += (int)energy - (int)sauve_energie;
#ifdef CL
    if ( losted_energy < 33 )
      jukebox.PlayProfile(GetTeam().GetSoundProfile(), "injured_light");
    else if ( losted_energy < 66 )
      jukebox.PlayProfile(GetTeam().GetSoundProfile(), "injured_medium");
    else 
      jukebox.PlayProfile(GetTeam().GetSoundProfile(), "injured_high");
#else
    if ( losted_energy < 33 )
      jukebox.Play (GetTeam().GetSoundProfile(), "injured_light");
    else if ( losted_energy < 66 )
      jukebox.Play (GetTeam().GetSoundProfile(), "injured_medium");
    else 
      jukebox.Play (GetTeam().GetSoundProfile(), "injured_high");
#
#endif

  }
  else 
    losted_energy = 0;

  // "Friendly fire !!"
  if ( (&ActiveCharacter() != this) && (&ActiveTeam() == m_team) )
#ifdef CL
    jukebox.PlayProfile(GetTeam().GetSoundProfile(), "friendly_fire");
#else
  jukebox.Play (GetTeam().GetSoundProfile(), "friendly_fire");
#endif
   
  // Dead character ?
  if (energy == 0) Die();
}

//-----------------------------------------------------------------------------

void Character::Draw()
{
  bool dessine_perte = (losted_energy != 0);
  if ((&ActiveCharacter() == this) 
      && (game_loop.ReadState() != gameEND_TURN)
      //&& (game_loop.ReadState() != jeuANIM_FIN_TOUR)
      )
    dessine_perte = false;

  // Gone in another world ?
  if (!IsActive()) return;

  // Draw skin
  if(!is_walking) //walking skins image update only when a keyboard key is pressed
#ifdef CL
    image.update();
#else
   image->Update();
#endif
   
  int x = GetX();
  int y = GetY();
#ifdef CL
  if (GetDirection()<0) x += GetWidth();
  image.draw (x,y);
#else
  image->Draw(x,y);
#endif
   
#ifdef DRAW_HAND_POSITION
  int a,b,c=3;
  GetHandPosition (a,b);
  CL_Display::draw_line (a-c,b-c, a+c,b+c, CL_Color::blue);
  CL_Display::draw_line (a+c,b-c, a-c,b+c, CL_Color::blue);
#endif

  // Draw animation
  if (anim.draw && current_skin=="walking")
#ifdef CL
   anim.image.draw (x,y);
#else
   anim.image->Draw(x,y);
#endif

   // Draw energy bar
  int dy = -ESPACE;
  bool est_ver_actif = (this == &ActiveCharacter());
  bool affiche_energie = config.affiche_energie_ver;
  affiche_energie &= !est_ver_actif || (game_loop.ReadState() != gamePLAYING);
  affiche_energie |= dessine_perte;
  if (affiche_energie)
  { 
    dy -= HAUT_ENERGIE; 
    DrawEnergyBar (dy); 
    dy -= ESPACE; 
  }

  // Draw name
  if (config.affiche_nom_ver && !est_ver_actif) 
  { 
    dy -= HAUT_FONT_MIX;
    DrawName (dy);
    dy -= ESPACE; 
  }

  // Draw lost energy
  if (dessine_perte)
  {
    ostringstream ss;
    ss << losted_energy;
    dy -= HAUT_FONT_MIX;
#ifdef CL
    police_mix.WriteCenterTop (GetX() +GetWidth()/2, GetY()+dy, ss.str());    
#else
    small_font.WriteCenterTop (GetX() +GetWidth()/2-camera.GetX(), GetY()+dy-camera.GetY(), ss.str(), white_color);    
#endif
  }

#if defined(DEBUG_CADRE_AUTOUR_VER)
  CL_Display::draw_rect (CL_Rect(GetX(), GetY(),
				 GetX()+image.get_width(), 
				 GetY()+image.get_height()),
			 CL_Color::blue);
  CL_Display::draw_rect (GetTestRect(), CL_Color::red);
#endif
}

//-----------------------------------------------------------------------------

void Character::Saute ()
{
#ifdef DEBUG_CHG_ETAT
  COUT_DBG << "Saute." << endl;
#endif
  if (!CanJump()) return;

#ifdef CL
  jukebox.PlayProfile(ActiveTeam().GetSoundProfile(), "jump");
#else
  jukebox.Play (ActiveTeam().GetSoundProfile(), "jump");
#endif
   
  m_rebounding = false;

  if(current_skin=="walking")
    SetSkin("jump");

  // Initialise la force
  double angle = Deg2Rad(game_mode.character.jump_angle);
  if (GetDirection() == -1) angle = InverseAngle(angle);
  SetSpeed (game_mode.character.jump_strength, angle);
}

//-----------------------------------------------------------------------------

void Character::SuperSaut ()
{
#ifdef DEBUG_CHG_ETAT
  COUT_DBG << "SuperSaut." << endl;
#endif
  if (!CanJump()) return;

  m_rebounding = false;

#ifdef CL
  jukebox.PlayProfile(ActiveTeam().GetSoundProfile(), "superjump");
#else
  jukebox.Play (ActiveTeam().GetSoundProfile(), "superjump");
#endif
   
  // Initialise la force
  double angle = Deg2Rad(game_mode.character.super_jump_angle);
  if (GetDirection() == -1) angle = InverseAngle(angle);
  SetSpeed (game_mode.character.super_jump_strength, angle);
}

//-----------------------------------------------------------------------------

void Character::DoShoot()
{
  ActiveTeam().AccessWeapon().StopLoading();
  ActiveTeam().GetWeapon().NewActionShoot();
}

//-----------------------------------------------------------------------------

void Character::HandleShoot(int event_type)
{
  switch (event_type) {
    case KEY_PRESSED:
      if (ActiveTeam().GetWeapon().max_strength == 0)
	DoShoot();
      else
	if ( (game_loop.ReadState() == gamePLAYING)
	     && ActiveTeam().GetWeapon().IsReady() )
	  ActiveTeam().AccessWeapon().InitLoading();
      break ;

    case KEY_RELEASED:
      if (ActiveTeam().GetWeapon().IsLoading())
	DoShoot();
      break ;

    case KEY_REFRESH:
      if ( ActiveTeam().GetWeapon().IsLoading() )
	{
	  // Strength == max strength -> Fire !!!
	  if (ActiveTeam().GetWeapon().ReadStrength() >=
	      ActiveTeam().GetWeapon().max_strength)
	    DoShoot();
	  else
	    {
	      // still pressing the Space key
	      ActiveTeam().AccessWeapon().UpdateStrength();
	    }
	}
      break ;

    default:
      break ;
  }
}

//-----------------------------------------------------------------------------

void Character::HandleKeyEvent(int action, int event_type)
{
  // The character cannot move anymove if the turn is over...
  if (game_loop.ReadState() == gameEND_TURN)
    return ;

  if (ActiveCharacter().IsDead())
    return;

  if (action == ACTION_SHOOT)
    {
      HandleShoot(event_type);
       std::cout << "SHOOT" << std::endl;
       return;
    }

  if (!ActiveCharacter().IsReady())
    return;

  if(action <= ACTION_CHANGE_CHARACTER)
    {
      switch (event_type)
      {
        case KEY_PRESSED:
          switch (action)
          {
            case ACTION_JUMP:
	      action_handler.NewAction (Action(ACTION_JUMP));
	      return ;

            case ACTION_SUPER_JUMP:
	      action_handler.NewAction (Action(ACTION_SUPER_JUMP));
	      return ;
            default:
	      break ;
          }
          //no break!! -> it's normal

        case KEY_REFRESH:
          switch (action) {
            case ACTION_MOVE_LEFT:
              if(event_type==KEY_PRESSED)
                InitMouvementDG(PAUSE_BOUGE);
              MoveCharacterLeft(ActiveCharacter());
                //        action_handler.NewAction (Action(ACTION_MOVE_LEFT));
              break ;

            case ACTION_MOVE_RIGHT:
              if(event_type==KEY_PRESSED)
                InitMouvementDG(PAUSE_BOUGE);
    	      MoveCharacterRight(ActiveCharacter());
    	      //        action_handler.NewAction (Action(ACTION_MOVE_RIGHT));
  	      break ;

            case ACTION_UP:
    	      if (ActiveTeam().crosshair.enable)
	        action_handler.NewAction (Action(ACTION_UP));
	      break ;

            case ACTION_DOWN:
	      if (ActiveTeam().crosshair.enable)
	        action_handler.NewAction (Action(ACTION_DOWN));
	      break ;
            default:
	      break ;
          }
          break;
        case KEY_RELEASED: break;
        default: break;
      }
    }
}

//-----------------------------------------------------------------------------

void Character::Refresh()
{
  if (desactive) return;

  UpdatePosition ();

  // Refresh de l'animation
  if (GetSkin().anim.utilise)
  {
    // C'est le début d'une animation ?
    if (!anim.draw && (anim.time < temps.Lit())) 
    {
#ifdef CL
      anim.image.restart();
      anim.draw = true;
#else
      anim.image->SetCurrentFrame(0);
      anim.draw = true;
#endif
    }

    // Animation active
    if (anim.draw)
    {
#ifdef CL
      anim.image.update();
#else
      anim.image->Update();
#endif
       
#ifdef CL
      if (anim.image.is_finished())
#else
      if ( anim.image->IsFinished() )
#endif
      {
        anim.draw = false;
        anim.time  = RandomLong (ANIM_PAUSE_MIN, ANIM_PAUSE_MAX);
        anim.time += temps.Lit();
      }
    }
  }
}

//-----------------------------------------------------------------------------

// Prepare a new turn
void Character::PrepareTour ()
{
  losted_energy = 0;
  pause_bouge_dg = temps.Lit();
}

//-----------------------------------------------------------------------------

const Team& Character::GetTeam() const
{
  assert (m_team != NULL);
  return *m_team;
}

//-----------------------------------------------------------------------------

Team& Character::TeamAccess()
{
  assert (m_team != NULL);
  return *m_team;
}

//-----------------------------------------------------------------------------

bool Character::MouvementDG_Autorise() const
{
  if (!IsReady() || IsFalling()) return false;
  return pause_bouge_dg < temps.Lit();
}

//-----------------------------------------------------------------------------

bool Character::CanJump() const
{
	return MouvementDG_Autorise();
}

//-----------------------------------------------------------------------------

void Character::InitMouvementDG(uint pause)
{
  m_rebounding = false;
  pause_bouge_dg = temps.Lit()+pause;
}

//-----------------------------------------------------------------------------

bool Character::CanStillMoveDG(uint pause)
{
  if(pause_bouge_dg+pause<temps.Lit())
  {
    pause_bouge_dg += pause;
    return true;
  }
  return false;
}

//-----------------------------------------------------------------------------

// Signal the end of a fall
void Character::SignalFallEnding()
{
  // Do not manage dead worms.
  if (IsDead()) return;

  pause_bouge_dg = temps.Lit();

  double norme, degat;
  DoubleVector speed_vector ;

  GetSpeedXY (speed_vector);
  norme = Norm (speed_vector);
  if (norme > game_mode.safe_fall && speed_vector.y>0.0)
  {
    norme -= game_mode.safe_fall;
    degat = norme * game_mode.damage_per_fall_unit;
    SetEnergyDelta (-(int)degat);

    game_loop.SignalCharacterDamageFalling(this);
  }
  if(current_skin=="jump")
    SetSkin("walking");
}

//-----------------------------------------------------------------------------

int Character::GetDirection() const 
{ 
  float x,y;
#ifdef CL
  image.get_scale(x,y);
#else
   image->GetScaleFactors(x,y);
#endif
   return (x<0)?-1:1;
}

//-----------------------------------------------------------------------------

bool Character::IsActive() const 
{ return !desactive; }

//-----------------------------------------------------------------------------

// Désactive le ver
void Character::Desactive()
{
  desactive = true;
}

//-----------------------------------------------------------------------------

// Réactive le ver
void Character::Reactive()
{
  assert (!IsGhost());
  desactive = false;
}

//-----------------------------------------------------------------------------

// Accès à l'avatar
const Skin& Character::GetSkin() const
{
  assert (skin != NULL);
  return *skin;
}

//-----------------------------------------------------------------------------

Skin& Character::AccessSkin()
{
  assert (skin != NULL);
  return *skin;
}

//-----------------------------------------------------------------------------
// Choose which skin to display (ie. dead skin, swiming skin...)
void Character::SetSkin(std::string skin_name)
{
  if(skin_name == current_skin) return;

  assert (skin != NULL);
  if(AccessSkin().many_skins.find(skin_name) != 
     AccessSkin().many_skins.end())
  {
    float sc_x,sc_y;
    if(current_skin!="")
#ifdef CL
       image.get_scale(sc_x,sc_y);
#else
       image->GetScaleFactors(sc_x,sc_y);
#endif
    
#ifdef CL
    image = AccessSkin().many_skins[skin_name].image;
#else
    image = new Sprite( *(AccessSkin().many_skins[skin_name].image));
#endif
    SetTestRect (AccessSkin().many_skins[skin_name].test_dx, 
                 AccessSkin().many_skins[skin_name].test_dx, 
                 AccessSkin().many_skins[skin_name].test_top, 
                 AccessSkin().many_skins[skin_name].test_bottom);
#ifdef CL
    SetSize (image.get_width(), image.get_height());
#else
    SetSize (image->GetWidth(), image->GetHeight());
#endif
     
    //Restore skins direction
    if(current_skin!="" && sc_x<0)
#ifdef CL
       image.set_scale(-1,1);
#else
       image->Scale(-1,1);
#endif
    current_skin = skin_name;
    is_walking = false;
    return;
  }
  else
  if(AccessSkin().many_walking_skins.find(skin_name) != 
     AccessSkin().many_walking_skins.end())
  {
    float sc_x,sc_y;
    if(current_skin!="")
#ifdef CL
       image.get_scale(sc_x,sc_y);
#else
       image->GetScaleFactors(sc_x,sc_y);
#endif

    walk_skin = &AccessSkin().many_walking_skins[skin_name];
#ifdef CL
    image = walk_skin->image;
#else
    image = new Sprite(*(walk_skin->image));
#endif
    SetTestRect (walk_skin->test_dx, 
                 walk_skin->test_dx,
                 walk_skin->test_top,
                 walk_skin->test_bottom);
    m_frame_repetition = walk_skin->repetition_frame;
#ifdef CL
    SetSize (image.get_width(), image.get_height());
#else
    SetSize (image->GetWidth(), image->GetHeight());
#endif
    //Restore skins direction
    if(current_skin!="" && sc_x<0.0)
#ifdef CL
      image.set_scale(-1.0,1.0);
#else
      image->Scale( -1.0,1.0);
#endif
     
    if(skin_name=="walking")
    {
        anim.draw = false;
        anim.time  = RandomLong (ANIM_PAUSE_MIN, ANIM_PAUSE_MAX);
        anim.time += temps.Lit();
    }

    current_skin = skin_name;
    is_walking = true;
    return;
  }
  else
  {
    std::cout << "Unable to set skin : " << skin_name << "\n";
    assert(skin_name!="walking");
    SetSkin("walking");
  }
}

//-----------------------------------------------------------------------------

void Character::FrameImageSuivante()
{
  m_image_frame++;
#ifdef CL
  if (image.get_frame_count()-1 < (int)(m_image_frame/m_frame_repetition)) 
    m_image_frame = 0;
  image.set_frame (m_image_frame/m_frame_repetition);
#else
  if (image->GetFrameCount()-1 < (int)(m_image_frame/m_frame_repetition)) 
    m_image_frame = 0;
  image->SetCurrentFrame (m_image_frame/m_frame_repetition); 
#endif
   
#ifdef CL
  static CL_SoundBuffer_Session *step_session = NULL;
   
  bool play = (step_session == NULL) || (!step_session -> is_playing());

  if (play)
    jukebox.PlayProfile(m_team->GetSoundProfile(), "step", false, &step_session);
#else
  if ( channel_step == -1 || !Mix_Playing(channel_step) ) {
    channel_step = jukebox.Play (m_team->GetSoundProfile(), "step");
  }
#endif
}

//-----------------------------------------------------------------------------

void Character::Init() {}

//-----------------------------------------------------------------------------

void Character::InitTeam (Team *ptr_equipe, const string &name, 
			  Skin* pskin)
{
  SetMass (game_mode.character.mass);
  SetAirResistFactor (game_mode.character.air_resist_factor);
  m_team = ptr_equipe;
  m_name = name;
  skin = pskin;

  // Animation ?
  anim.draw = false;
  if (GetSkin().anim.utilise)
  { 
    anim.draw = true;
#ifdef CL
    anim.image = GetSkin().anim.image;
#else
    anim.image = new Sprite(*GetSkin().anim.image);
#endif
  }

  // Energie
  energy_bar.InitVal (energy, 0, game_mode.character.max_energy);
  energy_bar.InitPos (0,0, LARG_ENERGIE, HAUT_ENERGIE);
#ifdef CL
  energy_bar.border_color = CL_Color::black;
  energy_bar.background_color = CL_Color::dimgray;
#else
  energy_bar.SetBorderColor(0,0,0);
  energy_bar.SetBackgroundColor(100,100,100);
#endif
}

//-----------------------------------------------------------------------------

void Character::Reset() 
{
  // Reset de l'état du ver
  desactive = false;
  Ready();

  //  Reset de l'image et les dimensions
  SetSkin("walking");

  // Animation ?
  if (anim.draw)
  { 
#ifdef CL
    anim.image.finish();
#else
    anim.image->Finish();
#endif 
    anim.time  = RandomLong (ANIM_PAUSE_MIN, ANIM_PAUSE_MAX);
    anim.time += temps.Lit();    
  }

  // Initialise l'image
  SetDirection( RandomBool()?1:-1 );
#ifdef CL
  image.set_frame ( RandomLong(0, image.get_frame_count()-1) );
  m_image_frame = image.get_current_frame();
#else
  image->SetCurrentFrame ( RandomLong(0, image->GetFrameCount()-1) );
  m_image_frame = image->GetCurrentFrame();   
#endif

#ifndef CL
  // Prépare l'image du nom
  if (config.affiche_nom_ver)
    name_surface = small_font.Render(m_name, white_color);
#endif

  // Energie
  energy = game_mode.character.init_energy-1;
  energy_bar.InitVal (energy, 0, game_mode.character.init_energy);
  SetEnergyDelta (1);
  losted_energy = 0;

  std::cout << "Reset character..." << std::endl;
   
  // Initialise la position
  uint bcl=0;
  bool pos_ok;
  do
  {
    // Vérifie qu'on ne tourne pas en rond
    FORCE_ASSERT (++bcl < NBR_BCL_MAX_EST_VIDE);

    // Objet physique dans l'état prêt
    pos_ok = true;
    desactive = false;
    Ready();

    // Prend des coordonnées au hasard
    uint x = RandomLong(0, monde.GetWidth() -GetWidth());
    uint y = RandomLong(0, monde.GetHeight() -GetHeight());

    SetXY (x, y);

#ifndef NO_POSITION_CHECK
    pos_ok &= !IsGhost() && IsInVacuum(0,0) && (GetY() < static_cast<int>(monde.GetHeight()) - (WATER_INITIAL_HEIGHT + 30));
    if (!pos_ok) continue;

    // Chute directe pour le sol
    DirectFall ();
    pos_ok &= !IsGhost() && (GetY() < static_cast<int>(monde.GetHeight()) - (WATER_INITIAL_HEIGHT + 30));
#ifdef DEBUG_PLACEMENT
    if (!pos_ok) COUT_PLACEMENT << "Fantome en tombant." << endl;
#endif
    if (!pos_ok) continue;

    // Vérifie que le ver ne fois pas trop près de ses voisins
    POUR_TOUS_VERS_VIVANTS(it_equipe,ver) if (&(*ver) != this)
    {
#ifdef CL
      double dst = Distance( ver->GetCenter(), GetCenter());
#else
       Point2i p1 = ver->GetCenter();
       Point2i p2 = GetCenter();
       double dst = Distance ( p1, p2);
#endif
      if (dst < monde.dst_min_entre_vers) {
	pos_ok = false;
      }
       
    }

    // La position est bonne ?
    pos_ok &= !IsGhost() & IsInVacuum(0,0);
#ifdef DEBUG_PLACEMENT
    if (!pos_ok) COUT_PLACEMENT << "Placement final manqué." << endl;
#endif

#endif // of #ifndef NO_POSITION_CHECK
  } while (!pos_ok);
  assert (!IsDead());
  Ready();
}

//-----------------------------------------------------------------------------

uint Character::GetEnergy() const 
{
  assert (!IsDead());
  return energy; 
}

//-----------------------------------------------------------------------------

// Hand position
void Character::GetHandPosition (int &x, int &y) {
#ifdef CL
   int frame = image.get_current_frame();
#else
   int frame = image->GetCurrentFrame();
#endif
   
  assert(walk_skin!=NULL);
  skin_translate_t hand = walk_skin->hand_position.at(frame);
  y = GetY() +hand.dy;
  if (GetDirection() == 1)
    x = GetX() +hand.dx;
  else
    x = GetX() +GetWidth() -hand.dx;
}

//-----------------------------------------------------------------------------

// Hand position
void Character::GetHandPositionf (double &x, double &y) {
#ifdef CL
   int frame = image.get_current_frame();
#else
   int frame = image->GetCurrentFrame();
#endif
  assert(walk_skin!=NULL);
  skin_translate_t hand = walk_skin->hand_position.at(frame);

  y = (double)GetY() +hand.dy;
  if (GetDirection() == 1) 
    x = (double)GetX() +hand.dx;
  else
    x = (double)GetX() +GetWidth() -hand.dx;
}

//-----------------------------------------------------------------------------
void Character::EndTurn()
{
  m_rebounding = true;
}

//-----------------------------------------------------------------------------
