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
 *Energy bar.
 *****************************************************************************/

#include <iostream>
#include <sstream>
#include "gui/energy_bar.h"
#include "tool/resource_manager.h"
#include "gui/progress_bar.h"

static const int energy_step[EnergyBar::NB_OF_ENERGY_COLOR] = { 16, 33, 50, 67, 84, 100 };

EnergyBar::EnergyBar(uint _x,
                     uint _y,
                     uint _width,
                     uint _height,
                     long _value,
                     long minValue,
                     long maxValue,
                     enum orientation _orientation) : 
  ProgressBar(_x,
              _y,
              _width,
              _height,
              _value,
              minValue,
              maxValue,
              _orientation)
{
  Profile *res = GetResourceManager().LoadXMLProfile("graphism.xml", false);

  for(int i = 0; i < NB_OF_ENERGY_COLOR ;i++) {
    std::ostringstream color_name;
    color_name << "energy_bar/energy_color_" << energy_step[i] << "_percent";
    Color colors_value = GetResourceManager().LoadColor(res, color_name.str());

    ProcessThresholds(i, energy_step[i], colors_value);
  }
  GetResourceManager().UnLoadXMLProfile(res);
  SortThresholds();
}

EnergyBar::EnergyBar(Profile * _profile,
                     const xmlNode * _widgetNode) :
  profile(_profile),
  widgetNode(_widgetNode)
{
}

bool EnergyBar::LoadXMLConfiguration()
{
  if (NULL == profile || NULL == widgetNode) {
    return false;
  }
  XmlReader * xmlFile = profile->GetXMLDocument();

  unsigned int thresholdCount = xmlFile->GetNbChildren(widgetNode);
  const xmlNode * thresholdNode = xmlFile->GetFirstChild(widgetNode);
  uint i = 0;
  double thresholdValue;

  for ( ; thresholdCount > 0; --thresholdCount) {

    if ("threshold" == xmlFile->GetNodeName(thresholdNode)) {
      if (xmlFile->ReadPercentageAttr(thresholdNode, "value", thresholdValue)) {
        Color thresholdColor(0, 0, 0, 255);
        xmlFile->ReadHexColorAttr(thresholdNode, "color", thresholdColor);
        ProcessThresholds(i++, thresholdValue, thresholdColor);
      } else {
        // Malformed threshold value !
      }
    }
    thresholdNode = xmlFile->GetNextSibling(thresholdNode);
  } 
  return true;
}

void EnergyBar::ProcessThresholds(int thresholdNumber,
                                  float thresholdMax,
                                  Color & colorMax)
{
  if (1 > thresholdNumber || NB_OF_ENERGY_COLOR < thresholdNumber) {
    if (0 == thresholdNumber) {
      Threshold first;
      first.thresholdValue = 0.0;
      first.color = colorMax;
      first.redCoef = 0.0;
      first.greenCoef = 0.0;
      first.blueCoef = 0.0;
      first.alphaCoef = 0.0;
      thresholds[0] = first; 
    }
    return;
  }
  
  Color colorMin = thresholds[thresholdNumber - 1].color;
  float thresholdMin = thresholds[thresholdNumber - 1].thresholdValue;
  uint size = orientation == PROG_BAR_HORIZONTAL ? larg : haut; 
  float range = size * (thresholdMax - thresholdMin) / 100.0;

  Threshold newThreshold; 

  newThreshold.thresholdValue = thresholdMax; 
  newThreshold.color = colorMax;

  newThreshold.redCoef   = (colorMax.GetRed()   - colorMin.GetRed())   / range;
  newThreshold.greenCoef = (colorMax.GetGreen() - colorMin.GetGreen()) / range;
  newThreshold.blueCoef  = (colorMax.GetBlue()  - colorMin.GetBlue())  / range;
  newThreshold.alphaCoef = (colorMax.GetAlpha() - colorMin.GetAlpha()) / range;

  thresholds[thresholdNumber] = newThreshold;
}

void EnergyBar::SortThresholds()
{
  bool needMove;

  do {
    needMove = false;
    for (int i = 0; i < NB_OF_ENERGY_COLOR - 1; ++i) {
      if (thresholds[i].thresholdValue > thresholds[i + 1].thresholdValue) {
        Threshold tmp = thresholds[i + 1];
        thresholds[i + 1] = thresholds[i];
        thresholds[i] = tmp;
        needMove = true;
      }
    }
  } while (needMove);
}

void EnergyBar::Actu(long real_energy)
{
  val = ComputeValue(real_energy);
  val_barre   = ComputeBarValue(val);
  float currentPercentage = abs(val) / (float)max * 100.0;
  Threshold thresholdMin;
  Threshold thresholdMax;

  for (int i = 0; i < NB_OF_ENERGY_COLOR; ++i) {
    if (currentPercentage > thresholds[i].thresholdValue) {
      continue;
    } else {
      if (i > 0) {
        thresholdMin = thresholds[i - 1];
      } else {
        thresholdMin = thresholds[0];
      }
      thresholdMax = thresholds[i];
      break;
    }
  }
  Color colorMin = thresholdMin.color;
  uint coefVal = ComputeBarValue(abs(real_energy)) - 
                 ComputeBarValue(max * thresholdMin.thresholdValue / 100.0);

  value_color.SetColor((Uint8) (colorMin.GetRed()   + (thresholdMax.redCoef   * coefVal)),
                       (Uint8) (colorMin.GetGreen() + (thresholdMax.greenCoef * coefVal)),
                       (Uint8) (colorMin.GetBlue()  + (thresholdMax.blueCoef  * coefVal)),
                       (Uint8) (colorMin.GetAlpha() + (thresholdMax.alphaCoef * coefVal)));
}

