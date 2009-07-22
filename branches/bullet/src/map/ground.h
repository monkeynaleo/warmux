/******************************************************************************
 *  Wormux is a convivial mass murder game.
 *  Copyright (C) 2001-2009 Wormux Team.
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
 * Ground
 *****************************************************************************/

#ifndef GROUND_H
#define GROUND_H

#include <list>


#include <vector>
#include <WORMUX_point.h>
#include <WORMUX_rectangle.h>

// Forward declarations
class Surface;
class Sprite;
class TileItem;
class TileItem_Empty; 
class PhysicalObj;

const uint EXPLOSION_BORDER_SIZE = 10;


class Ground : public Rectanglei
{
private:
  bool open;
  bool PointContigu(int x,int y, int & p_x,int & p_y,
                    int pas_bon_x,int pas_bon_y) const;
  Point2i lastPos;

  static float table[5][5];

public:
  Ground();
  ~Ground ();
  void Init();
  void Draw(bool redraw_all);
  void Reset();
  void Free() { FreeMem(); }

  // Is point (x,y) in vacuum ?
  bool IsEmpty(const Point2i &pos) const;

  // Is the playground "open" ?
  bool IsOpen() const { return open; }

  //returns the angle formed by the ground tangent au terrain
  double Tangent(int x,int y) const ;

  void RedrawParticleList(std::list<Rectanglei> &list) const;
  
  
  // Dig a hole
  void Dig(const Point2i &position, const Surface& provider);
  // Dig a circle hole
  void Dig(const Point2i &center, const uint radius);

  // Insert a sprite into the ground
  void PutSprite(const Point2i& pos, const Sprite* spr);
  // Merge a sprite into map (using alpha information)
  void MergeSprite(const Point2i &position, Surface & provider);

  // Load an image
  void LoadImage(Surface& ground_surface, const Point2i & upper_left_offset, const Point2i & lower_right_offset);

  // Get alpha value of a pixel
  unsigned char GetAlpha(const Point2i &pos) const;

  // Draw it (on the entire visible part)
  void DrawTile();

  // Draw a part that is inside the given clipping rectangle
  // Clipping rectangle is in World corrdinate not screen coordinates
  // usefull to redraw only a part that is under a sprite that has moved,...
  void DrawTile_Clipped(Rectanglei clip_rectangle) const;

  // Return a surface of the ground inside the rect
  Surface GetPart(const Rectanglei& rec);

  // Return the preview
  const Surface* GetPreview() const { return m_preview; };
  void  CheckPreview();
  const Point2i& GetPreviewSize() const { return m_preview_size; };
  const Rectanglei& GetPreviewRect() const { return m_preview_rect; };
  uint GetLastPreviewRedrawTime() const { return m_last_preview_redraw; };
  
  // Translate world coordinates into a preview ones
  // @warning assumes CELL_SIZE is 64x64
  Point2i PreviewCoordinates(const Point2i& pos) { return (pos-m_upper_left_offset)>>m_shift; };

  // Check if a title is empty, so we can delete it
  void CheckEmptyTiles();
  
  protected:
    void InitGround(const Point2i &pSize, const Point2i & upper_left_offset, const Point2i & lower_right_offset);

  void FreeMem();
  Point2i Clamp(const Point2i &v) const { return v.clamp(Point2i(0, 0), nbCells - 1); };

  // Ground dimensions
  Point2i nbCells;
  PhysicalObj *GetPhysicalObj() const ;

  void InitPreview();
  Surface*   m_preview;
  uint       m_last_preview_redraw;
  uint       m_shift;
  Point2i    m_last_video_size;
  Point2i    m_preview_size;
  Rectanglei m_preview_rect;

  Point2i m_upper_left_offset;
  Point2i m_lower_right_offset;
  PhysicalObj* m_physical_obj;

  // Canvas giving access to tiles
  std::vector<TileItem *> item;
  
};

#endif
