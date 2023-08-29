/**
 * @brief オブジェクトコマンドデータ
 */
#include "ObjectActionLinkConditionData.h"
#include "Manager/JavascriptManager.h"
#include "jsapi.h"
#include "jsfriendapi.h"
#include "scripting/js-bindings/manual/js_bindings_config.h"
#include "scripting/js-bindings/manual/js_bindings_core.h"
#include "scripting/js-bindings/manual/spidermonkey_specifics.h"
#include "scripting/js-bindings/manual/js_manual_conversions.h"
#include "mozilla/Maybe.h"
#include "scripting/js-bindings/manual/js-BindingsExport.h"

#define getJsBoolean	JavascriptManager::getBoolean
#define getJsInt32		JavascriptManager::getInt32
#define getJsDouble		JavascriptManager::getDouble
#define getJsString		JavascriptManager::getString
#define isJsDefined		JavascriptManager::isDefined

NS_AGTK_BEGIN
NS_DATA_BEGIN

//-------------------------------------------------------------------------------------------------------------------
ObjectActionLinkConditionData::ObjectActionLinkConditionData()
{
	_id = 0;
	_type = kConditionMax;
	_ignored = false;
	_condAnd = true;
	_notFlag = false;
}

ObjectActionLinkConditionData *ObjectActionLinkConditionData::create(const rapidjson::Value& json)
{
	EnumObjActionLinkConditionType type = (EnumObjActionLinkConditionType)json["type"].GetInt();
	if (type >= kConditionCustomHead) {
		return ObjectActionLinkConditionCustomData::create(json);
	} else
	switch (type) {
	case kConditionWallTouched: return ObjectActionLinkConditionWallTouchedData::create(json);
	case kConditionNoWall: return ObjectActionLinkConditionNoWallData::create(json);
	case kConditionWallAhead: return ObjectActionLinkConditionWallAheadData::create(json);
	case kConditionNoWallAhead: return ObjectActionLinkConditionNoWallAheadData::create(json);
	case kConditionObjectWallTouched: return ObjectActionLinkConditionObjectWallTouchedData::create(json);
	case kConditionNoObjectWallTouched: return ObjectActionLinkConditionNoObjectWallTouchedData::create(json);
	case kConditionAttackAreaTouched: return ObjectActionLinkConditionAttackAreaTouchedData::create(json);
	case kConditionAttackAreaNear: return ObjectActionLinkConditionAttackAreaNearData::create(json);
	case kConditionObjectNear: return ObjectActionLinkConditionObjectNearData::create(json);
	case kConditionObjectFacingEachOther: return ObjectActionLinkConditionObjectFacingEachOtherData::create(json);
	case kConditionObjectFacing: return ObjectActionLinkConditionObjectFacingData::create(json);
	case kConditionObjectFound: return ObjectActionLinkConditionObjectFoundData::create(json);
	case kConditionObjectFacingDirection: return ObjectActionLinkConditionObjectFacingDirectionData::create(json);
	case kConditionHpZero: return ObjectActionLinkConditionHpZeroData::create(json);
	case kConditionCameraOutOfRange: return ObjectActionLinkConditionCameraOutOfRangeData::create(json);
	case kConditionLocked: return ObjectActionLinkConditionLockedData::create(json);
	case kConditionProbability: return ObjectActionLinkConditionProbabilityData::create(json);
	case kConditionWaitTime: return ObjectActionLinkConditionWaitTimeData::create(json);
	case kConditionSwitchVariableChanged: return ObjectActionLinkConditionSwitchVariableChangedData::create(json);
	case kConditionAnimationFinished: return ObjectActionLinkConditionAnimationFinishedData::create(json);
	case kConditionJumpTop: return ObjectActionLinkConditionJumpTopData::create(json);
	case kConditionObjectActionChanged: return ObjectActionLinkConditionObjectActionChangedData::create(json);
	case kConditionSlopeTouched: return ObjectActionLinkConditionSlopeTouchedData::create(json);
	case kConditionBuriedInWall: return ObjectActionLinkConditionBuriedInWallData::create(json);
	case kConditionScript: return ObjectActionLinkConditionScriptData::create(json);
	case kConditionObjectHit: return ObjectActionLinkConditionObjectHitData::create(json);
	}
	CC_ASSERT(0);
	return nullptr;
}

bool ObjectActionLinkConditionData::init(const rapidjson::Value& json)
{
	CC_ASSERT(json.HasMember("id"));
	this->setId(json["id"].GetInt());
	CC_ASSERT(json.HasMember("type"));
	this->setType((EnumObjActionLinkConditionType)json["type"].GetInt());
	CC_ASSERT(json.HasMember("ignored"));
	this->setIgnored(json["ignored"].GetBool());
	CC_ASSERT(json.HasMember("condAnd"));
	this->setCondAnd(json["condAnd"].GetBool());
	if (json.HasMember("notFlag")) {
		this->setNotFlag(json["notFlag"].GetBool());
	}
	return true;
}

#if defined(AGTK_DEBUG)
void ObjectActionLinkConditionData::dump()
{
	CCLOG("id:%d", this->getId());
	CCLOG("type:%d", this->getType());
	CCLOG("ignroed:%d", this->getIgnored());
	CCLOG("condAnd:%d", this->getCondAnd());
	CCLOG("notFlag:%d", this->getNotFlag());
}
#endif

//-------------------------------------------------------------------------------------------------------------------
bool ObjectActionLinkConditionWallTouchedData::init(const rapidjson::Value& json)
{
	if (!ObjectActionLinkConditionData::init(json)) {
		return false;
	}
	CC_ASSERT(json.HasMember("wallTouched"));;
	const rapidjson::Value& cond = json["wallTouched"];
	CC_ASSERT(cond.HasMember("wallBit"));
	this->setWallBit(cond["wallBit"].GetInt());
	this->setUseTileGroup(cond["useTileGroup"].GetBool());
	this->setTileGroup(cond["tileGroup"].GetInt());
	return true;				
}

bool ObjectActionLinkConditionWallTouchedData::init(void *jsCx, void *jsObj)
{
	_type = kConditionWallTouched;
	//デフォルト値設定。
	this->setWallBit(0);
	this->setUseTileGroup(false);
	this->setTileGroup(kTileGroupDefault);

	auto cx = (JSContext *)jsCx;
	JS::RootedValue v(cx);
	JS::RootedObject rparams(cx, (JSObject *)jsObj);
	int iValue;
	bool bValue;
	if (getJsInt32(cx, rparams, "wallBit", &iValue)) {
		this->setWallBit(iValue);
	}
	if (getJsBoolean(cx, rparams, "useTileGroup", &bValue)) {
		this->setUseTileGroup(bValue);
	}
	if (getJsInt32(cx, rparams, "tileGroup", &iValue)) {
		this->setTileGroup(iValue);
	}
	return true;
}

#if defined(AGTK_DEBUG)
void ObjectActionLinkConditionWallTouchedData::dump()
{
	ObjectActionLinkConditionData::dump();
	CCLOG("wallBit:%d", this->getWallBit());
	CCLOG("useTileGroup:%d", this->getUseTileGroup());
	CCLOG("tileGroup:%d", this->getTileGroup());
}
#endif

//-------------------------------------------------------------------------------------------------------------------
bool ObjectActionLinkConditionNoWallData::init(const rapidjson::Value& json)
{
	if (!ObjectActionLinkConditionData::init(json)) {
		return false;
	}
	CC_ASSERT(json.HasMember("noWall"));
	const rapidjson::Value& cond = json["noWall"];
	this->setWallBit(cond["wallBit"].GetInt());
	return true;
}

