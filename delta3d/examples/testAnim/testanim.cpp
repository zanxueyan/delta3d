/*
 * Delta3D Open Source Game and Simulation Engine
 * Copyright (C) 2006, Alion Science and Technology
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
 * Bradley Anderegg
 */
#include "testanim.h"
#include "testaniminput.h"

#include <dtUtil/hotspotdefinition.h>
#include <dtUtil/mathdefines.h>

#include <dtCore/globals.h>
#include <dtCore/flymotionmodel.h>
#include <dtCore/camera.h>
#include <dtCore/deltawin.h>
#include <dtCore/scene.h>
#include <dtCore/system.h>
#include <dtCore/object.h>

#include <dtDAL/map.h>
#include <dtDAL/actorproxy.h>
#include <dtDAL/actorproxy.h>
#include <dtDAL/project.h>

#include <dtGame/gamemanager.h>
#include <dtGame/gameapplication.h>
#include <dtGame/logcontroller.h> 
#include <dtGame/gameactor.h>
#include <dtGame/defaultmessageprocessor.h>

#include <dtAnim/animationcomponent.h>
#include <dtAnim/animationhelper.h>
#include <dtAnim/attachmentcontroller.h>
#include <dtAnim/cal3dmodelwrapper.h>
#include <dtAnim/cal3danimator.h>
#include <dtAnim/sequencemixer.h>
#include <dtAnim/animatable.h>

#include <dtActors/animationgameactor2.h>
#include <dtActors/engineactorregistry.h>

#include <osg/Geode>
#include <osg/ShapeDrawable>
#include <osg/Shape>


extern "C" TEST_ANIM_EXPORT dtGame::GameEntryPoint* CreateGameEntryPoint()
{
   return new TestAnim;
}
 
//////////////////////////////////////////////////////////////////////////
extern "C" TEST_ANIM_EXPORT void DestroyGameEntryPoint(dtGame::GameEntryPoint* entryPoint)
{
   delete entryPoint;
}

//////////////////////////////////////////////////////////////////////////
TestAnim::TestAnim()
: dtGame::GameEntryPoint()
, mAnimationHelper(NULL)
, mFMM(NULL)
, mPerformanceTest(false)
{
   
}

//////////////////////////////////////////////////////////////////////////
TestAnim::~TestAnim()
{

}

//////////////////////////////////////////////////////////////////////////
void TestAnim::Initialize(dtGame::GameApplication& app, int argc, char **argv)
{
   if(mPerformanceTest)
   {
      mFMM = new dtCore::FlyMotionModel(app.GetKeyboard(), app.GetMouse());
      mFMM->SetTarget(app.GetCamera());
   }
}

//////////////////////////////////////////////////////////////////////////
dtCore::ObserverPtr<dtGame::GameManager> TestAnim::CreateGameManager(dtCore::Scene& scene)
{ 
   return dtGame::GameEntryPoint::CreateGameManager(scene);
}


//////////////////////////////////////////////////////////////////////////
void TestAnim::OnStartup()
{
   GetGameManager()->GetApplication().GetWindow()->SetWindowTitle("TestAnim");

   std::string dataPath = dtCore::GetDeltaDataPathList();
   dtCore::SetDataFilePathList(dataPath + ";" + 
                               dtCore::GetDeltaRootPath() + "/examples/data" + ";");

   std::string context = dtCore::GetDeltaRootPath() + "/examples/data/demoMap";

   typedef std::vector<dtDAL::ActorProxy* > ProxyContainer;
   ProxyContainer proxies;
   ProxyContainer groundActor;

   dtGame::GameManager& gameManager = *GetGameManager();

   try
   {
      dtDAL::Project::GetInstance().SetContext(context, true);
      gameManager.ChangeMap("TestAnim");
      dtCore::System::GetInstance().Start();
      dtCore::System::GetInstance().Step();
      dtCore::System::GetInstance().Step();
      gameManager.FindActorsByName("CharacterEntity", proxies);
      gameManager.FindActorsByName("GroundActor", groundActor);
   }
   catch (dtUtil::Exception& e)
   {
      LOG_ERROR("Can't find the project context or load the map. Exception follows.");
      e.LogException(dtUtil::Log::LOG_ERROR);
   }

   dtGame::DefaultMessageProcessor *dmp = new dtGame::DefaultMessageProcessor();   
   
   dtAnim::AnimationComponent* animComp = new dtAnim::AnimationComponent();

   gameManager.AddComponent(*dmp,dtGame::GameManager::ComponentPriority::HIGHEST);   
   gameManager.AddComponent(*animComp, dtGame::GameManager::ComponentPriority::NORMAL);
  
   if(!mPerformanceTest)
   {
      TestAnimInput* inputComp = new TestAnimInput("TestAnimInput"); 
      gameManager.AddComponent(*inputComp, dtGame::GameManager::ComponentPriority::NORMAL);

      ProxyContainer::iterator iter = proxies.begin();
      ProxyContainer::iterator endIter = proxies.end();

      for(;iter != endIter; ++iter)
      {
         dtActors::AnimationGameActorProxy2* gameProxy = dynamic_cast<dtActors::AnimationGameActorProxy2*>(*iter);

         if(gameProxy != NULL)
         {         
            static bool first = true;

            //the first one will be the player
            InitializeAnimationActor(gameProxy, animComp, true);

            if(first)
            {
               inputComp->SetAnimationHelper(*mAnimationHelper);
               inputComp->SetPlayerActor(*gameProxy);
               first = false;
            }         
         }
      }
   }
   else
   {
      osg::Vec3 startPos(0.0f, 0.0f, 10.0f);

      for(int i = 0; i < 10; ++i, startPos[0] += 2.0f)
      {
         for(int j = 0; j < 10; ++j, startPos[1] += 2.0f)
         {
            dtCore::RefPtr<dtActors::AnimationGameActorProxy2> proxy;
            GetGameManager()->CreateActor(*dtActors::EngineActorRegistry::ANIMATION_ACTOR_TYPE2, proxy);
            if(proxy.valid())
            {
               GetGameManager()->AddActor(*proxy);

               dtActors::AnimationGameActor2* actor = dynamic_cast<dtActors::AnimationGameActor2*>(&proxy->GetGameActor());
               actor->SetModel("SkeletalMeshes/marine.xml");
               InitializeAnimationActor(proxy.get(), animComp, false);

               proxy->SetTranslation(startPos);
            }
         }
         startPos[1] = 0.0f;
      }
      GetGameManager()->GetApplication().GetCamera()->SetNextStatisticsType();

   }

   if(!groundActor.empty())
   {
      dtDAL::ActorProxy* proxy = dynamic_cast<dtDAL::ActorProxy*>(groundActor.front());
      if(proxy)
      {
         dtCore::Transformable* transform = dynamic_cast<dtCore::Transformable*>(proxy->GetActor());
         if(transform)
         {
            animComp->SetTerrainActor(transform);
         }
      }
   }
   else
   {
      LOG_ERROR("Cannot find ground");
   }

   gameManager.DebugStatisticsTurnOn(false, false, 5);

   gameManager.GetApplication().GetWindow()->SetKeyRepeat(false);
}

