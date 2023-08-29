/**
 * @brief オブジェクトデータ
 */
#include "ObjectData.h"
#include "AssetData.h"
#include "Data/SceneData.h"

NS_AGTK_BEGIN
NS_DATA_BEGIN

//-------------------------------------------------------------------------------------------------------------------
ObjectActionData::ObjectActionData()
{
	_id = 0;
	_name = nullptr;
	_a = 255;
	_r = 255;
	_g = 255;
	_b = 255;
	_animMotionId = 0;
	_animDirectionId = -1;
	_takeOverMotion = false;
	_jumpable = false;
	_enableCustomizedJump = false;
	_keepDirection = false;
	_ignoreGravity = false;
	_gravity = 0.0;
	_reflectGravityToDisplayDirection = false;
	_moveSpeed = 0.0;
	_jumpSpeed = 0.0;
	_upDownMoveSpeed = 0.0;
	_turnSpeed = 0.0;
	_objCommandList = nullptr;
	_memo = nullptr;
	_x = 0;
	_y = 0;
	_width = 0;
	_height = 0;
}

ObjectActionData::~ObjectActionData()
{
	CC_SAFE_RELEASE_NULL(_name);
	CC_SAFE_RELEASE_NULL(_objCommandList);
	CC_SAFE_RELEASE_NULL(_memo);
}

bool ObjectActionData::init(const rapidjson::Value& json)
{
	//id
	CC_ASSERT(json.HasMember("id"));
	this->setId(json["id"].GetInt());
	//name
	CC_ASSERT(json.HasMember("name"));
	this->setName(cocos2d::__String::create(json["name"].GetString()));
	CC_ASSERT(json.HasMember("a"));
	this->setA(json["a"].GetInt());
	CC_ASSERT(json.HasMember("r"));
	this->setR(json["r"].GetInt());
	CC_ASSERT(json.HasMember("g"));
	this->setG(json["g"].GetInt());
	CC_ASSERT(json.HasMember("b"));
	this->setB(json["b"].GetInt());
	//animMotionId
	if (json.HasMember("animActionId")){
		this->setAnimMotionId(json["animActionId"].GetInt());
	} else {
		CC_ASSERT(json.HasMember("animMotionId"));
		this->setAnimMotionId(json["animMotionId"].GetInt());
	}
	//animMotionId
	if (json.HasMember("animDirectionId")) {
		this->setAnimDirectionId(json["animDirectionId"].GetInt());
	}
	//takeOverMotion
	CC_ASSERT(json.HasMember("takeOverMotion"));
	this->setTakeOverMotion(json["takeOverMotion"].GetBool());
	//jumpable
	CC_ASSERT(json.HasMember("jumpable"));
	this->setJumpable(json["jumpable"].GetBool());
	//enableCustomizedJump
	if (json.HasMember("enableCustomizedJump")) {
		this->setEnableCustomizedJump(json["enableCustomizedJump"].GetBool());
	}
	//ignoreMoveInput
	CC_ASSERT(json.HasMember("ignoreMoveInput"));
	this->setIgnoreMoveInput(json["ignoreMoveInput"].GetBool());
	//keepDirection
	CC_ASSERT(json.HasMember("keepDirection"));
	this->setKeepDirection(json["keepDirection"].GetBool());
	//ignoreGravity
	CC_ASSERT(json.HasMember("ignoreGravity"));
	this->setIgnoreGravity(json["ignoreGravity"].GetBool());
	//gravity
	CC_ASSERT(json.HasMember("gravity"));
	this->setGravity(json["gravity"].GetDouble());
	//reflectGravityToDisplayDirection
	if (json.HasMember("reflectGravityToDisplayDirection")) {
		this->setReflectGravityToDisplayDirection(json["reflectGravityToDisplayDirection"].GetBool());
	}
	//moveSpeed
	CC_ASSERT(json.HasMember("moveSpeed"));
	this->setMoveSpeed(json["moveSpeed"].GetDouble());
	//upDownMoveSpeed
	CC_ASSERT(json.HasMember("upDownMoveSpeed"));
	this->setUpDownMoveSpeed(json["upDownMoveSpeed"].GetDouble());
	//jumpSpeed
	CC_ASSERT(json.HasMember("jumpSpeed"));
	this->setJumpSpeed(json["jumpSpeed"].GetDouble());
	//turnSpeed
	CC_ASSERT(json.HasMember("turnSpeed"));
	this->setTurnSpeed(json["turnSpeed"].GetDouble());
	//objCommandList
	CC_ASSERT(json.HasMember("objCommandList"));
	auto objCommandList = cocos2d::__Dictionary::create();
	for (rapidjson::SizeType i = 0; i < json["objCommandList"].Size(); i++) {
		auto objCommandData = ObjectCommandData::create(json["objCommandList"][i]);
		if (objCommandData == nullptr) continue;
#if defined(AGTK_DEBUG)
		CC_ASSERT(objCommandList->objectForKey(objCommandData->getId()) == nullptr);
#endif
		objCommandList->setObject(objCommandData, objCommandData->getId());
	}
	this->setObjCommandList(objCommandList);
	//memo
	CC_ASSERT(json.HasMember("memo"));
	this->setMemo(cocos2d::__String::create(json["memo"].GetString()));
	//x
	CC_ASSERT(json.HasMember("x"));
	this->setX(json["x"].GetDouble());
	//y
	CC_ASSERT(json.HasMember("y"));
	this->setY(json["y"].GetDouble());
	//width
	CC_ASSERT(json.HasMember("width"));
	this->setWidth(json["width"].GetDouble());
	//height
	CC_ASSERT(json.HasMember("width"));
	this->setHeight(json["height"].GetDouble());
	return true;
}

const char *ObjectActionData::getName()
{
	CC_ASSERT(_name);
	return _name->getCString();
}

const char *ObjectActionData::getMemo()
{
	CC_ASSERT(_memo);
	return _memo->getCString();
}

agtk::data::ObjectCommandData *ObjectActionData::getObjectCommandData(int id)
{
	CC_ASSERT(_objCommandList);
	return dynamic_cast<agtk::data::ObjectCommandData *>(this->getObjCommandList()->objectForKey(id));
}

#if defined(AGTK_DEBUG)
void ObjectActionData::dump()
{
	CCLOG("id:%d", this->getId());
	CCLOG("name:%s", this->getName());
	CCLOG("a:%d", this->getA());
	CCLOG("r:%d", this->getR());
	CCLOG("g:%d", this->getG());
	CCLOG("b:%d", this->getB());
	CCLOG("animMotionId:%d", this->getAnimMotionId());
	CCLOG("animDirectionId:%d", this->getAnimDirectionId());
	CCLOG("takeOverMotion:%d", this->getTakeOverMotion());
	CCLOG("jumpable:%d", this->getJumpable());
	CCLOG("enableCustomizedJump:%d", this->getEnableCustomizedJump());
	CCLOG("ignoreMoveInput;%d", this->getIgnoreMoveInput());
	CCLOG("keepDirection:%d", this->getKeepDirection());
	CCLOG("ignoreGravity:%d", this->getIgnoreGravity());
	CCLOG("gravity:%d", this->getGravity());
	CCLOG("reflectGravityToDisplayDirection:%d", this->getReflectGravityToDisplayDirection());
	CCLOG("moveSpeed:%d", this->getMoveSpeed());
	CCLOG("upDownMoveSpeed:%d", this->getUpDownMoveSpeed());
	CCLOG("jumpSpeed:%d", this->getJumpSpeed());
	CCLOG("turnSpeed:%d", this->getTurnSpeed());
	cocos2d::DictElement *el = nullptr;
	auto objCommandList = this->getObjCommandList();
	CCDICT_FOREACH(objCommandList, el) {
		auto p = dynamic_cast<ObjectCommandData *>(el->getObject());
		p->dump();
	}
	CCLOG("memo:%s", this->getMemo());
	CCLOG("x:%d", this->getX());
	CCLOG("y:%d", this->getY());
	CCLOG("width:%d", this->getWidth());
	CCLOG("height:%d", this->getHeight());
}
#endif

//-------------------------------------------------------------------------------------------------------------------
ObjectInputConditionData::ObjectInputConditionData()
{
	_id = 0;
	_operationKeyId = 0;
	_triggerType = EnumTriggerType::kTriggerMax;
	_condAnd = false;
	_directionBit = 0;
	_directionInputType = EnumDirectionInputType::kDirectionInputCross;
	_useKey = false;
	_acceptFrameCount = 0;
}

ObjectInputConditionData::~ObjectInputConditionData()
{
}

bool ObjectInputConditionData::init(const rapidjson::Value& json)
{
	//id
	if (!json.HasMember("id")) {
		CC_ASSERT(0);
		return false;
	}
	this->setId(json["id"].GetInt());
	//operationKeyId
	if (!json.HasMember("operationKeyId")) {
		CC_ASSERT(0);
		return false;
	}
	this->setOperationKeyId(json["operationKeyId"].GetInt());
	//triggerType
	if (!json.HasMember("triggerType")) {
		CC_ASSERT(0);
		return false;
	}
	this->setTriggerType((EnumTriggerType)json["triggerType"].GetInt());
	//condAnd
	if (!json.HasMember("condAnd")) {
		CC_ASSERT(0);
		return false;
	}
	this->setCondAnd(json["condAnd"].GetBool());

	//directionBit
	CHECK_JSON_MENBER("directionBit");
	this->setDirectionBit(json["directionBit"].GetInt());

	//directionInputType
	CHECK_JSON_MENBER("directionInputType");
	this->setDirectionInputType((EnumDirectionInputType)json["directionInputType"].GetInt());

	//useKey
	CHECK_JSON_MENBER("useKey");
	this->setUseKey(json["useKey"].GetBool());

	//acceptFrameCount
	CHECK_JSON_MENBER("acceptFrameCount");
	this->setAcceptFrameCount(json["acceptFrameCount"].GetInt());

	return true;
}

#if defined(AGTK_DEBUG)
void ObjectInputConditionData::dump()
{
	CCLOG("id:%d", this->getId());
	CCLOG("operationKeyId:%d", this->getOperationKeyId());
	CCLOG("triggerType:%d", this->getTriggerType());
	CCLOG("condAnd:%d", this->getCondAnd());
	CCLOG("directionBit:%d", this->getDirectionBit());
	CCLOG("directionInputType:%d", this->getDirectionInputType());
	CCLOG("useKey:%s", DUMP_BOOLTEXT(this->getUseKey()));
	CCLOG("acceptFrameCount:%d", this->getAcceptFrameCount());
}
#endif

//-------------------------------------------------------------------------------------------------------------------
ObjectActionLinkData::ObjectActionLinkData()
{
	_id = 0;
	_actionIdPair = nullptr;
	_priority = 0;
	_a = 255;
	_r = 255;
	_g = 255;
	_b = 255;
	_changeConditionType = EnumChangeConditionType::kMax;
	_noInput = false;
	_useInput = false;
	_inputConditionList = nullptr;
	_linkConditionList = nullptr;
	_memo = nullptr;
	_coordList = nullptr;
	_inputConditionGroupList = nullptr;
	_linkConditionGroupList = nullptr;
	_commonActionSettingData = nullptr;
}

ObjectActionLinkData::~ObjectActionLinkData()
{
	CC_SAFE_RELEASE_NULL(_actionIdPair);
	CC_SAFE_RELEASE_NULL(_inputConditionList);
	CC_SAFE_RELEASE_NULL(_linkConditionList);
	CC_SAFE_RELEASE_NULL(_memo);
	CC_SAFE_RELEASE_NULL(_coordList);
	CC_SAFE_RELEASE_NULL(_inputConditionGroupList);
	CC_SAFE_RELEASE_NULL(_linkConditionGroupList);
}

bool ObjectActionLinkData::init(const rapidjson::Value& json)
{
	//id
	CC_ASSERT(json.HasMember("id"));
	this->setId(json["id"].GetInt());
	//actionIdPair
	if (json.HasMember("actionIdPair")) {
		auto actionIdPair = cocos2d::__Array::create();
		for (rapidjson::SizeType i = 0; i < json["actionIdPair"].Size(); i++) {
			actionIdPair->addObject(cocos2d::Integer::create(json["actionIdPair"][i].GetInt()));
		}
		this->setActionIdPair(actionIdPair);
	}
	else {
		CC_ASSERT(json.HasMember("typeIdPair"));
		auto actionIdPair = cocos2d::__Array::create();
		for (rapidjson::SizeType i = 0; i < json["typeIdPair"].Size(); i++) {
			actionIdPair->addObject(cocos2d::Integer::create(json["typeIdPair"][i][1].GetInt()));
		}
		this->setActionIdPair(actionIdPair);
	}
	//priority
	CC_ASSERT(json.HasMember("priority"));
	this->setPriority(json["priority"].GetInt());
	//color
	CC_ASSERT(json.HasMember("a"));
	this->setA(json["a"].GetInt());
	CC_ASSERT(json.HasMember("r"));
	this->setR(json["r"].GetInt());
	CC_ASSERT(json.HasMember("g"));
	this->setG(json["g"].GetInt());
	CC_ASSERT(json.HasMember("b"));
	this->setB(json["b"].GetInt());
	//changeConditionType
	CC_ASSERT(json.HasMember("changeConditionType"));
	this->setChangeConditionType((EnumChangeConditionType)json["changeConditionType"].GetInt());
	//noInput
	CC_ASSERT(json.HasMember("noInput"));
	this->setNoInput(json["noInput"].GetBool());
	//useInput
	CC_ASSERT(json.HasMember("useInput"));
	this->setUseInput(json["useInput"].GetBool());
	//inputConditionList
	CC_ASSERT(json.HasMember("inputConditionList"));
	auto inputConditionList = cocos2d::__Dictionary::create();
	for (rapidjson::SizeType i = 0; i < json["inputConditionList"].Size(); i++) {
		auto p = ObjectInputConditionData::create(json["inputConditionList"][i]);
#if defined(AGTK_DEBUG)
		CC_ASSERT(inputConditionList->objectForKey(p->getId()) == nullptr);
#endif
		inputConditionList->setObject(p, p->getId());
	}
	this->setInputConditionList(inputConditionList);
	//linkConditionList
	auto linkConditionList = cocos2d::__Dictionary::create();
	for (rapidjson::SizeType i = 0; i < json["linkConditionList"].Size(); i++) {
		auto p = ObjectActionLinkConditionData::create(json["linkConditionList"][i]);
#if defined(AGTK_DEBUG)
		CC_ASSERT(linkConditionList->objectForKey(p->getId()) == nullptr);
#endif
		linkConditionList->setObject(p, p->getId());
	}
	this->setLinkConditionList(linkConditionList);
	//memo
	CC_ASSERT(json.HasMember("memo"));
	this->setMemo(cocos2d::__String::create(json["memo"].GetString()));
	//coordList
	CC_ASSERT(json.HasMember("coordList"));
	auto coordList = cocos2d::__Array::create();
	for (rapidjson::SizeType i = 0; i < json["coordList"].Size(); i++) {
		auto coord = cocos2d::__Array::create();
		for (rapidjson::SizeType j = 0; j < json["coordList"][i].Size(); j++) {
			coord->addObject(cocos2d::Integer::create(json["coordList"][i][j].GetInt()));
		}
		coordList->addObject(coord);
	}
	this->setCoordList(coordList);

	//Additional Data
	if (this->initConditionGroupList() == false) {
		return false;
	}
	return true;
}

