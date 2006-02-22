/******************************************************************************
 *  Wormux, a free clone of the game Worms from Team17.
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
 *****************************************************************************/

#include "tile.h"
#include "../include/app.h"
#include "../map/camera.h"

// = Tile implementation
Tile::Tile(){
}

void Tile::FreeMem(){
    for (uint i=0; i<nbr_cell; ++i)
        delete item[i];
    nbr_cell = 0;
    item.clear();
}

Tile::~Tile(){ 
    FreeMem();
}

void Tile::InitTile(const Point2i &pSize){
    nbCells = pSize / CELL_SIZE;

    if( (pSize.x % CELL_SIZE.x) != 0 )
        nbCells.x++;

    if( (pSize.y % CELL_SIZE.y) != 0 )
        nbCells.y++;

    size = pSize;

    nbr_cell = nbCells.x * nbCells.y;
}

int clamp (const int val, const int min, const int max){   
    return ( val > max ) ? max : ( val < min ) ? min : val ;
}

void Tile::Dig(const Point2i &position, Surface& dig){  
    Rectanglei rect = Rectanglei(position, dig.GetSize()); 

    int first_cell_x = clamp( position.x/CELL_SIZE.x,          0, nbCells.x-1);
    int first_cell_y = clamp( position.y/CELL_SIZE.y,          0, nbCells.y-1);
    int last_cell_x  = clamp( (position.x+dig.GetWidth())/CELL_SIZE.x, 0, nbCells.x-1);
    int last_cell_y  = clamp( (position.y+dig.GetHeight())/CELL_SIZE.y, 0, nbCells.y-1);

    for( int cy = first_cell_y ; cy <= last_cell_y ; cy++ )
        for ( int cx = first_cell_x ; cx <= last_cell_x ; cx++){
            Point2i offset = position - Point2i(cx, cy) * CELL_SIZE;

            item[cy*nbCells.x + cx]->Dig(offset, dig);
        }
}

void Tile::LoadImage (Surface& terrain){
    FreeMem();

    InitTile(terrain.GetSize());
    assert(nbr_cell != 0);

    // Create the TileItem objects
    for (uint i=0; i<nbr_cell; ++i)
        if ( config.transparency == Config::ALPHA )
            item.push_back ( new TileItem_AlphaSoftware(CELL_SIZE) );
        else
            item.push_back ( new TileItem_ColorkeySoftware(CELL_SIZE) );

    // Fill the TileItem objects
    for( int iy = 0; iy < nbCells.y; iy++ )
        for( int ix = 0; ix < nbCells.x; ix++ ){
            int piece = iy * nbCells.x + ix;

            Rectanglei sr( Point2i(ix, iy) * CELL_SIZE, CELL_SIZE);

            terrain.SetAlpha(0, 0);
            item[piece]->GetSurface().Blit(terrain, sr, Point2i(0, 0));
            item[piece]->SyncBuffer();
        }

    // Replace transparent tiles by TileItem_Empty tiles
    for( uint i=0; i<nbr_cell; ++i )
        if( item[i]->IsEmpty() ){
            delete item[i];
            item[i] = (TileItem*)new TileItem_Empty;
        }
}

uchar Tile::GetAlpha(const Point2i &pos) const{
	int cell = pos.y / CELL_SIZE.y * nbCells.x + pos.x / CELL_SIZE.x;
    return item[cell]->GetAlpha(pos % CELL_SIZE);
}

void Tile::DrawTile() const{
   Point2i camPos = camera.GetPosition();

   int first_cell_x = clamp( camPos.x/CELL_SIZE.x,                      0, nbCells.x-1);
   int first_cell_y = clamp( camPos.y/CELL_SIZE.y,                      0, nbCells.y-1);
   int last_cell_x  = clamp( (camPos.x+camera.GetWidth())/CELL_SIZE.x,  0, nbCells.x-1);
   int last_cell_y  = clamp( (camPos.y+camera.GetHeight())/CELL_SIZE.y, 0, nbCells.y-1);  

    for ( int iy = first_cell_y ; iy <= last_cell_y ; iy++ )
        for ( int ix = first_cell_x ; ix <= last_cell_x ; ix++)
            item[iy*nbCells.x+ix]->Draw( Point2i(ix, iy) );
}

void Tile::DrawTile_Clipped(Rectanglei clip_r_world) const
{
   // Select only the items that are under the clip area
   int first_cell_x = clamp( (clip_r_world.GetPositionX())/CELL_SIZE.x,                0, nbCells.x-1);
   int first_cell_y = clamp( (clip_r_world.GetPositionY())/CELL_SIZE.y,                0, nbCells.y-1);
   int last_cell_x  = clamp( (clip_r_world.GetPositionX()+clip_r_world.GetSizeX() +1)/CELL_SIZE.x, 0, nbCells.x-1);
   int last_cell_y  = clamp( (clip_r_world.GetPositionY()+clip_r_world.GetSizeY() +1)/CELL_SIZE.y, 0, nbCells.y-1);

    for( int cy = first_cell_y ; cy <= last_cell_y ; cy++ )
        for ( int cx = first_cell_x ; cx <= last_cell_x ; cx++){
            // For all selected items, clip source and destination blitting rectangles 
            int dest_x = cx * CELL_SIZE.x;  
            int dest_y = cy * CELL_SIZE.y;
            int dest_w = CELL_SIZE.x;
            int dest_h = CELL_SIZE.y;
            int src_x = 0;
            int src_y = 0;

            if ( dest_x < clip_r_world.GetPositionX() ){ // left clipping
                    src_x  += clip_r_world.GetPositionX() - dest_x;
                    dest_w -= clip_r_world.GetPositionX() - dest_x;
                    dest_x  = clip_r_world.GetPositionX();
                }

                if ( dest_y < clip_r_world.GetPositionY() ){  // top clipping
                    src_y  += clip_r_world.GetPositionY() - dest_y;
                    dest_h -= clip_r_world.GetPositionY() - dest_y;
                    dest_y  = clip_r_world.GetPositionY();
                }

                if ( dest_x + dest_w > clip_r_world.GetPositionX() + clip_r_world.GetSizeX() +1) // right clipping
                    dest_w -= ( dest_x + dest_w ) - ( clip_r_world.GetPositionX() + clip_r_world.GetSizeX() +1);

                if ( dest_y + dest_h > clip_r_world.GetPositionY() + clip_r_world.GetSizeY() +1) // bottom clipping
                    dest_h -= ( dest_y + dest_h ) - ( clip_r_world.GetPositionY() + clip_r_world.GetSizeY() +1);

                Rectanglei sr(src_x, src_y, dest_w, dest_h);

                // Decall the destination rectangle along the camera offset
                Point2i dr( Point2i(dest_x, dest_y) - camera.GetPosition());

                app.video.window.Blit( item[cy*nbCells.x+cx]->GetSurface(), sr, dr);
        }
}
