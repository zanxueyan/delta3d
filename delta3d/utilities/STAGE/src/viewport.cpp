/* -*-c++-*-
 * Delta3D Simulation Training And Game Editor (STAGE)
 * STAGE - This source file (.h & .cpp) - Using 'The MIT License'
 * Copyright (C) 2005-2008, Alion Science and Technology Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * This software was developed by Alion Science and Technology Corporation under
 * circumstances in which the U. S. Government may have rights in the software.
 *
 * Matthew W. Campbell
 */
#include <prefix/dtstageprefix-src.h>
#include <QtGui/QAction>
#include <QtGui/QMouseEvent>

#include <dtEditQt/viewport.h>
#include <dtEditQt/viewportoverlay.h>
#include <dtEditQt/viewportmanager.h>
#include <dtEditQt/editoractions.h>
#include <dtEditQt/editorevents.h>
#include <dtEditQt/editordata.h>

#include <osg/StateSet>
#include <osg/PolygonMode>
#include <osg/PolygonOffset>
#include <osg/CullFace>
#include <osg/Viewport>
#include <osg/FrameStamp>
#include <osg/StateSet>
#include <osg/ClearNode>
#include <osg/AlphaFunc>

#include <osgDB/Registry>

#include <osgUtil/SceneView>

#include <dtCore/scene.h>
#include <dtCore/databasepager.h>
#include <osgDB/DatabasePager>
#include <dtCore/system.h>
#include <dtCore/isector.h>

#include <dtDAL/exceptionenum.h>
#include <dtDAL/map.h>
#include <dtDAL/transformableactorproxy.h>
#include <dtDAL/actorproxyicon.h>

#include <dtActors/prefabactorproxy.h>

#include <cmath>
#include <sstream>

namespace dtEditQt
{

   ///////////////////////////////////////////////////////////////////////////////
   IMPLEMENT_ENUM(Viewport::RenderStyle);
   const Viewport::RenderStyle Viewport::RenderStyle::WIREFRAME("WIREFRAME");
   const Viewport::RenderStyle Viewport::RenderStyle::LIT("LIT");
   const Viewport::RenderStyle Viewport::RenderStyle::TEXTURED("TEXTURED");
   const Viewport::RenderStyle Viewport::RenderStyle::LIT_AND_TEXTURED("LIT_AND_TEXTURED");
   ///////////////////////////////////////////////////////////////////////////////

   ///////////////////////////////////////////////////////////////////////////////
   IMPLEMENT_ENUM(Viewport::InteractionMode);
   const Viewport::InteractionMode Viewport::InteractionMode::NOTHING("NOTHING");
   const Viewport::InteractionMode Viewport::InteractionMode::CAMERA("CAMERA");
   const Viewport::InteractionMode Viewport::InteractionMode::ACTOR("ACTOR");
   const Viewport::InteractionMode Viewport::InteractionMode::SELECT_ACTOR("SELECT_ACTOR");
   ///////////////////////////////////////////////////////////////////////////////


   ///////////////////////////////////////////////////////////////////////////////
   Viewport::Viewport(ViewportManager::ViewportType& type, const std::string& name, QWidget* parent, QGLWidget* shareWith)
      : QGLWidget(parent, shareWith)
      , mInChangeTransaction(false)
      , mName(name)
      , mViewPortType(type)
      , mRedrawContinuously(false)
      , mUseAutoInteractionMode(false)
      , mAutoSceneUpdate(true)
      , mInitialized(false)
      , mEnableKeyBindings(true)
      , mIsector(new dtCore::Isector())
      , mIsMouseTrapped(false)
   {
      mFrameStamp = new osg::FrameStamp();
      mMouseSensitivity = 10.0f;
      mInteractionMode = &InteractionMode::NOTHING;
      mRenderStyle = &RenderStyle::WIREFRAME;

      mRootNodeGroup = new osg::Group();
      mSceneView = new osgUtil::SceneView();

      mSceneView->setDefaults();
      mSceneView->setFrameStamp(mFrameStamp.get());
      mSceneView->setSceneData(mRootNodeGroup.get());
      setOverlay(ViewportManager::GetInstance().getViewportOverlay());

      setMouseTracking(true);
      mCacheMouseLocation = true;

      connect(&mTimer, SIGNAL(timeout()), this, SLOT(updateGL()));
      mTimer.setInterval(10);
      //mTimer.setSingleShot(true);
   }

