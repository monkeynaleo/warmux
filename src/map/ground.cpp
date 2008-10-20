/******************************************************************************
 *  Wormux is a convivial mass murder game.
 *  Copyright (C) 2001-2008 Wormux Team.
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
 * Terrain de jeu.
 *****************************************************************************/

#include "map/ground.h"
#include <iostream>
#include <SDL_video.h>
#include <SDL_gfxPrimitives.h>
#include <limits.h>
#include <Box2D.h>
#include "map/tileitem.h"
#include "game/game.h"
#include "graphic/sprite.h"
#include "map/camera.h"
#include "map/map.h"
#include "map/maps_list.h"
#include "graphic/surface.h"
#include "graphic/video.h"
#include "graphic/colors.h"
#include "include/app.h"
#include "include/constant.h"
#include "physic/physical_engine.h"
#include "tool/i18n.h"
#include "tool/isnan.h"
#include "tool/resource_manager.h"

// We need only one empty tile
TileItem_Empty EmptyTile;


Ground::Ground() : nbCells(0,0), m_preview(NULL)
{

}

Ground::~Ground()
{
  FreeMem();
}


void Ground::Init()
{
  std::cout << "o " << _("Ground initialization...") << ' ';
  std::cout.flush();

  // Load ground data
  Surface& m_image = ActiveMap()->ReadImgGround();
  if(ActiveMap()->IsOpened()) {
    LoadImage(m_image, ActiveMap()->GetUpperLeftPad(), ActiveMap()->GetLowerRightPad());
  } else {
    LoadImage(m_image, Point2i(), Point2i());
  }
  // Check the size of the map
  ASSERT(Constants::MAP_MIN_SIZE <= GetSize());
  ASSERT(GetSizeX()*GetSizeY() <= Constants::MAP_MAX_SIZE);

  // Check if the map is "opened"
  open = ActiveMap()->IsOpened();

  std::cout << _("done") << std::endl;
}

void Ground::Reset()
{
  Init();
  lastPos.SetValues(INT_MAX, INT_MAX);
}

// Read the alpha channel of the pixel
bool Ground::IsEmpty(const Point2i &pos) const
{
        ASSERT( !GetWorld().IsOutsideWorldXY(pos.x, pos.y) );

        return GetAlpha( pos ) != 255; // IsTransparent
}

/*
 * Returns the angle between the tangent at point (x,y) of the ground and
 * horizontal
 * the angle is always > 0.
 * returns -1.0 if no tangent was found (pixel (x,y) does not touch any
 * other piece of ground
 */
double Ground::Tangent(int x,int y) const
{
  //Approximation : returns the chord instead of the tangent to the ground

  /* We try to find 2 points on the ground on each side of (x,y)
   * the points should be at the limit between land and vaccum
   * (p1 =  point on the left
   * p2 =  point on the right
   */
  Point2i p1,p2;
  if(!PointContigu(x,y, p1.x,p1.y, -1,-1))
    return getNaN();

  if(!PointContigu(x,y, p2.x,p2.y, p1.x,p1.y))
  {
    p2.x = x;
    p2.y = y;
  }
/*
  if(p1.x == p2.x)
    return M_PI / 2.0;
  if(p1.y == p2.y)
    return M_PI;
*/
  //ASSERT (p1.x != p2.x);

  /* double tangeante = atan((double)(p2.y-p1.y)/(double)(p2.x-p1.x));

  while(tangeante <= 0.0)
    tangeante += M_PI;
  while(tangeante > 2 * M_PI)
    tangeante -= M_PI;

  return tangeante; */

  //calculated with a good old TI-83... using table[a][b] = atan( (a-2) / (b-2) )
  const float table[5][5] = {
    {      .78539,       .46364,     M_PI, -.46364+M_PI, -.78539+M_PI},
    {      1.1071,       .78539,     M_PI, -.78539+M_PI, -1.1071+M_PI},
    {    M_PI/2.0,     M_PI/2.0, M_PI/2.0,     M_PI/2.0,   M_PI / 2.0},
    {-1.1071+M_PI, -.78539+M_PI,     M_PI,        78539,       1.1071},
    {-.78539+M_PI, -.46364+M_PI,     M_PI,       .46364,       .78539}};

  ASSERT(p2.x-p1.x >= -2 && p2.x-p1.x <= 2);
  ASSERT(p2.y-p1.y >= -2 && p2.y-p1.y <= 2);

  return table[(p2.y-p1.y)+2][(p2.x-p1.x)+2];
}

