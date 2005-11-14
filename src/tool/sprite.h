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

#include <vector>

struct SDL_Surface;
class ResourceManager;

extern SDL_Surface *newFlippedSurface(SDL_Surface *src, int fliph, int flipv);

enum Rotation_HotSpot {top_left, top_center, top_right, left_center, center, right_center, bottom_left, bottom_center, bottom_right};

 
class SpriteFrame
{
public:
  SpriteFrame(SDL_Surface *surface, unsigned int speed=100);
  SDL_Surface *surface;
  unsigned int speed; // in millisecond
};

class Sprite
{
  public:
typedef enum {
  show_last_frame,
  show_blank
} SpriteShowOnFinish;
	
 public:
   Sprite();
   Sprite( const Sprite &other);
   Sprite( SDL_Surface *surface);
   ~Sprite();
   void Init( SDL_Surface *surface, int frame_width, int frame_height, int nb_frames_x, int nb_frames_y);
   
   // Get physical characterisics
   unsigned int GetWidth();
   unsigned int GetHeight();
   unsigned int GetFrameCount();
   
   // Get/Set sprite parameters
   void SetCurrentFrame( unsigned int frame_no);    
   unsigned int GetCurrentFrame() const;
   const SpriteFrame& GetCurrentFrameObject() const;
   void Scale( float scale_x, float scale_y);
   void GetScaleFactors( float &scale_x, float &scale_y);
   void ScaleSize(int width, int height);
   void SetRotation_deg( float angle_deg);
   void SetRotation_HotSpot( Rotation_HotSpot rhs) {rot_hotspot = rhs;};
   void SetAlpha( float alpha); // Can't be combined with per pixel alpha
   float GetAlpha();
   void Start();
   void StartLoop();
   void Finish();
	 void SetPlayBackward(bool enable);
   void Show();
   void Hide();
   void SetShowOnFinish(SpriteShowOnFinish show);
     
   void Blit( SDL_Surface *dest, unsigned int pox_x, unsigned int pos_y);
   void Draw(int pos_x, int pos_y);
   void Update();
   bool IsFinished() const;

 private:
   uint last_update;
   bool finished;
   bool show;
   bool loop;
   SpriteShowOnFinish show_on_finish;
   int frame_width_pix;
   int frame_height_pix;
   float scale_x;
   float scale_y;
   float rotation_deg;
   float alpha;
   unsigned int current_frame;
   int frame_delta; // Used in Update() to get next frame
   bool backward;
   std::vector<SpriteFrame> frames;
   Rotation_HotSpot rot_hotspot;
   void Calculate_Rotation_Offset(int & rot_x, int & rot_y, SDL_Surface* tmp_surface);
};

#endif /* _SPRITE_H */
