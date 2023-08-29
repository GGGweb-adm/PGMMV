/**
* @brief その他パーツ関連データ
*/

#include "OthersData.h"
#include "ProjectData.h"

NS_AGTK_BEGIN
NS_DATA_BEGIN

//-------------------------------------------------------------------------------------------------------------------
OthersData::OthersData()
{
}

OthersData::~OthersData()
{
}

bool OthersData::init(const rapidjson::Value& json)
{
	return true;
}

OthersData::Point::Point()
{
	_switchVariableAssignList = nullptr;
	_switchId = -1;
	_switchObjectId = -1;
	_switchQualifierId = -1;
}

OthersData::Point::~Point()
{
	CC_SAFE_RELEASE_NULL(_switchVariableAssignList);
}

bool OthersData::Point::init(const rapidjson::Value& json)
{
	this->setId(json["id"].GetInt());
	this->setMove(json["move"].GetDouble());
	this->setOffsetX(json["offsetX"].GetInt());
	this->setOffsetY(json["offsetY"].GetInt());
	if (json.HasMember("switchId")) {
		_switchId = json["switchId"].GetInt();
	}
	if (json.HasMember("switchObjectId")) {
		_switchObjectId = json["switchObjectId"].GetInt();
	}
	if (json.HasMember("switchQualifierId")) {
		_switchQualifierId = json["switchQualifierId"].GetInt();
	}

	auto switchVariableAssignList = cocos2d::Array::create();
	for (rapidjson::SizeType i = 0; i < json["switchVariableAssignList"].Size(); i++)
	{
		auto switchVariableAssignData = SwitchVariableAssignData::create(json["switchVariableAssignList"][i]);
		switchVariableAssignList->addObject(switchVariableAssignData);
	}
	this->setSwitchVariableAssignList(switchVariableAssignList);

	return true;
}

//-------------------------------------------------------------------------------------------------------------------
OthersCameraData::OthersCameraData()
{
}

OthersCameraData::~OthersCameraData()
{
}

bool OthersCameraData::init(const rapidjson::Value& json)
{
	// 基底を呼び出す
	OthersData::init(json);

	// 「初期設定」を設定
	this->setInitialPlayerPosition(json["initialPlayerPosition"].GetBool());
	this->setX(json["x"].GetInt());
	this->setY(json["y"].GetInt());
	this->setWidth(json["width"].GetInt());
	this->setHeight(json["height"].GetInt());

	// 「動作設定」を設定
	this->setFollowTargetType((EnumFollowTargetType)json["followTargetType"].GetInt());
	this->setObjectId(json["objectId"].GetInt());
	this->setCourseScenePartId(json["courseScenePartId"].GetInt());
	this->setStartPointId(json["startPointId"].GetInt());
	this->setScrollToShowAllPlayers(json["scrollToShowAllPlayers"].GetBool());
	this->setScaleToShowAllPlayers(json["scaleToShowAllPlayers"].GetBool());
	this->setMaxScaling(json["maxScaling"].GetDouble());

	// 「カメラを有効にするスイッチを設定」を設定
	this->setSwitchObjectId(json["switchObjectId"].GetInt());
	this->setSwitchQualifierId(json["switchQualifierId"].GetInt());
	this->setSwitchId(json["switchId"].GetInt());

	return true;
}

//-------------------------------------------------------------------------------------------------------------------
OthersLineCourseData::OthersLineCourseData()
{
	_pointList = nullptr;
}

OthersLineCourseData::~OthersLineCourseData()
{
	CC_SAFE_RELEASE_NULL(_pointList);
}

