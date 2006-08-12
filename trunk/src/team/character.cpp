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

#include "character.h"
#include <SDL.h>
#include <sstream>
#include <iostream>
#include "body.h"
#include "macro.h"
#include "move.h"
#include "../game/game.h"
#include "../game/game_mode.h"
#include "../game/game_loop.h"
#include "../game/time.h"
#include "../game/config.h"
#include "../graphic/text.h"
#include "../graphic/font.h"
#include "../include/action_handler.h"
#include "../include/app.h"
#include "../include/constant.h"
#include "../interface/cursor.h"
#include "../map/camera.h"
#include "../map/map.h"
#include "../map/water.h"
#include "../network/network.h"
#include "../network/randomsync.h"
#include "../sound/jukebox.h"
#include "../tool/debug.h"
#include "../tool/random.h"
#include "../tool/math_tools.h"
#include "../weapon/suicide.h"
#include "../weapon/crosshair.h"
#include "../weapon/explosion.h"

const uint HAUT_FONT_MIX = 13;

// Space between the name, the skin and the energy bar
const uint ESPACE = 3; // pixels
const uint do_nothing_timeout = 5000;
const double MIN_SPEED_TO_FLY = 7.0;

// Pause for the animation
#ifdef DEBUG
//#define ANIME_VITE
#endif
#ifdef ANIME_VITE
  const uint ANIM_PAUSE_MIN = 100;
  const uint ANIM_PAUSE_MAX = 150;
#else
  const uint ANIM_PAUSE_MIN = 5*1000;
  const uint ANIM_PAUSE_MAX = 60*1000;
#endif

#ifdef DEBUG
//#define DEBUG_STATS
#endif

// Barre d'�ergie
const uint LARG_ENERGIE = 40;
const uint HAUT_ENERGIE = 6;

Character::Character (Team& my_team, const std::string &name) :
  PhysicalObj("character"), m_team(my_team)
{
  m_goes_through_wall = false;
  m_collides_with_characters = true;
  m_collides_with_objects = false;

  body = NULL;
  step_sound_played = true;
  pause_bouge_dg = 0;
  previous_strength = 0;
  energy = 100;
  lost_energy = 0;
  channel_step = -1;
  hidden = false;
  do_nothing_time = 0;
  m_allow_negative_y = true;
  animation_time = Time::GetInstance()->Read() + randomObj.GetLong(ANIM_PAUSE_MIN,ANIM_PAUSE_MAX);
  prepare_shoot = false;

  // Damage count
  damage_other_team = 0;
  damage_own_team = 0;
  max_damage = 0;
  current_total_damage = 0;

  // Survivals
  survivals = 0;

  character_name = name;

  ResetConstants();

  // Name Text object
  if (Config::GetInstance()->GetDisplayNameCharacter())
    name_text = new Text(character_name);
  else
    name_text = NULL;

  // Energy
  {
    energy_bar.InitVal (energy, 0, GameMode::GetInstance()->character.max_energy);
    energy_bar.InitPos (0,0, LARG_ENERGIE, HAUT_ENERGIE);

    energy_bar.SetBorderColor( black_color );
    energy_bar.SetBackgroundColor( gray_color );

    energy = GameMode::GetInstance()->character.init_energy-1;
    energy_bar.InitVal (energy, 0, GameMode::GetInstance()->character.init_energy);
    SetEnergyDelta (1, false);
    lost_energy = 0;
  }
  MSG_DEBUG("character", "Load character %s", character_name.c_str());
}

Character::~Character()
{
  MSG_DEBUG("character", "Unload character %s", character_name.c_str());
  if(body)
    delete body;
}

void Character::SetBody(Body* _body)
{
  body = _body;
  body->owner = this;
  SetClothe("normal");
  SetMovement("walk");

  SetDirection( randomSync.GetBool()?1:-1 );
  body->SetFrame( randomSync.GetLong(0, body->GetFrameCount()-1) );
  SetSize(body->GetSize());
}

// Signale la mort d'un ver
void Character::SignalDeath()
{
  MSG_DEBUG("character", "Dying");

  // No more energy ...
  energy = 0;

  jukebox.Play(GetTeam().GetSoundProfile(),"death");
  SetClothe("dead");
  SetMovement("dead");

  ExplosiveWeaponConfig cfg;

  ApplyExplosion ( GetCenter(), cfg);

  // Change test rectangle
  SetSize( body->GetSize() );
  SetXY( GetCenter() - GetSize()/2 );

  assert (m_alive == DEAD);
  assert (IsDead());
  
  // Signal the death
  GameLoop::GetInstance()->SignalCharacterDeath (this);
}

