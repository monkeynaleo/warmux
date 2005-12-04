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
 *
 * Color definitions taken from the Clanlib librarie, and then modified
 * to fit with SDL.
 * See there license below.
 *
 ******************************************************************************/

/*
**  ClanLib SDK
**  Copyright (c) 1997-2005 The ClanLib Team
**
**  This software is provided 'as-is', without any express or implied
**  warranty.  In no event will the authors be held liable for any damages
**  arising from the use of this software.
**
**  Permission is granted to anyone to use this software for any purpose,
**  including commercial applications, and to alter it and redistribute it
**  freely, subject to the following restrictions:
**
**  1. The origin of this software must not be misrepresented; you must not
**     claim that you wrote the original software. If you use this software
**     in a product, an acknowledgment in the product documentation would be
**     appreciated but is not required.
**  2. Altered source versions must be plainly marked as such, and must not be
**     misrepresented as being the original software.
**  3. This notice may not be removed or altered from any source distribution.
**
**  Note: Some of the libraries ClanLib may link to may have additional
**  requirements or restrictions.
**
**  File Author(s):
**
**    Magnus Norddahl
**    (if your name is missing here, please add it)
*/


#ifndef COLORS_H
#define COLORS_H

#include <SDL/SDL_video.h>

const SDL_Color aliceblue_color = { 240, 248, 255, SDL_ALPHA_OPAQUE };
const SDL_Color antiquewhite_color = { 250, 235, 215, SDL_ALPHA_OPAQUE };
const SDL_Color aqua_color = {  0, 255, 255, SDL_ALPHA_OPAQUE };
const SDL_Color aquamarine_color = { 127, 255, 212, SDL_ALPHA_OPAQUE };
const SDL_Color azure_color = { 240, 255, 255, SDL_ALPHA_OPAQUE };
const SDL_Color beige_color = { 245, 245, 220, SDL_ALPHA_OPAQUE };
const SDL_Color bisque_color = { 255, 228, 196, SDL_ALPHA_OPAQUE };
const SDL_Color black_color = {  0, 0, 0, SDL_ALPHA_OPAQUE };
const SDL_Color blanchedalmond_color = { 255, 235, 205, SDL_ALPHA_OPAQUE };
const SDL_Color blue_color = {  0, 0, 255, SDL_ALPHA_OPAQUE };
const SDL_Color blueviolet_color = { 138, 43, 226, SDL_ALPHA_OPAQUE };
const SDL_Color brown_color = { 165, 42, 42, SDL_ALPHA_OPAQUE };
const SDL_Color burlywood_color = { 222, 184, 135, SDL_ALPHA_OPAQUE };
const SDL_Color cadetblue_color = {  95, 158, 160, SDL_ALPHA_OPAQUE };
const SDL_Color chartreuse_color = { 127, 255, 0, SDL_ALPHA_OPAQUE };
const SDL_Color chocolate_color = { 210, 105, 30, SDL_ALPHA_OPAQUE };
const SDL_Color coral_color = { 255, 127, 80, SDL_ALPHA_OPAQUE };
const SDL_Color cornflowerblue_color = { 100, 149, 237, SDL_ALPHA_OPAQUE };
const SDL_Color cornsilk_color = { 255, 248, 220, SDL_ALPHA_OPAQUE };
const SDL_Color crimson_color = { 220, 20, 60, SDL_ALPHA_OPAQUE };
const SDL_Color cyan_color = {  0, 255, 255, SDL_ALPHA_OPAQUE };
const SDL_Color darkblue_color = {  0, 0, 139, SDL_ALPHA_OPAQUE };
const SDL_Color darkcyan_color = {  0, 139, 139, SDL_ALPHA_OPAQUE };
const SDL_Color darkgoldenrod_color = { 184, 134, 11, SDL_ALPHA_OPAQUE };
const SDL_Color darkgray_color = { 169, 169, 169, SDL_ALPHA_OPAQUE };
const SDL_Color darkgreen_color = {  0, 100, 0, SDL_ALPHA_OPAQUE };
const SDL_Color darkgrey_color = { 169, 169, 169, SDL_ALPHA_OPAQUE };
const SDL_Color darkkhaki_color = { 189, 183, 107, SDL_ALPHA_OPAQUE };
const SDL_Color darkmagenta_color = { 139, 0, 139, SDL_ALPHA_OPAQUE };
const SDL_Color darkolivegreen_color = {  85, 107, 47, SDL_ALPHA_OPAQUE };
const SDL_Color darkorange_color = { 255, 140, 0, SDL_ALPHA_OPAQUE };
const SDL_Color darkorchid_color = { 153, 50, 204, SDL_ALPHA_OPAQUE };
const SDL_Color darkred_color = { 139, 0, 0, SDL_ALPHA_OPAQUE };
const SDL_Color darksalmon_color = { 233, 150, 122, SDL_ALPHA_OPAQUE };
const SDL_Color darkseagreen_color = { 143, 188, 143, SDL_ALPHA_OPAQUE };
const SDL_Color darkslateblue_color = {  72, 61, 139, SDL_ALPHA_OPAQUE };
const SDL_Color darkslategray_color = {  47, 79, 79, SDL_ALPHA_OPAQUE };
const SDL_Color darkslategrey_color = {  47, 79, 79, SDL_ALPHA_OPAQUE };
const SDL_Color darkturquoise_color = {  0, 206, 209, SDL_ALPHA_OPAQUE };
const SDL_Color darkviolet_color = { 148, 0, 211, SDL_ALPHA_OPAQUE };
const SDL_Color deeppink_color = { 255, 20, 147, SDL_ALPHA_OPAQUE };
const SDL_Color deepskyblue_color = {  0, 191, 255, SDL_ALPHA_OPAQUE };
const SDL_Color dimgray_color = { 105, 105, 105, SDL_ALPHA_OPAQUE };
const SDL_Color dimgrey_color = { 105, 105, 105, SDL_ALPHA_OPAQUE };
const SDL_Color dodgerblue_color = {  30, 144, 255, SDL_ALPHA_OPAQUE };
const SDL_Color firebrick_color = { 178, 34, 34, SDL_ALPHA_OPAQUE };
const SDL_Color floralwhite_color = { 255, 250, 240, SDL_ALPHA_OPAQUE };
const SDL_Color forestgreen_color = {  34, 139, 34, SDL_ALPHA_OPAQUE };
const SDL_Color fuchsia_color = { 255, 0, 255, SDL_ALPHA_OPAQUE };
const SDL_Color gainsboro_color = { 220, 220, 220, SDL_ALPHA_OPAQUE };
const SDL_Color ghostwhite_color = { 248, 248, 255, SDL_ALPHA_OPAQUE };
const SDL_Color gold_color = { 255, 215, 0, SDL_ALPHA_OPAQUE };
const SDL_Color goldenrod_color = { 218, 165, 32, SDL_ALPHA_OPAQUE };
const SDL_Color gray_color = { 128, 128, 128, SDL_ALPHA_OPAQUE };
const SDL_Color grey_color = { 128, 128, 128, SDL_ALPHA_OPAQUE };
const SDL_Color green_color = {  0, 128, 0, SDL_ALPHA_OPAQUE };
const SDL_Color greenyellow_color = { 173, 255, 47, SDL_ALPHA_OPAQUE };
const SDL_Color honeydew_color = { 240, 255, 240, SDL_ALPHA_OPAQUE };
const SDL_Color hotpink_color = { 255, 105, 180, SDL_ALPHA_OPAQUE };
const SDL_Color indianred_color = { 205, 92, 92, SDL_ALPHA_OPAQUE };
const SDL_Color indigo_color = {  75, 0, 130, SDL_ALPHA_OPAQUE };
const SDL_Color ivory_color = { 255, 255, 240, SDL_ALPHA_OPAQUE };
const SDL_Color khaki_color = { 240, 230, 140, SDL_ALPHA_OPAQUE };
const SDL_Color lavender_color = { 230, 230, 250, SDL_ALPHA_OPAQUE };
const SDL_Color lavenderblush_color = { 255, 240, 245, SDL_ALPHA_OPAQUE };
const SDL_Color lawngreen_color = { 124, 252, 0, SDL_ALPHA_OPAQUE };
const SDL_Color lemonchiffon_color = { 255, 250, 205, SDL_ALPHA_OPAQUE };
const SDL_Color lightblue_color = { 173, 216, 230, SDL_ALPHA_OPAQUE };
const SDL_Color lightcoral_color = { 240, 128, 128, SDL_ALPHA_OPAQUE };
const SDL_Color lightcyan_color = { 224, 255, 255, SDL_ALPHA_OPAQUE };
const SDL_Color lightgoldenrodyellow_color = { 250, 250, 210, SDL_ALPHA_OPAQUE };
const SDL_Color lightgray_color = { 211, 211, 211, SDL_ALPHA_OPAQUE };
const SDL_Color lightgreen_color = { 144, 238, 144, SDL_ALPHA_OPAQUE };
const SDL_Color lightgrey_color = { 211, 211, 211, SDL_ALPHA_OPAQUE };
const SDL_Color lightpink_color = { 255, 182, 193, SDL_ALPHA_OPAQUE };
const SDL_Color lightsalmon_color = { 255, 160, 122, SDL_ALPHA_OPAQUE };
const SDL_Color lightseagreen_color = {  32, 178, 170, SDL_ALPHA_OPAQUE };
const SDL_Color lightskyblue_color = { 135, 206, 250, SDL_ALPHA_OPAQUE };
const SDL_Color lightslategray_color = { 119, 136, 153, SDL_ALPHA_OPAQUE };
const SDL_Color lightslategrey_color = { 119, 136, 153, SDL_ALPHA_OPAQUE };
const SDL_Color lightsteelblue_color = { 176, 196, 222, SDL_ALPHA_OPAQUE };
const SDL_Color lightyellow_color = { 255, 255, 224, SDL_ALPHA_OPAQUE };
const SDL_Color lime_color = {  0, 255, 0, SDL_ALPHA_OPAQUE };
const SDL_Color limegreen_color = {  50, 205, 50, SDL_ALPHA_OPAQUE };
const SDL_Color linen_color = { 250, 240, 230, SDL_ALPHA_OPAQUE };
const SDL_Color magenta_color = { 255, 0, 255, SDL_ALPHA_OPAQUE };
const SDL_Color maroon_color = { 128, 0, 0, SDL_ALPHA_OPAQUE };
const SDL_Color mediumaquamarine_color = { 102, 205, 170, SDL_ALPHA_OPAQUE };
const SDL_Color mediumblue_color = {  0, 0, 205, SDL_ALPHA_OPAQUE };
const SDL_Color mediumorchid_color = { 186, 85, 211, SDL_ALPHA_OPAQUE };
const SDL_Color mediumpurple_color = { 147, 112, 219, SDL_ALPHA_OPAQUE };
const SDL_Color mediumseagreen_color = {  60, 179, 113, SDL_ALPHA_OPAQUE };
const SDL_Color mediumslateblue_color = { 123, 104, 238, SDL_ALPHA_OPAQUE };
const SDL_Color mediumspringgreen_color = {  0, 250, 154, SDL_ALPHA_OPAQUE };
const SDL_Color mediumturquoise_color = {  72, 209, 204, SDL_ALPHA_OPAQUE };
const SDL_Color mediumvioletred_color = { 199, 21, 133, SDL_ALPHA_OPAQUE };
const SDL_Color midnightblue_color = {  25, 25, 112, SDL_ALPHA_OPAQUE };
const SDL_Color mintcream_color = { 245, 255, 250, SDL_ALPHA_OPAQUE };
const SDL_Color mistyrose_color = { 255, 228, 225, SDL_ALPHA_OPAQUE };
const SDL_Color moccasin_color = { 255, 228, 181, SDL_ALPHA_OPAQUE };
const SDL_Color navajowhite_color = { 255, 222, 173, SDL_ALPHA_OPAQUE };
const SDL_Color navy_color = {  0, 0, 128, SDL_ALPHA_OPAQUE };
const SDL_Color oldlace_color = { 253, 245, 230, SDL_ALPHA_OPAQUE };
const SDL_Color olive_color = { 128, 128, 0, SDL_ALPHA_OPAQUE };
const SDL_Color olivedrab_color = { 107, 142, 35, SDL_ALPHA_OPAQUE };
const SDL_Color orange_color = { 255, 165, 0, SDL_ALPHA_OPAQUE };
const SDL_Color orangered_color = { 255, 69, 0, SDL_ALPHA_OPAQUE };
const SDL_Color orchid_color = { 218, 112, 214, SDL_ALPHA_OPAQUE };
const SDL_Color palegoldenrod_color = { 238, 232, 170, SDL_ALPHA_OPAQUE };
const SDL_Color palegreen_color = { 152, 251, 152, SDL_ALPHA_OPAQUE };
const SDL_Color paleturquoise_color = { 175, 238, 238, SDL_ALPHA_OPAQUE };
const SDL_Color palevioletred_color = { 219, 112, 147, SDL_ALPHA_OPAQUE };
const SDL_Color papayawhip_color = { 255, 239, 213, SDL_ALPHA_OPAQUE };
const SDL_Color peachpuff_color = { 255, 218, 185, SDL_ALPHA_OPAQUE };
const SDL_Color peru_color = { 205, 133, 63, SDL_ALPHA_OPAQUE };
const SDL_Color pink_color = { 255, 192, 203, SDL_ALPHA_OPAQUE };
const SDL_Color plum_color = { 221, 160, 221, SDL_ALPHA_OPAQUE };
const SDL_Color powderblue_color = { 176, 224, 230, SDL_ALPHA_OPAQUE };
const SDL_Color purple_color = { 128, 0, 128, SDL_ALPHA_OPAQUE };
const SDL_Color red_color = { 255, 0, 0, SDL_ALPHA_OPAQUE };
const SDL_Color rosybrown_color = { 188, 143, 143, SDL_ALPHA_OPAQUE };
const SDL_Color royalblue_color = {  65, 105, 225, SDL_ALPHA_OPAQUE };
const SDL_Color saddlebrown_color = { 139, 69, 19, SDL_ALPHA_OPAQUE };
const SDL_Color salmon_color = { 250, 128, 114, SDL_ALPHA_OPAQUE };
const SDL_Color sandybrown_color = { 244, 164, 96, SDL_ALPHA_OPAQUE };
const SDL_Color seagreen_color = {  46, 139, 87, SDL_ALPHA_OPAQUE };
const SDL_Color seashell_color = { 255, 245, 238, SDL_ALPHA_OPAQUE };
const SDL_Color sienna_color = { 160, 82, 45, SDL_ALPHA_OPAQUE };
const SDL_Color silver_color = { 192, 192, 192, SDL_ALPHA_OPAQUE };
const SDL_Color skyblue_color = { 135, 206, 235, SDL_ALPHA_OPAQUE };
const SDL_Color slateblue_color = { 106, 90, 205, SDL_ALPHA_OPAQUE };
const SDL_Color slategray_color = { 112, 128, 144, SDL_ALPHA_OPAQUE };
const SDL_Color slategrey_color = { 112, 128, 144, SDL_ALPHA_OPAQUE };
const SDL_Color snow_color = { 255, 250, 250, SDL_ALPHA_OPAQUE };
const SDL_Color springgreen_color = {  0, 255, 127, SDL_ALPHA_OPAQUE };
const SDL_Color steelblue_color = {  70, 130, 180, SDL_ALPHA_OPAQUE };
const SDL_Color tan_color = { 210, 180, 140, SDL_ALPHA_OPAQUE };
const SDL_Color teal_color = {  0, 128, 128, SDL_ALPHA_OPAQUE };
const SDL_Color thistle_color = { 216, 191, 216, SDL_ALPHA_OPAQUE };
const SDL_Color tomato_color = { 255, 99, 71, SDL_ALPHA_OPAQUE };
const SDL_Color turquoise_color = {  64, 224, 208, SDL_ALPHA_OPAQUE };
const SDL_Color violet_color = { 238, 130, 238, SDL_ALPHA_OPAQUE };
const SDL_Color wheat_color = { 245, 222, 179, SDL_ALPHA_OPAQUE };
const SDL_Color white_color = { 255, 255, 255, SDL_ALPHA_OPAQUE };
const SDL_Color whitesmoke_color = { 245, 245, 245, SDL_ALPHA_OPAQUE };
const SDL_Color yellow_color = { 255, 255, 0, SDL_ALPHA_OPAQUE };
const SDL_Color yellowgreen_color = { 154, 205, 50, SDL_ALPHA_OPAQUE };

#endif
