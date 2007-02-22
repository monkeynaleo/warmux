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
  // Start with at least 32 points buffer
  array_size = 32;
  vx = new Sint16[array_size];
  vy = new Sint16[array_size];
  buffer_size = 0;
}

PolygonBuffer::~PolygonBuffer()
{
  delete vx;
  delete vy;
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
    Sint16 * tmp_vx = vx;
    Sint16 * tmp_vy = vy;
    // double the buffer size (64, 128, 256, 512)
    // to avoid call of delete/new at each new point
    array_size = (array_size * 2 > size ? array_size * 2 : size);
    vx = new Sint16[array_size];
    vy = new Sint16[array_size];
    for(int i = 0; i < buffer_size; i++) {
      vx[i] = tmp_vx[i];
      vy[i] = tmp_vy[i];
    }
    buffer_size = size;
    delete tmp_vx;
    delete tmp_vy;
  }
}

Polygon::Polygon()
{
  Init();
}

Polygon::Polygon(const std::vector<Point2d> shape)
{
  Init();
  original_shape = shape;
  shape_buffer->SetSize(original_shape.size());
}

Polygon::Polygon(const Polygon & poly)
{
  Init();
  texture = poly.texture;
  if(poly.IsPlaneColor()) {
    plane_color = new Color(poly.GetPlaneColor());
  }
  if(poly.IsBordered()) {
    border_color = new Color(poly.GetBorderColor());
  }
  original_shape = poly.original_shape;
  shape_buffer->SetSize(original_shape.size());
}

void Polygon::Init()
{
  texture = NULL;
  plane_color = NULL;
  border_color = NULL;
  original_shape.clear();
  shape_buffer = new PolygonBuffer();
}

void Polygon::ApplyTransformation(AffineTransform2D & trans)
{
  Point2d tmp;
  int i = 0;
  for(std::vector<Point2d>::iterator point = original_shape.begin();
      point != original_shape.end(); point++, i++) {
    tmp = trans * (*point);
    shape_buffer->vx[i] = (int)tmp.x;
    shape_buffer->vy[i] = (int)tmp.y;
  }
}

void Polygon::AddPoint(const Point2d & p)
{
  original_shape.push_back(p);
  shape_buffer->SetSize(original_shape.size());
  shape_buffer->vx[original_shape.size() - 1] = (int)p.x;
  shape_buffer->vy[original_shape.size() - 1] = (int)p.y;
}

// And the famous Bezier curve. And this algorithme is that simple ? I'm so disappointed !
// But now you can say to the world wormux is using Bezier curve.
void Polygon::AddBezierCurve(Point2d anchor1, Point2d control1, Point2d control2, Point2d anchor2, int num_steps)
{
  Point2d tmp1 = anchor1 + control1;
  Point2d tmp2 = anchor2 + control2;
  double a, b;
  for(int step = 0; step < num_steps; step++) {
    a = ((float)step / (float)num_steps) * 1.0;
    b = 1 - a;
    AddPoint(anchor1 * b * b * b + tmp1 * 3.0 * b * b * a + tmp2 * 3.0 * b * a * a + anchor2 * a * a * a);
  }
}

// Generate a new polygon with Bezier interpolation
Polygon * Polygon::GetBezierInterpolation(double smooth_value)
{
  int previous, index_p1, index_p2, next;
  Polygon * tmp = new Polygon();
  for(index_p1 = 0; index_p1 < (int)original_shape.size(); index_p1++) {
    previous = index_p1 - 1;
    previous = (previous > 0 ? previous : original_shape.size() - 1);
    index_p2 = (index_p1 + 1) % original_shape.size();
    next = (index_p1 + 2) % original_shape.size();

    Point2d p0 = original_shape[previous];
    Point2d p1 = original_shape[index_p1];
    Point2d p2 = original_shape[index_p2];
    Point2d p3 = original_shape[next];

    Point2d c1 = (p0 + p1) / 2.0;
    Point2d c2 = (p1 + p2) / 2.0;
    Point2d c3 = (p2 + p3) / 2.0;

    double len1 = sqrt((p1.x - p0.x) * (p1.x - p0.x) + (p1.y - p0.y) * (p1.y - p0.y));
    double len2 = sqrt((p2.x - p1.x) * (p2.x - p1.x) + (p2.y - p1.y) * (p2.y - p1.y));
    double len3 = sqrt((p3.x - p2.x) * (p3.x - p2.x) + (p3.y - p2.y) * (p3.y - p2.y));

    Point2d m1 = c1 + (c2 - c1) * (len1 / (len1 + len2));
    Point2d m2 = c2 + (c3 - c2) * (len2 / (len2 + len3));

    Point2d ctrl1 = m1 + (c2 - m1) * smooth_value + p1 - m1;
    Point2d ctrl2 = m2 + (c2 - m2) * smooth_value + p2 - m2;
    tmp->AddBezierCurve(p1, ctrl1, ctrl2, p2);
  }
  return tmp;
}

PolygonBuffer * Polygon::GetPolygonBuffer() const
{
  return shape_buffer;
}

// expand the polygon (to draw a little border for example)
void Polygon::Expand(const int expand_value)
{
  if(original_shape.size() < 2) return;
  std::vector<Point2d> tmp_shape;
  std::vector<Point2d>::iterator point = original_shape.begin();
  AffineTransform2D trans;
  trans.SetRotation(-M_PI_2);
  Point2d previous_point, tmp_point;
  tmp_shape.clear();
  previous_point = *point;
  point ++;
  int i = 0;
  for(i=0; point != original_shape.end(); point++, i++) {
    tmp_point = previous_point - *point;
    tmp_point = trans * tmp_point; // Rotate of -90°
    //Normalize(tmp_point);
    tmp_point *= expand_value; // set length
    tmp_point += previous_point;
    tmp_shape.push_back(tmp_point);
    shape_buffer->vx[i] = (int)tmp_point.x;
    shape_buffer->vy[i] = (int)tmp_point.y;
    previous_point = *point;
  }
  // loop back the last and first point
  tmp_point = *(original_shape.end()) - *(original_shape.begin());
  tmp_point = trans * tmp_point; // Rotate of -90°
  //Normalize(tmp_point);
  tmp_point *= expand_value; // set length
  tmp_point += previous_point;
  tmp_shape.push_back(tmp_point);
  shape_buffer->vx[original_shape.size() - 1] = (int)tmp_point.x;
  shape_buffer->vy[original_shape.size() - 1] = (int)tmp_point.y;
  original_shape.clear();
  original_shape = tmp_shape;
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

void Polygon::SetTexture(Surface * texture_surface)
{
  texture = texture_surface;
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