bool ObjectActionLinkData::initConditionGroupList()
{
	//inputCondition
	auto inputConditionList = this->getInputConditionList();
	CC_ASSERT(inputConditionList);
	cocos2d::DictElement *el = nullptr;
	auto inputConditionGroupList = cocos2d::__Array::create();
	cocos2d::__Array *arr = nullptr;
	CCDICT_FOREACH(inputConditionList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto inputConditionData = static_cast<ObjectInputConditionData *>(el->getObject());
#else
		auto inputConditionData = dynamic_cast<ObjectInputConditionData *>(el->getObject());
#endif
		if (arr == nullptr) {
			arr = cocos2d::__Array::create();
		}
		arr->addObject(inputConditionData);
		if (!inputConditionData->getCondAnd()) {
			inputConditionGroupList->addObject(arr);
			arr = nullptr;
		}
	}
	if (arr) {
		inputConditionGroupList->addObject(arr);
	}
	this->setInputConditionGroupList(inputConditionGroupList);

	//linkCondition
	auto linkConditionList = this->getLinkConditionList();
	CC_ASSERT(linkConditionList);
	el = nullptr;
	auto linkConditionGroupList = cocos2d::__Array::create();
	arr = nullptr;
	CCDICT_FOREACH(linkConditionList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto linkConditionData = static_cast<ObjectActionLinkConditionData *>(el->getObject());
#else
		auto linkConditionData = dynamic_cast<ObjectActionLinkConditionData *>(el->getObject());
#endif
		if (arr == nullptr) {
			arr = cocos2d::__Array::create();
		}
		arr->addObject(linkConditionData);
		if (!linkConditionData->getCondAnd()) {
			linkConditionGroupList->addObject(arr);
			arr = nullptr;
		}
	}
	if (arr) {
		linkConditionGroupList->addObject(arr);
	}
	this->setLinkConditionGroupList(linkConditionGroupList);
	return true;
}

const char *ObjectActionLinkData::getMemo()
{
	CC_ASSERT(_memo);
	return _memo->getCString();
}

int ObjectActionLinkData::getActionIdPair(int id)
{
	CC_ASSERT(_actionIdPair);
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	auto i = static_cast<cocos2d::Integer *>(this->getActionIdPair()->getObjectAtIndex(id));
#else
	auto i = dynamic_cast<cocos2d::Integer *>(this->getActionIdPair()->getObjectAtIndex(id));
#endif
	return i->getValue();
}

int ObjectActionLinkData::getPrevActionId()
{
	return this->getActionIdPair(0);
}

int ObjectActionLinkData::getNextActionId()
{
	return this->getActionIdPair(1);
}

#if defined(AGTK_DEBUG)
void ObjectActionLinkData::dump()
{
	CCLOG("id:%d", this->getId());
	std::string tmp = "actionIdPair:[";
	cocos2d::Ref *ref = nullptr;
	CCARRAY_FOREACH(this->getActionIdPair(), ref) {
		auto i = dynamic_cast<cocos2d::Integer *>(ref);
		tmp += std::to_string(i->getValue()) + std::string(",");
	}
	tmp += "]";
	CCLOG("%s", tmp.c_str());
	CCLOG("priority:%d", this->getPriority());
	CCLOG("a:%d", this->getA());
	CCLOG("r:%d", this->getR());
	CCLOG("g:%d", this->getG());
	CCLOG("b:%d", this->getB());
	CCLOG("changeConditionType:%d", this->getChangeConditionType());
	CCLOG("noInput:%d", this->getNoInput());
	CCLOG("useInput:%d", this->getUseInput());
	CCLOG("inputConditionList");
	cocos2d::DictElement *el = nullptr;
	auto inputConditionList = this->getInputConditionList();
	CCDICT_FOREACH(inputConditionList, el) {
		auto p = dynamic_cast<ObjectInputConditionData *>(el->getObject());
		p->dump();
	}
	CCLOG("linkConditionList");
	auto linkConditionList = this->getLinkConditionList();
	CCDICT_FOREACH(linkConditionList, el) {
		auto p = dynamic_cast<ObjectActionLinkConditionData *>(el->getObject());
		p->dump();
	}
	CCLOG("memo:%s", this->getMemo());
	tmp = "coordList:[";
	CCARRAY_FOREACH(this->getCoordList(), ref) {
		auto arr = dynamic_cast<cocos2d::__Array *>(ref);
		cocos2d::Ref *ref2 = nullptr;
		tmp += "[";
		CCARRAY_FOREACH(arr, ref2) {
			auto i = dynamic_cast<cocos2d::Integer *>(ref2);
			tmp += std::to_string(i->getValue());
			tmp += ",";
		}
		tmp += "],";
	}
	tmp += "]";
	CCLOG("%s", tmp.c_str());
}
#endif

//-------------------------------------------------------------------------------------------------------------------
ObjectActionGroupData::ObjectActionGroupData()
{
	_name = nullptr;
	_actionIdList = nullptr;
}

ObjectActionGroupData::~ObjectActionGroupData()
{
	CC_SAFE_RELEASE_NULL(_name);
	CC_SAFE_RELEASE_NULL(_actionIdList);
}

bool ObjectActionGroupData::init(const rapidjson::Value& json)
{
	CC_ASSERT(json.HasMember("id"));
	this->setId(json["id"].GetInt());
	CC_ASSERT(json.HasMember("name"));
	this->setName(cocos2d::__String::create(json["name"].GetString()));
	CC_ASSERT(json.HasMember("actionIdList"));
	auto actionIdList = cocos2d::__Array::create();
	for (rapidjson::SizeType i = 0; i < json["actionIdList"].Size(); i++) {
		auto p = cocos2d::Integer::create(json["actionIdList"][i].GetInt());
		actionIdList->addObject(p);
	}
	this->setActionIdList(actionIdList);
	CC_ASSERT(json.HasMember("x"));
	this->setX(json["x"].GetDouble());
	CC_ASSERT(json.HasMember("y"));
	this->setY(json["y"].GetDouble());
	CC_ASSERT(json.HasMember("width"));
	this->setWidth(json["width"].GetDouble());
	CC_ASSERT(json.HasMember("height"));
	this->setHeight(json["height"].GetDouble());
	return true;
}

const char *ObjectActionGroupData::getName()
{
	CC_ASSERT(_name);
	return _name->getCString();
}

#if defined(AGTK_DEBUG)
void ObjectActionGroupData::dump()
{
	CCLOG("id:%d", this->getId());
	CCLOG("name:%s", this->getName());
	std::string tmp;
	cocos2d::Ref *ref = nullptr;
	tmp = "actionIdList:[";
	CCARRAY_FOREACH(this->getActionIdList(), ref) {
		auto i = dynamic_cast<cocos2d::Integer *>(ref);
		tmp += std::to_string(i->getValue());
		tmp += ",";
	}
	tmp += "]";
	CCLOG("%s", tmp.c_str());
	CCLOG("x:%d", this->getX());
	CCLOG("y:%d", this->getY());
	CCLOG("width:%d", this->getWidth());
	CCLOG("height:%d", this->getHeight());
}
#endif

//-------------------------------------------------------------------------------------------------------------------
ObjectDisappearSettingData::ObjectDisappearSettingData()
{
	_otherObjectDecrementHp = false;
	_decrement = 0;
	_targetObjectType = kTargetObjectMax;
	_targetObjectGroup = 0;
	_targetObjectId = 0;
	_damageRange = kRangeMax;
	_cameraRangeAdjust = 0;
	_objCommandList = nullptr;
}

ObjectDisappearSettingData::~ObjectDisappearSettingData()
{
	CC_SAFE_RELEASE_NULL(_objCommandList);
}

bool ObjectDisappearSettingData::init(const rapidjson::Value& json)
{
	if (!json.HasMember("otherObjectDecrementHp")) {
		CC_ASSERT(0);
		return false;
	}
	this->setOtherObjectDecrementHp(json["otherObjectDecrementHp"].GetBool());
	if (!json.HasMember("decrement")) {
		CC_ASSERT(0);
		return false;
	}
	this->setDecrement(json["decrement"].GetInt());
	if (!json.HasMember("targetObjectType")) {
		CC_ASSERT(0);
		return false;
	}
	this->setTargetObjectType((EnumTargetObjectType)json["targetObjectType"].GetInt());
	if (!json.HasMember("targetObjectGroup")) {
		CC_ASSERT(0);
		return false;
	}
	this->setTargetObjectGroup(json["targetObjectGroup"].GetInt());
	if (!json.HasMember("targetObjectId")) {
		CC_ASSERT(0);
		return false;
	}
	this->setTargetObjectId(json["targetObjectId"].GetInt());
	if (!json.HasMember("damageRange")) {
		CC_ASSERT(0);
		return false;
	}
	this->setDamageRange((EnumDamageRange)json["damageRange"].GetInt());
	if (!json.HasMember("cameraRangeAdjust")) {
		CC_ASSERT(0);
		return false;
	}
	this->setCameraRangeAdjust(json["cameraRangeAdjust"].GetInt());

	auto objCommandList = cocos2d::__Dictionary::create();
	for (rapidjson::SizeType i = 0; i < json["objCommandList"].Size(); i++) {
		auto data = ObjectCommandData::create(json["objCommandList"][i]);
#if defined(AGTK_DEBUG)
		CC_ASSERT(objCommandList->objectForKey(data->getId()) == nullptr);
#endif
		objCommandList->setObject(data, data->getId());
	}
	this->setObjCommandList(objCommandList);

	return true;
}

#if defined(AGTK_DEBUG)
void ObjectDisappearSettingData::dump()
{
	CCLOG("otherObjectDecrementHp:%d", this->getOtherObjectDecrementHp());
	CCLOG("decrement:%d", this->getDecrement());
	CCLOG("targetObjectType:%d",this->getTargetObjectType());
	CCLOG("targetObjectGroup:%d", this->getTargetObjectGroup());
	CCLOG("targetObjectId:%d", this->getTargetObjectId());
	CCLOG("damageRange:%d", this->getDamageRange());
	CCLOG("cameraRangeAdjust:%d",this->getCameraRangeAdjust());
	cocos2d::DictElement *el = nullptr;
	auto actionList = this->getObjCommandList();
	CCDICT_FOREACH(actionList, el) {
		auto p = dynamic_cast<ObjectCommandData *>(el->getObject());
		p->dump();
	}
};
#endif

//-------------------------------------------------------------------------------------------------------------------
ObjectFireBulletSettingData::ObjectFireBulletSettingData()
{
	_name = nullptr;
	_initialBulletLocus = kInitialBulletLocusFree;
	_nextBulletLocus = kNextBulletLocusFree;
	_towardObjectGroupBit = 0;
	_boomerangTurnDuration300 = 0;
	_boomerangDecelBeforeTurn = false;
	_boomerangTurnWhenTouchingWall = false;
	_followLockedObjectStartDelayStart300 = 0;
	_followLockedObjectStartDelayEnd300 = 0;
	_followObjectInsideCameraStartDelayStart300 = 0;
	_followObjectInsideCameraStartDelayEnd300 = 0;
	_followObjectInsideCameraTargetObjectGroupBit = 0;
}

ObjectFireBulletSettingData::~ObjectFireBulletSettingData()
{
	CC_SAFE_RELEASE_NULL(_name);
}

bool ObjectFireBulletSettingData::init(const rapidjson::Value& json)
{
	CC_ASSERT(json.HasMember("id"));
	this->setId(json["id"].GetInt());
	CC_ASSERT(json.HasMember("name"));
	this->setName(cocos2d::__String::create(json["name"].GetString()));
	CC_ASSERT(json.HasMember("bulletChild"));
	this->setBulletChild(json["bulletChild"].GetBool());
	CC_ASSERT(json.HasMember("bulletObjectId"));
	this->setBulletObjectId(json["bulletObjectId"].GetInt());
	CC_ASSERT(json.HasMember("fireBulletCount"));
	this->setFireBulletCount(json["fireBulletCount"].GetInt());
	CC_ASSERT(json.HasMember("bulletInterval300"));
	this->setBulletInterval300(json["bulletInterval300"].GetInt());
	CC_ASSERT(json.HasMember("dispBulletCount"));
	this->setDispBulletCount(json["dispBulletCount"].GetInt());
	CC_ASSERT(json.HasMember("dispBulletUnlimited"));
	this->setDispBulletUnlimited(json["dispBulletUnlimited"].GetBool());
	if (json.HasMember("initialBulletLocus")) {
		this->setInitialBulletLocus((EnumInitialBulletLocus)json["initialBulletLocus"].GetInt());
	}
	if (json.HasMember("nextBulletLocus")) {
		this->setNextBulletLocus((EnumNextBulletLocus)json["nextBulletLocus"].GetInt());
	}
	CC_ASSERT(json.HasMember("setActionDirectionToFireObjectDirection"));
	this->setSetActionDirectionToFireObjectDirection(json["setActionDirectionToFireObjectDirection"].GetBool());
	this->setTowardObjectGroupBit(json["towardObjectGroupBit"].GetInt());
	CC_ASSERT(json.HasMember("oneDirectionAngle"));
	this->setOneDirectionAngle(json["oneDirectionAngle"].GetInt());
	CC_ASSERT(json.HasMember("oneDirectionSpreadRange"));
	this->setOneDirectionSpreadRange(json["oneDirectionSpreadRange"].GetInt());
	CC_ASSERT(json.HasMember("oneDirectionSpreadType"));
	this->setOneDirectionSpreadType((EnumSpreadType)json["oneDirectionSpreadType"].GetInt());
	CC_ASSERT(json.HasMember("followLockedObjectPerformance"));
	this->setFollowLockedObjectPerformance(json["followLockedObjectPerformance"].GetInt());
	if (json.HasMember("followLockedObjectStartDelayStart300")) {
		this->setFollowLockedObjectStartDelayStart300(json["followLockedObjectStartDelayStart300"].GetInt());
	}
	if (json.HasMember("followLockedObjectStartDelayEnd300")) {
		this->setFollowLockedObjectStartDelayEnd300(json["followLockedObjectStartDelayEnd300"].GetInt());
	}
	CC_ASSERT(json.HasMember("followObjectInsideCameraPerformance"));
	this->setFollowObjectInsideCameraPerformance(json["followObjectInsideCameraPerformance"].GetInt());
	if (json.HasMember("followObjectInsideCameraStartDelayStart300")) {
		this->setFollowObjectInsideCameraStartDelayStart300(json["followObjectInsideCameraStartDelayStart300"].GetInt());
	}
	if (json.HasMember("followObjectInsideCameraStartDelayEnd300")) {
		this->setFollowObjectInsideCameraStartDelayEnd300(json["followObjectInsideCameraStartDelayEnd300"].GetInt());
	}
	CC_ASSERT(json.HasMember("followObjectInsideCameraTargetObjectGroupBit"));
	this->setFollowObjectInsideCameraTargetObjectGroupBit(json["followObjectInsideCameraTargetObjectGroupBit"].GetInt());
	
	if (json.HasMember("boomerangTurnDuration300")) {
		this->setBoomerangTurnDuration300(json["boomerangTurnDuration300"].GetInt());
	}
	if (json.HasMember("boomerangDecelBeforeTurn")) {
		this->setBoomerangDecelBeforeTurn(json["boomerangDecelBeforeTurn"].GetBool());
	}
	CC_ASSERT(json.HasMember("boomerangComebackPerformance"));
	this->setBoomerangComebackPerformance(json["boomerangComebackPerformance"].GetInt());
	if (json.HasMember("boomerangTurnWhenTouchingWall")) {
		this->setBoomerangTurnWhenTouchingWall(json["boomerangTurnWhenTouchingWall"].GetBool());
	}
	CC_ASSERT(json.HasMember("freeBulletGravityFlag"));
	this->setFreeBulletGravityFlag(json["freeBulletGravityFlag"].GetBool());
	CC_ASSERT(json.HasMember("freeBulletGravity"));
	this->setFreeBulletGravity(json["freeBulletGravity"].GetDouble());
	CC_ASSERT(json.HasMember("freeBulletMoveSpeedFlag"));
	this->setFreeBulletMoveSpeedFlag(json["freeBulletMoveSpeedFlag"].GetBool());
	CC_ASSERT(json.HasMember("freeBulletMoveSpeed"));
	this->setFreeBulletMoveSpeed(json["freeBulletMoveSpeed"].GetDouble());
	return true;
}