   ///////////////////////////////////////////////////////////////////////////////
   Viewport::~Viewport()
   {
   }

   ///////////////////////////////////////////////////////////////////////////////
   void Viewport::initializeGL()
   {
      setupInitialRenderState();
      ViewportManager::GetInstance().initializeGL();
      mInitialized = true;
      if (mRedrawContinuously)
      {
         mTimer.start();
      }
   }

   ///////////////////////////////////////////////////////////////////////////////
   void Viewport::setScene(dtCore::Scene* scene)
   {
      //First, remove the old scene, then add the new one.
      if (mSceneView.valid())
      {
         if (mScene != NULL)
         {
            mRootNodeGroup->replaceChild(mScene->GetSceneNode(), scene->GetSceneNode());
         }
         else
         {
            mRootNodeGroup->addChild(scene->GetSceneNode());
         }

         mScene = scene;
         //mScene->GetSceneNode()->setStateSet(mGlobalStateSet.get());
      }
   }

   ///////////////////////////////////////////////////////////////////////////////
   void Viewport::setOverlay(ViewportOverlay* overlay)
   {
      if (mSceneView.valid())
      {
         //If the new overlay is NULL, clear the current overlay.
         if (overlay == NULL)
         {
            mRootNodeGroup->removeChild(mOverlay->getOverlayGroup());
            mOverlay = NULL;
            return;
         }

         //Else update the current overlay in both the scene and in the viewport.
         if (mOverlay != NULL)
         {
            mRootNodeGroup->replaceChild(mOverlay->getOverlayGroup(), overlay->getOverlayGroup());
         }
         else
         {
            mRootNodeGroup->addChild(overlay->getOverlayGroup());
         }

         mOverlay = overlay;
      }
   }

   ///////////////////////////////////////////////////////////////////////////////
   void Viewport::SetRedrawContinuously(bool contRedraw)
   {
      if (mRedrawContinuously == contRedraw)
      {
         return;
      }

      mRedrawContinuously = contRedraw;
      if (mRedrawContinuously)
      {
         mTimer.start();
      }
      else
      {
         mTimer.stop();
      }
   }

   ///////////////////////////////////////////////////////////////////////////////
   void Viewport::resizeGL(int width, int height)
   {
      mSceneView->setViewport(0, 0, width, height);
   }

   ///////////////////////////////////////////////////////////////////////////////
   void Viewport::paintGL()
   {
      if (!mSceneView.valid() || !mScene.valid() || !mCamera.valid())
      {
         return;
      }

      renderFrame();
   }

   ////////////////////////////////////////////////////////////////////////////////
   void Viewport::refreshActorSelection(const std::vector< dtCore::RefPtr<dtDAL::ActorProxy> >& actors)
   {
   }

   ///////////////////////////////////////////////////////////////////////////////
   void Viewport::refresh()
   {
      updateGL();
   }

   ///////////////////////////////////////////////////////////////////////////////
   void Viewport::setClearColor(const osg::Vec4& color)
   {
      if (mClearNode.valid())
      {
         mClearNode->setClearColor(color);
      }
   }

   ///////////////////////////////////////////////////////////////////////////////
   void Viewport::renderFrame()
   {
      getCamera()->update();
      getSceneView()->setProjectionMatrix(getCamera()->getProjectionMatrix());
      getSceneView()->setViewMatrix(getCamera()->getWorldViewMatrix());

      // Make sure the billboards of any actor proxies are oriented towards the
      // camera in this viewport.
      if (getAutoSceneUpdate())
      {
         updateActorProxyBillboards();
      }

      if (ViewportManager::GetInstance().IsPagingEnabled())
      {
         const dtCore::DatabasePager* dbp = ViewportManager::GetInstance().GetDatabasePager();
         if (dbp != NULL)
         {
            dbp->SignalBeginFrame(mFrameStamp.get());
#if OPENSCENEGRAPH_MAJOR_VERSION < 2 || (OPENSCENEGRAPH_MAJOR_VERSION == 2 && OPENSCENEGRAPH_MINOR_VERSION <= 6)
            dbp->UpdateSceneGraph(mFrameStamp->getReferenceTime());
#else
            dbp->UpdateSceneGraph(mFrameStamp.get());
#endif
         }
      }

      mFrameStamp->setReferenceTime(osg::Timer::instance()->delta_s(ViewportManager::GetInstance().GetStartTick(), osg::Timer::instance()->tick()));
      mFrameStamp->setFrameNumber(mFrameStamp->getFrameNumber()+ 1);

      mSceneView->update();
      mSceneView->cull();
      mSceneView->draw();

      if (ViewportManager::GetInstance().IsPagingEnabled())
      {
         const dtCore::DatabasePager* dbp = ViewportManager::GetInstance().GetDatabasePager();
         if (dbp != NULL)
         {
            dbp->SignalEndFrame();
            // This magic number is the default amount of time that dtCore Scene USED to use.
            double cleanupTime = 0.0025;
            dbp->CompileGLObjects(*mSceneView->getState(), cleanupTime);

            mSceneView->flushDeletedGLObjects(cleanupTime);
         }
      }
   }

