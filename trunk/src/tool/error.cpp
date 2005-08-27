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

#include "../include/base.h"
//-----------------------------------------------------------------------------
#include <iostream>
#include <signal.h>
#include "../tool/i18n.h"

#ifndef WIN32
#include <sys/types.h>
#include <unistd.h>
#endif
//-----------------------------------------------------------------------------

void AssertionManquee (const char *nomfich, unsigned long ligne,
		       const char *message)
{
  std::cout << std::endl;
  std::cerr << nomfich << ':' << ligne 
	    << ": Assertion manquée \"" << message << "\"."
	    << std::endl;
#ifdef DEBUG
  kill (getpid(), SIGABRT);
#endif
  abort();
}

//-----------------------------------------------------------------------------

CErreur::CErreur (const char *nomfich, unsigned long ligne, 
		const std::string &txt) 
  : m_nomfich(nomfich), m_txt(txt), m_ligne(ligne)
{}

//-----------------------------------------------------------------------------

CErreur::~CErreur() throw()
{}

//-----------------------------------------------------------------------------

const char* CErreur::what() const throw()
{
  return m_txt.c_str();
}


//-----------------------------------------------------------------------------

std::ostream& CErreur::operator<< (std::ostream &os) const
{
  os << m_txt;
  return os;
}

//-----------------------------------------------------------------------------

void DeclancheErreur (const char *nomfich, 
		      unsigned long ligne, 
		      const std::string &txt)
{
  std::cout << "! " 
	    << Format(_("Error in %s:%lu"), nomfich, ligne)
	    << ": " << txt << std::endl;

  throw CErreur (nomfich, ligne, txt);
}

//-----------------------------------------------------------------------------