const char *ObjectFireBulletSettingData::getName()
{
	CC_ASSERT(_name);
	return _name->getCString();
}

#if defined(AGTK_DEBUG)
void ObjectFireBulletSettingData::dump()
{
	CCLOG("id:%d", this->getId());
	CCLOG("name:%s", this->getName());
	CCLOG("bulletChild:%s", this->getBulletChild() ? "True" : "False");
	CCLOG("bulletObjectId:%d", this->getBulletObjectId());
	CCLOG("fireBulletCount:%d", this->getFireBulletCount());
	CCLOG("bulletInterval300:%d", this->getBulletInterval300());
	CCLOG("dispBulletCount:%d", this->getDispBulletCount());
	CCLOG("dispBulletUnlimited:%d", this->getDispBulletUnlimited());
	CCLOG("initialBulletLocus:%d", this->getInitialBulletLocus());
	CCLOG("nextBulletLocus:%d", this->getNextBulletLocus());
	CCLOG("setActionDirectionToFireObjectDirection:%d", this->getSetActionDirectionToFireObjectDirection());
	CCLOG("towardObjectGroupBit:%d", this->getTowardObjectGroupBit());
	CCLOG("oneDirectionAngle:%d", this->getOneDirectionAngle());
	CCLOG("oneDirectionSpreadRange:%d", this->getOneDirectionSpreadRange());
	CCLOG("oneDirectionSpreadType:%d", this->getOneDirectionSpreadType());
	CCLOG("followLockedObjectPerformance:%d", this->getFollowLockedObjectPerformance());
	CCLOG("followLockedObjectStartDelayStart300:%d", this->getFollowLockedObjectStartDelayStart300());
	CCLOG("followLockedObjectStartDelayEnd300:%d", this->getFollowLockedObjectStartDelayEnd300());
	CCLOG("followObjectInsideCameraPerformance:%d", this->getFollowObjectInsideCameraPerformance());
	CCLOG("followObjectInsideCameraStartDelayStart300:%f", this->getFollowObjectInsideCameraStartDelayStart300());
	CCLOG("followObjectInsideCameraStartDelayEnd300:%f", this->getFollowObjectInsideCameraStartDelayEnd300());
	CCLOG("followObjectInsideCameraTargetGroupBit:%d", this->getFollowObjectInsideCameraTargetObjectGroupBit());
	CCLOG("boomerangTurnDuration300:%d", this->getBoomerangTurnDuration300());
	CCLOG("boomerangDecelBeforeTurn:%d", this->getBoomerangDecelBeforeTurn());
	CCLOG("boomerangComebackPerformance:%d", this->getBoomerangComebackPerformance());
	CCLOG("boomerangTurnWhenTouchingWall:%d", this->getBoomerangTurnWhenTouchingWall());
	CCLOG("freeBulletGravityFlag:%d", this->getFreeBulletGravityFlag());
	CCLOG("freeBulletGravity:%f", this->getFreeBulletGravity());
	CCLOG("freeBulletMoveSpeedFlag:%d", this->getFreeBulletMoveSpeedFlag());
	CCLOG("freeBulletMoveSpeed:%f", this->getFreeBulletMoveSpeed());
}
#endif

//-------------------------------------------------------------------------------------------------------------------
ObjectEffectSettingData::ObjectEffectSettingData()
{
	_name = nullptr;
}

ObjectEffectSettingData::~ObjectEffectSettingData()
{
	CC_SAFE_RELEASE_NULL(_name);
}

bool ObjectEffectSettingData::init(const rapidjson::Value& json)
{
	CC_ASSERT(json.HasMember("id"));
	this->setId(json["id"].GetInt());
	CC_ASSERT(json.HasMember("name"));
	this->setName(cocos2d::__String::create(json["name"].GetString()));
	CC_ASSERT(json.HasMember("animationType"));
	this->setAnimationType((EnumAnimationType)json["animationType"].GetInt());
	CC_ASSERT(json.HasMember("effectId"));
	this->setEffectId(json["effectId"].GetInt());
	CC_ASSERT(json.HasMember("particleId"));
	this->setParticleId(json["particleId"].GetInt());
	CC_ASSERT(json.HasMember("objectSwitch"));
	this->setObjectSwitch(json["objectSwitch"].GetBool());
	CC_ASSERT(json.HasMember("objectSwitchId"));
	this->setObjectSwitchId(json["objectSwitchId"].GetInt());
	CC_ASSERT(json.HasMember("systemSwitchId"));
	this->setSystemSwitchId(json["systemSwitchId"].GetInt());
	CC_ASSERT(json.HasMember("positionType"));
	this->setPositionType((EnumPositionType)json["positionType"].GetInt());
	CC_ASSERT(json.HasMember("connectionId"));
	this->setConnectionId(json["connectionId"].GetInt());
	CC_ASSERT(json.HasMember("adjustX"));
	this->setAdjustX(json["adjustX"].GetInt());
	CC_ASSERT(json.HasMember("adjustY"));
	this->setAdjustY(json["adjustY"].GetInt());
	CC_ASSERT(json.HasMember("dispDuration300"));
	this->setDispDuration300(json["dispDuration300"].GetInt());
	CC_ASSERT(json.HasMember("dispDurationUnlimited"));
	this->setDispDurationUnlimited(json["dispDurationUnlimited"].GetBool());
	return true;
}

const char *ObjectEffectSettingData::getName()
{
	CC_ASSERT(_name);
	return _name->getCString();
}

#if defined(AGTK_DEBUG)
void ObjectEffectSettingData::dump()
{
	CCLOG("id:%d", this->getId());
	CCLOG("name:%s", this->getName());
	CCLOG("effectId:%d", this->getEffectId());
	CCLOG("objectSwitch:%d", this->getObjectSwitch());
	CCLOG("objectSwitchId:%d", this->getObjectSwitchId());
	CCLOG("systemSwitchId:%d", this->getSystemSwitchId());
	CCLOG("positionType:%d", this->getPositionType());
	CCLOG("connectionId:%d", this->getConnectionId());
	CCLOG("adjustX:%d", this->getAdjustX());
	CCLOG("adjustY:%d", this->getAdjustY());
	CCLOG("dispDuration300:%d", this->getDispDuration300());
	CCLOG("dispDurationUnlimited:%d", this->getDispDurationUnlimited());
}
#endif

//-------------------------------------------------------------------------------------------------------------------
ObjectConnectSettingData::ObjectConnectSettingData()
{
	_name = nullptr;
}

ObjectConnectSettingData::~ObjectConnectSettingData()
{
	CC_SAFE_RELEASE_NULL(_name);
}

bool ObjectConnectSettingData::init(const rapidjson::Value& json)
{
	CC_ASSERT(json.HasMember("id"));
	this->setId(json["id"].GetInt());
	CC_ASSERT(json.HasMember("name"));
	this->setName(cocos2d::__String::create(json["name"].GetString()));
	CC_ASSERT(json.HasMember("objectId"));
	this->setObjectId(json["objectId"].GetInt());
	CC_ASSERT(json.HasMember("objectSwitch"));
	this->setObjectSwitch(json["objectSwitch"].GetBool());
	CC_ASSERT(json.HasMember("objectSwitchId"));
	this->setObjectSwitchId(json["objectSwitchId"].GetInt());
	CC_ASSERT(json.HasMember("systemSwitchId"));
	this->setSystemSwitchId(json["systemSwitchId"].GetInt());
	CC_ASSERT(json.HasMember("positionType"));
	this->setPositionType((EnumPositionType)json["positionType"].GetInt());
	CC_ASSERT(json.HasMember("connectionId"));
	this->setConnectionId(json["connectionId"].GetInt());
	CC_ASSERT(json.HasMember("adjustX"));
	this->setAdjustX(json["adjustX"].GetInt());
	CC_ASSERT(json.HasMember("adjustY"));
	this->setAdjustY(json["adjustY"].GetInt());
	CC_ASSERT(json.HasMember("childObject"));
	this->setChildObject(json["childObject"].GetBool());
	CC_ASSERT(json.HasMember("setDirectionToConnectObjectDirection"));
	this->setSetDirectionToConnectObjectDirection(json["setDirectionToConnectObjectDirection"].GetBool());
	CC_ASSERT(json.HasMember("lowerPriority"));
	this->setLowerPriority(json["lowerPriority"].GetBool());
	return true;
}

const char *ObjectConnectSettingData::getName()
{
	CC_ASSERT(_name);
	return _name->getCString();
}

#if defined(AGTK_DEBUG)
void ObjectConnectSettingData::dump()
{
	CCLOG("id:%d", this->getId());
	CCLOG("name:%s", this->getName());
	CCLOG("objectId:%d", this->getObjectId());
	CCLOG("objectSwitch:%d", this->getObjectSwitch());
	CCLOG("objectSwitchId:%d", this->getObjectSwitchId());
	CCLOG("systemSwitchId:%d", this->getSystemSwitchId());
	CCLOG("positionType:%d", this->getPositionType());
	CCLOG("connectionId:%d", this->getConnectionId());
	CCLOG("adjustX:%d", this->getAdjustX());
	CCLOG("adjustY:%d", this->getAdjustY());
	CCLOG("childObject:%d", this->getChildObject());
	CCLOG("setDirectionToConnectObjectDirection:%d", this->getSetDirectionToConnectObjectDirection());
	CCLOG("lowerPriority:%d", this->getLowerPriority());
}
#endif

//-------------------------------------------------------------------------------------------------------------------
ObjectAdditionalDisplayData::ObjectAdditionalDisplayData()
{
}

ObjectAdditionalDisplayData::~ObjectAdditionalDisplayData()
{
}

bool ObjectAdditionalDisplayData::init(const rapidjson::Value& json)
{
	this->setId(json["id"].GetInt());
	this->setShowText(json["showText"].GetBool());
	this->setTextId(json["textId"].GetInt());
	this->setTextColorR(json["textColorR"].GetInt());
	this->setTextColorG(json["textColorG"].GetInt());
	this->setTextColorB(json["textColorB"].GetInt());
	this->setTextColorA(json["textColorA"].GetInt());
	this->setParamDisplayType((EnumParamDisplayType)json["paramDisplayType"].GetInt());
	this->setVariableObjectId(json["variableObjectId"].GetInt());
	this->setVariableId(json["variableId"].GetInt());
	this->setUseParentVariable(json["useParentVariable"].GetBool());
	this->setVariableMaxEnabled(json["variableMaxEnabled"].GetBool());
	this->setVariableMaxObjectId(json["variableMaxObjectId"].GetInt());
	this->setVariableMaxVariableId(json["variableMaxVariableId"].GetInt());
	this->setVariableMaxAutoScalingEnabled(json["variableMaxAutoScalingEnabled"].GetBool());
	this->setVariableMaxUseParent(json["variableMaxUseParent"].GetBool());
	this->setParamTextColorR(json["paramTextColorR"].GetInt());
	this->setParamTextColorG(json["paramTextColorG"].GetInt());
	this->setParamTextColorB(json["paramTextColorB"].GetInt());
	this->setParamTextColorA(json["paramTextColorA"].GetInt());
	this->setParamTextFontId(json["paramTextFontId"].GetInt());
	this->setParamGaugeColorR(json["paramGaugeColorR"].GetInt());
	this->setParamGaugeColorG(json["paramGaugeColorG"].GetInt());
	this->setParamGaugeColorB(json["paramGaugeColorB"].GetInt());
	this->setParamGaugeColorA(json["paramGaugeColorA"].GetInt());
	this->setParamGaugeBgColorR(json["paramGaugeBgColorR"].GetInt());
	this->setParamGaugeBgColorG(json["paramGaugeBgColorG"].GetInt());
	this->setParamGaugeBgColorB(json["paramGaugeBgColorB"].GetInt());
	this->setParamGaugeBgColorA(json["paramGaugeBgColorA"].GetInt());
	this->setAdjustX(json["adjustX"].GetDouble());
	this->setAdjustY(json["adjustY"].GetDouble());
	this->setScaleX(json["scaleX"].GetDouble());
	this->setScaleY(json["scaleY"].GetDouble());
	this->setRotation(json["rotation"].GetDouble());
	this->setHide(json["hide"].GetBool());
	return true;
}

#if defined(AGTK_DEBUG)
void ObjectAdditionalDisplayData::dump()
{
	CCLOG("id:%d", this->getId());
	CCLOG("showText:%d", this->getShowText());
	CCLOG("textId:%d", this->getTextId());
	CCLOG("textColorR:%d", this->getTextColorR());
	CCLOG("textColorG:%d", this->getTextColorG());
	CCLOG("textColorB:%d", this->getTextColorB());
	CCLOG("textColorA:%d", this->getTextColorA());
	CCLOG("paramDisplayType:%d", this->getParamDisplayType());
	CCLOG("variableObjectId:%d", this->getVariableObjectId());
	CCLOG("variableId:%d", this->getVariableId());
	CCLOG("useParentVariable:%d", this->getUseParentVariable());
	CCLOG("variableMaxEnabled:%d", this->getVariableMaxEnabled());
	CCLOG("variableMaxObjectId:%d", this->getVariableMaxObjectId());
	CCLOG("variableMaxVariableId:%d", this->getVariableMaxVariableId());
	CCLOG("variableMaxAutoScalingEnabled:%d", this->getVariableMaxAutoScalingEnabled());
	CCLOG("variableMaxUseParent:%d", this->getVariableMaxUseParent());
	CCLOG("paramTextColorR:%d", this->getParamTextColorR());
	CCLOG("paramTextColorG:%d", this->getParamTextColorG());
	CCLOG("paramTextColorB:%d", this->getParamTextColorB());
	CCLOG("paramTextColorA:%d", this->getParamTextColorA());
	CCLOG("paramTextFontId:%d", this->getParamTextFontId());
	CCLOG("paramGaugeColorR:%d", this->getParamGaugeColorR());
	CCLOG("paramGaugeColorG:%d", this->getParamGaugeColorG());
	CCLOG("paramGaugeColorB:%d", this->getParamGaugeColorB());
	CCLOG("paramGaugeColorA:%d", this->getParamGaugeColorA());
	CCLOG("paramGaugeBgColorR:%d", this->getParamGaugeBgColorR());
	CCLOG("paramGaugeBgColorG:%d", this->getParamGaugeBgColorG());
	CCLOG("paramGaugeBgColorB:%d", this->getParamGaugeBgColorB());
	CCLOG("paramGaugeBgColorA:%d", this->getParamGaugeBgColorA());
	CCLOG("adjustX:%f", this->getAdjustX());
	CCLOG("adjustY:%f", this->getAdjustY());
	CCLOG("scaleX:%f", this->getScaleX());
	CCLOG("scaleY:%f", this->getScaleY());
	CCLOG("rotation:%f", this->getRotation());
	CCLOG("hide:%d", this->getHide());
}
#endif

