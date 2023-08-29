/**
* @brief シーン関連データ
*/
#include "SceneData.h"
#include "Data/TileData.h"
#include "Data/ObjectCommandData.h"

NS_AGTK_BEGIN
NS_DATA_BEGIN
//-------------------------------------------------------------------------------------------------------------------
//! 物理パーツベース
/**
* コンストラクタ
*/
PhysicsBaseData::PhysicsBaseData()
{
	_placementType = EnumPlacement::kPlacementOriginalSize;
	_nonCollisionGroup = nullptr;
}

/**
* デストラクタ
*/
PhysicsBaseData::~PhysicsBaseData()
{
	CC_SAFE_RELEASE_NULL(_nonCollisionGroup);
}

/**
* 初期化
* @param	json	初期化用JSONデータ
* @return			初期化の成否
*/
bool PhysicsBaseData::init(const rapidjson::Value& json)
{
	ASSIGN_JSON_MEMBER("coloring", Coloring, Bool);

	ASSIGN_JSON_MEMBER("a", ColorA, Int);
	ASSIGN_JSON_MEMBER("r", ColorR, Int);
	ASSIGN_JSON_MEMBER("g", ColorG, Int);
	ASSIGN_JSON_MEMBER("b", ColorB, Int);

	ASSIGN_JSON_MEMBER("imageId", ImageId, Int);
	ASSIGN_JSON_MEMBER_WITH_CAST("placementType", PlacementType, Int, EnumPlacement);
	ASSIGN_JSON_MEMBER("placementX", PlacementX, Int);
	ASSIGN_JSON_MEMBER("placementY", PlacementY, Int);
	ASSIGN_JSON_MEMBER("scaling", Scaling, Double);

	auto arr = cocos2d::__Array::create();
	if (json.HasMember("nonCollisionGroup")) {
		for (rapidjson::SizeType i = 0; i < json["nonCollisionGroup"].Size(); i++) {
			auto p = cocos2d::Integer::create(json["nonCollisionGroup"][i].GetInt());
			CC_ASSERT(p);
			arr->addObject(p);
		}
	}
	this->setNonCollisionGroup(arr);

	ASSIGN_JSON_MEMBER("invisible", Invisible, Bool);

	return true;
}

#if defined(AGTK_DEBUG)
/**
* ダンプ
*/
void PhysicsBaseData::dump()
{
	CCLOG("coloring: %s", DUMP_BOOLTEXT(this->getColoring()));

	CCLOG("a: %d", this->getColorA());
	CCLOG("r: %d", this->getColorR());
	CCLOG("g: %d", this->getColorG());
	CCLOG("b: %d", this->getColorB());

	CCLOG("imageId: %d", this->getImageId());
	CCLOG("placementType: %d", this->getPlacementType());
	CCLOG("placementX: %d", this->getPlacementX());
	CCLOG("placementY: %d", this->getPlacementY());
	CCLOG("scaling: %f", this->getScaling());

	CCLOG("nonCollisionGroup: [");
	cocos2d::Ref * ref = nullptr;
	CCARRAY_FOREACH(this->getNonCollisionGroup(), ref) {
		auto p = dynamic_cast<cocos2d::Integer *>(ref);
		CCLOG("%d", p->getValue());
	}
	CCLOG("]");

	CCLOG("invisible: %s", DUMP_BOOLTEXT(this->getInvisible()));
}
#endif

//-------------------------------------------------------------------------------------------------------------------
//! 物理パーツ：円のデータ
/**
* コンストラクタ
*/
PhysicsDiskData::PhysicsDiskData() : PhysicsBaseData()
{
}

/**
* デストラクタ
*/
PhysicsDiskData::~PhysicsDiskData()
{
}

/**
* 初期化
* @param	json	初期化用JSONデータ
* @return			初期化の成否
*/
bool PhysicsDiskData::init(const rapidjson::Value& json)
{
	if (!PhysicsBaseData::init(json)) {
		return false;
	}

	ASSIGN_JSON_MEMBER("x", X, Double);
	ASSIGN_JSON_MEMBER("y", Y, Double);
	ASSIGN_JSON_MEMBER("width", Width, Double);
	ASSIGN_JSON_MEMBER("height", Height, Double);
	ASSIGN_JSON_MEMBER("rotation", Rotation, Double);

	ASSIGN_JSON_MEMBER("density", Density, Double);
	ASSIGN_JSON_MEMBER("mass", Mass, Double);
	ASSIGN_JSON_MEMBER("friction", Friction, Double);
	ASSIGN_JSON_MEMBER("repulsion", Repulsion, Double);

	return true;
}

#if defined(AGTK_DEBUG)
/**
* ダンプ
*/
void PhysicsDiskData::dump()
{
	PhysicsBaseData::dump();

	CCLOG("x: %f", this->getX());
	CCLOG("y: %f", this->getY());
	CCLOG("width: %f", this->getWidth());
	CCLOG("height: %f", this->getHeight());
	CCLOG("rotation: %f", this->getRotation());

	CCLOG("density: %f", this->getDensity());
	CCLOG("mass: %f", this->getMass());
	CCLOG("friction: %f", this->getFriction());
	CCLOG("repulsion: %f", this->getRepulsion());
}
#endif

//-------------------------------------------------------------------------------------------------------------------
//! 物理パーツ：四角形のデータ
/**
* コンストラクタ
*/
PhysicsRectangleData::PhysicsRectangleData() : PhysicsDiskData()
{
}

/**
* デストラクタ
*/
PhysicsRectangleData::~PhysicsRectangleData()
{
}

/**
* 初期化
* @param	json	初期化用JSONデータ
* @return			初期化の成否
*/
bool PhysicsRectangleData::init(const rapidjson::Value& json)
{
	if (!PhysicsDiskData::init(json)) {
		return false;
	}

	return true;
}

#if defined(AGTK_DEBUG)
/**
* ダンプ
*/
void PhysicsRectangleData::dump()
{
	PhysicsDiskData::dump();
}
#endif

//-------------------------------------------------------------------------------------------------------------------
//! 物理パーツ：接着のデータ
/**
* コンストラクタ
*/
PhysicsPinData::PhysicsPinData() : PhysicsBaseData()
{
	_upperSubId = -1;
	_lowerSubId = -1;
}

/**
* デストラクタ
*/
PhysicsPinData::~PhysicsPinData()
{
}

/**
* 初期化
* @param	json	初期化用JSONデータ
* @return			初期化の成否
*/
bool PhysicsPinData::init(const rapidjson::Value& json)
{
	if (!PhysicsBaseData::init(json)) {
		return false;
	}

	ASSIGN_JSON_MEMBER("width", Width, Double);
	ASSIGN_JSON_MEMBER("height", Height, Double);

	ASSIGN_JSON_MEMBER("upperId", UpperId, Int);
	if (json.HasMember("upperSubId")) {
		ASSIGN_JSON_MEMBER("upperSubId", UpperSubId, Int);
	}
	ASSIGN_JSON_MEMBER("upperX", UpperX, Int);
	ASSIGN_JSON_MEMBER("upperY", UpperY, Int);
	ASSIGN_JSON_MEMBER("lowerId", LowerId, Int);
	if (json.HasMember("lowerSubId")) {
		ASSIGN_JSON_MEMBER("lowerSubId", LowerSubId, Int);
	}
	ASSIGN_JSON_MEMBER("lowerX", LowerX, Int);
	ASSIGN_JSON_MEMBER("lowerY", LowerY, Int);

	return true;
}

#if defined(AGTK_DEBUG)
/**
* ダンプ
*/
void PhysicsPinData::dump()
{
	PhysicsBaseData::dump();

	CCLOG("width: %f", this->getWidth());
	CCLOG("height: %f", this->getHeight());

	CCLOG("upperId: %d", this->getUpperId());
	CCLOG("upperSubId: %d", this->getUpperSubId());
	CCLOG("upperX: %d", this->getUpperX());
	CCLOG("upperY: %d", this->getUpperY());
	CCLOG("lowerId: %d", this->getLowerId());
	CCLOG("lowerSubId: %d", this->getLowerSubId());
	CCLOG("lowerX: %d", this->getLowerX());
	CCLOG("lowerY: %d", this->getLowerY());
}
#endif

//-------------------------------------------------------------------------------------------------------------------
//! 物理パーツ：ロープのデータ
/**
* コンストラクタ
*/
PhysicsRopeData::PhysicsRopeData() : PhysicsBaseData()
{
	this->_pointList = nullptr;
	this->_connectSubId1 = -1;
	this->_connectSubId2 = -1;
}

/**
* デストラクタ
*/
PhysicsRopeData::~PhysicsRopeData()
{
	CC_SAFE_RELEASE_NULL(_pointList);
}

/**
* 初期化
* @param	json	初期化用JSONデータ
* @return			初期化の成否
*/
bool PhysicsRopeData::init(const rapidjson::Value& json)
{
	if (!PhysicsBaseData::init(json)) {
		return false;
	}

	ASSIGN_JSON_MEMBER("length", Length, Double);
	ASSIGN_JSON_MEMBER("width", Width, Double);

	auto arr = cocos2d::__Array::create();
	for (rapidjson::SizeType i = 0; i < json["pointList"].Size(); i++) {
		auto arr2 = cocos2d::__Array::create();
		arr2->addObject(cocos2d::Double::create(json["pointList"][i][POINT_X].GetDouble()));
		arr2->addObject(cocos2d::Double::create(json["pointList"][i][POINT_Y].GetDouble()));
		arr->addObject(arr2);
	}
	this->setPointList(arr);

	ASSIGN_JSON_MEMBER("connectId1", ConnectId1, Int);
	if (json.HasMember("connectSubId1")) {
		ASSIGN_JSON_MEMBER("connectSubId1", ConnectSubId1, Int);
	}
	ASSIGN_JSON_MEMBER("connectX1", ConnectX1, Int);
	ASSIGN_JSON_MEMBER("connectY1", ConnectY1, Int);
	ASSIGN_JSON_MEMBER("connectId2", ConnectId2, Int);
	if (json.HasMember("connectSubId2")) {
		ASSIGN_JSON_MEMBER("connectSubId2", ConnectSubId2, Int);
	}
	ASSIGN_JSON_MEMBER("connectX2", ConnectX2, Int);
	ASSIGN_JSON_MEMBER("connectY2", ConnectY2, Int);

	return true;
}

