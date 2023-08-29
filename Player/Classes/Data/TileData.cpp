/**
 * @brief タイルデータ
 */
#include "TileData.h"
#include "Manager/GameManager.h"

NS_AGTK_BEGIN
NS_DATA_BEGIN

//-------------------------------------------------------------------------------------------------------------------
TileAnimationData::TileAnimationData()
{
	_x = 0;
	_y = 0;
	_frame300 = 0;
}

TileAnimationData::~TileAnimationData()
{
}

bool TileAnimationData::init(const rapidjson::Value& json)
{
	_x = json[0].GetInt();
	_y = json[1].GetInt();
	_frame300 = json[2].GetInt();
	return true;
}

#if defined(AGTK_DEBUG)
void TileAnimationData::dump()
{
	CCLOG("(x,y):(%d,%d)", this->getX(), this->getY());
	CCLOG("frame300:%d", this->getFrame300());
}
#endif

//-------------------------------------------------------------------------------------------------------------------
TileSwitchVariableConditionData::TileSwitchVariableConditionData()
{
}

TileSwitchVariableConditionData::~TileSwitchVariableConditionData()
{
}

bool TileSwitchVariableConditionData::init(const rapidjson::Value& json)
{
	this->setSwtch(json["swtch"].GetBool());
	//スイッチを条件に設定
	this->setSwitchObjectId(json["switchObjectId"].GetInt());
	this->setSwitchId(json["switchId"].GetInt());
	this->setSwitchValue(json["switchValue"].GetInt());
	this->setSwitchQualifierId((EnumQualifierType)json["switchQualifierId"].GetInt());
	//変数を条件に設定
	this->setVariableObjectId(json["variableObjectId"].GetInt());
	this->setVariableId(json["variableId"].GetInt());
	this->setVariableQualifierId((EnumQualifierType)json["variableQualifierId"].GetInt());
	this->setCompareOperator(json["compareOperator"].GetInt());
	this->setCompareValueType((EnumCompareValueType)json["compareValueType"].GetInt());
	this->setComparedValue(json["comparedValue"].GetDouble());
	this->setComparedVariableObjectId(json["comparedVariableObjectId"].GetInt());
	this->setComparedVariableId(json["comparedVariableId"].GetInt());
	this->setComparedVariableQualifierId((EnumQualifierType)json["comparedVariableQualifierId"].GetInt());
	return true;
}

#if defined(AGTK_DEBUG)
void TileSwitchVariableConditionData::dump()
{
	CCLOG("swtch:%d", this->getSwtch());
	CCLOG("switchObjectId:%d", this->getSwitchObjectId());
	CCLOG("switchId:%d", this->getSwitchId());
	CCLOG("switchValue:%d", this->getSwitchValue());
	CCLOG("variableObjectId:%d", this->getVariableObjectId());
	CCLOG("variableId:%d", this->getVariableId());
	CCLOG("variableQualifierId:%d", this->getVariableQualifierId());
	CCLOG("compareOperator:%d", this->getCompareOperator());
	CCLOG("compareValueType:%d", this->getCompareValueType());
	CCLOG("comparedValue:%d", this->getComparedValue());
	CCLOG("comparedVariableObjectId:%d", this->getComparedVariableObjectId());
	CCLOG("comparedVariableId:%d", this->getComparedVariableId());
}
#endif

//-------------------------------------------------------------------------------------------------------------------
TileSwitchVariableAssignData::TileSwitchVariableAssignData()
{
	_assignScript = nullptr;
}

TileSwitchVariableAssignData::~TileSwitchVariableAssignData()
{
	CC_SAFE_RELEASE_NULL(_assignScript);
}

