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

#include "../tool/geometry_tools.h"
//-----------------------------------------------------------------------------

// Est-ce qu'un point est dans un rectangle ?
bool DansRect (const CL_Rect &rect, const CL_Point &p)
{
  if (p.x < rect.left) return false;
  if (rect.right < p.x) return false;
  if (p.y < rect.top) return false;
  if (rect.bottom < p.y) return false;
  return true;
}

//-----------------------------------------------------------------------------

// Est-ce que le rectangle a contient le rectangle a ?
bool Contient (const CL_Rect &a, const CL_Rect &b)
{
  return ((a.left <= b.left) 
	  && (b.right <= a.right)
	  && (a.top <= b.top) 
	  && (b.bottom <= a.bottom));
}

//-----------------------------------------------------------------------------

// Est-ce que les deux rectangles se touchent ?
bool RectTouche (const CL_Rect &a, const CL_Rect &b)
{
  return DansRect(a, CL_Point(b.left,b.top))
    || DansRect(a, CL_Point(b.left,b.bottom))
    || DansRect(a, CL_Point(b.right,b.top))
    || DansRect(a, CL_Point(b.right,b.bottom));
}

//-----------------------------------------------------------------------------

// Est-ce que deux objets se touchent ? (utilise les rectangles de test)
bool ObjTouche (const PhysicalObj &a, const PhysicalObj &b)
{
  return RectTouche (a.GetTestRect(), b.GetTestRect());
}

//-----------------------------------------------------------------------------

// Est-ce que le point p touche l'objet a ?
bool ObjTouche (const PhysicalObj &a, const CL_Point &p)
{
  return DansRect (a.GetTestRect(), p);
}

//-----------------------------------------------------------------------------

// Distance entre deux points
double Distance (const CL_Point &p1, const CL_Point &p2)
{
  return hypot (p2.x - p1.x, p2.y - p1.y);
}

//-----------------------------------------------------------------------------

// Distance entre deux points
double Distance (double x1, double y1, double x2, double y2)
{
  return hypot (x2-x1, y2-y1);
}

//-----------------------------------------------------------------------------

void MoveRect (CL_Rect &r, int dx, int dy)
{
  r.left += dx;
  r.right += dx;
  r.top += dy;
  r.bottom += dy;
}

//-----------------------------------------------------------------------------
