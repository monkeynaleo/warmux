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
 * Handle a SDL_Surface.
 *****************************************************************************/

#ifndef COLOR_H
#define COLOR_H

#include <SDL.h>

class Color
{
	private:

	public:
		Uint8 red;
		Uint8 green;
		Uint8 blue;
		Uint8 alpha;
		
		Color();
		Color(Uint8 r, Uint8 g, Uint8 b, Uint8 a);

		bool operator==(const Color &color) const;

		void SetColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a);

		Uint8 GetRed() const;
		Uint8 GetGreen() const;
		Uint8 GetBlue() const;
		Uint8 GetAlpha() const;

		SDL_Color GetSDLColor() const;
};

#endif