   ///////////////////////////////////////////////////////////////////////////////
   void Viewport::setRenderStyle(const RenderStyle& style, bool refreshView)
   {
      int i;
      int numTextureUnits = ViewportManager::GetInstance().getNumTextureUnits();

      mRenderStyle = &style;
      if (!mSceneView.valid())
      {
         throw dtUtil::Exception(dtDAL::ExceptionEnum::BaseException,"Cannot set render style "
               "because the current scene view is invalid.", __FILE__, __LINE__);
      }

      osg::StateAttribute::GLModeValue turnOn  = osg::StateAttribute::OVERRIDE |osg::StateAttribute::ON;
      osg::StateAttribute::GLModeValue turnOff = osg::StateAttribute::OVERRIDE |osg::StateAttribute::OFF;

      osg::PolygonMode* pm = dynamic_cast<osg::PolygonMode*>(
            mGlobalStateSet->getAttribute(osg::StateAttribute::POLYGONMODE));

      if (*mRenderStyle == RenderStyle::WIREFRAME)
      {
         for (i = 0; i < numTextureUnits; ++i)
         {
            mGlobalStateSet->setTextureMode(i, GL_TEXTURE_2D, turnOff);
         }
         mGlobalStateSet->setMode(GL_LIGHTING, turnOff);
         pm->setMode(osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::LINE);
      }
      else if (*mRenderStyle == RenderStyle::TEXTURED)
      {
         for (i = 0; i < numTextureUnits; ++i)
         {
            mGlobalStateSet->removeTextureMode(i, GL_TEXTURE_2D);
         }
         mGlobalStateSet->setMode(GL_LIGHTING, turnOff);
         pm->setMode(osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::FILL);
      }
      else if (*mRenderStyle == RenderStyle::LIT)
      {
         for (i = 0; i < numTextureUnits; ++i)
         {
            mGlobalStateSet->setTextureMode(i, GL_TEXTURE_2D, turnOff);
         }
         mGlobalStateSet->setMode(GL_LIGHTING, turnOn);
         pm->setMode(osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::FILL);
      }
      else if (*mRenderStyle == RenderStyle::LIT_AND_TEXTURED)
      {
         for (i = 0; i < numTextureUnits; ++i)
         {
            mGlobalStateSet->removeTextureMode(i, GL_TEXTURE_2D);
         }
         mGlobalStateSet->setMode(GL_LIGHTING, turnOn);
         pm->setMode(osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::FILL);
      }

      if (refreshView)
      {
         if (!isInitialized())
         {
            throw dtUtil::Exception(dtDAL::ExceptionEnum::BaseException,"Cannot refresh the viewport. "
                  "It has not been initialized.", __FILE__, __LINE__);
         }
         updateGL();
      }

      emit renderStyleChanged();
   }