bool Ground::PointContigu(int x,int y,  int & p_x,int & p_y,
                           int bad_x,int bad_y) const
{
  //Look for a pixel around (x,y) that is at the edge of the ground
  //and vaccum
  //return true (and set p_x and p_y) if this point have been found
  if(GetWorld().IsOutsideWorld(Point2i(x-1,y))
  || GetWorld().IsOutsideWorld(Point2i(x+1,y))
  || GetWorld().IsOutsideWorld(Point2i(x,y-1))
  || GetWorld().IsOutsideWorld(Point2i(x,y+1)) )
    return false;

  // check adjacents pixels one by one:
  //upper right pixel
  if(x-1 != bad_x
  || y-1 != bad_y)
  if( !IsEmpty(Point2i(x-1,y-1) )
  &&( IsEmpty(Point2i(x-1,y))
  || IsEmpty(Point2i(x,y-1))))
  {
    p_x=x-1;
    p_y=y-1;
    return true;
  }
  //upper pixel
  if(x != bad_x
  || y-1 != bad_y)
  if(!IsEmpty(Point2i(x,y-1))
  &&(IsEmpty(Point2i(x-1,y-1))
  || IsEmpty(Point2i(x+1,y-1))))
  {
    p_x=x;
    p_y=y-1;
    return true;
  }
  //upper right pixel
  if(x+1 != bad_x
  || y-1 != bad_y)
  if(!IsEmpty(Point2i(x+1,y-1))
  &&(IsEmpty(Point2i(x,y-1))
  || IsEmpty(Point2i(x+1,y))))
  {
    p_x=x+1;
    p_y=y-1;
    return true;
  }
  //pixel at the right
  if(x+1 != bad_x
  || y != bad_y)
  if(!IsEmpty(Point2i(x+1,y))
  &&(IsEmpty(Point2i(x+1,y-1))
  || IsEmpty(Point2i(x,y+1))))
  {
    p_x=x+1;
    p_y=y;
    return true;
  }
  //bottom right pixel
  if(x+1 != bad_x
  || y+1 != bad_y)
  if(!IsEmpty(Point2i(x+1,y+1))
  &&(IsEmpty(Point2i(x+1,y))
  || IsEmpty(Point2i(x,y+1))))
  {
    p_x=x+1;
    p_y=y+1;
    return true;
  }
  //bottom pixel
  if(x != bad_x
  || y+1 != bad_y)
  if(!IsEmpty(Point2i(x,y+1))
  &&(IsEmpty(Point2i(x-1,y+1))
  || IsEmpty(Point2i(x+1,y+1))))
  {
    p_x=x;
    p_y=y+1;
    return true;
  }
  //bottom left pixel
  if(x-1 != bad_x
  || y+1 != bad_y)
  if(!IsEmpty(Point2i(x-1,y+1))
  &&(IsEmpty(Point2i(x-1,y))
  || IsEmpty(Point2i(x,y+1))))
  {
    p_x=x-1;
    p_y=y+1;
    return true;
  }
  //pixel at left
  if(x-1 == bad_x
  && y == bad_y)
  if(!IsEmpty(Point2i(x-1,y))
  &&(IsEmpty(Point2i(x-1,y-1))
  || IsEmpty(Point2i(x-1,y+1))))
  {
    p_x=x-1;
    p_y=y;
    return true;
  }
  return false;
}