#if defined(AGTK_DEBUG)
/**
* ダンプ
*/
void PhysicsRopeData::dump()
{
	PhysicsBaseData::dump();

	CCLOG("length: %f", this->getLength());
	CCLOG("width: %f", this->getWidth());

	CCLOG("connectId1: %d", this->getConnectId1());
	CCLOG("connectSubId1: %d", this->getConnectSubId1());
	CCLOG("connectX1: %d", this->getConnectX1());
	CCLOG("connectY1: %d", this->getConnectY1());
	CCLOG("connectId2: %d", this->getConnectId2());
	CCLOG("connectSubId2: %d", this->getConnectSubId2());
	CCLOG("connectX2: %d", this->getConnectX2());
	CCLOG("connectY2: %d", this->getConnectY2());
}
#endif

//-------------------------------------------------------------------------------------------------------------------
//! 物理パーツ：バネのデータ
/**
* コンストラクタ
*/
PhysicsSpringData::PhysicsSpringData() : PhysicsBaseData()
{
	this->_connectSubId1 = -1;
	this->_connectSubId2 = -1;
}

/**
* デストラクタ
*/
PhysicsSpringData::~PhysicsSpringData()
{
}

/**
* 初期化
* @param	json	初期化用JSONデータ
* @return			初期化の成否
*/
bool PhysicsSpringData::init(const rapidjson::Value& json)
{
	if (!PhysicsBaseData::init(json)) {
		return false;
	}

	ASSIGN_JSON_MEMBER("fixConnectAngle1", FixConnectAngle1, Bool);
	ASSIGN_JSON_MEMBER("fixConnectAngle2", FixConnectAngle2, Bool);
	ASSIGN_JSON_MEMBER("springConstant", SpringConstant, Double);
	ASSIGN_JSON_MEMBER("dampingCoefficient", DampingCoefficient, Double);
	ASSIGN_JSON_MEMBER("naturalLength", NaturalLength, Double);
	ASSIGN_JSON_MEMBER("width", Width, Double);

	ASSIGN_JSON_MEMBER("connectId1", ConnectId1, Int);
	if (json.HasMember("connectSubId1")) {
		ASSIGN_JSON_MEMBER("connectSubId1", ConnectSubId1, Int);
	}
	ASSIGN_JSON_MEMBER("connectX1", ConnectX1, Int);
	ASSIGN_JSON_MEMBER("connectY1", ConnectY1, Int);
	ASSIGN_JSON_MEMBER("connectId2", ConnectId2, Int);
	if (json.HasMember("connectSubId2")) {
		ASSIGN_JSON_MEMBER("connectSubId2", ConnectSubId2, Int);
	}
	ASSIGN_JSON_MEMBER("connectX2", ConnectX2, Int);
	ASSIGN_JSON_MEMBER("connectY2", ConnectY2, Int);

	return true;
}

#if defined(AGTK_DEBUG)
/**
* ダンプ
*/
void PhysicsSpringData::dump()
{
	PhysicsBaseData::dump();

	CCLOG("fixConnectAngle1: %s", DUMP_BOOLTEXT(this->getFixConnectAngle1()));
	CCLOG("fixConnectAngle2: %s", DUMP_BOOLTEXT(this->getFixConnectAngle1()));
	CCLOG("springConstant: %f", this->getSpringConstant());
	CCLOG("dampingCoefficient: %f", this->getDampingCoefficient());
	CCLOG("naturalLength: %f", this->getNaturalLength());
	CCLOG("width: %f", this->getWidth());

	CCLOG("connectId1: %d", this->getConnectId1());
	CCLOG("connectSubId1: %d", this->getConnectSubId1());
	CCLOG("connectX1: %d", this->getConnectX1());
	CCLOG("connectY1: %d", this->getConnectY1());
	CCLOG("connectId2: %d", this->getConnectId2());
	CCLOG("connectSubId2: %d", this->getConnectSubId2());
	CCLOG("connectX2: %d", this->getConnectX2());
	CCLOG("connectY2: %d", this->getConnectY2());
}
#endif

//-------------------------------------------------------------------------------------------------------------------
//! 物理パーツ：回転軸のデータ
/**
* コンストラクタ
*/
PhysicsAxisData::PhysicsAxisData() : PhysicsBaseData()
{
	_upperSubId = -1;
	_lowerSubId = -1;
}

/**
* デストラクタ
*/
PhysicsAxisData::~PhysicsAxisData()
{
}

/**
* 初期化
* @param	json	初期化用JSONデータ
* @return			初期化の成否
*/
bool PhysicsAxisData::init(const rapidjson::Value& json)
{
	if (!PhysicsBaseData::init(json)) {
		return false;
	}

	ASSIGN_JSON_MEMBER("width", Width, Double);
	ASSIGN_JSON_MEMBER("height", Height, Double);

	ASSIGN_JSON_MEMBER("addRightRotation", AddRightRotation, Bool);
	ASSIGN_JSON_MEMBER("reverseDirection", ReverseDirection, Bool);
	ASSIGN_JSON_MEMBER("addBrakeFunction", AddBrakeFunction, Bool);
	ASSIGN_JSON_MEMBER("enableBySwitch", EnableBySwitch, Bool);
	ASSIGN_JSON_MEMBER("rpm", Rpm, Double);
	ASSIGN_JSON_MEMBER("torque", Torque, Double);
	ASSIGN_JSON_MEMBER("dampingRatio", DampingRatio, Double);

	ASSIGN_JSON_MEMBER("rightRotationSwitchId", RightRotationSwitchId, Int);
	ASSIGN_JSON_MEMBER("rightRotationSwitchObjectId", RightRotationSwitchObjectId, Int);
	ASSIGN_JSON_MEMBER("rightRotationSwitchQualifierId", RightRotationSwitchQualifierId, Int);

	ASSIGN_JSON_MEMBER("leftRotationSwitchId", LeftRotationSwitchId, Int);
	ASSIGN_JSON_MEMBER("leftRotationSwitchObjectId", LeftRotationSwitchObjectId, Int);
	ASSIGN_JSON_MEMBER("leftRotationSwitchQualifierId", LeftRotationSwitchQualifierId, Int);

	ASSIGN_JSON_MEMBER("brakeSwitchId", BrakeSwitchId, Int);
	ASSIGN_JSON_MEMBER("brakeSwitchObjectId", BrakeSwitchObjectId, Int);
	ASSIGN_JSON_MEMBER("brakeSwitchQualifierId", BrakeSwitchQualifierId, Int);

	ASSIGN_JSON_MEMBER("upperId", UpperId, Int);
	if (json.HasMember("upperSubId")) {
		ASSIGN_JSON_MEMBER("upperSubId", UpperSubId, Int);
	}
	ASSIGN_JSON_MEMBER("upperX", UpperX, Int);
	ASSIGN_JSON_MEMBER("upperY", UpperY, Int);
	ASSIGN_JSON_MEMBER("lowerId", LowerId, Int);
	if (json.HasMember("lowerSubId")) {
		ASSIGN_JSON_MEMBER("lowerSubId", LowerSubId, Int);
	}
	ASSIGN_JSON_MEMBER("lowerX", LowerX, Int);
	ASSIGN_JSON_MEMBER("lowerY", LowerY, Int);

	ASSIGN_JSON_MEMBER("useVerificationKey", UseVerificationKey, Bool);
	ASSIGN_JSON_MEMBER("rightRotationKeyId", RightRotationKeyId, Int);
	ASSIGN_JSON_MEMBER("leftRotationKeyId", LeftRotationKeyId, Int);
	ASSIGN_JSON_MEMBER("brakeKeyId", BrakeKeyId, Int);

	return true;
}

#if defined(AGTK_DEBUG)
/**
* ダンプ
*/
void PhysicsAxisData::dump()
{
	PhysicsBaseData::dump();

	CCLOG("width: %f", this->getWidth());
	CCLOG("height: %f", this->getHeight());

	CCLOG("addRightRotation: %s", DUMP_BOOLTEXT(this->getAddRightRotation()));
	CCLOG("reverseDirection: %s", DUMP_BOOLTEXT(this->getReverseDirection()));
	CCLOG("addBrakeFunction: %s", DUMP_BOOLTEXT(this->getAddBrakeFunction()));
	CCLOG("enableBySwitch: %s", DUMP_BOOLTEXT(this->getEnableBySwitch()));
	CCLOG("rpm: %f", this->getRpm());
	CCLOG("torque: %f", this->getTorque());
	CCLOG("dampingRatio: %f", this->getDampingRatio());

	CCLOG("rightRotationSwitchId: %d", this->getRightRotationSwitchId());
	CCLOG("rightRotationSwitchObjectId: %d", this->getRightRotationSwitchObjectId());
	CCLOG("rightRotationSwitchQualifierId: %d", this->getRightRotationSwitchQualifierId());

	CCLOG("leftRotationSwitchId: %d", this->getLeftRotationSwitchId());
	CCLOG("leftRotationSwitchObjectId: %d", this->getLeftRotationSwitchObjectId());
	CCLOG("leftRotationSwitchQualifierId: %d", this->getLeftRotationSwitchQualifierId());

	CCLOG("brakeSwitchId: %d", this->getBrakeSwitchId());
	CCLOG("brakeSwitchObjectId: %d", this->getBrakeSwitchObjectId());
	CCLOG("brakeSwitchQualifierId: %d", this->getBrakeSwitchQualifierId());

	CCLOG("upperId: %d", this->getUpperId());
	CCLOG("upperSubId: %d", this->getUpperSubId());
	CCLOG("upperX: %d", this->getUpperX());
	CCLOG("upperY: %d", this->getUpperY());
	CCLOG("lowerId: %d", this->getLowerId());
	CCLOG("lowerSubId: %d", this->getLowerSubId());
	CCLOG("lowerX: %d", this->getLowerX());
	CCLOG("lowerY: %d", this->getLowerY());

	CCLOG("useVerificationKey: %s", DUMP_BOOLTEXT(this->getUseVerificationKey()));
	CCLOG("rightRotationKeyId: %d", this->getRightRotationKeyId());
	CCLOG("leftRotationKeyId: %d", this->getLeftRotationKeyId());
	CCLOG("brakeKeyId: %d", this->getBrakeKeyId());
}
#endif