#if defined(AGTK_DEBUG)
void ObjectActionLinkConditionNoWallData::dump()
{
	ObjectActionLinkConditionData::dump();
	CCLOG("wallBit:%d", this->getWallBit());
}
#endif

//-------------------------------------------------------------------------------------------------------------------
bool ObjectActionLinkConditionWallAheadData::init(const rapidjson::Value& json)
{
	if (!ObjectActionLinkConditionData::init(json)) {
		return false;
	}
	CC_ASSERT(json.HasMember("wallAhead"));
	const rapidjson::Value& cond = json["wallAhead"];
	this->setWallBit(cond["wallBit"].GetInt());
	this->setUseTileGroup(cond["useTileGroup"].GetBool());
	this->setTileGroup(cond["tileGroup"].GetInt());
	return true;
}

bool ObjectActionLinkConditionWallAheadData::init(void *jsCx, void *jsObj)
{
	_type = kConditionWallAhead;
	//デフォルト値設定。
	this->setWallBit(0);
	this->setUseTileGroup(false);
	this->setTileGroup(kTileGroupDefault);

	auto cx = (JSContext *)jsCx;
	JS::RootedValue v(cx);
	JS::RootedObject rparams(cx, (JSObject *)jsObj);
	int iValue;
	bool bValue;
	if (getJsInt32(cx, rparams, "wallBit", &iValue)) {
		this->setWallBit(iValue);
	}
	if (getJsBoolean(cx, rparams, "useTileGroup", &bValue)) {
		this->setUseTileGroup(iValue);
	}
	if (getJsInt32(cx, rparams, "tileGroup", &iValue)) {
		this->setTileGroup(iValue);
	}
	return true;
}

#if defined(AGTK_DEBUG)
void ObjectActionLinkConditionWallAheadData::dump()
{
	ObjectActionLinkConditionData::dump();
	CCLOG("wallBit:%d", this->getWallBit());
	CCLOG("useTileGroup:%d", this->getUseTileGroup());
	CCLOG("tileGroup:%d", this->getTileGroup());
}
#endif

//-------------------------------------------------------------------------------------------------------------------
bool ObjectActionLinkConditionNoWallAheadData::init(const rapidjson::Value& json)
{
	if (!ObjectActionLinkConditionData::init(json)) {
		return false;
	}
	CC_ASSERT(json.HasMember("noWallAhead"));
	const rapidjson::Value& cond = json["noWallAhead"];
	this->setWallBit(cond["wallBit"].GetInt());
	return true;
}

#if defined(AGTK_DEBUG)
void ObjectActionLinkConditionNoWallAheadData::dump()
{
	ObjectActionLinkConditionData::dump();
	CCLOG("wallBit:%d", this->getWallBit());
}
#endif

//-------------------------------------------------------------------------------------------------------------------
bool ObjectActionLinkConditionObjectWallTouchedData::init(const rapidjson::Value& json)
{
	if (!ObjectActionLinkConditionData::init(json)) {
		return false;
	}
	CC_ASSERT(json.HasMember("objectWallTouched"));
	const rapidjson::Value& cond = json["objectWallTouched"];
	this->setWallBit(cond["wallBit"].GetInt());
	this->setObjectType(cond["objectType"].GetInt());
	this->setObjectGroup(cond["objectGroup"].GetInt());
	this->setObjectId(cond["objectId"].GetInt());
	return true;
}

bool ObjectActionLinkConditionObjectWallTouchedData::init(void *jsCx, void *jsObj)
{
	_type = kConditionObjectWallTouched;
	//デフォルト値設定。
	this->setWallBit(0);
	this->setObjectType(kObjectByGroup);
	this->setObjectGroup(kObjectGroupAll);
	this->setObjectId(-1);

	auto cx = (JSContext *)jsCx;
	JS::RootedValue v(cx);
	JS::RootedObject rparams(cx, (JSObject *)jsObj);
	int iValue;
	if (getJsInt32(cx, rparams, "wallBit", &iValue)) {
		this->setWallBit(iValue);
	}
	if (getJsInt32(cx, rparams, "objectType", &iValue)) {
		this->setObjectType(iValue);
	}
	if (isJsDefined(cx, rparams, "objectGroup")) {
		if (getJsInt32(cx, rparams, "objectGroup", &iValue)) {
			this->setObjectGroup(iValue);
		}
	}
	else {
		if (getJsInt32(cx, rparams, "objectTypeByType:", &iValue)) {
			this->setObjectGroup(iValue - kObjectGroupAllOffset);
		}
	}
	if (getJsInt32(cx, rparams, "objectId", &iValue)) {
		this->setObjectId(iValue);
	}

	return true;
}

#if defined(AGTK_DEBUG)
void ObjectActionLinkConditionObjectWallTouchedData::dump()
{
	ObjectActionLinkConditionData::dump();
	CCLOG("wallBit:%d", this->getWallBit());
	CCLOG("objectType:%d", this->getObjectType());
	CCLOG("objectGroup:%d", this->getObjectGroup());
	CCLOG("objectId:%d", this->getObjectId());
}
#endif

//-------------------------------------------------------------------------------------------------------------------
bool ObjectActionLinkConditionNoObjectWallTouchedData::init(const rapidjson::Value& json)
{
	if (!ObjectActionLinkConditionData::init(json)) {
		return false;
	}
	CC_ASSERT(json.HasMember("noObjectWallTouched"));
	const rapidjson::Value& cond = json["noObjectWallTouched"];
	this->setWallBit(cond["wallBit"].GetInt());
	this->setObjectType(cond["objectType"].GetInt());
	this->setObjectTypeByType(cond["objectTypeByType"].GetInt());
	this->setObjectId(cond["objectId"].GetInt());
	return true;
}

#if defined(AGTK_DEBUG)
void ObjectActionLinkConditionNoObjectWallTouchedData::dump()
{
	ObjectActionLinkConditionData::dump();
	CCLOG("wallBit:%d", this->getWallBit());
	CCLOG("objectType:%d", this->getObjectType());
	CCLOG("objectTypeByType:%d", this->getObjectTypeByType());
	CCLOG("objectId:%d", this->getObjectId());
}
#endif

//-------------------------------------------------------------------------------------------------------------------
bool ObjectActionLinkConditionAttackAreaTouchedData::init(const rapidjson::Value& json)
{
	if (!ObjectActionLinkConditionData::init(json)) {
		return false;
	}
	CC_ASSERT(json.HasMember("attackAreaTouched"));
	const rapidjson::Value& cond = json["attackAreaTouched"];
	this->setWallBit(cond["wallBit"].GetInt());
	this->setObjectType(cond["objectType"].GetInt());
	this->setObjectGroup(cond["objectGroup"].GetInt());
	this->setObjectId(cond["objectId"].GetInt());
	this->setAttributeType((EnumAttackAttribute)cond["attributeType"].GetInt());
	this->setAttributePresetId(cond["attributePresetId"].GetInt());
	this->setAttributeValue(cond["attributeValue"].GetInt());
	this->setAttributeEqual(cond["attributeEqual"].GetBool());
	return true;

}

