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
 * Random map generator
 *****************************************************************************/

#include "random_map.h"
#include "../tool/random.h"

RandomMap::RandomMap(Profile *profile, const int width, const int height)
{
  this->profile = profile;
  SetSize(width, height);
  number_of_island = Random::GetInt(2, 10);
  border_size = 8;
  result = Surface(Point2i(width,height), SDL_SWSURFACE|SDL_SRCALPHA, true);
  border_color = resource_manager.LoadColor(profile, "border_color");
}

void RandomMap::SetSize(const int width, const int height)
{
  this->width = width;
  this->height = height;
}

const Point2i RandomMap::GetSize()
{
  return Point2i(width, height);
}

const int RandomMap::GetWidth()
{
  return width;
}

const int RandomMap::GetHeight()
{
  return height;
}

void RandomMap::AddObject(Surface & object)
{
}

void RandomMap::SetBorderSize(const double border)
{
}

void RandomMap::SetBorderColor(const Color color)
{
}

const bool RandomMap::IsOpen()
{
  return is_open;
}

void RandomMap::Generate()
{
  srand(time(NULL));
  double h = (height * 2.0) / number_of_island;
  double w = (width * 2.0) / number_of_island;
  double current_h_position = (width / 2.0) + Random::GetDouble(-w, w);
  double current_v_position = (height / 2.0) + Random::GetDouble(-h, h);
  double x_direction = 1.0;
  double y_direction = 1.0;
  Surface texture = resource_manager.LoadImage(profile, "texture");
  Surface element = resource_manager.LoadImage(profile, "element");
  AffineTransform2D translate;
  result.Fill(0);
  for(int i = 0; i < number_of_island; i++) {
    translate = AffineTransform2D::Translate(current_h_position, current_v_position);
    // Generate Island
    GenerateIsland(Random::GetDouble(w * 1.25, w * 2.25),
                   Random::GetDouble(h * 1.25, h * 2.25));
    bezier_shape->SetTexture(&texture);
    expanded_bezier_shape->SetPlaneColor(border_color);
    bezier_shape->ApplyTransformation(translate);
    expanded_bezier_shape->ApplyTransformation(translate);
    // Then draw it
    result.DrawPolygon(*expanded_bezier_shape);
    result.DrawPolygon(*bezier_shape);
    // compute a new position
    current_h_position += x_direction * Random::GetDouble(w * 0.75, w * 1.25);
    current_v_position += y_direction * Random::GetDouble(h * 0.75, h * 1.25);
    if(current_h_position > width) {
      x_direction = -x_direction;
      current_h_position = width - (current_h_position - width);
    } else if(current_h_position < 0) {
      x_direction = -x_direction;
      current_h_position = -current_h_position;
    }
    if(current_v_position > height) {
      y_direction = -y_direction;
      current_v_position = height - (current_v_position - height);
    } else if(current_v_position < 0) {
      y_direction = -y_direction;
      current_v_position = -current_v_position;
    }
  }
}

void RandomMap::GenerateIsland(double width, double height)
{
  double x_rand_offset = Random::GetDouble(10.0, 15.0);
  double y_rand_offset = Random::GetDouble(10.0, 15.0);
  double coef = Random::GetSign() * Random::GetDouble(0.5, 1.0);
  // Generate a random shape
  random_shape = PolygonGenerator::GenerateRandomTrapeze(width, height, x_rand_offset, y_rand_offset, coef);
  bezier_shape = random_shape->GetBezierInterpolation();
  expanded_bezier_shape = new Polygon(*bezier_shape);
  // Expand the random, bezier shape !
  expanded_bezier_shape->Expand(border_size);
}

void RandomMap::SaveMap()
{
  result.ImgSave("/tmp/test.png");
}

Surface RandomMap::GetRandomMap()
{
  return result;
}