//-------------------------------------------------------------------------------------------------------------------
//! 物理パーツ：力学系基本データ
/**
* コンストラクタ
*/
PhysicsForceBaseData::PhysicsForceBaseData()
{
	_connectSubId = -1;
}

/**
* デストラクタ
*/
PhysicsForceBaseData::~PhysicsForceBaseData()
{
}

/**
* 初期化
* @param	json	初期化用JSONデータ
* @return			初期化の成否
*/
bool PhysicsForceBaseData::init(const rapidjson::Value& json)
{
	ASSIGN_JSON_MEMBER("rotateAccordingToConnectedObject", RotateAccordingToConnectedObject, Bool);
	ASSIGN_JSON_MEMBER("limitDirectionRange", LimitDirectionRange, Bool);
	ASSIGN_JSON_MEMBER("enableBySwitch", EnableBySwitch, Bool);

	ASSIGN_JSON_MEMBER("strength", Strength, Double);
	ASSIGN_JSON_MEMBER("direction", Direction, Double);

	ASSIGN_JSON_MEMBER_WITH_CAST("rangeType", rangeType, Int, EnumRangeType);
	ASSIGN_JSON_MEMBER("fanAngle", FanAngle, Double);
	ASSIGN_JSON_MEMBER("bandWidth", BandWidth, Double);

	ASSIGN_JSON_MEMBER("connectId", ConnectId, Int);
	if (json.HasMember("connectSubId")) {
		ASSIGN_JSON_MEMBER("connectSubId", ConnectSubId, Int);
	}
	ASSIGN_JSON_MEMBER("connectX", ConnectX, Int);
	ASSIGN_JSON_MEMBER("connectY", ConnectY, Int);

	ASSIGN_JSON_MEMBER("useVerificationKey", UseVerificationKey, Bool);

	return true;
}

#if defined(AGTK_DEBUG)
/**
* ダンプ
*/
void PhysicsForceBaseData::dump()
{
	CCLOG("rangeType: %d", this->getrangeType());
	CCLOG("fanAngle: %f", this->getFanAngle());
	CCLOG("bandWidth: %f", this->getBandWidth());

	CCLOG("rotateAccordingToConnectedObject: %s", DUMP_BOOLTEXT(this->getRotateAccordingToConnectedObject()));

	CCLOG("connectId: %d", this->getConnectId());
	CCLOG("connectSubId: %d", this->getConnectSubId());
	CCLOG("connectX: %d", this->getConnectX());
	CCLOG("connectY: %d", this->getConnectY());

	CCLOG("useVerificationKey: %s", DUMP_BOOLTEXT(this->getUseVerificationKey()));

}
#endif

//-------------------------------------------------------------------------------------------------------------------
//! 物理パーツ：爆発のデータ
/**
* コンストラクタ
*/
PhysicsExplosionData::PhysicsExplosionData() : PhysicsForceBaseData()
{
}

/**
* デストラクタ
*/
PhysicsExplosionData::~PhysicsExplosionData()
{
}

/**
* 初期化
* @param	json	初期化用JSONデータ
* @return			初期化の成否
*/
bool PhysicsExplosionData::init(const rapidjson::Value& json)
{
	if (!PhysicsForceBaseData::init(json)) {
		return false;
	}

	ASSIGN_JSON_MEMBER("duration300", Duration300, Int);

	ASSIGN_JSON_MEMBER("effectiveDistance", EffectiveDistance, Double);
	ASSIGN_JSON_MEMBER("effectiveInfinite", EffectiveInfinite, Bool);

	ASSIGN_JSON_MEMBER("switchId", SwitchId, Int);
	ASSIGN_JSON_MEMBER("switchObjectId", SwitchObjectId, Int);
	ASSIGN_JSON_MEMBER("switchQualifierId", SwitchQualifierId, Int);

	ASSIGN_JSON_MEMBER("keyId", KeyId, Int);

	return true;
}

#if defined(AGTK_DEBUG)
/**
* ダンプ
*/
void PhysicsExplosionData::dump()
{
	PhysicsForceBaseData::dump();

	CCLOG("duration300: %d", this->getDuration300());
	CCLOG("effectiveDistance: %d", this->getEffectiveDistance());
	CCLOG("effectiveInfinite: %s", DUMP_BOOLTEXT(this->getEffectiveInfinite()));

	CCLOG("switchId: %d", this->getSwitchId());
	CCLOG("switchObjectId: %d", this->getSwitchObjectId());
	CCLOG("switchQualifierId: %d", this->getSwitchQualifierId());

	CCLOG("keyId: %d", this->getKeyId());
}
#endif

//-------------------------------------------------------------------------------------------------------------------
//! 物理パーツ：引力・斥力のデータ
/**
* コンストラクタ
*/
PhysicsForceData::PhysicsForceData() : PhysicsForceBaseData()
{
}

/**
* デストラクタ
*/
PhysicsForceData::~PhysicsForceData()
{
}

/**
* 初期化
* @param	json	初期化用JSONデータ
* @return			初期化の成否
*/
bool PhysicsForceData::init(const rapidjson::Value& json)
{
	if (!PhysicsForceBaseData::init(json)) {
		return false;
	}

	ASSIGN_JSON_MEMBER("attractiveForce", AttractiveForce, Bool);
	ASSIGN_JSON_MEMBER("constantForce", ConstantForce, Bool);
	ASSIGN_JSON_MEMBER("distance", Distance, Double);
	ASSIGN_JSON_MEMBER("infinite", Infinite, Bool);

	ASSIGN_JSON_MEMBER("attractiveForceSwitchId", AttractiveForceSwitchId, Int);
	ASSIGN_JSON_MEMBER("attractiveForceSwitchObjectId", AttractiveForceSwitchObjectId, Int);
	ASSIGN_JSON_MEMBER("attractiveForceSwitchQualifierId", AttractiveForceSwitchQualifierId, Int);

	ASSIGN_JSON_MEMBER("repulsiveForceSwitchId", RepulsiveForceSwitchId, Int);
	ASSIGN_JSON_MEMBER("repulsiveForceSwitchObjectId", RepulsiveForceSwitchObjectId, Int);
	ASSIGN_JSON_MEMBER("repulsiveForceSwitchQualifierId", RepulsiveForceSwitchQualifierId, Int);

	ASSIGN_JSON_MEMBER("attractiveForceKeyId", AttractiveForceKeyId, Int);
	ASSIGN_JSON_MEMBER("repulsiveForceKeyId", RepulsiveForceKeyId, Int);

	return true;
}

#if defined(AGTK_DEBUG)
/**
* ダンプ
*/
void PhysicsForceData::dump()
{
	PhysicsForceBaseData::dump();

	CCLOG("attractiveForce: %s", DUMP_BOOLTEXT(this->getAttractiveForce()));
	CCLOG("constantForce: %s", DUMP_BOOLTEXT(this->getConstantForce()));
	CCLOG("distance: %d", this->getDistance());
	CCLOG("infinite: %s", DUMP_BOOLTEXT(this->getInfinite()));

	CCLOG("attractiveForceSwitchId: %d", this->getAttractiveForceSwitchId());
	CCLOG("attractiveForceSwitchObjectId: %d", this->getAttractiveForceSwitchObjectId());
	CCLOG("attractiveForceSwitchQualifierId: %d", this->getAttractiveForceSwitchQualifierId());

	CCLOG("repulsiveForceSwitchId: %d", this->getRepulsiveForceSwitchId());
	CCLOG("repulsiveForceSwitchObjectId: %d", this->getRepulsiveForceSwitchObjectId());
	CCLOG("repulsiveForceSwitchQualifierId: %d", this->getRepulsiveForceSwitchQualifierId());

	CCLOG("attractiveForceKeyId: %d", this->getAttractiveForceKeyId());
	CCLOG("repulsiveForceKeyId: %d", this->getRepulsiveForceKeyId());
}
#endif

//-------------------------------------------------------------------------------------------------------------------
//! 物理パーツデータ
/**
* コンストラクタ
*/
PhysicsPartData::PhysicsPartData()
{
	_physicsData = nullptr;
	_name = nullptr;
	_dispPriority = 0;
}

/**
* デストラクタ
*/
PhysicsPartData::~PhysicsPartData()
{
	CC_SAFE_RELEASE_NULL(_physicsData);
	CC_SAFE_RELEASE_NULL(_name);
}

