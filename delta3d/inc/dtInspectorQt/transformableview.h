#ifndef transformableview_h__
#define transformableview_h__

#include <dtInspectorQt/iview.h>
#include <dtCore/observerptr.h>
#include <dtCore/transformable.h>

namespace Ui
{
   class InspectorWidget;
}

namespace dtInspectorQt
{
   ///Handles the properties of dtCore::Transformable
   class TransformableView : public IView
   {
      Q_OBJECT

   public:
   	TransformableView(Ui::InspectorWidget& ui);
   	~TransformableView();

      virtual void OperateOn(dtCore::Base* b);

   protected slots:
      void OnXYZHPRChanged(double val);
      void OnCollisionDetection(int checked);
      void OnRenderCollision(int checked);
      void OnCategoryBits(const QString& text);
      void OnCollideBits(const QString& text);
      void Update();
   	
   private:
      dtCore::ObserverPtr<dtCore::Transformable> mOperateOn;
      Ui::InspectorWidget* mUI;
   };
}
#endif // transformableview_h__
