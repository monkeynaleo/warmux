/******************************************************************************
 *  Wormux is a convivial mass murder game.
 *  Copyright (C) 2001-2010 Wormux Team.
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
 *****************************************************************************/

#include <SDL.h>
#include <png.h>
#include "map/tile.h"
#include "map/tileitem.h"
#include "game/game.h"
#include "game/time.h"
#include "graphic/surface.h"
#include "graphic/sprite.h"
#include "graphic/video.h"
#include "include/app.h"
#include "map/camera.h"

// We need only one empty tile
TileItem_Empty EmptyTile;

Tile::Tile() :
  nbCells(Point2i(0, 0)),
  m_preview(NULL),
  m_last_preview_redraw(0)
{
}

void Tile::FreeMem()
{
  for (int i = 0; i < nbCells.x * nbCells.y; i++) {
    // Don't delete empty tile as we use only one instance for empty tile
    if(item[i] != &EmptyTile)
      delete item[i];
  }
  nbCells.x = nbCells.y = 0;
  item.clear();
  if (m_preview)
    delete m_preview;
  m_preview = NULL;

  crc = 0;
}

Tile::~Tile()
{
  FreeMem();
}

void Tile::InitTile(const Point2i &pSize, const Point2i & upper_left_offset, const Point2i & lower_right_offset)
{
  m_upper_left_offset = upper_left_offset;
  m_lower_right_offset = lower_right_offset;
  startCell = upper_left_offset / CELL_SIZE;

  size = pSize + upper_left_offset;
  endCell = size / CELL_SIZE;
  if (size.x % CELL_SIZE.x)
    endCell.x++;
  if (size.y % CELL_SIZE.y)
    endCell.y++;

  size += lower_right_offset;
  nbCells = size / CELL_SIZE;
  if (size.x % CELL_SIZE.x)
    nbCells.x++;
  if (size.y % CELL_SIZE.y)
    nbCells.y++;
}

void Tile::Dig(const Point2i &position, const Surface& dig)
{
  Point2i  firstCell = Clamp(position / CELL_SIZE);
  Point2i  lastCell  = Clamp((position + dig.GetSize()) / CELL_SIZE);
  uint     index     = firstCell.y*nbCells.x;
  uint8_t *dst       = m_preview->GetPixels();
  int      pitch     = m_preview->GetPitch();
  Point2i c;

  dst += (firstCell.y-startCell.y)*(CELL_SIZE.y>>m_shift)*pitch;

  for (c.y = firstCell.y; c.y <= lastCell.y; c.y++) {
    for(c.x = firstCell.x; c.x <= lastCell.x; c.x++) {
      TileItem *ti = item[index + c.x];
      if (!ti->IsTotallyEmpty()) {
        TileItem_NonEmpty *tin = static_cast<TileItem_NonEmpty *>(ti);
        tin->Dig(position - c * CELL_SIZE, dig);
        tin->ScalePreview(dst, c.x-startCell.x, pitch, m_shift);
      }
    }
    dst   += (CELL_SIZE.y>>m_shift)*pitch;
    index += nbCells.x;
  }

  m_last_preview_redraw = Time::GetInstance()->Read();
}

void Tile::Dig(const Point2i &center, const uint radius)
{
  Point2i size = Point2i(2 * (radius + EXPLOSION_BORDER_SIZE),
                         2 * (radius + EXPLOSION_BORDER_SIZE));
  Point2i position  = center - Point2i(radius + EXPLOSION_BORDER_SIZE,
                                       radius + EXPLOSION_BORDER_SIZE);

  Point2i  firstCell = Clamp(position/CELL_SIZE);
  Point2i  lastCell  = Clamp((position+size)/CELL_SIZE);
  uint     index     = firstCell.y*nbCells.x;
  uint8_t *dst       = m_preview->GetPixels();
  int      pitch     = m_preview->GetPitch();
  Point2i  c;

  dst += (firstCell.y-startCell.y)*(CELL_SIZE.y>>m_shift)*pitch;

  for (c.y = firstCell.y; c.y <= lastCell.y; c.y++) {
    for (c.x = firstCell.x; c.x <= lastCell.x; c.x++) {
      TileItem *ti = item[index + c.x];
      if (!ti->IsTotallyEmpty()) {
        TileItem_NonEmpty *tin = static_cast<TileItem_NonEmpty *>(ti);

        tin->Dig(center - c * CELL_SIZE, radius);
        tin->ScalePreview(dst, c.x-startCell.x, pitch, m_shift);
      }
    }
    dst   += (CELL_SIZE.y>>m_shift)*pitch;
    index += nbCells.x;
  }

  m_last_preview_redraw = Time::GetInstance()->Read();
}