/**
* 初期化
* @param	json	初期化用JSONデータ
* @return			初期化の成否
*/
bool PhysicsPartData::init(const rapidjson::Value& json)
{
	ASSIGN_JSON_MEMBER("id", Id, Int);
	ASSIGN_JSON_MEMBER("partType", PartType, Int);
	ASSIGN_JSON_MEMBER("layerIndex", LayerIndex, Int);
	ASSIGN_JSON_MEMBER_STR("name", Name, String);
	ASSIGN_JSON_MEMBER("priority", Priority, Int);
	if (json.HasMember("dispPriority")) {
		ASSIGN_JSON_MEMBER("dispPriority", DispPriority, Int);
	}
	ASSIGN_JSON_MEMBER_WITH_CAST("type", Type, Int, EnumPhysicsType);

	cocos2d::Ref *ptr = nullptr;
	switch (this->getType()) {
	case EnumPhysicsType::kDisk:
		ptr = PhysicsDiskData::create(json["disk"]);
		break;
	case EnumPhysicsType::kRectangle:
		ptr = PhysicsRectangleData::create(json["rectangle"]);
		break;
	case EnumPhysicsType::kPin:
		ptr = PhysicsPinData::create(json["pin"]);
		break;
	case EnumPhysicsType::kRope:
		ptr = PhysicsRopeData::create(json["rope"]);
		break;
	case EnumPhysicsType::kSpring:
		ptr = PhysicsSpringData::create(json["spring"]);
		break;
	case EnumPhysicsType::kAxis:
		ptr = PhysicsAxisData::create(json["axis"]);
		break;
	case EnumPhysicsType::kExplosion:
		ptr = PhysicsExplosionData::create(json["explosion"]);
		break;
	case EnumPhysicsType::kForce:
		ptr = PhysicsForceData::create(json["force"]);
		break;
	}
	CC_ASSERT(ptr);
	this->setPhysicsData(ptr);

	return true;
}

#if defined(AGTK_DEBUG)
/**
* ダンプ
*/
void PhysicsPartData::dump()
{
	CCLOG("id: %d", this->getId());
	CCLOG("partType: %d", this->getPartType());
	CCLOG("layerIndex: %d", this->getLayerIndex());
	CCLOG("name: %s", this->getName()->getCString());
	CCLOG("priority: %d", this->getPriority());
	CCLOG("type: %d", this->getType());

	auto data = this->getPhysicsData();
	switch (this->getType()) {
	case EnumPhysicsType::kDisk:
		dynamic_cast<PhysicsDiskData *>(data)->dump();
		break;
	case EnumPhysicsType::kRectangle:
		dynamic_cast<PhysicsRectangleData *>(data)->dump();
		break;
	case EnumPhysicsType::kPin:
		dynamic_cast<PhysicsPinData *>(data)->dump();
		break;
	case EnumPhysicsType::kRope:
		dynamic_cast<PhysicsRopeData *>(data)->dump();
		break;
	case EnumPhysicsType::kSpring:
		dynamic_cast<PhysicsSpringData *>(data)->dump();
		break;
	case EnumPhysicsType::kAxis:
		dynamic_cast<PhysicsAxisData *>(data)->dump();
		break;
	case EnumPhysicsType::kExplosion:
		dynamic_cast<PhysicsExplosionData *>(data)->dump();
		break;
	case EnumPhysicsType::kForce:
		dynamic_cast<PhysicsForceData *>(data)->dump();
		break;
	}
}
#endif

//-------------------------------------------------------------------------------------------------------------------
LayerData::LayerData()
{
	_tileList = nullptr;
	_name = nullptr;
	_layerId = 0;
}

LayerData::~LayerData()
{
	CC_SAFE_RELEASE_NULL(_tileList);
	CC_SAFE_RELEASE_NULL(_name);
}

bool LayerData::init(const rapidjson::Value& json, int layerId)
{
	CC_ASSERT(json.HasMember("tile"));
	auto arr = cocos2d::__Array::create();
	const rapidjson::Value& tile = json["tile"];
	for (rapidjson::Value::ConstMemberIterator itr = tile.MemberBegin(); itr != tile.MemberEnd(); itr++) {
		auto data = Tile::create(itr->value, itr->name.GetString());
		arr->addObject(data);
	}

	// タイルのデータは不完全な昇順ソートの為ここでソートし直す
	auto compare = [](cocos2d::Ref *a, cocos2d::Ref *b) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto tileA = static_cast<Tile *>(a);
		auto tileB = static_cast<Tile *>(b);
#else
		auto tileA = dynamic_cast<Tile *>(a);
		auto tileB = dynamic_cast<Tile *>(b);
#endif
		return (tileA->getPosition().x < tileB->getPosition().x || (tileA->getPosition().x == tileB->getPosition().x && tileA->getPosition().y < tileB->getPosition().y));
	};
	std::sort(arr->data->arr, arr->data->arr + arr->data->num, compare);

	this->setName(cocos2d::__String::create(json["name"].GetString()));
	this->setTileList(arr);
	this->setLayerId(layerId);
	return true;
}

const char *LayerData::getName()
{
	CC_ASSERT(_name);
	return _name->getCString();
}

#if defined(AGTK_DEBUG)
void LayerData::dump()
{
	cocos2d::Ref *ref = nullptr;
	CCARRAY_FOREACH(this->getTileList(), ref) {
		auto tile = dynamic_cast<Tile *>(ref);
		tile->dump();
	}
	CCLOG("layerId:%d", this->getLayerId());
	CCLOG("name:%s", this->getName());
}
#endif

//-------------------------------------------------------------------------------------------------------------------
ScenePartData::ScenePartData()
{
	_name = nullptr;
	_objectChildren = nullptr;
	_physicsChildren = nullptr;
	_othersChildren = nullptr;
	_portalChildren = nullptr;
	_dispPriority = 0;
}

ScenePartData::~ScenePartData()
{
	CC_SAFE_RELEASE_NULL(_name);
	CC_SAFE_RELEASE_NULL(_objectChildren);
	CC_SAFE_RELEASE_NULL(_physicsChildren);
	CC_SAFE_RELEASE_NULL(_othersChildren);
	CC_SAFE_RELEASE_NULL(_portalChildren);
}

bool ScenePartData::init(const rapidjson::Value& json)
{
	this->setId(json["id"].GetInt());
	this->setName(cocos2d::__String::create(json["name"].GetString()));
	this->setFolder(json["folder"].GetBool());
	if (this->getFolder()) {
		auto objectDic = cocos2d::__Dictionary::create();
		auto physicsDic = cocos2d::__Dictionary::create();
		auto othersDic = cocos2d::__Dictionary::create();
		auto portalDic = cocos2d::__Dictionary::create();

		if (json.HasMember("children")) {
			for (rapidjson::SizeType i = 0; i < json["children"].Size(); i++) {

				switch (json["children"][i]["partType"].GetInt())
				{
					// オブジェクト
					case ScenePartData::kPartObject:
					{
						auto p = ScenePartObjectData::create(json["children"][i]);
#if defined(AGTK_DEBUG)
						CC_ASSERT(objectDic->objectForKey(p->getId()) == nullptr);
#endif
						objectDic->setObject(p, p->getId());
					} break;

					// 物理演算
					case ScenePartData::kPartPhysics:
					{
						auto p = PhysicsPartData::create(json["children"][i]);
#if defined(AGTK_DEBUG)
						CC_ASSERT(physicsDic->objectForKey(p->getId()) == nullptr);
#endif
						physicsDic->setObject(p, p->getId());
					} break;

					// その他
					case ScenePartData::kPartOthers:
					{
						auto p = ScenePartOthersData::create(json["children"][i]);
#if defined(AGTK_DEBUG)
						CC_ASSERT(othersDic->objectForKey(p->getId()) == nullptr);
#endif
						othersDic->setObject(p, p->getId());
					} break;

					// ポータル確認
					case ScenePartData::kPartPortal:
					{
						// エディタ専用の項目
					} break;
				}

				if (json["children"][i]["partType"].GetInt() != kPartObject) continue;	//todo: 通常オブジェクト以外に要対応。
				auto p = ScenePartObjectData::create(json["children"][i]);
			}
		}

		this->setObjectChildren(objectDic);
		this->setPhysicsChildren(physicsDic);
		this->setOthersChildren(othersDic);
		this->setPortalChildren(portalDic);

		return true;
	}
	this->setPartType((EnumPartType)json["partType"].GetInt());
	this->setLayerIndex(json["layerIndex"].GetInt());
	this->setVisible(json["visible"].GetBool());
	this->setLocked(json["locked"].GetBool());
	this->setPriority(json["priority"].GetInt());
	if (json.HasMember("dispPriority")) {
		this->setDispPriority(json["dispPriority"].GetInt());
	}

	return true;
}

const char *ScenePartData::getName()
{
	CC_ASSERT(_name);
	return _name->getCString();
}

#if defined(AGTK_DEBUG)
void ScenePartData::dump()
{
	CCLOG("id:%d", this->getId());
	CCLOG("name:%s", this->getName());
	CCLOG("folder:%d", this->getFolder());
	if (this->getFolder() && this->getObjectChildren()) {
		CCLOG("-- ObjectPartData --");
		auto keys = this->getObjectChildren()->allKeys();
		cocos2d::Ref *ref = nullptr;
		CCARRAY_FOREACH(keys, ref) {
			auto id = dynamic_cast<cocos2d::Integer *>(ref);
			auto data = dynamic_cast<ScenePartData *>(this->getObjectChildren()->objectForKey(id->getValue()));
			data->dump();
		}
		return;
	}
	if (this->getFolder() && this->getPhysicsChildren()) {
		CCLOG("-- physicsPartData --");
		auto keys = this->getPhysicsChildren()->allKeys();
		cocos2d::Ref *ref = nullptr;
		CCARRAY_FOREACH(keys, ref) {
			auto id = dynamic_cast<cocos2d::Integer *>(ref);
			auto data = dynamic_cast<PhysicsPartData *>(this->getPhysicsChildren()->objectForKey(id->getValue()));
			data->dump();
		}
		return;
	}
}
#endif

//-------------------------------------------------------------------------------------------------------------------
ScenePartObjectData::ScenePartObjectData()
{
	_actionCommandListObject = nullptr;
	_commonActionCommandListObject = nullptr;
	_disappearActionCommandList = nullptr;
	_startPointGroupIndex = -1;
	_startPointPlayerBit = 0;
	_physicsPartList = nullptr;
}

ScenePartObjectData::~ScenePartObjectData()
{
	CC_SAFE_RELEASE_NULL(_actionCommandListObject);
	CC_SAFE_RELEASE_NULL(_commonActionCommandListObject);
	CC_SAFE_RELEASE_NULL(_disappearActionCommandList);
	CC_SAFE_RELEASE_NULL(_physicsPartList);
}