   ///////////////////////////////////////////////////////////////////////////////
   void Viewport::pick(int x, int y)
   {
      dtCore::RefPtr<dtDAL::Map> currMap = EditorData::GetInstance().getCurrentMap();
      if (!currMap.valid() || getCamera()== NULL)
      {
         return;
      }

      std::vector<dtCore::RefPtr<dtDAL::ActorProxy> > toSelect;

      dtCore::DeltaDrawable* drawable = getPickDrawable(x, y);
      if (!drawable)
      {
         EditorEvents::GetInstance().emitActorsSelected(toSelect);
         return;
      }

      ViewportOverlay* overlay = ViewportManager::GetInstance().getViewportOverlay();
      ViewportOverlay::ActorProxyList& selection = overlay->getCurrentActorSelection();

      // First see if the selected drawable is an actor.
      dtDAL::ActorProxy* newSelection = currMap->GetProxyById(drawable->GetUniqueId());

      // if its not an actor that is directly part of the map then it may be an actor under a prefab.
      if (newSelection == NULL)
      {
         dtCore::DeltaDrawable* parent = drawable->GetParent();
         if (parent)
         {
            // If the parent is a prefab actor, then we want to select that instead.
            dtActors::PrefabActor* prefabActor = dynamic_cast<dtActors::PrefabActor*>(parent);
            if (prefabActor)
            {
               newSelection = currMap->GetProxyById(prefabActor->GetUniqueId());
            }
         }
      }

      // If its not an actor then it may be a billboard placeholder for an actor.
      if (newSelection == NULL)
      {
         const std::map< dtCore::UniqueId, dtCore::RefPtr<dtDAL::ActorProxy> >&
            proxyList = currMap->GetAllProxies();
         std::map< dtCore::UniqueId, dtCore::RefPtr<dtDAL::ActorProxy> >::const_iterator proxyItor;

         // Loop through the proxies searching for the one with billboard geometry
         // matching what was selected.
         for (proxyItor = proxyList.begin(); proxyItor != proxyList.end(); ++proxyItor)
         {
            dtDAL::ActorProxy* proxy =const_cast<dtDAL::ActorProxy*>(proxyItor->second.get());

            if (proxy->GetRenderMode() == dtDAL::ActorProxy::RenderMode::DRAW_ACTOR)
            {
               continue;
            }

            const dtDAL::ActorProxyIcon* billBoard = proxy->GetBillBoardIcon();
            if (billBoard && billBoard->OwnsDrawable(drawable))
            {
               newSelection = proxy;
               break;
            }
         }
      }

      if (newSelection)
      {
         // Determine if this new selection is part of a group.
         std::vector<dtDAL::ActorProxy*> groupActors;
         groupActors.push_back(newSelection);

         int groupIndex = currMap->FindGroupForActor(newSelection);

         if (groupIndex > -1)
         {
            int actorCount = currMap->GetGroupActorCount(groupIndex);
            for (int actorIndex = 0; actorIndex < actorCount; actorIndex++)
            {
               dtDAL::ActorProxy* proxy = currMap->GetActorFromGroup(groupIndex, actorIndex);
               if (proxy != newSelection)
               {
                  groupActors.push_back(proxy);
               }
            }
         }

         if (overlay->isActorSelected(newSelection))
         {
            for (int index = 0; index < (int)groupActors.size(); index++)
            {
               overlay->removeActorFromCurrentSelection(groupActors[index]);
            }
         }
         else
         {
            for (int index = 0; index < (int)groupActors.size(); index++)
            {
               toSelect.push_back(groupActors[index]);
            }
         }
      }

      // Inform the world what objects were selected and refresh all the viewports
      // affected by the change.  If we are in multi-selection mode (i.e. the control
      // key is pressed) add the current selection to the newly selected proxy.
      if (overlay->getMultiSelectMode())
      {
         ViewportOverlay::ActorProxyList::iterator itor;
         for (itor = selection.begin(); itor != selection.end(); ++itor)
         {
            toSelect.push_back(const_cast<dtDAL::ActorProxy*>(itor->get()));
         }
      }

      EditorEvents::GetInstance().emitActorsSelected(toSelect);
   }

   ////////////////////////////////////////////////////////////////////////////////
   dtCore::DeltaDrawable* Viewport::getPickDrawable(int x, int y)
   {
      if (!mScene.valid())
      {
         throw dtUtil::Exception(dtDAL::ExceptionEnum::BaseException,
            "Scene is invalid.  Cannot pick objects from an invalid scene.", __FILE__, __LINE__);
         return NULL;
      }

      dtCore::RefPtr<dtDAL::Map> currMap = EditorData::GetInstance().getCurrentMap();
      if (!currMap.valid() || getCamera()== NULL)
      {
         return NULL;
      }

      // Before we do any intersection tests, make sure the billboards are updated
      // to reflect their orientation in this viewport.
      getCamera()->update();
      if (getAutoSceneUpdate())
      {
         updateActorProxyBillboards();
      }

      mIsector->Reset();
      mIsector->SetScene( getScene());
      osg::Vec3 nearPoint, farPoint;
      int yLoc = int(mSceneView->getViewport()->height()-y);

      mSceneView->projectWindowXYIntoObject(x, yLoc, nearPoint, farPoint);
      mIsector->SetStartPosition(nearPoint);
      mIsector->SetDirection(farPoint-nearPoint);

      // If we found no intersections no need to continue so emit an empty selection
      // and return.
      if (!mIsector->Update())
      {
         return NULL;
      }

      if (mIsector->GetClosestDeltaDrawable()== NULL)
      {
         LOG_ERROR("Intersection query reported an intersection but returned an "
            "invalid DeltaDrawable.");
         return NULL;
      }

      return mIsector->GetClosestDeltaDrawable();
   }