TileItem_NonEmpty* Tile::GetNonEmpty(uint x, uint y)
{
  TileItem          *ti  = item[y*nbCells.x + x];
  TileItem_NonEmpty *tin = NULL;

  if (ti->IsTotallyEmpty()) {
    // Delete the TileItem_Empty object
    delete item[y*nbCells.x + x];
    tin = TileItem_AlphaSoftware::NewEmpty();
    item[y*nbCells.x +x] = tin;
  } else {
    tin = static_cast<TileItem_NonEmpty*>(ti);
  }

  return tin;
}

void Tile::PutSprite(const Point2i& pos, const Sprite* spr)
{
  Rectanglei rec       = Rectanglei(pos, spr->GetSizeMax());
  Point2i    firstCell = Clamp(pos/CELL_SIZE);
  Point2i    lastCell  = Clamp((pos + spr->GetSizeMax())/CELL_SIZE);
  Surface    s         = spr->GetSurface();
  uint8_t   *pdst      = m_preview->GetPixels();
  int        pitch     = m_preview->GetPitch();
  Point2i c;

  s.SetAlpha(0, 0);
  pdst += (firstCell.y-startCell.y)*(CELL_SIZE.y>>m_shift)*pitch;

  for (c.y = firstCell.y; c.y <= lastCell.y; c.y++) {
    for (c.x = firstCell.x; c.x <= lastCell.x; c.x++) {
      Point2i cell_pos = c * CELL_SIZE;
      Rectanglei src;
      Rectanglei dst;
      src.SetPosition(rec.GetPosition() - cell_pos);
      if (src.GetPositionX() < 0)
        src.SetPositionX(0);
      if (src.GetPositionY() < 0)
        src.SetPositionY(0);

      src.SetSize(rec.GetPosition() + rec.GetSize() - cell_pos - src.GetPosition());
      if (src.GetSizeX() + src.GetPositionX() > CELL_SIZE.x)
        src.SetSizeX(CELL_SIZE.x - src.GetPositionX());
      if (src.GetSizeY() + src.GetPositionY() > CELL_SIZE.y)
        src.SetSizeY(CELL_SIZE.y - src.GetPositionY());

      dst.SetPosition(cell_pos - rec.GetPosition());
      if (dst.GetPositionX() < 0)
        dst.SetPositionX(0);
      if (dst.GetPositionY() < 0)
        dst.SetPositionY(0);
      dst.SetSize(src.GetSize());

      TileItem_NonEmpty *tin = GetNonEmpty(c.x, c.y);
      tin->GetSurface().Blit(s, dst, src.GetPosition());
      tin->ResetEmptyCheck();
      tin->ScalePreview(pdst, c.x-startCell.x, pitch, m_shift);
    }
    pdst += (CELL_SIZE.y>>m_shift)*pitch;
  }

  s.SetAlpha(SDL_SRCALPHA, 0);

  m_last_preview_redraw = Time::GetInstance()->Read();
}

void Tile::MergeSprite(const Point2i &position, Surface& surf)
{
  Point2i  firstCell = Clamp(position/CELL_SIZE);
  Point2i  lastCell  = Clamp((position + surf.GetSize())/CELL_SIZE);
  uint8_t *dst       = m_preview->GetPixels();
  int      pitch     = m_preview->GetPitch();
  Point2i  c;

  dst += (firstCell.y-startCell.y)*(CELL_SIZE.y>>m_shift)*pitch;

  for (c.y = firstCell.y; c.y <= lastCell.y; c.y++) {
    for (c.x = firstCell.x; c.x <= lastCell.x; c.x++) {

      Point2i offset = position - c * CELL_SIZE;
      TileItem_NonEmpty *tin    = GetNonEmpty(c.x, c.y);

      tin->MergeSprite(offset, surf);
      tin->ScalePreview(dst, c.x-startCell.x, pitch, m_shift);
    }
    dst += (CELL_SIZE.y>>m_shift)*pitch;
  }

  m_last_preview_redraw = Time::GetInstance()->Read();
}