bool ScenePartObjectData::init(const rapidjson::Value& json)
{
	// 基底処理を呼び出す
	if (ScenePartData::init(json) == false) {
		return false;
	}

	this->setObjectId(json["objectId"].GetInt());
	if (this->getObjectId() == START_POINT_OBJECT_ID) {
		this->setStartPointGroupIndex(json["startPointGroupIndex"].GetInt());
		this->setStartPointPlayerBit(json["startPointPlayerBit"].GetInt());
	}
	this->setInitialActionId(json["initialActionId"].GetInt());
	if (json.HasMember("initialDisplayDirectionFlag")) {
		//CCLOG("initialDisplayDirectionFlag:%d", json.HasMember("initialDisplayDirectionFlag"));
		this->setInitialDisplayDirectionFlag(json["initialDisplayDirectionFlag"].GetBool());
		this->setInitialDisplayDirection(json["initialDisplayDirection"].GetDouble());
	}
	this->setX(json["x"].GetDouble());
	this->setY(json["y"].GetDouble());
	this->setScalingX(json["scalingX"].GetDouble());
	this->setScalingY(json["scalingY"].GetDouble());
	this->setRotation(json["rotation"].GetDouble());

	auto actionCommandListObjectList = cocos2d::__Dictionary::create();
	CC_ASSERT(json.HasMember("actionCommandListObject"));
	const rapidjson::Value& actionCommandListObject = json["actionCommandListObject"];
	for (rapidjson::Value::ConstMemberIterator itr = actionCommandListObject.MemberBegin(); itr != actionCommandListObject.MemberEnd(); itr++) {
		auto name = itr->name.GetString();
		auto objCommandList = cocos2d::__Dictionary::create();
		for (rapidjson::SizeType i = 0; i < itr->value.Size(); i++) {
			auto objCommandData = ObjectCommandData::create(itr->value[i]);
			objCommandList->setObject(objCommandData, objCommandData->getId());
		}
		actionCommandListObjectList->setObject(objCommandList, std::atoi(name));
	}
	this->setActionCommandListObject(actionCommandListObjectList);

	auto commonActionCommandListObjectList = cocos2d::__Dictionary::create();
	CC_ASSERT(json.HasMember("commonActionCommandListObject"));
	const rapidjson::Value& commonActionCommandListObject = json["commonActionCommandListObject"];
	for (rapidjson::Value::ConstMemberIterator itr = commonActionCommandListObject.MemberBegin(); itr != commonActionCommandListObject.MemberEnd(); itr++) {
		auto name = itr->name.GetString();
		auto objCommandList = cocos2d::__Dictionary::create();
		for (rapidjson::SizeType i = 0; i < itr->value.Size(); i++) {
			auto objCommandData = ObjectCommandData::create(itr->value[i]);
			objCommandList->setObject(objCommandData, objCommandData->getId());
		}
		commonActionCommandListObjectList->setObject(objCommandList, std::atoi(name));
	}
	this->setCommonActionCommandListObject(commonActionCommandListObjectList);

	auto disappearActionCommandList = cocos2d::__Dictionary::create();
	CC_ASSERT(json.HasMember("disappearActionCommandList"));
	for (rapidjson::SizeType i = 0; i < json["disappearActionCommandList"].Size(); i++) {
		auto objCommandData = ObjectCommandData::create(json["disappearActionCommandList"][i]);
		disappearActionCommandList->setObject(objCommandData, objCommandData->getId());
	}
	this->setDisappearActionCommandList(disappearActionCommandList);

	if (json.HasMember("physicsPartList")) {
		auto physicsPartList = cocos2d::__Dictionary::create();
		for (rapidjson::SizeType i = 0; i < json["physicsPartList"].Size(); i++) {
			auto physicsPartData = PhysicsPartData::create(json["physicsPartList"][i]);
#if defined(AGTK_DEBUG)
			CC_ASSERT(physicsPartList->objectForKey(physicsPartData->getPriority()) == nullptr);
#endif
			physicsPartList->setObject(physicsPartData, physicsPartData->getPriority());
		}
		this->setPhysicsPartList(physicsPartList);
	}

	this->setCourseScenePartId(json["courseScenePartId"].GetInt());
	this->setCourseStartPointId(json["courseStartPointId"].GetInt());
	if (this->getCourseStartPointId() < 0) {
		this->setCourseScenePartId(-1);
	}

	return true;
}

bool ScenePartObjectData::isStartPointObjectData()
{
	return (this->getObjectId() == START_POINT_OBJECT_ID);
}

#if defined(AGTK_DEBUG)
void ScenePartObjectData::dump()
{
	// 基底処理を呼び出す
	ScenePartData::dump();

	CCLOG("objectId:%d", this->getObjectId());
	CCLOG("initialActionId:%d", this->getInitialActionId());
	CCLOG("initialDisplayDirectionFlag:%d", this->getInitialDisplayDirectionFlag());
	CCLOG("initialDisplayDirection:%f", this->getInitialDisplayDirection());
	CCLOG("x:%f", this->getX());
	CCLOG("y:%f", this->getY());
	CCLOG("layerIndex:%d", this->getLayerIndex());
	CCLOG("scalingX:%f", this->getScalingX());
	CCLOG("scalingY:%f", this->getScalingY());
	CCLOG("rotatin:%f", this->getRotation());
	CCLOG("courseScenePartId:%d", this->getCourseScenePartId());
	CCLOG("courseStartPointId:%d", this->getCourseStartPointId());

	CCLOG("actionCommandListObject");
	auto keys = this->getActionCommandListObject()->allKeys();
	cocos2d::Ref *ref = nullptr;
	CCARRAY_FOREACH(keys, ref) {
		auto id = dynamic_cast<cocos2d::Integer *>(ref);
		auto list = dynamic_cast<cocos2d::__Dictionary *>(this->getActionCommandListObject()->objectForKey(id->getValue()));
		cocos2d::DictElement *el = nullptr;
		CCDICT_FOREACH(list, el) {
			auto data = dynamic_cast<agtk::data::ObjectCommandData *>(el->getObject());
			data->dump();
		}
	}
	CCLOG("commonActionCommandListObject");
	keys = this->getCommonActionCommandListObject()->allKeys();
	ref = nullptr;
	CCARRAY_FOREACH(keys, ref) {
		auto id = dynamic_cast<cocos2d::Integer *>(ref);
		auto list = dynamic_cast<cocos2d::__Dictionary *>(this->getCommonActionCommandListObject()->objectForKey(id->getValue()));
		cocos2d::DictElement *el = nullptr;
		CCDICT_FOREACH(list, el) {
			auto data = dynamic_cast<agtk::data::ObjectCommandData *>(el->getObject());
			data->dump();
		}
	}
	CCLOG("disappearActionCommandList");
	auto disappearActionCommandList = this->getDisappearActionCommandList();
	cocos2d::DictElement *el = nullptr;
	CCDICT_FOREACH(disappearActionCommandList, el) {
		auto p = dynamic_cast<ObjectCommandData *>(el->getObject());
		p->dump();
	}
}
#endif

//-------------------------------------------------------------------------------------------------------------------
ScenePartOthersData::ScenePartOthersData()
{
	_part = nullptr;
}

ScenePartOthersData::~ScenePartOthersData()
{
	CC_SAFE_RELEASE_NULL(_part);
}

bool ScenePartOthersData::init(const rapidjson::Value& json)
{
	// 基底処理を呼び出す
	ScenePartData::init(json);

	this->setOthersType((OthersType)json["othersType"].GetInt());

	OthersData *part = nullptr;
	switch (getOthersType())
	{
		// カメラ
		case OthersType::kOthersCamera:
		{
			part = OthersCameraData::create(json["camera"]);
		} break;

		// コース（直線）
		case OthersType::kOthersLineCourse:
		{
			part = OthersLineCourseData::create(json["lineCourse"]);
		} break;

		// コース（曲線）
		case OthersType::kOthersCurveCourse:
		{
			part = OthersCurveCourseData::create(json["curveCourse"]);
		} break;

		// コース（円）
		case OthersType::kOthersCircleCourse:
		{
			part = OthersCircleCourseData::create(json["circleCourse"]);
		} break;

		// 坂を設置
		case OthersType::kOthersSlope:
		{
			part = OthersSlopeData::create(json["slope"]);
			if (json.HasMember("dispPriority")) {
				((OthersSlopeData *)part)->setDispPriority(json["dispPriority"].GetInt());
			}
		} break;

		// 360度ループを設置
		case OthersType::kOthersLoop:
		{
			part = OthersLoopData::create(json["loop"]);
			if (json.HasMember("dispPriority")) {
				((OthersLoopData *)part)->setDispPriority(json["dispPriority"].GetInt());
			}
		} break;
	}
	this->setPart(part);

	return true;
}

#if defined(AGTK_DEBUG)
void ScenePartOthersData::dump()
{
	// 基底処理を呼び出す
	ScenePartData::dump();

}
#endif

//-------------------------------------------------------------------------------------------------------------------
SceneFilterEffectData::SceneFilterEffectData()
{
	_filterEffect = nullptr;
	_id = 0;
	_layerIndex = 0;
	_disabled = false;
}

SceneFilterEffectData::~SceneFilterEffectData()
{
	CC_SAFE_RELEASE_NULL(_filterEffect);
}

bool SceneFilterEffectData::init(const rapidjson::Value& json)
{
	this->setId(json["id"].GetInt());
	this->setLayerIndex(json["layerIndex"].GetInt());
	this->setDisabled(json["disabled"].GetBool());
	auto filterEffect = FilterEffect::create(json["filterEffect"]);
	if (filterEffect == nullptr) {
		CC_ASSERT(0);
		return false;
	}
	this->setFilterEffect(filterEffect);
	return true;
}

void SceneFilterEffectData::setFilterEffect(FilterEffect *filterEffect)
{
	CC_SAFE_RELEASE_NULL(_filterEffect);
	if (filterEffect) {
		CC_SAFE_RETAIN(filterEffect);
		_filterEffect = filterEffect;
	}
}