bool TileSwitchVariableAssignData::init(const rapidjson::Value& json)
{
	this->setSwtch(json["swtch"].GetBool());
	this->setSwitchObjectId(json["switchObjectId"].GetInt());
	this->setSwitchQualifierId((EnumQualifierType)json["switchQualifierId"].GetInt());
	this->setSwitchId(json["switchId"].GetInt());
	this->setSwitchValue(json["switchValue"].GetInt());
	this->setVariableObjectId(json["variableObjectId"].GetInt());
	this->setVariableId(json["variableId"].GetInt());
	this->setVariableQualifierId((EnumQualifierType)json["variableQualifierId"].GetInt());
	this->setVariableAssignValueType(json["variableAssignValueType"].GetInt());
	this->setVariableAssignOperator((EnumVariableAssignOperatorType)json["variableAssignOperator"].GetInt());
	this->setAssignValue(json["assignValue"].GetDouble());
	this->setAssignVariableObjectId(json["assignVariableObjectId"].GetInt());
	this->setAssignVariableId(json["assignVariableId"].GetInt());
	this->setAssignVariableQualifierId((EnumQualifierType)json["assignVariableQualifierId"].GetInt());
	this->setRandomMin(json["randomMin"].GetInt());
	this->setRandomMax(json["randomMax"].GetInt());
	if (json.HasMember("useCoffeeScript") && json["useCoffeeScript"].GetBool()) {
		this->setAssignScript(cocos2d::__String::create(json["javaScript"].GetString()));
	}
	else {
		this->setAssignScript(cocos2d::__String::create(json["assignScript"].GetString()));
	}
	return true;
}

#if defined(AGTK_DEBUG)
void TileSwitchVariableAssignData::dump()
{
	CCLOG("swtch:%d", this->getSwtch());
	CCLOG("switchObjectId:%d", this->getSwitchObjectId());
	CCLOG("switchQualifierId:%d", this->getSwitchQualifierId());
	CCLOG("switchId:%d", this->getSwitchId());
	CCLOG("switchValue:%s", this->getSwitchValue());
	CCLOG("variableObjectId:%d", this->getVariableObjectId());
	CCLOG("variableId:%d", this->getVariableId());
	CCLOG("variableQualifierId:%d", this->getVariableQualifierId());
	CCLOG("variableAssignValueType:%d", this->getVariableAssignValueType());
	CCLOG("variableAssignOperatorType:%d", this->getVariableAssignOperator());
	CCLOG("assignValue:%f", this->getAssignValue());
	CCLOG("assignVariableObjectId:%d", this->getAssignVariableObjectId());	
	CCLOG("assignVariableId:%d", this->getAssignVariableId());
	CCLOG("assignVariableQualifierId:%d", this->getAssignVariableQualifierId());
	CCLOG("randomMin:%d", this->getRandomMin());
	CCLOG("randomMax:%d", this->getRandomMax());
	CCLOG("assignScript:%s", this->getAssignScript());
}
#endif

//-------------------------------------------------------------------------------------------------------------------
TileData::TileData()
{
	_conditionType = kConditionMax;
	_group = 0;
	_targetObjectGroupBit = 0;
	_areaAttributeFlag = false;
	_areaAttribute = 0;
	_moveSpeedChanged = false;
	_moveSpeedChange = 0;
	_jumpChanged = false;
	_jumpChange = 0;
	_moveXFlag = false;
	_moveX = 0;
	_moveYFlag = false;
	_moveY = 0;
	_slipChanged = false;
	_slipChange = 0;
	_getDead = false;
	_hpChanged = false;
	_hpChange = 0;
	_triggerPeriodically = false;
	_triggerPeriod = 0;
	_gravityEffectChanged = false;
	_gravityEffectChange = 0;
	_a = 255;
	_r = 255;
	_g = 255;
	_b = 255;
	_physicsRepulsion = 0;
	_physicsFriction = 0;
	_tileAnimationData = nullptr;
	_memo = nullptr;
	_gimmickDescription = nullptr;
	_playerMoveType = kPlayerMoveMax;
	_tileAttackAreaTouched = false;
	_timePassed = false;
	_passedTime = 0;
	_switchVariableCondition = false;
	_switchVariableConditionList = nullptr;
	_gimmickTargetObjectGroupBit = 0;
//	_changeTile = false;
	_changeTileType = kChangeTileMax;
	_changeTileX = 0;
	_changeTileY = 0;
	_playSe = false;
	_playSeId = 0;
	_appearObject = false;
	_appearObjectId = 0;
	_showEffect = false;
	_showEffectId = 0;
	_switchVariableAssign = false;
	_switchVariableAssignList = nullptr;
	_gimmickMemo = nullptr;
}

TileData::~TileData()
{
	CC_SAFE_RELEASE_NULL(_tileAnimationData);
	CC_SAFE_RELEASE_NULL(_memo);
	CC_SAFE_RELEASE_NULL(_gimmickDescription);
	CC_SAFE_RELEASE_NULL(_gimmickMemo);
	CC_SAFE_RELEASE_NULL(_switchVariableConditionList);
	CC_SAFE_RELEASE_NULL(_switchVariableAssignList);
}

