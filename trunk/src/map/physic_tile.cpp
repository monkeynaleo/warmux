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
 * PhysicTile
 *****************************************************************************/
#include <Box2D.h>
#include <iostream>

#include "map/physic_tile.h"
#include "map/tileitem.h"
#include "physic/physical_engine.h"
#include "physic/physical_shape.h"

#ifdef DEBUG
#include "graphic/color.h"
#include "graphic/colors.h"
#include "graphic/video.h"
#include "map/camera.h"
#endif

static const double GROUND_FRICTION = 5.0;

PhysicTile::PhysicTile(Point2i size, Point2i offset, Point2i tile_offset, TileItem *tile, PhysicTile *parent_physic_tile, int level):
  m_is_subdivided(false),
  m_is_containing_polygon(false),
  m_shape(NULL),
  m_parent_physic_tile(parent_physic_tile),
  m_parent_tile(tile),
  m_size(size),
  m_offset(offset),
  m_tile_offset(tile_offset),
  m_level(level)
{
  Generate();
}

PhysicTile::~PhysicTile()
{
  Clean();
}

void PhysicTile::Generate()
{
  Clean();
  m_fullness = IsFull();

  switch (m_fullness) {
  case EMPTY:
    GenerateEmpty();
    break;
  case FULL:
    GenerateFull();
    break;
  case MIXTE:
    GenerateMixte();
    break;
  }
}

void PhysicTile::GenerateEmpty()
{
  m_is_subdivided = false;
  m_is_containing_polygon = false;

}

void PhysicTile::GenerateFull()
{
  m_is_subdivided = false;
  m_is_containing_polygon = true;

  InitShape();

  m_shape->AddPoint(Point2d((double(m_offset.x + m_tile_offset.x) / PIXEL_PER_METER),
                         (double(m_offset.y + m_tile_offset.y)/ PIXEL_PER_METER)));

  m_shape->AddPoint(Point2d((double(m_offset.x + m_tile_offset.x + m_size.x-1)/ PIXEL_PER_METER),
                         (double(m_offset.y + m_tile_offset.y)/ PIXEL_PER_METER)));

  m_shape->AddPoint(Point2d((double(m_offset.x + m_size.x-1 + m_tile_offset.x)/ PIXEL_PER_METER),
                         (double(m_offset.y + m_size.y-1 + m_tile_offset.y)/ PIXEL_PER_METER)));

  m_shape->AddPoint(Point2d((double(m_offset.x + m_tile_offset.x)/ PIXEL_PER_METER),
                         (double(m_offset.y + m_size.y-1 + m_tile_offset.y)/ PIXEL_PER_METER)));

  m_shape->Generate();

}