#if defined(AGTK_DEBUG)
void SceneFilterEffectData::dump()
{
	CCLOG("id:%d", this->getId());
	CCLOG("layerIndex:%d", this->getLayerIndex());
	CCLOG("disabled:%d", this->getDisabled());
	this->getFilterEffect()->dump();
}
#endif

//-------------------------------------------------------------------------------------------------------------------
SceneData::SceneData()
{
	_id = 0;
	_name = nullptr;
	_folder = false;
	_children = nullptr;
	_horzScreenCount = 0;
	_vertScreenCount = 0;
	_tileset = nullptr;//->cocos2d::Integer
	_memo = nullptr;
	_playBgmType = kPlayBgmNone;
	_bgmId = -1;
	_loopBgmFlag = false;
	_bgColor = cocos2d::Color3B(0, 0, 0);
	_setBgImageFlag = false;
	_bgImageId = 0;
	_bgImageMoveSpeed = 0;
	_bgImageMoveDirection = 0;
	_bgMoveSpeedX = 0;
	_bgMoveSpeedY = 0;
	_layerMoveSpeedX = nullptr;//->cocos2d::Double
	_layerMoveSpeedY = nullptr;//->cocos2d::Double
	_gravityAccel = 0.0f;
	_airResistance = 0.0f;
	_gravityDirection = 0.0f;
	_screenAutoScroll = false;
	_screenAutoScrollSpeed = 0;
	_screenAutoScrollDirection = 0;
	_screenAutoScrollSwitchObjectId = 0;
	_screenAutoScrollSwitchQualifierId = 0;
	_screenAutoScrollSwitchId = 0;
	_verticalLoop = false;
	_horizontalLoop = false;
	_limitAreaX = 0.0f;
	_limitAreaY = 0.0f;
	_limitAreaWidth = 0.0f;
	_limitAreaHeight = 0.0f;
	_disableLimitArea = false;
	_limitCameraRect = cocos2d::Rect(0, 0, 0, 0);//limitCameraX,limitCameraY,limitCameraWidth,limitCameraHeight
	_disableLimitCamera = false;
	_layerList = nullptr;//->LayerData(layer1,layer2,layer3,layer4)
	_scenePartObjectList = nullptr;//->ScenePartObjectData
	_scenePartPhysicsList = nullptr;//->ScenePartPhysicsData
	_scenePartOthersList = nullptr;//->ScenePartOthersData
	_scenePartPortalList = nullptr;//->ScenePartPortalData
	_sceneFilterEffectList = nullptr;//->FilterEffect
	_objectSingleIdHash = nullptr;//->cocos2d::Integer
	_initialMenuLayerId = -1;
	_preloadMenuLayerIdList = nullptr;//->cocos2d::Integer
}

SceneData::~SceneData()
{
	CC_SAFE_RELEASE_NULL(_name);
	CC_SAFE_RELEASE_NULL(_children);
	CC_SAFE_RELEASE_NULL(_tileset);
	CC_SAFE_RELEASE_NULL(_memo);
	CC_SAFE_RELEASE_NULL(_layerMoveSpeedX);
	CC_SAFE_RELEASE_NULL(_layerMoveSpeedY);
	CC_SAFE_RELEASE_NULL(_layerList);
	CC_SAFE_RELEASE_NULL(_scenePartObjectList);
	CC_SAFE_RELEASE_NULL(_scenePartPhysicsList);
	CC_SAFE_RELEASE_NULL(_scenePartOthersList);
	CC_SAFE_RELEASE_NULL(_scenePartObjectList);
	CC_SAFE_RELEASE_NULL(_scenePartPortalList);
	CC_SAFE_RELEASE_NULL(_sceneFilterEffectList);
	CC_SAFE_RELEASE_NULL(_preloadMenuLayerIdList);
}

