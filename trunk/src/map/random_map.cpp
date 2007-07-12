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
#include "game/config.h"
#include "graphic/polygon_generator.h"
#include "map/maps_list.h"
#include "tool/affine_transform.h"
#include "tool/random.h"
#include "tool/resource_manager.h"

MapElement::MapElement(Surface & object, Point2i & pos)
{
  element = object;
  position = pos;
}

Surface & MapElement::GetElement()
{
  return element;
}

Point2i & MapElement::GetPosition()
{
  return position;
}

RandomMap::RandomMap(Profile *profile, const int width, const int height)
{
  this->profile = profile;
  SetSize(width, height);
  border_size = 8;
  result = Surface(Point2i(width,height), SDL_SWSURFACE|SDL_SRCALPHA, true);
  random_shape = NULL;
  bezier_shape = NULL;
  expanded_bezier_shape = NULL;
  // Loading resources
  border_color = resource_manager.LoadColor(profile, "border_color");
  texture = resource_manager.LoadImage(profile, "texture");
  element = resource_manager.LoadImage(profile, "element");
  element_list.clear();
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

void RandomMap::AddElement(Surface & object, Point2i position)
{
  element_list.push_back(MapElement(object, position));
}

void RandomMap::DrawElement()
{
  std::vector<MapElement>::iterator elt;
  for(elt = element_list.begin(); elt != element_list.end(); elt++) {
    Surface & tmp = elt->GetElement();
    result.MergeSurface(tmp, elt->GetPosition() - Point2i((int)(tmp.GetWidth() / 2.0), tmp.GetHeight()));
  }
}

void RandomMap::SetBorderSize(const double border)
{
  border_size = border;
}

void RandomMap::SetBorderColor(const Color color)
{
  border_color = color;
}

const bool RandomMap::IsOpen()
{
  return is_open;
}

void RandomMap::Generate()
{
  srand(time(NULL));

  double minhei = height / Random::GetDouble(7, 5);
  double maxhei = height / Random::GetDouble(1.5, 4);

  double current_y_pos = height - Random::GetDouble(minhei, maxhei);
  int num_of_points = Random::GetInt(5,20);

  result.Fill(0);

  Polygon *tmp = new Polygon();

  // +10 so it's outside the screen
  tmp->AddPoint(Point2d(-10, height+10));

  for (int i = 2; i < num_of_points-1; i++) {
      current_y_pos = height - Random::GetDouble(minhei, maxhei);
      double current_x_pos = (((double)i / (double) num_of_points) * (double)width);
      tmp->AddPoint( Point2d(current_x_pos,  current_y_pos));
      if (Random::GetInt(0,100) < 20) {
	  // +20 hardcoded, yucky.
	  AddElement(element, Point2i((int)current_x_pos, (int)(current_y_pos+20.0)));
      }
  }

  tmp->AddPoint(Point2d(width+10, height+10));
  tmp->AddPoint(Point2d(width/2, height+10));

  bezier_shape = tmp->GetBezierInterpolation(1.0, 30, 0.3);

  bezier_shape->SetTexture(&texture);
  bezier_shape->SetPlaneColor(border_color);
  bezier_shape->Draw(&result);

  DrawElement();
}

bool RandomMap::GenerateIsland(double width, double height)
{
  int nb_of_point;
  double x_rand_offset, y_rand_offset, coef;
  if(random_shape) {
    random_shape = NULL;
    delete random_shape;
  }
  if(bezier_shape) {
    bezier_shape = NULL;
    delete bezier_shape;
  }
  if(expanded_bezier_shape) {
    expanded_bezier_shape = NULL;
    delete expanded_bezier_shape;
  }
  // Generate a random shape
  switch(Random::GetInt(DENTED_CIRCLE, ROUNDED_RECTANGLE)) {
    case DENTED_CIRCLE:
      nb_of_point = Random::GetInt(5, 20);
      x_rand_offset = width / Random::GetDouble(2.0, 15.0);
      random_shape = PolygonGenerator::GenerateDentedCircle(width, nb_of_point, x_rand_offset);
      break;
    case ROUNDED_RECTANGLE:
      random_shape = PolygonGenerator::GenerateRectangle(width, height);
      break;
    default: case DENTED_TRAPEZE:
      x_rand_offset = Random::GetDouble(10.0, 15.0);
      y_rand_offset = Random::GetDouble(10.0, 15.0);
      coef = Random::GetSign() * Random::GetDouble(0.5, 1.0);
      random_shape = PolygonGenerator::GenerateRandomTrapeze(width, height, x_rand_offset, y_rand_offset, coef);
      break;
  }
  if(random_shape->GetNbOfPoint() < 4)
    return false;
  bezier_shape = random_shape->GetBezierInterpolation(1.0, 20, 1.5);
  expanded_bezier_shape = new Polygon(*bezier_shape);
  // Expand the random, bezier shape !
  expanded_bezier_shape->Expand(border_size);
  // Setting texture and border color
  bezier_shape->SetTexture(&texture);
  expanded_bezier_shape->SetPlaneColor(border_color);
  return true;
}

void RandomMap::SaveMap()
{
  result.ImgSave(Config::GetInstance()->GetPersonalDir() + ActiveMap().ReadName() + " - last random generation.png");
}

Surface RandomMap::GetRandomMap()
{
  return result;
}
