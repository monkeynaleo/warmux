/******************************************************************************
 *  Wormux is a convivial mass murder game.
 *  Copyright (C) 2001-2010 Wormux Team.
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
 * Refresh water that may be placed in bottom of the ground.
 *****************************************************************************/

#include <assert.h>
#include <SDL.h>
#include "game/game_mode.h"
#include "game/time.h"
#include "graphic/video.h"
#include "interface/interface.h"
#include "map/camera.h"
#include "map/map.h"
#include "map/maps_list.h"
#include "map/water.h"
#include "particles/particle.h"
#include "tool/math_tools.h"
#include "tool/resource_manager.h"
#include "tool/string_tools.h"

const uint GO_UP_TIME = 1; // min
const uint GO_UP_STEP = 15; // pixels
const uint GO_UP_OSCILLATION_TIME = 30; // seconds
const uint GO_UP_OSCILLATION_NBR = 30; // amplitude
const uint MS_BETWEEN_SHIFTS = 20;
const uint PATTERN_WIDTH = 180;
const Double WAVE_HEIGHT_A = 5;
const Double WAVE_HEIGHT_B = 8;
const Double DEGREE = TWO_PI/360;
const int WAVE_INC = 5;
const int WAVE_COUNT = 3;
const std::vector<int> EMPTY_WAVE_HEIGHT_VECTOR(PATTERN_WIDTH);

int Water::pattern_height = 0;

Water::Water()
  : type_color(NULL)
  , height_mvt(0)
  , shift1(0)
  , water_height(0)
  , time_raise(0)
  , height(PATTERN_WIDTH, 0)
  , wave_height(3, EMPTY_WAVE_HEIGHT_VECTOR)
  , water_type("no")
  , m_last_preview_redraw(0)
  , next_wave_shift(0)
{
}

Water::~Water()
{
  if (type_color)
    delete type_color;
}

/*
 * Water consists of 1) water.png texture, which is the actual wave and
 * 2) water_bottom.png, which is water below the wave. Composing the water in
 * this way is done because of optimization purposes.
 *
 * The water consists of three waves, which are drawn using SIN functions in
 * different phases.
 *
 * The water is drawn in 180 x 128 blocks. The size 180 comes from SIN(2x)
 * cycle. The pattern surface is rendered using water.png (and SIN functions)
 * and water_bottom.png.
 */
void Water::Init()
{
  ASSERT(water_type != "no");

  std::string image = "gfx/";
  image += water_type;

  Profile *res = GetResourceManager().LoadXMLProfile("graphism.xml", false);

  surface = GetResourceManager().LoadImage(res, image);
  surface.SetAlpha(0, 0);

  image += "_bottom";

  type_color = new Color(GetResourceManager().LoadColor(res, "water_colors/" + water_type));
  bottom = GetResourceManager().LoadImage(res, image);
  bottom.SetAlpha(0, 0);

  pattern_height = bottom.GetHeight();

  pattern.NewSurface(Point2i(PATTERN_WIDTH, pattern_height),
                     SDL_SWSURFACE|SDL_SRCALPHA, true);
  /* Convert the pattern into the same format than surface. This allow not to
   * need conversions on fly and thus saves CPU */
  pattern.SetSurface(SDL_ConvertSurface(pattern.GetSurface(),
                                        surface.GetSurface()->format,
                                        SDL_SWSURFACE|SDL_SRCALPHA),
                     true /* free old one */);

  // Turn on transparency for water bottom texture
  bottom.SetSurface(SDL_ConvertSurface(bottom.GetSurface(),
                                       bottom.GetSurface()->format,
                                       SDL_SWSURFACE|SDL_SRCALPHA),
                    true);

  shift1 = 0;
  next_wave_shift = 0;
  GetResourceManager().UnLoadXMLProfile(res);
}

void Water::Reset()
{
  water_type = ActiveMap()->GetWaterType();

  if (type_color)
    delete type_color;
  type_color = NULL;

  if (!IsActive())
    return;

  Init();
  water_height = WATER_INITIAL_HEIGHT;
  time_raise = 1000 * GameMode::GetInstance()->duration_before_death_mode;

  Refresh(); // Calculate first height position
}

void Water::Free()
{
  if (!IsActive()) {
    return;
  }

  bottom.Free();
  surface.Free();
  pattern.Free();
  pattern_height = 0;
}