bool TileData::init(const rapidjson::Value& json)
{
	CC_ASSERT(json.HasMember("tileGroup"));
	if (json.HasMember("tileGroup")) {
		this->setGroup( json["tileGroup"].GetInt() );
	}
	CC_ASSERT(json.HasMember("conditionType"));
	this->setConditionType((EnumConditionType)json["conditionType"].GetInt());
	CC_ASSERT(json.HasMember("objectGroupBit"));
	if (json.HasMember("objectGroupBit")) {
		this->setTargetObjectGroupBit( json["objectGroupBit"].GetInt() );
	}
	CC_ASSERT(json.HasMember("areaAttributeFlag"));
	this->setAreaAttributeFlag(json["areaAttributeFlag"].GetBool());
	CC_ASSERT(json.HasMember("areaAttribute"));
	this->setAreaAttribute(json["areaAttribute"].GetInt());
	CC_ASSERT(json.HasMember("moveSpeedChanged"));
	this->setMoveSpeedChanged(json["moveSpeedChanged"].GetBool());
	CC_ASSERT(json.HasMember("moveSpeedChange"));
	this->setMoveSpeedChange(json["moveSpeedChange"].GetDouble());
	CC_ASSERT(json.HasMember("jumpChanged"));
	this->setJumpChanged(json["jumpChanged"].GetBool());
	CC_ASSERT(json.HasMember("jumpChange"));
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
	CC_ASSERT(json.HasMember("slipChanged"));
	this->setSlipChanged(json["slipChanged"].GetBool());
	CC_ASSERT(json.HasMember("slipChange"));
	this->setSlipChange(json["slipChange"].GetDouble());
	CC_ASSERT(json.HasMember("getDead"));
	this->setGetDead(json["getDead"].GetBool());
	CC_ASSERT(json.HasMember("hpChanged"));
	this->setHpChanged(json["hpChanged"].GetBool());
	CC_ASSERT(json.HasMember("hpChange"));
	this->setHpChange(json["hpChange"].GetDouble());
	if (json.HasMember("triggerPeriodically")) {
		this->setTriggerPeriodically(json["triggerPeriodically"].GetBool());
	}
	if (json.HasMember("triggerPeriod")) {
		this->setTriggerPeriod(json["triggerPeriod"].GetDouble());
	}
	CC_ASSERT(json.HasMember("gravityEffectChanged"));
	this->setGravityEffectChanged(json["gravityEffectChanged"].GetBool());
	CC_ASSERT(json.HasMember("gravityEffectChange"));
	this->setGravityEffectChange(json["gravityEffectChange"].GetDouble());
	CC_ASSERT(json.HasMember("a"));
	this->setA(json["a"].GetInt());
	CC_ASSERT(json.HasMember("r"));
	this->setR(json["r"].GetInt());
	CC_ASSERT(json.HasMember("g"));
	this->setG(json["g"].GetInt());
	CC_ASSERT(json.HasMember("b"));
	this->setB(json["b"].GetInt());
	CC_ASSERT(json.HasMember("physicsRepulsion"));
	this->setPhysicsRepulsion(json["physicsRepulsion"].GetDouble());
	CC_ASSERT(json.HasMember("physicsFriction"));
	this->setPhysicsFriction(json["physicsFriction"].GetDouble());
	if (json.HasMember("tileAnimation")) {
		auto arr = cocos2d::__Array::create();
		for (rapidjson::SizeType i = 0; i < json["tileAnimation"].Size(); i++) {
			auto tileAnimation = TileAnimationData::create(json["tileAnimation"][i]);
			arr->addObject(tileAnimation);
		}
		this->setTileAnimationData(arr);
	}
	CC_ASSERT(json.HasMember("memo"));
	this->setMemo(cocos2d::__String::create(json["memo"].GetString()));
	CC_ASSERT(json.HasMember("gimmickDescription"));
	this->setGimmickDescription(cocos2d::__String::create(json["gimmickDescription"].GetString()));
	this->setPlayerMoveType((EnumPlayerMoveType)json["playerMoveType"].GetInt());
	this->setTileAttackAreaTouched(json["tileAttackAreaTouched"].GetBool());
	CC_ASSERT(json.HasMember("timePassed"));
	this->setTimePassed(json["timePassed"].GetBool());
	CC_ASSERT(json.HasMember("passedTime"));
	this->setPassedTime(json["passedTime"].GetInt());
	CC_ASSERT(json.HasMember("switchVariableCondition"));
	this->setSwitchVariableCondition(json["switchVariableCondition"].GetBool());
	if (this->getSwitchVariableCondition() && json.HasMember("switchVariableConditionList")) {
		CC_ASSERT(json.HasMember("switchVariableConditionList"));
		auto switchVariableConditionList = cocos2d::__Array::create();
		for (rapidjson::SizeType i = 0; i < json["switchVariableConditionList"].Size(); i++) {
			auto data = TileSwitchVariableConditionData::create(json["switchVariableConditionList"][i]);
			switchVariableConditionList->addObject(data);
		}
		this->setSwitchVariableConditionList(switchVariableConditionList);
	}
	CC_ASSERT(json.HasMember("gimmickObjectGroupBit"));
	if (json.HasMember("gimmickObjectGroupBit")) {
		this->setGimmickTargetObjectGroupBit(json["gimmickObjectGroupBit"].GetInt());
	}
	
	CC_ASSERT(json.HasMember("changeTileType"));
	this->setChangeTileType((EnumChangeTileType)json["changeTileType"].GetInt());
	CC_ASSERT(json.HasMember("changeTileX"));
	this->setChangeTileX(json["changeTileX"].GetInt());
	CC_ASSERT(json.HasMember("changeTileY"));
	this->setChangeTileY(json["changeTileY"].GetInt());
	CC_ASSERT(json.HasMember("playSe"));
	this->setPlaySe(json["playSe"].GetBool());
	CC_ASSERT(json.HasMember("playSeId"));
	this->setPlaySeId(json["playSeId"].GetInt());
	CC_ASSERT(json.HasMember("appearObject"));
	this->setAppearObject(json["appearObject"].GetBool());
	CC_ASSERT(json.HasMember("appearObjectId"));
	this->setAppearObjectId(json["appearObjectId"].GetInt());
	CC_ASSERT(json.HasMember("showEffect"));
	this->setShowEffect(json["showEffect"].GetBool());
	CC_ASSERT(json.HasMember("showEffectId"));
	this->setShowEffectId(json["showEffectId"].GetInt());
	CC_ASSERT(json.HasMember("showParticle"));
	this->setShowParticle(json["showParticle"].GetBool());
	CC_ASSERT(json.HasMember("showParticleId"));
	this->setShowParticleId(json["showParticleId"].GetInt());
	CC_ASSERT(json.HasMember("switchVariableAssign"));
	this->setSwitchVariableAssign(json["switchVariableAssign"].GetBool());
	if (this->getSwitchVariableAssign() && json.HasMember("switchVariableAssignList")) {
		CC_ASSERT(json.HasMember("switchVariableAssignList"));
		auto switchVariableAssignList = cocos2d::__Array::create();
		for (rapidjson::SizeType i = 0; i < json["switchVariableAssignList"].Size(); i++) {
			auto data = TileSwitchVariableAssignData::create(json["switchVariableAssignList"][i]);
			switchVariableAssignList->addObject(data);
		}
		this->setSwitchVariableAssignList(switchVariableAssignList);
	}
	CC_ASSERT(json.HasMember("gimmickMemo"));
	this->setGimmickMemo(cocos2d::__String::create(json["gimmickMemo"].GetString()));
	return true;
}