bool ObjectActionLinkConditionAttackAreaTouchedData::init(void *jsCx, void *jsObj)
{
	_type = kConditionAttackAreaTouched;
	//デフォルト値設定。
	this->setWallBit(0);
	this->setObjectType(kObjectByGroup);
	this->setObjectGroup(kObjectGroupAll);
	this->setObjectId(-1);
	this->setAttributeType(kAttributeNone);
	this->setAttributePresetId(kFire);
	this->setAttributeEqual(true);
	this->setAttributeValue(0);

	auto cx = (JSContext *)jsCx;
	JS::RootedValue v(cx);
	JS::RootedObject rparams(cx, (JSObject *)jsObj);
	bool bValue;
	int iValue;
	if (getJsInt32(cx, rparams, "wallBit", &iValue)) {
		this->setWallBit(iValue);
	}
	if (getJsInt32(cx, rparams, "objectType", &iValue)) {
		this->setObjectType(iValue);
	}
	if (isJsDefined(cx, rparams, "objectGroup")) {
		if (getJsInt32(cx, rparams, "objectGroup", &iValue)) {
			this->setObjectGroup(iValue);
		}
	}
	else {
		if (getJsInt32(cx, rparams, "objectTypeByType:", &iValue)) {
			this->setObjectGroup(iValue - kObjectGroupAllOffset);
		}
	}
	if (getJsInt32(cx, rparams, "objectId", &iValue)) {
		this->setObjectId(iValue);
	}
	if (getJsInt32(cx, rparams, "attributeType", &iValue)) {
		this->setAttributeType((EnumAttackAttribute)iValue);
	}
	if (getJsInt32(cx, rparams, "attributePresetId", &iValue)) {
		this->setAttributePresetId(iValue);
	}
	if (getJsBoolean(cx, rparams, "attributeEqual", &bValue)) {
		this->setAttributeEqual(bValue);
	}
	if (getJsInt32(cx, rparams, "attributeValue", &iValue)) {
		this->setAttributeValue(iValue);
	}

	return true;
}

#if defined(AGTK_DEBUG)
void ObjectActionLinkConditionAttackAreaTouchedData::dump()
{
	ObjectActionLinkConditionData::dump();
	CCLOG("wallBit:%d", this->getWallBit());
	CCLOG("objectType:%d", this->getObjectType());
	CCLOG("objectGroup:%d", this->getObjectGroup());
	CCLOG("objectId:%d", this->getObjectId());
	CCLOG("attributeType:%d", this->getAttributeType());
	CCLOG("attributePresetId:%d", this->getAttributePresetId());
	CCLOG("attributeValue:%d", this->getAttributeValue());
	CCLOG("attributeEqual:%d", this->getAttributeEqual());
}
#endif

//-------------------------------------------------------------------------------------------------------------------
bool ObjectActionLinkConditionAttackAreaNearData::init(const rapidjson::Value& json)
{
	if (!ObjectActionLinkConditionData::init(json)) {
		return false;
	}
	CC_ASSERT(json.HasMember("attackAreaNear"));
	const rapidjson::Value& cond = json["attackAreaNear"];
	this->setOtherDirections(cond["otherDirections"].GetBool());
	this->setObjectDirection(cond["objectDirection"].GetBool());
	if (cond.HasMember("directionBit")) {
		this->setDirectionBit(cond["directionBit"].GetInt());
	} else if (cond.HasMember("direction")) {
		this->setDirectionBit(1 << cond["direction"].GetInt());
	}
	this->setDistanceType((EnumDistanceType)cond["distanceType"].GetInt());
	this->setDistance(cond["distance"].GetInt());
	this->setObjectType(cond["objectType"].GetInt());
	this->setObjectGroup(cond["objectGroup"].GetInt());
	this->setObjectId(cond["objectId"].GetInt());
	this->setAttributeType((EnumAttackAttribute)cond["attributeType"].GetInt());
	this->setAttributePresetId(cond["attributePresetId"].GetInt());
	this->setAttributeValue(cond["attributeValue"].GetInt());
	this->setAttributeEqual(cond["attributeEqual"].GetBool());
	return true;
}

bool ObjectActionLinkConditionAttackAreaNearData::init(void *jsCx, void *jsObj)
{
	_type = kConditionAttackAreaNear;
	//デフォルト値設定。
	this->setOtherDirections(false);
	this->setObjectDirection(true);
	this->setDirectionBit(kAllDirectionBit);
	this->setDistanceType(kDistanceNone);
	this->setDistance(0);
	this->setObjectType(kObjectByGroup);
	this->setObjectGroup(kObjectGroupAll);
	this->setObjectId(-1);
	this->setAttributeType(kAttributeNone);
	this->setAttributePresetId(kFire);
	this->setAttributeEqual(true);
	this->setAttributeValue(0);

	auto cx = (JSContext *)jsCx;
	JS::RootedValue v(cx);
	JS::RootedObject rparams(cx, (JSObject *)jsObj);
	bool bValue;
	int iValue;
	if (getJsBoolean(cx, rparams, "otherDirections", &bValue)) {
		this->setOtherDirections(bValue);
	}
	if (getJsBoolean(cx, rparams, "objectDirection", &bValue)) {
		this->setObjectDirection(bValue);
	}
	if (getJsInt32(cx, rparams, "directionBit", &iValue)) {
		this->setDirectionBit(iValue);
	}
	if (getJsInt32(cx, rparams, "distanceType", &iValue)) {
		this->setDistanceType((EnumDistanceType)iValue);
	}
	if (getJsInt32(cx, rparams, "distance", &iValue)) {
		this->setDistance(iValue);
	}
	if (getJsInt32(cx, rparams, "objectType", &iValue)) {
		this->setObjectType(iValue);
	}
	if (isJsDefined(cx, rparams, "objectGroup")) {
		if (getJsInt32(cx, rparams, "objectGroup", &iValue)) {
			this->setObjectGroup(iValue);
		}
	}
	else {
		if (getJsInt32(cx, rparams, "objectTypeByType:", &iValue)) {
			this->setObjectGroup(iValue - kObjectGroupAllOffset);
		}
	}
	if (getJsInt32(cx, rparams, "objectId", &iValue)) {
		this->setObjectId(iValue);
	}
	if (getJsInt32(cx, rparams, "attributeType", &iValue)) {
		this->setAttributeType((EnumAttackAttribute)iValue);
	}
	if (getJsInt32(cx, rparams, "attributePresetId", &iValue)) {
		this->setAttributePresetId(iValue);
	}
	if (getJsBoolean(cx, rparams, "attributeEqual", &bValue)) {
		this->setAttributeEqual(bValue);
	}
	if (getJsInt32(cx, rparams, "attributeValue", &iValue)) {
		this->setAttributeValue(iValue);
	}

	return true;
}

#if defined(AGTK_DEBUG)
void ObjectActionLinkConditionAttackAreaNearData::dump()
{
	ObjectActionLinkConditionData::dump();
	CCLOG("otherDirections:%d", this->getOtherDirections());
	CCLOG("objectDirection:%d", this->getObjectDirection());
	CCLOG("directionBit:%d", this->getDirectionBit());
	CCLOG("distanceType:%d", this->getDistanceType());
	CCLOG("distance:%d", this->getDistance());
	CCLOG("objectType:%d", this->getObjectType());
	CCLOG("objectGroup:%d", this->getObjectGroup());
	CCLOG("objectId:%d", this->getObjectId());
	CCLOG("attributeType:%d", this->getAttributeType());
	CCLOG("attributePresetId:%d", this->getAttributePresetId());
	CCLOG("attributeValue:%d", this->getAttributeValue());
	CCLOG("attributeEqual:%d", this->getAttributeEqual());
}
#endif