bool OthersLineCourseData::init(const rapidjson::Value& json)
{
	// 基底を呼び出す
	OthersData::init(json);

	//
	this->setR(json["r"].GetInt());
	this->setG(json["g"].GetInt());
	this->setB(json["b"].GetInt());
	this->setA(json["a"].GetInt());

	this->setX(json["x"].GetDouble());
	this->setY(json["y"].GetDouble());

	// 「ループの設定」を設定
	this->setLoopCount(json["loopCount"].GetInt());
	this->setLoopUnlimited(json["loopUnlimited"].GetBool());
	this->setConnectEndAndStart(json["connectEndAndStart"].GetBool());
	this->setEndReverseMove(json["endReverseMove"].GetBool());

	// 「配置されているポイント」を設定
	auto pointList = cocos2d::Array::create();
	for (rapidjson::SizeType i = 0; i < json["pointList"].Size(); i++)
	{
		auto point = Point::create(json["pointList"][i]);
		pointList->addObject(point);
	}
	this->setPointList(pointList);

	return true;
}

//-------------------------------------------------------------------------------------------------------------------
OthersCurveCourseData::OthersCurveCourseData()
{
	_pointList = nullptr;
}

OthersCurveCourseData::~OthersCurveCourseData()
{
	CC_SAFE_RELEASE_NULL(_pointList);
}

bool OthersCurveCourseData::init(const rapidjson::Value& json)
{
	// 基底を呼び出す
	OthersData::init(json);

	//
	this->setR(json["r"].GetInt());
	this->setG(json["g"].GetInt());
	this->setB(json["b"].GetInt());
	this->setA(json["a"].GetInt());

	this->setX(json["x"].GetDouble());
	this->setY(json["y"].GetDouble());

	// 「ループの設定」を設定
	this->setLoopCount(json["loopCount"].GetInt());
	this->setLoopUnlimited(json["loopUnlimited"].GetBool());
	this->setConnectEndAndStart(json["connectEndAndStart"].GetBool());
	this->setEndReverseMove(json["endReverseMove"].GetBool());

	// 「配置されているポイント」を設定
	auto pointList = cocos2d::Array::create();
	for (rapidjson::SizeType i = 0; i < json["pointList"].Size(); i++)
	{
		auto point = Point::create(json["pointList"][i]);
		pointList->addObject(point);
	}

	this->setPointList(pointList);

	return true;
}

//-------------------------------------------------------------------------------------------------------------------
OthersCircleCourseData::OthersCircleCourseData()
{
	pointList = nullptr;
	_reverseCourse = false;
}

OthersCircleCourseData::~OthersCircleCourseData()
{
	CC_SAFE_RELEASE_NULL(pointList);
}

bool OthersCircleCourseData::init(const rapidjson::Value& json)
{
	// 基底を呼び出す
	OthersData::init(json);

	// 
	this->setR(json["r"].GetInt());
	this->setG(json["g"].GetInt());
	this->setB(json["b"].GetInt());
	this->setA(json["a"].GetInt());

	this->setX(json["x"].GetDouble());
	this->setY(json["y"].GetDouble());
	this->setRadiusX(json["radiusX"].GetDouble());
	this->setRadiusY(json["radiusY"].GetDouble());

	// 「ループの設定」を設定
	this->setLoopCount(json["loopCount"].GetInt());
	this->setLoopUnlimited(json["loopUnlimited"].GetBool());
	this->setEndReverseMove(json["endReverseMove"].GetBool());
	if (json.HasMember("reverseCourse")) {
		this->setReverseCourse(json["reverseCourse"].GetBool());
	}

	// 「配置されているポイント」を設定
	auto pointList = cocos2d::Array::create();
	for (rapidjson::SizeType i = 0; i < json["pointList"].Size(); i++)
	{
		auto point = Point::create(json["pointList"][i]);
		pointList->addObject(point);
	}
	this->setPointList(pointList);

	return true;
}

//-------------------------------------------------------------------------------------------------------------------
OthersSlopeData::OthersSlopeData()
{
	_dispPriority = 0;
	_moveObjectAlongSlope = true;
}

OthersSlopeData::~OthersSlopeData()
{
}