void Character::SignalDrowning()
{
  energy = 0;
  SetMovement("drowned");

  jukebox.Play (GetTeam().GetSoundProfile(),"sink");
  GameLoop::GetInstance()->SignalCharacterDeath (this);
}

// Si un ver devient un fantome, il meurt ! Signale sa mort
void Character::SignalGhostState (bool was_dead)
{  
  // Report to damage performer this character lost all of its energy
  ActiveCharacter().MadeDamage(energy, *this);

  MSG_DEBUG("character", "ghost");

  // Signal the death
  if (!was_dead) GameLoop::GetInstance()->SignalCharacterDeath (this);
}

void Character::SetDirection (int nv_direction)
{ 
  body->SetDirection(nv_direction);
  uint l,r,t,b;
  body->GetTestRect(l,r,t,b);
  SetTestRect(l,r,t,b);
}

void Character::DrawEnergyBar(int dy)
{
  if( IsDead() )
	return;

  energy_bar.DrawXY( Point2i( GetCenterX() - energy_bar.GetWidth() / 2, GetY() + dy)  
		  - camera.GetPosition() );
}

void Character::DrawName (int dy) const
{
  if(IsDead()) return;

  const int x =  GetCenterX();
  const int y = GetY()+dy;

  if (Config::GetInstance()->GetDisplayNameCharacter())
  {
    name_text->DrawCenterTopOnMap(x,y);
  }
}

void Character::SetEnergyDelta (int delta, bool do_report)
{
  // If already dead, do nothing
  if (IsDead()) return;

  // Report damage to damage performer
  if (do_report)
    ActiveCharacter().MadeDamage(-delta, *this);

  uint sauve_energie = energy;
  Color color;

  // Change energy
  energy = BorneLong((int)energy +delta, 0, GameMode::GetInstance()->character.max_energy);
  energy_bar.Actu (energy);
    
  // Energy bar color
  if (70 < energy)
	  color.SetColor(0, 255, 0, 255);
  else if (50 < energy)
	  color.SetColor(255, 255, 0, 255);
  else if (20 < energy) 
	  color.SetColor(255, 128, 0, 255);
  else 
	  color.SetColor(255, 0, 0, 255);

  energy_bar.SetValueColor( color );

   
  // Compute energy lost
  if (delta < 0)
  {

    lost_energy += (int)energy - (int)sauve_energie;

    if ( lost_energy < 33 )
      jukebox.Play (GetTeam().GetSoundProfile(), "injured_light");
    else if ( lost_energy < 66 )
      jukebox.Play (GetTeam().GetSoundProfile(), "injured_medium");
    else 
      jukebox.Play (GetTeam().GetSoundProfile(), "injured_high");
    
  }
  else 
    lost_energy = 0;

  // "Friendly fire !!"
  if ( (&ActiveCharacter() != this) && (&ActiveTeam() == &m_team) )
  jukebox.Play (GetTeam().GetSoundProfile(), "friendly_fire");
   
  // Dead character ?
  if (energy == 0) Die();
}