//-------------------------------------------------------------------------------------------------------------------
bool ObjectActionLinkConditionObjectNearData::init(const rapidjson::Value& json)
{
	if (!ObjectActionLinkConditionData::init(json)) {
		return false;
	}
	CC_ASSERT(json.HasMember("objectNear"));
	const rapidjson::Value& cond = json["objectNear"];
	this->setOtherDirections(cond["otherDirections"].GetBool());
	this->setObjectDirection(cond["objectDirection"].GetBool());
	if (cond.HasMember("directionBit")) {
		this->setDirectionBit(cond["directionBit"].GetInt());
	} else if(cond.HasMember("direction")){
		this->setDirectionBit(1 << cond["direction"].GetInt());
	}
	this->setDistanceType(cond["distanceType"].GetInt());
	this->setDistance(cond["distance"].GetInt());
	this->setObjectType(cond["objectType"].GetInt());
	this->setObjectGroup(cond["objectGroup"].GetInt());
	this->setObjectId(cond["objectId"].GetInt());
	return true;
}

bool ObjectActionLinkConditionObjectNearData::init(void *jsCx, void *jsObj)
{
	_type = kConditionObjectNear;
	//デフォルト値設定。
	this->setOtherDirections(false);
	this->setObjectDirection(true);
	this->setDirectionBit(kAllDirectionBit);
	this->setDistanceType(kDistanceNone);
	this->setDistance(0);
	this->setObjectType(kObjectByGroup);
	this->setObjectGroup(kObjectGroupAll);
	this->setObjectId(-1);

	auto cx = (JSContext *)jsCx;
	JS::RootedValue v(cx);
	JS::RootedObject rparams(cx, (JSObject *)jsObj);
	bool bValue;
	int iValue;
	if (getJsBoolean(cx, rparams, "otherDirections", &bValue)) {
		this->setOtherDirections(bValue);
	}
	if (getJsBoolean(cx, rparams, "objectDirection", &bValue)) {
		this->setObjectDirection(bValue);
	}
	if (getJsInt32(cx, rparams, "directionBit", &iValue)) {
		this->setDirectionBit(iValue);
	}
	if (getJsInt32(cx, rparams, "distanceType", &iValue)) {
		this->setDistanceType(iValue);
	}
	if (getJsInt32(cx, rparams, "distance", &iValue)) {
		this->setDistance(iValue);
	}
	if (getJsInt32(cx, rparams, "objectType", &iValue)) {
		this->setObjectType(iValue);
	}
	if (isJsDefined(cx, rparams, "objectGroup")) {
		if (getJsInt32(cx, rparams, "objectGroup", &iValue)) {
			this->setObjectGroup(iValue);
		}
	}
	else {
		if (getJsInt32(cx, rparams, "objectTypeByType:", &iValue)) {
			this->setObjectGroup(iValue - kObjectGroupAllOffset);
		}
	}
	if (getJsInt32(cx, rparams, "objectId", &iValue)) {
		this->setObjectId(iValue);
	}

	return true;
}

#if defined(AGTK_DEBUG)
void ObjectActionLinkConditionObjectNearData::dump()
{
	ObjectActionLinkConditionData::dump();
	CCLOG("otherDirections:%d", this->getOtherDirections());
	CCLOG("objectDirection:%d", this->getObjectDirection());
	CCLOG("directionBit:%d", this->getDirectionBit());
	CCLOG("distanceType:%d", this->getDistanceType());
	CCLOG("distance:%d", this->getDistance());
	CCLOG("objectType:%d", this->getObjectType());
	CCLOG("objectGroup:%d", this->getObjectGroup());
	CCLOG("objectId:%d", this->getObjectId());
}
#endif

//-------------------------------------------------------------------------------------------------------------------
bool ObjectActionLinkConditionObjectFacingEachOtherData::init(const rapidjson::Value& json)
{
	if (!ObjectActionLinkConditionData::init(json)) {
		return false;
	}
	CC_ASSERT(json.HasMember("objectFacingEachOther"));
	const rapidjson::Value& cond = json["objectFacingEachOther"];
	this->setObjectType(cond["objectType"].GetInt());
	this->setObjectGroup(cond["objectGroup"].GetInt());
	this->setObjectId(cond["objectId"].GetInt());
	return true;
}

bool ObjectActionLinkConditionObjectFacingEachOtherData::init(void *jsCx, void *jsObj)
{
	_type = kConditionObjectFacingEachOther;
	//デフォルト値設定。
	this->setObjectType(kObjectByGroup);
	this->setObjectGroup(kObjectGroupAll);
	this->setObjectId(-1);

	auto cx = (JSContext *)jsCx;
	JS::RootedValue v(cx);
	JS::RootedObject rparams(cx, (JSObject *)jsObj);
	int iValue;
	if (getJsInt32(cx, rparams, "objectType", &iValue)) {
		this->setObjectType(iValue);
	}
	if (isJsDefined(cx, rparams, "objectGroup")) {
		if (getJsInt32(cx, rparams, "objectGroup", &iValue)) {
			this->setObjectGroup(iValue);
		}
	}
	else {
		if (getJsInt32(cx, rparams, "objectTypeByType:", &iValue)) {
			this->setObjectGroup(iValue - kObjectGroupAllOffset);
		}
	}
	if (getJsInt32(cx, rparams, "objectId", &iValue)) {
		this->setObjectId(iValue);
	}

	return true;
}

#if defined(AGTK_DEBUG)
void ObjectActionLinkConditionObjectFacingEachOtherData::dump()
{
	ObjectActionLinkConditionData::dump();
	CCLOG("objectType:%d", this->getObjectType());
	CCLOG("objectGroup:%d", this->getObjectGroup());
	CCLOG("objectId:%d", this->getObjectId());
}
#endif

//-------------------------------------------------------------------------------------------------------------------
bool ObjectActionLinkConditionObjectFacingData::init(const rapidjson::Value& json)
{
	if (!ObjectActionLinkConditionData::init(json)) {
		return false;
	}
	CC_ASSERT(json.HasMember("objectFacing"));
	const rapidjson::Value& cond = json["objectFacing"];
	this->setObjectType(cond["objectType"].GetInt());
	this->setObjectGroup(cond["objectGroup"].GetInt());
	this->setObjectId(cond["objectId"].GetInt());
	return true;
}

bool ObjectActionLinkConditionObjectFacingData::init(void *jsCx, void *jsObj)
{
	_type = kConditionObjectFacing;
	//デフォルト値設定。
	this->setObjectType(kObjectByGroup);
	this->setObjectGroup(kObjectGroupAll);
	this->setObjectId(-1);

	auto cx = (JSContext *)jsCx;
	JS::RootedValue v(cx);
	JS::RootedObject rparams(cx, (JSObject *)jsObj);
	int iValue;
	if (getJsInt32(cx, rparams, "objectType", &iValue)) {
		this->setObjectType(iValue);
	}
	if (isJsDefined(cx, rparams, "objectGroup")) {
		if (getJsInt32(cx, rparams, "objectGroup", &iValue)) {
			this->setObjectGroup(iValue);
		}
	}
	else {
		if (getJsInt32(cx, rparams, "objectTypeByType:", &iValue)) {
			this->setObjectGroup(iValue - kObjectGroupAllOffset);
		}
	}
	if (getJsInt32(cx, rparams, "objectId", &iValue)) {
		this->setObjectId(iValue);
	}

	return true;
}

#if defined(AGTK_DEBUG)
void ObjectActionLinkConditionObjectFacingData::dump()
{
	ObjectActionLinkConditionData::dump();
	CCLOG("objectType:%d", this->getObjectType());
	CCLOG("objectGroup:%d", this->getObjectGroup());
	CCLOG("objectId:%d", this->getObjectId());
}
#endif