const char *TileData::getMemo()
{
	CC_ASSERT(_memo);
	return _memo->getCString();
}

const char *TileData::getGimmickDescription()
{
	CC_ASSERT(_gimmickDescription);
	return _gimmickDescription->getCString();
}

const char *TileData::getGimmickMemo()
{
	CC_ASSERT(_gimmickMemo);
	return _gimmickMemo->getCString();
}

int TileData::getTileAnimationMaxFrame300()
{
	int frame300 = 0;
	cocos2d::Ref *ref = nullptr;
	CCARRAY_FOREACH(this->getTileAnimationData(), ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto p = static_cast<TileAnimationData *>(ref);
#else
		auto p = dynamic_cast<TileAnimationData *>(ref);
#endif
		frame300 += p->getFrame300();
	}
	return frame300;
}

int TileData::getGroupBit()const
{
	return (1 << _group);
}

#if defined(AGTK_DEBUG)
void TileData::dump()
{			
	CCLOG("Group:%d", this->getGroup());
	CCLOG("objectGroupBit:%d", this->getTargetObjectGroupBit());
	CCLOG("moveSpeedChanged:%d", this->getMoveSpeedChanged());
	CCLOG("moveSpeedChange:%f", this->getMoveSpeedChange());
	CCLOG("jumpChanged:%d", this->getJumpChanged());
	CCLOG("jumpChange:%f", this->getJumpChange());
	CCLOG("moveXFlag:%d", this->getMoveXFlag());
	CCLOG("moveX:%f", this->getMoveX());
	CCLOG("moveYFlag:%d", this->getMoveYFlag());
	CCLOG("moveY:%f", this->getMoveY());
	CCLOG("slipChanged:%d", this->getSlipChanged());
	CCLOG("slipChange:%f", this->getSlipChange());
	CCLOG("getDead:%d", this->getGetDead());
	CCLOG("hpChanged:%d", this->getHpChanged());
	CCLOG("hpChange:%f", this->getHpChange());
	CCLOG("triggerPeriodically:%d", this->getTriggerPeriodically());
	CCLOG("triggerPeriod:%f", this->getTriggerPeriod());
	CCLOG("gravityEffectChanged:%d", this->getGravityEffectChanged());
	CCLOG("gravityEffectChange:%f", this->getGravityEffectChange());
	CCLOG("a:%d", this->getA());
	CCLOG("r:%d", this->getR());
	CCLOG("g:%d", this->getG());
	CCLOG("b:%d", this->getB());
	CCLOG("physicsRepulsion:%f", this->getPhysicsRepulsion());
	CCLOG("physicsFriction:%f", this->getPhysicsFriction());
	if (this->getTileAnimationData()) {
		cocos2d::Ref *ref = nullptr;
		CCARRAY_FOREACH(this->getTileAnimationData(), ref) {
			auto p = dynamic_cast<TileAnimationData *>(ref);
			p->dump();
		}
	}
	CCLOG("memo:%s", this->getMemo());
	CCLOG("gimmickDescription:%s", this->getGimmickDescription());
	CCLOG("playerMoveType:%d", this->getPlayerMoveType());
	CCLOG("tileAttackAreaTouched:%d", this->getTileAttackAreaTouched());
	CCLOG("timePassed:%d", this->getTimePassed());
	CCLOG("passedTime:%d", this->getPassedTime());
	CCLOG("switchVariableCondition:%d", this->getSwitchVariableCondition());
	CCLOG("gimmickTargetObjectGroupBit:%d", this->getGimmickTargetObjectGroupBit());
	CCLOG("changeTileType:%d", this->getChangeTileType());
	CCLOG("changeTileX:%d", this->getChangeTileX());
	CCLOG("changeTileY:%d", this->getChangeTileY());
	CCLOG("playSe:%d", this->getPlaySe());
	CCLOG("playSeId:%d", this->getPlaySeId());
	CCLOG("appearObject:%d", this->getAppearObject());
	CCLOG("appearObjectId:%d", this->getAppearObjectId());
	CCLOG("showEffect:%d", this->getShowEffect());
	CCLOG("showEffectId:%d", this->getShowEffectId());
	CCLOG("switchVariableAssign:%d", this->getSwitchVariableAssign());
	CCLOG("gimmickMemo:%s", this->getGimmickMemo());
}
#endif