void PhysicTile::GenerateMixte()
{
  bool is_generation_work = true; // WARNING: is true the right default value ??
  if (m_level <= 0) {
    is_generation_work = GeneratePolygone();
  }

  if (m_level > 0 || !is_generation_work) {
    //Subdivise
    m_is_subdivided = true;
    m_is_containing_polygon = false;
    int new_width1 = m_size.x/2;
    int new_width2 = m_size.x - new_width1;
    int new_height1 = m_size.y/2;;
    int new_height2 = m_size.y - new_height1;

    m_physic_tiles[0] = new PhysicTile(Point2d(new_width1,new_height1),
				       m_offset,
				       m_tile_offset, m_parent_tile, m_parent_physic_tile, m_level -1);

    m_physic_tiles[1] = new PhysicTile(Point2d(new_width2,new_height1),
				       Point2d(m_offset.x + new_width1, m_offset.y),
				       m_tile_offset, m_parent_tile, m_parent_physic_tile, m_level -1);

    m_physic_tiles[2] = new PhysicTile(Point2d(new_width1,new_height2),
				       Point2d(m_offset.x , m_offset.y+ new_height1),
				       m_tile_offset, m_parent_tile, m_parent_physic_tile, m_level -1);

    m_physic_tiles[3] = new PhysicTile(Point2d(new_width2,new_height2),
				       Point2d(m_offset.x + new_width1, m_offset.y+ new_height1),
				       m_tile_offset, m_parent_tile, m_parent_physic_tile, m_level -1);


    // Group some physic tile together if it's possible (WARNING: we must not create concav polygon such as L)
    if (m_physic_tiles[0]->m_fullness == FULL &&
	m_physic_tiles[1]->m_fullness == FULL) {

      //std::cout<<"PhysicTile::m_fullness regroup 0&1" << std::endl;

      ASSERT(m_physic_tiles[0]->m_is_containing_polygon);
      ASSERT(!m_physic_tiles[0]->m_is_subdivided);
      ASSERT(m_physic_tiles[1]->m_is_containing_polygon);
      ASSERT(!m_physic_tiles[1]->m_is_subdivided);
      delete m_physic_tiles[1];
      m_physic_tiles[1] = NULL;

      //std::cout<<"PhysicTile::m_fullness = FULL, level"<<m_level<<std::endl;
      PhysicalPolygon* shape = new PhysicalPolygon();
      shape->SetBody(m_parent_tile->GetBody());

      shape->AddPoint(Point2d((double(m_offset.x + m_tile_offset.x) / PIXEL_PER_METER),
			      (double(m_offset.y + m_tile_offset.y)/ PIXEL_PER_METER)));

      shape->AddPoint(Point2d((double(m_offset.x + m_tile_offset.x + m_size.x-1)/ PIXEL_PER_METER),
			      (double(m_offset.y + m_tile_offset.y)/ PIXEL_PER_METER)));

      shape->AddPoint(Point2d((double(m_offset.x + m_tile_offset.x + m_size.x-1)/ PIXEL_PER_METER),
			      (double(m_offset.y + m_tile_offset.y + new_height1)/ PIXEL_PER_METER)));

      shape->AddPoint(Point2d((double(m_offset.x + m_tile_offset.x)/ PIXEL_PER_METER),
			      (double(m_offset.y + m_tile_offset.y + new_height1)/ PIXEL_PER_METER)));

      shape->SetFriction(GROUND_FRICTION);

      b2FilterData filter_data;
      filter_data.categoryBits = 0x000B;
      filter_data.maskBits = 0xFFFF;
      shape->SetFilter(filter_data);

      shape->Generate();
      delete (m_physic_tiles[0]->m_shape);
      m_physic_tiles[0]->m_shape = shape;

    } else if (m_physic_tiles[2]->m_fullness == FULL &&
	       m_physic_tiles[3]->m_fullness == FULL) {
      //std::cout<<"PhysicTile::m_fullness regroup 2&3" << std::endl;

      ASSERT(m_physic_tiles[2]->m_is_containing_polygon);
      ASSERT(!m_physic_tiles[2]->m_is_subdivided);
      ASSERT(m_physic_tiles[3]->m_is_containing_polygon);
      ASSERT(!m_physic_tiles[3]->m_is_subdivided);
      delete m_physic_tiles[3];
      m_physic_tiles[3] = NULL;

      //std::cout<<"PhysicTile::m_fullness = FULL, level"<<m_level<<std::endl;
      PhysicalPolygon* shape = new PhysicalPolygon();
      shape->SetBody(m_parent_tile->GetBody());

      shape->AddPoint(Point2d((double(m_offset.x + m_tile_offset.x) / PIXEL_PER_METER),
			      (double(m_offset.y + m_tile_offset.y + new_height1)/ PIXEL_PER_METER)));

      shape->AddPoint(Point2d((double(m_offset.x + m_tile_offset.x + m_size.x-1)/ PIXEL_PER_METER),
			      (double(m_offset.y + m_tile_offset.y + new_height1)/ PIXEL_PER_METER)));

      shape->AddPoint(Point2d((double(m_offset.x + m_tile_offset.x + m_size.x-1)/ PIXEL_PER_METER),
			      (double(m_offset.y + m_tile_offset.y + m_size.y-1)/ PIXEL_PER_METER)));

      shape->AddPoint(Point2d((double(m_offset.x + m_tile_offset.x)/ PIXEL_PER_METER),
			      (double(m_offset.y + m_tile_offset.y + m_size.y-1)/ PIXEL_PER_METER)));

      shape->SetFriction(GROUND_FRICTION);

      b2FilterData filter_data;
      filter_data.categoryBits = 0x000B;
      filter_data.maskBits = 0xFFFF;
      shape->SetFilter(filter_data);

      shape->Generate();
      delete (m_physic_tiles[2]->m_shape);
      m_physic_tiles[2]->m_shape = shape;

    } else if (m_physic_tiles[0]->m_fullness == FULL &&
	       m_physic_tiles[2]->m_fullness == FULL) {

      //std::cout<<"PhysicTile::m_fullness regroup 0&2" << std::endl;

      ASSERT(m_physic_tiles[0]->m_is_containing_polygon);
      ASSERT(!m_physic_tiles[0]->m_is_subdivided);
      ASSERT(m_physic_tiles[2]->m_is_containing_polygon);
      ASSERT(!m_physic_tiles[2]->m_is_subdivided);
      delete m_physic_tiles[2];
      m_physic_tiles[2] = NULL;

      //std::cout<<"PhysicTile::m_fullness = FULL, level"<<m_level<<std::endl;
      PhysicalPolygon* shape = new PhysicalPolygon();
      shape->SetBody(m_parent_tile->GetBody());

      shape->AddPoint(Point2d((double(m_offset.x + m_tile_offset.x) / PIXEL_PER_METER),
			      (double(m_offset.y + m_tile_offset.y) / PIXEL_PER_METER)));

      shape->AddPoint(Point2d((double(m_offset.x + m_tile_offset.x + new_width1)/ PIXEL_PER_METER),
			      (double(m_offset.y + m_tile_offset.y) / PIXEL_PER_METER)));

      shape->AddPoint(Point2d((double(m_offset.x + m_tile_offset.x + new_width1)/ PIXEL_PER_METER),
			      (double(m_offset.y + m_tile_offset.y + m_size.y-1)/ PIXEL_PER_METER)));

      shape->AddPoint(Point2d((double(m_offset.x + m_tile_offset.x)/ PIXEL_PER_METER),
			      (double(m_offset.y + m_tile_offset.y + m_size.y-1)/ PIXEL_PER_METER)));

      shape->SetFriction(GROUND_FRICTION);

      b2FilterData filter_data;
      filter_data.categoryBits = 0x000B;
      filter_data.maskBits = 0xFFFF;
      shape->SetFilter(filter_data);

      shape->Generate();
      delete (m_physic_tiles[0]->m_shape);
      m_physic_tiles[0]->m_shape = shape;

    } else if (m_physic_tiles[1]->m_fullness == FULL &&
	       m_physic_tiles[3]->m_fullness == FULL) {
      //std::cout<<"PhysicTile::m_fullness regroup 0&2" << std::endl;

      ASSERT(m_physic_tiles[1]->m_is_containing_polygon);
      ASSERT(!m_physic_tiles[1]->m_is_subdivided);
      ASSERT(m_physic_tiles[3]->m_is_containing_polygon);
      ASSERT(!m_physic_tiles[3]->m_is_subdivided);
      delete m_physic_tiles[3];
      m_physic_tiles[3] = NULL;

      //std::cout<<"PhysicTile::m_fullness = FULL, level"<<m_level<<std::endl;
      PhysicalPolygon* shape = new PhysicalPolygon();
      shape->SetBody(m_parent_tile->GetBody());

      shape->AddPoint(Point2d((double(m_offset.x + m_tile_offset.x + new_width1) / PIXEL_PER_METER),
			      (double(m_offset.y + m_tile_offset.y) / PIXEL_PER_METER)));

      shape->AddPoint(Point2d((double(m_offset.x + m_tile_offset.x + m_size.x-1)/ PIXEL_PER_METER),
			      (double(m_offset.y + m_tile_offset.y) / PIXEL_PER_METER)));

      shape->AddPoint(Point2d((double(m_offset.x + m_tile_offset.x + m_size.x-1)/ PIXEL_PER_METER),
			      (double(m_offset.y + m_tile_offset.y + m_size.y-1)/ PIXEL_PER_METER)));

      shape->AddPoint(Point2d((double(m_offset.x + m_tile_offset.x + new_width1)/ PIXEL_PER_METER),
			      (double(m_offset.y + m_tile_offset.y + m_size.y-1)/ PIXEL_PER_METER)));

      shape->SetFriction(GROUND_FRICTION);

      b2FilterData filter_data;
      filter_data.categoryBits = 0x000B;
      filter_data.maskBits = 0xFFFF;
      shape->SetFilter(filter_data);

      shape->Generate();
      delete (m_physic_tiles[1]->m_shape);
      m_physic_tiles[1]->m_shape = shape;
    }
  }
}

