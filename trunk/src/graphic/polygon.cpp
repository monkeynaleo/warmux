/******************************************************************************
 *  Wormux is a convivial mass murder game.
 *  Copyright (C) 2001-2007 Wormux Team.
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
#include "../tool/random.h"
#include "../include/app.h"
#include "../map/map.h"

//=========== POLYGON BUFFER ============ //
// Use this structure to store transformed point
// In affine transformation, never transform directly the original point !
// If you do it, your point will becoming dented.

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
  delete[] vx;
  delete[] vy;
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
    delete[] tmp_vx;
    delete[] tmp_vy;
  }
}

//=========== POLYGON ITEM ============ //
// Use this structure to draw item

PolygonItem::PolygonItem()
{
  transformed_position = position = Point2d(0, 0);
  SetSprite(NULL);
  SetAlignment(V_CENTERED, H_CENTERED);
}

PolygonItem::PolygonItem(Sprite * sprite, const Point2d & pos, V_align v_a, H_align h_a)
{
  SetPosition(pos);
  SetSprite(sprite);
  SetAlignment(v_a, h_a);
}

PolygonItem::~PolygonItem()
{
}

void PolygonItem::SetPosition(const Point2d & pos)
{
  transformed_position = position = pos;
}

void PolygonItem::SetAlignment(V_align v_a, H_align h_a)
{
  v_align = v_a;
  h_align = h_a;
}

const Point2d & PolygonItem::GetPosition() const
{
  return position;
}

const Point2d & PolygonItem::GetTransformedPosition() const
{
  return transformed_position;
}

Point2i PolygonItem::GetIntTransformedPosition() const
{
  return Point2i((int)transformed_position.x, (int)transformed_position.y);
}

void PolygonItem::SetSprite(Sprite * sprite)
{
  item = sprite;
}

const Sprite * PolygonItem::GetSprite()
{
  return item;
}

void PolygonItem::ApplyTransformation(const AffineTransform2D & trans)
{
  transformed_position = trans * position;
}

void PolygonItem::Draw(Surface * dest)
{
  if(item == NULL)
    return;
  item->Blit(*dest, GetOffsetAlignment());
}

bool PolygonItem::Contains(const Point2d & p) const
{
  return Rectanglei(GetOffsetAlignment(), item->GetSize()).Contains(Point2i((int)p.x, (int)p.y));
}

Point2i PolygonItem::GetOffsetAlignment() const
{
  Point2i offset;
  switch(v_align) {
    case TOP: offset.y = (int)transformed_position.y; break;
    case BOTTOM: offset.y = (int)transformed_position.y - item->GetHeight(); break;
    default : offset.y = (int)transformed_position.y - (item->GetHeight() / 2); break;
  }
  switch(h_align) {
    case LEFT: offset.x = (int)transformed_position.x; break;
    case RIGHT: offset.x = (int)transformed_position.x - item->GetWidth(); break;
    default : offset.x = (int)transformed_position.x - (item->GetWidth() / 2); break;
  }
  return offset;
}

//=========== POLYGON ============ //
// Store a vector of point and handle affine transformation,
// Bezier interpolation handling etc.

Polygon::Polygon()
{
  Init();
}

Polygon::Polygon(const std::vector<Point2d> shape)
{
  Init();
  transformed_shape = original_shape = shape;
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
  transformed_shape = original_shape = poly.original_shape;
  shape_buffer->SetSize(original_shape.size());
}

void Polygon::Init()
{
  texture = NULL;
  plane_color = NULL;
  border_color = NULL;
  original_shape.clear();
  transformed_shape.clear();
  items.clear();
  shape_buffer = new PolygonBuffer();
  min = max = Point2d(0.0, 0.0);
}

void Polygon::ApplyTransformation(const AffineTransform2D & trans)
{
  int i = 0;
  for(std::vector<Point2d>::iterator point = original_shape.begin();
      point != original_shape.end(); point++, i++) {
    transformed_shape[i] = trans * (*point);
    shape_buffer->vx[i] = (int)transformed_shape[i].x;
    shape_buffer->vy[i] = (int)transformed_shape[i].y;
    max = max.max(transformed_shape[i]);
    min = min.min(transformed_shape[i]);
  }
  for(std::vector<PolygonItem *>::iterator item = items.begin();
      item != items.end(); item++) {
    (*item)->ApplyTransformation(trans);
  }
}

// Check if a point is inside the polygon using Jordan curve theorem (amen).
// For better explanation : http://www.ecse.rpi.edu/Homepages/wrf/Research/Short_Notes/pnpoly.html
// Note : Compile but not tested yet !
bool Polygon::IsInsidePolygon(const Point2d & p) const
{
  int i, j;
  bool c = false;
  for (i = 0, j = GetNbOfPoint() - 1; i < GetNbOfPoint(); j = i++) {
    if ((((transformed_shape[i].y <= p.y) && (p.y < transformed_shape[j].y)) ||
         ((transformed_shape[j].y <= p.y) && (p.y < transformed_shape[i].y))) &&
        (p.x < (transformed_shape[j].x - transformed_shape[i].x) * (p.y - transformed_shape[i].y) / (transformed_shape[j].y - transformed_shape[i].y) + transformed_shape[i].x))
          c = !c;
  }
  return c;
}

// Not accurate at 100% but sufficent for the moment
bool Polygon::IsOverlapping(const Polygon & poly) const
{
  for(int i = 0; i < GetNbOfPoint(); i++) {
    if(poly.IsInsidePolygon(transformed_shape[i]))
      return true;
  }
  return false;
}

void Polygon::AddPoint(const Point2d & p)
{
  original_shape.push_back(p);
  transformed_shape.push_back(p);
  shape_buffer->SetSize(original_shape.size());
  shape_buffer->vx[original_shape.size() - 1] = (int)p.x;
  shape_buffer->vy[original_shape.size() - 1] = (int)p.y;
  max = p.max(max);
  min = p.min(min);
}

void Polygon::InsertPoint(int index, const Point2d & p)
{
  if(index == GetNbOfPoint()) {
    AddPoint(p);
    return;
  }
  std::vector<Point2d> vector_tmp;
  std::vector<Point2d>::iterator point;
  Point2d tmp;
  int i = 0;
  shape_buffer->SetSize(original_shape.size() + 1);
  // Inserting first part of the point
  for(point = original_shape.begin();
      point != original_shape.end() && i < index; point++, i++) {
    tmp = *point;
    vector_tmp.push_back(tmp);
    shape_buffer->vx[i] = (int)tmp.x;
    shape_buffer->vy[i] = (int)tmp.y;
  }
  // Inserting a new point
  vector_tmp.push_back(p);
  shape_buffer->vx[i] = (int)p.x;
  shape_buffer->vy[i++] = (int)p.y;
  // And interting remaining point of previous shape
  for(; point != original_shape.end(); point++, i++) {
    tmp = *point;
    vector_tmp.push_back(tmp);
    shape_buffer->vx[i] = (int)tmp.x;
    shape_buffer->vy[i] = (int)tmp.y;
  }
  transformed_shape = original_shape = vector_tmp;
}

void Polygon::DeletePoint(int index)
{
  std::vector<Point2d> vector_tmp;
  Point2d tmp;
  int i = 0;
  shape_buffer->SetSize(shape_buffer->GetSize() - 1);
  for(std::vector<Point2d>::iterator point = original_shape.begin();
      point != original_shape.end(); point++, i++) {
    if(i == index) continue; // Skip point to remove
    tmp = (*point);
    vector_tmp.push_back(*point);
    shape_buffer->vx[i] = (int)tmp.x;
    shape_buffer->vy[i] = (int)tmp.y;
    max = max.max(tmp);
    min = min.min(tmp);
  }
  transformed_shape = original_shape = vector_tmp;
}

void Polygon::AddItem(Sprite * sprite, const Point2d & pos)
{
  items.push_back(new PolygonItem(sprite, pos));
}

void Polygon::AddItem(PolygonItem * item)
{
  items.push_back(item);
}

void Polygon::DelItem(int index)
{
  std::vector<PolygonItem *> vector_tmp;
  Point2d tmp;
  int i = 0;
  for(std::vector<PolygonItem *>::iterator item = items.begin();
      item != items.end(); item++, i++) {
    if(i == index) continue; // Skip point to remove
    vector_tmp.push_back(*item);
  }
  items = vector_tmp;
}

std::vector<PolygonItem *> Polygon::GetItem() const
{
  return items;
}

double Polygon::GetWidth() const
{
  return max.x - min.x;
}

double Polygon::GetHeight() const
{
  return max.y - min.y;
}

Point2d Polygon::GetSize() const
{
  return max - min;
}

Point2i Polygon::GetIntSize() const
{
  return GetIntMax() - GetIntMin() + Point2i(1, 1);
}

Point2d Polygon::GetMin() const
{
  return min;
}

Point2i Polygon::GetIntMin() const
{
  return Point2i((int)min.x, (int)min.y);
}

Point2d Polygon::GetMax() const
{
  return max;
}

Point2i Polygon::GetIntMax() const
{
  return Point2i((int)max.x, (int)max.y);
}

Rectanglei Polygon::GetRectangleToRefresh() const
{
  return Rectanglei(GetIntMin(), GetIntSize());
}

Point2d Polygon::GetRandomUpperPoint()
{
  std::vector<Point2d>::iterator point = transformed_shape.begin();
  Point2d tmp, previous;
  tmp = *point;
  int start = Random::GetInt(0, GetNbOfPoint());
  int i;
  for(i = 0; i < start; i++)
    point++;
  while(point != transformed_shape.end()) {
    previous = *point++;
    i++;
    tmp = *point - previous;
    tmp = tmp / tmp.Norm();
    if(tmp.y > 0.4)
      return tmp;
  }
  return Point2d();
}

int Polygon::GetNbOfPoint() const
{
  return (int)original_shape.size();
}

// And the famous Bezier curve. And this algorithme is that simple ? I'm so disappointed !
// But now you can say to the world wormux is using Bezier curve.
void Polygon::AddBezierCurve(const Point2d anchor1, const Point2d control1,
                             const Point2d control2, const Point2d anchor2,
                             const int num_steps, const bool add_first_point,
                             const bool add_last_point)
{
  Point2d tmp1 = anchor1 + control1;
  Point2d tmp2 = anchor2 + control2;
  double a, b;
  if(add_first_point)
    AddPoint(anchor1);
  for(int step = 1; step < num_steps - 1; step++) {
    a = ((float)step / (float)num_steps) * 1.0;
    b = 1 - a;
    AddPoint(anchor1 * b * b * b + tmp1 * 3.0 * b * b * a + tmp2 * 3.0 * b * a * a + anchor2 * a * a * a);
  }
  if(add_last_point)
    AddPoint(anchor2);
}

// Generate random point between 2 points
void Polygon::AddRandomCurve(const Point2d start, const Point2d end,
                             const double x_random_offset, const double y_random_offset,
                             const int num_steps, const bool add_first_point,
                             const bool add_last_point)
{
  Point2d step = (end - start) / num_steps;
  Point2d tmp;
  if(add_first_point)
    AddPoint(start);
  for (int i = 1; i < num_steps - 1; i++) {
    AddPoint(start + (step * i) + Point2d(Random::GetDouble(-x_random_offset, x_random_offset),
                                          Random::GetDouble(-y_random_offset, y_random_offset)));
  }
  if(add_last_point)
    AddPoint(end);
}

// Generate a new polygon with Bezier interpolation
Polygon * Polygon::GetBezierInterpolation(double smooth_value, int num_steps, double rand)
{
  Point2d p0, p1, p2, p3, c0, c1, c2, v1, v2;
  Polygon * shape = new Polygon();
  double l1, l2, l3;
  AffineTransform2D trans = AffineTransform2D();
  for(int index_p1 = 0; index_p1 < (int)original_shape.size(); index_p1++) {
    p0 = original_shape[(index_p1 == 0 ? original_shape.size() : index_p1) - 1];
    p1 = original_shape[index_p1];
    p2 = original_shape[(index_p1 + 1) % original_shape.size()];
    p3 = original_shape[(index_p1 + 2) % original_shape.size()];

    // compute center of [p0,p1], [p1,p2] and [p2,p3]
    c0 = p0 + ((p1 - p0) / 2.0);
    c1 = p1 + ((p2 - p1) / 2.0);
    c2 = p2 + ((p3 - p2) / 2.0);

    // Distance
    l1 = p0.Distance(p1);
    l2 = p1.Distance(p2);
    l3 = p2.Distance(p3);

    // Point control
    v1 = (c1 - c0) * (l2 / (l1 + l2)) * smooth_value;
    v2 = (c1 - c2) * (l2 / (l2 + l3)) * smooth_value;

    // Randomization
    if(rand != 0.0) {
      trans.SetRotation(Random::GetDouble(-rand, rand));
      v1 = trans * v1;
      trans.SetRotation(Random::GetDouble(-rand, rand));
      v2 = trans * v2;
    }

    shape->AddBezierCurve(p1, v1, v2, p2, num_steps, false);
  }
  return shape;
}

PolygonBuffer * Polygon::GetPolygonBuffer() const
{
  return shape_buffer;
}

// expand the polygon (to draw a little border for example)
void Polygon::Expand(const double expand_value)
{
  if(original_shape.size() < 2) return;
  std::vector<Point2d> tmp_shape;
  AffineTransform2D trans = AffineTransform2D::Rotate(M_PI_2);
  Point2d current, next, vector, expand;
  int i, j, k;
  for(i = 0; i < (int)original_shape.size(); i++) {
    j = (i + 1) % original_shape.size();
    current = original_shape[i];
    next    = original_shape[j];
    // If the next point is to close to current point skip next point
    // Avoid visual artefact
    k = 0;
    while(k < 10 && next.Distance(current) < 0.1) {
      j = (j + 1) % original_shape.size();
      next    = original_shape[j];
      k++;
    }
    vector = trans * (next - current);
    vector = (vector / vector.Norm()) * expand_value; // Normalize and length
    expand = current + vector;
    tmp_shape.push_back(expand);
    shape_buffer->vx[i] = (int)expand.x;
    shape_buffer->vy[i] = (int)expand.y;
  }
  original_shape.clear();
  transformed_shape = original_shape = tmp_shape;
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

const Color & Polygon::GetBorderColor() const
{
  return *border_color;
}

const Color & Polygon::GetPlaneColor() const
{
  return *plane_color;
}

void Polygon::Draw(Surface * dest)
{
  // Draw polygon
  if(IsPlaneColor())
    dest->FilledPolygon(shape_buffer->vx, shape_buffer->vy, shape_buffer->GetSize(), *plane_color);
  if(IsTextured())
    dest->TexturedPolygon(shape_buffer->vx, shape_buffer->vy, shape_buffer->GetSize(), texture, 0, 0);
  if(IsBordered())
    dest->AAPolygonColor(shape_buffer->vx, shape_buffer->vy, shape_buffer->GetSize(), *border_color);

  // Draw Item
  for(std::vector<PolygonItem *>::iterator item = items.begin();
      item != items.end(); item++) {
    (*item)->Draw(dest);
  }
}

void Polygon::DrawOnScreen()
{
  Draw(&AppWormux::GetInstance()->video.window);
  world.ToRedrawOnScreen(GetRectangleToRefresh());
}