//-------------------------------------------------------------------------------------------------------------------
TilesetData::TilesetData()
{
	_id = 0;
	_name = nullptr;
	_imageId = 0;
	_memo = nullptr;
	_horzTileCount = 0;
	_vertTileCount = 0;
	_wallSetting = nullptr;//->cocos2d::Integer
	_tileDataList = nullptr;//->TileData
	_folder = false;
	_children = nullptr;
	_tilesetType = kMax;
}

TilesetData::~TilesetData()
{
	CC_SAFE_RELEASE_NULL(_name);
	CC_SAFE_RELEASE_NULL(_memo);
	CC_SAFE_RELEASE_NULL(_wallSetting);
	CC_SAFE_RELEASE_NULL(_tileDataList);
	CC_SAFE_RELEASE_NULL(_children);
}

bool TilesetData::init(const rapidjson::Value& json)
{
	CC_ASSERT(json.HasMember("id"));
	this->setId(json["id"].GetInt());
	CC_ASSERT(json.HasMember("name"));
	this->setName(cocos2d::__String::create(json["name"].GetString()));
	this->setFolder(json["folder"].GetBool());
	if (this->getFolder()) {
		auto dic = cocos2d::__Dictionary::create();
		if (json.HasMember("children")) {
			for (rapidjson::SizeType i = 0; i < json["children"].Size(); i++) {
				auto p = TilesetData::create(json["children"][i]);
#if defined(AGTK_DEBUG)
				CC_ASSERT(dic->objectForKey(p->getId()) == nullptr);
#endif
				dic->setObject(p, p->getId());
			}
		}
		this->setChildren(dic);
		return true;
	}

	CC_ASSERT(json.HasMember("tilesetType"));
	this->setTilesetType((EnumTilesetType)json["tilesetType"].GetInt());
	CC_ASSERT(json.HasMember("imageId"));
	this->setImageId(json["imageId"].GetInt());
	CC_ASSERT(json.HasMember("memo"));
	this->setMemo(cocos2d::__String::create(json["memo"].GetString()));
	CC_ASSERT(json.HasMember("folder"));
	this->setFolder(json["folder"].GetBool());
	CC_ASSERT(json.HasMember("horzTileCount"));
	this->setHorzTileCount(json["horzTileCount"].GetInt());
	CC_ASSERT(json.HasMember("vertTileCount"));
	this->setVertTileCount(json["vertTileCount"].GetInt());
	CC_ASSERT(json.HasMember("wallSetting"));
	auto wallSetting = cocos2d::__Array::create();
	for (rapidjson::SizeType i = 0; i < json["wallSetting"].Size(); i++) {
		auto integer = cocos2d::Integer::create(json["wallSetting"][i].GetInt());
		wallSetting->addObject(integer);
	}
	this->setWallSetting(wallSetting);
	CC_ASSERT(json.HasMember("tileData"));
	auto dic = cocos2d::__Dictionary::create();
	const rapidjson::Value& tileData = json["tileData"];
	for (rapidjson::Value::ConstMemberIterator itr = tileData.MemberBegin(); itr != tileData.MemberEnd(); itr++) {
		auto name = itr->name.GetString();
		auto data = TileData::create(itr->value);
#if defined(AGTK_DEBUG)
		CC_ASSERT(dic->objectForKey(std::atoi(name)) == nullptr);
#endif
		dic->setObject(data, std::atoi(name));
	}
	this->setTileDataList(dic);
	return true;
}