   ///////////////////////////////////////////////////////////////////////////////
   void Viewport::onGotoActor(dtCore::RefPtr<dtDAL::ActorProxy> proxy)
   {
      dtDAL::TransformableActorProxy* tProxy = dynamic_cast<dtDAL::TransformableActorProxy*>(proxy.get());

      if (tProxy != NULL && getCamera()!= NULL)
      {
         osg::Vec3 viewDir = getCamera()->getViewDir();

         osg::Vec3 translation = tProxy->GetTranslation();
         const osg::BoundingSphere& bs = tProxy->GetActor()->GetOSGNode()->getBound();
         float actorCreationOffset = EditorData::GetInstance().GetActorCreationOffset();
         float offset = (bs.radius() < 1000.0f) ? bs.radius() : 1.0f;
         if (offset <= 0.0f)
         {
            offset = actorCreationOffset;
         }

         getCamera()->setPosition(translation);
         if (mViewPortType == ViewportManager::ViewportType::PERSPECTIVE)
         {
            getCamera()->move(viewDir*-offset*1.5f);
         }

         refresh();
      }
   }

   //////////////////////////////////////////////////////////////////////////
   void Viewport::onGotoPosition(double x, double y, double z)
   {
      StageCamera* cam = getCamera();
      if (cam != NULL)
      {
         cam->setPosition(osg::Vec3(x, y, z));
      }

      refresh(); // manually redraw the viewport to show new position
   }

   ///////////////////////////////////////////////////////////////////////////////
   void Viewport::trapMouseCursor()
   {
      // Get the current cursor so we can restore it later.
      if (cursor().shape()!= Qt::BlankCursor)
      {
         mOldMouseCursor = cursor();
      }
      setCursor(QCursor(Qt::BlankCursor));

      // Cache the old mouse location so the cursor doesn't appear to jump when
      // the camera mode operation is complete.
      if (mCacheMouseLocation)
      {
         mOldMouseLocation = QCursor::pos();
         mCacheMouseLocation = false;
      }

      // I disabled this because the mouse move event does this whenever the mouse moves.
      // Commenting this out helps mouse movement work better in Mac OS X.

      // Put the mouse cursor in the center of the viewport.
      QPoint center((x()+width())/2, (y()+height())/2);
      mLastMouseUpdateLocation = center;
      QCursor::setPos(mapToGlobal(center));
      mIsMouseTrapped = true;
   }

   ///////////////////////////////////////////////////////////////////////////////
   void Viewport::releaseMouseCursor(const QPoint& mousePosition)
   {
      mIsMouseTrapped = false;
      setCursor(mOldMouseCursor);

      if (mousePosition.x() != -1 && mousePosition.y() != -1)
      {
         QCursor::setPos(mousePosition);
      }
      else
      {
         QCursor::setPos(mOldMouseLocation);
      }

      mCacheMouseLocation = true;
   }

   ///////////////////////////////////////////////////////////////////////////////
   void Viewport::mouseMoveEvent(QMouseEvent* e)
   {
      static bool mouseMoving = false;
      // Moving the mouse back to the center makes the movement recurse
      // so this is a flag to prevent the recursion

      if (mouseMoving)
      {
         return;
      }

      float dx, dy;

      dx = (float)(e->pos().x() - mLastMouseUpdateLocation.x());
      dy = (float)(e->pos().y() - mLastMouseUpdateLocation.y());

      onMouseMoveEvent(e, dx, dy);

      QPoint mousePos = e->pos();

      if (mIsMouseTrapped)
      {
         QPoint center((x()+width())/2, (y()+height())/2);

         float dxCenter = std::abs(float(e->pos().x() - center.x()));
         float dyCenter = std::abs(float(e->pos().y() - center.y()));

         if (dxCenter > (width()/2) || dyCenter > (height()/2))
         {
            // Moving the mouse back to the center makes the movement recurse
            // so this is a flag to prevent the recursion
            mouseMoving = true;
            QCursor::setPos(mapToGlobal(center));
            mousePos = center;
            mouseMoving = false;
         }
      }

      mLastMouseUpdateLocation = mousePos;

      refresh();
   }

