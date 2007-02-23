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
 * Polygon Generator. Generate various polygon shape on demand (including random one).
 *****************************************************************************/

#include <stdlib.h>
#include "polygon_generator.h"

Polygon * PolygonGenerator::GenerateCircle(double diameter, int nb_point)
{
  Polygon * tmp = new Polygon();
  AffineTransform2D trans = AffineTransform2D();
  Point2d top = Point2d(0.0, diameter / 2.0);
  tmp->AddPoint(top);
  for(int i = 1; i < nb_point; i++) {
    trans.SetRotation((2.0 * M_PI * i) / nb_point);
    tmp->AddPoint(trans * top);
  }
  return tmp;
}

Polygon * PolygonGenerator::GenerateRectangle(double width, double height)
{
  Polygon * tmp = new Polygon();
  tmp->AddPoint(Point2d(-width / 2.0, -height / 2.0));
  tmp->AddPoint(Point2d( width / 2.0, -height / 2.0));
  tmp->AddPoint(Point2d( width / 2.0,  height / 2.0));
  tmp->AddPoint(Point2d(-width / 2.0,  height / 2.0));
  return tmp;
}

Polygon * PolygonGenerator::GenerateRoundedRectangle(double width, double height, double edge)
{
  Polygon * tmp = new Polygon();
  double edge_vector = edge / 2.0;
  tmp->AddBezierCurve(Point2d(-width / 2 + edge, -height / 2),
                      Point2d(-edge_vector, 0),
                      Point2d(0, -edge_vector),
                      Point2d(-width / 2, -height / 2 + edge));
  tmp->AddBezierCurve(Point2d(-width / 2, height / 2 - edge),
                      Point2d(0, edge_vector),
                      Point2d(-edge_vector, 0),
                      Point2d(-width / 2 + edge, height / 2));
  tmp->AddBezierCurve(Point2d(width / 2 - edge, height / 2),
                      Point2d(edge_vector, 0),
                      Point2d(0, edge_vector),
                      Point2d(width / 2, height / 2 - edge));
  tmp->AddBezierCurve(Point2d(width / 2, -height / 2 + edge),
                      Point2d(0, -edge_vector),
                      Point2d(edge_vector, 0),
                      Point2d(width / 2 - edge, -height / 2));
  return tmp;
}

double PolygonGenerator::Random(double min, double max)
{
  if(max < min) {
    double tmp = min;
    min = max;
    max = tmp;
  }
  return min + ((double)rand() / (double)RAND_MAX) * (max - min);
}

Polygon * PolygonGenerator::GenerateRandomShape()
{
  Polygon * tmp = new Polygon();
  srand(time(NULL));
  int number_of_bottom_point = (int)Random(10.0, 20.0);
  int number_of_upper_point  = (int)Random(10.0, 15.0);
  double height = Random(400.0, 1500.0);
  double width  = Random(400.0, 2000.0);
  double bottom_range;
  double offset_range;
  double x;
  // First we generate bottom element from left to right
  for(int i = 0; i < number_of_bottom_point; i++) {
    x = (width / (double)number_of_bottom_point) * i;
    bottom_range = Random(-10.0, 10.0);
    offset_range = Random(-5.0, 5.0);
    tmp->AddPoint(Point2d(x + offset_range, height + bottom_range));
  }
  // Then we generate upper element from right to left
  for(int i = 0; i < number_of_upper_point; i++) {
    x = (width / number_of_upper_point) * (number_of_upper_point - i);
    bottom_range = Random(-10.0, 10.0);
    offset_range = Random(-5.0, 5.0);
    tmp->AddPoint(Point2d(x + offset_range, bottom_range));
  }
  return tmp;
}