// Initialize preview depending on current video and map sizes
void Tile::InitPreview()
{
  Point2i world_size = (endCell+1-startCell)*CELL_SIZE;

  m_last_video_size  = GetMainWindow().GetSize();
  m_shift = 0;

  // Task 6730: biggest dimension won't be bigger than 2/5,
  // often less in fact
  while (5*world_size.x>2*m_last_video_size.x ||
         5*world_size.y>2*m_last_video_size.y) {
    world_size >>= 1;
    m_shift++;
  }

  m_preview = new Surface;
  m_preview->NewSurface(world_size, SDL_SWSURFACE|SDL_SRCALPHA, true);
  m_preview->SetAlpha(SDL_SRCALPHA, 0);

  // Actual preview size from pixel-wise information
  m_preview_size = (size - m_upper_left_offset - m_lower_right_offset)>>m_shift;
  // Offset is the difference between pixel-wise info and cell-wise one
  m_preview_rect = Rectanglei((m_upper_left_offset % CELL_SIZE)>>m_shift,
                              m_preview_size);

  m_last_preview_redraw = Time::GetInstance()->Read();
}

// Rerender all of the preview
void Tile::CheckPreview()
{
  if (m_last_preview_redraw == 0) {
    m_last_preview_redraw = Time::GetInstance()->Read();
  }

  if (GetMainWindow().GetSize() == m_last_video_size)
    return;

  InitPreview();
  uint8_t *dst   = m_preview->GetPixels();
  int      pitch = m_preview->GetPitch();

  // Fill the TileItem objects
  Point2i i;
  uint    offset = startCell.y*nbCells.x;

  for (i.y = startCell.y; i.y < endCell.y; i.y++) {
    for (i.x =startCell.x; i.x < endCell.x; i.x++) {
      item[i.x + offset]->ScalePreview(dst, i.x-startCell.x, pitch, m_shift);
    }
    dst    += (CELL_SIZE.y>>m_shift)*pitch;
    offset += nbCells.x;
  }
}

static uint32_t read_png_rows(png_structp png_ptr,
                              uint8_t *buffer, uint height, int stride,
                              uint32_t crc, uint width)
{
  while (height--) {
    uint i;
    png_read_row(png_ptr, buffer, NULL);
    for (i=0; i<4*width; i++)
      crc += buffer[i+0] + buffer[i+1] + buffer[i+2] + buffer[i+3];
    buffer += stride;
    crc = crc % 429496000;
  }
  return crc;
}