//Home made algorithm
/*

a  0->            <-1  b
 +-------------------+
7¦      e     f      ¦2
|¦      ¦     ¦      ¦|
V¦l-----+-----+-----g¦V
 ¦      ¦     ¦      ¦
 ¦      ¦     ¦      ¦
 ¦      ¦     ¦      ¦
^¦------+-----+-----h¦^
|¦      ¦     ¦      ¦|
6¦      j     i      ¦3
 +-------------------+
d  5->            <-4  c


*/


bool PhysicTile::GeneratePolygone()
{
  m_is_containing_polygon = false;
  //8 verticles max
  Point2i pts[8];
  bool pts_state[8];
  for (uint i=0; i < 8; i++) {
      pts_state[i] = false;
  }

  // pts 0 in 1;
  //std::cout<<"PhysicTile::GeneratePolygone"<<std::endl;


  //(0) search ground between (a) and (b)
  pts[0] = m_offset;
  while ((pts[0].x < m_size.x + m_offset.x) && !pts_state[0]) {
    if (m_parent_tile->GetAlpha(pts[0])!= SDL_ALPHA_TRANSPARENT) {
      pts_state[0] = true; //lock position
    } else {
      pts[0].x++;
    }
  }
  // std::cout<<"PhysicTile::pt1 done"<<std::endl;

  //if (0) found ground, send (1) from (b) to (a)
  if (pts_state[0]) {
    pts[1] = m_offset;
    pts[1].x += m_size.x - 1;
    while ((pts[1].x >= m_offset.x) && !pts_state[1]) {
      if (m_parent_tile->GetAlpha(pts[1]) != SDL_ALPHA_TRANSPARENT) {
        pts_state[1] = true; //lock position
      } else {
        pts[1].x--;
      }
    }
  }
  // std::cout<<"PhysicTile::pt2 done"<<std::endl;

  //(2) search ground between (b) and (c)
  pts[2] = m_offset;
  pts[2].x += m_size.x-1;
  while ((pts[2].y < m_size.y + m_offset.y) && !pts_state[2]) {
    if (m_parent_tile->GetAlpha(pts[2]) != SDL_ALPHA_TRANSPARENT) {
      pts_state[2] = true; //lock position
    } else {
      pts[2].y++;
    }
  }
  //std::cout<<"PhysicTile::pt3 done"<<std::endl;

  // if (2) found ground, send (3) from (c) to (b)
  if (pts_state[2]) {
    pts[3] = m_offset;
    pts[3].x += m_size.x-1;
    pts[3].y += m_size.y-1;
    while ((pts[3].y >= m_offset.y) && !pts_state[3]) {
      if (m_parent_tile->GetAlpha(pts[3]) != SDL_ALPHA_TRANSPARENT) {
        pts_state[3] = true; //lock position
      } else {
        pts[3].y--;
      }
    }
  }
  //std::cout<<"PhysicTile::pt4 done"<<std::endl;

  //(4) search ground between (c) and (d)
  pts[4] = m_offset + m_size;
  pts[4].x--;
  pts[4].y--;
  while ((pts[4].x >= m_offset.x) && !pts_state[4]) {
    if (m_parent_tile->GetAlpha(pts[4])!= SDL_ALPHA_TRANSPARENT){
      pts_state[4] = true; //lock position
    } else {
      pts[4].x--;
    }
  }
  //std::cout<<"PhysicTile::pt5 done"<<std::endl;

  //if (4) found ground, send (5) from (d) to (c)
  if(pts_state[4]){
    pts[5] = m_offset;
    pts[5].y += m_size.y-1;
    while ((pts[5].x < m_size.x+m_offset.x) && !pts_state[5]) {
      if (m_parent_tile->GetAlpha(pts[5]) != SDL_ALPHA_TRANSPARENT) {
        pts_state[5] = true; //lock position
      } else {
        pts[5].x++;
      }
    }
  }
  //std::cout<<"PhysicTile::pt6 done"<<std::endl;

  //(6) search ground between (d) and (a)
  pts[6] = m_offset;
  pts[6].y += m_size.y-1;
  while ((pts[6].y >= m_offset.y ) && !pts_state[6]) {
    if (m_parent_tile->GetAlpha(pts[6]) != SDL_ALPHA_TRANSPARENT) {
      pts_state[6] = true; //lock position
    } else {
      pts[6].y--;
    }
  }
  //std::cout<<"PhysicTile::pt7 done"<<std::endl;
  //if (6) found ground, send (7) from (a) to (d)
  if (pts_state[6]) {
    pts[7] = m_offset;
    while ((pts[7].y < m_size.y+m_offset.y ) && !pts_state[7]) {
      if (m_parent_tile->GetAlpha(pts[7]) != SDL_ALPHA_TRANSPARENT) {
        pts_state[7] = true; //lock position
      } else {
        pts[7].y++;
      }
    }
  }
 // std::cout<<"PhysicTile::pt8 done"<<std::endl;

  //TODO : Second pass
  if (!pts_state[0]) {
    //(0) search ground between (e) and (j)
    pts[0] = m_offset;
    pts[0].x += m_size.x/3;
    while ((pts[2].y <  m_size.y+m_offset.y ) && !pts_state[2]) {
      if (m_parent_tile->GetAlpha(pts[2]) != SDL_ALPHA_TRANSPARENT) {
	pts_state[2] = true; //lock position
      } else {
	pts[2].y++;
      }
    }
  }

  //Stop to try to place points and now generate the polygone
  b2PolygonDef rect;
  rect.vertexCount = 0;

  // Delete double point
  for (uint i=0; i < 7 ; i++) {

    if (pts_state[i]) {

      for (uint j=i+1; j < 8; j++) {

        if (pts_state[j]) {

	  /*if((pts[i].x == pts[j].x)&&(pts[i].y == pts[j].y)) {
            pts_state[j] = false;
           }*/

	  if (pts[i].Distance(pts[j]) < 3) {
            pts_state[j] = false;
	  }

        }
      }
    }
  }


  //Count found point
  for (uint i=0; i < 8; i++) {
    if (pts_state[i]) {
      rect.vertexCount++;
    }
  }

  Point2d min_pixel(0,0);
  Point2d max_pixel(0,0);
 
  if (rect.vertexCount > 2) {

    //Verify
    if (m_level > -1) {

      for (uint i=0; i<8; i++) {
	    if (pts_state[i]) {
	      for (uint j=i; j < 8; j++) {
	        if (pts_state[j]) {
	          if (m_parent_tile->GetAlpha(Point2i(m_offset.x+abs(pts[i].x-pts[j].x),
						  m_offset.y+abs(pts[i].y-pts[j].y))) == SDL_ALPHA_TRANSPARENT) {
		        return false;
	          }
	        }
	      }
	    }
      }
    }

    m_is_containing_polygon = true;

    PhysicalPolygon* shape = new PhysicalPolygon();
    shape->SetBody(m_parent_tile->GetBody());


    for (uint i=0; i < 8; i++) {
      if (pts_state[i]) {
	//std::cout<<"PhysicTile::Add pt"<<i<<" x "<<pts[i].x<<" y "<<pts[i].y<<std::endl;
	//std::cout<<"PhysicTile::Add ph"<<i<<" x "<<pts[i].x+m_tile_offset.x<<" y "<<pts[i].y+m_tile_offset.y<<std::endl;

	    Point2d new_point((double(pts[i].x + m_tile_offset.x) / PIXEL_PER_METER),
				(double(pts[i].y + m_tile_offset.y) / PIXEL_PER_METER));
        shape->AddPoint(new_point);

		if(new_point.x < min_pixel.x)
		{
			min_pixel.x = new_point.x;
		}
		if(new_point.y < min_pixel.y)
		{
			min_pixel.y = new_point.y;
		}
		if(new_point.x > max_pixel.x)
		{
			max_pixel.x = new_point.x;
		}
		if(new_point.y < max_pixel.y)
		{
			max_pixel.y = new_point.y;
		}
		}
      }
    

	if ((max_pixel.x - min_pixel.x > 0.00001) || (max_pixel.y - min_pixel.y > 0.00001)) {
	Point2d min_pixel(0,0);
      shape->SetFriction(GROUND_FRICTION);
  
      b2FilterData filter_data;
      filter_data.categoryBits = 0x0004; // Why this is different than upper ??
      filter_data.maskBits = 0xFFFB; // Why this is different than upper ??
      shape->SetFilter(filter_data);
  
      shape->Generate();
      m_shape = shape;
	} else {
	  delete shape;
	  m_is_containing_polygon = false;
	}

  } else {
    return true;
  }

  return true;


}