const char *TilesetData::getName()
{
	CC_ASSERT(_name);
	return _name->getCString();
}

const char *TilesetData::getMemo()
{
	CC_ASSERT(_memo);
	return _memo->getCString();
}

int TilesetData::getWallSetting(int id)
{
	CC_ASSERT(_wallSetting);
	if (id >= _wallSetting->count()) {
		return 0;
	}
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	auto p = static_cast<cocos2d::Integer *>(_wallSetting->getObjectAtIndex(id));
#else
	auto p = dynamic_cast<cocos2d::Integer *>(_wallSetting->getObjectAtIndex(id));
	CC_ASSERT(p);
#endif
	return p->getValue();
}

bool TilesetData::getWallSetting(int id, WallSet set)
{
	int v = this->getWallSetting(id);
	if (v & 1 << (int)set) {
		return true;
	}
	return false;
}

TileData *TilesetData::getTileData(int id)
{
	CC_ASSERT(this->getTileDataList());
	return dynamic_cast<TileData *>(this->getTileDataList()->objectForKey(id));
}

TileData *TilesetData::getTileData(int x, int y)
{
	CC_ASSERT(this->getTileDataList());
	int id = x + y * this->getHorzTileCount();
	return dynamic_cast<TileData *>(this->getTileDataList()->objectForKey(id));
}

