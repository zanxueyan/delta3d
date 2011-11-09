/* -*-c++-*-
 * Delta3D Open Source Game and Simulation Engine
 * Copyright (C) 2006, Alion Science and Technology, BMH Operation
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * William E. Johnson II
 */
#ifndef DELTA_FIRE_FIGHTER_EXCEPTION
#define DELTA_FIRE_FIGHTER_EXCEPTION

#include <dtUtil/exception.h>
#include <fireFighter/export.h>

class FIRE_FIGHTER_EXPORT CEGUIException : public dtUtil::Exception
{
public:
   CEGUIException(const std::string& message, const std::string& filename, unsigned int linenum);
   virtual ~CEGUIException() {};
};

class FIRE_FIGHTER_EXPORT CommandLineException : public dtUtil::Exception
{
public:
   CommandLineException(const std::string& message, const std::string& filename, unsigned int linenum);
   virtual ~CommandLineException() {};
};

class FIRE_FIGHTER_EXPORT MissingActorException : public dtUtil::Exception
{
public:
   MissingActorException(const std::string& message, const std::string& filename, unsigned int linenum);
   virtual ~MissingActorException() {};
};

#endif