//-------------------------------------------------------------------------------------------------------------------
ObjectMoveDispDirectionSettingData::ObjectMoveDispDirectionSettingData()
{
	_actionInfoList = nullptr;
}

ObjectMoveDispDirectionSettingData::~ObjectMoveDispDirectionSettingData()
{
	CC_SAFE_RELEASE_NULL(_actionInfoList);
}

bool ObjectMoveDispDirectionSettingData::init(const rapidjson::Value& json)
{
	// 2017/10/19 agusa-k: operationKeyIdに変更。
	if (json.HasMember("upOperationKeyId")){
		this->setUpOperationKeyId(json["upOperationKeyId"].GetInt());
		this->setDownOperationKeyId(json["downOperationKeyId"].GetInt());
		this->setLeftOperationKeyId(json["leftOperationKeyId"].GetInt());
		this->setRightOperationKeyId(json["rightOperationKeyId"].GetInt());
	}
	CC_ASSERT(json.HasMember("actionInfoList"));
	auto dic = cocos2d::__Dictionary::create();
	if (json.HasMember("actionInfoList")) {
		for (rapidjson::SizeType i = 0; i < json["actionInfoList"].Size(); i++) {
			auto p = ObjectActionInfoList::create(json["actionInfoList"][i]);
			dic->setObject(p, p->getId());
		}
	}
	this->setActionInfoList(dic);

	return true;
}

/**
 * 指定のアクションIDが「対応するアクションボックス」リストに存在するかチェック
 * @param	actionId	チェック対象のアクションID
 * @return				存在の有無
 */
bool ObjectMoveDispDirectionSettingData::checkExistsActionInfo(int actionId)
{
	cocos2d::DictElement *el = nullptr;
	CCDICT_FOREACH(_actionInfoList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto data = static_cast<agtk::data::ObjectActionInfoList *>(el->getObject());
#else
		auto data = dynamic_cast<agtk::data::ObjectActionInfoList *>(el->getObject());
#endif
		if (data->getActionId() == actionId && !data->getDisabled()) {
			return true;
		}
	}

	return false;
}

#if defined(AGTK_DEBUG)
void ObjectMoveDispDirectionSettingData::dump()
{
	CCLOG("upOperationKeyId:%d", this->getUpOperationKeyId());
	CCLOG("downOperationKeyId:%d", this->getDownOperationKeyId());
	CCLOG("leftOperationKeyId:%d", this->getLeftOperationKeyId());
	CCLOG("rightOperationKeyId:%d", this->getRightOperationKeyId());
	CCLOG("actionInfoList:[");

	if (this->getActionInfoList()->count() > 0) {
		auto keys = this->getActionInfoList()->allKeys();
		cocos2d::Ref *ref = nullptr;
		CCARRAY_FOREACH(keys, ref) {
			auto id = dynamic_cast<cocos2d::Integer *>(ref);
			auto data = dynamic_cast<ObjectActionInfoList *>(this->getActionInfoList()->objectForKey(id->getValue()));
			data->dump();
		}
	}
	CCLOG("]");
}
#endif
//-------------------------------------------------------------------------------------------------------------------
/**
 * コンストラクタ
 */
ObjectActionInfoList::ObjectActionInfoList()
{
	_id = 0;
	_actionId = 0;
	_disabled = false;
}
/**
 * デストラクタ
 */
ObjectActionInfoList::~ObjectActionInfoList()
{

}

/**
 * 初期化
 * @param	json	初期化用JSONデータ
 * @return			初期化の成否
 */
bool ObjectActionInfoList::init(const rapidjson::Value& json)
{
	CC_ASSERT(json.HasMember("id"));
	this->setId(json["id"].GetInt());
	CC_ASSERT(json.HasMember("actionId"));
	this->setActionId(json["actionId"].GetInt());
	CC_ASSERT(json.HasMember("disabled"));
	this->setDisabled(json["disabled"].GetBool());

	return true;
}
#if defined(AGTK_DEBUG)
/**
 * ダンプ
 */
void ObjectActionInfoList::dump()
{
	CCLOG("Id:%d", this->getId());
	CCLOG("actionId:%d", this->getActionId());
	CCLOG("disabled:%d", this->getDisabled());
}
#endif
//-------------------------------------------------------------------------------------------------------------------
ObjectPhysicsSettingData::ObjectPhysicsSettingData()
{
	_physicsAffected = false;
	_affectPhysics = false;
	_templateId = 0;
	_density = 0;
	_mass = 0;
	_friction = 0;
	_repulsion = 0;
	_nonCollisionGroup = nullptr;
}

ObjectPhysicsSettingData::~ObjectPhysicsSettingData()
{
	CC_SAFE_RELEASE_NULL(_nonCollisionGroup);
}

bool ObjectPhysicsSettingData::init(const rapidjson::Value& json)
{
	CC_ASSERT(json.HasMember("physicsAffected"));
	this->setPhysicsAffected(json["physicsAffected"].GetBool());
	CC_ASSERT(json.HasMember("affectPhysics"));
	this->setAffectPhysics(json["affectPhysics"].GetBool());
	CC_ASSERT(json.HasMember("followConnectedPhysics"));
	this->setFollowConnectedPhysics(json["followConnectedPhysics"].GetBool());
	CC_ASSERT(json.HasMember("templateId"));
	this->setTemplateId(json["templateId"].GetInt());
	CC_ASSERT(json.HasMember("density"));
	this->setDensity(json["density"].GetDouble());
	CC_ASSERT(json.HasMember("mass"));
	this->setMass(json["mass"].GetDouble());
	CC_ASSERT(json.HasMember("friction"));
	this->setFriction(json["friction"].GetDouble());
	CC_ASSERT(json.HasMember("repulsion"));
	this->setRepulsion(json["repulsion"].GetDouble());
	CC_ASSERT(json.HasMember("nonCollisionGroup"));
	auto nonCollisionGroup = cocos2d::__Array::create();
	for (rapidjson::SizeType i = 0; i < json["nonCollisionGroup"].Size(); i++) {
		auto p = cocos2d::Integer::create(json["nonCollisionGroup"][i].GetInt());
		nonCollisionGroup->addObject(p);
	}
	this->setNonCollisionGroup(nonCollisionGroup);
	return true;
}

#if defined(AGTK_DEBUG)
void ObjectPhysicsSettingData::dump()
{
	CCLOG("physicsAffected:%d", this->getPhysicsAffected());
	CCLOG("affectPhysics:%d", this->getAffectPhysics());
	CCLOG("followConnectedPhysics:%d", this->getFollowConnectedPhysics());
	CCLOG("templateId:%d", this->getTemplateId());
	CCLOG("density:%d", this->getDensity());
	CCLOG("mass:%d", this->getMass());
	CCLOG("friction:%d", this->getFriction());
	CCLOG("repulsion:%d", this->getRepulsion());
	std::string tmp = "nonCollisionGroup:[";
	cocos2d::Ref *ref = nullptr;
	CCARRAY_FOREACH(this->getNonCollisionGroup(), ref) {
		auto p = dynamic_cast<cocos2d::Integer *>(ref);
		tmp += std::to_string(p->getValue());
		tmp += ",";
	}
	tmp += "]";
	CCLOG("%s", tmp.c_str());
}
#endif

//-------------------------------------------------------------------------------------------------------------------
ObjectViewportLightSettingData::ObjectViewportLightSettingData()
{
	_name = nullptr;
}

ObjectViewportLightSettingData::~ObjectViewportLightSettingData()
{
	CC_SAFE_RELEASE_NULL(_name);
}

bool ObjectViewportLightSettingData::init(const rapidjson::Value& json)
{
	CC_ASSERT(json.HasMember("id"));
	this->setId(json["id"].GetInt());
	CC_ASSERT(json.HasMember("name"));
	this->setName(cocos2d::__String::create(json["name"].GetString()));
	CC_ASSERT(json.HasMember("viewport"));
	this->setViewport(json["viewport"].GetBool());
	CC_ASSERT(json.HasMember("objectSwitch"));
	this->setObjectSwitch(json["objectSwitch"].GetBool());
	CC_ASSERT(json.HasMember("objectSwitchId"));
	this->setObjectSwitchId(json["objectSwitchId"].GetInt());
	CC_ASSERT(json.HasMember("systemSwitchId"));
	this->setSystemSwitchId(json["systemSwitchId"].GetInt());
	CC_ASSERT(json.HasMember("angleRange"));
	this->setAngleRange(json["angleRange"].GetInt());
	CC_ASSERT(json.HasMember("radius"));
	this->setRadius(json["radius"].GetInt());
	CC_ASSERT(json.HasMember("scaleX"));
	this->setScaleX(json["scaleX"].GetInt());
	CC_ASSERT(json.HasMember("scaleY"));
	this->setScaleY(json["scaleY"].GetInt());
	CC_ASSERT(json.HasMember("rotation"));
	this->setRotation(json["rotation"].GetDouble());
	CC_ASSERT(json.HasMember("coloring"));
	this->setColoring(json["coloring"].GetBool());
	CC_ASSERT(json.HasMember("a"));
	this->setA(json["a"].GetInt());
	CC_ASSERT(json.HasMember("r"));
	this->setR(json["r"].GetInt());
	CC_ASSERT(json.HasMember("g"));
	this->setG(json["g"].GetInt());
	CC_ASSERT(json.HasMember("b"));
	this->setB(json["b"].GetInt());
	this->setIntensityFlag(false);
	this->setIntensityOffset(0);
	CC_ASSERT(json.HasMember("defocusCircumferenceFlag"));
	this->setDefocusCircumferenceFlag(json["defocusCircumferenceFlag"].GetBool());
	CC_ASSERT(json.HasMember("defocusCircumference"));
	this->setDefocusCircumference(json["defocusCircumference"].GetInt());
	this->setDefocusSideFlag(false);
	this->setDefocusSide(0);
	CC_ASSERT(json.HasMember("positionType"));
	this->setPositionType((EnumPositionType)json["positionType"].GetInt());
	CC_ASSERT(json.HasMember("connectionId"));
	this->setConnectionId(json["connectionId"].GetInt());
	CC_ASSERT(json.HasMember("adjustX"));
	this->setAdjustX(json["adjustX"].GetInt());
	CC_ASSERT(json.HasMember("adjustY"));
	this->setAdjustY(json["adjustY"].GetInt());
	return true;
}

const char *ObjectViewportLightSettingData::getName()
{
	CC_ASSERT(_name);
	return _name->getCString();
}

#if defined(AGTK_DEBUG)
void ObjectViewportLightSettingData::dump()
{
	CCLOG("id:%d", this->getId());
	CCLOG("name:%s", this->getName());
	CCLOG("viewport:%d", this->getViewport());
	CCLOG("objectSwitch:%d", this->getObjectSwitch());
	CCLOG("objectSwitchId:%d", this->getObjectSwitchId());
	CCLOG("systemSwitchId:%d", this->getSystemSwitchId());
	CCLOG("angleRange:%d", this->getAngleRange());
	CCLOG("radius:%d", this->getRadius());
	CCLOG("scaleX:%d", this->getScaleX());
	CCLOG("scaleY:%d", this->getScaleY());
	CCLOG("rotation:%f", this->getRotation());
	CCLOG("coloring:%d", this->getColoring());
	CCLOG("a:%d", this->getA());
	CCLOG("r:%d", this->getR());
	CCLOG("g:%d", this->getG());
	CCLOG("b:%d", this->getB());
	CCLOG("intensityFlag:%d", this->getIntensityFlag());
	CCLOG("intensityOffset:%d", this->getIntensityOffset());
	CCLOG("defocusCircumferenceFlag:%d", this->getDefocusCircumferenceFlag());
	CCLOG("defocusCircumference:%d", this->getDefocusCircumference());
	CCLOG("defocusSideFlag:%d", this->getDefocusSideFlag());
	CCLOG("defocusSide:%d", this->getDefocusSide());
	CCLOG("positionType:%d", this->getPositionType());
	CCLOG("connectionId:%d", this->getConnectionId());
	CCLOG("adjustX:%d", this->getAdjustX());
	CCLOG("adjustY:%d", this->getAdjustY());
}
#endif

//-------------------------------------------------------------------------------------------------------------------
ObjectCommonActionSettingData::ObjectCommonActionSettingData()
{
	_objActionLink = nullptr;
	_objAction = nullptr;
	_objActionLink2 = nullptr;
}

ObjectCommonActionSettingData::~ObjectCommonActionSettingData()
{
	CC_SAFE_RELEASE_NULL(_objActionLink);
	CC_SAFE_RELEASE_NULL(_objAction);
	CC_SAFE_RELEASE_NULL(_objActionLink2);
}

bool ObjectCommonActionSettingData::init(const rapidjson::Value& json)
{
	CC_ASSERT(json.HasMember("id"));
	this->setId(json["id"].GetInt());
	CC_ASSERT(json.HasMember("objActionLink"));
	this->setObjActionLink(ObjectActionLinkData::create(json["objActionLink"]));
	this->getObjActionLink()->setCommonActionSettingData(this);
	CC_ASSERT(json.HasMember("objAction"));
	this->setObjAction(ObjectActionData::create(json["objAction"]));
	CC_ASSERT(json.HasMember("objActionLink2"));
	this->setObjActionLink2(ObjectActionLinkData::create(json["objActionLink2"]));
	this->getObjActionLink2()->setCommonActionSettingData(this);
	return true;
}

#if defined(AGTK_DEBUG)
void ObjectCommonActionSettingData::dump()
{
	CCLOG("id:%d", this->getId());
	CCLOG("objActionLink");
	this->getObjActionLink()->dump();
	CCLOG("objAction");
	this->getObjAction()->dump();
	CCLOG("objActionLink2");
	this->getObjActionLink2()->dump();
}
#endif

//-------------------------------------------------------------------------------------------------------------------
ObjectCommonActionGroupData::ObjectCommonActionGroupData()
{
	_name = nullptr;
	_a = 255;
	_r = 255;
	_g = 255;
	_b = 255;
	_actionIdList = nullptr;
}

ObjectCommonActionGroupData::~ObjectCommonActionGroupData()
{
	CC_SAFE_RELEASE_NULL(_name);
	CC_SAFE_RELEASE_NULL(_actionIdList);
}

const char *ObjectCommonActionGroupData::getName()
{
	CC_ASSERT(_name);
	return _name->getCString();
}

bool ObjectCommonActionGroupData::init(const rapidjson::Value& json)
{
	this->setId(json["id"].GetInt());
	this->setName(cocos2d::__String::create(json["name"].GetString()));
	this->setA(json["a"].GetInt());
	this->setR(json["r"].GetInt());
	this->setG(json["g"].GetInt());
	this->setB(json["b"].GetInt());
	auto actionIdList = cocos2d::__Array::create();
	for (rapidjson::SizeType i = 0; i < json["actionIdList"].Size(); i++) {
		auto p = cocos2d::Integer::create(json["actionIdList"][i].GetInt());
		actionIdList->addObject(p);
	}
	this->setActionIdList(actionIdList);
	this->setX(json["x"].GetInt());
	this->setY(json["y"].GetInt());
	this->setWidth(json["width"].GetInt());
	this->setHeight(json["height"].GetInt());
	this->setCollapsed(json["collapsed"].GetBool());
	return true;
}