#if defined(AGTK_DEBUG)
void TilesetData::dump()
{
	CCLOG("id:%d", this->getId());
	CCLOG("name:%s", this->getName());
	CCLOG("folder:%d", this->getFolder());
	if (this->getFolder() && this->getChildren()) {
		auto keys = this->getChildren()->allKeys();
		cocos2d::Ref *ref = nullptr;
		CCARRAY_FOREACH(keys, ref) {
			auto id = dynamic_cast<cocos2d::Integer *>(ref);
			auto data = dynamic_cast<TilesetData *>(this->getChildren()->objectForKey(id->getValue()));
			data->dump();
		}
		return;
	}
	CCLOG("tilesetType:%d", this->getTilesetType());
	CCLOG("imageId:%d", this->getImageId());
	CCLOG("memo:%s", this->getMemo());
	CCLOG("folder:%d", this->getFolder());
	CCLOG("horzTileCount:%d", this->getHorzTileCount());
	CCLOG("vertTileCount:%d", this->getVertTileCount());

	std::string tmp = "wallSetting:[";
	cocos2d::Ref *ref = nullptr;
	CCARRAY_FOREACH(this->getWallSetting(), ref) {
		auto i = dynamic_cast<cocos2d::Integer *>(ref);
		tmp += std::to_string(i->getValue()) + std::string(",");
	}
	tmp += "]";
	CCLOG("%s", tmp.c_str());

	auto keys = this->getTileDataList()->allKeys();
	CCARRAY_FOREACH(keys, ref) {
		auto id = dynamic_cast<cocos2d::Integer *>(ref);
		auto data = dynamic_cast<TileData *>(this->getTileDataList()->objectForKey(id->getValue()));
		data->dump();
	}
}
#endif

//-------------------------------------------------------------------------------------------------------------------
Tile::Tile()
{
	_tilesetId = 0;
	_x = 0;
	_y = 0;
	_position = cocos2d::Vec2::ZERO;
	_id = nullptr;
	for (int j = 0; j < VertSubtileCount; j++) {
		for (int i = 0; i < HorzSubtileCount; i++) {
			subtileX[i][j] = -1;
			subtileY[i][j] = -1;
		}
	}
}

Tile::~Tile()
{
	CC_SAFE_RELEASE_NULL(_id);
}

bool Tile::init(const rapidjson::Value& json, const char* id)
{
	//json -> tilesetId, x, y
	CC_ASSERT(json.HasMember("tilesetId"));
	this->setTilesetId(json["tilesetId"].GetInt());
	CC_ASSERT(json.HasMember("x"));
	this->setX(json["x"].GetInt());
	CC_ASSERT(json.HasMember("y"));
	this->setY(json["y"].GetInt());
	if (json.HasMember("subtileInfo")) {
		for (int j = 0; j < VertSubtileCount; j++) {
			for (int i = 0; i < HorzSubtileCount; i++) {
				subtileX[i][j] = json["subtileInfo"][i + j * 2][0].GetInt();
				subtileY[i][j] = json["subtileInfo"][i + j * 2][1].GetInt();
			}
		}
	}
	//position
	std::string sid = id;
	int pos = sid.find(",");
	CC_ASSERT(pos != std::string::npos);
	int x = std::stoi(sid.substr(0, pos));
	int y = std::stoi(sid.substr(pos + 1));
	this->setPosition(cocos2d::Vec2(x, y));
	//id
	this->setId(cocos2d::__String::create(id));
	return true;
}

const char *Tile::getId()
{
	CC_ASSERT(_id);
	return _id->getCString();
}

int Tile::getSubtileX(int sx, int sy)
{
	CCASSERT(sx >= 0 && sx < HorzSubtileCount && sy >= 0 && sy < VertSubtileCount, "subtileX, out of range");
	return subtileX[sx][sy];
}

int Tile::getSubtileY(int sx, int sy)
{
	CCASSERT(sx >= 0 && sx < HorzSubtileCount && sy >= 0 && sy < VertSubtileCount, "subtileY, out of range");
	return subtileY[sx][sy];
}

#if defined(AGTK_DEBUG)
void Tile::dump()
{
	CCLOG("tilesetId:%d", this->getTilesetId());
	CCLOG("x,y: %d,%d", this->getX(), this->getY());
	CCLOG("pos(%f,%f)", this->getPosition().x, this->getPosition().y);
	CCLOG("id:%s", this->getId());
	CCLOG("subtile x,y: (%d,%d)(%d,%d)(%d,%d)(%d,%d)", this->getSubtileX(0, 0), this->getSubtileY(0, 0), this->getSubtileX(1, 0), this->getSubtileY(1, 0), this->getSubtileX(0, 1), this->getSubtileY(0, 1), this->getSubtileX(1, 1), this->getSubtileY(1, 1));
}
#endif

NS_DATA_END
NS_AGTK_END
