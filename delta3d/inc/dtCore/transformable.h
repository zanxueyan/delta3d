/* 
 * Delta3D Open Source Game and Simulation Engine 
 * Copyright (C) 2004 MOVES Institute 
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
*/

#ifndef DELTA_TRANSFORMABLE
#define DELTA_TRANSFORMABLE




#include "dtCore/base.h"
#include "sg.h"
#include "dtCore/transform.h"
#include <osg/ref_ptr>

namespace dtCore
{
   ///Anything that can be located and moved in 3D space
   
   /** The Transformable class is the base class of anything that can move in the 
     * virtual world.
     * 
     * The default coordinate system of dtCore is +X to the right, +Y forward into
     * the screen, and +Z is up.  Therefore, heading is around the Z axis, pitch
     * is around the X axis, and roll is around the Y axis.  The angles are all
     * right-hand-rule.
     */
   class DT_EXPORT Transformable : public Base  
   {
      DECLARE_MANAGEMENT_LAYER(Transformable)
   public:
      enum CoordSysEnum{
      REL_CS, ///< The Transform coordinate system is relative to the parent
      ABS_CS  ///< The Transform coordinate system is absolute
      } ;

      Transformable();
      virtual ~Transformable();

      //typedef std::vector<Transformable>  ChildList;
      typedef std::vector<osg::ref_ptr<Transformable> > ChildList;

      ///Test to see if child
      bool CanBeChild( Transformable *child );

      ///Add a Transformable child
	   void AddChild( Transformable *child );
         
      ///Remove a Transformable child
      void RemoveChild( Transformable *child );

      ///Return the number of Transformable children added
      inline unsigned int GetNumChildren() { return mChildList.size(); }

      ///Get the child specified by idx (0 to number of children-1)
      Transformable* GetChild( unsigned int idx ) {return mChildList[idx].get();}

      ///Get the immediate parent of this instance
      Transformable* GetParent(void) {return mParent.get();}

      ///Set the Transform to reposition this Transformable
      virtual  void SetTransform( Transform *xform, CoordSysEnum cs=ABS_CS );

      ///Get the current Transform of this Transformable
      virtual  void GetTransform( Transform *xform, CoordSysEnum cs=ABS_CS  );

   protected:
      ///Override function for derived object to know when attaching to scene
      virtual void SetParent(Transformable* parent) {mParent=parent;}
      
   protected:

      Transform *mRelTransform;  ///<position relative to the parent
      ChildList mChildList;      ///<List of children Transformables added
      osg::ref_ptr<Transformable> mParent; ///<Any immediate parent of this instance

   private:

   /** Get the index number of child, return a value between
     * 0 and the number of children-1 if found, if not found then
     * return the number of children.
     */
      inline unsigned int GetChildIndex( const Transformable* child ) const
      {
         for (unsigned int childNum=0;childNum<mChildList.size();++childNum)
         {
            if (mChildList[childNum]==child) return childNum;
         } 
         return mChildList.size(); // node not found.
      }

      void Transformable::CalcAbsTransform( Transform *xform );

   };
};



#endif // DELTA_TRANSFORMABLE