#if defined(AGTK_DEBUG)
void ObjectCommonActionGroupData::dump()
{
	CCLOG("id:%d", this->getId());
	CCLOG("name:%s", this->getName());
	CCLOG("a:%d", this->getA());
	CCLOG("r:%d", this->getR());
	CCLOG("g:%d", this->getG());
	CCLOG("b:%d", this->getB());
	std::string tmp = "actionIdList:[";
	cocos2d::Ref *ref = nullptr;
	CCARRAY_FOREACH(this->getActionIdList(), ref) {
		auto p = dynamic_cast<cocos2d::Integer *>(ref);
		tmp += std::to_string(p->getValue());
		tmp += ",";
	}
	tmp += "]";
	CCLOG("%s", tmp.c_str());

	CCLOG("x:%d", this->getX());
	CCLOG("y:%d", this->getY());
	CCLOG("width:%d", this->getWidth());
	CCLOG("height:%d", this->getHeight());
	CCLOG("collapsed:%d", this->getCollapsed());
}
#endif

//-------------------------------------------------------------------------------------------------------------------
ObjectAroundCharacterViewSettingData::ObjectAroundCharacterViewSettingData()
{
	_tileOn = false;
	_objectOn = false;
	_viewType = EnumViewType::kDrawSOLID;
	_multiplyFill = false;
	_fillA = 0;
	_fillR = 0;
	_fillG = 0;
	_fillB = 0;
	_transparency = 0;
	_width = 0;
	_height = 0;
}

ObjectAroundCharacterViewSettingData::~ObjectAroundCharacterViewSettingData()
{
}

bool ObjectAroundCharacterViewSettingData::init(const rapidjson::Value& json)
{
	CC_ASSERT(json.HasMember("tileOn"));
	this->setTileOn(json["tileOn"].GetBool());
	CC_ASSERT(json.HasMember("objectOn"));
	this->setObjectOn(json["objectOn"].GetBool());
	CC_ASSERT(json.HasMember("viewType"));
	this->setViewType((EnumViewType)json["viewType"].GetInt());
	CC_ASSERT(json.HasMember("multiplyFill"));
	this->setMultiplyFill(json["multiplyFill"].GetBool());
	CC_ASSERT(json.HasMember("fillA"));
	this->setFillA(json["fillA"].GetInt());
	CC_ASSERT(json.HasMember("fillR"));
	this->setFillR(json["fillR"].GetInt());
	CC_ASSERT(json.HasMember("fillG"));
	this->setFillG(json["fillG"].GetInt());
	CC_ASSERT(json.HasMember("fillB"));
	this->setFillB(json["fillB"].GetInt());
	CC_ASSERT(json.HasMember("transparency"));
	this->setTransparency(json["transparency"].GetInt());
	CC_ASSERT(json.HasMember("width"));
	this->setWidth(json["width"].GetInt());
	CC_ASSERT(json.HasMember("height"));
	this->setHeight(json["height"].GetInt());
	return true;
}

#if defined(AGTK_DEBUG)
void ObjectAroundCharacterViewSettingData::dump()
{
	CCLOG("tileOn:%d", this->getTileOn());
	CCLOG("objectOn:%d", this->getObjectOn());
	CCLOG("viewType:%d", this->getViewType());
	CCLOG("multiplyFill:%d", this->getMultiplyFill());
	CCLOG("fillA:%d", this->getFillA());
	CCLOG("fillR:%d", this->getFillR());
	CCLOG("fillG:%d", this->getFillG());
	CCLOG("fillB:%d", this->getFillB());
	CCLOG("transparency:%d", this->getTransparency());
	CCLOG("width:%d", this->getWidth());
	CCLOG("height:%d", this->getHeight());
}
#endif

//-------------------------------------------------------------------------------------------------------------------
ObjectInvincibleSettingData::ObjectInvincibleSettingData()
{
	_name = nullptr;
	_filterEffect = nullptr;
}

ObjectInvincibleSettingData::~ObjectInvincibleSettingData()
{
	CC_SAFE_RELEASE_NULL(_name);
	CC_SAFE_RELEASE_NULL(_filterEffect);
}

bool ObjectInvincibleSettingData::init(const rapidjson::Value& json)
{
	CC_ASSERT(json.HasMember("id"));
	this->setId(json["id"].GetInt());
	CC_ASSERT(json.HasMember("name"));
	this->setName(cocos2d::__String::create(json["name"].GetString()));
	CC_ASSERT(json.HasMember("duration300"));
	this->setDuration300(json["duration300"].GetInt());
	CC_ASSERT(json.HasMember("infinite"));
	this->setInfinite(json["infinite"].GetBool());
	CC_ASSERT(json.HasMember("wink"));
	this->setWink(json["wink"].GetBool());
	CC_ASSERT(json.HasMember("winkInterval300"));
	this->setWinkInterval300(json["winkInterval300"].GetInt());
	CC_ASSERT(json.HasMember("finishWink"));
	this->setFinishWink(json["finishWink"].GetBool());
	CC_ASSERT(json.HasMember("finishWinkInterval300"));
	this->setFinishWinkInterval300(json["finishWinkInterval300"].GetInt());
	CC_ASSERT(json.HasMember("finishWinkDuration300"));
	this->setFinishWinkDuration300(json["finishWinkDuration300"].GetInt());
	CC_ASSERT(json.HasMember("wallAreaAttack"));
	this->setWallAreaAttack(json["wallAreaAttack"].GetBool());
	CC_ASSERT(json.HasMember("playBgm"));
	this->setPlayBgm(json["playBgm"].GetBool());
	CC_ASSERT(json.HasMember("bgmId"));
	this->setBgmId(json["bgmId"].GetInt());
	CC_ASSERT(json.HasMember("filterEffectFlag"));
	this->setFilterEffectFlag(json["filterEffectFlag"].GetBool());
	CC_ASSERT(json.HasMember("filterEffect"));
	this->setFilterEffect(FilterEffect::create(json["filterEffect"]));
	CC_ASSERT(json.HasMember("objectSwitch"));
	this->setObjectSwitch(json["objectSwitch"].GetBool());
	CC_ASSERT(json.HasMember("objectSwitchId"));
	this->setObjectSwitchId(json["objectSwitchId"].GetInt());
	CC_ASSERT(json.HasMember("systemSwitchId"));
	this->setSystemSwitchId(json["systemSwitchId"].GetInt());
	return true;
}

const char *ObjectInvincibleSettingData::getName()
{
	CC_ASSERT(_name);
	return _name->getCString();
}

#if defined(AGTK_DEBUG)
void ObjectInvincibleSettingData::dump()
{
	CCLOG("id:%d", this->getId());
	CCLOG("name:%s", this->getName());
	CCLOG("duration300:%d", this->getDuration300());
	CCLOG("wink:%d", this->getWink());
	CCLOG("winkInterval300:%d", this->getWinkInterval300());
	CCLOG("finishWink:%d", this->getFinishWink());
	CCLOG("finishWinkInterval300:%d", this->getFinishWinkInterval300());
	CCLOG("finishWinkDuration300:%d", this->getFinishWinkDuration300());
	CCLOG("wallAreaAttack:%d", this->getWallAreaAttack());
	CCLOG("playBgm:%d", this->getPlayBgm());
	CCLOG("bgmId:%d", this->getBgmId());
	CCLOG("FilterEffect");
	this->getFilterEffect()->dump();
	CCLOG("objectSwitch:%d", this->getObjectSwitch());
	CCLOG("objectSwitchId:%d", this->getObjectSwitchId());
	CCLOG("systemSwitchId:%d", this->getSystemSwitchId());
}
#endif

//-------------------------------------------------------------------------------------------------------------------
ObjectDamagedSettingData::ObjectDamagedSettingData()
{
	_name = nullptr;
	_damagedScript = nullptr;
	_filterEffect = nullptr;
}

ObjectDamagedSettingData::~ObjectDamagedSettingData()
{
	CC_SAFE_RELEASE_NULL(_name);
	CC_SAFE_RELEASE_NULL(_damagedScript);
	CC_SAFE_RELEASE_NULL(_filterEffect);
}

bool ObjectDamagedSettingData::init(const rapidjson::Value& json)
{
	CC_ASSERT(json.HasMember("id"));
	this->setId(json["id"].GetInt());
	CC_ASSERT(json.HasMember("name"));
	this->setName(cocos2d::__String::create(json["name"].GetString()));
	CC_ASSERT(json.HasMember("damagedRateFlag"));
	this->setDamagedRateFlag(json["damagedRateFlag"].GetBool());
	CC_ASSERT(json.HasMember("damagedRate"));
	this->setDamagedRate(json["damagedRate"].GetInt());
	CC_ASSERT(json.HasMember("damagedScript"));
	this->setDamagedScript(cocos2d::__String::create(json["damagedScript"].GetString()));
	CC_ASSERT(json.HasMember("attributeType"));
	this->setAttributeType(json["attributeType"].GetInt());
	CC_ASSERT(json.HasMember("attributePresetId"));
	this->setAttributePresetId(json["attributePresetId"].GetInt());
	CC_ASSERT(json.HasMember("attributeValue"));
	this->setAttributeValue(json["attributeValue"].GetInt());
	CC_ASSERT(json.HasMember("attributeEqual"));
	this->setAttributeEqual(json["attributeEqual"].GetBool());
	this->setCritical(json["critical"].GetBool());
	this->setCriticalRate(json["criticalRate"].GetInt());
	this->setDuration300(json["duration300"].GetDouble());
	this->setFilterEffectFlag(json["filterEffectFlag"].GetBool());
	if (this->getFilterEffectFlag()) {
		this->setFilterEffect(agtk::data::FilterEffect::create(json["filterEffect"]));
	}
	this->setWink(json["wink"].GetBool());
	this->setWinkInterval300(json["winkInterval300"].GetInt());
	this->setDioGameSpeed(json["dioGameSpeed"].GetDouble());
	this->setDioEffectDuration(json["dioEffectDuration"].GetDouble());
	this->setDioReceiving(json["dioReceiving"].GetBool());
	this->setDioRecvParent(json["dioRecvParent"].GetBool());
	this->setDioRecvChild(json["dioRecvChild"].GetBool());
	this->setDioDealing(json["dioDealing"].GetBool());
	this->setDioDealParent(json["dioDealParent"].GetBool());
	this->setDioDealChild(json["dioDealChild"].GetBool());
	this->setPlaySe(json["playSe"].GetBool());
	this->setSeId(json["seId"].GetInt());
	this->setObjectSwitch(json["objectSwitch"].GetBool());
	this->setObjectSwitchId(json["objectSwitchId"].GetInt());
	this->setSystemSwitchId(json["systemSwitchId"].GetInt());
	return true;
}

const char *ObjectDamagedSettingData::getName()
{
	CC_ASSERT(_name);
	return _name->getCString();
}

const char *ObjectDamagedSettingData::getDamagedScript()
{
	CC_ASSERT(_damagedScript);
	return _damagedScript->getCString();
}

#if defined(AGTK_DEBUG)
void ObjectDamagedSettingData::dump()
{
	CCLOG("id:%d", this->getId());
	CCLOG("name:%s", this->getName());
	CCLOG("damagedRateFlag:%d", this->getDamagedRateFlag());
	CCLOG("damagedRate:%d", this->getDamagedRate());
	CCLOG("damagedScript:%s", this->getDamagedScript());
	CCLOG("propertyType:%d", this->getAttributeType());
	CCLOG("propertyPresetId:%d", this->getAttributePresetId());
	CCLOG("propertyValue:%d", this->getAttributeValue());
	CCLOG("propertyEqual:%d", this->getAttributeEqual());
	CCLOG("critical:%d", this->getCritical());
	CCLOG("ciritcalRate:%d", this->getCriticalRate());
	CCLOG("duration300:%f", this->getDuration300());
	CCLOG("filterEffectFlag:%d", this->getFilterEffectFlag());
	if (this->getFilterEffect()) {
		this->getFilterEffect()->dump();
	}
	CCLOG("wink:%d", this->getWink());
	CCLOG("winkInterval300:%d", this->getWinkInterval300());
	CCLOG("dioGameSpeed:%f", this->getDioGameSpeed());
	CCLOG("dioEffectDuration:%f", this->getDioEffectDuration());
	CCLOG("dioReceiving:%d", this->getDioReceiving());
	CCLOG("dioRecvParent:%d", this->getDioRecvParent());
	CCLOG("dioRecvChild:%d", this->getDioRecvChild());
	CCLOG("dioDealing:%d", this->getDioDealing());
	CCLOG("dioDealParent:%d", this->getDioDealParent());
	CCLOG("dioDealChild:%d", this->getDioDealChild());
	CCLOG("playSe:%d", this->getPlaySe());
	CCLOG("seId:%d", this->getSeId());
	CCLOG("objectSwitch:%d", this->getObjectSwitch());
	CCLOG("objectSwitchId:%d", this->getObjectSwitchId());
	CCLOG("systemSwitchId:%d", this->getSystemSwitchId());
}
#endif

//-------------------------------------------------------------------------------------------------------------------
ObjectMoveRestrictionSettingData::ObjectMoveRestrictionSettingData()
{
	_name = nullptr;
}

ObjectMoveRestrictionSettingData::~ObjectMoveRestrictionSettingData()
{
	CC_SAFE_RELEASE_NULL(_name);
}

bool ObjectMoveRestrictionSettingData::init(const rapidjson::Value& json)
{
	CC_ASSERT(json.HasMember("id"));
	this->setId(json["id"].GetInt());
	CC_ASSERT(json.HasMember("name"));
	this->setName(cocos2d::__String::create(json["name"].GetString()));
	CC_ASSERT(json.HasMember("left"));
	this->setLeft(json["left"].GetInt());
	CC_ASSERT(json.HasMember("right"));
	this->setRight(json["right"].GetInt());
	CC_ASSERT(json.HasMember("top"));
	this->setTop(json["top"].GetInt());
	CC_ASSERT(json.HasMember("bottom"));
	this->setBottom(json["bottom"].GetInt());
	CC_ASSERT(json.HasMember("objectSwitch"));
	this->setObjectSwitch(json["objectSwitch"].GetBool());
	CC_ASSERT(json.HasMember("objectSwitchId"));
	this->setObjectSwitchId(json["objectSwitchId"].GetInt());
	CC_ASSERT(json.HasMember("systemSwitchId"));
	this->setSystemSwitchId(json["systemSwitchId"].GetInt());
	return true;
}

const char *ObjectMoveRestrictionSettingData::getName()
{
	CC_ASSERT(_name);
	return _name->getCString();
}

#if defined(AGTK_DEBUG)
void ObjectMoveRestrictionSettingData::dump()
{
	CCLOG("id:%d", this->getId());
	CCLOG("name:%s", this->getName());
	CCLOG("left:%d", this->getLeft());
	CCLOG("right:%d", this->getRight());
	CCLOG("top:%d", this->getTop());
	CCLOG("bottom:%d", this->getBottom());
	CCLOG("objectSwitch:%d", this->getObjectSwitch());
	CCLOG("objectSwitchId:%d", this->getObjectSwitchId());
	CCLOG("systemSwitchId:%d", this->getSystemSwitchId());
}
#endif

//-------------------------------------------------------------------------------------------------------------------
ObjectReappearData::ObjectReappearData()
{
	_sceneId = -1;
	_scenePartsId = 0;
	_objectId = -1;
	_initialActionId = -1;
	_initialPosition = cocos2d::Vec2::ZERO;
	_initialScale = cocos2d::Vec2::ONE;
	_initialRotation = 0;
	_initialMoveDirectionId = 0;
	_reappearFlag = false;
	_initialCourseId = -1;
	_initialCoursePointId = -1;
}

