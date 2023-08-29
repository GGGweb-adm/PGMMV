#ifndef __CAMERA_OBJECT_H__
#define __CAMERA_OBJECT_H__

#include "Lib/Macros.h"
#include "Data/OthersData.h"
#include "Lib/Object.h"

NS_AGTK_BEGIN

//-------------------------------------------------------------------------------------------------------------------
class Scene;
class AGTKPLAYER_API CameraObject : public cocos2d::Node
{
private:
	CameraObject();
	virtual ~CameraObject();
	virtual bool init(int id, agtk::data::OthersCameraData* cameraData);

public:
	CREATE_FUNC_PARAM2(CameraObject, int, id, agtk::data::OthersCameraData*, cameraData);

	CC_SYNTHESIZE_RETAIN(agtk::data::OthersCameraData*, _cameraData, CameraData);

	// 追従するコースの初期化
	void initCourse(int courseId, int pointId, agtk::Scene *scene);

public:
	// ID
	CC_SYNTHESIZE_READONLY(int, _id, Id);

	// コース移動用
	CC_SYNTHESIZE_RETAIN(agtk::ObjectCourseMove*, _objectCourseMove, ObjectCourseMove);

	// 前のフレームのスイッチの値
	CC_SYNTHESIZE(bool, _switchValueOld, SwitchValueOld);

	// 今のフレームのスイッチの値
	CC_SYNTHESIZE(bool, _switchValue, SwitchValue);

};

NS_AGTK_END

#endif //__CAMERA_OBJECT_H__