bool OthersSlopeData::init(const rapidjson::Value& json)
{
	// 基底を呼び出す
	OthersData::init(json);

	// 「表示の設定」を設定
	this->setStartX(json["startX"].GetDouble());
	this->setStartY(json["startY"].GetDouble());
	this->setEndX(json["endX"].GetDouble());
	this->setEndY(json["endY"].GetDouble());

	// 「坂専用パラメータ」を設定
	this->setPassableFromUpper(json["passableFromUpper"].GetBool());
	this->setPassableFromLower(json["passableFromLower"].GetBool());

	// 「坂に簡易効果を設定」を設定
	if (json.HasMember("objectGroupBit")) {
		this->setObjectGroupBit(json["objectGroupBit"].GetInt());
	}
	this->setMoveSpeedChanged(json["moveSpeedChanged"].GetBool());
	this->setMoveSpeedChange(json["moveSpeedChange"].GetDouble());
	this->setMoveXFlag(json["moveXFlag"].GetBool());
	this->setMoveX(json["moveX"].GetDouble());
	this->setMoveYFlag(json["moveYFlag"].GetBool());
	this->setMoveY(json["moveY"].GetDouble());
	this->setJumpChanged(json["jumpChanged"].GetBool());
	this->setJumpChange(json["jumpChange"].GetDouble());
	if (json.HasMember("moveXFlag")) {
		this->setMoveXFlag(json["moveXFlag"].GetBool());
	}
	if (json.HasMember("moveX")) {
		this->setMoveX(json["moveX"].GetDouble());
	}
	if (json.HasMember("moveYFlag")) {
		this->setMoveYFlag(json["moveYFlag"].GetBool());
	}
	if (json.HasMember("moveY")) {
		this->setMoveY(json["moveY"].GetDouble());
	}
	this->setSlipChanged(json["slipChanged"].GetBool());
	this->setSlipChange(json["slipChange"].GetDouble());
	this->setGetDead(json["getDead"].GetBool());
	this->setHpChanged(json["hpChanged"].GetBool());
	this->setHpChange(json["hpChange"].GetDouble());
	if (json.HasMember("triggerPeriodically")) {
		this->setTriggerPeriodically(json["triggerPeriodically"].GetBool());
	}
	if (json.HasMember("triggerPeriod")) {
		this->setTriggerPeriod(json["triggerPeriod"].GetDouble());
	}
	this->setGravityEffectChanged(json["gravityEffectChanged"].GetBool());
	this->setGravityEffectChange(json["gravityEffectChange"].GetInt());

	// 「物理演算用の設定」を設定
	this->setPhysicsFriction(json["physicsFriction"].GetDouble());
	this->setPhysicsRepulsion(json["physicsRepulsion"].GetDouble());

	if (json.HasMember("moveObjectAlongSlope")) {
		this->setMoveObjectAlongSlope(json["moveObjectAlongSlope"].GetBool());
	}
	return true;
}

//-------------------------------------------------------------------------------------------------------------------
OthersLoopData::OthersLoopData()
{
	_dispPriority = 0;
}

OthersLoopData::~OthersLoopData()
{
}

bool OthersLoopData::init(const rapidjson::Value& json)
{
	// 基底を呼び出す
	OthersData::init(json);
	
	// 「表示の設定」を設定
	this->setX(json["x"].GetDouble());
	this->setY(json["y"].GetDouble());
	this->setRotation(json["rotation"].GetInt());
	this->setRadius(json["radius"].GetDouble());
	this->setStartX(json["startX"].GetDouble());
	this->setStartY(json["startY"].GetDouble());
	this->setEndX(json["endX"].GetDouble());
	this->setEndY(json["endY"].GetDouble());

	// 「360度ループ用のパラメータ」を設定
	this->setNeedMoveFlag(json["needMoveFlag"].GetBool());
	this->setNeedMove(json["needMove"].GetInt());
	this->setMoveToEnd(json["moveToEnd"].GetBool());
	this->setDisabledFromRight(json["disabledFromRight"].GetBool());
	this->setDisabledFromLeft(json["disabledFromLeft"].GetBool());

	return true;
}

NS_DATA_END
NS_AGTK_END