ObjectReappearData::~ObjectReappearData()
{
}

bool ObjectReappearData::init()
{
	return true;
}

#if defined(AGTK_DEBUG)
void ObjectReappearData::dump()
{
	CCLOG("sceneId:%d", this->getSceneId());
	CCLOG("objectId:%d", this->getObjectId());
	CCLOG("initialActionId:%d", this->getInitialActionId());
	CCLOG("initialPosition(%f,%f)", this->getInitialPosition().x, this->getInitialPosition().y);
	CCLOG("initialScale(%f,%f)", this->getInitialScale().x, this->getInitialScale().y);
	CCLOG("initialRotation:%f", this->getInitialRotation());
	CCLOG("initialMoveDirectionId:%d", this->getInitialMoveDirectionId());
	CCLOG("initialCourseId:%d", this->getInitialCourseId());
	CCLOG("initialCoursePointId:%d", this->getInitialCoursePointId());
}
#endif

//-------------------------------------------------------------------------------------------------------------------
/**
 * @class ObjectData
 */
ObjectData::ObjectData()
{
	_name = nullptr;
	_children = nullptr;
	_actionList = nullptr;
	_actionLinkList = nullptr;
	_actionGroupList = nullptr;
	_disappearSetting = nullptr;
	_fireBulletSettingList = nullptr;
	_effectSettingList = nullptr;
	_connectSettingList = nullptr;
	_additionalDisplayList = nullptr;
	_moveDispDirectionSetting = nullptr;
	_physicsSetting = nullptr;
	_viewportLightSettingList = nullptr;
	_commonActionSettingList = nullptr;
	_commonActionGroup = nullptr;
	_aroundCharacterViewSetting = nullptr;
	_invincibleSettingList = nullptr;
	_damagedSettingList = nullptr;
	_moveRestrictionSettingList = nullptr;
	_memo = nullptr;
	_variableList = nullptr;
	_switchList = nullptr;
	_physicsPartList = nullptr;
	_moveRestrictionSettingFlag = false;
	_moveAroundWithinDots = false;
	_dotsToMoveAround = 0;
	_priorityInPhysics = 0;
	_priority = 0;
	_dispPriority = 0;
	_fallOnCollideWithWall = true;
	_fallOnCollideWithHitbox = true;
	_takeOverDamageRateToParent = false;
}

ObjectData::~ObjectData()
{
	CC_SAFE_RELEASE_NULL(_name);
	CC_SAFE_RELEASE_NULL(_children);
	CC_SAFE_RELEASE_NULL(_actionList);
	CC_SAFE_RELEASE_NULL(_actionLinkList);
	CC_SAFE_RELEASE_NULL(_actionGroupList);
	CC_SAFE_RELEASE_NULL(_disappearSetting);
	CC_SAFE_RELEASE_NULL(_fireBulletSettingList);
	CC_SAFE_RELEASE_NULL(_effectSettingList);
	CC_SAFE_RELEASE_NULL(_connectSettingList);
	CC_SAFE_RELEASE_NULL(_additionalDisplayList);
	CC_SAFE_RELEASE_NULL(_moveDispDirectionSetting);
	CC_SAFE_RELEASE_NULL(_physicsSetting);
	CC_SAFE_RELEASE_NULL(_viewportLightSettingList);
	CC_SAFE_RELEASE_NULL(_commonActionSettingList);
	CC_SAFE_RELEASE_NULL(_commonActionGroup);
	CC_SAFE_RELEASE_NULL(_aroundCharacterViewSetting);
	CC_SAFE_RELEASE_NULL(_invincibleSettingList);
	CC_SAFE_RELEASE_NULL(_damagedSettingList);
	CC_SAFE_RELEASE_NULL(_moveRestrictionSettingList);
	CC_SAFE_RELEASE_NULL(_memo);
	CC_SAFE_RELEASE_NULL(_variableList);
	CC_SAFE_RELEASE_NULL(_switchList);
	CC_SAFE_RELEASE_NULL(_physicsPartList);
}

bool ObjectData::init(const rapidjson::Value& json)
{
	this->setId(json["id"].GetInt());
	this->setName(cocos2d::__String::create(json["name"].GetString()));
	this->setFolder(json["folder"].GetBool());
	if (this->getFolder()) {
		auto dic = cocos2d::__Dictionary::create();
		if (json.HasMember("children")) {
			for (rapidjson::SizeType i = 0; i < json["children"].Size(); i++) {
				auto p = ObjectData::create(json["children"][i]);
#if defined(AGTK_DEBUG)
				CC_ASSERT(dic->objectForKey(p->getId()) == nullptr);
#endif
				dic->setObject(p, p->getId());
			}
		}
		this->setChildren(dic);
		return true;
	}
	this->setGroup((EnumObjGroup)json["group"].GetInt());
	this->setAnimationId(json["animationId"].GetInt());
	this->setOperatable(json["operatable"].GetBool());
	this->setPriority(json["priority"].GetInt());
	if (json.HasMember("dispPriority")) {
		this->setDispPriority(json["dispPriority"].GetInt());
	}
	this->setInitialActionId(json["initialActionId"].GetInt());
	this->setHitObjectGroupBit(json["hitObjectGroupBit"].GetInt());
	this->setCollideWithObjectGroupBit(json["collideWithObjectGroupBit"].GetInt());
	this->setCollideWithTileGroupBit(json["collideWithTileGroupBit"].GetInt());
	this->setInitialDisplayDirectionFlag(json["initialDisplayDirectionFlag"].GetBool());
	this->setInitialDisplayDirection(json["initialDisplayDirection"].GetDouble());
	auto actionList = cocos2d::__Dictionary::create();
	for (rapidjson::SizeType i = 0; i < json["actionList"].Size(); i++) {
		auto actionData = ObjectActionData::create(json["actionList"][i]);
#if defined(AGTK_DEBUG)
		CC_ASSERT(actionList->objectForKey(actionData->getId()) == nullptr);
#endif
		actionList->setObject(actionData, actionData->getId());
	}
	this->setActionList(actionList);
	auto actionLinkList = cocos2d::__Dictionary::create();
	for (rapidjson::SizeType i = 0; i < json["actionLinkList"].Size(); i++) {
		auto &actionLinkJson = json["actionLinkList"][i];
		if (actionLinkJson.HasMember("typeIdPair")) {
			bool ignored = false;
			for (rapidjson::SizeType j = 0; j < actionLinkJson["typeIdPair"].Size(); j++) {
				CC_ASSERT(actionLinkJson["typeIdPair"][j].Size() >= 2);
				if (actionLinkJson["typeIdPair"][j][0].GetInt() != 0) {
					ignored = true;
					break;
				}
			}
			if (ignored) continue;
		}
		auto actionLinkData = ObjectActionLinkData::create(json["actionLinkList"][i]);
#if defined(AGTK_DEBUG)
		CC_ASSERT(actionLinkList->objectForKey(actionLinkData->getId()) == nullptr);
#endif
		actionLinkList->setObject(actionLinkData, actionLinkData->getId());
	}
	this->setActionLinkList(actionLinkList);
	auto actionGroupList = cocos2d::__Dictionary::create();
	for (rapidjson::SizeType i = 0; i < json["actionGroupList"].Size(); i++) {
		auto actionGroupData = ObjectActionGroupData::create(json["actionGroupList"][i]);
#if defined(AGTK_DEBUG)
		CC_ASSERT(actionGroupList->objectForKey(actionGroupData->getId()) == nullptr);
#endif
		actionGroupList->setObject(actionGroupData, actionGroupData->getId());
	}
	this->setActionGroupList(actionGroupList);
	this->setMemo(cocos2d::__String::create(json["memo"].GetString()));
	this->setTestplayOnly(json["testplayOnly"].GetBool());
	this->setAppearCondition((EnumAppearCondition)json["appearCondition"].GetInt());
	this->setAppearConditionTileCount(json["appearConditionTileCount"].GetInt());
	this->setReappearCondition((EnumReappearCondition)json["reappearCondition"].GetInt());
	this->setReappearConditionTileCount(json["reappearConditionTileCount"].GetInt());
	this->setHp(json["hp"].GetInt());
	this->setMaxHp(json["maxHp"].GetInt());
	if (json.HasMember("minAttack")) {
		this->setMinAttack(json["minAttack"].GetInt());
	} 	else if (json.HasMember("initialAttack")) {
		this->setMinAttack(json["initialAttack"].GetInt());
	} else {
		this->setMinAttack(0);
	}
	this->setMaxAttack(json["maxAttack"].GetInt());
	this->setInitialDamageRate(json["initialDamageRate"].GetInt());
	if (json.HasMember("damageVariationValue")) {
		this->setDamageVariationValue(json["damageVariationValue"].GetInt());
	}
	else {
		this->setDamageVariationValue(0);
	}
	this->setCritical(json["critical"].GetBool());
	this->setCriticalIncidence(json["criticalIncidence"].GetDouble());
	this->setCriticalRatio(json["criticalRatio"].GetDouble());
	this->setMoveType((EnumMoveType)json["moveType"].GetInt());
	this->setNormalAccelMove(json["normalAccelMove"].GetBool());
	this->setTankAccelMove(json["tankAccelMove"].GetBool());
	this->setCarAccelMove(json["carAccelMove"].GetBool());
	this->setSlipOnSlope(json["slipOnSlope"].GetBool());
	this->setDiagonalMoveWithPolar(json["diagonalMoveWithPolar"].GetBool());
	if (json.HasMember("moveAroundWithinDots")) {
		this->setMoveAroundWithinDots(json["moveAroundWithinDots"].GetBool());
	}
	if (json.HasMember("dotsToMoveAround")) {
		this->setDotsToMoveAround(json["dotsToMoveAround"].GetInt());
	}
	this->setUpMoveKeyId(json["upMoveKeyId"].GetInt());
	this->setDownMoveKeyId(json["downMoveKeyId"].GetInt());
	this->setLeftMoveKeyId(json["leftMoveKeyId"].GetInt());
	this->setRightMoveKeyId(json["rightMoveKeyId"].GetInt());
	this->setForwardMoveKeyId(json["forwardMoveKeyId"].GetInt());
	this->setBackwardMoveKeyId(json["backwardMoveKeyId"].GetInt());
	this->setLeftTurnKeyId(json["leftTurnKeyId"].GetInt());
	this->setRightTurnKeyId(json["rightTurnKeyId"].GetInt());
	this->setHorizontalMove(json["horizontalMove"].GetDouble());
	this->setVerticalMove(json["verticalMove"].GetDouble());
	this->setHorizontalAccel(json["horizontalAccel"].GetDouble());
	this->setHorizontalMaxMove(json["horizontalMaxMove"].GetDouble());
	this->setHorizontalDecel(json["horizontalDecel"].GetDouble());
	this->setVerticalAccel(json["verticalAccel"].GetDouble());
	this->setVerticalMaxMove(json["verticalMaxMove"].GetDouble());
	this->setVerticalDecel(json["verticalDecel"].GetDouble());
	this->setForwardMove(json["forwardMove"].GetDouble());
	this->setBackwardMove(json["backwardMove"].GetDouble());
	this->setForwardAccel(json["forwardAccel"].GetDouble());
	this->setForwardMaxMove(json["forwardMaxMove"].GetDouble());
	this->setForwardDecel(json["forwardDecel"].GetDouble());
	this->setBackwardAccel(json["backwardAccel"].GetDouble());
	this->setBackwardMaxMove(json["backwardMaxMove"].GetDouble());
	this->setBackwardDecel(json["backwardDecel"].GetDouble());
	this->setLeftTurn(json["leftTurn"].GetDouble());
	this->setRightTurn(json["rightTurn"].GetDouble());
	this->setJumpInitialSpeed(json["jumpInitialSpeed"].GetDouble());
	this->setGravity(json["gravity"].GetDouble());
	this->setMovableWhenJumping(json["movableWhenJumping"].GetBool());
	this->setMovableWhenFalling(json["movableWhenFalling"].GetBool());
	if (json.HasMember("fallOnCollideWithWall")) {
		this->setFallOnCollideWithWall(json["fallOnCollideWithWall"].GetBool());
	}
	if (json.HasMember("fallOnCollideWithHitbox")) {
		this->setFallOnCollideWithHitbox(json["fallOnCollideWithHitbox"].GetBool());
	}
	if (json.HasMember("jumpInputOperationKeyId")) {
		this->setJumpInputOperationKeyId(json["jumpInputOperationKeyId"].GetInt());
	}
	this->setAffectInputDuration300(json["affectInputDuration300"].GetInt());
	this->setInvincibleOnDamaged(json["invincibleOnDamaged"].GetBool());
	this->setInvincibleDuration300(json["invincibleDuration300"].GetInt());
	this->setWinkWhenInvincible(json["winkWhenInvincible"].GetBool());
	if (json.HasMember("winkInterval300")) {
		this->setWinkInterval300(json["winkInterval300"].GetInt());
	} else if (json.HasMember("winkDuration300")) {
		this->setWinkInterval300(json["winkDuration300"].GetInt());
	}
	this->setAfterimage(json["afterimage"].GetBool());
	this->setAfterimageCount(json["afterimageCount"].GetInt());
	this->setAfterimageInterval(json["afterimageInterval"].GetDouble());
	this->setAfterimageDuration300(json["afterimageDuration300"].GetInt());
	this->setFillAfterimage(json["fillAfterimage"].GetBool());
	this->setFillAfterimageA(json["fillAfterimageA"].GetInt());
	this->setFillAfterimageR(json["fillAfterimageR"].GetInt());
	this->setFillAfterimageG(json["fillAfterimageG"].GetInt());
	this->setFillAfterimageB(json["fillAfterimageB"].GetInt());
	this->setFadeoutAfterimage(json["fadeoutAfterimage"].GetBool());
	this->setFadeoutAfterimageStart300(json["fadeoutAfterimageStart300"].GetDouble());
	this->setUseAnimationAfterimage(json["useAnimationAfterimage"].GetBool());
	this->setAfterimageAnimationId(json["afterimageAnimationId"].GetInt());
	this->setFollowType((EnumFollowType)json["followType"].GetInt());
	if (json.HasMember("followPrecision")) {
		this->setFollowPrecision(json["followPrecision"].GetInt());
	}
	this->setFollowIntervalByTime300(json["followIntervalByTime300"].GetDouble());
	if (this->getFollowIntervalByTime300() < 0) {
		this->setFollowIntervalByTime300(0);
	}
	this->setFollowIntervalByLocusLength(json["followIntervalByLocusLength"].GetDouble());
	if (this->getFollowIntervalByLocusLength() < 0) {
		this->setFollowIntervalByLocusLength(0);
	}
	this->setChildDamageType((EnumChildDamageType)json["childDamageType"].GetInt());
	if (json.HasMember("takeOverDamageRateToParent")) {
		this->setTakeOverDamageRateToParent(json["takeOverDamageRateToParent"].GetBool());
	}
	this->setUnattackableToParent(json["unattackableToParent"].GetBool());
	if (json.HasMember("takeoverDispDirection")) {
		this->setTakeoverDispDirection(json["takeoverDispDirection"].GetBool());
	}
	this->setTakeoverAngle(json["takeoverAngle"].GetBool());
	this->setTakeoverScaling(json["takeoverScaling"].GetBool());
	this->setTakeoverIntensity(json["takeoverIntensity"].GetBool());
	this->setDisappearWhenHp0(json["disappearWhenHp0"].GetBool());
	this->setDisappearWhenOutOfCamera(json["disappearWhenOutOfCamera"].GetBool());
	this->setDisappearCameraMarginTileCount(json["disappearCameraMarginTileCount"].GetInt());
	this->setFixedInCamera(json["fixedInCamera"].GetBool());
	this->setLimitFalldownAmount(json["limitFalldownAmount"].GetBool());
	this->setMaxFalldownAmount(json["maxFalldownAmount"].GetDouble());
	if (json.HasMember("pushedbackByObject")) {
		this->setPushedbackByObject(json["pushedbackByObject"].GetBool());
	}
	if (json.HasMember("takeoverStatesAtSceneEnd")) {
		this->setTakeoverStatesAtSceneEnd(json["takeoverStatesAtSceneEnd"].GetBool());
	}
	this->setDisappearSettingFlag(json["disappearSettingFlag"].GetBool());
	this->setDisappearSetting(ObjectDisappearSettingData::create(json["disappearSetting"]));

	//fireBulletSettingList
	this->setFireBulletSettingFlag(json["fireBulletSettingFlag"].GetBool());
	auto fireBulletSettingList = cocos2d::__Dictionary::create();
	for (rapidjson::SizeType i = 0; i < json["fireBulletSettingList"].Size(); i++) {
		auto fireBulletSettingData = ObjectFireBulletSettingData::create(json["fireBulletSettingList"][i]);
#if defined(AGTK_DEBUG)
		CC_ASSERT(fireBulletSettingList->objectForKey(fireBulletSettingData->getId()) == nullptr);
#endif
		fireBulletSettingList->setObject(fireBulletSettingData, fireBulletSettingData->getId());
	}
	this->setFireBulletSettingList(fireBulletSettingList);

	//effectSettting
	this->setEffectSettingFlag(json["effectSettingFlag"].GetBool());
	auto effectSettingList = cocos2d::__Dictionary::create();
	for (rapidjson::SizeType i = 0; i < json["effectSettingList"].Size(); i++) {
		auto effectSettingData = ObjectEffectSettingData::create(json["effectSettingList"][i]);
#if defined(AGTK_DEBUG)
		CC_ASSERT(effectSettingList->objectForKey(effectSettingData->getId()) == nullptr);
#endif
		effectSettingList->setObject(effectSettingData, effectSettingData->getId());
	}
	this->setEffectSettingList(effectSettingList);

	//connectSetting
	this->setConnectSettingFlag(json["connectSettingFlag"].GetBool());
	auto connectSettingList = cocos2d::__Dictionary::create();
	for (rapidjson::SizeType i = 0; i < json["connectSettingList"].Size(); i++) {
		auto connectSettingData = ObjectConnectSettingData::create(json["connectSettingList"][i]);
#if defined(AGTK_DEBUG)
		CC_ASSERT(connectSettingList->objectForKey(connectSettingData->getId()) == nullptr);
#endif
		connectSettingList->setObject(connectSettingData, connectSettingData->getId());
	}
	this->setConnectSettingList(connectSettingList);

	//additionalDisplay
	if (json.HasMember("additionalDisplayList")) {
		auto additionalDisplayList = cocos2d::__Dictionary::create();
		for (rapidjson::SizeType i = 0; i < json["additionalDisplayList"].Size(); i++) {
			auto additionalDisplayData = ObjectAdditionalDisplayData::create(json["additionalDisplayList"][i]);
#if defined(AGTK_DEBUG)
			CC_ASSERT(additionalDisplayList->objectForKey(additionalDisplayData->getId()) == nullptr);
#endif
			additionalDisplayList->setObject(additionalDisplayData, additionalDisplayData->getId());
		}
		this->setAdditionalDisplayList(additionalDisplayList);
	}

	this->setMoveDispDirectionSettingFlag(json["moveDispDirectionSettingFlag"].GetBool());
	this->setMoveDispDirectionSetting(ObjectMoveDispDirectionSettingData::create(json["moveDispDirectionSetting"]));
	this->setPhysicsSettingFlag(json["physicsSettingFlag"].GetBool());
	this->setPhysicsSetting(ObjectPhysicsSettingData::create(json["physicsSetting"]));

	//physicsPartList
	auto physicsPartList = cocos2d::__Dictionary::create();
	std::function<void(const rapidjson::Value &)> setPhysicsPartDataRecur = [&setPhysicsPartDataRecur, physicsPartList](const rapidjson::Value &jsonData)
	{
		for (rapidjson::SizeType i = 0; i < jsonData.Size(); i++) {

			if (jsonData[i]["folder"].GetBool()) {
				// 子供がある場合
				if (jsonData[i].HasMember("children")) {
					setPhysicsPartDataRecur(jsonData[i]["children"]);
				}
				continue;
			}

			auto physicsPartData = PhysicsPartData::create(jsonData[i]);
#if defined(AGTK_DEBUG)
			CC_ASSERT(physicsPartList->objectForKey(physicsPartData->getPriority()) == nullptr);
#endif
			physicsPartList->setObject(physicsPartData, physicsPartData->getPriority());
		}
	};
	setPhysicsPartDataRecur(json["physicsPartList"]);
	this->setPhysicsPartList(physicsPartList);

	if (json.HasMember("priorityInPhysics")) {
		this->setPriorityInPhysics(json["priorityInPhysics"].GetInt());
	}

	//viewportLightSetting
	this->setViewportLightSettingFlag(json["viewportLightSettingFlag"].GetBool());
	auto viewportLightSettingList = cocos2d::__Dictionary::create();
	for (rapidjson::SizeType i = 0; i < json["viewportLightSettingList"].Size(); i++) {
		auto viewportLightSettingData = ObjectViewportLightSettingData::create(json["viewportLightSettingList"][i]);
#if defined(AGTK_DEBUG)
		CC_ASSERT(viewportLightSettingList->objectForKey(viewportLightSettingData->getId()) == nullptr);
#endif
		viewportLightSettingList->setObject(viewportLightSettingData, viewportLightSettingData->getId());
	}
	this->setViewportLightSettingList(viewportLightSettingList);

	//commonActionSetting
	this->setCommonActionSettingFlag(json["commonActionSettingFlag"].GetBool());
	auto commonActionSettingList = cocos2d::__Dictionary::create();
	for (rapidjson::SizeType i = 0; i < json["commonActionSettingList"].Size(); i++) {
		auto commonActionSettingData = ObjectCommonActionSettingData::create(json["commonActionSettingList"][i]);
#if defined(AGTK_DEBUG)
		CC_ASSERT(commonActionSettingList->objectForKey(commonActionSettingData->getId()) == nullptr);
#endif
		commonActionSettingList->setObject(commonActionSettingData, commonActionSettingData->getId());
	}
	this->setCommonActionSettingList(commonActionSettingList);

	auto commonActionGroup = ObjectCommonActionGroupData::create(json["commonActionGroup"]);
	this->setCommonActionGroup(commonActionGroup);
	this->setAroundCharacterViewSettingFlag(json["aroundCharacterViewSettingFlag"].GetBool());
	this->setAroundCharacterViewSetting(ObjectAroundCharacterViewSettingData::create(json["aroundCharacterViewSetting"]));

	//invincibleSetting
	this->setInvincibleSettingFlag(json["invincibleSettingFlag"].GetBool());
	auto invincibleSettingList = cocos2d::__Dictionary::create();
	for (rapidjson::SizeType i = 0; i < json["invincibleSettingList"].Size(); i++) {
		auto invincibleSettingData = ObjectInvincibleSettingData::create(json["invincibleSettingList"][i]);
#if defined(AGTK_DEBUG)
		CC_ASSERT(invincibleSettingList->objectForKey(invincibleSettingData->getId()) == nullptr);
#endif
		invincibleSettingList->setObject(invincibleSettingData, invincibleSettingData->getId());
	}
	this->setInvincibleSettingList(invincibleSettingList);

	//damagedSetting
	this->setDamagedSettingFlag(json["damagedSettingFlag"].GetBool());
	auto damagedSettingList = cocos2d::__Dictionary::create();
	for (rapidjson::SizeType i = 0; i < json["damagedSettingList"].Size(); i++) {
		auto damagedSettingData = ObjectDamagedSettingData::create(json["damagedSettingList"][i]);
#if defined(AGTK_DEBUG)
		CC_ASSERT(damagedSettingList->objectForKey(damagedSettingData->getId()) == nullptr);
#endif
		damagedSettingList->setObject(damagedSettingData, damagedSettingData->getId());
	}
	this->setDamagedSettingList(damagedSettingList);

	//moveRestrictionSetting
	if (json.HasMember("moveRestrictionSettingFlag")) {
		this->setMoveRestrictionSettingFlag(json["moveRestrictionSettingFlag"].GetBool());
	}
	auto moveRestrictionSettingList = cocos2d::__Dictionary::create();
	if (json.HasMember("moveRestrictionSettingList")) {
		for (rapidjson::SizeType i = 0; i < json["moveRestrictionSettingList"].Size(); i++) {
			auto moveRestrictionSettingData = ObjectMoveRestrictionSettingData::create(json["moveRestrictionSettingList"][i]);
#if defined(AGTK_DEBUG)
			CC_ASSERT(moveRestrictionSettingList->objectForKey(moveRestrictionSettingData->getId()) == nullptr);
#endif
			moveRestrictionSettingList->setObject(moveRestrictionSettingData, moveRestrictionSettingData->getId());
		}
	}
	this->setMoveRestrictionSettingList(moveRestrictionSettingList);

	//variableList
	auto variableList = cocos2d::__Dictionary::create();
	for (rapidjson::SizeType i = 0; i < json["variableList"].Size(); i++) {
		auto variableData = VariableData::create(json["variableList"][i]);
#if defined(AGTK_DEBUG)
		CC_ASSERT(variableList->objectForKey(variableData->getId()) == nullptr);
#endif
		variableList->setObject(variableData, variableData->getId());
	}
	this->setVariableList(variableList);

	//switchList
	auto switchList = cocos2d::__Dictionary::create();
	for (rapidjson::SizeType i = 0; i < json["switchList"].Size(); i++) {
		auto switchData = SwitchData::create(json["switchList"][i]);
#if defined(AGTK_DEBUG)
		CC_ASSERT(switchList->objectForKey(switchData->getId()) == nullptr);
#endif
		switchList->setObject(switchData, switchData->getId());
	}
	this->setSwitchList(switchList);

// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
	this->setDatabaseId(json["databaseId"].GetInt());
#endif
	return true;
}

