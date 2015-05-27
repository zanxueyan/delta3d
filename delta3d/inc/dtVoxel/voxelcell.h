/* -*-c++-*-
 * Delta3D Open Source Game and Simulation Engine
 * Copyright (C) 2015, Caper Holdings, LLC
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
 */

#ifndef DTVOXEL_VOXELCELL_H_
#define DTVOXEL_VOXELCELL_H_

#include <dtVoxel/export.h>
#include <dtVoxel/voxelactor.h>
#include <dtUtil/getsetmacros.h>

#include <osg/Vec3>
#include <osg/Matrix>

#include <openvdb/openvdb.h>

namespace dtVoxel
{
    /***
    *  A VoxelCell represents a 3d grid of individual voxels stored in a 3d texture.    
    */
   class VoxelCellImpl;

   class DT_VOXEL_EXPORT VoxelCell
   {
   public:
      VoxelCell();
      virtual ~VoxelCell();

      void CreateImage(VoxelActor& voxelActor, openvdb::GridBase::Ptr localGrid, osg::Matrix& transform, const osg::Vec3& cellSize, const osg::Vec3i& texture_resolution);
      
      void CreateMesh(VoxelActor& voxelActor, openvdb::GridBase::Ptr localGrid, osg::Matrix& transform, const osg::Vec3& cellSize, const osg::Vec3i& resolution);
      
      bool IsAllocated();

      osg::Vec3 GetOffset() const;

      osg::Node* GetOSGNode();
      const osg::Node* GetOSGNode() const;

   protected:
      
      void AllocateImage(VoxelActor& voxelActor, openvdb::GridBase::Ptr gridPtr, const osg::Vec3& cellSize, int width, int height, int slices);
      void AllocateMesh(VoxelActor& voxelActor, openvdb::GridBase::Ptr gridPtr, const osg::Vec3& cellSize, int width, int height, int slices);

   
   private:
      VoxelCellImpl* mImpl;
   };

} /* namespace dtVoxel */

#endif /* DTVOXEL_VOXELCELL_H_ */