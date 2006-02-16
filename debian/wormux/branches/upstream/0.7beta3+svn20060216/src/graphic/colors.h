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
 * Colors.
 ******************************************************************************/

#ifndef COLORS_H
#define COLORS_H

#include <SDL_video.h>
#include "color.h"

// Greyscale
const Color white_color (255, 255, 255, SDL_ALPHA_OPAQUE);
const Color lightgray_color (169, 169, 169, SDL_ALPHA_OPAQUE);
const Color gray_color (128, 128, 128, SDL_ALPHA_OPAQUE);
const Color black_color (0, 0, 0, SDL_ALPHA_OPAQUE);

// Green
const Color green_color (68, 120, 51, SDL_ALPHA_OPAQUE);

const Color primary_red_color(255, 0, 0, SDL_ALPHA_OPAQUE);

// Default colors:
const Color defaultColorBox(80, 80, 159, 206);
const Color defaultColorRect(49, 32, 122, 255);

const Color defaultListColor1(255, 255, 255, 255*3/10);
const Color defaultListColor2(0, 0, 255*6/10, 255*8/10);
const Color defaultListColor3(0, 0, 255*6/10, 255*4/10);


const Color WeaponStrengthBarValue(255, 255, 255, 127);
const Color WeaponStrengthBarBorder(0, 0, 0, 127);
const Color WeaponStrengthBarBackground(255*6/10, 255*6/10, 255*6/10, 96);


#endif