bool Tile::LoadImage(const std::string& filename,
                     uint alpha_threshold,
                     const Point2i & upper_left_offset,
                     const Point2i & lower_right_offset)
{
  FILE        *f        = NULL;
  png_structp  png_ptr  = NULL;
  png_infop    info_ptr = NULL;
  bool         ret      = false;
  uint8_t     *buffer   = NULL;
  int          bpp      = SDL_GetVideoInfo()->vfmt->BytesPerPixel;
  int          stride;
  int          offsetx, offsety, endoffy;
  Point2i      i;
  uint8_t     *dst;
  int          width, height, pitch, color_type, depth, has_tRNS;

  // Opening the existing file
  f = fopen(filename.c_str(), "rb");
  if(f == NULL)
    goto end;

  // Creating a png ...
  png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if(png_ptr == NULL) // Structure and ...
    goto end;
  info_ptr = png_create_info_struct(png_ptr);
  if(info_ptr == NULL) // Information.
    goto end;

  // Associate png struture with a file
  png_init_io(png_ptr, f);
  png_read_info(png_ptr, info_ptr);
  width  = png_get_image_width(png_ptr, info_ptr);
  height = png_get_image_height(png_ptr, info_ptr);
  color_type = png_get_color_type(png_ptr, info_ptr);
  depth = png_get_bit_depth(png_ptr, info_ptr);

  // expand paletted colors into true RGB triplets
  has_tRNS = png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS);
  if (color_type == PNG_COLOR_TYPE_PALETTE || has_tRNS
      || (color_type == PNG_COLOR_TYPE_GRAY && depth < 8))
    png_set_expand(png_ptr);
  if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
    png_set_gray_to_rgb(png_ptr) ;

  // tell libpng to strip 16 bit/color files down to 8 bits/color
  if (depth == 16)
    png_set_strip_16(png_ptr);

  // Find the proper mapping of channels
  png_set_bgr(png_ptr);

  FreeMem();
  crc = 0;
  InitTile(Point2i(width, height), upper_left_offset, lower_right_offset);

  stride  = (endCell.x+1 - startCell.x)*CELL_SIZE.x*4;
  buffer  = new uint8_t[CELL_SIZE.y*stride];
  offsetx = (upper_left_offset.x % CELL_SIZE.x)*4;
  offsety = upper_left_offset.y % CELL_SIZE.y;
  endoffy = (height + upper_left_offset.y) % CELL_SIZE.y;

  InitPreview();
  dst   = m_preview->GetPixels();
  pitch = m_preview->GetPitch();

  // Top transparent+empty row
  for (i.y = 0; i.y < startCell.y; i.y++)
    for (i.x = 0; i.x < nbCells.x; i.x++)
      item.push_back(&EmptyTile);

  for (; i.y < endCell.y; i.y++) {
    // Empty left border
    for (i.x = 0; i.x<startCell.x; i.x++)
      item.push_back(&EmptyTile);

    // Prepare buffer for current line
    if (i.y == startCell.y) {
      memset(buffer, 0, CELL_SIZE.y*stride);
      // Only some lines are to be loaded
      crc = read_png_rows(png_ptr,
                          buffer + offsetx + offsety*stride,
                          CELL_SIZE.y - offsety, stride,
                          crc, width);
    } else if (i.y == endCell.y-1) {
      memset(buffer, 0, CELL_SIZE.y*stride);
      // Only some lines are to be loaded
      crc = read_png_rows(png_ptr,
                          buffer + offsetx, endoffy, stride,
                          crc, width);
    } else {
      crc = read_png_rows(png_ptr,
                          buffer + offsetx, CELL_SIZE.y, stride,
                          crc, width);
    }

    for (; i.x < endCell.x; i.x++) {
      TileItem_NonEmpty *ti;

      if (bpp==2)
        ti = new TileItem_ColorKey(buffer + (i.x - startCell.x)*CELL_SIZE.x*4, stride, alpha_threshold);
      else
        ti = new TileItem_AlphaSoftware(buffer + (i.x - startCell.x)*CELL_SIZE.x*4, stride);

      if (ti->NeedDelete()) {
        // no need to display this tile as it can be deleted!
#ifdef DBG_TILE
        printf("Deleting tile %i\n",i);
#endif
        delete ti;
        // Don't instanciate a new empty tile but use the already existing one
        item.push_back(&EmptyTile);
      } else {
        ti->ScalePreview(dst, i.x-startCell.x, pitch, m_shift);
        item.push_back(ti);
      }
    }
    dst += (CELL_SIZE.y>>m_shift)*pitch;

    // Empty right border
    for (; i.x < nbCells.x; i.x++)
      item.push_back(&EmptyTile);
  }

  // Bottom transparent+empty row
  for (; i.y < nbCells.y; i.y++)
    for (i.x = 0; i.x < nbCells.x; i.x++)
      item.push_back(&EmptyTile);
  ret = true;

end:
  if (buffer)
    delete[] buffer;
  if (png_ptr)
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
  if (info_ptr)
    png_destroy_info_struct(png_ptr, &info_ptr);
  if (f)
    fclose(f);
  return ret;
}

uchar Tile::GetAlpha(const Point2i & pos) const
{
  return item[pos.y / CELL_SIZE.y * nbCells.x + pos.x / CELL_SIZE.x]->GetAlpha(pos % CELL_SIZE);
}

void Tile::DrawTile()
{
  Point2i firstCell = Clamp(Camera::GetInstance()->GetPosition() / CELL_SIZE);
  Point2i lastCell = Clamp((Camera::GetInstance()->GetPosition() + Camera::GetInstance()->GetSize()) / CELL_SIZE);
  Point2i i;
  for (i.y = firstCell.y; i.y <= lastCell.y; i.y++)
    for (i.x = firstCell.x; i.x <= lastCell.x; i.x++)
      item[i.y*nbCells.x + i.x]->Draw(i);
}

