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

#ifndef POLYGON_H
#define POLYGON_H

#include <vector>
#include "../tool/affine_transform.h"
#include "surface.h"
#include "sprite.h"

class Surface;
class Sprite;

/** Use to draw the polygon */
class PolygonBuffer {
 public:
  Sint16 * vx;
  Sint16 * vy;
  int buffer_size;
  int array_size;
  PolygonBuffer();
  ~PolygonBuffer();
  int GetSize() const;
  void SetSize(const int size);
};

/** Store information about a item (sprite) of the polygon */
class PolygonItem {
 typedef enum { TOP,  V_CENTERED, BOTTOM } V_align;
 typedef enum { LEFT, H_CENTERED, RIGHT } H_align;
 protected:
  Point2d position;
  Point2i trans_position;
  Sprite * item;
  V_align v_align;
  H_align h_align;
 public:
  PolygonItem(Sprite * sprite, const Point2d & pos, V_align v_a = V_CENTERED, H_align h_a = H_CENTERED);
  void SetPosition(const Point2d & pos);
  void SetAlignment(V_align v_a = V_CENTERED, H_align h_a = H_CENTERED);
  const Point2d & GetPosition();
  const Point2i & GetTransformedPosition();
  void SetSprite(Sprite * sprite);
  const Sprite * GetSprite();
  void ApplyTransformation(const AffineTransform2D & trans);
  void Draw(Surface * dest);
};

/** Store information about a simple shape */
class Polygon {
 protected:
  Surface * texture;
  Color * plane_color;
  Color * border_color;
  Point2d max;
  Point2d min;
  // Original shape
  std::vector<Point2d> original_shape;
  // Vector of icons
  std::vector<PolygonItem> items;
  // Shape position after an affine transformation
  PolygonBuffer * shape_buffer;

 public:
  Polygon();
  Polygon(const std::vector<Point2d> shape);
  Polygon(const Polygon & poly);
  void Init();
  // Point handling
  void AddPoint(const Point2d & p);
  void InsertPoint(int index, const Point2d & p);
  void DeletePoint(int index);
  void ApplyTransformation(const AffineTransform2D & trans);

  // Use to randomize a construction
  Point2d GetRandomUpperPoint();
  int GetRandomPointIndex();

  // Interpolation handling
  void AddBezierCurve(const Point2d anchor1, const Point2d control1,
                             const Point2d control2, const Point2d anchor2,
                             const int num_steps = 20, const bool add_first_point = true,
                             const bool add_last_point = true);
  void AddRandomCurve(const Point2d start, const Point2d end,
                      const double x_random_offset, const double y_random_offset,
                      const int num_steps, const bool add_first_point = true,
                      const bool add_last_point = true);
  Polygon * GetBezierInterpolation(double smooth_value = 1.0, int num_steps = 20, double rand = 0.0);
  void Expand(const double expand_value);

  // Size information
  double GetWidth() const;
  double GetHeight() const;
  Point2d GetSize() const;
  Point2i GetIntSize() const;
  int GetNbOfPoint() const;
  Point2d GetMin() const;
  Point2i GetIntMin() const;
  Point2d GetMax() const;
  Point2i GetIntMax() const;
  Rectanglei GetRectangleToRefresh() const;

  // Buffer of transformed point
  PolygonBuffer * GetPolygonBuffer() const;

  // Type of the polygon
  bool IsTextured() const;
  bool IsPlaneColor() const;
  bool IsBordered() const;
  // Texture handling
  Surface * GetTexture() const;
  void SetTexture(Surface * texture_surface);
  // Color handling
  void SetBorderColor(const Color & color);
  void SetPlaneColor(const Color & color);
  const Color & GetBorderColor() const;
  const Color & GetPlaneColor() const;

  // Drawing
  void Draw(Surface * dest);
  void DrawOnScreen();

  // Item management
  void AddItem(Sprite * sprite, const Point2d & pos);
  void DelItem(int index);
};

#endif /* POLYGON_H */