void TestAnim::OnShutdown()
{    
   const std::string mapName = GetGameManager()->GetCurrentMap();

   dtDAL::Map& map = dtDAL::Project::GetInstance().GetMap(mapName);
   dtDAL::Project::GetInstance().CloseMap(map, true);
}

void TestAnim::InitializeAnimationActor(dtActors::AnimationGameActorProxy2* gameProxy, dtAnim::AnimationComponent* animComp, bool isPlayer)
{   
      dtActors::AnimationGameActor2* actor = dynamic_cast<dtActors::AnimationGameActor2*>(&gameProxy->GetGameActor());

      if(actor != NULL)
      {
         dtAnim::AnimationHelper* helper = actor->GetHelper();         

         if(helper != NULL)
         {
            //we must register the helper with the animation component
            animComp->RegisterActor(*gameProxy, *helper);                  


            //since we are doing hardware skinning there is no need for 
            //the physique driver
            //TODO: this should be refactored out of here and into the place that decides 
            //      whether or not we are doing hardware skinning
            helper->GetAnimator()->SetPhysiqueDriver(NULL);

            if(isPlayer)
            {
               //set camera offset
               dtCore::Transform trans;
               trans.SetTranslation(-1.0f, 5.5f, 1.5f);
               trans.SetRotation(180.0f, -2.0f, 0.0f); 

               mAnimationHelper = helper;
               mAnimationHelper->SetGroundClamp(true);

               actor->AddChild(GetGameManager()->GetApplication().GetCamera());
               GetGameManager()->GetApplication().GetCamera()->SetTransform(trans, dtCore::Transformable::REL_CS);

               dtCore::RefPtr<osg::ShapeDrawable> drawable = new osg::ShapeDrawable(new osg::Cylinder(osg::Vec3(), 0.02f, 0.3));
               dtCore::RefPtr<osg::Geode> geode = new osg::Geode;
               geode->addDrawable(drawable.get());

               //std::vector<std::string> bones;
               //mAnimationHelper->GetModelWrapper()->GetCoreBoneNames(bones);
               //if (bones.size() > 0)
               //{
               //  hotspotDef.mParentName = bones.front();

               dtCore::RefPtr<dtCore::Transformable> attachment = new dtCore::Transformable;
               attachment->GetOSGNode()->asGroup()->addChild(geode.get());

               //dtCore::RefPtr<dtCore::Object> attachment = new dtCore::Object("Arrow");
               //attachment->LoadFile(dtCore::GetDeltaRootPath() + "/examples/data/models/arrow.ive");

               dtUtil::HotSpotDefinition hotspotDef;
               hotspotDef.mName = "jojo";
               hotspotDef.mParentName = "Bip02 Head";
               hotspotDef.mLocalTranslation = osg::Vec3(0.1f, 0.0f, 0.0f);

               mAnimationHelper->GetAttachmentController().AddAttachment(*attachment, hotspotDef);
               actor->AddChild(attachment.get());

               }
            else
            {
               helper->PlayAnimation("Walk");
               helper->GetSequenceMixer().GetActiveAnimation("Walk")->SetStartDelay(dtUtil::RandFloat(20.0f, 40.0f));
               helper->GetSequenceMixer().ForceRecalculate();
            }
         }
      }
}

