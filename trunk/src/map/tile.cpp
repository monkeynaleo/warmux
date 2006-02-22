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

Point2i Tile::clamp(const Point2i &v) const{
	return v.clamp(Point2i(0, 0), nbCells - 1);
}

void Tile::Dig(const Point2i &position, Surface& dig){  
    Rectanglei rect = Rectanglei(position, dig.GetSize());
	Point2i firstCell = clamp(position/CELL_SIZE);
	Point2i lastCell = clamp((position + dig.GetSize())/CELL_SIZE);
	Point2i c;

    for( c.y = firstCell.y; c.y <= lastCell.y; c.y++ )
        for( c.x = firstCell.x; c.x <= lastCell.x; c.x++){
            Point2i offset = position - c * CELL_SIZE;

            item[c.y*nbCells.x + c.x]->Dig(offset, dig);
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
    Point2i firstCell = clamp(camera.GetPosition() / CELL_SIZE);
    Point2i lastCell = clamp((camera.GetPosition() + camera.GetSize()) / CELL_SIZE);
	Point2i i;

    for( i.y = firstCell.y; i.y <= lastCell.y; i.y++ )
        for( i.x = firstCell.x; i.x <= lastCell.x; i.x++)
            item[i.y*nbCells.x + i.x]->Draw( i );
}

void Tile::DrawTile_Clipped(Rectanglei clip_r_world) const
{
    Point2i firstCell = clamp(clip_r_world.GetPosition() / CELL_SIZE);
    Point2i lastCell  = clamp((clip_r_world.GetPosition() + clip_r_world.GetSize() + 1) / CELL_SIZE);
	Point2i c;

    for( c.y = firstCell.y; c.y <= lastCell.y; c.y++ )
        for( c.x = firstCell.x; c.x <= lastCell.x; c.x++){
            // For all selected items, clip source and destination blitting rectangles 
			Point2i destPos = c * CELL_SIZE;
			Point2i destSize = CELL_SIZE;
			Point2i src;

            if ( destPos.x < clip_r_world.GetPositionX() ){ // left clipping
                    src.x  += clip_r_world.GetPositionX() - destPos.x;
                    destSize.x -= clip_r_world.GetPositionX() - destPos.x;
                    destPos.x  = clip_r_world.GetPositionX();
                }

                if ( destPos.y < clip_r_world.GetPositionY() ){  // top clipping
                    src.y  += clip_r_world.GetPositionY() - destPos.y;
                    destSize.y -= clip_r_world.GetPositionY() - destPos.y;
                    destPos.y  = clip_r_world.GetPositionY();
                }

                if ( destPos.x + destSize.x > clip_r_world.GetPositionX() + clip_r_world.GetSizeX() +1) // right clipping
                    destSize.x -= ( destPos.x + destSize.x ) - ( clip_r_world.GetPositionX() + clip_r_world.GetSizeX() +1);

                if ( destPos.y + destSize.y > clip_r_world.GetPositionY() + clip_r_world.GetSizeY() +1) // bottom clipping
                    destSize.y -= ( destPos.y + destSize.y ) - ( clip_r_world.GetPositionY() + clip_r_world.GetSizeY() +1);

                Rectanglei sr(src, destSize);

                // Decall the destination rectangle along the camera offset
                Point2i dr( destPos - camera.GetPosition());

                app.video.window.Blit( item[c.y*nbCells.x + c.x]->GetSurface(), sr, dr); 
        }
}