   ///////////////////////////////////////////////////////////////////////////////
   void Viewport::focusInEvent(QFocusEvent* event)
   {
      QGLWidget::focusInEvent(event);
      mTimer.setInterval(10);
   }

   ///////////////////////////////////////////////////////////////////////////////
   void Viewport::focusOutEvent(QFocusEvent* event)
   {
      QGLWidget::focusOutEvent(event);
      // One half of a second.
      mTimer.setInterval(500);
   }

   ///////////////////////////////////////////////////////////////////////////////
   void Viewport::connectInteractionModeSlots()
   {
      // Connect the global actions we want to track.
      EditorActions& ga = EditorActions::GetInstance();
      EditorEvents&  ge = EditorEvents::GetInstance();

      connect(&ge, SIGNAL(gotoActor(ActorProxyRefPtr)),          this, SLOT(onGotoActor(ActorProxyRefPtr)));
      connect(&ge, SIGNAL(gotoPosition(double, double, double)), this, SLOT(onGotoPosition(double,double,double)));
      connect(&ge, SIGNAL(beginChangeTransaction()),             this, SLOT(onBeginChangeTransaction()));
      connect(&ge, SIGNAL(endChangeTransaction()),               this, SLOT(onEndChangeTransaction()));
   }

   ///////////////////////////////////////////////////////////////////////////////
   void Viewport::disconnectInteractionModeSlots()
   {
      //Disconnect from all our global actions we were previously tracking.
      EditorActions &ga = EditorActions::GetInstance();
      EditorEvents  &ge = EditorEvents::GetInstance();

      disconnect(&ge, SIGNAL(gotoActor(ActorProxyRefPtr)), this, SLOT(onGotoActor(ActorProxyRefPtr)));
   }

   ///////////////////////////////////////////////////////////////////////////////
   void Viewport::saveSelectedActorOrigValues(const std::string& propName)
   {
      ViewportOverlay::ActorProxyList& selection = ViewportManager::GetInstance().getViewportOverlay()->getCurrentActorSelection();
      ViewportOverlay::ActorProxyList::iterator itor;

      //Clear the old list first.
      mSelectedActorOrigValues.erase(propName);
      std::vector<std::string> savedValues;

      for (itor = selection.begin(); itor != selection.end(); ++itor)
      {
         dtDAL::ActorProxy* proxy = const_cast<dtDAL::ActorProxy*>(itor->get());
         dtDAL::ActorProperty* prop = proxy->GetProperty(propName);

         if (prop != NULL)
         {
            std::string oldValue = prop->ToString();
            savedValues.push_back(oldValue);
         }
      }

      mSelectedActorOrigValues[propName] = savedValues;
   }

   ///////////////////////////////////////////////////////////////////////////////
   void Viewport::updateActorSelectionProperty(const std::string& propName)
   {
      ViewportOverlay::ActorProxyList& selection =ViewportManager::GetInstance().getViewportOverlay()->getCurrentActorSelection();
      ViewportOverlay::ActorProxyList::iterator itor;
      std::map< std::string, std::vector<std::string> >::iterator
            saveEntry = mSelectedActorOrigValues.find(propName);
      int oldValueIndex = 0;

      // Make sure we actually saved values for this property.
      if (saveEntry == mSelectedActorOrigValues.end())
      {
         return;
      }

      for (itor = selection.begin(); itor != selection.end(); ++itor)
      {
         dtDAL::ActorProxy*    proxy = const_cast<dtDAL::ActorProxy*>(itor->get());
         dtDAL::ActorProperty* prop  = proxy->GetProperty(propName);

         if (prop != NULL)
         {
            // emit the old value before the change so undo/redo can recover.
            std::string oldValue = saveEntry->second[oldValueIndex];
            std::string newValue = prop->ToString();
            EditorEvents::GetInstance().emitActorPropertyAboutToChange(proxy, prop, oldValue, newValue);
            ++oldValueIndex;

            EditorEvents::GetInstance().emitActorPropertyChanged(proxy, prop);
         }
      }
   }

