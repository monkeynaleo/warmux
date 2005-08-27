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

#ifndef ACTION_H
#define ACTION_H
//-----------------------------------------------------------------------------
#include "base.h"
#include "enum.h"
#include <string>
#include <ClanLib/core.h>
//-----------------------------------------------------------------------------

class Action
{
private:
	Action_t m_type;
public:
	Action (Action_t type);
	virtual ~Action();
	virtual Action_t GetType() const;
	virtual void Write(CL_OutputSource &os) const;
	virtual Action* clone() const;
	virtual std::ostream& out(std::ostream&os) const;
};

//-----------------------------------------------------------------------------

class ActionInt : public Action
{
private:
	int m_value;
public:
	ActionInt (Action_t type, int value);
	ActionInt (Action_t type, CL_InputSource &is);
	int GetValue() const;
	void Write(CL_OutputSource &os) const;
	Action* clone() const;
	std::ostream& out(std::ostream&os) const;
};

//-----------------------------------------------------------------------------

class ActionInt2 : public Action
{
private:
	int m_value1;
	int m_value2;
public:
	ActionInt2 (Action_t type, int val1, int val2);
	ActionInt2 (Action_t type, CL_InputSource &is);
	int GetValue1() const;
	int GetValue2() const;
	void Write(CL_OutputSource &os) const;
	Action* clone() const;
	std::ostream& out(std::ostream&os) const;
};

//-----------------------------------------------------------------------------

class ActionDoubleInt : public Action
{
private:
	double m_value1;
	int m_value2;
public:
	ActionDoubleInt (Action_t type, double val1, int val2);
	ActionDoubleInt (Action_t type, CL_InputSource &is);
	double GetValue1() const;
	int GetValue2() const;
	void Write(CL_OutputSource &os) const;
	Action* clone() const;
	std::ostream& out(std::ostream&os) const;
};

//-----------------------------------------------------------------------------

class ActionString : public Action
{
private:
	std::string m_value;
public:
	ActionString (Action_t type, const std::string& value);
	ActionString (Action_t type, CL_InputSource &is);
	std::string GetValue() const;
	void Write(CL_OutputSource &os) const;
	Action* clone() const;
	std::ostream& out(std::ostream&os) const;
};

//-----------------------------------------------------------------------------

// Output action in a ostream (for debug)
std::ostream& operator<<(std::ostream& os, const Action &a);

//-----------------------------------------------------------------------------
#endif
