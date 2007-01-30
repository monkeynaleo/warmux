/******************************************************************************
 *  Wormux is a convivial mass murder game.
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
 * Polygon class. Store point of a polygon and handle affine transformation
 *****************************************************************************/

#include "polygon.h"

PolygonBuffer::PolygonBuffer()
{
  vx = NULL;
  vy = NULL;
  buffer_size = 0;
  array_size = 0;
}

PolygonBuffer::~PolygonBuffer()
{
  if(vx != NULL) {
    delete vx;
    delete vy;
  }
}

int PolygonBuffer::GetSize() const
{
  return buffer_size;
}

void PolygonBuffer::SetSize(const int size)
{
  if(array_size > size) {
    buffer_size = size;
  } else {
    if(vx != NULL) {
      delete vx;
      delete vy;
    }
    vx = new Sint16[size];
    vy = new Sint16[size];
    array_size = buffer_size = size;
  }
}

Polygon::Polygon()
{
  texture = NULL;
  plane_color = NULL;
  border_color = NULL;
  original_shape.clear();
  shape_buffer = new PolygonBuffer();
}

Polygon::Polygon(const std::list<Point2i> shape)
{
  texture = NULL;
  plane_color = NULL;
  border_color = NULL;
  original_shape = shape;
  shape_buffer = new PolygonBuffer();
  shape_buffer->SetSize(original_shape.size());
}

Polygon::Polygon(const Polygon & poly)
{
  texture = poly.texture;
  plane_color = NULL;
  border_color = NULL;
  if(poly.IsPlaneColor()) {
    plane_color = new Color(poly.GetPlaneColor());
  }
  if(poly.IsBordered()) {
    border_color = new Color(poly.GetBorderColor());
  }
  original_shape = poly.original_shape;
  shape_buffer = new PolygonBuffer();
  shape_buffer->SetSize(original_shape.size());
}

void Polygon::ApplyTransformation(AffineTransform2D & trans)
{
  Point2i tmp;
  int i = 0;
  for(std::list<Point2i>::iterator point = original_shape.begin();
      point != original_shape.end(); point++, i++) {
    tmp = trans * (*point);
    shape_buffer->vx[i] = tmp.x;
    shape_buffer->vy[i] = tmp.y;
  }
}

void Polygon::AddPoint(const Point2i & p)
{
  original_shape.push_back(p);
  shape_buffer->SetSize(original_shape.size());
}

// Get information about Polygon
bool Polygon::IsTextured() const
{
  return texture != NULL;
}

bool Polygon::IsPlaneColor() const
{
  return plane_color != NULL;
}

bool Polygon::IsBordered() const
{
  return border_color != NULL;
}

// Texture handling
Surface * Polygon::GetTexture() const
{
  return texture;
}

void Polygon::SetTexture(const Surface * texture)
{
  texture = texture;
}

// Color handling
void Polygon::SetBorderColor(const Color & color)
{
  if(border_color == NULL)
    border_color = new Color();
  *border_color = color;
}

void Polygon::SetPlaneColor(const Color & color)
{
  if(plane_color == NULL)
    plane_color = new Color();
  *plane_color = color;
}

Color & Polygon::GetBorderColor() const
{
  return *border_color;
}

Color & Polygon::GetPlaneColor() const
{
  return *plane_color;
}
