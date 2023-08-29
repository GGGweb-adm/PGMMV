#include "CameraObject.h"

NS_AGTK_BEGIN
//-------------------------------------------------------------------------------------------------------------------
CameraObject::CameraObject()
{
	_id = -1;
	_switchValueOld = false;
	_switchValue = false;

	_cameraData = nullptr;
	_objectCourseMove = nullptr;
}

CameraObject::~CameraObject()
{
	CC_SAFE_RELEASE_NULL(_cameraData);
	CC_SAFE_RELEASE_NULL(_objectCourseMove);
}

bool CameraObject::init(int id, agtk::data::OthersCameraData* cameraData)
{
	setCameraData(cameraData);

	_id = id;

	return true;
}

void CameraObject::initCourse(int courseId, int pointId, agtk::Scene *scene)
{
	// コース移動を未生成時
	if (this->getObjectCourseMove() == nullptr) {
		// 生成する
		setObjectCourseMove(agtk::ObjectCourseMove::create(this, courseId, pointId, scene));
	}

	// コース移動のリセットを行う
	if (this->getObjectCourseMove()) {
		this->getObjectCourseMove()->reset();
	}
}

NS_AGTK_END