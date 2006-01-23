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
 * Refresh des erreurs, exceptions, assertions, etc.
 *****************************************************************************/

#ifndef ERROR_H
#define ERROR_H
//-----------------------------------------------------------------------------

#ifndef BASE_H
#  error "You have to include base.h only (not the error.h file)"
#endif

//-----------------------------------------------------------------------------
#include <string>
//-----------------------------------------------------------------------------

void MissedAssertion (const char *filename, unsigned long line,
		       const char *message);

//-----------------------------------------------------------------------------

// Assertion (disabled in release mode)
#undef assert
#ifdef DEBUG
#  define assert(COND) \
     if (!(COND)) MissedAssertion (__FILE__, __LINE__, #COND);
#else
#  define assert(COND)
#endif

// Force une assertion
#define FORCE_ASSERT(COND) \
  if (!(COND)) MissedAssertion (__FILE__, __LINE__, #COND);

//-----------------------------------------------------------------------------

class CError : public std::exception
{
protected:
  std::string m_filename, m_txt;
  unsigned long m_line;

public:
  CError (const char *filename, unsigned long line, const std::string &txt);
  ~CError() throw();
  virtual const char* what() const throw();
  std::ostream& operator<< (std::ostream &os) const;
};

//-----------------------------------------------------------------------------

void TriggerError (const char *filename, unsigned long line, 
		      const std::string &txt);

//-----------------------------------------------------------------------------

#define Error(txt) TriggerError(__FILE__, __LINE__, txt)

//-----------------------------------------------------------------------------
#endif