bool SceneData::init(const rapidjson::Value& json, cocos2d::Size tileSize)
{
	CC_ASSERT(json.HasMember("id"));
	this->setId(json["id"].GetInt());
	CC_ASSERT(json.HasMember("name"));
	this->setName(cocos2d::__String::create(json["name"].GetString()));
	CC_ASSERT(json.HasMember("folder"));
	this->setFolder(json["folder"].GetBool());
	if (this->getFolder()) {
		auto dic = cocos2d::__Dictionary::create();
		if (json.HasMember("children")) {
			for (rapidjson::SizeType i = 0; i < json["children"].Size(); i++) {
				auto p = SceneData::create(json["children"][i], tileSize);
#if defined(AGTK_DEBUG)
				CC_ASSERT(dic->objectForKey(p->getId()) == nullptr);
#endif
				dic->setObject(p, p->getId());
			}
		}
		this->setChildren(dic);
		return true;
	}
	CC_ASSERT(json.HasMember("horzScreenCount"));
	this->setHorzScreenCount(json["horzScreenCount"].GetInt());
	CC_ASSERT(json.HasMember("vertScreenCount"));
	this->setVertScreenCount(json["vertScreenCount"].GetInt());
	CC_ASSERT(json.HasMember("tileset"));
	auto tileset = cocos2d::__Array::create();
	for (rapidjson::SizeType i = 0; i < json["tileset"].Size(); i++) {
		tileset->addObject(cocos2d::Integer::create(json["tileset"][i].GetInt()));
	}
	this->setTileset(tileset);
	CC_ASSERT(json.HasMember("memo"));
	this->setMemo(cocos2d::__String::create(json["memo"].GetString()));
	CC_ASSERT(json.HasMember("playBgmType"));
	this->setPlayBgmType((EnumPlayBgmType)json["playBgmType"].GetInt());
	if (json.HasMember("bgmId")){
		this->setBgmId(json["bgmId"].GetInt());
	}
	CC_ASSERT(json.HasMember("loopBgmFlag"));
	this->setLoopBgmFlag(json["loopBgmFlag"].GetBool());
	CC_ASSERT(json.HasMember("bgColorR"));
	CC_ASSERT(json.HasMember("bgColorG"));
	CC_ASSERT(json.HasMember("bgColorB"));
	this->setBgColor(cocos2d::Color3B(
		json["bgColorR"].GetInt(),
		json["bgColorG"].GetInt(),
		json["bgColorB"].GetInt()
	));
	CC_ASSERT(json.HasMember("setBgImageFlag"));
	this->setSetBgImageFlag(json["setBgImageFlag"].GetBool());
	CC_ASSERT(json.HasMember("bgImageId"));
	this->setBgImageId(json["bgImageId"].GetInt());
	CC_ASSERT(json.HasMember("bgImagePlacement"));
	this->setBgImagePlacement((EnumBgImagePlacement)json["bgImagePlacement"].GetInt());
	CC_ASSERT(json.HasMember("bgImageMoveSpeed"));
	this->setBgImageMoveSpeed(json["bgImageMoveSpeed"].GetDouble());
	CC_ASSERT(json.HasMember("bgImageMoveDirection"));
	this->setBgImageMoveDirection(json["bgImageMoveDirection"].GetDouble());
	if (json.HasMember("bgMoveSpeed")) {
		this->setBgMoveSpeedX(json["bgMoveSpeed"].GetDouble());
	}
	if (json.HasMember("bgMoveSpeedY")) {
		this->setBgMoveSpeedY(json["bgMoveSpeedY"].GetDouble());
	} else {
		this->setBgMoveSpeedY(this->getBgMoveSpeedX());
	}
	CC_ASSERT(json.HasMember("layerMoveSpeed"));
	auto layerMoveSpeedX = cocos2d::__Array::create();
	for (rapidjson::SizeType i = 0; i < json["layerMoveSpeed"].Size(); i++) {
		layerMoveSpeedX->addObject(cocos2d::Double::create(json["layerMoveSpeed"][i].GetDouble()));
	}
	this->setLayerMoveSpeedX(layerMoveSpeedX);
	auto layerMoveSpeedY = cocos2d::__Array::create();
	const char *keyName = "layerMoveSpeed";
	if (json.HasMember("layerMoveSpeedY")) {
		keyName = "layerMoveSpeedY";
	}
	for (rapidjson::SizeType i = 0; i < json[keyName].Size(); i++) {
		layerMoveSpeedY->addObject(cocos2d::Double::create(json[keyName][i].GetDouble()));
	}
	this->setLayerMoveSpeedY(layerMoveSpeedY);
	CC_ASSERT(json.HasMember("airResistance"));
	this->setAirResistance(json["airResistance"].GetDouble());
	CC_ASSERT(json.HasMember("gravityAccel"));
	this->setGravityAccel(json["gravityAccel"].GetDouble());
	CC_ASSERT(json.HasMember("gravityDirection"));
	this->setGravityDirection(json["gravityDirection"].GetDouble());
	CC_ASSERT(json.HasMember("screenAutoScroll"));
	this->setScreenAutoScroll(json["screenAutoScroll"].GetBool());
	CC_ASSERT(json.HasMember("screenAutoScrollSpeed"));
	this->setScreenAutoScrollSpeed(json["screenAutoScrollSpeed"].GetDouble());
	CC_ASSERT(json.HasMember("screenAutoScrollDirection"));
	this->setScreenAutoScrollDirection(json["screenAutoScrollDirection"].GetDouble());
	CC_ASSERT(json.HasMember("screenAutoScrollSwitchObjectId"));
	this->setScreenAutoScrollSwitchObjectId(json["screenAutoScrollSwitchObjectId"].GetInt());
	CC_ASSERT(json.HasMember("screenAutoScrollSwitchQualifierId"));
	this->setScreenAutoScrollSwitchQualifierId(json["screenAutoScrollSwitchQualifierId"].GetInt());
	CC_ASSERT(json.HasMember("screenAutoScrollSwitchId"));
	this->setScreenAutoScrollSwitchId(json["screenAutoScrollSwitchId"].GetInt());
	this->setVerticalLoop(false);
	this->setHorizontalLoop(false);
	CC_ASSERT(json.HasMember("limitAreaX"));
	this->setLimitAreaX(json["limitAreaX"].GetInt());
	CC_ASSERT(json.HasMember("limitAreaY"));
	this->setLimitAreaY(json["limitAreaY"].GetInt());
	CC_ASSERT(json.HasMember("limitAreaWidth"));
	this->setLimitAreaWidth(json["limitAreaWidth"].GetInt());
	CC_ASSERT(json.HasMember("limitAreaHeight"));
	this->setLimitAreaHeight(json["limitAreaHeight"].GetInt());
	if (json.HasMember("disableLimitArea")) {
		this->setDisableLimitArea(json["disableLimitArea"].GetBool());
	}
	CC_ASSERT(json.HasMember("limitCameraX"));
	CC_ASSERT(json.HasMember("limitCameraY"));
	CC_ASSERT(json.HasMember("limitCameraWidth"));
	CC_ASSERT(json.HasMember("limitCameraHeight"));
	this->setLimitCameraRect(cocos2d::Rect(
		json["limitCameraX"].GetInt(),
		json["limitCameraY"].GetInt(),
		json["limitCameraWidth"].GetInt(),
		json["limitCameraHeight"].GetInt()
	));
	if (json.HasMember("disableLimitCamera")) {
		this->setDisableLimitCamera(json["disableLimitCamera"].GetBool());
	}
	if (json.HasMember("initialMenuLayerId")) {
		this->setInitialMenuLayerId(json["initialMenuLayerId"].GetInt());
	}
	if (json.HasMember("preloadMenuLayerIdList")) {
		auto preloadMenuLayerIdList = cocos2d::__Array::create();
		for (rapidjson::SizeType i = 0; i < json["preloadMenuLayerIdList"].Size(); i++) {
			auto id = json["preloadMenuLayerIdList"][i].GetInt();
			if (id == 0) {
				//レイヤー選択無し。
				continue;
			}
			preloadMenuLayerIdList->addObject(cocos2d::Integer::create(id));
		}
		this->setPreloadMenuLayerIdList(preloadMenuLayerIdList);
	}
	auto layerList = cocos2d::__Dictionary::create();
	int layerId = 1;
	while (1) {
		std::string layerName = "layer";
		layerName += std::to_string(layerId);
		if (json.HasMember(layerName.c_str()) == false) {
			break;
		}
		auto layerData = LayerData::create(json[layerName.c_str()], layerId);
		if (layerData == nullptr) {
			return false;
		}
		layerList->setObject(layerData, layerId);
		layerId++;
	}
	if (getId() == kMenuSceneId) {
		{
			//暗黙の最前面メニューレイヤーを追加する。
			layerId = kHudMenuLayerId;
			rapidjson::Document doc;
			doc.Parse("{ \"name\": \"HUD\", \"tile\" : { } }");
			bool error = doc.HasParseError();
			if (error) {
				CCASSERT(0, "Error: Json Parse.");
				return false;
			}
			auto layerData = LayerData::create(doc, layerId);
			if (layerData == nullptr) {
				return false;
			}
			layerList->setObject(layerData, layerId);
		}
		++layerId;
		{
			//暗黙の最前面レイヤーを追加する。
			layerId = kHudTopMostLayerId;
			rapidjson::Document doc;
			doc.Parse("{ \"name\": \"TOPMOST\", \"tile\" : { } }");
			bool error = doc.HasParseError();
			if (error) {
				CCASSERT(0, "Error: Json Parse.");
				return false;
			}
			auto layerData = LayerData::create(doc, layerId);
			if (layerData == nullptr) {
				return false;
			}
			layerList->setObject(layerData, layerId);
		}
	}
	this->setLayerList(layerList);
	//ScenePartsList
	CC_ASSERT(json.HasMember("scenePartList"));
	auto scenePartObjectList = cocos2d::__Dictionary::create();
	auto scenePartPhysicsList = cocos2d::__Dictionary::create();
	auto scenePartOthersList = cocos2d::__Dictionary::create();
	auto scenePartPortalList = cocos2d::__Dictionary::create();


#if 1	//フォルダーに仮対応
	//TODO: フォルダー構造をPlayer側で扱う必要があるなら要対応。
	std::function<void(const rapidjson::Value &)> setScenePartDataRecur = [&setScenePartDataRecur, scenePartObjectList, scenePartPhysicsList, scenePartOthersList, scenePartPortalList](const rapidjson::Value &jsonList)
	{
		for (rapidjson::SizeType i = 0; i < jsonList.Size(); i++) {
			if (jsonList[i]["folder"].GetBool()) {
				// 子供がある場合
				if (jsonList[i].HasMember("children")) {
					setScenePartDataRecur(jsonList[i]["children"]);
				}
				continue;
			}

			switch (jsonList[i]["partType"].GetInt())
			{
				// オブジェクト
				case ScenePartData::kPartObject:
				{
					auto scenePartObjectData = ScenePartObjectData::create(jsonList[i]);
#if defined(AGTK_DEBUG)
					CC_ASSERT(scenePartObjectList->objectForKey(scenePartObjectData->getId()) == nullptr);
#endif
					scenePartObjectList->setObject(scenePartObjectData, scenePartObjectData->getId());
				} break;

				// 物理演算
				case ScenePartData::kPartPhysics:
				{
					auto physicsPartData = PhysicsPartData::create(jsonList[i]);
#if defined(AGTK_DEBUG)
					CC_ASSERT(scenePartPhysicsList->objectForKey(physicsPartData->getId()) == nullptr);
#endif
					scenePartPhysicsList->setObject(physicsPartData, physicsPartData->getId());
				} break;

				// その他
				case ScenePartData::kPartOthers:
				{
					auto scenePartOthersData = ScenePartOthersData::create(jsonList[i]);
#if defined(AGTK_DEBUG)
					CC_ASSERT(scenePartOthersList->objectForKey(scenePartOthersData->getId()) == nullptr);
#endif
					scenePartOthersList->setObject(scenePartOthersData, scenePartOthersData->getId());
				} break;

				// ポータル確認
				case ScenePartData::kPartPortal:
				{
					// エディタ専用の項目
				} break;
			}
		}
	};
	setScenePartDataRecur(json["scenePartList"]);
#else
	for (rapidjson::SizeType i = 0; i < json["scenePartList"].Size(); i++) {
		if (json["scenePartList"][i]["partType"].GetInt() != ScenePartData::kPartObject) continue;	//todo: 通常オブジェクト以外に要対応。
		auto scenePartData = ScenePartData::create(json["scenePartList"][i]);
#if defined(AGTK_DEBUG)
		CC_ASSERT(scenePartList->objectForKey(scenePartData->getId()) == nullptr);
#endif
		scenePartList->setObject(scenePartData, scenePartData->getId());
	}
#endif
	this->setScenePartObjectList(scenePartObjectList);
	this->setScenePartPhysicsList(scenePartPhysicsList);
	this->setScenePartOthersList(scenePartOthersList);
	this->setScenePartPortalList(scenePartPortalList);

	//objectSingleIdHash
	auto objectSingleIdHashList = cocos2d::__Dictionary::create();
	CC_ASSERT(json.HasMember("objectSingleIdHash"));
	const rapidjson::Value& objectSingleIdHash = json["objectSingleIdHash"];
	for (rapidjson::Value::ConstMemberIterator itr = objectSingleIdHash.MemberBegin(); itr != objectSingleIdHash.MemberEnd(); itr++) {
		auto name = itr->name.GetString();
		objectSingleIdHashList->setObject(cocos2d::__Integer::create(itr->value.GetInt()), atoi(name));
	}
	this->setObjectSingleIdHash(objectSingleIdHashList);

	//filterEffect
	auto sceneFilterEffectList = cocos2d::__Dictionary::create();
	CC_ASSERT(json.HasMember("sceneFilterEffectList"));
	for (rapidjson::SizeType i = 0; i < json["sceneFilterEffectList"].Size(); i++) {
		auto sceneFilterEffectData = agtk::data::SceneFilterEffectData::create(json["sceneFilterEffectList"][i]);
#if defined(AGTK_DEBUG)
		CC_ASSERT(sceneFilterEffectList->objectForKey(sceneFilterEffectData->getId()) == nullptr);
#endif
		sceneFilterEffectList->setObject(sceneFilterEffectData, sceneFilterEffectData->getId());
	}
	this->setSceneFilterEffectList(sceneFilterEffectList);

	return true;
}

const char *SceneData::getName()
{
	CC_ASSERT(_name);
	return _name->getCString();
}

const char *SceneData::getMemo()
{
	CC_ASSERT(_memo);
	return _memo->getCString();
}

LayerData *SceneData::getLayer(int id)
{
	CC_ASSERT(this->getLayerList());
// #AGTK-NX
#if defined(STATIC_DOWN_CAST) && defined(FORCE_STATIC_DOWN_CAST)
	auto p = static_cast<LayerData *>(this->getLayerList()->objectForKey(id));
#else
	auto p = dynamic_cast<LayerData *>(this->getLayerList()->objectForKey(id));
	CC_ASSERT(p);
#endif
	return p;
}

int SceneData::getObjectSingleId(int id)
{
	CC_ASSERT(this->getObjectSingleIdHash());
	auto p = dynamic_cast<cocos2d::Integer *>(this->getObjectSingleIdHash()->objectForKey(id));
	if (p == nullptr) {
		//なし。
		return -1;
	}
	return p->getValue();
}