void Water::Refresh()
{
  if (!IsActive()) {
    return;
  }

  height_mvt = 0;
  uint now = Time::GetInstance()->Read();

  if (next_wave_shift <= now) {
    shift1 += 2*DEGREE;
    next_wave_shift += MS_BETWEEN_SHIFTS;
  }

  // Height Calculation:
  const Double t = (GO_UP_OSCILLATION_TIME*1000.0);
  const Double a = GO_UP_STEP/t;
  const Double b = 1.0;

  if (time_raise < now) {
    m_last_preview_redraw = now;
    if (time_raise + GO_UP_OSCILLATION_TIME * 1000 > now) {
      uint dt = now - time_raise;
      height_mvt = GO_UP_STEP + (int)(((Double)GO_UP_STEP *
                 sin(((Double)(dt*(GO_UP_OSCILLATION_NBR-(Double)0.25))
                     / GO_UP_OSCILLATION_TIME/(Double)1000.0)*TWO_PI)
                 )/(a*dt+b));
    } else {
      time_raise += GO_UP_TIME * 60 * 1000;
      water_height += GO_UP_STEP;
    }
  }

  CalculateWaveHeights();
}

void Water::CalculateWaveHeights()
{
  Double angle1 = -shift1;
  Double angle2 = shift1;

  for (uint x = 0; x < PATTERN_WIDTH; x++) {
    // TODO: delete the first dimension of wave_height (now unused)
    wave_height[0][x] = static_cast<int>(sin(angle1)*WAVE_HEIGHT_A + sin(angle2)*WAVE_HEIGHT_B);
    height[x] = wave_height[0][x];

    angle1 += 2*DEGREE;
    angle2 += 4*DEGREE;
  }
}


void Water::CalculateWavePattern()
{
  pattern.SetAlpha(0, 0);
  pattern.Fill(0x00000000);

  /* Locks on SDL_Surface must be taken when accessing pixel member */
  SDL_LockSurface(surface.GetSurface());
  SDL_LockSurface(pattern.GetSurface());

  /* Copy directly the surface image into the pattern image. This doesn't use
   * blit in order to save CPU but it makes this code not really easy to read...
   * The copy is done pixel per pixel */
  uint bpp = surface.GetSurface()->format->BytesPerPixel;

  Uint32  pitch = pattern.GetSurface()->pitch;
  Uint8 * dst;
  Uint8 * src;

  for (uint x = 0; x < PATTERN_WIDTH; x++) {
    dst = (Uint8*)pattern.GetSurface()->pixels + x * bpp + (wave_height[0][x] + 15 + WAVE_INC * (WAVE_COUNT-1)) * pitch;
    src = (Uint8*)surface.GetSurface()->pixels;
    for (uint y=0; y < (uint)surface.GetHeight(); y++) {
      memcpy(dst, src, bpp);
      dst += pitch;
      src += bpp;
    }
  }

  SDL_UnlockSurface(pattern.GetSurface());
  SDL_UnlockSurface(surface.GetSurface());

  pattern.SetAlpha(SDL_SRCALPHA, 0);
}

void Water::Draw()
{
  if (!IsActive()) {
    return;
  }

  const Camera* cam = Camera::GetConstInstance();
  int screen_bottom = (int)cam->GetPosition().y + (int)cam->GetSize().y;
  int water_top = GetWorld().GetHeight() - (water_height + height_mvt) - 20;

  if (screen_bottom < water_top) {
    return; // save precious CPU time
  }

  int x0 = cam->GetPosition().x % PATTERN_WIDTH;
  int cameraRightPosition = cam->GetPosition().x + cam->GetSize().x;
  int y = water_top + (WAVE_HEIGHT_A + WAVE_HEIGHT_B) * 2 + WAVE_INC;

  int h = y - cam->GetPosition().y;
  GetMainWindow().BoxColor(Rectanglei(0, h,
                                      cam->GetSize().x, cam->GetSize().y-h),
                           *type_color);

  CalculateWavePattern();
  y = water_top;
  for (int wave = 0; wave < WAVE_COUNT; wave++) {
    for (int x = cam->GetPosition().x - x0 - ((PATTERN_WIDTH/4) * wave);
         x < cameraRightPosition;
         x += PATTERN_WIDTH) {
      AbsoluteDraw(pattern, Point2i(x, y));
    }
    y += wave * WAVE_INC;
  }
}

int Water::GetHeight(int x) const
{
  if (IsActive()) {
    //printf("Height would have been %i\n", height[x % PATTERN_WIDTH]);
    int h = height[x % PATTERN_WIDTH];
    return h + GetWorld().GetHeight() - int(water_height + height_mvt);
  } else {
    return GetWorld().GetHeight();
  }
}

void Water::Splash(const Point2i& pos) const
{
  if (water_type == "no")
    return;

  ParticleEngine::AddNow(Point2i(pos.x, pos.y-5), 5, particle_WATER, true, -1, 20);
}

void Water::Smoke(const Point2i& pos) const
{
  ParticleEngine::AddNow(Point2i(pos.x, pos.y-5), 2, particle_SMOKE, true, 0, 1);
}
