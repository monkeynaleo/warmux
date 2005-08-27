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
 ******************************************************************************
 * Utilitaires de calculs sur des rectangles et des points.
 *****************************************************************************/

#ifndef UTILITAIRE_RECT_H
#define UTILITAIRE_RECT_H
//-----------------------------------------------------------------------------
#include "../include/base.h"
#include "../object/physical_obj.h"
#include <ClanLib/core.h>
//-----------------------------------------------------------------------------

// Distance entre deux points
double Distance (const CL_Point &p1, const CL_Point &p2);
double Distance (double x1, double y1, double x2, double y2);

// Est-ce qu'un point est dans un rectangle ?
bool DansRect (const CL_Rect &rect, const CL_Point &p);

// Est-ce que le rectangle a contient le rectangle a ?
bool Contient (const CL_Rect &a, const CL_Rect &b);

// Est-ce que les deux rectangles se touchent ?
bool RectTouche (const CL_Rect &a, const CL_Rect &b);

// Est-ce que deux objets se touchent ? (utilise les rectangles de test)
bool ObjTouche (const PhysicalObj &a, const PhysicalObj &b);

// Est-ce que le point p touche l'objet a ?
bool ObjTouche (const PhysicalObj &a, const CL_Point &p);

// Bouge un rectangle
void MoveRect (CL_Rect &r, int dx, int dy);

//-----------------------------------------------------------------------------
#endif
