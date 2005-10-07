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
 * Sprite:     Simple sprite management
 ******************************************************************************
 * 2005/09/21: Jean-Christophe Duberga (jcduberga@gmx.de) 
 *             Initial version
 * TODO:       Scale,Rotation...
 *****************************************************************************/

#ifndef _SPRITE_H
#define _SPRITE_H

struct SDL_Surface;
class ResourceManager;

SDL_Surface *newFlippedSurface(SDL_Surface *src, int fliph, int flipv);

class Sprite
{
 public:
   Sprite();
   ~Sprite();

   // Get physical characterisics
   unsigned int GetWidth();
   unsigned int GetHeight();
   unsigned int GetFrameCount();
   
   // Get/Set sprite parameters
   void SetCurrentFrame( unsigned int frame_no);    
   unsigned int GetCurrentFrame();
   void Scale( float scale_x, float scale_y);
   void GetScaleFactors( float &scale_x, float &scale_y);
   void SetAlpha( float alpha); // Can't be combined with per pixel alpha
   float GetAlpha();
   void Start();
   void StartLoop();
   void Finish();
     
   void Blit( SDL_Surface *dest, unsigned int pox_x, unsigned int pos_y);
   void Draw(int pos_x, int pos_y);
   void Update();
   
 private:
   SDL_Surface *surface;
   int frame_width_pix;
   int frame_height_pix;
   int nb_frames;
   float scale_x;
   float scale_y;
   float rotation_deg;
   float alpha;
   int current_frame;
   
friend class ResourceManager;
   
};

#endif /* _SPRITE_H */