void Character::Draw()
{
  if (hidden) return;

  // Gone in another world ?
  if (IsGhost()) return;

  bool dessine_perte = (lost_energy != 0);
  if ((&ActiveCharacter() == this
    && GameLoop::GetInstance()->ReadState() != GameLoop::END_TURN)
      //&& (game_loop.ReadState() != jeuANIM_FIN_TOUR)
    || IsDead()
     )
    dessine_perte = false;

  if(GameLoop::GetInstance()->ReadState() == GameLoop::END_TURN && body->IsWalking())
    body->ResetWalk();

  if(Time::GetInstance()->Read() > animation_time && &ActiveCharacter()!=this && !IsDead())
  {
    body->PlayAnimation();
    animation_time = Time::GetInstance()->Read() + body->GetMovementDuration() + randomObj.GetLong(ANIM_PAUSE_MIN,ANIM_PAUSE_MAX);
  }

  // Stop the animation if we are playing
  if(&ActiveCharacter() == this && body->GetMovement().substr(0,9) == "animation")
  {
    SetClothe("normal");
    SetMovement("walk");
  }

  // Stop flying if we don't go fast enough
  double n, a;
  GetSpeed(n, a);
  if(body->GetMovement() == "fly" && n < MIN_SPEED_TO_FLY)
    SetMovement("walk");


  Point2i pos = GetPosition();

  if(prepare_shoot)
  {
    body->Build(Point2i(0,0)); // Refresh the body
    if(body->GetMovement() != "weapon-" + ActiveTeam().GetWeapon().GetID() + "-begin-shoot")
    {
      // if the movement is finnished, shoot !
      DoShoot();
      prepare_shoot = false;
    }
  }

  body->Draw(pos);

   // Draw energy bar
  int dy = -ESPACE;
  bool est_ver_actif = (this == &ActiveCharacter());
  Config * config = Config::GetInstance();
  bool display_energy = config->GetDisplayEnergyCharacter();
  display_energy &= !est_ver_actif || (GameLoop::GetInstance()->ReadState() != GameLoop::PLAYING);
  display_energy |= dessine_perte;
  display_energy &= !IsDead();
  if (display_energy)
  { 
    dy -= HAUT_ENERGIE; 
    DrawEnergyBar (dy); 
    dy -= ESPACE; 
  }

  // Draw name
  if (config->GetDisplayNameCharacter() && !est_ver_actif) 
  { 
    dy -= HAUT_FONT_MIX;
    DrawName (dy);
    dy -= ESPACE; 
  }

  // Draw lost energy
  if (dessine_perte)
  {
    std::ostringstream ss;
    ss << lost_energy;
    dy -= HAUT_FONT_MIX;
    (*Font::GetInstance(Font::FONT_SMALL)).WriteCenterTop (
			GetPosition() - camera.GetPosition() + Point2i( GetWidth()/2, dy),
		   	ss.str(), white_color);    
  }

}

void Character::Jump ()
{
  MSG_DEBUG("character", "Jump");
  do_nothing_time = Time::GetInstance()->Read();

  if (!CanJump()) return;

  jukebox.Play (ActiveTeam().GetSoundProfile(), "jump");
   
  SetRebounding(false);
  SetMovement("jump");

  // Initialise la force
  double angle = Deg2Rad(GameMode::GetInstance()->character.jump_angle);
  if (GetDirection() == -1) angle = InverseAngle(angle);
  SetSpeed (GameMode::GetInstance()->character.jump_strength, angle);
}

void Character::HighJump ()
{
  MSG_DEBUG("character", "HighJump");
  do_nothing_time = Time::GetInstance()->Read();

  if (!CanJump()) return;

  SetRebounding(false);

  jukebox.Play (ActiveTeam().GetSoundProfile(), "superjump");
  SetMovement("jump");

  // Initialise la force
  double angle = Deg2Rad(GameMode::GetInstance()->character.super_jump_angle);
  if (GetDirection() == -1) angle = InverseAngle(angle);
  SetSpeed (GameMode::GetInstance()->character.super_jump_strength, angle);
}

void Character::PrepareShoot()
{
  SetMovementOnce("weapon-" + ActiveTeam().GetWeapon().GetID() + "-begin-shoot");
  if(body->GetMovement() != "weapon-" + ActiveTeam().GetWeapon().GetID() + "-begin-shoot")
  {
    // If a movement is defined for this weapon, just shoot
    DoShoot();
  }
  else
    prepare_shoot = true;
}

void Character::DoShoot()
{
  ActiveTeam().AccessWeapon().StopLoading();
  SetMovementOnce("weapon-" + ActiveTeam().GetWeapon().GetID() + "-end-shoot");
  ActiveTeam().GetWeapon().NewActionShoot();
}