void Ground::Draw(bool redraw_all)
{
  CheckEmptyTiles();
  AppWormux * app = AppWormux::GetInstance();

  Point2i cPos = Camera::GetInstance()->GetPosition();
  Point2i windowSize = app->video->window.GetSize();
  Point2i margin = (windowSize - GetSize())/2;

  if( Camera::GetInstance()->HasFixedX() ){// ground is less wide than screen !
    app->video->window.BoxColor( Rectanglei(0, 0, margin.x, windowSize.y), black_color);
    app->video->window.BoxColor( Rectanglei(windowSize.x - margin.x, 0, margin.x, windowSize.y), black_color);
  }

  if( Camera::GetInstance()->HasFixedY() ){// ground is less wide than screen !
    app->video->window.BoxColor( Rectanglei(0, 0, windowSize.x, margin.y), black_color);
    app->video->window.BoxColor( Rectanglei(0, windowSize.y - margin.y, windowSize.x, margin.y), black_color);
  }

  if( lastPos != cPos || redraw_all){
    lastPos = cPos;
    DrawTile();
    return;
  }

  RedrawParticleList(*GetWorld().to_redraw_now);

  // Draw on top of sky (redisplayed on top of particles)
  RedrawParticleList(*GetWorld().to_redraw_particles_now);

  // Draw on top of new position of particles (redisplayed on top of particles)
  RedrawParticleList(*GetWorld().to_redraw_particles);

  CheckPreview();
}

void Ground::RedrawParticleList(std::list<Rectanglei> &list) const {
  std::list<Rectanglei>::iterator it;

  for( it = list.begin(); it != list.end(); ++it )
    DrawTile_Clipped(*it);
}

void Ground::FreeMem()
{
  for (int i=0; i < (nbCells.x * nbCells.y); ++i) {
    // Don't delete empty tile as we use only one instance for empty tile
    if(item[i] != &EmptyTile)
      delete item[i];
  }
  nbCells.SetValues(0, 0);
  item.clear();
  if (m_preview)
    delete m_preview;
  m_preview = NULL;
}


void Ground::InitGround(const Point2i &pSize, const Point2i & upper_left_offset, const Point2i & lower_right_offset)
{
  m_upper_left_offset = upper_left_offset;
  m_lower_right_offset = lower_right_offset;
  size = pSize + upper_left_offset + lower_right_offset;
  nbCells = size / CELL_SIZE;

  if((size.x % CELL_SIZE.x) != 0)
    nbCells.x++;

  if((size.y % CELL_SIZE.y) != 0)
    nbCells.y++;
}

void Ground::Dig(const Point2i &position, const Surface& dig)
{
  Point2i firstCell = Clamp(position / CELL_SIZE);
  Point2i lastCell = Clamp((position + dig.GetSize()) / CELL_SIZE);
  Point2i c;
  uint    index = firstCell.y*nbCells.x;
  uint8_t *dst  = m_preview->GetPixels();
  uint    pitch = m_preview->GetPitch();
  dst += firstCell.y*(CELL_SIZE.y>>m_shift)*pitch;

  for( c.y = firstCell.y; c.y <= lastCell.y; c.y++ )
  {
    for( c.x = firstCell.x; c.x <= lastCell.x; c.x++)
    {
      Point2i offset = position - c * CELL_SIZE;

      item[index + c.x]->Dig(offset, dig);

      item[index + c.x]->ScalePreview(dst+4*c.x*(CELL_SIZE.x>>m_shift), pitch, m_shift);
    }
    dst += (CELL_SIZE.y>>m_shift)*pitch;
    index += nbCells.x;
  }
}

