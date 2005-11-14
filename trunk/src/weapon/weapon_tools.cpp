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
 * Utilitaires pour les armes : applique une explosion en un point.
 *****************************************************************************/

#include "../weapon/weapon_tools.h"
//-----------------------------------------------------------------------------
#include "../graphic/video.h"
#include "../interface/interface.h"
#include "../map/camera.h"
#include "../map/map.h"
#include "../object/objects_list.h"
#include "../object/particle.h"
#include "../sound/jukebox.h"
#include "../team/macro.h"
#include "../tool/math_tools.h"

#ifdef CL
#include "../tool/geometry_tools.h"

#else

#endif
//-----------------------------------------------------------------------------

#ifdef DEBUG
//#  define DEBUG_EXPLOSION
#  define COUT_DBG cout << "[Explosion] "
#endif

#ifndef CL
Profile *weapons_res_profile = NULL;
#endif

//-----------------------------------------------------------------------------

#ifdef CL
void AppliqueExplosion (const CL_Point &explosion, 
			const CL_Point &trou, 
			CL_Surface &impact,
			const ExplosiveWeaponConfig &config,
			PhysicalObj *obj_exclu,
			const std::string& son,
			bool fire_particle
			)
#else
void AppliqueExplosion (const Point2i &explosion, 
			const Point2i &trou, 
			SDL_Surface *impact,
			const ExplosiveWeaponConfig &config,
			PhysicalObj *obj_exclu,
			const std::string& son,
			bool fire_particle
			)
#endif
{
  // Make a hole in the ground
#ifdef CL
  monde.Creuse (trou.x - impact.get_width()/2, trou.y-impact.get_height()/2,impact);
#else
  monde.Creuse (trou.x-impact->w/2, trou.y-impact->h/2,impact);   
#endif
   
  // Play a sound
#ifdef CL
  jukebox.Play (son);
#else
  jukebox.Play ("share", son);
#endif
   
  // Apply damage on the worms.
  // Do not care about the death of the active worm.
  POUR_TOUS_VERS(equipe,ver)
  {
    // Is it the object we do not want to manage ?
    if ((obj_exclu != NULL) && (&(*ver) == obj_exclu)) continue;

    double distance, angle;
    distance = MeterDistance (explosion, ver -> GetCenter());
#ifdef DEBUG_EXPLOSION
    COUT_DBG << " ver " << ver -> nom << " : distance=" << distance;
#endif

    // If the worm is in the explosion range, apply damage on it !
    if (distance <= config.explosion_range)
    {
      uint hit_point_loss = (uint)(distance*config.damage/config.explosion_range);
      hit_point_loss = config.damage-hit_point_loss;
#ifdef DEBUG_EXPLOSION
      COUT_DBG << "Hit_Point_Loss énergie=" << hit_point_loss << endl;
#endif
      ver -> SetEnergyDelta (-hit_point_loss);
    }

    // If the worm is in the blast range, apply the blast on it !
    if (distance <= config.blast_range)
    {
      double force = distance;
      if (!EgalZero(distance))
	{
	  force *= config.blast_force / config.blast_range;
	  force  = config.blast_force - force;
	  angle  = CalculeAngle (explosion, ver -> GetCenter());
	}
      else
	{
	  force = config.blast_force;
	  angle = -M_PI/2;
	}
#ifdef DEBUG_EXPLOSION
      cout << ", force=" << force << endl;
#endif
      ver -> AddSpeed (force, angle);
      ver -> UpdatePosition();
    } else {
#ifdef DEBUG_EXPLOSION
      cout << " -> trop loin." << endl;
#endif
    }

    // Update the worm state.
    ver -> Refresh();
#ifdef DEBUG_EXPLOSION
    COUT_DBG << ver -> nom << " fantome " << ver -> IsGhost()
	     << " mort " << ver -> IsDead() 
	     << " actif " << ver -> EstActif() << endl;
#endif
  }

  // Apply the blast on physical objects.
  POUR_CHAQUE_OBJET(obj) if (obj -> ptr -> GetObjectType() == objCLASSIC)
  { 
    // Is this the object we do not want to manage ?
    if ((obj_exclu != NULL) && (obj -> ptr == obj_exclu)) continue;

    double distance, angle;
    distance = MeterDistance (explosion, obj -> ptr -> GetCenter());
    if (distance <= config.blast_range)
    {
      if (!EgalZero(distance)) {
	distance *= config.blast_force / config.blast_range;
	distance  = config.blast_force - distance;
	angle  = CalculeAngle (explosion, obj -> ptr -> GetCenter());
      } else {
	distance = config.blast_force;
	angle = -M_PI/2;
      }
      obj -> ptr -> AddSpeed (distance, angle);
      obj -> ptr -> UpdatePosition();
    }
  }

  // Do we need to generate some fire particles ?
  if (fire_particle) {
#ifdef CL
     global_particle_engine.AddNow ( trou.x - impact.get_width()/2, trou.y-impact.get_height()/2,
				     5, particle_FIRE );
#else
     global_particle_engine.AddNow ( trou.x - impact->w/2, trou.y-impact->h/2,
				     5, particle_FIRE );
#endif
  
  }

}

//-----------------------------------------------------------------------------
