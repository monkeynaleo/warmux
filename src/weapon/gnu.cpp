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
 * Weapon gnu : a gnu jump in (more or less) random directions and explodes
 *****************************************************************************/

#include "../weapon/gnu.h"
//-----------------------------------------------------------------------------
#include <sstream>
#include "../game/config.h"
#include "../game/time.h"
#include "../team/teams_list.h"
#include "../interface/interface.h"
#include "../graphic/video.h"
#include "../tool/math_tools.h"
#include "../game/game_loop.h"
#include "../tool/i18n.h"
#include "../map/camera.h"
#include "../weapon/weapon_tools.h"
#include "../interface/game_msg.h"
#include "../object/objects_list.h"
#include "../sound/jukebox.h"
#include "../tool/random.h"
//-----------------------------------------------------------------------------
GnuLauncher gnu_launcher;
//-----------------------------------------------------------------------------
using namespace Wormux;
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

Gnu::Gnu() : PhysicalObj ("Gnu!")
{
  m_allow_negative_y = true;
}

//-----------------------------------------------------------------------------

void Gnu::Tire (double force)
{
  SetAirResistFactor(gnu_launcher.cfg().air_resist_factor);

  // Set the initial position.
  int x,y;

  ActiveTeam().GetWeapon().PosXY (x, y);
  SetXY (x,y);

  // Set the initial speed.
  double angle = ActiveTeam().crosshair.GetAngleRad();
  SetSpeed (force, angle);
  is_active=true;

  PutOutOfGround(angle);

  //Dummy value, we only need save_x!=x and save_y!=y
  //To avoid a comparison in Refresh()
  save_x=(double)GetX()-1.0;
  save_y=(double)GetY()-1.0;

  launched_time=global_time.Read();

  if(angle<M_PI/2 && angle>-M_PI/2)
    m_sens = 1;
  else
    m_sens = -1;
}

//-----------------------------------------------------------------------------

void Gnu::Init()
{
  image = resource_manager.LoadSprite( weapons_res_profile, "gnu"); 
  SetSize (image->GetWidth(), image->GetHeight());
  SetMass (gnu_launcher.cfg().mass);
}

//-----------------------------------------------------------------------------

void Gnu::Refresh()
{
  if (!is_active) return;

  uint tmp=global_time.Read();
  if(tmp > (1000*gnu_launcher.cfg().timeout)+launched_time)
  {
    gnu_launcher.Explosion();
    return;
  }

  double angle,norm;
  GetSpeed(norm,angle);
  if((!IsMoving() || norm<1.0)&& !FootsInVacuum())
  {
    norm = RandomLong(50,100)*double(gnu_launcher.cfg().rebound_factor)/100.0;
    angle = -(M_PI/2)-(m_sens*(RandomLong(0,90)*M_PI/45.0));
    SetSpeed(norm,angle);
  }


  angle *= 180.0/M_PI;

//  printf("\n%f",angle);

  if(angle<-90 || angle>90)
  {
    m_sens=-1;
    angle=-angle + 180;
  }
  else
    m_sens=1;

//  if(angle<-45 && angle>-135)
//    angle=0;

  image->SetRotation_deg(angle);
  image->Update();
  // Fixe le rectangle de test
  SetTestRect (2, 2, 2, 2);

  if(IsGhost())
  {
    is_active=false;
    gnu_launcher.Explosion();
    return;
  }
}

//-----------------------------------------------------------------------------

void Gnu::SignalFallEnding()
{
}

//-----------------------------------------------------------------------------
void Gnu::SignalCollision()
{
  is_active=false;
  gnu_launcher.Explosion();
}
void Gnu::SignalGhostState()
{
  is_active=false;
  gnu_launcher.Explosion();
}

//-----------------------------------------------------------------------------

void Gnu::Draw()
{
  if (!is_active) return;

#ifdef CL
  image.set_scale(m_sens,1.0);
  if(m_sens==1)
    image.draw(GetX(),GetY());
  else
    image.draw(GetX()+image.get_width(),GetY());
#else
  image->Scale(m_sens,1.0);
//  if(m_sens==1)
    image->Draw(GetX(),GetY());
//  else
//    image->Draw(GetX()+image->GetWidth(),GetY());   
#endif

  int tmp=gnu_launcher.cfg().timeout;
  tmp -= (int) ((global_time.Read() - launched_time) / 1000);
  std::ostringstream ss;
  ss << tmp;

  int txt_x = GetX() + GetWidth() / 2;
  int txt_y = GetY() - GetHeight();
  small_font.WriteCenterTop (txt_x-camera.GetX(), txt_y-camera.GetY(), ss.str(), white_color);

}

//-----------------------------------------------------------------------------

void Gnu::Reset()
{
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

GnuLauncher::GnuLauncher() : Weapon(WEAPON_GNU, "gnulauncher")
{
  m_name = _("GnuLauncher");
  extra_params = new GrenadeConfig();
}

//-----------------------------------------------------------------------------

bool GnuLauncher::p_Shoot ()
{
  ActiveCharacter().previous_strength = m_strength;
  if (m_strength == max_strength)
    m_strength = 0;

  // Initialise le gnu
  gnu.Tire (m_strength);
  lst_objets.AjouteObjet (&gnu, true);
  camera.ChangeObjSuivi (&gnu,true,true,true);
  m_strength = 0;

  jukebox.Play(ActiveTeam().GetSoundProfile(), "fire");

  return true;
}

//-----------------------------------------------------------------------------

void GnuLauncher::Explosion()
{
  m_is_active = false;
  
  lst_objets.RetireObjet (&gnu);

  // On fait un trou ?
  if (gnu.IsGhost()||!gnu.is_active) return;

  // Applique les degats et le souffle aux vers
  Point2i pos = gnu.GetCenter();
  AppliqueExplosion (pos, pos, impact, cfg(), NULL);
}

//-----------------------------------------------------------------------------

void GnuLauncher::Refresh()
{
  if (!m_is_active) return;
}

//-----------------------------------------------------------------------------

void GnuLauncher::p_Init()
{
  impact = resource_manager.LoadImage( weapons_res_profile, "gnulauncher_impact");
  gnu.Init();
}

//-----------------------------------------------------------------------------

GrenadeConfig& GnuLauncher::cfg()
{ return static_cast<GrenadeConfig&>(*extra_params); }

//-----------------------------------------------------------------------------