const char *ObjectData::getName()
{
	CC_ASSERT(_name);
	return _name->getCString();
}

ObjectActionData *ObjectData::getActionData(int id)
{
	CC_ASSERT(_actionList);
	auto data = dynamic_cast<ObjectActionData *>(this->getActionList()->objectForKey(id));
	if (nullptr == data && this->getCommonActionSettingFlag()) {
		auto list = this->getCommonActionSettingList();
		cocos2d::DictElement* el = nullptr;
		CCDICT_FOREACH(list, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto p = static_cast<ObjectCommonActionSettingData *>(el->getObject());
#else
			auto p = dynamic_cast<ObjectCommonActionSettingData *>(el->getObject());
#endif
			if (p->getObjAction()->getId() == id) {
				data = p->getObjAction();
				break;
			}
		}
	}
	return data;
}

cocos2d::__Array *ObjectData::getVariableArray()
{
	std::function<void(cocos2d::__Dictionary *, cocos2d::__Array *)> func = [&](cocos2d::__Dictionary *children, cocos2d::__Array *arr) {
		cocos2d::DictElement *el = nullptr;
		CCDICT_FOREACH(children, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto p = static_cast<agtk::data::VariableData *>(el->getObject());
#else
			auto p = dynamic_cast<agtk::data::VariableData *>(el->getObject());
#endif
			if (p->getChildren()) {
				func(p->getChildren(), arr);
			}
			if (p->getFolder() == false) {
				arr->addObject(p);
			}
		}
	};
	auto arr = cocos2d::__Array::create();
	func(this->getVariableList(), arr);
	return arr;
}

agtk::data::VariableData *ObjectData::getVariableData(int id)
{
	std::function<agtk::data::VariableData *(cocos2d::__Dictionary *)> func = [&](cocos2d::__Dictionary *children) {
		cocos2d::DictElement *el = nullptr;
		CCDICT_FOREACH(children, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto p = static_cast<agtk::data::VariableData *>(el->getObject());
#else
			auto p = dynamic_cast<agtk::data::VariableData *>(el->getObject());
#endif
			if (p->getChildren()) {
				auto data = func(p->getChildren());
				if (data) {
					return data;
				}
			}
			if (p->getFolder() == false) {
				if (p->getId() == id) {
					return p;
				}
			}
		}
		return (agtk::data::VariableData *)nullptr;
	};
	return func(this->getVariableList());
}

agtk::data::VariableData *ObjectData::getVariableDataByName(const char *name)
{
	std::function<agtk::data::VariableData *(cocos2d::__Dictionary *)> func = [&](cocos2d::__Dictionary *children) {
		cocos2d::DictElement *el = nullptr;
		CCDICT_FOREACH(children, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto p = static_cast<agtk::data::VariableData *>(el->getObject());
#else
			auto p = dynamic_cast<agtk::data::VariableData *>(el->getObject());
#endif
			if (p->getChildren()) {
				auto data = func(p->getChildren());
				if (data) {
					return data;
				}
			}
			if (p->getFolder() == false) {
				if (strcmp(p->getName(), name) == 0) {
					return p;
				}
			}
		}
		return (agtk::data::VariableData *)nullptr;
	};
	return func(this->getVariableList());
}

cocos2d::__Array *ObjectData::getSwitchArray()
{
	std::function<void(cocos2d::__Dictionary *, cocos2d::__Array *)> func = [&](cocos2d::__Dictionary *children, cocos2d::__Array *arr) {
		cocos2d::DictElement *el = nullptr;
		CCDICT_FOREACH(children, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto p = static_cast<agtk::data::SwitchData *>(el->getObject());
#else
			auto p = dynamic_cast<agtk::data::SwitchData *>(el->getObject());
#endif
			if (p->getChildren()) {
				func(p->getChildren(), arr);
			}
			if (p->getFolder() == false) {
				arr->addObject(p);
			}
		}
	};
	auto arr = cocos2d::__Array::create();
	func(this->getSwitchList(), arr);
	return arr;
}

agtk::data::SwitchData *ObjectData::getSwitchData(int id)
{
	std::function<agtk::data::SwitchData *(cocos2d::__Dictionary *)> func = [&](cocos2d::__Dictionary *children) {
		cocos2d::DictElement *el = nullptr;
		CCDICT_FOREACH(children, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto p = static_cast<agtk::data::SwitchData *>(el->getObject());
#else
			auto p = dynamic_cast<agtk::data::SwitchData *>(el->getObject());
#endif
			if (p->getChildren()) {
				auto data = func(p->getChildren());
				if (data) {
					return data;
				}
			}
			if (p->getFolder() == false) {
				if (p->getId() == id) {
					return p;
				}
			}
		}
		return (agtk::data::SwitchData *)nullptr;
	};
	return func(this->getSwitchList());
}