void Ground::Dig(const Point2i &center, const uint radius)
{
  Point2i size = Point2i(2 * (radius + EXPLOSION_BORDER_SIZE),
                         2 * (radius + EXPLOSION_BORDER_SIZE));
  Point2i position = center - Point2i(radius + EXPLOSION_BORDER_SIZE,
                                       radius + EXPLOSION_BORDER_SIZE);

  Point2i firstCell = Clamp(position/CELL_SIZE);
  Point2i lastCell = Clamp((position+size)/CELL_SIZE);
  Point2i c;
  uint    index = firstCell.y*nbCells.x;
  uint8_t *dst  = m_preview->GetPixels();
  uint    pitch = m_preview->GetPitch();
  dst += firstCell.y*(CELL_SIZE.y>>m_shift)*pitch;

  for( c.y = firstCell.y; c.y <= lastCell.y; c.y++ )
  {
    for( c.x = firstCell.x; c.x <= lastCell.x; c.x++)
    {
      Point2i offset = center - c * CELL_SIZE;
      item[index + c.x]->Dig(offset, radius);
      item[index + c.x]->ScalePreview(dst+4*c.x*(CELL_SIZE.x>>m_shift), pitch, m_shift);
    }
    dst += (CELL_SIZE.y>>m_shift)*pitch;
    index += nbCells.x;
  }
}

void Ground::MergeSprite(const Point2i &position, Surface& surf)
{
  Point2i firstCell = Clamp(position/CELL_SIZE);
  Point2i lastCell = Clamp((position + surf.GetSize())/CELL_SIZE);
  Point2i c;
  uint8_t *dst = m_preview->GetPixels();
  uint    pitch = m_preview->GetPitch();
  dst += firstCell.y*(CELL_SIZE.y>>m_shift)*pitch;

  for( c.y = firstCell.y; c.y <= lastCell.y; c.y++ ) {

    for( c.x = firstCell.x; c.x <= lastCell.x; c.x++) {

      TileItem *ti = item[c.y*nbCells.x + c.x];
      Point2i spr_offset = position - c * CELL_SIZE;

      if (ti->IsTotallyEmpty()) {
        // Don't delete the old item as we use only one empty tile
        // delete item[c.y*nbCells.x + c.x];
  Point2d tile_offset = c * CELL_SIZE ;

        ti = item[c.y*nbCells.x + c.x] = new TileItem_AlphaSoftware(CELL_SIZE, tile_offset);
        ti->GetSurface().SetAlpha(0,0);
        ti->GetSurface().Fill(0x00000000);
        ti->GetSurface().SetAlpha(SDL_SRCALPHA,0);
      }
      ti->MergeSprite(spr_offset, surf);
      ti->ScalePreview(dst+4*c.x*(CELL_SIZE.x>>m_shift), pitch, m_shift);
    }
    dst += (CELL_SIZE.y>>m_shift)*pitch;
  }
}

// Initialize preview depending on current video and map sizes
void Ground::InitPreview()
{
  Point2i offset     =  m_upper_left_offset + m_lower_right_offset;
  Point2i world_size = size - offset;
  m_last_video_size = GetMainWindow().GetSize();
  m_shift = 0;
  while (world_size > m_last_video_size/4)
  {
    world_size >>= 1;
    m_shift++;
  }
  if (m_preview) delete m_preview;
  m_preview = new Surface();
  *m_preview = Surface(Point2i(nbCells.x*(CELL_SIZE.x>>m_shift), nbCells.y*(CELL_SIZE.y>>m_shift)),
                       SDL_SWSURFACE|SDL_SRCALPHA, true).DisplayFormatAlpha();
  m_preview->SetAlpha(SDL_SRCALPHA, 0);

  m_preview_size = m_preview->GetSize() - (offset / (1<<m_shift));
  m_preview_rect = Rectanglei(m_upper_left_offset / (1<<m_shift), m_preview_size);
}

// Rerender all of the preview
void Ground::CheckPreview()
{
  if (GetMainWindow().GetSize() == m_last_video_size)
    return;

  InitPreview();
  uint8_t *dst  = m_preview->GetPixels();
  uint    pitch = m_preview->GetPitch();

  // Fill the TileItem objects
  Point2i i;
  int     piece = 0;
  for( i.y = 0; i.y < nbCells.y; i.y++)
  {
    for( i.x = 0; i.x < nbCells.x; i.x++, piece++ )
      item[piece]->ScalePreview(dst+4*i.x*(CELL_SIZE.x>>m_shift), pitch, m_shift);

    dst += (CELL_SIZE.y>>m_shift)*pitch;
  }
}