void Character::HandleShoot(int event_type)
{
  switch (event_type) {
    case KEY_PRESSED:
      if (ActiveTeam().GetWeapon().max_strength == 0)
        PrepareShoot();
      else
      if ( (GameLoop::GetInstance()->ReadState() == GameLoop::PLAYING)
         && ActiveTeam().GetWeapon().IsReady() )
        ActiveTeam().AccessWeapon().InitLoading();
      break ;

    case KEY_RELEASED:
      if (ActiveTeam().GetWeapon().IsLoading())
	PrepareShoot();
      break ;

    case KEY_REFRESH:
      if ( ActiveTeam().GetWeapon().IsLoading() )
	{
	  // Strength == max strength -> Fire !!!
	  if (ActiveTeam().GetWeapon().ReadStrength() >=
	      ActiveTeam().GetWeapon().max_strength)
	    PrepareShoot();
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

void Character::HandleKeyEvent(int action, int event_type)
{
  // The character cannot move anymove if the turn is over...
  if (GameLoop::GetInstance()->ReadState() == GameLoop::END_TURN)
    return ;

  if (ActiveCharacter().IsDead())
    return;

  if (action == ACTION_SHOOT)
    {
      HandleShoot(event_type);
      do_nothing_time = Time::GetInstance()->Read();
      CharacterCursor::GetInstance()->Hide();
      return;
    }

  ActionHandler * action_handler = ActionHandler::GetInstance();

  if(action <= ACTION_CHANGE_CHARACTER)
    {
      switch (event_type)
      {
        case KEY_PRESSED:
          switch (action)
          {
            case ACTION_JUMP:
              if(ActiveCharacter().IsReady())
                action_handler->NewAction (new Action(ACTION_JUMP));
	            return ;
            case ACTION_HIGH_JUMP:
              if(ActiveCharacter().IsReady())
                action_handler->NewAction (new Action(ACTION_HIGH_JUMP));
              return ;
            case ACTION_MOVE_LEFT:
            case ACTION_MOVE_RIGHT:
              body->StartWalk();
              break;
            default:
      	      break;
          }
          //no break!! -> it's normal

        case KEY_REFRESH:
          switch (action) {
            case ACTION_MOVE_LEFT:
              if(ActiveCharacter().IsReady())
              {
                if(event_type==KEY_PRESSED)
                  InitMouvementDG(PAUSE_BOUGE);
                MoveCharacterLeft(ActiveCharacter());
              }
              break ;

            case ACTION_MOVE_RIGHT:
              if(ActiveCharacter().IsReady())
              {
                if(event_type==KEY_PRESSED)
                  InitMouvementDG(PAUSE_BOUGE);
                MoveCharacterRight(ActiveCharacter());
              }
              break ;

            case ACTION_UP:
              if(ActiveCharacter().IsReady())
              {
                if (ActiveTeam().crosshair.enable)
                {
                  do_nothing_time = Time::GetInstance()->Read();
                  CharacterCursor::GetInstance()->Hide();
                  action_handler->NewAction (new Action(ACTION_UP));
                }
              }
	      break ;

            case ACTION_DOWN:
              if(ActiveCharacter().IsReady())
              {
                if (ActiveTeam().crosshair.enable)
                {
                  do_nothing_time = Time::GetInstance()->Read();
                  CharacterCursor::GetInstance()->Hide();
                  action_handler->NewAction (new Action(ACTION_DOWN));
                }
              }
	      break ;
            default:
	      break ;
          }
          break;
        case KEY_RELEASED:
          switch (action) {
            case ACTION_MOVE_LEFT:
            case ACTION_MOVE_RIGHT:
               body->StopWalk();
               break;
            }
        default: break;
      }
    }
}

void Character::Refresh()
{
  if (IsGhost()) return;

  UpdatePosition ();
  Time * global_time = Time::GetInstance();

  if( &ActiveCharacter() == this && GameLoop::GetInstance()->ReadState() == GameLoop::PLAYING)
  {
    if(do_nothing_time + do_nothing_timeout < global_time->Read())
      CharacterCursor::GetInstance()->FollowActiveCharacter();
  }

  if(body->IsWalking())
  {
    // Play the step sound only twice during the walking animation
    uint frame_nbr = body->GetFrameCount();
    uint cur = body->GetFrame();
    frame_nbr /= 2;
    cur %= frame_nbr;

    if(cur < frame_nbr / 2 && !step_sound_played)
    {
      step_sound_played = true;
      jukebox.Play (GetTeam().GetSoundProfile(),"step");
    }

    if(cur > frame_nbr / 2)
      step_sound_played = false;
  }
}

// Prepare a new turn
void Character::PrepareTurn ()
{
  HandleMostDamage();
  lost_energy = 0;
  pause_bouge_dg = Time::GetInstance()->Read();
}

const Team& Character::GetTeam() const
{
  return m_team;
}

Team& Character::TeamAccess()
{
  return m_team;
}

bool Character::MouvementDG_Autorise() const
{
  if (!IsReady() || IsFalling()) return false;
  return pause_bouge_dg < Time::GetInstance()->Read();
}

bool Character::CanJump() const
{
	return MouvementDG_Autorise();
}

void Character::InitMouvementDG(uint pause)
{
  do_nothing_time = Time::GetInstance()->Read();
  CharacterCursor::GetInstance()->Hide();
  step_sound_played = true;
  SetRebounding(false);
  pause_bouge_dg = Time::GetInstance()->Read()+pause;
}

bool Character::CanStillMoveDG(uint pause)
{
  if(pause_bouge_dg+pause<Time::GetInstance()->Read())
  {
    pause_bouge_dg += pause;
    return true;
  }
  return false;
}

// Signal the end of a fall
void Character::SignalFallEnding()
{
  // Do not manage dead worms.
  if (IsDead()) return;

  pause_bouge_dg = Time::GetInstance()->Read();

  double norme, degat;
  Point2d speed_vector;
  GameMode * game_mode = GameMode::GetInstance();
  SetMovement("walk");
  SetMovementOnce("soft-land");

  GetSpeedXY (speed_vector);
  norme = speed_vector.Norm();
  if (norme > game_mode->safe_fall && speed_vector.y>0.0)
  {
    norme -= game_mode->safe_fall;
    degat = norme * game_mode->damage_per_fall_unit;
    SetEnergyDelta (-(int)degat);
    GameLoop::GetInstance()->SignalCharacterDamageFalling(this);
    SetMovement("walk");
    SetMovementOnce("hard-land");
  }
}

void Character::SignalExplosion()
{
  if(IsDead()) return;

  double n, a;
  GetSpeed(n, a);
  SetRebounding(true);
  if(n > MIN_SPEED_TO_FLY)
    SetMovement("fly");
  else
  {
    SetClotheOnce("black");
    SetMovementOnce("black");
  }
}

int Character::GetDirection() const 
{ 
  return body->GetDirection();
}

// End of turn or change of character
void Character::StopPlaying()
{
  SetMovement("walk");
  body->ResetWalk();
}

// Begining of turn or changed to this character
void Character::StartPlaying()
{
  assert (!IsGhost());
  SetClothe("weapon-" + m_team.GetWeapon().GetID());
}

uint Character::GetEnergy() const 
{
  assert (!IsDead());
  return energy; 
}

// Hand position
Point2i Character::GetHandPosition() {
  return body->GetHandPosition();
}

// Hand position
void Character::GetHandPositionf (double &x, double &y) 
{
  x = (double) GetHandPosition().x;
  x = (double) GetHandPosition().y;
}

void Character::HandleMostDamage()
{
  if (current_total_damage > max_damage)
  {
    max_damage = current_total_damage;
  }
#ifdef DEBUG_STATS
  std::cerr << m_name << " most damage: " << max_damage << std::endl;
#endif
  current_total_damage = 0;
}

void Character::Hide() { hidden = true; }
void Character::Show() { hidden = false; }

void Character::MadeDamage(const int Dmg, const Character &other)
{
  if (m_team.IsSameAs(other.GetTeam()))
  {
#ifdef DEBUG_STATS
    std::cerr << m_name << " damaged own team with " << Dmg << std::endl;
#endif
    if (Character::IsSameAs(other))
      damage_own_team += Dmg;
  }
  else
  {
#ifdef DEBUG_STATS
    std::cerr << m_name << " damaged other team with " << Dmg << std::endl;
#endif
    damage_other_team += Dmg;
  }
  
  current_total_damage += Dmg;
}

void Character::SetMovement(std::string name)
{
  MSG_DEBUG("body","Character %s -> SetMovement : %s",character_name.c_str(),name.c_str());
  body->SetMovement(name);
  uint l,r,t,b;
  body->GetTestRect(l,r,t,b);
  SetTestRect(l,r,t,b);
}

void Character::SetMovementOnce(std::string name)
{
  MSG_DEBUG("body","Character %s -> SetMovementOnce : %s",character_name.c_str(),name.c_str());
  body->SetMovementOnce(name);
  uint l,r,t,b;
  body->GetTestRect(l,r,t,b);
  SetTestRect(l,r,t,b);
}

void Character::SetClothe(std::string name)
{
  MSG_DEBUG("body","Character %s -> SetClothe : %s",character_name.c_str(),name.c_str());
  body->SetClothe(name);
}

void Character::SetClotheOnce(std::string name)
{
  MSG_DEBUG("body","Character %s -> SetClotheOnce : %s",character_name.c_str(),name.c_str());
  body->SetClotheOnce(name);
}

uint Character::GetTeamIndex()
{
  uint index = 0;
  teams_list.FindPlayingById( GetTeam().GetId(), index);
  return index;
}

uint Character::GetCharacterIndex()
{
  uint index = 0;
  for(Team::iterator it = TeamAccess().begin();
                     it != TeamAccess().end() ; ++it, ++index )
  {
    if( &(*it) == this)
      return index;
  }
  assert(false);
  return 0;
}

