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
 * Define all Wormux actions.
 *****************************************************************************/

#include "action.h"
//-----------------------------------------------------------------------------
#include <SDL_net.h>
#include "action_handler.h"
//-----------------------------------------------------------------------------

Action::Action (Action_t type)
{
  m_type = type; 
}

Action::Action (Action_t type, Uint32 *is)
{
  m_type = type; 
  int m_lenght = SDLNet_Read32(is);
  is++;

  for(int i=0; i < m_lenght; i++)
  {
    Uint32 val = SDLNet_Read32(is);
    var.push_back(val);
    is++;
  }
}

Action::~Action ()
{
}

Action_t Action::GetType() const 
{ 
  return m_type; 
}

void Action::Write(Uint32 *os)
{ 
  SDLNet_Write32(m_type, os);
  os++;
  Uint32 tmp;
  int size = (int)var.size();
  memcpy(&tmp, &size, 4);
  SDLNet_Write32(tmp, os);
  os++;

  for(std::list<Uint32>::iterator val = var.begin(); val!=var.end(); val++)
  {
    SDLNet_Write32(*val, os);
    os++;
  }
}

void Action::Push(int val)
{
  Uint32 tmp;
  memcpy(&tmp, &val, 4);
  var.push_back(tmp);
  std::cout << "Pushing int value: " << val << std::endl;
}

void Action::Push(double val)
{
  Uint32 tmp[2];
  memcpy(&tmp, &val, 8);
  var.push_back(tmp[0]);
  var.push_back(tmp[1]);
  std::cout << "Pushing double value: " << val << " (" << tmp << ")" << std::endl;
}

void Action::Push(std::string val)
{
  //Cut the string into 32bit values
  //But first, we write the size of the string:
  var.push_back((Uint32)val.size());
  char* ch = (char*)val.c_str();

  int count = val.size();
  while(count > 0)
  {
    Uint32 tmp;
    // Fix-me : We are reading out of the c_str() buffer there :
    memcpy(&tmp, ch, 4);
    var.push_back(tmp);
    ch += 4;
    count -= 4;
  }
  std::cout << "Pushing string value: " << val << std::endl;
}

int Action::PopInt()
{
  assert(var.size() > 0);
  int val;
  Uint32 tmp = var.front();
  memcpy(&val, &tmp, 4);
  var.pop_front();
  std::cout << "Poping int value: " << val << std::endl;
  return val;
}

double Action::PopDouble()
{
  assert(var.size() > 0);
  double val;
  Uint32 tmp[2];
  tmp[0] = var.front();
  var.pop_front();
  tmp[1] = var.front();
  var.pop_front();
  memcpy(&val, &tmp, 8);
  std::cout << "Poping double value: " << val << " (" << tmp << ")" << std::endl;
  return val;
}

std::string Action::PopString()
{
  assert(var.size() > 1);
  int lenght = (int) var.front();
  var.pop_front();

  std::string str="";
  assert((int)var.size() >= lenght/4);
  while(lenght > 0)
  {
    Uint32 tmp = var.front();  
    var.pop_front();
    char tmp_str[5] = {0, 0, 0, 0, 0};
    memcpy(tmp_str, &tmp, 4);
    str += tmp_str;
    lenght -= 4;
  }
  std::cout << "Poping string value: " << str << std::endl;
  return str;
}

std::ostream& Action::out(std::ostream &os) const
{
  os << ActionHandler::GetInstance()->GetActionName(m_type);
  os << " (Mutliformat) ";
  return os;
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------

ActionInt2::ActionInt2 (Action_t type, int v1, int v2) : Action(type) 
{ 
  m_value1 = v1; m_value2 = v2; 
  Push(v1);
  Push(v2);
}

ActionInt2::ActionInt2(Action_t type, Uint32* is) : Action(type, is)
{
  m_value1 = PopInt();
  m_value2 = PopInt();
}

int ActionInt2::GetValue1() const 
{ 
  return m_value1; 
}

int ActionInt2::GetValue2() const 
{ 
  return m_value2; 
}

std::ostream& ActionInt2::out(std::ostream &os) const
{
  Action::out (os);
  os <<  " (2x int) = " << m_value1 << ", " << m_value2;
  return os;
}

//-----------------------------------------------------------------------------

ActionInt::ActionInt (Action_t type, int value) : Action(type)
{
  m_value = value;
  Push(value);
}

ActionInt::ActionInt (Action_t type, Uint32* is) : Action(type, is)
{
  m_value = PopInt();
}

int ActionInt::GetValue() const { return m_value; }

std::ostream& ActionInt::out(std::ostream &os) const
{
  Action::out (os);
  os << " (int) = " << m_value;
  return os;
}

//-----------------------------------------------------------------------------

ActionDouble::ActionDouble (Action_t type, double value) : Action(type)
{
  m_value = value;
  Push(value);
}

ActionDouble::ActionDouble (Action_t type, Uint32* is) : Action(type, is)
{
  m_value = PopDouble();
}

double ActionDouble::GetValue() const { return m_value; }

std::ostream& ActionDouble::out(std::ostream &os) const
{
  Action::out (os);
  os << " (double) = " << m_value;
  return os;
}

//-----------------------------------------------------------------------------

ActionDouble2::ActionDouble2 (Action_t type, double v1, double v2) : Action(type) 
{ 
  m_value1 = v1; m_value2 = v2;
  Push(v1);
  Push(v2);
}

ActionDouble2::ActionDouble2(Action_t type, Uint32* is) : Action(type, is)
{
  m_value1 = PopDouble();
  m_value2 = PopDouble();
}

double ActionDouble2::GetValue1() const 
{ 
  return m_value1; 
}

double ActionDouble2::GetValue2() const 
{ 
  return m_value2; 
}

std::ostream& ActionDouble2::out(std::ostream &os) const
{
  Action::out (os);
  os <<  " (2x double) = " << m_value1 << ", " << m_value2;
  return os;
}

//-----------------------------------------------------------------------------

ActionDoubleInt::ActionDoubleInt (Action_t type, double v1, int v2) : Action(type) 
{
  m_value1 = v1;
  m_value2 = v2;
  Push(m_value1);
  Push(m_value2);
}

ActionDoubleInt::ActionDoubleInt(Action_t type, Uint32* is) : Action(type, is)
{
  m_value1 = PopDouble();
  m_value2 = PopInt();
}

double ActionDoubleInt::GetValue1() const 
{ 
  return m_value1; 
}

int ActionDoubleInt::GetValue2() const 
{ 
  return m_value2; 
}

std::ostream& ActionDoubleInt::out(std::ostream &os) const
{
  Action::out (os);
  os << " (double, int) = " << m_value1 << ", " << m_value2;
  return os;
}

//-----------------------------------------------------------------------------

ActionString::ActionString (Action_t type, const std::string &value) : Action(type)
{
  m_value = value;
  Push(m_value);
}

ActionString::ActionString (Action_t type, Uint32 *is) : Action(type, is)
{
  m_value = PopString();
}

ActionString::~ActionString ()
{
}

const char* ActionString::GetValue() const { return m_value.c_str(); }

std::ostream& ActionString::out(std::ostream &os) const
{
  Action::out (os);
  os << " (string) = " << m_value;
  return os;
}

//-----------------------------------------------------------------------------
std::ostream& operator<<(std::ostream &os, const Action &a)
{
  a.out(os);
  return os;
}