void Ground::LoadImage(Surface& ground_img, const Point2i & upper_left_offset, const Point2i & lower_right_offset)
{
  Point2i offset = upper_left_offset + lower_right_offset;
  FreeMem();
  InitGround(ground_img.GetSize(), upper_left_offset, lower_right_offset);
  ASSERT(nbCells.x != 0 && nbCells.y != 0);

  //m_tile_body->SetXForm(b2Vec2(upper_left_offset.x/PIXEL_PER_METER, upper_left_offset.y/PIXEL_PER_METER), m_tile_body->GetAngle());

  InitPreview();
  uint8_t *dst  = m_preview->GetPixels();
  uint    pitch = m_preview->GetPitch();

  // Create and fill the TileItem objects
  Point2i i;
  for( i.y = 0; i.y < nbCells.y; i.y++ )
  {
    for( i.x = 0; i.x < nbCells.x; i.x++ )
    {
      Rectanglei sr(i * CELL_SIZE - upper_left_offset, CELL_SIZE);
      Point2d offset = i* CELL_SIZE;

      TileItem_AlphaSoftware* t = new TileItem_AlphaSoftware(CELL_SIZE, offset);
      ground_img.SetAlpha(0, 0);
      t->GetSurface().Blit(ground_img, sr, Point2i(0, 0));
      t->ScalePreview(dst+4*i.x*(CELL_SIZE.x>>m_shift), pitch, m_shift);

      while (t->need_check_empty)
  t->CheckEmpty();

      if (t->NeedDelete()) {
#ifdef DBG_TILE
  printf("\nDeleting tile %i",i);
#endif
  delete t;
  item.push_back((TileItem*)&EmptyTile);
      } else {
#ifdef DBG_TILE
  if (i % nbCells.x % 2 == (i / nbCells.x) % 2)
    item[i]->FillWithRGB(0, 0, 255);
  else
    item[i]->FillWithRGB(0, 255, 0);
#endif
  t->InitShape();
  item.push_back(t);
      }
    }
    dst += (CELL_SIZE.y>>m_shift)*pitch;
  }
}

uchar Ground::GetAlpha(const Point2i &pos) const
{
  int cell = pos.y / CELL_SIZE.y * nbCells.x + pos.x / CELL_SIZE.x;
  return item[cell]->GetAlpha(pos % CELL_SIZE);
}

void Ground::DrawTile()
{
  Point2i firstCell = Clamp(Camera::GetInstance()->GetPosition() / CELL_SIZE);
  Point2i lastCell = Clamp((Camera::GetInstance()->GetPosition() + Camera::GetInstance()->GetSize()) / CELL_SIZE);
  Point2i i;
  for( i.y = firstCell.y; i.y <= lastCell.y; i.y++ )
    for( i.x = firstCell.x; i.x <= lastCell.x; i.x++)
      item[i.y*nbCells.x + i.x]->Draw( i );

#ifdef DEBUG
  if (IsLOGGING("map")) {
    for ( i.x = firstCell.x; i.x <= lastCell.x; i.x++)
      GetMainWindow().LineColor(i.x*CELL_SIZE.x - Camera::GetInstance()->GetPosition().x,
        i.x*CELL_SIZE.x - Camera::GetInstance()->GetPosition().x,
        0, GetMainWindow().GetHeight(),
        primary_green_color);

    for ( i.y = firstCell.y; i.y <= lastCell.y; i.y++)
      GetMainWindow().LineColor(0, GetMainWindow().GetWidth(),
        i.y*CELL_SIZE.y - Camera::GetInstance()->GetPosition().y,
        i.y*CELL_SIZE.y - Camera::GetInstance()->GetPosition().y,
        primary_green_color);
  }


#endif
}