//-------------------------------------------------------------------------------------------------------------------
bool ObjectActionLinkConditionObjectFoundData::init(const rapidjson::Value& json)
{
	if (!ObjectActionLinkConditionData::init(json)) {
		return false;
	}
	CC_ASSERT(json.HasMember("objectFound"));
	const rapidjson::Value& cond = json["objectFound"];
	this->setViewportId(cond["viewportId"].GetInt());
	if (cond.HasMember("discoveredAcrossLayersObject")) {
		this->setDiscoveredAcrossLayersObject(cond["discoveredAcrossLayersObject"].GetBool());
	}
	else {
		this->setDiscoveredAcrossLayersObject(false);
	}
	this->setObjectType(cond["objectType"].GetInt());
	this->setObjectGroup(cond["objectGroup"].GetInt());
	this->setObjectId(cond["objectId"].GetInt());
	return true;
}

bool ObjectActionLinkConditionObjectFoundData::init(void *jsCx, void *jsObj)
{
	_type = kConditionObjectFound;
	//デフォルト値設定。
	this->setViewportId(-1);
	this->setDiscoveredAcrossLayersObject(false);
	this->setObjectType(kObjectByGroup);
	this->setObjectGroup(kObjectGroupAll);
	this->setObjectId(-1);

	auto cx = (JSContext *)jsCx;
	JS::RootedValue v(cx);
	JS::RootedObject rparams(cx, (JSObject *)jsObj);
	int iValue;
	bool bValue;
	if (getJsInt32(cx, rparams, "viewportId", &iValue)) {
		this->setViewportId(iValue);
	}
	if (getJsBoolean(cx, rparams, "lockViewportLightOfAcrossLayerObject", &bValue)) {
		this->setDiscoveredAcrossLayersObject(bValue);
	}
	if (getJsInt32(cx, rparams, "objectType", &iValue)) {
		this->setObjectType(iValue);
	}
	if (isJsDefined(cx, rparams, "objectGroup")) {
		if (getJsInt32(cx, rparams, "objectGroup", &iValue)) {
			this->setObjectGroup(iValue);
		}
	}
	else {
		if (getJsInt32(cx, rparams, "objectTypeByType:", &iValue)) {
			this->setObjectGroup(iValue - kObjectGroupAllOffset);
		}
	}
	if (getJsInt32(cx, rparams, "objectId", &iValue)) {
		this->setObjectId(iValue);
	}

	return true;
}

#if defined(AGTK_DEBUG)
void ObjectActionLinkConditionObjectFoundData::dump()
{
	ObjectActionLinkConditionData::dump();
	CCLOG("viewportId:%d", this->getViewportId());
	CCLOG("discoveredAcrossLayersObject:%d", this->getDiscoveredAcrossLayersObject());
	CCLOG("objectType:%d", this->getObjectType());
	CCLOG("objectGroup:%d", this->getObjectGroup());
	CCLOG("objectId:%d", this->getObjectId());
}
#endif

//-------------------------------------------------------------------------------------------------------------------
bool ObjectActionLinkConditionObjectFacingDirectionData::init(const rapidjson::Value& json)
{
	if (!ObjectActionLinkConditionData::init(json)) {
		return false;
	}
	CC_ASSERT(json.HasMember("objectFacingDirection"));
	const rapidjson::Value& cond = json["objectFacingDirection"];
	this->setOtherDirections(cond["otherDirections"].GetBool());
	this->setObjectDirection(cond["objectDirection"].GetBool());
	if (cond.HasMember("directionBit")) {
		this->setDirectionBit(cond["directionBit"].GetInt());
	} else if (cond.HasMember("direction")) {
		this->setDirectionBit(1 << cond["direction"].GetInt());
	}
	this->setObjectType(cond["objectType"].GetInt());
	this->setObjectGroup(cond["objectGroup"].GetInt());
	this->setObjectId(cond["objectId"].GetInt());
	return true;
}

bool ObjectActionLinkConditionObjectFacingDirectionData::init(void *jsCx, void *jsObj)
{
	_type = kConditionObjectFacingDirection;
	//デフォルト値設定。
	this->setOtherDirections(false);
	this->setObjectDirection(true);
	this->setDirectionBit(kAllDirectionBit);
	this->setObjectType(kObjectByGroup);
	this->setObjectGroup(kObjectGroupAll);
	this->setObjectId(-1);

	auto cx = (JSContext *)jsCx;
	JS::RootedValue v(cx);
	JS::RootedObject rparams(cx, (JSObject *)jsObj);
	bool bValue;
	int iValue;
	if (getJsBoolean(cx, rparams, "otherDirections", &bValue)) {
		this->setOtherDirections(bValue);
	}
	if (getJsBoolean(cx, rparams, "objectDirection", &bValue)) {
		this->setObjectDirection(bValue);
	}
	if (getJsInt32(cx, rparams, "directionBit", &iValue)) {
		this->setDirectionBit(iValue);
	}
	if (getJsInt32(cx, rparams, "objectType", &iValue)) {
		this->setObjectType(iValue);
	}
	if (isJsDefined(cx, rparams, "objectGroup")) {
		if (getJsInt32(cx, rparams, "objectGroup", &iValue)) {
			this->setObjectGroup(iValue);
		}
	}
	else {
		if (getJsInt32(cx, rparams, "objectTypeByType:", &iValue)) {
			this->setObjectGroup(iValue - kObjectGroupAllOffset);
		}
	}
	if (getJsInt32(cx, rparams, "objectId", &iValue)) {
		this->setObjectId(iValue);
	}

	return true;
}

#if defined(AGTK_DEBUG)
void ObjectActionLinkConditionObjectFacingDirectionData::dump()
{
	ObjectActionLinkConditionData::dump();
	CCLOG("otherDirections:%d", this->getOtherDirections());
	CCLOG("objectDirection:%d", this->getObjectDirection());
	CCLOG("directionBit:%d", this->getDirectionBit());
	CCLOG("objectType:%d", this->getObjectType());
	CCLOG("objectGroup:%d", this->getObjectGroup());
	CCLOG("objectId:%d", this->getObjectId());
}
#endif

//-------------------------------------------------------------------------------------------------------------------
bool ObjectActionLinkConditionHpZeroData::init(const rapidjson::Value& json)
{
	if (!ObjectActionLinkConditionData::init(json)) {
		return false;
	}
	CC_ASSERT(json.HasMember("hpZero"));
	const rapidjson::Value& cond = json["hpZero"];
	this->setObjectId(cond["objectId"].GetInt());
	return true;
}

bool ObjectActionLinkConditionHpZeroData::init(void *jsCx, void *jsObj)
{
	_type = kConditionHpZero;
	//デフォルト値設定。
	this->setObjectId(kSelfObject);

	auto cx = (JSContext *)jsCx;
	JS::RootedValue v(cx);
	JS::RootedObject rparams(cx, (JSObject *)jsObj);
	int iValue;
	if (getJsInt32(cx, rparams, "objectId", &iValue)) {
		this->setObjectId(iValue);
	}

	return true;
}

#if defined(AGTK_DEBUG)
void ObjectActionLinkConditionHpZeroData::dump()
{
	ObjectActionLinkConditionData::dump();
	CCLOG("objectId:%d", this->getObjectId());
}
#endif