/**
* 指定レイヤーの移動速度取得
* @param	layerId	レイヤーID
* @return	移動速度
*/
double SceneData::getLayerMoveSpeedX(int layerId)
{
	auto layerMoveSpeed = this->getLayerMoveSpeedX();
	if (layerMoveSpeed->count() <= layerId - 1) {
		return 0.0;
	}
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	auto p = static_cast<cocos2d::Double *>(layerMoveSpeed->getObjectAtIndex(layerId - 1));
#else
	auto p = dynamic_cast<cocos2d::Double *>(layerMoveSpeed->getObjectAtIndex(layerId - 1));
#endif
	CC_ASSERT(p);
	return p->getValue();
}
double SceneData::getLayerMoveSpeedY(int layerId)
{
	auto layerMoveSpeed = this->getLayerMoveSpeedY();
	if (layerMoveSpeed->count() <= layerId - 1) {
		return 0.0;
	}
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	auto p = static_cast<cocos2d::Double *>(layerMoveSpeed->getObjectAtIndex(layerId - 1));
#else
	auto p = dynamic_cast<cocos2d::Double *>(layerMoveSpeed->getObjectAtIndex(layerId - 1));
#endif
	CC_ASSERT(p);
	return p->getValue();
}

ScenePartObjectData *SceneData::getScenePartObjectData(int id)
{
	CC_ASSERT(this->getScenePartObjectList());
	auto scenePartObjectList = this->getScenePartObjectList();
	return dynamic_cast<ScenePartObjectData *>(scenePartObjectList->objectForKey(id));
}

cocos2d::__Array *SceneData::getScenePartObjectList(int layerId)
{
	CC_ASSERT(this->getScenePartObjectList());
	auto scenePartObjectList = this->getScenePartObjectList();
	cocos2d::DictElement *el = nullptr;

	cocos2d::__Array *arr = cocos2d::__Array::create();
	CCDICT_FOREACH(scenePartObjectList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto p = static_cast<ScenePartData *>(el->getObject());
#else
		auto p = dynamic_cast<ScenePartData *>(el->getObject());
#endif
		if (p->getLayerIndex() == layerId) {
			arr->addObject(p);
		}
	}
	return arr;
}

cocos2d::__Array *SceneData::getScenePartOthersList(int layerId)
{
	CC_ASSERT(this->getScenePartOthersList());
	auto scenPartOthersList = this->getScenePartOthersList();
	cocos2d::DictElement *el = nullptr;

	cocos2d::__Array *arr = cocos2d::__Array::create();
	CCDICT_FOREACH(scenPartOthersList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto p = static_cast<ScenePartData *>(el->getObject());
#else
		auto p = dynamic_cast<ScenePartData *>(el->getObject());
#endif
		if (p->getLayerIndex() == layerId) {
			arr->addObject(p);
		}
	}

	return arr;
}

int SceneData::getScenePartObjectDataMaxId()
{
	CC_ASSERT(this->getScenePartObjectList());
	auto scenePartList = this->getScenePartObjectList();
	cocos2d::DictElement *el = nullptr;
	int id = -1;
	CCDICT_FOREACH(scenePartList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto p = static_cast<ScenePartData *>(el->getObject());
#else
		auto p = dynamic_cast<ScenePartData *>(el->getObject());
#endif
		if (id < (int)p->getId()) {
			id = p->getId();
		}
	}
	return id;
}

bool SceneData::isScenePartObject(ScenePartObjectData *data)
{
	CC_ASSERT(this->getScenePartObjectList());
	auto scenePartList = this->getScenePartObjectList();
	cocos2d::DictElement *el;
	CCDICT_FOREACH(scenePartList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto p = static_cast<ScenePartData *>(el->getObject());
#else
		auto p = dynamic_cast<ScenePartData *>(el->getObject());
#endif
		if (p == data) {
			return true;
		}
	}
	return false;
}

bool SceneData::isMenuScene()
{
	return (this->getId() == kMenuSceneId);
}

#if defined(AGTK_DEBUG)
void SceneData::dump()
{
	CCLOG("id:%d", this->getId());
	CCLOG("name:%s", this->getName());
	CCLOG("folder:%d", this->getFolder());
	if (this->getFolder() && this->getChildren()) {
		auto keys = this->getChildren()->allKeys();
		cocos2d::Ref *ref = nullptr;
		CCARRAY_FOREACH(keys, ref) {
			auto id = dynamic_cast<cocos2d::Integer *>(ref);
			auto data = dynamic_cast<SceneData *>(this->getChildren()->objectForKey(id->getValue()));
			data->dump();
		}
		return;
	}
	CCLOG("horzScreenCount:%d", this->getHorzScreenCount());
	CCLOG("vertScreenCount:%d", this->getVertScreenCount());
	std::string tmp = "tileset:[";
	cocos2d::Ref *ref = nullptr;
	CCARRAY_FOREACH(this->getTileset(), ref) {
		auto i = dynamic_cast<cocos2d::Integer *>(ref);
		tmp += std::to_string(i->getValue()) + std::string(",");
	}
	tmp += "]";
	CCLOG("%s", tmp.c_str());
	CCLOG("memo:%s", this->getMemo());
	CCLOG("playBgmType:%d", this->getPlayBgmType());
	CCLOG("bgmId:%d", this->getBgmId());
	CCLOG("loopBgmFlag:%d", this->getLoopBgmFlag());
	CCLOG("bgColorR:%d", this->getBgColor().r);
	CCLOG("bgColorG:%d", this->getBgColor().g);
	CCLOG("bgColorB:%d", this->getBgColor().b);
	CCLOG("setBgImageFlag:%d", this->getSetBgImageFlag());
	CCLOG("bgImageId:%d", this->getBgImageId());
	CCLOG("bgImagePlacement:%d", this->getBgImagePlacement());
	CCLOG("bgImageMoveSpeed:%f", this->getBgImageMoveSpeed());
	CCLOG("bgImageMoveDirection:%f", this->getBgImageMoveDirection());
	CCLOG("bgMoveSpeedX:%f", this->getBgMoveSpeedX());
	CCLOG("bgMoveSpeedY:%f", this->getBgMoveSpeedY());
	tmp = "layerMoveSpeedX:[";
	CCARRAY_FOREACH(this->getLayerMoveSpeedX(), ref) {
		auto i = dynamic_cast<cocos2d::Double *>(ref);
		tmp += std::to_string(i->getValue()) + std::string(",");
	}
	tmp += "]";
	CCLOG("%s", tmp.c_str());
	tmp = "layerMoveSpeedY:[";
	CCARRAY_FOREACH(this->getLayerMoveSpeedY(), ref) {
		auto i = dynamic_cast<cocos2d::Double *>(ref);
		tmp += std::to_string(i->getValue()) + std::string(",");
	}
	tmp += "]";
	CCLOG("%s", tmp.c_str());
	CCLOG("airResistance: %f", this->getAirResistance());
	CCLOG("gravityAccel:%f", this->getGravityAccel());
	CCLOG("screenAutoScroll:%d", this->getScreenAutoScroll());
	CCLOG("screenAutoScrollSpeed:%f", this->getScreenAutoScrollSpeed());
	CCLOG("screenAutoScrollDirection:%f", this->getScreenAutoScrollDirection());
	CCLOG("screenAutoScrollSwitchObjectId:%d", this->getScreenAutoScrollSwitchObjectId());
	CCLOG("screenAutoScrollSwitchQualifierId:%d", this->getScreenAutoScrollSwitchQualifierId());
	CCLOG("screenAutoScrollSwitchId:%d", this->getScreenAutoScrollSwitchId());
	CCLOG("verticalLoop:%d", this->getVerticalLoop());
	CCLOG("horizontalLoop:%d", this->getHorizontalLoop());
	CCLOG("limitAreaX:%d", (int)this->getLimitAreaX());
	CCLOG("limitAreaY:%d", (int)this->getLimitAreaY());
	CCLOG("limitAreaWidth:%d", (int)this->getLimitAreaWidth());
	CCLOG("limitAreaHeight:%d", (int)this->getLimitAreaHeight());
	CCLOG("disableLimitArea:%d", (int)this->getDisableLimitArea());
	CCLOG("disableLimitCamera:%d", (int)this->getDisableLimitCamera());
	CCLOG("initialMenuLayerId:%d", this->getInitialMenuLayerId());
	tmp = "preloadMenuLayerIdList:[";
	CCARRAY_FOREACH(this->getPreloadMenuLayerIdList(), ref) {
		auto i = dynamic_cast<cocos2d::Integer *>(ref);
		tmp += std::to_string(i->getValue()) + std::string(",");
	}
	tmp += "]";
	CCLOG("%s", tmp.c_str());
	auto keys = this->getLayerList()->allKeys();
	CCARRAY_FOREACH(keys, ref) {
		auto id = dynamic_cast<cocos2d::Integer *>(ref);
		auto layer = dynamic_cast<LayerData *>(this->getLayerList()->objectForKey(id->getValue()));
		layer->dump();
	}
	CCLOG("scenePartList start -------");
	keys = this->getScenePartObjectList()->allKeys();
	CCARRAY_FOREACH(keys, ref) {
		auto id = dynamic_cast<cocos2d::Integer *>(ref);
		auto data = dynamic_cast<ScenePartData *>(this->getScenePartObjectList()->objectForKey(id->getValue()));
		data->dump();
	}
	CCLOG("scenePartList end -------");

	CCLOG("PhysicsPartList start -------");
	keys = this->getScenePartPhysicsList()->allKeys();
	CCARRAY_FOREACH(keys, ref) {
		auto id = dynamic_cast<cocos2d::Integer *>(ref);
		auto data = dynamic_cast<PhysicsPartData *>(this->getScenePartPhysicsList()->objectForKey(id->getValue()));
		data->dump();
	}
	CCLOG("PhysicsPartList end -------");

	keys = this->getSceneFilterEffectList()->allKeys();
	if (keys && keys->count()) {
		CCLOG("sceneFilterEffect start ---");
		CCARRAY_FOREACH(keys, ref) {
			auto id = dynamic_cast<cocos2d::Integer *>(ref);
			auto data = dynamic_cast<SceneFilterEffectData *>(this->getSceneFilterEffectList()->objectForKey(id->getValue()));
			data->dump();
		}
		CCLOG("sceneFilterEffect end ---");
	}	keys = this->getSceneFilterEffectList()->allKeys();

}
#endif

NS_DATA_END
NS_AGTK_END