void Ground::DrawTile_Clipped(Rectanglei worldClip) const
{
  // Revision 514:
  // worldClip.SetSize( worldClip.GetSize() + 1); // mmm, does anything gives areas
  // too small to redraw ?
  //
  // Revision 3095:
  // Sorry, I don't understand that comment. Moreover the +1 produces a bug when the ground of
  // a map have an alpha value != 255 and != 0
  worldClip.SetSize( worldClip.GetSize());
  Point2i firstCell = Clamp(worldClip.GetPosition() / CELL_SIZE);
  Point2i lastCell  = Clamp((worldClip.GetBottomRightPoint()) / CELL_SIZE);
  Point2i c;

  for( c.y = firstCell.y; c.y <= lastCell.y; c.y++ )
    for( c.x = firstCell.x; c.x <= lastCell.x; c.x++){
      // For all selected items, clip source and destination blitting rectangles
      Rectanglei destRect(c * CELL_SIZE, CELL_SIZE);

      destRect.Clip(worldClip);
      if( destRect.Intersect( *Camera::GetInstance() ) ){
        Point2i ptDest = destRect.GetPosition() - Camera::GetInstance()->GetPosition();
        Point2i ptSrc = destRect.GetPosition() - c * CELL_SIZE;

        GetMainWindow().Blit( item[c.y*nbCells.x + c.x]->GetSurface(), Rectanglei(ptSrc, destRect.GetSize()) , ptDest);
      }
    }
}

Surface Ground::GetPart(const Rectanglei& rec)
{
  Surface part(rec.GetSize(), SDL_SWSURFACE|SDL_SRCALPHA, true);
  part.SetAlpha(0,0);
  part.Fill(0x00000000);
  part.SetAlpha(SDL_SRCALPHA,0);

  Point2i firstCell = Clamp(rec.GetPosition() / CELL_SIZE);
  Point2i lastCell = Clamp((rec.GetPosition() + rec.GetSize()) / CELL_SIZE);
  Point2i i = nbCells - 1;

  for( i.y = firstCell.y; i.y <= lastCell.y; i.y++ )
  for( i.x = firstCell.x; i.x <= lastCell.x; i.x++ )
  {
    if(item[i.y*nbCells.x + i.x]->IsTotallyEmpty()) continue;

    Point2i cell_pos = i * CELL_SIZE;
    Rectanglei src;
    Point2i dst;
    src.SetPosition( rec.GetPosition() - cell_pos );
    if(src.GetPositionX() < 0) src.SetPositionX(0);
    if(src.GetPositionY() < 0) src.SetPositionY(0);

    src.SetSize( rec.GetPosition() + rec.GetSize() - cell_pos - src.GetPosition());
    if(src.GetSizeX() + src.GetPositionX() > CELL_SIZE.x) src.SetSizeX(CELL_SIZE.x - src.GetPositionX());
    if(src.GetSizeY() + src.GetPositionY() > CELL_SIZE.y) src.SetSizeY(CELL_SIZE.y - src.GetPositionY());

    dst =  cell_pos - rec.GetPosition();
    if(dst.x < 0) dst.x = 0;
    if(dst.y < 0) dst.y = 0;

    item[i.y*nbCells.x + i.x]->GetSurface().SetAlpha(0, 0);
    part.Blit(item[i.y*nbCells.x + i.x]->GetSurface(), src, dst);
    item[i.y*nbCells.x + i.x]->GetSurface().SetAlpha(SDL_SRCALPHA, 0);
  }
  return part;
}

void Ground::CheckEmptyTiles()
{
  for (int i=0; i < (nbCells.x * nbCells.y); ++i) {

    if (item[i]->IsTotallyEmpty()) continue;

    TileItem_AlphaSoftware* t = static_cast<TileItem_AlphaSoftware*>(item[i]);
    if (t->need_check_empty)
      t->CheckEmpty();

    if (t->need_delete) {
      // no need to display this tile as it can be deleted!
#ifdef DBG_TILE
      printf("Deleting tile %i\n",i);
#endif
      delete item[i];

      // Don't instanciate a new empty tile but use the already existing one
      item[i] = (TileItem*)&EmptyTile;
    }
  }
}