//-------------------------------------------------------------------------------------------------------------------
bool ObjectActionLinkConditionCameraOutOfRangeData::init(const rapidjson::Value& json)
{
	if (!ObjectActionLinkConditionData::init(json)) {
		return false;
	}
	CC_ASSERT(json.HasMember("cameraOutOfRange"));
	const rapidjson::Value& cond = json["cameraOutOfRange"];
	this->setObjectId(cond["objectId"].GetInt());
	this->setDistanceFlag(cond["distanceFlag"].GetBool());
	this->setDistance(cond["distance"].GetInt());
	return true;
}

bool ObjectActionLinkConditionCameraOutOfRangeData::init(void *jsCx, void *jsObj)
{
	_type = kConditionCameraOutOfRange;
	//デフォルト値設定。
	this->setObjectId(kSelfObject);
	this->setDistanceFlag(false);
	this->setDistance(0);

	auto cx = (JSContext *)jsCx;
	JS::RootedValue v(cx);
	JS::RootedObject rparams(cx, (JSObject *)jsObj);
	bool bValue;
	int iValue;
	if (getJsInt32(cx, rparams, "objectId", &iValue)) {
		this->setObjectId(iValue);
	}
	if (getJsBoolean(cx, rparams, "distanceFlag", &bValue)) {
		this->setDistanceFlag(bValue);
	}
	if (getJsInt32(cx, rparams, "distance", &iValue)) {
		this->setDistance(iValue);
	}

	return true;
}

#if defined(AGTK_DEBUG)
void ObjectActionLinkConditionCameraOutOfRangeData::dump()
{
	ObjectActionLinkConditionData::dump();
	CCLOG("objectId:%d", this->getObjectId());
	CCLOG("distanceFlag:%d", this->getDistanceFlag());
	CCLOG("distance:%d", this->getDistance());
}
#endif

//-------------------------------------------------------------------------------------------------------------------
bool ObjectActionLinkConditionLockedData::init(const rapidjson::Value& json)
{
	if (!ObjectActionLinkConditionData::init(json)) {
		return false;
	}
	CC_ASSERT(json.HasMember("locked"));
	const rapidjson::Value& cond = json["locked"];
	this->setLockingObjectId(cond["lockingObjectId"].GetInt());
	this->setLockedObjectType(cond["lockedObjectType"].GetInt());
	this->setLockedObjectGroup(cond["lockedObjectGroup"].GetInt());
	this->setLockedObjectId(cond["lockedObjectId"].GetInt());
	return true;
}

bool ObjectActionLinkConditionLockedData::init(void *jsCx, void *jsObj)
{
	_type = kConditionLocked;
	//デフォルト値設定。
	this->setLockingObjectId(-1);
	this->setLockedObjectType(kObjectByGroup);
	this->setLockedObjectGroup(kObjectGroupAll);
	this->setLockedObjectId(-1);

	auto cx = (JSContext *)jsCx;
	JS::RootedValue v(cx);
	JS::RootedObject rparams(cx, (JSObject *)jsObj);
	int iValue;
	if (getJsInt32(cx, rparams, "lockingObjectId", &iValue)) {
		this->setLockingObjectId(iValue);
	}
	if (getJsInt32(cx, rparams, "lockedObjectType", &iValue)) {
		this->setLockedObjectType(iValue);
	}
	if (isJsDefined(cx, rparams, "lockedObjectGroup")) {
		if (getJsInt32(cx, rparams, "lockedObjectGroup", &iValue)) {
			this->setLockedObjectGroup(iValue);
		}
	}
	else {
		if (getJsInt32(cx, rparams, "lockedObjectTypeByType:", &iValue)) {
			this->setLockedObjectGroup(iValue - kObjectGroupAllOffset);
		}
	}
	if (getJsInt32(cx, rparams, "lockedObjectId", &iValue)) {
		this->setLockedObjectId(iValue);
	}

	return true;
}

#if defined(AGTK_DEBUG)
void ObjectActionLinkConditionLockedData::dump()
{
	ObjectActionLinkConditionData::dump();
	CCLOG("lockingObjectId:%d", this->getLockingObjectId());
	CCLOG("lockedObjectType:%d", this->getLockedObjectType());
	CCLOG("lockedObjectGroup:%d", this->getLockedObjectGroup());
	CCLOG("lockedObjectId:%d", this->getLockedObjectId());
}
#endif

//-------------------------------------------------------------------------------------------------------------------
bool ObjectActionLinkConditionProbabilityData::init(const rapidjson::Value& json)
{
	if (!ObjectActionLinkConditionData::init(json)) {
		return false;
	}
	CC_ASSERT(json.HasMember("probability"));
	const rapidjson::Value& cond = json["probability"];
	this->setProbability(cond["probability"].GetDouble());
	return true;
}

bool ObjectActionLinkConditionProbabilityData::init(void *jsCx, void *jsObj)
{
	_type = kConditionProbability;
	//デフォルト値設定。
	this->setProbability(0);

	auto cx = (JSContext *)jsCx;
	JS::RootedValue v(cx);
	JS::RootedObject rparams(cx, (JSObject *)jsObj);
	double dValue;
	if (getJsDouble(cx, rparams, "probability", &dValue)) {
		this->setProbability(dValue);
	}

	return true;
}

#if defined(AGTK_DEBUG)
void ObjectActionLinkConditionProbabilityData::dump()
{
	ObjectActionLinkConditionData::dump();
	CCLOG("probability:%f", this->getProbability());
}
#endif

//-------------------------------------------------------------------------------------------------------------------
bool ObjectActionLinkConditionWaitTimeData::init(const rapidjson::Value& json)
{
	if (!ObjectActionLinkConditionData::init(json)) {
		return false;
	}
	CC_ASSERT(json.HasMember("waitTime"));
	const rapidjson::Value& cond = json["waitTime"];
	this->setTime(cond["time"].GetDouble());
	return true;
}

#if defined(AGTK_DEBUG)
void ObjectActionLinkConditionWaitTimeData::dump()
{
	ObjectActionLinkConditionData::dump();
	CCLOG("time:%f", this->getTime());
}
#endif

//-------------------------------------------------------------------------------------------------------------------
bool ObjectActionLinkConditionSwitchVariableChangedData::init(const rapidjson::Value& json)
{
	if (!ObjectActionLinkConditionData::init(json)) {
		return false;
	}
	CC_ASSERT(json.HasMember("switchVariableChanged"));
	const rapidjson::Value& cond = json["switchVariableChanged"];
	this->setSwtch(cond["swtch"].GetBool());
	this->setSwitchObjectId(cond["switchObjectId"].GetInt());
	this->setSwitchId(cond["switchId"].GetInt());
	this->setSwitchQualifierId((EnumQualifierType)cond["switchQualifierId"].GetInt());
	this->setSwitchCondition((EnumSwitchCondition)cond["switchCondition"].GetInt());
	this->setVariableObjectId(cond["variableObjectId"].GetInt());
	this->setVariableId(cond["variableId"].GetInt());
	this->setVariableQualifierId((EnumQualifierType)cond["variableQualifierId"].GetInt());
	this->setCompareVariableOperator(cond["compareVariableOperator"].GetInt());
	this->setCompareValueType((EnumVariableCompareValueType)cond["compareValueType"].GetInt());
	this->setCompareValue(cond["compareValue"].GetDouble());
	this->setCompareVariableObjectId(cond["compareVariableObjectId"].GetInt());
	this->setCompareVariableId(cond["compareVariableId"].GetInt());
	this->setCompareVariableQualifierId((EnumQualifierType)cond["compareVariableQualifierId"].GetInt());
	return true;
}