void PhysicTile::Clean()
{
  if (m_is_subdivided) {
    delete m_physic_tiles[0];
    delete m_physic_tiles[1];
    delete m_physic_tiles[2];
    delete m_physic_tiles[3];
    m_physic_tiles[0] =NULL;
    m_physic_tiles[1] =NULL;
    m_physic_tiles[2] =NULL;
    m_physic_tiles[3] =NULL;
    m_is_subdivided = false;
  }

  if (m_is_containing_polygon) {
    delete m_shape;
    m_shape = NULL;
    m_is_containing_polygon = false;
  }
}

PhysicTile::Fullness PhysicTile::IsFull() const
{
  Fullness fullness = EMPTY;

  bool is_empty = true;
  bool is_full = true;

  for (int i = 0; i < m_size.x; i++) {
    for (int j = 0; j < m_size.y; j++) {
        if (m_parent_tile->GetAlpha(Point2i(i+m_offset.x,j+m_offset.y)) == SDL_ALPHA_TRANSPARENT) {
            is_full = false;
        } else {
            is_empty = false;
        }
    }

    if (!is_empty && !is_full) {
      fullness = MIXTE;
      return fullness;
    }
  }

  if (is_full) {
    fullness = FULL;
  }

  return fullness;
}

void PhysicTile::InitShape()
{
  PhysicalPolygon* shape = new PhysicalPolygon();
  shape->SetBody(m_parent_tile->GetBody());

  shape->SetFriction(GROUND_FRICTION);

  b2FilterData filter_data;
  filter_data.categoryBits = 0x0004; // Why this is different than upper ??
  filter_data.maskBits = 0xFFFB; // Why this is different than upper ??
  shape->SetFilter(filter_data);

  m_shape = shape;
}


#ifdef DEBUG
void PhysicTile::DrawBorder(const Color& color) const
{
  if (!m_is_subdivided && m_is_containing_polygon) {
    ASSERT(m_shape != NULL);
    m_shape->DrawBorder(color);
  } else if (m_is_subdivided) {
    for (uint i=0; i < 4; i++) {
      if (m_physic_tiles[i] != NULL)
	m_physic_tiles[i]->DrawBorder(color);
    }
  }
}
#endif