agtk::data::SwitchData *ObjectData::getSwitchDataByName(const char *name)
{
	std::function<agtk::data::SwitchData *(cocos2d::__Dictionary *)> func = [&](cocos2d::__Dictionary *children) {
		cocos2d::DictElement *el = nullptr;
		CCDICT_FOREACH(children, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto p = static_cast<agtk::data::SwitchData *>(el->getObject());
#else
			auto p = dynamic_cast<agtk::data::SwitchData *>(el->getObject());
#endif
			if (p->getChildren()) {
				auto data = func(p->getChildren());
				if (data) {
					return data;
				}
			}
			if (p->getFolder() == false) {
				if (strcmp(p->getName(), name) == 0) {
					return p;
				}
			}
		}
		return (agtk::data::SwitchData *)nullptr;
	};
	return func(this->getSwitchList());
}

ObjectActionLinkData *ObjectData::getActionLinkData(int id)
{
	CC_ASSERT(_actionLinkList);
	return dynamic_cast<ObjectActionLinkData *>(this->getActionLinkList()->objectForKey(id));
}

ObjectActionGroupData *ObjectData::getActionGroupData(int id)
{
	CC_ASSERT(_actionGroupList);
	return dynamic_cast<ObjectActionGroupData *>(this->getActionGroupList()->objectForKey(id));
}

ObjectFireBulletSettingData *ObjectData::getFireBulletSettingData(int id)
{
	CC_ASSERT(_fireBulletSettingList);
	return dynamic_cast<ObjectFireBulletSettingData *>(this->getFireBulletSettingList()->objectForKey(id));
}

ObjectViewportLightSettingData *ObjectData::getViewportLightSettingData(int id)
{
	CC_ASSERT(_viewportLightSettingList);
	return dynamic_cast<ObjectViewportLightSettingData *>(this->getViewportLightSettingList()->objectForKey(id));
}

bool ObjectData::isGroupPlayer()const
{
	return (this->getGroup() == kObjGroupPlayer);
}

bool ObjectData::isGroupEnemy()const
{
	return (this->getGroup() == kObjGroupEnemy);
}

int ObjectData::getGroupBit()const 
{ 
	return (1 << _group); 
}

bool ObjectData::isCollideWith(ObjectData* data)const 
{
	if ((isCollideWithObjectGroup(data->getGroup())) &&
		(data->isCollideWithObjectGroup(getGroup())) ) {
		return true;
	}
	return false;// 相手グループとは接触しない場合、または相手が自分のグループとは接触しない場合
}

bool ObjectData::isCollideWithObjectGroup(int group)const
{
	return getCollideWithObjectGroupBit() & (1 << group);
}

#if defined(AGTK_DEBUG)
void ObjectData::dump()
{
	CCLOG("id:%d", this->getId());
	CCLOG("name:%s", this->getName());
	CCLOG("folder:%d", this->getFolder());
	if (this->getFolder() && this->getChildren()) {
		auto keys = this->getChildren()->allKeys();
		cocos2d::Ref *ref = nullptr;
		CCARRAY_FOREACH(keys, ref) {
			auto id = dynamic_cast<cocos2d::Integer *>(ref);
			auto data = dynamic_cast<ObjectData *>(this->getChildren()->objectForKey(id->getValue()));
			data->dump();
		}
		return;
	}
	CCLOG("group:%d", this->getGroup());
	CCLOG("animationId:%d", this->getAnimationId());
	CCLOG("operatable:%d", this->getOperatable());
	CCLOG("priority:%d", this->getPriority());
	CCLOG("dispPriority:%d", this->getDispPriority());
	CCLOG("initialActionId:%d", this->getInitialActionId());
	CCLOG("hitObjectGroupBit:%d", this->getHitObjectGroupBit());
	CCLOG("collideWithObjectGroupBit:%d", this->getCollideWithObjectGroupBit());
	CCLOG("collideWithTileGroupBit:%d", this->getCollideWithTileGroupBit());
	CCLOG("initialDisplayDirectionFlag:%d", this->getInitialDisplayDirectionFlag());
	CCLOG("initialDisplayDirection:%f", this->getInitialDisplayDirection());
	cocos2d::DictElement *el = nullptr;
	auto actionList = this->getActionList();
	CCDICT_FOREACH(actionList, el) {
		auto p = dynamic_cast<ObjectActionData *>(el->getObject());
		p->dump();
	}
	auto actionLinkList = this->getActionLinkList();
	CCDICT_FOREACH(actionLinkList, el) {
		auto p = dynamic_cast<ObjectActionLinkData *>(el->getObject());
		p->dump();
	}
	auto actionGroupList = this->getActionGroupList();
	CCDICT_FOREACH(actionGroupList, el) {
		auto p = dynamic_cast<ObjectActionGroupData *>(el->getObject());
		p->dump();
	}
	CCLOG("memo:%s", this->getMemo());
	CCLOG("testplayOnly:%d", this->getTestplayOnly());
	CCLOG("appearCondition:%d", this->getAppearCondition());
	CCLOG("appearConditionTileCount:%d", this->getAppearConditionTileCount());
	CCLOG("reappearCondition:%d", this->getReappearCondition());
	CCLOG("reappearConditionTileCount:%d", this->getReappearConditionTileCount());
	CCLOG("hp:%d", this->getHp());
	CCLOG("maxHp:%d", this->getMaxHp());
	CCLOG("minAttack:%d", this->getMinAttack());
	CCLOG("maxAttack:%d", this->getMaxAttack());
	CCLOG("initialDamageRate:%d", this->getInitialDamageRate());
	CCLOG("damageVariationValue:%d", this->getDamageVariationValue());
	CCLOG("critical:%d", this->getCritical());
	CCLOG("criticalIncidence:%f", this->getCriticalIncidence());
	CCLOG("criticalRatio:%f", this->getCriticalRatio());
	CCLOG("moveType:%d", this->getMoveType());
	CCLOG("normalAccelMove:%d", this->getNormalAccelMove());
	CCLOG("tankAccelMove:%d", this->getTankAccelMove());
	CCLOG("carAccelMove:%d", this->getCarAccelMove());
	CCLOG("slipOnSlope:%d", this->getSlipOnSlope());
	CCLOG("diagonalMoveWithPolar:%d", this->getDiagonalMoveWithPolar());
	CCLOG("moveAroundWithinDots:%d", this->getMoveAroundWithinDots());
	CCLOG("dotsToMoveAround:%d", this->getDotsToMoveAround());
	CCLOG("upMoveKeyId:%d", this->getUpMoveKeyId());
	CCLOG("downMoveKeyId:%d", this->getDownMoveKeyId());
	CCLOG("leftMoveKeyId:%d", this->getLeftMoveKeyId());
	CCLOG("rightMoveKeyId:%d", this->getRightMoveKeyId());
	CCLOG("forwardMoveKeyId:%d", this->getForwardMoveKeyId());
	CCLOG("backwardMoveKeyId:%d", this->getBackwardMoveKeyId());
	CCLOG("leftTurnKeyId:%d", this->getLeftTurnKeyId());
	CCLOG("rightTurnKeyId:%d", this->getRightTurnKeyId());
	CCLOG("horizontalMove:%f", this->getHorizontalMove());
	CCLOG("verticalMove:%f", this->getVerticalMove());
	CCLOG("horizontalAccel:%f", this->getHorizontalAccel());
	CCLOG("horizontalMaxMove:%f", this->getHorizontalMaxMove());
	CCLOG("horizontalDecel:%f", this->getHorizontalDecel());
	CCLOG("verticalAccel:%f", this->getVerticalAccel());
	CCLOG("verticalMaxMove:%f", this->getVerticalMaxMove());
	CCLOG("verticalDecel:%f", this->getVerticalDecel());
	CCLOG("forwardMove:%f", this->getForwardMove());
	CCLOG("backwardMove:%f", this->getBackwardMove());
	CCLOG("forwardAccel:%f", this->getForwardAccel());
	CCLOG("forwardMaxMove:%f", this->getForwardMaxMove());
	CCLOG("forwardDecel:%f", this->getForwardDecel());
	CCLOG("backwardAccel:%f", this->getBackwardAccel());
	CCLOG("backwardMaxMove:%f", this->getBackwardMaxMove());
	CCLOG("backwardDecel:%f", this->getBackwardDecel());
	CCLOG("leftTurn:%f", this->getLeftTurn());
	CCLOG("rightTurn:%f", this->getRightTurn());
	CCLOG("jumpInitialSpeed:%d", this->getJumpInitialSpeed());
	CCLOG("gravity:%d", this->getGravity());
	CCLOG("movableWhenJumping:%d", this->getMovableWhenJumping());
	CCLOG("movableWhenFalling:%d", this->getMovableWhenFalling());
	CCLOG("jumpInputOperationKeyId:%d", this->getJumpInputOperationKeyId());
	CCLOG("affectInputDuration300:%d", this->getAffectInputDuration300());
	CCLOG("invincibleOnDamaged:%d", this->getInvincibleOnDamaged());
	CCLOG("invincibleDuration300:%d", this->getInvincibleDuration300());
	CCLOG("winkWhenInvincible:%d", this->getWinkWhenInvincible());
	CCLOG("winkInterval300:%d", this->getWinkInterval300());
	CCLOG("afterimage:%d", this->getAfterimage());
	CCLOG("afterimageCount:%d", this->getAfterimageCount());
	CCLOG("afterimageInterval:%f", this->getAfterimageInterval());
	CCLOG("afterimageDuration300:%d", this->getAfterimageDuration300());
	CCLOG("fillAfterimage:%d", this->getFillAfterimage());
	CCLOG("fillAfterimageA:%d", this->getFillAfterimageA());
	CCLOG("fillAfterimageR:%d", this->getFillAfterimageR());
	CCLOG("fillAfterimageG:%d", this->getFillAfterimageG());
	CCLOG("fillAfterimageB:%d", this->getFillAfterimageB());
	CCLOG("fadeoutAfterimage:%d", this->getFadeoutAfterimage());
	CCLOG("fadeoutAfterimageStart300:%d", this->getFadeoutAfterimageStart300());
	CCLOG("useAnimationAfterimage:%d", this->getUseAnimationAfterimage());
	CCLOG("afterimageAnimationId:%d", this->getAfterimageAnimationId());
	CCLOG("followType:%d", this->getFollowType());
	CCLOG("followPrecision:%d", this->getFollowPrecision());
	CCLOG("followIntervalByTime300:%d", this->getFollowIntervalByTime300());
	CCLOG("followIntervalByLocusLength:%d", this->getFollowIntervalByLocusLength());
	CCLOG("childDamageType:%d", this->getChildDamageType());
	CCLOG("takeOverDamageRateToParent:%d", this->getTakeOverDamageRateToParent());
	CCLOG("unattackableToParent:%d", this->getUnattackableToParent());
	CCLOG("takeoverDispDirection:%d", this->getTakeoverDispDirection());
	CCLOG("takeoverAngle:%d", this->getTakeoverAngle());
	CCLOG("takeoverScaling:%d", this->getTakeoverScaling());
	CCLOG("takeoverIntensity:%d", this->getTakeoverIntensity());
	CCLOG("disappearWhenHp0:%d", this->getDisappearWhenHp0());
	CCLOG("fixedInCamera:%d", this->getFixedInCamera());
	CCLOG("pushedbackByObject:%d", this->getPushedbackByObject());
	CCLOG("takeoverStatesAtSceneEnd:%d", this->getTakeoverStatesAtSceneEnd());
	CCLOG("disappearSettingFlag:%d", this->getDisappearSettingFlag());
	this->getDisappearSetting()->dump();
	CCLOG("fireBulletSettingFlag:%d", this->getFireBulletSettingFlag());
	auto fireBulletSettingList = this->getFireBulletSettingList();
	CCDICT_FOREACH(fireBulletSettingList, el) {
		auto p = dynamic_cast<ObjectFireBulletSettingData *>(el->getObject());
		p->dump();
	}
	CCLOG("effectSettingFlag:%d", this->getEffectSettingFlag());
	auto effectSettingList = this->getEffectSettingList();
	CCDICT_FOREACH(effectSettingList, el) {
		auto p = dynamic_cast<ObjectEffectSettingData *>(el->getObject());
		p->dump();
	}
	CCLOG("connectSettingFlag:%d", this->getConnectSettingFlag());
	auto connectSettingList = this->getConnectSettingList();
	CCDICT_FOREACH(connectSettingList, el) {
		auto p = dynamic_cast<ObjectConnectSettingData *>(el->getObject());
		p->dump();
	}
	CCLOG("moveDispDirectionSettingFlag:%d", this->getMoveDispDirectionSettingFlag());
	this->getMoveDispDirectionSetting()->dump();
	CCLOG("physicsSettingFlag:%d", this->getPhysicsSettingFlag());
	this->getPhysicsSetting()->dump();
	CCLOG("viewportLightSettingFlag:%d", this->getViewportLightSettingFlag());
	auto viewportLightSettingList = this->getViewportLightSettingList();
	CCDICT_FOREACH(viewportLightSettingList, el) {
		auto p = dynamic_cast<ObjectViewportLightSettingData *>(el->getObject());
		p->dump();
	}
	CCLOG("commonActionSettingFlag:%d", this->getCommonActionSettingFlag());
	auto commonActionSettingList = this->getCommonActionSettingList();
	CCDICT_FOREACH(commonActionSettingList, el) {
		auto p = dynamic_cast<ObjectCommonActionSettingData *>(el->getObject());
		p->dump();
	}
	CCLOG("commonActionGroup");
	this->getCommonActionGroup()->dump();
	CCLOG("aroundCharacterViewSettingFlag:%d", this->getAroundCharacterViewSettingFlag());
	this->getAroundCharacterViewSetting()->dump();
	CCLOG("invincibleSettingFlag:%d", this->getInvincibleSettingFlag());
	auto invincibleSettingList = this->getInvincibleSettingList();
	CCDICT_FOREACH(invincibleSettingList, el) {
		auto p = dynamic_cast<ObjectInvincibleSettingData *>(el->getObject());
		p->dump();
	}
	CCLOG("damagedSettingFlag:%d", this->getDamagedSettingFlag());
	auto damagedSettingList = this->getDamagedSettingList();
	CCDICT_FOREACH(damagedSettingList, el) {
		auto p = dynamic_cast<ObjectDamagedSettingData *>(el->getObject());
		p->dump();
	}
	CCLOG("moveRestrictionSettingFlag:%d", this->getMoveRestrictionSettingFlag());
	auto moveRestrictionSettingList = this->getMoveRestrictionSettingList();
	CCDICT_FOREACH(moveRestrictionSettingList, el) {
		auto p = dynamic_cast<ObjectMoveRestrictionSettingData *>(el->getObject());
		p->dump();
	}
	CCLOG("variableList");
	auto variableList = this->getVariableList();
	CCDICT_FOREACH(variableList, el) {
		auto p = dynamic_cast<VariableData *>(el->getObject());
		p->dump();
	}
	CCLOG("switchList");
	auto switchList = this->getSwitchList();
	CCDICT_FOREACH(switchList, el) {
		auto p = dynamic_cast<SwitchData *>(el->getObject());
		p->dump();
	}
	CCLOG("physicsPartList");
	auto physicsPartList = this->getPhysicsPartList();
	CCDICT_FOREACH(physicsPartList, el) {
		auto p = dynamic_cast<PhysicsPartData *>(el->getObject());
		p->dump();
	}
	CCLOG("priorityInPhysics:%d", this->getPriorityInPhysics());

	CCLOG("databasId:%d", this->getDatabaseId());
}
#endif

NS_DATA_END
NS_AGTK_END