   ///////////////////////////////////////////////////////////////////////////////
   void Viewport::updateActorProxyBillboards()
   {
      dtDAL::Map* currentMap = EditorData::GetInstance().getCurrentMap();
      std::vector<dtCore::RefPtr<dtDAL::ActorProxy> > proxies;
      std::vector<dtCore::RefPtr<dtDAL::ActorProxy> >::iterator itor;

      if (currentMap == NULL || getCamera() == NULL)
      {
         return;
      }

      currentMap->GetAllProxies(proxies);
      for (itor = proxies.begin(); itor != proxies.end(); ++itor)
      {
         dtDAL::ActorProxy* proxy = itor->get();
         const dtDAL::ActorProxy::RenderMode& renderMode = proxy->GetRenderMode();

         if (renderMode == dtDAL::ActorProxy::RenderMode::DRAW_ACTOR_AND_BILLBOARD_ICON ||renderMode == dtDAL::ActorProxy::RenderMode::DRAW_BILLBOARD_ICON)
         {
            dtDAL::ActorProxyIcon* billBoard = proxy->GetBillBoardIcon();
            if (billBoard != NULL)
            {
               billBoard->SetRotation(osg::Matrix::rotate(getCamera()->getOrientation().inverse()));
            }
         }
         else if (renderMode == dtDAL::ActorProxy::RenderMode::DRAW_AUTO)
         {
         }
      }
   }

   ///////////////////////////////////////////////////////////////////////////////
   void Viewport::setAutoInteractionMode(bool on)
   {
      if (on)
      {
         if (mUseAutoInteractionMode)
         {
            return; // Already on, so do nothing.
         }
         else
         {
            mUseAutoInteractionMode = true;
            connectInteractionModeSlots();
         }
      }
      else
      {
         if (!mUseAutoInteractionMode)
         {
            return; // Already off, so do nothing.
         }
         else
         {
            mUseAutoInteractionMode = false;
            disconnectInteractionModeSlots();
         }
      }
   }

   ///////////////////////////////////////////////////////////////////////////////
   void Viewport::setupInitialRenderState()
   {
      // Some actors in the scene may have a clear node that disables screen clears
      // each frame. (The skybox for example).  Therefore, we add this node to the
      // scene to ensure a clear happens if needed.
      mClearNode = new osg::ClearNode;
      mClearNode->setRequiresClear(true);
      mClearNode->setClearColor(osg::Vec4(0.2f, 0.2f, 0.4f, 1.0f));
      osg::Group* group = getSceneView()->getSceneData()->asGroup();
      if (group != NULL)
      {
         group->addChild(mClearNode.get());
      }

      // Sets up the global state set used to render the viewport's contents.
      // This also sets up some default modes which are shared amoung
      // all viewports.
      mGlobalStateSet = new osg::StateSet();

      osg::AlphaFunc* alphafunc = new osg::AlphaFunc;
      alphafunc->setFunction(osg::AlphaFunc::GREATER, 0.0f);
      osg::PolygonMode* pm = new osg::PolygonMode();
      pm->setMode(osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::LINE);

      osg::CullFace* cf = new osg::CullFace();
      cf->setMode(osg::CullFace::BACK);

      mGlobalStateSet->setGlobalDefaults();
      mGlobalStateSet->setMode(GL_DEPTH_TEST,osg::StateAttribute::ON);
      mGlobalStateSet->setAttributeAndModes(alphafunc, osg::StateAttribute::ON);
      mGlobalStateSet->setAttributeAndModes(pm, osg::StateAttribute::ON);
      mGlobalStateSet->setAttributeAndModes(cf, osg::StateAttribute::ON);

      mSceneView->setGlobalStateSet(mGlobalStateSet.get());
   }

   ///////////////////////////////////////////////////////////////////////////////
   void Viewport::onBeginChangeTransaction()
   {
      mInChangeTransaction = true;
   }

   ///////////////////////////////////////////////////////////////////////////////
   void Viewport::onEndChangeTransaction()
   {
      mInChangeTransaction = false;
//      ViewportManager::GetInstance().refreshAllViewports();
   }

} // namespace dtEditQt