void Tile::DrawTile_Clipped(Rectanglei & worldClip) const
{
  // Revision 514:
  // worldClip.SetSize( worldClip.GetSize() + 1); // mmm, does anything gives areas
  // too small to redraw ?
  //
  // Revision 3095:
  // Sorry, I don't understand that comment. Moreover the +1 produces a bug when the ground of
  // a map have an alpha value != 255 and != 0
  worldClip.SetSize(worldClip.GetSize());
  Point2i firstCell = Clamp(worldClip.GetPosition() / CELL_SIZE);
  Point2i lastCell  = Clamp((worldClip.GetBottomRightPoint()) / CELL_SIZE);
  Point2i c;

  for (c.y = firstCell.y; c.y <= lastCell.y; c.y++) {
    for (c.x = firstCell.x; c.x <= lastCell.x; c.x++) {
      if (item[c.y*nbCells.x + c.x]->IsTotallyEmpty())
        continue;
      TileItem_NonEmpty *tin = static_cast<TileItem_NonEmpty*>(item[c.y*nbCells.x + c.x]);

      // For all selected items, clip source and destination blitting rectangles
      Rectanglei destRect(c * CELL_SIZE, CELL_SIZE);

      destRect.Clip(worldClip);
      if (destRect.Intersect(*Camera::GetInstance())) {
        Point2i ptDest = destRect.GetPosition() - Camera::GetInstance()->GetPosition();
        Point2i ptSrc = destRect.GetPosition() - c * CELL_SIZE;


        GetMainWindow().Blit(tin->GetSurface(),
                             Rectanglei(ptSrc, destRect.GetSize()),
                             ptDest);
      }
    }
  }
}

Surface Tile::GetPart(const Rectanglei& rec)
{
  Surface part(rec.GetSize(), SDL_SWSURFACE|SDL_SRCALPHA, true);
  part.SetAlpha(0,0);
  part.Fill(0x00000000);
  part.SetAlpha(SDL_SRCALPHA,0);

  Point2i firstCell = Clamp(rec.GetPosition() / CELL_SIZE);
  Point2i lastCell = Clamp((rec.GetPosition() + rec.GetSize()) / CELL_SIZE);
  Point2i i = nbCells - 1;

  for (i.y = firstCell.y; i.y <= lastCell.y; i.y++) {
    for (i.x = firstCell.x; i.x <= lastCell.x; i.x++) {
      TileItem *ti = item[i.y*nbCells.x + i.x];
      if (ti->IsTotallyEmpty())
        continue;
      TileItem_NonEmpty *tin = static_cast<TileItem_NonEmpty*>(ti);

      Point2i cell_pos = i * CELL_SIZE;
      Rectanglei src;
      Point2i dst;
      src.SetPosition(rec.GetPosition() - cell_pos);
      if (src.GetPositionX() < 0) src.SetPositionX(0);
      if (src.GetPositionY() < 0) src.SetPositionY(0);

      src.SetSize(rec.GetPosition() + rec.GetSize() - cell_pos - src.GetPosition());
      if (src.GetSizeX() + src.GetPositionX() > CELL_SIZE.x) src.SetSizeX(CELL_SIZE.x - src.GetPositionX());
      if (src.GetSizeY() + src.GetPositionY() > CELL_SIZE.y) src.SetSizeY(CELL_SIZE.y - src.GetPositionY());

      dst =  cell_pos - rec.GetPosition();
      if (dst.x < 0) dst.x = 0;
      if (dst.y < 0) dst.y = 0;

      tin->GetSurface().SetAlpha(0, 0);
      part.Blit(tin->GetSurface(), src, dst);
      tin->GetSurface().SetAlpha(SDL_SRCALPHA, 0);
    }
  }
  return part;
}

void Tile::CheckEmptyTiles()
{
  int cellsCount = nbCells.x * nbCells.y;

  for (int i = 0; i < cellsCount; i++) {
    if (item[i]->IsTotallyEmpty())
      continue;
    TileItem_NonEmpty *t = static_cast<TileItem_NonEmpty*>(item[i]);

    if (t->NeedCheckEmpty() && t->NeedDelete()) {
      // no need to display this tile as it can be deleted!
#ifdef DBG_TILE
      printf("Deleting tile %i\n",i);
#endif
      delete item[i];
      // Don't instanciate a new empty tile but use the already existing one
      item[i] = &EmptyTile;
    }
  }
}