bool ObjectActionLinkConditionSwitchVariableChangedData::init(void *jsCx, void *jsObj)
{
	_type = kConditionSwitchVariableChanged;
	//デフォルト値設定。
	this->setSwtch(true);
	this->setSwitchObjectId(-1);
	this->setSwitchQualifierId(kQualifierSingle);
	this->setSwitchId(-1);
	this->setSwitchCondition(kSwitchConditionOn);
	this->setVariableObjectId(-1);
	this->setVariableQualifierId(kQualifierSingle);
	this->setVariableId(-1);
	this->setCompareVariableOperator(kOperatorLess);
	this->setCompareValueType(kVariableCompareValue);
	this->setCompareValue(0);
	this->setCompareVariableObjectId(-1);
	this->setCompareVariableQualifierId(kQualifierSingle);
	this->setCompareVariableId(-1);

	auto cx = (JSContext *)jsCx;
	JS::RootedValue v(cx);
	JS::RootedObject rparams(cx, (JSObject *)jsObj);
	bool bValue;
	int iValue;
	double dValue;
	if (getJsBoolean(cx, rparams, "swtch", &bValue)) {
		this->setSwtch(bValue);
	}
	if (getJsInt32(cx, rparams, "switchObjectId", &iValue)) {
		this->setSwitchObjectId(iValue);
	}
	if (getJsInt32(cx, rparams, "switchQualifierId", &iValue)) {
		this->setSwitchQualifierId((EnumQualifierType)iValue);
	}
	if (getJsInt32(cx, rparams, "switchId", &iValue)) {
		this->setSwitchId(iValue);
	}
	if (getJsInt32(cx, rparams, "switchCondition", &iValue)) {
		this->setSwitchCondition((EnumSwitchCondition)iValue);
	}
	if (getJsInt32(cx, rparams, "variableObjectId", &iValue)) {
		this->setVariableObjectId(iValue);
	}
	if (getJsInt32(cx, rparams, "variableQualifierId", &iValue)) {
		this->setVariableQualifierId((EnumQualifierType)iValue);
	}
	if (getJsInt32(cx, rparams, "variableId", &iValue)) {
		this->setVariableId(iValue);
	}
	if (getJsInt32(cx, rparams, "compareVariableOperator", &iValue)) {
		this->setCompareVariableOperator(iValue);
	}
	if (getJsInt32(cx, rparams, "compareValueType", &iValue)) {
		this->setCompareValueType((EnumVariableCompareValueType)iValue);
	}
	if (getJsDouble(cx, rparams, "compareValue", &dValue)) {
		this->setCompareValue(dValue);
	}
	if (getJsInt32(cx, rparams, "compareVariableObjectId", &iValue)) {
		this->setCompareVariableObjectId(iValue);
	}
	if (getJsInt32(cx, rparams, "compareVariableQualifierId", &iValue)) {
		this->setCompareVariableQualifierId((EnumQualifierType)iValue);
	}
	if (getJsInt32(cx, rparams, "compareVariableId", &iValue)) {
		this->setCompareVariableId(iValue);
	}

	return true;
}

#if defined(AGTK_DEBUG)
void ObjectActionLinkConditionSwitchVariableChangedData::dump()
{
	ObjectActionLinkConditionData::dump();
	CCLOG("swch:%d", this->getSwtch());
	CCLOG("switchObjectId:%d", this->getSwitchObjectId());
	CCLOG("switchId:%d", this->getSwitchId());
	CCLOG("switchQualifierId:%d", this->getSwitchQualifierId());
	CCLOG("switchCondition:%d", this->getSwitchCondition());
	CCLOG("variableObjectId:%d", this->getVariableObjectId());
	CCLOG("variableId:%d", this->getVariableId());
	CCLOG("variableQualifierId:%d", this->getVariableQualifierId());
	CCLOG("compareVariableOperator:%d", this->getCompareVariableOperator());
	CCLOG("compareValueType:%d", this->getCompareValueType());
	CCLOG("compareValue:%d", this->getCompareValue());
	CCLOG("compareVariableObjectId:%d", this->getCompareVariableObjectId());
	CCLOG("compareVariableId:%d", this->getCompareVariableId());
	CCLOG("compareVariableQualifierId:%d", this->getCompareVariableQualifierId());
}
#endif

//-------------------------------------------------------------------------------------------------------------------
bool ObjectActionLinkConditionAnimationFinishedData::init(const rapidjson::Value& json)
{
	if (!ObjectActionLinkConditionData::init(json)) {
		return false;
	}
	CC_ASSERT(json.HasMember("animationFinished"));
	return true;
}

bool ObjectActionLinkConditionAnimationFinishedData::init(void *jsCx, void *jsObj)
{
	_type = kConditionAnimationFinished;

	return true;
}

#if defined(AGTK_DEBUG)
void ObjectActionLinkConditionAnimationFinishedData::dump()
{
	ObjectActionLinkConditionData::dump();
}
#endif

//-------------------------------------------------------------------------------------------------------------------
bool ObjectActionLinkConditionJumpTopData::init(const rapidjson::Value& json)
{
	if (!ObjectActionLinkConditionData::init(json)) {
		return false;
	}
	CC_ASSERT(json.HasMember("jumpTop"));
	return true;
}

bool ObjectActionLinkConditionJumpTopData::init(void *jsCx, void *jsObj)
{
	_type = kConditionJumpTop;

	return true;
}

#if defined(AGTK_DEBUG)
void ObjectActionLinkConditionJumpTopData::dump()
{
	ObjectActionLinkConditionData::dump();
}
#endif

//-------------------------------------------------------------------------------------------------------------------
bool ObjectActionLinkConditionObjectActionChangedData::init(const rapidjson::Value& json)
{
	if (!ObjectActionLinkConditionData::init(json)) {
		return false;
	}
	CC_ASSERT(json.HasMember("objectActionChanged"));
	const rapidjson::Value& cond = json["objectActionChanged"];
	this->setObjectId(cond["objectId"].GetInt());
	if (cond.HasMember("actionObjectId")) {
		this->setActionObjectId(cond["actionObjectId"].GetInt());
	}
	this->setActionId(cond["actionId"].GetInt());
	this->setOtherActions(cond["otherActions"].GetBool());
	return true;
}

bool ObjectActionLinkConditionObjectActionChangedData::init(void *jsCx, void *jsObj)
{
	_type = kConditionObjectActionChanged;
	//デフォルト値設定。
	this->setObjectId(-1);
	this->setActionObjectId(-1);
	this->setActionId(-1);
	this->setOtherActions(false);

	auto cx = (JSContext *)jsCx;
	JS::RootedValue v(cx);
	JS::RootedObject rparams(cx, (JSObject *)jsObj);
	bool bValue;
	int iValue;
	if (getJsInt32(cx, rparams, "objectId", &iValue)) {
		this->setObjectId(iValue);
	}
	if (getJsInt32(cx, rparams, "actionObjectId", &iValue)) {
		this->setActionObjectId(iValue);
	}
	if (getJsInt32(cx, rparams, "actionId", &iValue)) {
		this->setActionId(iValue);
	}
	if (getJsBoolean(cx, rparams, "otherActions", &bValue)) {
		this->setOtherActions(bValue);
	}

	return true;
}

#if defined(AGTK_DEBUG)
void ObjectActionLinkConditionObjectActionChangedData::dump()
{
	ObjectActionLinkConditionData::dump();
	CCLOG("objectId:%d", this->getObjectId());
	CCLOG("actionObjectId:%d", this->getActionObjectId());
	CCLOG("actionId:%d", this->getActionId());
	CCLOG("otherActions:%d", this->getOtherActions());
}
#endif

//-------------------------------------------------------------------------------------------------------------------
bool ObjectActionLinkConditionSlopeTouchedData::init(const rapidjson::Value& json)
{
	if (!ObjectActionLinkConditionData::init(json)) {
		return false;
	}
	CC_ASSERT(json.HasMember("slopeTouched"));
	const rapidjson::Value& cond = json["slopeTouched"];
	this->setDirectionType((EnumDirectionType)cond["directionType"].GetInt());
	this->setDownwardType((EnumDownwardType)cond["downwardType"].GetInt());
	return true;
}

bool ObjectActionLinkConditionSlopeTouchedData::init(void *jsCx, void *jsObj)
{
	_type = kConditionSlopeTouched;
	//デフォルト値設定。
	this->setDirectionType(kDirectionUpper);
	this->setDownwardType(kDownwardLeft);

	auto cx = (JSContext *)jsCx;
	JS::RootedValue v(cx);
	JS::RootedObject rparams(cx, (JSObject *)jsObj);
	int iValue;
	if (getJsInt32(cx, rparams, "directionType", &iValue)) {
		this->setDirectionType((EnumDirectionType)iValue);
	}
	if (getJsInt32(cx, rparams, "downwardType", &iValue)) {
		this->setDownwardType((EnumDownwardType)iValue);
	}

	return true;
}

#if defined(AGTK_DEBUG)
void ObjectActionLinkConditionSlopeTouchedData::dump()
{
	ObjectActionLinkConditionData::dump();
	CCLOG("directionType:%d", this->getDirectionType());
	CCLOG("downwardType:%d", this->getDownwardType());
}
#endif

//-------------------------------------------------------------------------------------------------------------------
bool ObjectActionLinkConditionBuriedInWallData::init(const rapidjson::Value& json)
{
	if (!ObjectActionLinkConditionData::init(json)) {
		return false;
	}
	CC_ASSERT(json.HasMember("buriedInWall"));
	const rapidjson::Value& cond = json["buriedInWall"];
	this->setObjectId(cond["objectId"].GetInt());
	return true;
}

bool ObjectActionLinkConditionBuriedInWallData::init(void *jsCx, void *jsObj)
{
	_type = kConditionBuriedInWall;
	//デフォルト値設定。
	this->setObjectId(kSelfObject);

	auto cx = (JSContext *)jsCx;
	JS::RootedValue v(cx);
	JS::RootedObject rparams(cx, (JSObject *)jsObj);
	int iValue;
	if (getJsInt32(cx, rparams, "objectId", &iValue)) {
		this->setObjectId(iValue);
	}

	return true;
}

#if defined(AGTK_DEBUG)
void ObjectActionLinkConditionBuriedInWallData::dump()
{
	ObjectActionLinkConditionData::dump();
	CCLOG("objectId:%d", this->getObjectId());
}
#endif

//-------------------------------------------------------------------------------------------------------------------
ObjectActionLinkConditionScriptData::ObjectActionLinkConditionScriptData()
: ObjectActionLinkConditionData()
{
	_script = nullptr;
}

ObjectActionLinkConditionScriptData::~ObjectActionLinkConditionScriptData()
{
	CC_SAFE_RELEASE_NULL(_script);
}

bool ObjectActionLinkConditionScriptData::init(const rapidjson::Value& json)
{
	if (!ObjectActionLinkConditionData::init(json)) {
		return false;
	}
	CC_ASSERT(json.HasMember("script"));
	auto &cond = json["script"];
	if (cond["useCoffeeScript"].GetBool()) {
		this->setScript(cocos2d::__String::create(cond["javaScript"].GetString()));
	}
	else {
		this->setScript(cocos2d::__String::create(cond["script"].GetString()));
	}
	return true;
}

const char *ObjectActionLinkConditionScriptData::getScript()
{
	CC_ASSERT(_script);
	return _script->getCString();
}

#if defined(AGTK_DEBUG)
void ObjectActionLinkConditionScriptData::dump()
{
	ObjectActionLinkConditionData::dump();
	CCLOG("script:%s", this->getScript());
}
#endif

//-------------------------------------------------------------------------------------------------------------------
bool ObjectActionLinkConditionObjectHitData::init(const rapidjson::Value& json)
{
	if (!ObjectActionLinkConditionData::init(json)) {
		return false;
	}
	CC_ASSERT(json.HasMember("objectHit"));
	const rapidjson::Value& cond = json["objectHit"];
	this->setWallBit(cond["wallBit"].GetInt());
	this->setObjectType(cond["objectType"].GetInt());
	this->setObjectGroup(cond["objectGroup"].GetInt());
	this->setObjectId(cond["objectId"].GetInt());
	return true;
}

bool ObjectActionLinkConditionObjectHitData::init(void *jsCx, void *jsObj)
{
	_type = kConditionObjectHit;
	//デフォルト値設定。
	this->setWallBit(0);
	this->setObjectType(kObjectByGroup);
	this->setObjectGroup(kObjectGroupAll);
	this->setObjectId(-1);

	auto cx = (JSContext *)jsCx;
	JS::RootedValue v(cx);
	JS::RootedObject rparams(cx, (JSObject *)jsObj);
	int iValue;
	if (getJsInt32(cx, rparams, "wallBit", &iValue)) {
		this->setWallBit(iValue);
	}
	if (getJsInt32(cx, rparams, "objectType", &iValue)) {
		this->setObjectType(iValue);
	}
	if (isJsDefined(cx, rparams, "objectGroup")) {
		if (getJsInt32(cx, rparams, "objectGroup", &iValue)) {
			this->setObjectGroup(iValue);
		}
	}
	else {
		if (getJsInt32(cx, rparams, "objectTypeByType:", &iValue)) {
			this->setObjectGroup(iValue - kObjectGroupAllOffset);
		}
	}
	if (getJsInt32(cx, rparams, "objectId", &iValue)) {
		this->setObjectId(iValue);
	}

	return true;
}

#if defined(AGTK_DEBUG)
void ObjectActionLinkConditionObjectHitData::dump()
{
	ObjectActionLinkConditionData::dump();
	CCLOG("wallBit:%d", this->getWallBit());
	CCLOG("objectType:%d", this->getObjectType());
	CCLOG("objectGroup:%d", this->getObjectGroup());
	CCLOG("objectId:%d", this->getObjectId());
}
#endif

//-------------------------------------------------------------------------------------------------------------------
ObjectActionLinkConditionCustomData::ObjectActionLinkConditionCustomData()
	: ObjectActionLinkConditionData()
{
	_customId = -1;
	_valueJson = nullptr;
}

ObjectActionLinkConditionCustomData::~ObjectActionLinkConditionCustomData()
{
	CC_SAFE_RELEASE_NULL(_valueJson);
}

bool ObjectActionLinkConditionCustomData::init(const rapidjson::Value& json)
{
	if (!ObjectActionLinkConditionData::init(json)) {
		return false;
	}
	CC_ASSERT(json.HasMember("custom"));
	auto &cond = json["custom"];
	this->setCustomId(cond["customId"].GetInt());
	this->setValueJson(cocos2d::__String::create(cond["valueJson"].GetString()));
	return true;
}

const char *ObjectActionLinkConditionCustomData::getValueJson()
{
	CC_ASSERT(_valueJson);
	return _valueJson->getCString();
}

#if defined(AGTK_DEBUG)
void ObjectActionLinkConditionCustomData::dump()
{
	ObjectActionLinkConditionData::dump();
	CCLOG("customId:%d", this->getCustomId());
	CCLOG("valueJson:%s", this->getValueJson());
}
#endif

NS_DATA_END
NS_AGTK_END
