/**
 * @brief プロジェクトデータ
 */
#include "ProjectData.h"
#include "GameManager.h"
#include "AudioManager.h"

NS_AGTK_BEGIN
NS_DATA_BEGIN

//-------------------------------------------------------------------------------------------------------------------
//! BaseSceneChangeEffectData クラス
/**
* コンストラクタ
*/
BaseSceneChangeEffectData::BaseSceneChangeEffectData()
{
	_id = -1;
	_preMoveEffect = EnumMoveEffect::kMoveEffectMax;
	_preMoveDuration300 = 0;
	_preMoveBgmChangeType = EnumBgmChangeType::kBgmChangeTypeMax;
	_preMoveBgmFadeout = false;
	_preMoveBgmId = MOVE_BGM_ID_NONE;
	_preMoveBgmLoop = false;
	_preMoveBgmChangeTiming = EnumBgmChangeTiming::kBgmChangeTimingMax;
	_preMovePlaySe = false;
	_preMoveSeId = MOVE_SE_ID_NONE;

	_postMoveEffect = EnumMoveEffect::kMoveEffectMax;
	_postMoveDuration300 = 0;
	_postMoveBgmChangeType = EnumBgmChangeType::kBgmChangeTypeMax;
	_postMoveBgmChangeTiming = EnumBgmChangeTiming::kBgmChangeTimingMax;
	_postMoveBgmFadeout = false;
	_postMoveBgmId = MOVE_BGM_ID_NONE;
	_postMoveBgmLoop = false;
	_postMovePlaySe = false;
	_postMoveSeId = MOVE_SE_ID_NONE;

	_postMoveChangeSwitchVariable = false;
	_postMoveSwitchVariableAssignList = nullptr;
	_postMoveExecuteObjectAction = false;
	_postMoveObjectActionList = nullptr;
}

/**
* デストラクタ
*/
BaseSceneChangeEffectData::~BaseSceneChangeEffectData()
{
	CC_SAFE_RELEASE_NULL(_postMoveObjectActionList);
	CC_SAFE_RELEASE_NULL(_postMoveSwitchVariableAssignList);
}

/**
* 初期化
* @param	json	初期化用JSONデータ
* @return			初期化の成否
*/
bool BaseSceneChangeEffectData::init(const rapidjson::Value& json, int id)
{
	_id = id;
	ASSIGN_JSON_MEMBER_WITH_CAST("preMoveEffect", PreMoveEffect, Int, EnumMoveEffect);
	ASSIGN_JSON_MEMBER("preMoveDuration300", PreMoveDuration300, Int);
	ASSIGN_JSON_MEMBER_WITH_CAST("preMoveBgmChangeType", PreMoveBgmChangeType, Int, EnumBgmChangeType);
	ASSIGN_JSON_MEMBER("preMoveBgmFadeout", PreMoveBgmFadeout, Bool);
	ASSIGN_JSON_MEMBER("preMoveBgmId", PreMoveBgmId, Int);
	ASSIGN_JSON_MEMBER("preMoveBgmLoop", PreMoveBgmLoop, Bool);
	ASSIGN_JSON_MEMBER_WITH_CAST("preMoveBgmChangeTiming", PreMoveBgmChangeTiming, Int, EnumBgmChangeTiming);
	ASSIGN_JSON_MEMBER("preMovePlaySe", PreMovePlaySe, Bool);
	ASSIGN_JSON_MEMBER("preMoveSeId", PreMoveSeId, Int);

	ASSIGN_JSON_MEMBER_WITH_CAST("postMoveEffect", PostMoveEffect, Int, EnumMoveEffect);
	ASSIGN_JSON_MEMBER("postMoveDuration300", PostMoveDuration300, Int);
	ASSIGN_JSON_MEMBER_WITH_CAST("postMoveBgmChangeType", PostMoveBgmChangeType, Int, EnumBgmChangeType);
	ASSIGN_JSON_MEMBER_WITH_CAST("postMoveBgmChangeTiming", PostMoveBgmChangeTiming, Int, EnumBgmChangeTiming);
	ASSIGN_JSON_MEMBER("postMoveBgmFadeout", PostMoveBgmFadeout, Bool);
	ASSIGN_JSON_MEMBER("postMoveBgmId", PostMoveBgmId, Int);
	ASSIGN_JSON_MEMBER("postMoveBgmLoop", PostMoveBgmLoop, Bool);
	ASSIGN_JSON_MEMBER("postMovePlaySe", PostMovePlaySe, Bool);
	ASSIGN_JSON_MEMBER("postMoveSeId", PostMoveSeId, Int);

	ASSIGN_JSON_MEMBER("postMoveChangeSwitchVariable", PostMoveChangeSwitchVariable, Bool);
	auto arr1 = cocos2d::__Array::create();
	for (rapidjson::SizeType i = 0; i < json["postMoveSwitchVariableAssignList"].Size(); i++) {
		auto p = SwitchVariableAssignData::create(json["postMoveSwitchVariableAssignList"][i]);
		CC_ASSERT(p);
		arr1->addObject(p);
	}
	this->setPostMoveSwitchVariableAssignList(arr1);

	ASSIGN_JSON_MEMBER("postMoveExecuteObjectAction", PostMoveExecuteObjectAction, Bool);
	auto arr2 = cocos2d::__Array::create();
	for (rapidjson::SizeType i = 0; i < json["postMoveObjectActionList"].Size(); i++) {
		auto p = ObjectActionExcecData::create(json["postMoveObjectActionList"][i]);
		CC_ASSERT(p);
		arr2->addObject(p);
	}
	this->setPostMoveObjectActionList(arr2);

	return true;
}

#if defined(AGTK_DEBUG)
/**
* ダンプ
*/
void BaseSceneChangeEffectData::dump()
{
	CCLOG("preMoveEffect: %d", this->getPreMoveEffect());
	CCLOG("preMoveDuration300: %d", this->getPreMoveDuration300());
	CCLOG("preMoveBgmChangeType: %d", this->getPreMoveBgmChangeType());
	CCLOG("preMoveBgmFadeout: %s", DUMP_BOOLTEXT(this->getPreMoveBgmFadeout()));
	CCLOG("preMoveBgmId: %d", this->getPreMoveBgmId());
	CCLOG("preMoveBgmLoop: %s", DUMP_BOOLTEXT(this->getPreMoveBgmLoop()));
	CCLOG("preMoveBgmChangeTiming: %d", this->getPreMoveBgmChangeTiming());
	CCLOG("preMovePlaySe: %s", DUMP_BOOLTEXT(this->getPreMovePlaySe()));
	CCLOG("preMoveSeId: %d", this->getPreMoveSeId());

	CCLOG("postMoveEffect: %d", this->getPostMoveEffect());
	CCLOG("postMoveDuration300: %d", this->getPostMoveDuration300());
	CCLOG("postMoveBgmChangeType: %d", this->getPostMoveBgmChangeType());
	CCLOG("postMoveBgmChangeTiming: %d", this->getPostMoveBgmChangeTiming());
	CCLOG("postMoveBgmFadeout: %s", DUMP_BOOLTEXT(this->getPostMoveBgmFadeout()));
	CCLOG("postMoveBgmId: %d", this->getPostMoveBgmId());
	CCLOG("postMoveBgmLoop: %s", DUMP_BOOLTEXT(this->getPostMoveBgmLoop()));
	CCLOG("postMovePlaySe: %s", DUMP_BOOLTEXT(this->getPostMovePlaySe()));
	CCLOG("postMoveSeId: %d", this->getPostMoveSeId());

	CCLOG("postMoveChangeSwitchVariable: %s", DUMP_BOOLTEXT(this->getPostMoveChangeSwitchVariable()));
	CCLOG("postMoveSwitchVariableAssignList: [");
	cocos2d::Ref *ref = nullptr;
	CCARRAY_FOREACH(this->getPostMoveSwitchVariableAssignList(), ref) {
		auto p = dynamic_cast<SwitchVariableAssignData *>(ref);
		CC_ASSERT(p);
		p->dump();
	}
	CCLOG("]");
	CCLOG("postMoveExecuteObjectAction: %s", DUMP_BOOLTEXT(this->getPostMoveExecuteObjectAction()));
	CCLOG("postMoveObjectActionList: [");
	CCARRAY_FOREACH(this->getPostMoveObjectActionList(), ref) {
		auto p = dynamic_cast<ObjectActionExcecData *>(ref);
		CC_ASSERT(p);
		p->dump();
	}
	CCLOG("]");
}
#endif

//-------------------------------------------------------------------------------------------------------------------
//! MoveSettingData クラス
/**
 * コンストラクタ
 */
MoveSettingData::MoveSettingData() : BaseSceneChangeEffectData()
{
	_preMoveConditionType = EnumConditionType::kConditionTypeAllPlayersTouched;
	_preMoveConditionDuration300 = 0;
	_preMoveSwitchVariableCondition = false;
	_preMoveSwitchVariableConditionList = nullptr;

	_preMoveInvalidateTouchedPlayer = false;
	_preMoveKeepDisplay = false;
	_preMoveChangeSwitchVariable = false;
	_preMoveSwitchVariableAssignList = nullptr;

	_preMoveKeyInput = false;
	_preMoveOperationKeyId = 0;
	_preMoveTouchedSwitchVariableCondition = false;
	_preMoveTouchedSwitchVariableConditionList = nullptr;


	_postMoveOnlyTransportPlayersTouching = false;
	_postMoveShowPlayersInTouchedOrder = false;
	_postMoveShowDuration300 = 0;
	_abId = -1;
}

/**
 * デストラクタ
 */
MoveSettingData::~MoveSettingData()
{
	CC_SAFE_RELEASE_NULL(_preMoveTouchedSwitchVariableConditionList);
	CC_SAFE_RELEASE_NULL(_preMoveSwitchVariableAssignList);
	CC_SAFE_RELEASE_NULL(_preMoveSwitchVariableConditionList);
}

/**
 * 初期化
 * @param	json	初期化用JSONデータ
 * @return			初期化の成否
 */
bool MoveSettingData::init(const rapidjson::Value& json, int id, int abId)
{

	if (!BaseSceneChangeEffectData::init(json, id)) {
		return false;
	}

	_abId = abId;
	ASSIGN_JSON_MEMBER_WITH_CAST("preMoveConditionType", PreMoveConditionType, Int, EnumConditionType);
	ASSIGN_JSON_MEMBER("preMoveConditionDuration300", PreMoveConditionDuration300, Int);
	ASSIGN_JSON_MEMBER("preMoveSwitchVariableCondition", PreMoveSwitchVariableCondition, Bool);
	auto arr1 = cocos2d::__Array::create();
	for (rapidjson::SizeType i = 0; i < json["preMoveSwitchVariableConditionList"].Size(); i++) {
		auto p = SwitchVariableConditionData::create(json["preMoveSwitchVariableConditionList"][i]);
		CC_ASSERT(p);
		arr1->addObject(p);
	}
	this->setPreMoveSwitchVariableConditionList(arr1);

	ASSIGN_JSON_MEMBER("preMoveInvalidateTouchedPlayer", PreMoveInvalidateTouchedPlayer, Bool);
	ASSIGN_JSON_MEMBER("preMoveKeepDisplay", PreMoveKeepDisplay, Bool);
	ASSIGN_JSON_MEMBER("preMoveChangeSwitchVariable", PreMoveChangeSwitchVariable, Bool);
	auto arr2 = cocos2d::__Array::create();
	for (rapidjson::SizeType i = 0; i < json["preMoveSwitchVariableAssignList"].Size(); i++) {
		auto p = SwitchVariableAssignData::create(json["preMoveSwitchVariableAssignList"][i]);
		CC_ASSERT(p);
		arr2->addObject(p);
	}
	this->setPreMoveSwitchVariableAssignList(arr2);

	ASSIGN_JSON_MEMBER("preMoveKeyInput", PreMoveKeyInput, Bool);
	ASSIGN_JSON_MEMBER("preMoveOperationKeyId", PreMoveOperationKeyId, Int);
	ASSIGN_JSON_MEMBER("preMoveTouchedSwitchVariableCondition", PreMoveTouchedSwitchVariableCondition, Bool);
	auto arr3 = cocos2d::__Array::create();
	for (rapidjson::SizeType i = 0; i < json["preMoveTouchedSwitchVariableConditionList"].Size(); i++) {
		auto p = SwitchVariableConditionData::create(json["preMoveTouchedSwitchVariableConditionList"][i]);
		CC_ASSERT(p);
		arr3->addObject(p);
	}
	this->setPreMoveTouchedSwitchVariableConditionList(arr3);

	if (json.HasMember("postMoveOnlyTransportPlayersTouching")) {
		ASSIGN_JSON_MEMBER("postMoveOnlyTransportPlayersTouching", PostMoveOnlyTransportPlayersTouching, Bool);
	}
	ASSIGN_JSON_MEMBER("postMoveShowPlayersInTouchedOrder", PostMoveShowPlayersInTouchedOrder, Bool);
	ASSIGN_JSON_MEMBER("postMoveShowDuration300", PostMoveShowDuration300, Int);

	return true;
}

#if defined(AGTK_DEBUG)
/**
* ダンプ
*/
void MoveSettingData::dump()
{
	BaseSceneChangeEffectData::dump();

	CCLOG("preMoveConditionType: %d", this->getPreMoveConditionType());
	CCLOG("preMoveConditionDuration300: %d", this->getPreMoveConditionDuration300());
	CCLOG("preMoveSwitchVariableCondition: %s", DUMP_BOOLTEXT(this->getPreMoveSwitchVariableCondition()));
	cocos2d::Ref *ref = nullptr;
	CCARRAY_FOREACH(this->getPreMoveSwitchVariableConditionList(), ref) {
		auto p = dynamic_cast<SwitchVariableConditionData *>(ref);
		CC_ASSERT(p);
		p->dump();
	}
	CCLOG("preMoveInvalidateTouchedPlayer: %s", DUMP_BOOLTEXT(this->getPreMoveInvalidateTouchedPlayer()));
	CCLOG("preMoveKeepDisplay: %s", DUMP_BOOLTEXT(this->getPreMoveKeepDisplay()));
	CCLOG("preMoveChangeSwitchVariable: %s", DUMP_BOOLTEXT(this->getPreMoveChangeSwitchVariable()));
	CCLOG("preMoveSwitchVariableAssignList: [");
	CCARRAY_FOREACH(this->getPreMoveSwitchVariableAssignList(), ref) {
		auto p = dynamic_cast<SwitchVariableAssignData *>(ref);
		CC_ASSERT(p);
		p->dump();
	}
	CCLOG("]");
	CCLOG("preMoveKeyInput: %s", DUMP_BOOLTEXT(this->getPreMoveKeyInput()));
	CCLOG("preMoveOperationKeyId: %d", this->getPreMoveOperationKeyId());
	CCLOG("preMoveTouchedSwitchVariableCondition: %s", DUMP_BOOLTEXT(this->getPreMoveTouchedSwitchVariableCondition()));
	CCLOG("preMoveTouchedSwitchVariableConditionList: [");
	CCARRAY_FOREACH(this->getPreMoveTouchedSwitchVariableConditionList(), ref) {
		auto p = dynamic_cast<SwitchVariableConditionData *>(ref);
		CC_ASSERT(p);
		p->dump();
	}
	CCLOG("]");

	CCLOG("postMoveOnlyTransportPlayersTouching: %s", DUMP_BOOLTEXT(this->getPostMoveOnlyTransportPlayersTouching()));
	CCLOG("postMoveShowPlayersInTouchedOrder: %s", DUMP_BOOLTEXT(this->getPostMoveShowPlayersInTouchedOrder()));
	CCLOG("postMoveShowDuration300: %d", this->getPostMoveShowDuration300());
}
#endif

//-------------------------------------------------------------------------------------------------------------------
//! AreaSettingData クラス
/**
 * コンストラクタ
 */
AreaSettingData::AreaSettingData()
{
	_x = 0;
	_y = 0;
	_width = 0;
	_height = 0;
	_sceneId = 0;
	_layerIndex = 0;
	_keepHorzPosition = false;
	_keepVertPosition = false;
	_rePortalDirectionBit = 0;
}

/**
 * デストラクタ
 */
AreaSettingData::~AreaSettingData()
{

}

/**
* 初期化
* @param	json	初期化用JSONデータ
* @return			初期化の成否
*/
bool AreaSettingData::init(const rapidjson::Value& json)
{
	ASSIGN_JSON_MEMBER("sceneId", SceneId, Int);
	ASSIGN_JSON_MEMBER("layerIndex", LayerIndex, Int);
	ASSIGN_JSON_MEMBER("x", X, Int);
	ASSIGN_JSON_MEMBER("y", Y, Int);
	ASSIGN_JSON_MEMBER("width", Width, Int);
	ASSIGN_JSON_MEMBER("height", Height, Int);
	ASSIGN_JSON_MEMBER("keepHorzPosition", KeepHorzPosition, Bool);
	ASSIGN_JSON_MEMBER("keepVertPosition", KeepVertPosition, Bool);
	ASSIGN_JSON_MEMBER("rePortalDirectionBit", RePortalDirectionBit, Int);

	return true;
}

#if defined(AGTK_DEBUG)
/**
* ダンプ
*/
void AreaSettingData::dump()
{
	CCLOG("sceneId:%d", this->getSceneId());
	CCLOG("layerIndex:%d", this->getLayerIndex());
	CCLOG("x:%d", this->getX());
	CCLOG("y:%d", this->getY());
	CCLOG("width:%d", this->getWidth());
	CCLOG("height:%d", this->getHeight());
	CCLOG("keepHorzPosition:%s", DUMP_BOOLTEXT(this->getKeepHorzPosition()));
	CCLOG("keepVertPosition:%s", DUMP_BOOLTEXT(this->getKeepVertPosition()));
	CCLOG("rePortalDirectionBit:%d", this->getRePortalDirectionBit());
}
#endif
//-------------------------------------------------------------------------------------------------------------------
//! TransitionPortalData クラス
/**
 * コンストラクタ
 */
TransitionPortalData::TransitionPortalData()
{
	_id = 0;
	_name = nullptr;
	_reverseMoveSettingSame = false;
	_areaSettingList = nullptr;
	_movableList = nullptr;
	_moveSettingList = nullptr;
	_children = nullptr;
}

/**
 * デストラクタ
 */
TransitionPortalData::~TransitionPortalData()
{
	CC_SAFE_RELEASE_NULL(_children);
	CC_SAFE_RELEASE_NULL(_moveSettingList);
	CC_SAFE_RELEASE_NULL(_movableList);
	CC_SAFE_RELEASE_NULL(_areaSettingList);
	CC_SAFE_RELEASE_NULL(_name);
}

/**
 * 初期化
 * @param	json	初期化用JSONデータ
 * @return			初期化の成否
 */
bool TransitionPortalData::init(const rapidjson::Value& json)
{
	ASSIGN_JSON_MEMBER("id", Id, Int);
	ASSIGN_JSON_MEMBER_STR("name", Name, String);
	ASSIGN_JSON_MEMBER("folder", Folder, Bool);

	// フォルダの場合
	if (this->getFolder()) {
		auto arr = cocos2d::__Array::create();
		if (json.HasMember("children")) {
			for (rapidjson::SizeType i = 0; i < json["children"].Size(); i++) {
				auto p = TransitionPortalData::create(json["children"][i]);
				arr->addObject(p);
			}
		}
		this->setChildren(arr);
		return true;
	}

	ASSIGN_JSON_MEMBER("reverseMoveSettingSame", ReverseMoveSettingSame, Bool);

	CHECK_JSON_MENBER("areaSettingList");
	auto arr1 = cocos2d::__Array::create();
	for (rapidjson::SizeType i = 0; i < json["areaSettingList"].Size(); i++) {
		arr1->addObject(AreaSettingData::create(json["areaSettingList"][i]));
	}
	this->setAreaSettingList(arr1);

	CHECK_JSON_MENBER("movableList");
	auto arr2 = cocos2d::__Array::create();
	for (rapidjson::SizeType i = 0; i < json["movableList"].Size(); i++) {
		arr2->addObject(cocos2d::Bool::create(json["movableList"][i].GetBool()));
	}
	this->setMovableList(arr2);

	CHECK_JSON_MENBER("moveSettingList");
	auto arr3 = cocos2d::__Array::create();
	for (rapidjson::SizeType i = 0; i < json["moveSettingList"].Size(); i++) {
		arr3->addObject(MoveSettingData::create(json["moveSettingList"][i], _id, i));
	}
	this->setMoveSettingList(arr3);

	return true;
}

#if defined(AGTK_DEBUG)
/**
* ダンプ
*/
void TransitionPortalData::dump()
{
	CCLOG("id:%d", this->getId());
	CCLOG("name:%s", this->getName()->getCString());
	CCLOG("folder:%s", DUMP_BOOLTEXT(this->getFolder()));

	if (this->getFolder()) {
		CCLOG("children[ ");
		cocos2d::Ref *ref = nullptr;
		CCARRAY_FOREACH(this->getChildren(), ref) {
			auto p = dynamic_cast<TransitionPortalData *>(ref);
			p->dump();
		}
		CCLOG("]");
		return;
	}

	CCLOG("reverseMoveSettingSame:%s", DUMP_BOOLTEXT(this->getReverseMoveSettingSame()));

	CCLOG("areaSettingList:[");
	cocos2d::Ref *ref = nullptr;
	CCARRAY_FOREACH(this->getAreaSettingList(), ref) {
		auto p = dynamic_cast<AreaSettingData *>(ref);
		CC_ASSERT(p);
		p->dump();
	}
	CCLOG("]");

	CCLOG("movableList:[");
	ref = nullptr;
	CCARRAY_FOREACH(this->getMovableList(), ref) {
		auto b = dynamic_cast<cocos2d::Bool *>(ref);
		CC_ASSERT(b);
		CCLOG("%s", DUMP_BOOLTEXT(b->getValue()));
	}
	CCLOG("]");

	CCLOG("moveSettingList:[");
	ref = nullptr;
	CCARRAY_FOREACH(this->getMoveSettingList(), ref) {
		auto p = dynamic_cast<MoveSettingData *>(ref);
		CC_ASSERT(p);
		p->dump();
	}
	CCLOG("]");
}
#endif


//-------------------------------------------------------------------------------------------------------------------
//! SwitchVariableConditionData クラス
/**
 * コンストラクタ
 */
SwitchVariableConditionData::SwitchVariableConditionData()
{
	_compareOperator = EnumCompareOperator::kCompareEqual;
	_compareValueType = EnumCompareValueType::kCompareValue;
	_comparedValue = 0;
	_comparedVariableId = 0;
	_comparedVariableObjectId = 0;
	_comparedVariableQualifierId = 0;
	_switchId = 0;
	_switchObjectId = 0;
	_switchQualifierId = 0;
	_switchValue = EnumSwitchValueType::kSwitchConditionOn;
	_swtch = false;
	_variableId = 0;
	_variableObjectId = 0;
	_variableQualifierId = 0;
}

/**
 * デストラクタ
 */
SwitchVariableConditionData::~SwitchVariableConditionData()
{

}

/**
* 初期化
* @param	json	初期化用JSONデータ
* @return			初期化の成否
*/
bool SwitchVariableConditionData::init(const rapidjson::Value& json)
{
	ASSIGN_JSON_MEMBER_WITH_CAST("compareOperator", CompareOperator, Int, EnumCompareOperator);
	ASSIGN_JSON_MEMBER_WITH_CAST("compareValueType", CompareValueType, Int, EnumCompareValueType);
	ASSIGN_JSON_MEMBER("comparedValue", ComparedValue, Double);
	ASSIGN_JSON_MEMBER("comparedVariableId", ComparedVariableId, Int);
	ASSIGN_JSON_MEMBER("comparedVariableObjectId", ComparedVariableObjectId, Int);
	ASSIGN_JSON_MEMBER("comparedVariableQualifierId", ComparedVariableQualifierId, Int);
	ASSIGN_JSON_MEMBER("switchId", SwitchId, Int);
	ASSIGN_JSON_MEMBER("switchObjectId", SwitchObjectId, Int);
	ASSIGN_JSON_MEMBER("switchQualifierId", SwitchQualifierId, Int);
	ASSIGN_JSON_MEMBER_WITH_CAST("switchValue", SwitchValue, Int, EnumSwitchValueType);
	ASSIGN_JSON_MEMBER("swtch", Swtch, Bool);
	ASSIGN_JSON_MEMBER("variableId", VariableId, Int);
	ASSIGN_JSON_MEMBER("variableObjectId", VariableObjectId, Int);
	ASSIGN_JSON_MEMBER("variableQualifierId", VariableQualifierId, Int);

	return true;
}

#if defined(AGTK_DEBUG)
/**
* ダンプ
*/
void SwitchVariableConditionData::dump()
{
	CCLOG("compareOperator:%d", this->getCompareOperator());
	CCLOG("compareValueType:%d", this->getCompareValueType());
	CCLOG("comparedValue:%d", this->getComparedValue());
	CCLOG("comparedVariableId:%d", this->getComparedVariableId());
	CCLOG("comparedVariableObjectId:%d", this->getComparedVariableObjectId());
	CCLOG("comparedVariableQualifierId:%d", this->getComparedVariableQualifierId());
	CCLOG("switchId:%d", this->getSwitchId());
	CCLOG("switchObjectId:%d", this->getSwitchObjectId());
	CCLOG("switchQualifierId:%d", this->getSwitchQualifierId());
	CCLOG("switchValue:%d", this->getSwitchValue());
	CCLOG("swtch:%s", DUMP_BOOLTEXT(this->getSwtch()));
	CCLOG("variableId:%d", this->getVariableId());
	CCLOG("variableObjectId:%d", this->getVariableObjectId());
	CCLOG("variableQualifierId:%d", this->getVariableQualifierId());
}
#endif

//-------------------------------------------------------------------------------------------------------------------
//! SwitchVariableAssignData クラス
/**
 * コンストラクタ
 */
SwitchVariableAssignData::SwitchVariableAssignData()
{
	_swtch = false;
	_switchId = 0;
	_switchObjectId = 0;
	_switchQualifierId = 0;
	_switchValue = EnumSwitchAssignValue::kSwitchAssignMax;

	_variableId = 0;
	_variableObjectId = 0;
	_variableQualifierId = 0;
	_variableAssignValueType = EnumVariableAssignValueType::kVariableAssignConstant;
	_variableAssignOperator = EnumVariavleAssignValue::kVariableAdd;
	_assignValue = 0;
	_assignVariableId = 0;
	_assignVariableObjectId = 0;
	_assignVariableQualifierId = 0;
	_randomMin = 0;
	_randomMax = 0;
	_assignScript = nullptr;
}

/**
 * デストラクタ
 */
SwitchVariableAssignData::~SwitchVariableAssignData()
{
	CC_SAFE_RELEASE_NULL(_assignScript);
}

/**
 * 初期化
 * @param	json	初期化用JSONデータ
 * @return			初期化の成否
 */
bool SwitchVariableAssignData::init(const rapidjson::Value& json)
{
	ASSIGN_JSON_MEMBER("swtch", Swtch, Bool);
	ASSIGN_JSON_MEMBER("switchId", SwitchId, Int);
	ASSIGN_JSON_MEMBER("switchObjectId", SwitchObjectId, Int);
	ASSIGN_JSON_MEMBER("switchQualifierId", SwitchQualifierId, Int);
	ASSIGN_JSON_MEMBER_WITH_CAST("switchValue", SwitchValue, Int, EnumSwitchAssignValue);

	ASSIGN_JSON_MEMBER("variableId", VariableId, Int);
	ASSIGN_JSON_MEMBER("variableObjectId", VariableObjectId, Int);
	ASSIGN_JSON_MEMBER("variableQualifierId", VariableQualifierId, Int);
	ASSIGN_JSON_MEMBER_WITH_CAST("variableAssignValueType", VariableAssignValueType, Int, EnumVariableAssignValueType);
	ASSIGN_JSON_MEMBER_WITH_CAST("variableAssignOperator", VariableAssignOperator, Int, EnumVariavleAssignValue);
	ASSIGN_JSON_MEMBER("assignValue", AssignValue, Double);
	ASSIGN_JSON_MEMBER("assignVariableId", AssignVariableId, Int);
	ASSIGN_JSON_MEMBER("assignVariableObjectId", AssignVariableObjectId, Int);
	ASSIGN_JSON_MEMBER("assignVariableQualifierId", AssignVariableQualifierId, Int);
	ASSIGN_JSON_MEMBER("randomMin", RandomMin, Int);
	ASSIGN_JSON_MEMBER("randomMax", RandomMax, Int);
	ASSIGN_JSON_MEMBER_STR("assignScript", AssignScript, String);

	return true;
}

const char *SwitchVariableAssignData::getAssignScript()
{
	CC_ASSERT(_assignScript);
	return _assignScript->getCString();
}

#if defined(AGTK_DEBUG)
/**
* ダンプ
*/
void SwitchVariableAssignData::dump()
{
	CCLOG("swtch:%s", DUMP_BOOLTEXT(this->getSwtch()));
	CCLOG("switchId:%d", this->getSwitchId());
	CCLOG("switchObjectId:%d", this->getSwitchObjectId());
	CCLOG("switchQualifierId:%d", this->getSwitchQualifierId());
	CCLOG("switchValue:%d", this->getSwitchValue());
	
	CCLOG("variableId:%d", this->getVariableId());
	CCLOG("variableObjectId:%d", this->getVariableObjectId());
	CCLOG("variableQualifierId:%d", this->getVariableQualifierId());
	CCLOG("variableAssignValueType:%d", this->getVariableAssignValueType());
	CCLOG("variableAssignOperator:%d", this->getVariableAssignOperator());
	CCLOG("assignValue:%d", this->getAssignValue());
	CCLOG("assignVariableId:%d", this->getAssignVariableId());
	CCLOG("assignVariableObjectId:%d", this->getAssignVariableObjectId());
	CCLOG("assignVariableQualifierId:%d", this->getAssignVariableQualifierId());
	CCLOG("randomMin:%d", this->getRandomMin());
	CCLOG("randomMax:%d", this->getRandomMax());
	CCLOG("assignScript:%s", this->getAssignScript());
}
#endif

//-------------------------------------------------------------------------------------------------------------------
//! ObjectActionExcecData クラス
/**
 * コンストラクタ
 */
ObjectActionExcecData::ObjectActionExcecData()
{

}

/**
 * デストラクタ
 */
ObjectActionExcecData::~ObjectActionExcecData()
{

}

/**
* 初期化
* @param	json	初期化用JSONデータ
* @return			初期化の成否
*/
bool ObjectActionExcecData::init(const rapidjson::Value& json)
{
	ASSIGN_JSON_MEMBER("actionBox", ActionBox, Bool);
	ASSIGN_JSON_MEMBER("actionId", ActionId, Int);
	ASSIGN_JSON_MEMBER("commonActionId", CommonActionId, Int);
	ASSIGN_JSON_MEMBER("objectId", ObjectId, Int);
	ASSIGN_JSON_MEMBER("qualifierId", QualifierId, Int);

	return true;
}

#if defined(AGTK_DEBUG)
/**
 * ダンプ
 */
void ObjectActionExcecData::dump()
{
	CCLOG("actionBox:%s", DUMP_BOOLTEXT(this->getActionBox()));
	CCLOG("actionId:%d", this->getActionId());
	CCLOG("commonActionId:%d", this->getCommonActionId());
	CCLOG("objectId:%d", this->getObjectId());
	CCLOG("qulifierId:%d", this->getQualifierId());
}
#endif

//-------------------------------------------------------------------------------------------------------------------
//! FlowLinkInputConditionData クラス
/**
* 初期化
* @param	json	初期化用JSONデータ
* @return			初期化の成否
*/
bool FlowLinkInputConditionData::init(const rapidjson::Value& json)
{
	if (!ObjectInputConditionData::init(json)) {
		return false;
	}

	return true;
}

#if defined(AGTK_DEBUG)
/**
* ダンプ
*/
void FlowLinkInputConditionData::dump()
{
	ObjectInputConditionData::dump();
}
#endif


//-------------------------------------------------------------------------------------------------------------------
//! FlowScreenData クラス
/**
 * コンストラクタ
 */
FlowScreenData::FlowScreenData()
{
	_id = -1;
	_scene = true;
	_sceneMenuId = SCENE_MENU_ID_START;
}

/**
 * デストラクタ
 */
FlowScreenData::~FlowScreenData()
{
}

/**
 * 初期化
 * @param	json	初期化用JSONデータ
 * @return			初期化の成否
 */
bool FlowScreenData::init(const rapidjson::Value& json)
{
	ASSIGN_JSON_MEMBER("id", Id, Int);
	ASSIGN_JSON_MEMBER("scene", Scene, Bool);
	ASSIGN_JSON_MEMBER("sceneMenuId", SceneMenuId, Int);

	return true;
}

#if defined(AGTK_DEBUG)
/**
 * ダンプ
 */
void FlowScreenData::dump()
{
	CCLOG("id:%d", this->getId());
	CCLOG("scene:%d", this->getScene());
	CCLOG("sceneMenuId:%d", this->getSceneMenuId());
}
#endif

//-------------------------------------------------------------------------------------------------------------------
//! FlowLinkData クラス
/**
 * コンストラクタ
 */
FlowLinkData::FlowLinkData() : BaseSceneChangeEffectData()
{
	_priority = 0;
	_screenIdPair = nullptr;
	_startPointGroup = 0;
	_changeConditionType = EnumChangeConditionType::kAllConditionsSatisfied;
	_noInput = false;
	_useInput = false;
	_inputConditionList = nullptr;
	_sceneTerminated = false;
	_timeElapsed = false;
	_timeElapsed300 = 0;
	_switchVariableChanged = false;
	_switchVariableConditionList = nullptr;
	_memo = nullptr;
	_inputConditionGroupList = nullptr;
}

/**
 * デストラクタ
 */
FlowLinkData::~FlowLinkData()
{
	CC_SAFE_RELEASE_NULL(_switchVariableConditionList);
	CC_SAFE_RELEASE_NULL(_memo);
	CC_SAFE_RELEASE_NULL(_inputConditionList);
	CC_SAFE_RELEASE_NULL(_screenIdPair);
	CC_SAFE_RELEASE_NULL(_inputConditionGroupList);
}

/**
* 初期化
* @param	json	初期化用JSONデータ
* @return			初期化の成否
*/
bool FlowLinkData::init(const rapidjson::Value& json)
{
	ASSIGN_JSON_MEMBER("id", Id, Int);
	if (!BaseSceneChangeEffectData::init(json, _id)) {
		return false;
	}

	ASSIGN_JSON_MEMBER("priority", Priority, Int);
	auto arr1 = cocos2d::__Array::create();
	if (json.HasMember("typeIdPair")) {
		for (rapidjson::SizeType i = 0; i < json["typeIdPair"].Size(); i++) {
			arr1->addObject(cocos2d::Integer::create(json["typeIdPair"][i][1].GetInt()));
		}
	}
	else {
		CHECK_JSON_MENBER("screenIdPair");
		for (rapidjson::SizeType i = 0; i < json["screenIdPair"].Size(); i++) {
			arr1->addObject(cocos2d::Integer::create(json["screenIdPair"][i].GetInt()));
		}
	}
	this->setscreenIdPair(arr1);

	ASSIGN_JSON_MEMBER("startPointGroup", StartPointGroup, Int);
	ASSIGN_JSON_MEMBER_WITH_CAST("changeConditionType", ChangeConditionType, Int, EnumChangeConditionType);
	ASSIGN_JSON_MEMBER("noInput", NoInput, Bool);
	ASSIGN_JSON_MEMBER("useInput", UseInput, Bool);
	CHECK_JSON_MENBER("inputConditionList");
	auto dic1 = cocos2d::__Dictionary::create();
	for (rapidjson::SizeType i = 0; i < json["inputConditionList"].Size(); i++) {
		auto data = FlowLinkInputConditionData::create(json["inputConditionList"][i]);
#if defined(AGTK_DEBUG)
		CC_ASSERT(dic1->objectForKey(data->getId()) == nullptr);
#endif
		dic1->setObject(data, data->getId());
	}
	this->setInputConditionList(dic1);
	ASSIGN_JSON_MEMBER("sceneTerminated", SceneTerminated, Bool);
	ASSIGN_JSON_MEMBER("timeElapsed", TimeElapsed, Bool);
	ASSIGN_JSON_MEMBER("timeElapsed300", TimeElapsed300, Int);
	ASSIGN_JSON_MEMBER("switchVariableChanged", SwitchVariableChanged, Bool);
	CHECK_JSON_MENBER("switchVariableConditionList");
	auto arr2 = cocos2d::__Array::create();
	for (rapidjson::SizeType i = 0; i < json["switchVariableConditionList"].Size(); i++) {
		auto data = SwitchVariableConditionData::create(json["switchVariableConditionList"][i]);
		CC_ASSERT(data);
		arr2->addObject(data);
	}
	this->setSwitchVariableConditionList(arr2);
	ASSIGN_JSON_MEMBER_STR("memo", Memo, String);

	if (initConditionGroupList() == false) {
		CC_ASSERT(0);
		return false;
	}

	return true;
}

bool FlowLinkData::initConditionGroupList()
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

	return true;
}

#if defined(AGTK_DEBUG)
/**
* ダンプ
*/
void FlowLinkData::dump()
{
	CCLOG("id:%d", this->getId());
	// 基本パラメータのダンプ
	CCLOG("priority:%d", this->getPriority());
	CCLOG("screenIdPair:[");
	cocos2d::Ref *ref = nullptr;
	CCARRAY_FOREACH(this->getscreenIdPair(), ref) {
		auto p = dynamic_cast<cocos2d::Integer *>(ref);
		CC_ASSERT(p);
		CCLOG("%d", p->getValue());
	}
	CCLOG("]");

	// 切替後の初期位置パラメータのダンプ
	CCLOG("startPointGroup:%d", this->getStartPointGroup());

	// 切替条件パラメータのダンプ
	CCLOG("changeConditionType:%d", this->getChangeConditionType());
	CCLOG("noInput:%s", DUMP_BOOLTEXT(this->getNoInput()));
	CCLOG("useInput:%s", DUMP_BOOLTEXT(this->getUseInput()));
	CCLOG("inputConditionList:");
	auto dicList1 = this->getInputConditionList();
	cocos2d::DictElement *el = nullptr;
	CCDICT_FOREACH(dicList1, el) {
		auto p = dynamic_cast<FlowLinkInputConditionData *>(el->getObject());
		CC_ASSERT(p);
		p->dump();
	}
	CCLOG("sceneTerminated:%s", DUMP_BOOLTEXT(this->getSceneTerminated()));
	CCLOG("timeElapsed:%s", DUMP_BOOLTEXT(this->getTimeElapsed()));
	CCLOG("timeElapsed300:%d", this->getTimeElapsed300());
	CCLOG("switchVariableChanged:%s", DUMP_BOOLTEXT(this->getSwitchVariableChanged()));
	CCLOG("switchVariableConditionList:");
	ref = nullptr;
	CCARRAY_FOREACH(this->getSwitchVariableConditionList(), ref) {
		auto p = dynamic_cast<SwitchVariableConditionData *>(ref);
		CC_ASSERT(p);
		p->dump();
	}
	CCLOG("memo:%s", this->getMemo()->getCString());

	BaseSceneChangeEffectData::dump();
}
#endif


//-------------------------------------------------------------------------------------------------------------------
//! TransitionFlowData クラス
/**
 * コンストラクタ
 */
TransitionFlowData::TransitionFlowData()
{
	_flowLinkList = nullptr;
	_flowScreenList = nullptr;
}

/**
 * デストラクタ
 */
TransitionFlowData::~TransitionFlowData()
{
	CC_SAFE_RELEASE_NULL(_flowScreenList);
	CC_SAFE_RELEASE_NULL(_flowLinkList);
}

/**
 * 初期化
 * @param	json	初期化用JSONデータ
 * @return			初期化の成否
 */
bool TransitionFlowData::init(const rapidjson::Value& json)
{
	CC_ASSERT(json.HasMember("flowLinkList"));
	auto arr1 = cocos2d::__Array::create();
	for (rapidjson::SizeType i = 0; i < json["flowLinkList"].Size(); i++) {
		if (json["flowLinkList"][i].HasMember("typeIdPair")) {
			if (json["flowLinkList"][i]["typeIdPair"][0][0].GetInt() != 0 || json["flowLinkList"][i]["typeIdPair"][1][0].GetInt() != 0) {
				//スクリーン同士のリンクでなければ読み込み不要。
				continue;
			}
		}
		auto obj = FlowLinkData::create(json["flowLinkList"][i]);
		CC_ASSERT(obj);
		arr1->addObject(obj);
	}
	this->setFlowLinkList(arr1);

	CC_ASSERT(json.HasMember("flowScreenList"));
	auto arr2 = cocos2d::__Array::create();
	for (rapidjson::SizeType i = 0; i < json["flowScreenList"].Size(); i++) {
		auto obj = FlowScreenData::create(json["flowScreenList"][i]);
		CC_ASSERT(obj);
		arr2->addObject(obj);
	}
	this->setFlowScreenList(arr2);

	return true;
}

FlowScreenData *TransitionFlowData::getFlowScreenData(int id)
{
	cocos2d::Ref *ref = nullptr;
	CCARRAY_FOREACH(this->getFlowScreenList(), ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto flowScreenData = static_cast<FlowScreenData *>(ref);
#else
		auto flowScreenData = dynamic_cast<FlowScreenData *>(ref);
#endif
		if (flowScreenData->getId() == id) {
			return flowScreenData;
		}
	}
	return nullptr;
}

FlowLinkData *TransitionFlowData::getFlowLinkData(int id)
{
	cocos2d::Ref *ref = nullptr;
	CCARRAY_FOREACH(this->getFlowLinkList(), ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto flowLinkData = static_cast<FlowLinkData *>(ref);
#else
		auto flowLinkData = dynamic_cast<FlowLinkData *>(ref);
#endif
		if (flowLinkData->getId() == id) {
			return flowLinkData;
		}
	}
	return nullptr;
}

#if defined(AGTK_DEBUG)
/**
* ダンプ
*/
void TransitionFlowData::dump()
{
	CCLOG("flowLinkList:");
	cocos2d::Ref *ref1 = nullptr;
	CCARRAY_FOREACH(this->getFlowLinkList(), ref1) {
		auto p = dynamic_cast<FlowLinkData *>(ref1);
		CC_ASSERT(p);
		p->dump();
	}

	CCLOG("flowScreenList:");
	cocos2d::Ref *ref2 = nullptr;
	CCARRAY_FOREACH(this->getFlowScreenList(), ref2) {
		auto p = dynamic_cast<FlowScreenData *>(ref2);
		CC_ASSERT(p);
		p->dump();
	}
}
#endif

//-------------------------------------------------------------------------------------------------------------------
GameInformationData::GameInformationData()
{
	_title = nullptr;
	_author = nullptr;
	_genre = nullptr;
	_description = nullptr;
	_mainLanguage = nullptr;
	_initMainLanguage = nullptr;
	this->setMainLanguage(cocos2d::String::create("en_US"));
	this->setInitMainLanguage(_mainLanguage);
	_languageList = nullptr;
}

GameInformationData::~GameInformationData()
{
	CC_SAFE_RELEASE_NULL(_title);
	CC_SAFE_RELEASE_NULL(_author);
	CC_SAFE_RELEASE_NULL(_genre);
	CC_SAFE_RELEASE_NULL(_description);
	CC_SAFE_RELEASE_NULL(_initMainLanguage);
	CC_SAFE_RELEASE_NULL(_mainLanguage);
	CC_SAFE_RELEASE_NULL(_languageList);
}

bool GameInformationData::init(const rapidjson::Value& json)
{
	CC_ASSERT(json.HasMember("title"));
	this->setTitle(cocos2d::__String::create(json["title"].GetString()));
	CC_ASSERT(json.HasMember("author"));
	this->setAuthor(cocos2d::__String::create(json["author"].GetString()));
	CC_ASSERT(json.HasMember("genre"));
	this->setGenre(cocos2d::__String::create(json["genre"].GetString()));
	CC_ASSERT(json.HasMember("description"));
	this->setDescription(cocos2d::__String::create(json["description"].GetString()));

	// 言語リスト作成
	auto arr = cocos2d::__Array::create();
	CC_ASSERT(json.HasMember("language"));
	for (rapidjson::SizeType i = 0; i < json["language"].Size(); i++) {
		auto const str = json["language"][i].GetString();
		arr->addObject(cocos2d::__String::create(str));
	}
	this->setLanguageList(arr);

	// メイン言語設定
	//    言語リストにPC の言語がある場合は、メイン言語はPC言語
	//    ないばあいはプロジェクトのメイン言語に設定
	{
		if (json.HasMember("mainLanguage")) {
			this->setMainLanguage(cocos2d::__String::create(json["mainLanguage"].GetString()));
		}
#ifdef USE_RUNTIME
		// 動作環境の表示言語取得
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
		std::string const env = CCApplication::getInstance()->getCurrentLanguageShortName();
#endif
		for (auto i = 0; i < getLanguageCount(); ++i) {
			std::string const l = getLanguage(i);
			if ( env == l ) {
				setMainLanguage(cocos2d::__String::create(l));
			}
		}
		bool found = false;
		cocos2d::Ref *ref = nullptr;
		CCARRAY_FOREACH(_languageList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto language = static_cast<cocos2d::__String *>(ref);
#else
			auto language = dynamic_cast<cocos2d::__String *>(ref);
#endif
			if (strcmp(language->getCString(), _mainLanguage->getCString()) == 0) {
				found = true;
				break;
			}
		}
		if (!found) {
			_languageList->addObject(_mainLanguage);
		}
#endif
		setInitMainLanguage(_mainLanguage);
	}
	return true;
}

const char *GameInformationData::getTitle()
{
	CC_ASSERT(_title);
	return _title->getCString();
}

const char *GameInformationData::getAuthor()
{
	CC_ASSERT(_author);
	return _author->getCString();
}

const char *GameInformationData::getGenre()
{
	CC_ASSERT(_genre);
	return _genre->getCString();
}

const char *GameInformationData::getDescription()
{
	CC_ASSERT(_description);
	return _description->getCString();
}

const char *GameInformationData::getMainLanguage()
{
	CC_ASSERT(_mainLanguage);
	return _mainLanguage->getCString();
}

const char *GameInformationData::getInitMainLanguage()
{
	CC_ASSERT(_initMainLanguage);
	return _initMainLanguage->getCString();
}

const char *GameInformationData::getLanguage(int id)
{
	auto languageList = this->getLanguageList();
	if (id < 0 || id >= languageList->count()) {
		return nullptr;
	}
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	auto string = static_cast<cocos2d::String *>(this->getLanguageList()->objectAtIndex(id));
#else
	auto string = dynamic_cast<cocos2d::String *>(this->getLanguageList()->objectAtIndex(id));
#endif
	return string->getCString();
}

int GameInformationData::getLanguageCount()
{
	return this->getLanguageList()->count();
}

int GameInformationData::getMainLanguageId()
{
	return getLanguageId(getMainLanguage());
}

int GameInformationData::getInitMainLanguageId()
{
	return getLanguageId(getInitMainLanguage());
}

int GameInformationData::getLanguageId(const char* lang)
{
	auto languageList = this->getLanguageList();
	for (int i = 0; i < languageList->count(); i++) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto language = static_cast<cocos2d::__String *>(languageList->objectAtIndex(i));
#else
		auto language = dynamic_cast<cocos2d::__String *>(languageList->objectAtIndex(i));
#endif
		if (language->compare(lang) == 0) {
			return i;
		}
	}
	return -1;
}

#if defined(AGTK_DEBUG)
void GameInformationData::dump()
{
	CCLOG("title:%s", this->getTitle());
	CCLOG("author:%s", this->getAuthor());
	CCLOG("genre:%s", this->getGenre());
	CCLOG("description:%s", this->getDescription());
	CCLOG("mainLanguage:%s", this->getMainLanguage());
	CCLOG("language:");
	cocos2d::Ref *ref = nullptr;
	CCARRAY_FOREACH(this->getLanguageList(), ref) {
		auto p = dynamic_cast<cocos2d::__String *>(ref);
		CC_ASSERT(p);
		CCLOG("  %s", p->getCString());
	}
}
#endif

//-------------------------------------------------------------------------------------------------------------------
SoundSettingData::SoundSettingData()
{
	_bgmVolume = 100;
	_seVolume = 100;
	_voiceVolume = 100;
	_initBgmVolume = _bgmVolume;
	_initSeVolume = _seVolume;
	_initVoiceVolume = _voiceVolume;
}

SoundSettingData::~SoundSettingData()
{
}

rapidjson::Value SoundSettingData::serialize(rapidjson::Document::AllocatorType& allocator)
{
	rapidjson::Value soundData(rapidjson::kObjectType);
	soundData.AddMember("bgmVolume", _bgmVolume, allocator);
	soundData.AddMember("seVolume", _seVolume, allocator);
	soundData.AddMember("voiceVolume", _voiceVolume, allocator);
	return soundData;
}

bool SoundSettingData::deserialize(const rapidjson::Value& json)
{
	CC_ASSERT(json.HasMember("bgmVolume"));
	this->setBgmVolume(json["bgmVolume"].GetInt());
	CC_ASSERT(json.HasMember("seVolume"));
	this->setSeVolume(json["seVolume"].GetInt());
	CC_ASSERT(json.HasMember("voiceVolume"));
	this->setVoiceVolume(json["voiceVolume"].GetInt());
	return true;
}
bool SoundSettingData::init(const rapidjson::Value& json)
{
	deserialize(json);
	this->setInitBgmVolume(this->getBgmVolume());
	this->setInitSeVolume(this->getSeVolume());
	this->setInitVoiceVolume(this->getVoiceVolume());
	return true;
}

//-------------------------------------------------------------------------------------------------------------------
ActionProgramSettingData::ActionProgramSettingData()
{
	_memo = nullptr;
}

ActionProgramSettingData::~ActionProgramSettingData()
{
	CC_SAFE_RELEASE_NULL(_memo);
}

bool ActionProgramSettingData::init(const rapidjson::Value& json)
{
	this->setAutoApart(json["autoApart"].GetBool());
	this->setGridShow(json["gridShow"].GetBool());
	this->setGridMagnet(json["gridMagnet"].GetBool());
	this->setActionIconHide(json["actionIconHide"].GetBool());
	this->setMemo(cocos2d::__String::create(json["memo"].GetString()));
	return true;
}

const char *ActionProgramSettingData::getMemo()
{
	CC_ASSERT(_memo);
	return _memo->getCString();
}

#if defined(AGTK_DEBUG)
void ActionProgramSettingData::dump()
{
	CCLOG("autoApart:%d", this->getAutoApart());
	CCLOG("gridShow:%d", this->getGridShow());
	CCLOG("gridMagnet:%d", this->getGridMagnet());
	CCLOG("actionIconHide:%d", this->getActionIconHide());
	CCLOG("memo:%s", this->getMemo());
}
#endif

//-------------------------------------------------------------------------------------------------------------------
InputMapping::InputMapping()
{
}

InputMapping::~InputMapping()
{
}

static struct {
	int keyCode;
	int code;
} sKeyCodeMapping[] = {
	{ (int)EventKeyboard::KeyCode::KEY_PAUSE,  16777224 },	//Pause
	{ (int)EventKeyboard::KeyCode::KEY_SCROLL_LOCK,  16777254 },	//ScrollLock
	{ (int)EventKeyboard::KeyCode::KEY_PRINT,  16777225 },	//Print
	{ (int)EventKeyboard::KeyCode::KEY_SYSREQ,  16777226 },	//SysReq
	{ (int)EventKeyboard::KeyCode::KEY_BREAK,  16908289 },	//Cancel
	{ (int)EventKeyboard::KeyCode::KEY_ESCAPE,  16777216 },	//Escape
	{ (int)EventKeyboard::KeyCode::KEY_ESCAPE,  16777313 },	//Back//KEY_BACK 
	{ (int)EventKeyboard::KeyCode::KEY_BACKSPACE,  16777219 },	//Backspace
	{ (int)EventKeyboard::KeyCode::KEY_TAB,  16777217 },	//Tab
	{ (int)EventKeyboard::KeyCode::KEY_BACK_TAB,  16777218 },	//Backtab
	{ (int)EventKeyboard::KeyCode::KEY_RETURN,  16777220 },	//Return
	{ (int)EventKeyboard::KeyCode::KEY_CAPS_LOCK,  16777252 },	//CapsLock
	{ (int)EventKeyboard::KeyCode::KEY_LEFT_SHIFT,  16777248 },	//Shift
	{ (int)EventKeyboard::KeyCode::KEY_RIGHT_SHIFT,  16777248 },	//Shift
	{ (int)EventKeyboard::KeyCode::KEY_LEFT_CTRL,  16777249 },	//Control
	{ (int)EventKeyboard::KeyCode::KEY_RIGHT_CTRL,  16777249 },	//Control
	{ (int)EventKeyboard::KeyCode::KEY_LEFT_ALT,  16777251 },	//Alt
	{ (int)EventKeyboard::KeyCode::KEY_RIGHT_ALT,  16777251 },	//Alt
	{ (int)EventKeyboard::KeyCode::KEY_MENU,  16777301 },	//Menu
	{ (int)EventKeyboard::KeyCode::KEY_HYPER,  16777250 },	//Meta
	{ (int)EventKeyboard::KeyCode::KEY_INSERT,  16777222 },	//Insert
	{ (int)EventKeyboard::KeyCode::KEY_HOME,  16777232 },	//Home
	{ (int)EventKeyboard::KeyCode::KEY_PG_UP,  16777238 },	//PageUp
	{ (int)EventKeyboard::KeyCode::KEY_DELETE,  16777223 },	//Delete
	{ (int)EventKeyboard::KeyCode::KEY_END,  16777233 },	//End
	{ (int)EventKeyboard::KeyCode::KEY_PG_DOWN,  16777239 },	//PageDown
	{ (int)EventKeyboard::KeyCode::KEY_LEFT_ARROW,  16777234 },	//Left
	{ (int)EventKeyboard::KeyCode::KEY_RIGHT_ARROW,  16777236 },	//Right
	{ (int)EventKeyboard::KeyCode::KEY_UP_ARROW,  16777235 },	//Up
	{ (int)EventKeyboard::KeyCode::KEY_DOWN_ARROW,  16777237 },	//Down
	{ (int)EventKeyboard::KeyCode::KEY_NUM_LOCK,  16777253 },	//NumLock
	//{ (int)EventKeyboard::KeyCode::KEY_KP_PLUS,  43 },	//Plus
	//{ (int)EventKeyboard::KeyCode::KEY_KP_MINUS,  45 },	//Minus
	//{ (int)EventKeyboard::KeyCode::KEY_KP_MULTIPLY,  42 },	//Asterisk
	//{ (int)EventKeyboard::KeyCode::KEY_KP_DIVIDE,  47 },	//Slash
	{ (int)EventKeyboard::KeyCode::KEY_KP_ENTER,  16777221 },	//Enter
	{ (int)EventKeyboard::KeyCode::KEY_KP_HOME,  16777232 },	//Home
	{ (int)EventKeyboard::KeyCode::KEY_KP_UP,  16777235 },	//Up
	{ (int)EventKeyboard::KeyCode::KEY_KP_PG_UP,  16777238 },	//PageUp
	{ (int)EventKeyboard::KeyCode::KEY_KP_LEFT,  16777234 },	//Left
	{ (int)EventKeyboard::KeyCode::KEY_KP_FIVE,  16777227 },	//Break
	{ (int)EventKeyboard::KeyCode::KEY_KP_RIGHT,  16777236 },	//Right
	{ (int)EventKeyboard::KeyCode::KEY_KP_END,  16777233 },	//End
	{ (int)EventKeyboard::KeyCode::KEY_KP_DOWN,  16777237 },	//Down
	{ (int)EventKeyboard::KeyCode::KEY_KP_PG_DOWN,  16777239 },	//PageDown
	{ (int)EventKeyboard::KeyCode::KEY_KP_INSERT,  16777222 },	//Insert
	{ (int)EventKeyboard::KeyCode::KEY_KP_DELETE,  16777223 },	//Delete
	{ (int)EventKeyboard::KeyCode::KEY_F1,  16777264 },	//F1
	{ (int)EventKeyboard::KeyCode::KEY_F2,  16777265 },	//F2
	{ (int)EventKeyboard::KeyCode::KEY_F3,  16777266 },	//F3
	{ (int)EventKeyboard::KeyCode::KEY_F4,  16777267 },	//F4
	{ (int)EventKeyboard::KeyCode::KEY_F5,  16777268 },	//F5
	{ (int)EventKeyboard::KeyCode::KEY_F6,  16777269 },	//F6
	{ (int)EventKeyboard::KeyCode::KEY_F7,  16777270 },	//F7
	{ (int)EventKeyboard::KeyCode::KEY_F8,  16777271 },	//F8
	{ (int)EventKeyboard::KeyCode::KEY_F9,  16777272 },	//F9
	{ (int)EventKeyboard::KeyCode::KEY_F10,  16777273 },	//F10
	{ (int)EventKeyboard::KeyCode::KEY_F11,  16777274 },	//F11
	{ (int)EventKeyboard::KeyCode::KEY_F12,  16777275 },	//F12
	{ (int)EventKeyboard::KeyCode::KEY_SPACE,  32 },	//Space
	{ (int)EventKeyboard::KeyCode::KEY_EXCLAM,  33 },	//Exclam
	{ (int)EventKeyboard::KeyCode::KEY_QUOTE, 34 },	//QuoteDbl
	{ (int)EventKeyboard::KeyCode::KEY_NUMBER,  35 },	//NumberSign
	{ (int)EventKeyboard::KeyCode::KEY_DOLLAR,  36 },	//Dollar
	{ (int)EventKeyboard::KeyCode::KEY_PERCENT,  37 },	//Percent
	{ (int)EventKeyboard::KeyCode::KEY_CIRCUMFLEX,  94 },	//AsciiCircum
	{ (int)EventKeyboard::KeyCode::KEY_AMPERSAND,  38 },	//Ampersand
	{ (int)EventKeyboard::KeyCode::KEY_APOSTROPHE,  39 },	//Apostrophe
	{ (int)EventKeyboard::KeyCode::KEY_LEFT_PARENTHESIS,  40 },	//ParenLeft
	{ (int)EventKeyboard::KeyCode::KEY_RIGHT_PARENTHESIS,  41 },	//ParenRight
	{ (int)EventKeyboard::KeyCode::KEY_ASTERISK,  42 },	//Asterisk
	{ (int)EventKeyboard::KeyCode::KEY_PLUS,  43 },	//Plus
	{ (int)EventKeyboard::KeyCode::KEY_COMMA,  44 },	//Comma
	{ (int)EventKeyboard::KeyCode::KEY_MINUS,  45 },	//Minus
	{ (int)EventKeyboard::KeyCode::KEY_PERIOD,  46 },	//Period
	{ (int)EventKeyboard::KeyCode::KEY_SLASH,  47 },	//Slash
	{ (int)EventKeyboard::KeyCode::KEY_0,  48 },	//0
	{ (int)EventKeyboard::KeyCode::KEY_1,  49 },	//1
	{ (int)EventKeyboard::KeyCode::KEY_2,  50 },	//2
	{ (int)EventKeyboard::KeyCode::KEY_3,  51 },	//3
	{ (int)EventKeyboard::KeyCode::KEY_4,  52 },	//4
	{ (int)EventKeyboard::KeyCode::KEY_5,  53 },	//5
	{ (int)EventKeyboard::KeyCode::KEY_6,  54 },	//6
	{ (int)EventKeyboard::KeyCode::KEY_7,  55 },	//7
	{ (int)EventKeyboard::KeyCode::KEY_8,  56 },	//8
	{ (int)EventKeyboard::KeyCode::KEY_9,  57 },	//9
	{ (int)EventKeyboard::KeyCode::KEY_COLON,  58 },	//Colon
	{ (int)EventKeyboard::KeyCode::KEY_SEMICOLON,  59 },	//Semicolon
	{ (int)EventKeyboard::KeyCode::KEY_LESS_THAN,  60 },	//Less
	{ (int)EventKeyboard::KeyCode::KEY_EQUAL,  61 },	//Equal
	{ (int)EventKeyboard::KeyCode::KEY_GREATER_THAN,  62 },	//Greater
	{ (int)EventKeyboard::KeyCode::KEY_QUESTION,  63 },	//Question
	{ (int)EventKeyboard::KeyCode::KEY_AT,  64 },	//At
	{ (int)EventKeyboard::KeyCode::KEY_CAPITAL_A,  65 },	//A
	{ (int)EventKeyboard::KeyCode::KEY_CAPITAL_B,  66 },	//B
	{ (int)EventKeyboard::KeyCode::KEY_CAPITAL_C,  67 },	//C
	{ (int)EventKeyboard::KeyCode::KEY_CAPITAL_D,  68 },	//D
	{ (int)EventKeyboard::KeyCode::KEY_CAPITAL_E,  69 },	//E
	{ (int)EventKeyboard::KeyCode::KEY_CAPITAL_F,  70 },	//F
	{ (int)EventKeyboard::KeyCode::KEY_CAPITAL_G,  71 },	//G
	{ (int)EventKeyboard::KeyCode::KEY_CAPITAL_H,  72 },	//H
	{ (int)EventKeyboard::KeyCode::KEY_CAPITAL_I,  73 },	//I
	{ (int)EventKeyboard::KeyCode::KEY_CAPITAL_J,  74 },	//J
	{ (int)EventKeyboard::KeyCode::KEY_CAPITAL_K,  75 },	//K
	{ (int)EventKeyboard::KeyCode::KEY_CAPITAL_L,  76 },	//L
	{ (int)EventKeyboard::KeyCode::KEY_CAPITAL_M,  77 },	//M
	{ (int)EventKeyboard::KeyCode::KEY_CAPITAL_N,  78 },	//N
	{ (int)EventKeyboard::KeyCode::KEY_CAPITAL_O,  79 },	//O
	{ (int)EventKeyboard::KeyCode::KEY_CAPITAL_P,  80 },	//P
	{ (int)EventKeyboard::KeyCode::KEY_CAPITAL_Q,  81 },	//Q
	{ (int)EventKeyboard::KeyCode::KEY_CAPITAL_R,  82 },	//R
	{ (int)EventKeyboard::KeyCode::KEY_CAPITAL_S,  83 },	//S
	{ (int)EventKeyboard::KeyCode::KEY_CAPITAL_T,  84 },	//T
	{ (int)EventKeyboard::KeyCode::KEY_CAPITAL_U,  85 },	//U
	{ (int)EventKeyboard::KeyCode::KEY_CAPITAL_V,  86 },	//V
	{ (int)EventKeyboard::KeyCode::KEY_CAPITAL_W,  87 },	//W
	{ (int)EventKeyboard::KeyCode::KEY_CAPITAL_X,  88 },	//X
	{ (int)EventKeyboard::KeyCode::KEY_CAPITAL_Y,  89 },	//Y
	{ (int)EventKeyboard::KeyCode::KEY_CAPITAL_Z,  90 },	//Z
	{ (int)EventKeyboard::KeyCode::KEY_LEFT_BRACKET,  91 },	//BracketLeft
	{ (int)EventKeyboard::KeyCode::KEY_BACK_SLASH,  92 },	//Backslash
	{ (int)EventKeyboard::KeyCode::KEY_RIGHT_BRACKET,  93 },	//BracketRight
	{ (int)EventKeyboard::KeyCode::KEY_UNDERSCORE,  95 },	//Underscore
	{ (int)EventKeyboard::KeyCode::KEY_GRAVE,  96 },	//QuoteLeft
	{ (int)EventKeyboard::KeyCode::KEY_A,  65 },	//A
	{ (int)EventKeyboard::KeyCode::KEY_B,  66 },	//B
	{ (int)EventKeyboard::KeyCode::KEY_C,  67 },	//C
	{ (int)EventKeyboard::KeyCode::KEY_D,  68 },	//D
	{ (int)EventKeyboard::KeyCode::KEY_E,  69 },	//E
	{ (int)EventKeyboard::KeyCode::KEY_F,  70 },	//F
	{ (int)EventKeyboard::KeyCode::KEY_G,  71 },	//G
	{ (int)EventKeyboard::KeyCode::KEY_H,  72 },	//H
	{ (int)EventKeyboard::KeyCode::KEY_I,  73 },	//I
	{ (int)EventKeyboard::KeyCode::KEY_J,  74 },	//J
	{ (int)EventKeyboard::KeyCode::KEY_K,  75 },	//K
	{ (int)EventKeyboard::KeyCode::KEY_L,  76 },	//L
	{ (int)EventKeyboard::KeyCode::KEY_M,  77 },	//M
	{ (int)EventKeyboard::KeyCode::KEY_N,  78 },	//N
	{ (int)EventKeyboard::KeyCode::KEY_O,  79 },	//O
	{ (int)EventKeyboard::KeyCode::KEY_P,  80 },	//P
	{ (int)EventKeyboard::KeyCode::KEY_Q,  81 },	//Q
	{ (int)EventKeyboard::KeyCode::KEY_R,  82 },	//R
	{ (int)EventKeyboard::KeyCode::KEY_S,  83 },	//S
	{ (int)EventKeyboard::KeyCode::KEY_T,  84 },	//T
	{ (int)EventKeyboard::KeyCode::KEY_U,  85 },	//U
	{ (int)EventKeyboard::KeyCode::KEY_V,  86 },	//V
	{ (int)EventKeyboard::KeyCode::KEY_W,  87 },	//W
	{ (int)EventKeyboard::KeyCode::KEY_X,  88 },	//X
	{ (int)EventKeyboard::KeyCode::KEY_Y,  89 },	//Y
	{ (int)EventKeyboard::KeyCode::KEY_Z,  90 },	//Z
	{ (int)EventKeyboard::KeyCode::KEY_LEFT_BRACE,  123 },	//BraceLeft
	{ (int)EventKeyboard::KeyCode::KEY_BAR,  124 },	//Bar
	{ (int)EventKeyboard::KeyCode::KEY_RIGHT_BRACE,  125 },	//BraceRight
	{ (int)EventKeyboard::KeyCode::KEY_TILDE,  126 },	//AsciiTilde
	{ (int)EventKeyboard::KeyCode::KEY_EURO,  -1 },	//euro
	{ (int)EventKeyboard::KeyCode::KEY_POUND,  -1 },	//pound
	{ (int)EventKeyboard::KeyCode::KEY_YEN,  165 },	//yen
	{ (int)EventKeyboard::KeyCode::KEY_MIDDLE_DOT,  -1 },	//middle dot
	{ (int)EventKeyboard::KeyCode::KEY_SEARCH,  16777362 },	//Search
	{ (int)EventKeyboard::KeyCode::KEY_DPAD_LEFT,  -1 },	//DPAD_LEFT
	{ (int)EventKeyboard::KeyCode::KEY_DPAD_RIGHT,  -1 },	//DPAD_RIGHT
	{ (int)EventKeyboard::KeyCode::KEY_DPAD_UP,  -1 },	//DPAD_UP
	{ (int)EventKeyboard::KeyCode::KEY_DPAD_DOWN,  -1 },	//DPAD_DOWN
	{ (int)EventKeyboard::KeyCode::KEY_DPAD_CENTER,  -1 },	//DPAD_CENTER
	{ (int)EventKeyboard::KeyCode::KEY_ENTER,  16777221 },	//Enter
	{ (int)EventKeyboard::KeyCode::KEY_PLAY,  16908293 },	//Play
};

rapidjson::Value InputMapping::serialize(rapidjson::Document::AllocatorType& allocator)const
{
	rapidjson::Value data(rapidjson::kObjectType);
	data.AddMember("id", getId(), allocator);
	data.AddMember("system", getSystem(), allocator);
	data.AddMember("keyId", getKeyId(), allocator);
	data.AddMember("custom1Input", getCustom1Input(), allocator);
	data.AddMember("custom2Input", getCustom2Input(), allocator);
	data.AddMember("diInput", getDiInput(), allocator);
	auto pcInput = getPcInput();
	if (pcInput >= InputGamepadData::kInputMax) {
		for (int i = 0; i < CC_ARRAYSIZE(sKeyCodeMapping); i++) {
			if (sKeyCodeMapping[i].keyCode == pcInput - InputGamepadData::kInputMax) {
				pcInput = sKeyCodeMapping[i].code + InputGamepadData::kInputMax;
				break;
			}
		}
	}
	data.AddMember("pcInput", pcInput, allocator);
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif
	return data;
}

#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif


bool InputMapping::deserialize(const rapidjson::Value& json)
{
	CC_ASSERT(json.HasMember("id"));
	this->setId(json["id"].GetInt());
	CC_ASSERT(json.HasMember("system"));
	this->setSystem(json["system"].GetBool());
	CC_ASSERT(json.HasMember("keyId"));
	this->setKeyId(json["keyId"].GetInt());
	CC_ASSERT(json.HasMember("custom1Input"));
	this->setCustom1Input(json["custom1Input"].GetInt());
	CC_ASSERT(json.HasMember("custom2Input"));
	this->setCustom2Input(json["custom2Input"].GetInt());
	CC_ASSERT(json.HasMember("diInput"));
	this->setDiInput(json["diInput"].GetInt());
	CC_ASSERT(json.HasMember("pcInput"));
	auto pcInput = json["pcInput"].GetInt();
	if (pcInput >= InputGamepadData::kInputMax) {
		for (int i = 0; i < CC_ARRAYSIZE(sKeyCodeMapping); i++) {
			if (sKeyCodeMapping[i].code == pcInput - InputGamepadData::kInputMax) {
				pcInput = sKeyCodeMapping[i].keyCode + InputGamepadData::kInputMax;
				break;
			}
		}
	}
	this->setPcInput(pcInput);
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif
	return true;
}

bool InputMapping::init(const rapidjson::Value& json)
{
	return deserialize(json);
}

bool InputMapping::init(const agtk::data::InputMapping *inputMapping)
{
	if (inputMapping == nullptr) {
		return false;
	}
	this->copy(inputMapping);
	return true;
}

void InputMapping::copy(const agtk::data::InputMapping *inputMapping)
{
	CC_ASSERT(inputMapping);
	this->setId(inputMapping->getId());
	this->setSystem(inputMapping->getSystem());
	this->setKeyId(inputMapping->getKeyId());
	this->setCustom1Input(inputMapping->getCustom1Input());
	this->setCustom2Input(inputMapping->getCustom2Input());
	this->setDiInput(inputMapping->getDiInput());
	this->setPcInput(inputMapping->getPcInput());
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif
}

bool InputMapping::operator==(const InputMapping& rhs)const
{
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
	return getId() == rhs.getId() && getSystem() == rhs.getSystem() &&
		getKeyId() == rhs.getKeyId() && getCustom1Input() == rhs.getCustom1Input() && 
		getCustom2Input() == rhs.getCustom2Input() && getDiInput() == rhs.getDiInput() &&
		getPcInput() == rhs.getPcInput();
#endif
}
#if defined(AGTK_DEBUG)
void InputMapping::dump()
{
	CCLOG("id:%d", this->getId());
	CCLOG("system:%d", this->getSystem());
	CCLOG("keyId:%d", this->getKeyId());
	CCLOG("custom1Input:%d", this->getCustom1Input());
	CCLOG("custom2Input:%d", this->getCustom2Input());
	CCLOG("diInput:%d", this->getDiInput());
	CCLOG("pcInput:%d", this->getPcInput());
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif
}
#endif

//-------------------------------------------------------------------------------------------------------------------
OperationKey::OperationKey()
{
	_id = -1;
	_name = nullptr;
}

OperationKey::~OperationKey()
{
	CC_SAFE_RELEASE_NULL(_name);
}

bool OperationKey::init(const rapidjson::Value& json)
{
	CC_ASSERT(json.HasMember("id"));
	this->setId(json["id"].GetInt());
	CC_ASSERT(json.HasMember("name"));
	this->setName(cocos2d::__String::create(json["name"].GetString()));
	return true;
}

bool OperationKey::init(const agtk::data::OperationKey *operationKey)
{
	if (operationKey == nullptr) {
		return false;
	}
	this->copy(operationKey);
	return true;
}

bool OperationKey::init(int id, const char *name)
{
	this->setId(id);
	this->setName(cocos2d::__String::create(name));
	return true;
}

void OperationKey::copy(const agtk::data::OperationKey *operationKey)
{
	CC_ASSERT(operationKey);
	this->setId(operationKey->getId());
	auto name = operationKey->getName();
	this->setName(cocos2d::__String::create(name->getCString()));
}

const char *OperationKey::getName()
{
	CC_ASSERT(_name);
	return _name->getCString();
}

#if defined(AGTK_DEBUG)
void OperationKey::dump()
{
	CCLOG("id:%d", this->getId());
	CCLOG("name:%s", this->getName());
}
#endif

//-------------------------------------------------------------------------------------------------------------------
InputMappingData::InputMappingData()
{
	_inputMappingList = nullptr;
	_operationKeyList = nullptr;
}

InputMappingData::~InputMappingData()
{
	CC_SAFE_RELEASE_NULL(_inputMappingList);
	CC_SAFE_RELEASE_NULL(_operationKeyList);
}

rapidjson::Value InputMappingData::serialize(rapidjson::Document::AllocatorType& allocator)const
{
	rapidjson::Value data(rapidjson::kObjectType);
	{
		auto * iml = getInputMappingList();
		auto * allkey = iml->allKeys();
		rapidjson::Value imlJson(rapidjson::kArrayType);
		for (auto i = 0; i < allkey->count(); ++i) {
			auto const key = dynamic_cast<cocos2d::Integer*>(allkey->getObjectAtIndex(i))->getValue();
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto const obj = static_cast<InputMapping*>(iml->objectForKey(key));
#else
			auto const obj = dynamic_cast<InputMapping*>(iml->objectForKey(key));
#endif
			imlJson.PushBack(obj->serialize(allocator), allocator);
		}
		data.AddMember("inputMappingList", imlJson, allocator);
	}
	return data;
}
bool InputMappingData::deserialize(const rapidjson::Value& json)
{
	CCASSERT(json.HasMember("inputMappingList"), "Need inputMappingList");
	auto inputMappingList = cocos2d::__Dictionary::create();
	for (rapidjson::SizeType i = 0; i < json["inputMappingList"].Size(); i++) {
		auto inputMapping = InputMapping::create(json["inputMappingList"][i]);
#if defined(AGTK_DEBUG)
		CC_ASSERT(inputMappingList->objectForKey(inputMapping->getId()) == nullptr);
#endif
		inputMappingList->setObject(inputMapping, inputMapping->getId());
	}
	if (json.HasMember("systemInputMappingList")) {
		for (rapidjson::SizeType i = 0; i < json["systemInputMappingList"].Size(); i++) {
			auto inputMapping = InputMapping::create(json["systemInputMappingList"][i]);
#if defined(AGTK_DEBUG)
			CC_ASSERT(inputMappingList->objectForKey(inputMapping->getId()) == nullptr);
#endif
			inputMappingList->setObject(inputMapping, inputMapping->getId());
		}
	}
	this->setInputMappingList(inputMappingList);

#if 1	// ACT2-5850 システムの操作キーにいずれかのボタンが設定されている場合は、設定無しにコピーする。
	struct DeviceKey {
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
		DeviceKey() : custom1Input(-1), custom2Input(-1), diInput(-1), pcInput(-1) {}
#else
#endif
		int custom1Input;
		int custom2Input;
		int diInput;
		int pcInput;
// #AGTK-NX
#if CC_TARGET_PLATFORM == CC_PLATFORM_NX
#endif
	};
	struct Info {
		Info(const std::list<int> &_keyIdSet) : keyIdSet(_keyIdSet), inputMappingSet(), deviceKey() {}
		std::list<int> keyIdSet;
		std::list<InputMapping *> inputMappingSet;
		DeviceKey deviceKey;
	};
	std::list<Info> infoList;
	infoList.push_back(Info(std::list<int>({ kSystemKeyDisplaySwitch1OperationKeyId, kSystemKeyDisplaySwitch2OperationKeyId, kSystemKeyDisplaySwitch3OperationKeyId, kSystemKeyDisplaySwitch4OperationKeyId })));
	infoList.push_back(Info(std::list<int>({ kSystemKeyMenuDisplay1OperationKeyId, kSystemKeyMenuDisplay2OperationKeyId, kSystemKeyMenuDisplay3OperationKeyId, kSystemKeyMenuDisplay4OperationKeyId })));
	infoList.push_back(Info(std::list<int>({ kSystemKeyReset1OperationKeyId, kSystemKeyReset2OperationKeyId, kSystemKeyReset3OperationKeyId, kSystemKeyReset4OperationKeyId })));

	cocos2d::DictElement *el = nullptr;
	CCDICT_FOREACH(inputMappingList, el) {
		auto inputMapping = dynamic_cast<InputMapping *>(el->getObject());
		for (auto &info : infoList) {
			for (auto keyId : info.keyIdSet) {
				if (keyId == inputMapping->getKeyId()) {
					info.inputMappingSet.push_back(inputMapping);
					auto custom1Input = inputMapping->getCustom1Input();
					if (custom1Input >= 0) {
						info.deviceKey.custom1Input = custom1Input;
					}
					auto custom2Input = inputMapping->getCustom2Input();
					if (custom2Input >= 0) {
						info.deviceKey.custom2Input = custom2Input;
					}
					auto diInput = inputMapping->getDiInput();
					if (diInput >= 0) {
						info.deviceKey.diInput = diInput;
					}
					auto pcInput = inputMapping->getPcInput();
					if (pcInput >= 0) {
						info.deviceKey.pcInput = pcInput;
					}
// #AGTK-NX
#if CC_TARGET_PLATFORM == CC_PLATFORM_NX
#endif
					break;
				}
			}
		}
	}
	for (auto &info : infoList) {
		if (info.deviceKey.custom1Input >= 0) {
			for (auto inputMapping : info.inputMappingSet) {
				if (inputMapping->getCustom1Input() < 0) {
					inputMapping->setCustom1Input(info.deviceKey.custom1Input);
				}
			}
		}
		if (info.deviceKey.custom2Input >= 0) {
			for (auto inputMapping : info.inputMappingSet) {
				if (inputMapping->getCustom2Input() < 0) {
					inputMapping->setCustom2Input(info.deviceKey.custom2Input);
				}
			}
		}
		if (info.deviceKey.diInput >= 0) {
			for (auto inputMapping : info.inputMappingSet) {
				if (inputMapping->getDiInput() < 0) {
					inputMapping->setDiInput(info.deviceKey.diInput);
				}
			}
		}
		if (info.deviceKey.pcInput >= 0) {
			for (auto inputMapping : info.inputMappingSet) {
				if (inputMapping->getPcInput() < 0) {
					inputMapping->setPcInput(info.deviceKey.pcInput);
				}
			}
		}
// #AGTK-NX
#if CC_TARGET_PLATFORM == CC_PLATFORM_NX
#endif
	}
#endif

	return true;
}

bool InputMappingData::init(const rapidjson::Value& json)
{
	deserialize(json);

	auto operationKeyList = cocos2d::__Dictionary::create();
	//system
	struct {
		int id;//※inputMapping->keyId
		const char *name;
	} gOperationSystem[] = {
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
		{ 1, GameManager::tr("A") },
		{ 2, GameManager::tr("B") },
		{ 3, GameManager::tr("X") },
		{ 4, GameManager::tr("Y") },
		{ 5, GameManager::tr("R1") },
		{ 6, GameManager::tr("R2") },
		{ 7, GameManager::tr("L1") },
		{ 8, GameManager::tr("L2") },
		{ 9, GameManager::tr("Up") },
		{ 10, GameManager::tr("Down") },
		{ 11, GameManager::tr("Left") },
		{ 12, GameManager::tr("Right") },
		{ 13, GameManager::tr("Left stick(Up)") },
		{ 14, GameManager::tr("Left stick(Down)") },
		{ 15, GameManager::tr("Left stick(Left)") },
		{ 16, GameManager::tr("Left stick(Right)") },
		{ 17, GameManager::tr("Right stick(Up)") },
		{ 18, GameManager::tr("Right stick(Down)") },
		{ 19, GameManager::tr("Right stick(Left)") },
		{ 20, GameManager::tr("Right stick(Right)") },
		{ 26, GameManager::tr("OK") },
		{ 27, GameManager::tr("CANCEL") },
#endif
	};

	for (int i = 0; i < CC_ARRAYSIZE(gOperationSystem); i++) {
		auto operationKey = OperationKey::create(gOperationSystem[i].id, gOperationSystem[i].name);
		CC_ASSERT(operationKeyList->objectForKey(operationKey->getId()) == nullptr);
		operationKeyList->setObject(operationKey, operationKey->getId());
	}
	if (json.HasMember("operationKeyList"))
	{
		//custum
		for (rapidjson::SizeType i = 0; i < json["operationKeyList"].Size(); i++) {
			auto operationKey = OperationKey::create(json["operationKeyList"][i]);
#if defined(AGTK_DEBUG)
			CC_ASSERT(operationKeyList->objectForKey(operationKey->getId()) == nullptr);
#endif
			operationKeyList->setObject(operationKey, operationKey->getId());
		}
		this->setOperationKeyList(operationKeyList);
	}
	return true;
}

bool InputMappingData::init(const InputMappingData& data)
{
	copy(data);
	return true;
}

void InputMappingData::copy(const InputMappingData& data)
{
	{
		this->setInputMappingList(cocos2d::__Dictionary::create());
		auto const list = data.getInputMappingList();
		cocos2d::DictElement *el = nullptr;
		CCDICT_FOREACH(list, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto obj = static_cast<InputMapping *>(el->getObject());
#else
			auto obj = dynamic_cast<InputMapping *>(el->getObject());
#endif
			auto instance = InputMapping::create(obj);
			getInputMappingList()->setObject(instance, instance->getId());
		}
	}
	{
		this->setOperationKeyList(cocos2d::__Dictionary::create());
		auto const list = data.getOperationKeyList();
		cocos2d::DictElement *el = nullptr;
		CCDICT_FOREACH(list, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto obj = static_cast<OperationKey*>(el->getObject());
#else
			auto obj = dynamic_cast<OperationKey*>(el->getObject());
#endif
			auto instance = OperationKey::create(obj);
			getOperationKeyList()->setObject(instance, instance->getId());
		}
	}
}
InputMapping *InputMappingData::getInputMapping(int id)
{
	return dynamic_cast<agtk::data::InputMapping *>(this->getInputMappingList()->objectForKey(id));
}

OperationKey *InputMappingData::getOperationKey(int id)
{
	return dynamic_cast<agtk::data::OperationKey *>(this->getOperationKeyList()->objectForKey(id));
}

int InputMappingData::getInputMappingCount()
{
	return this->getInputMappingList()->count();
}

int InputMappingData::getOperationKeyCount()
{
	return this->getOperationKeyList()->count();
}

bool InputMappingData::checkSystemKey(int keyId)
{
	if (keyId == kSystemKeyDisplaySwitch1OperationKeyId
	|| keyId == kSystemKeyDisplaySwitch2OperationKeyId
	|| keyId == kSystemKeyDisplaySwitch3OperationKeyId
	|| keyId == kSystemKeyDisplaySwitch4OperationKeyId
	|| keyId == kSystemKeyMenuDisplay1OperationKeyId
	|| keyId == kSystemKeyMenuDisplay2OperationKeyId
	|| keyId == kSystemKeyMenuDisplay3OperationKeyId
	|| keyId == kSystemKeyMenuDisplay4OperationKeyId
	|| keyId == kSystemKeyReset1OperationKeyId
	|| keyId == kSystemKeyReset2OperationKeyId
	|| keyId == kSystemKeyReset3OperationKeyId
	|| keyId == kSystemKeyReset4OperationKeyId) {
		return true;
	}
	return false;
}

#if defined(AGTK_DEBUG)
bool InputMappingData::operator==(const InputMappingData& rhs)const
{
	cocos2d::DictElement *el1 = nullptr;
	auto iml1 = this->getInputMappingList();
	CCDICT_FOREACH(iml1, el1) {
		auto p1 = dynamic_cast<InputMapping *>(el1->getObject());
		bool same = false;
		cocos2d::DictElement *el2 = nullptr;
		auto iml2 = rhs.getInputMappingList();
		CCDICT_FOREACH(iml2, el2) {
			auto p2 = dynamic_cast<InputMapping *>(el2->getObject());
			if (*p1 == *p2) { 
				same = true;
				break; 
			}
		}
		if (!same) {
			CCLOG("===not same!===");
			CCLOG("InputMapping lhs");
			p1->dump();
			CCLOG("======");
			return false;
		}
	}
	return true;
}
void InputMappingData::dump()
{
	cocos2d::DictElement *el = nullptr;
	CCLOG("inputMappoingList");
	auto inputMappingList = this->getInputMappingList();
	CCDICT_FOREACH(inputMappingList, el) {
		auto p = dynamic_cast<InputMapping *>(el->getObject());
		p->dump();
	}
	CCLOG("operationKeyList");
	auto operationKeyList = this->getOperationKeyList();
	CCDICT_FOREACH(operationKeyList, el) {
		auto p = dynamic_cast<OperationKey *>(el->getObject());
		p->dump();
	}
}
#endif

//-------------------------------------------------------------------------------------------------------------------
//※ ActionProgramSetting と同じ内容です。重複してる？
ObjectActionProgramSettingsData::ObjectActionProgramSettingsData()
{
	_memo = nullptr;
}

ObjectActionProgramSettingsData::~ObjectActionProgramSettingsData()
{
	CC_SAFE_RELEASE_NULL(_memo);
}

bool ObjectActionProgramSettingsData::init(const rapidjson::Value& json)
{
	this->setAutoApart(json["autoApart"].GetBool());
	this->setGridShow(json["gridShow"].GetBool());
	this->setGridMagnet(json["gridMagnet"].GetBool());
	this->setActionIconHide(json["actionIconHide"].GetBool());
	this->setMemo(cocos2d::__String::create(json["memo"].GetString()));
	return true;
}

const char *ObjectActionProgramSettingsData::getMemo()
{
	CC_ASSERT(_memo);
	return _memo->getCString();
}

#if defined(AGTK_DEBUG)
void ObjectActionProgramSettingsData::dump()
{
	CCLOG("autoApart:%d", this->getAutoApart());
	CCLOG("gridShow:%d", this->getGridShow());
	CCLOG("gridMagnet:%d", this->getGridMagnet());
	CCLOG("actionIconHide:%d", this->getActionIconHide());
	CCLOG("memo:%s", this->getMemo());
}
#endif

//-------------------------------------------------------------------------------------------------------------------
PlayerCharacterData::PlayerCharacterData()
{
	_player1CharacterList = nullptr;
	_player2CharacterList = nullptr;
	_player3CharacterList = nullptr;
	_player4CharacterList = nullptr;
}

PlayerCharacterData::~PlayerCharacterData()
{
	CC_SAFE_RELEASE_NULL(_player1CharacterList);
	CC_SAFE_RELEASE_NULL(_player2CharacterList);
	CC_SAFE_RELEASE_NULL(_player3CharacterList);
	CC_SAFE_RELEASE_NULL(_player4CharacterList);
}

bool PlayerCharacterData::init(const rapidjson::Value& json)
{
	//1p
	auto player1CharacterList = cocos2d::__Array::create();
	for (rapidjson::SizeType i = 0; i < json[0].Size(); i++) {
		auto v = cocos2d::Integer::create(json[0][i].GetInt());
		player1CharacterList->addObject(v);
	}
	this->setPlayer1CharacterList(player1CharacterList);
	//2p
	auto player2CharacterList = cocos2d::__Array::create();
	for (rapidjson::SizeType i = 0; i < json[1].Size(); i++) {
		auto v = cocos2d::Integer::create(json[1][i].GetInt());
		player2CharacterList->addObject(v);
	}
	this->setPlayer2CharacterList(player2CharacterList);
	//3p
	auto player3CharacterList = cocos2d::__Array::create();
	for (rapidjson::SizeType i = 0; i < json[2].Size(); i++) {
		auto v = cocos2d::Integer::create(json[2][i].GetInt());
		player3CharacterList->addObject(v);
	}
	this->setPlayer3CharacterList(player3CharacterList);
	//4p
	auto player4CharacterList = cocos2d::__Array::create();
	for (rapidjson::SizeType i = 0; i < json[3].Size(); i++) {
		auto v = cocos2d::Integer::create(json[3][i].GetInt());
		player4CharacterList->addObject(v);
	}
	this->setPlayer4CharacterList(player4CharacterList);

	return true;
}

cocos2d::__Array *PlayerCharacterData::getPlayerCharacterLine(int line)
{
	auto arr = cocos2d::__Array::create();
	cocos2d::Integer *v = nullptr;
	if (line < this->getPlayer1CharacterList()->count()) {
		v = dynamic_cast<cocos2d::Integer *>(this->getPlayer1CharacterList()->getObjectAtIndex(line));
	} else {
		v = cocos2d::Integer::create(-1);
	}
	arr->addObject(v);
	if (line < this->getPlayer2CharacterList()->count()) {
		v = dynamic_cast<cocos2d::Integer *>(this->getPlayer2CharacterList()->getObjectAtIndex(line));
	} else {
		v = cocos2d::Integer::create(-1);
	}
	arr->addObject(v);
	if (line < this->getPlayer3CharacterList()->count()) {
		v = dynamic_cast<cocos2d::Integer *>(this->getPlayer3CharacterList()->getObjectAtIndex(line));
	} else {
		v = cocos2d::Integer::create(-1);
	}
	arr->addObject(v);
	if (line < this->getPlayer4CharacterList()->count()) {
		v = dynamic_cast<cocos2d::Integer *>(this->getPlayer4CharacterList()->getObjectAtIndex(line));
	} else {
		v = cocos2d::Integer::create(-1);
	}
	arr->addObject(v);
	return arr;
}

cocos2d::__Array *PlayerCharacterData::getPlayerCharacterList(int index)
{
	if (index == 0) {
		return this->getPlayer1CharacterList();
	}
	else if (index == 1) {
		return this->getPlayer2CharacterList();
	}
	else if (index == 2) {
		return this->getPlayer3CharacterList();
	}
	else if (index == 3) {
		return this->getPlayer4CharacterList();
	}
	else {
		return nullptr;
	}
}

#if defined(AGTK_DEBUG)
void PlayerCharacterData::dump()
{
	cocos2d::__Array *arr = nullptr;
	cocos2d::Ref *ref = nullptr;
	//1p
	arr = this->getPlayer1CharacterList();
	std::string tmp = "[";
	CCARRAY_FOREACH(arr, ref) {
		auto p = dynamic_cast<cocos2d::Integer *>(ref);
		tmp += std::to_string(p->getValue());
		tmp += ", ";
	}
	tmp += "]";
	CCLOG("%s", tmp.c_str());
	arr = this->getPlayer2CharacterList();
	tmp = "[";
	CCARRAY_FOREACH(arr, ref) {
		auto p = dynamic_cast<cocos2d::Integer *>(ref);
		tmp += std::to_string(p->getValue());
		tmp += ", ";
	}
	tmp += "]";
	CCLOG("%s", tmp.c_str());
	arr = this->getPlayer3CharacterList();
	tmp = "[";
	CCARRAY_FOREACH(arr, ref) {
		auto p = dynamic_cast<cocos2d::Integer *>(ref);
		tmp += std::to_string(p->getValue());
		tmp += ", ";
	}
	tmp += "]";
	CCLOG("%s", tmp.c_str());
	arr = this->getPlayer4CharacterList();
	tmp = "[";
	CCARRAY_FOREACH(arr, ref) {
		auto p = dynamic_cast<cocos2d::Integer *>(ref);
		tmp += std::to_string(p->getValue());
		tmp += ", ";
	}
	tmp += "]";
	CCLOG("%s", tmp.c_str());
}
#endif

//-------------------------------------------------------------------------------------------------------------------

AnimParticleImageData::AnimParticleImageData()
{
	_id = 0;
	_filename = nullptr;
}

AnimParticleImageData::~AnimParticleImageData()
{
	CC_SAFE_RELEASE_NULL(_filename);
}

bool AnimParticleImageData::init(const rapidjson::Value& json)
{
	if (json.IsObject()) {
		this->setId(json["id"].GetInt());
		this->setFilename(cocos2d::String::create(json["filename"].GetString()));
	} else {
		this->setId(json.GetInt());
	}
	return true;
}
bool AnimParticleImageData::init(int id)
{
	this->setId(id);
	return true;
}
#if defined(AGTK_DEBUG)
void AnimParticleImageData::dump()
{
	CCLOG("Id:%d", this->getId());
	CCLOG("fileName:%s", this->getFilename()->getCString());
}
#endif

//-------------------------------------------------------------------------------------------------------------------
ProjectData::ProjectData()
{
	_gameIcon = nullptr;
	_playerCount = 1;
// #AGTK-NX, #AGTK-WIN
#ifdef USE_30FPS
	_mode30Fps = false;
#endif
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX) && defined(USE_NX60FPS)
#endif
	_magnifyWindow = false;
	_windowMagnification = 1;
	_adjustPixelMagnification = false;
	_pixelMagnificationType = kPixelMagnificationTypeEnlargeByIntegalMultiple;
	_displayMenuBar = false;
	_displayMousePointer = true;
	_loadingSceneId = -1;
	_wallDetectionOverlapMargin = 0.6;
	_multithreading = true;
	_screenEffectValueList = nullptr;//->cocos2d::Double
	_screenEffectFlagList = nullptr;//->cocos2d::Bool
	_gameInformation = nullptr;//->GameInformationData
	_soundSetting = nullptr;//->SoundSettingData
	_actionProgramSetting = nullptr;//->ActionProgramSettingData
	_inputMapping = nullptr;//->InputMappingData
	_initInputMapping = nullptr;
	_sceneList = nullptr;//->SceneData
	_tilesetList = nullptr;//->TilesetData
	_fontList = nullptr;//->FontData
	_imageList = nullptr;//->ImageData
	_movieList = nullptr;//->MovieData
	_bgmList = nullptr;//->BgmData
	_seList = nullptr;//->SeData
	_voiceList = nullptr;//->VoiceData
	_variableList = nullptr;//->VariableData
	_switchList = nullptr;//->SwitchData
	_textList = nullptr;//->TextData
	_animationOnlyList = nullptr;//->AnimationOnlyData
	_objectList = nullptr;//->ObjectData
	_animationList = nullptr;//->AnimationData
	_objectActionProgramSettings = nullptr;//->ObjectActionProgramSettingsData
	_animParticleImageList = nullptr;
	_transitionFlow = nullptr;
	_transitionPortalList = nullptr;
	_playerCharacterData = nullptr;//->PlayerCharacterData
	_pluginList = nullptr;//->PluginData
	_databaseList = nullptr;//->DatabaseData
	_objectGroup = nullptr;
	_playerCount = 1;
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif
}

ProjectData::~ProjectData()
{
	CC_SAFE_RELEASE_NULL(_gameIcon);
	CC_SAFE_RELEASE_NULL(_screenEffectValueList);
	CC_SAFE_RELEASE_NULL(_screenEffectFlagList);
	CC_SAFE_RELEASE_NULL(_gameInformation);
	CC_SAFE_RELEASE_NULL(_soundSetting);
	CC_SAFE_RELEASE_NULL(_actionProgramSetting);
	CC_SAFE_RELEASE_NULL(_inputMapping);
	CC_SAFE_RELEASE_NULL(_initInputMapping);
	CC_SAFE_RELEASE_NULL(_sceneList);
	CC_SAFE_RELEASE_NULL(_tilesetList);
	CC_SAFE_RELEASE_NULL(_fontList);
	CC_SAFE_RELEASE_NULL(_imageList);
	CC_SAFE_RELEASE_NULL(_movieList);
	CC_SAFE_RELEASE_NULL(_bgmList);
	CC_SAFE_RELEASE_NULL(_seList);
	CC_SAFE_RELEASE_NULL(_voiceList);
	CC_SAFE_RELEASE_NULL(_variableList);
	CC_SAFE_RELEASE_NULL(_switchList);
	CC_SAFE_RELEASE_NULL(_textList);
	CC_SAFE_RELEASE_NULL(_animationOnlyList);
	CC_SAFE_RELEASE_NULL(_objectList);
	CC_SAFE_RELEASE_NULL(_animationList);
	CC_SAFE_RELEASE_NULL(_objectActionProgramSettings);
	CC_SAFE_RELEASE_NULL(_animParticleImageList);
	CC_SAFE_RELEASE_NULL(_transitionFlow);
	CC_SAFE_RELEASE_NULL(_transitionPortalList);
	CC_SAFE_RELEASE_NULL(_playerCharacterData);
	CC_SAFE_RELEASE_NULL(_pluginList);
	CC_SAFE_RELEASE_NULL(_objectGroup);
	CC_SAFE_RELEASE_NULL(_databaseList);
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif
}

bool ProjectData::init(const rapidjson::Value& json, const std::string &projectPath)
{
	CC_ASSERT(json.HasMember("gameIcon"));
	this->setGameIcon(cocos2d::__String::create(json["gameIcon"].GetString()));
	CC_ASSERT(json.HasMember("gameType"));
	this->setGameType((EnumGameType)json["gameType"].GetInt());
	if (json.HasMember("playerCount")) {
		this->setPlayerCount(json["playerCount"].GetInt());
	}
// #AGTK-NX, #AGTK-WIN
#ifdef USE_30FPS
	if (json.HasMember("mode30Fps")) {
		this->setMode30Fps(json["mode30Fps"].GetBool());
	}
#endif
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX) && defined(USE_NX60FPS)
#endif
	CC_ASSERT(json.HasMember("tileWidth"));
	this->setTileWidth(json["tileWidth"].GetInt());
	CC_ASSERT(json.HasMember("tileHeight"));
	this->setTileHeight(json["tileHeight"].GetInt());
	CC_ASSERT(json.HasMember("screenWidth"));
	this->setScreenWidth(json["screenWidth"].GetInt());
	CC_ASSERT(json.HasMember("screenHeight"));
	this->setScreenHeight(json["screenHeight"].GetInt());
	CC_ASSERT(json.HasMember("screenSettings"));
	this->setScreenSettings(json["screenSettings"].GetInt());
	this->setInitScreenSettings(this->getScreenSettings());
	if (json.HasMember("magnifyWindow")) {
		this->setMagnifyWindow(json["magnifyWindow"].GetBool());
		this->setInitMagnifyWindow(this->getMagnifyWindow());
	}
	if (json.HasMember("windowMagnification")) {
		this->setWindowMagnification(json["windowMagnification"].GetInt());
		this->setInitWindowMagnification(this->getWindowMagnification());
	}
	if (json.HasMember("adjustPixelMagnification")) {
		this->setAdjustPixelMagnification(json["adjustPixelMagnification"].GetBool());
	}
	if (json.HasMember("pixelMagnificationType")) {
		this->setPixelMagnificationType(json["pixelMagnificationType"].GetInt());
	}
	CC_ASSERT(json.HasMember("displayMenuBar"));
	this->setDisplayMenuBar(json["displayMenuBar"].GetBool());
	if (json.HasMember("displayMousePointer")) {
		this->setDisplayMousePointer(json["displayMousePointer"].GetBool());
	}
	if (json.HasMember("loadingSceneId")) {
		this->setLoadingSceneId(json["loadingSceneId"].GetInt());
	}
	if (json.HasMember("wallDetectionOverlapMargin")) {
		this->setWallDetectionOverlapMargin(json["wallDetectionOverlapMargin"].GetDouble());
	}
	if (json.HasMember("multithreading")) {
		this->setMultithreading(json["multithreading"].GetBool());
	}
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif

	CC_ASSERT(json.HasMember("screenEffectValueList"));
	auto screenEffectValueList = cocos2d::__Array::create();
	for (rapidjson::SizeType i = 0; i < json["screenEffectValueList"].Size(); i++) {
		screenEffectValueList->addObject(cocos2d::Double::create(json["screenEffectValueList"][i].GetDouble()));
	}
	CC_ASSERT(json.HasMember("screenEffectFlagList"));
	this->setScreenEffectValueList(screenEffectValueList);
	auto screenEffectFlagList = cocos2d::__Array::create();
	for (rapidjson::SizeType i = 0; i < json["screenEffectFlagList"].Size(); i++) {
		screenEffectFlagList->addObject(cocos2d::Bool::create(json["screenEffectFlagList"][i].GetBool()));
	}
	this->setScreenEffectFlagList(screenEffectFlagList);
	CC_ASSERT(json.HasMember("gameInformation"));
	this->setGameInformation(GameInformationData::create(json["gameInformation"]));
	CC_ASSERT(json.HasMember("soundSetting"));
	this->setSoundSetting(SoundSettingData::create(json["soundSetting"]));
	CC_ASSERT(json.HasMember("actionProgramSetting"));
	this->setActionProgramSetting(ActionProgramSettingData::create(json["actionProgramSetting"]));
	CC_ASSERT(json.HasMember("inputMapping"));
	this->setInputMapping(InputMappingData::create(json["inputMapping"]));
	this->setInitInputMapping(InputMappingData::create(*this->getInputMapping()));

	CC_ASSERT(json.HasMember("sceneList"));
	auto sceneList = cocos2d::__Dictionary::create();
	for (rapidjson::SizeType i = 0; i < json["sceneList"].Size(); i++) {
		auto sceneData = SceneData::create(json["sceneList"][i], this->getTileSize());
#if defined(AGTK_DEBUG)
		CC_ASSERT(sceneList->objectForKey(sceneData->getId()) == nullptr);
#endif
		sceneList->setObject(sceneData, sceneData->getId());
	}
	this->setSceneList(sceneList);

	CC_ASSERT(json.HasMember("tilesetList"));
	auto tilesetList = cocos2d::__Dictionary::create();
	for (rapidjson::SizeType i = 0; i < json["tilesetList"].Size(); i++) {
		auto tilesetData = TilesetData::create(json["tilesetList"][i]);
#if defined(AGTK_DEBUG)
		CC_ASSERT(tilesetList->objectForKey(tilesetData->getId()) == nullptr);
#endif
		tilesetList->setObject(tilesetData, tilesetData->getId());
	}
	this->setTilesetList(tilesetList);

	CC_ASSERT(json.HasMember("fontList"));
	auto fontList = cocos2d::__Dictionary::create();
	for (rapidjson::SizeType i = 0; i < json["fontList"].Size(); i++) {
		auto fontData = FontData::create(json["fontList"][i]);
#if defined(AGTK_DEBUG)
		CC_ASSERT(fontList->objectForKey(fontData->getId()) == nullptr);
#endif
		fontList->setObject(fontData, fontData->getId());
	}
	this->setFontList(fontList);

	CC_ASSERT(json.HasMember("imageList"));
	auto imageList = cocos2d::__Dictionary::create();
	for (rapidjson::SizeType i = 0; i < json["imageList"].Size(); i++) {
		auto imageData = ImageData::create(json["imageList"][i], projectPath);
#if defined(AGTK_DEBUG)
		CC_ASSERT(imageList->objectForKey(imageData->getId()) == nullptr);
#endif
		imageList->setObject(imageData, imageData->getId());
	}
	this->setImageList(imageList);

	CC_ASSERT(json.HasMember("movieList"));
	auto movieList = cocos2d::__Dictionary::create();
	for (rapidjson::SizeType i = 0; i < json["movieList"].Size(); i++) {
		auto movieData = MovieData::create(json["movieList"][i], projectPath);
#if defined(AGTK_DEBUG)
		CC_ASSERT(movieList->objectForKey(movieData->getId()) == nullptr);
#endif
		movieList->setObject(movieData, movieData->getId());
	}
	this->setMovieList(movieList);

	CC_ASSERT(json.HasMember("bgmList"));
	auto bgmList = cocos2d::__Dictionary::create();
	for (rapidjson::SizeType i = 0; i < json["bgmList"].Size(); i++) {
		auto bgmData = BgmData::create(json["bgmList"][i], projectPath);
#if defined(AGTK_DEBUG)
		CC_ASSERT(bgmList->objectForKey(bgmData->getId()) == nullptr);
#endif
		bgmList->setObject(bgmData, bgmData->getId());
	}
	this->setBgmList(bgmList);

	CC_ASSERT(json.HasMember("seList"));
	auto seList = cocos2d::__Dictionary::create();
	for (rapidjson::SizeType i = 0; i < json["seList"].Size(); i++) {
		auto seData = SeData::create(json["seList"][i], projectPath);
#if defined(AGTK_DEBUG)
		CC_ASSERT(seList->objectForKey(seData->getId()) == nullptr);
#endif
		seList->setObject(seData, seData->getId());
	}
	this->setSeList(seList);

	CC_ASSERT(json.HasMember("voiceList"));
	auto voiceList = cocos2d::__Dictionary::create();
	for (rapidjson::SizeType i = 0; i < json["voiceList"].Size(); i++) {
		auto voiceData = VoiceData::create(json["voiceList"][i], projectPath);
#if defined(AGTK_DEBUG)
		CC_ASSERT(voiceList->objectForKey(voiceData->getId()) == nullptr);
#endif
		voiceList->setObject(voiceData, voiceData->getId());
	}
	this->setVoiceList(voiceList);

	CC_ASSERT(json.HasMember("variableList"));
	auto variableList = cocos2d::__Dictionary::create();
	for (rapidjson::SizeType i = 0; i < json["variableList"].Size(); i++) {
		auto variableData = VariableData::create(json["variableList"][i]);
#if defined(AGTK_DEBUG)
		CC_ASSERT(variableList->objectForKey(variableData->getId()) == nullptr);
#endif
		variableList->setObject(variableData, variableData->getId());
	}
	this->setVariableList(variableList);

	CC_ASSERT(json.HasMember("switchList"));
	auto switchList = cocos2d::__Dictionary::create();
	for (rapidjson::SizeType i = 0; i < json["switchList"].Size(); i++) {
		auto switchData = SwitchData::create(json["switchList"][i]);
#if defined(AGTK_DEBUG)
		CC_ASSERT(switchList->objectForKey(switchData->getId()) == nullptr);
#endif
		switchList->setObject(switchData, switchData->getId());
	}
	this->setSwitchList(switchList);

	CC_ASSERT(json.HasMember("textList"));
	auto textList = cocos2d::__Dictionary::create();
	for (rapidjson::SizeType i = 0; i < json["textList"].Size(); i++) {
		auto textData = TextData::create(json["textList"][i], this->getGameInformation()->getLanguageList());
#if defined(AGTK_DEBUG)
		CC_ASSERT(textList->objectForKey(textData->getId()) == nullptr);
#endif
		textList->setObject(textData, textData->getId());
	}
	this->setTextList(textList);

	CC_ASSERT(json.HasMember("animationOnlyList"));
	auto animationOnlyList = cocos2d::__Dictionary::create();
	for (rapidjson::SizeType i = 0; i < json["animationOnlyList"].Size(); i++) {
		auto animationOnlyData = AnimationOnlyData::create(json["animationOnlyList"][i], projectPath);
#if defined(AGTK_DEBUG)
		CC_ASSERT(animationOnlyList->objectForKey(animationOnlyData->getId()) == nullptr);
#endif
		animationOnlyList->setObject(animationOnlyData, animationOnlyData->getId());
	}
	this->setAnimationOnlyList(animationOnlyList);

	CC_ASSERT(json.HasMember("objectList"));
	auto objectList = cocos2d::__Dictionary::create();
	for (rapidjson::SizeType i = 0; i < json["objectList"].Size(); i++) {
		auto objectData = ObjectData::create(json["objectList"][i]);
#if defined(AGTK_DEBUG)
		CC_ASSERT(objectList->objectForKey(objectData->getId()) == nullptr);
#endif
		objectList->setObject(objectData, objectData->getId());
	}
	this->setObjectList(objectList);

	CC_ASSERT(json.HasMember("animationList"));
	auto animationList = cocos2d::__Dictionary::create();
	for (rapidjson::SizeType i = 0; i < json["animationList"].Size(); i++) {
		auto animationData = AnimationData::create(json["animationList"][i]);
#if defined(AGTK_DEBUG)
		CC_ASSERT(animationList->objectForKey(animationData->getId()) == nullptr);
#endif
		animationList->setObject(animationData, animationData->getId());
	}
	this->setAnimationList(animationList);

// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif
	CC_ASSERT(json.HasMember("databaseList"));
	auto databaseList = cocos2d::__Dictionary::create();
	for (rapidjson::SizeType i = 0; i < json["databaseList"].Size(); i++) {
		auto databaseData = DatabaseData::create(json["databaseList"][i]);
#if defined(AGTK_DEBUG)
		CC_ASSERT(databaseList->objectForKey(databaseData->getId()) == nullptr);
#endif
		databaseList->setObject(databaseData, databaseData->getId());
	}
	this->setDatabaseList(databaseList);
	// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif

	CC_ASSERT(json.HasMember("objectActionProgramSettings"));
	this->setObjectActionProgramSettings(ObjectActionProgramSettingsData::create(json["objectActionProgramSettings"]));

	// -----------------------------------------------------
	// パーティクルの画像を読み込む
	// -----------------------------------------------------
	auto animParticleImageList = cocos2d::__Dictionary::create();

	// 組込みパーティクル画像の読み込み
	// ※IDは負数になるが画像名は二桁の正数となることに注意
	for (int id = AnimParticleImageData::BUILT_IN_PARTICLE_ID_START; id <= AnimParticleImageData::BUILT_IN_PARTICLE_ID_MAX; id++) {
		auto animParticleImageData = AnimParticleImageData::create(-id);
		animParticleImageData->setFilename(cocos2d::String::createWithFormat("img/particle%02d.png", id));
		animParticleImageList->setObject(animParticleImageData, animParticleImageData->getId());
	}

	// ユーザーが追加したパーティクル画像の読み込み
	// IDは1から始まる正数値で画像名は"三桁"であることに注意
	CC_ASSERT(json.HasMember("animParticleImageList"));
	for (rapidjson::SizeType i = 0; i < json["animParticleImageList"].Size(); i++) {
		auto animParticleImageData = AnimParticleImageData::create(json["animParticleImageList"][i]);
		auto id = animParticleImageData->getId();
#ifdef USE_PREVIEW
		animParticleImageData->setFilename(cocos2d::String::create(getFullFilename(animParticleImageData->getFilename()->getCString(), projectPath)));
#else
		animParticleImageData->setFilename(cocos2d::String::create(projectPath + cocos2d::String::createWithFormat("img/particle%03d.png", id)->getCString()));
#endif
#if defined(AGTK_DEBUG)
		CC_ASSERT(animParticleImageList->objectForKey(animParticleImageData->getId()) == nullptr);
#endif
		animParticleImageList->setObject(animParticleImageData, animParticleImageData->getId());
	}
	this->setAnimParticleImageList(animParticleImageList);

	// -----------------------------------------------------
	// 画面フロー設定データ読み込み
	// -----------------------------------------------------
	CHECK_JSON_MENBER("transitionFlow");
	auto transitionFlowData = TransitionFlowData::create(json["transitionFlow"]);
	CC_ASSERT(transitionFlowData);
	this->setTransitionFlow(transitionFlowData);

	// -----------------------------------------------------
	// ポータル移動設定データ読み込み
	// -----------------------------------------------------
	CHECK_JSON_MENBER("transitionPortalList");
	auto transitionPortalList = cocos2d::__Array::create();
	for (rapidjson::SizeType i = 0; i < json["transitionPortalList"].Size(); i++) {
		auto p = TransitionPortalData::create(json["transitionPortalList"][i]);
		CC_ASSERT(p);
		transitionPortalList->addObject(p);
	}
	this->setTransitionPortalList(transitionPortalList);

	CHECK_JSON_MENBER("playerCharacter");
	auto playerCharacterData = PlayerCharacterData::create(json["playerCharacter"]);
	this->setPlayerCharacterData(playerCharacterData);

	// -----------------------------------------------------
	// プラグイン設定データ読み込み
	// -----------------------------------------------------
	CC_ASSERT(json.HasMember("pluginList"));
	auto pluginList = cocos2d::__Dictionary::create();
	for (rapidjson::SizeType i = 0; i < json["pluginList"].Size(); i++) {
		auto pluginData = PluginData::create(json["pluginList"][i], projectPath);
#if defined(AGTK_DEBUG)
		CC_ASSERT(pluginList->objectForKey(pluginData->getId()) == nullptr);
#endif
		pluginList->setObject(pluginData, pluginData->getId());
	}
	this->setPluginList(pluginList);

	// オブジェクトグループ
	{
		char const * const member = "objectGroup";
		CC_ASSERT(json.HasMember(member));
		cocos2d::__Array* ary = cocos2d::Array::create();
		for (rapidjson::SizeType i = 0; i < json[member].Size(); ++i) {
			ary->addObject(cocos2d::__String::create(json[member][i].GetString()));
		}
		setObjectGroup(ary);
	}

// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif

#if defined(AGTK_DEBUG)
	this->dump();
#endif

	return true;
}

rapidjson::Document ProjectData::serializeConfig()const
{
	rapidjson::Document doc;
	doc.SetObject();
	rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();

	rapidjson::Value displayData(rapidjson::kObjectType);
	displayData.AddMember("screenSettings", getScreenSettings(), allocator);
	displayData.AddMember("magnifyWindow", getMagnifyWindow(), allocator);
	displayData.AddMember("windowMagnification", getWindowMagnification(), allocator);

	rapidjson::Value v;
	v.SetString(getGameInformation()->getMainLanguage(), allocator);
	displayData.AddMember("language", v, allocator);
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif

	doc.AddMember("displayData", displayData, allocator);

	doc.AddMember("soundData", getSoundSetting()->serialize(allocator), allocator);

	doc.AddMember("inputMapping", getInputMapping()->serialize(allocator), allocator);
#if defined(AGTK_DEBUG)
	// シリアライズ、デシリアライズ動作テスト
	//InputMappingData const * imd = InputMappingData::create(doc["inputMapping"]);
	//CCASSERT(*getInputMapping() == *imd,"invalid deserialize");
#endif
	return doc;
}
void ProjectData::deserializeConfig(const rapidjson::Document& doc)
{
	const auto & displayData = doc["displayData"];
	setScreenSettings(displayData["screenSettings"].GetInt());
	setMagnifyWindow(displayData["magnifyWindow"].GetBool());
	setWindowMagnification(displayData["windowMagnification"].GetInt());

	const char *language = nullptr;
	auto gameInformationData = getGameInformation();
	if (displayData["language"].IsString()) {
		language = displayData["language"].GetString();
	}
	else {
		// 後方互換性。
		language = gameInformationData->getLanguage(displayData["language"].GetInt());
	}
	if (language && gameInformationData->getLanguageId(language) >= 0) {
		// 保存されていた言語に対応していればそれをメイン言語にする。
		gameInformationData->setMainLanguage(cocos2d::__String::create(language));
	}
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif

	getSoundSetting()->deserialize(doc["soundData"]);
	
	getInputMapping()->deserialize(doc["inputMapping"]);
}

const char *ProjectData::getGameIcon()
{
	CC_ASSERT(_gameIcon);
	return _gameIcon->getCString();
}

cocos2d::Size ProjectData::getTileSize()
{
	return cocos2d::Size(_tileWidth, _tileHeight);
}

cocos2d::Size ProjectData::getScreenSize()
{
	return cocos2d::Size(_screenWidth, _screenHeight);
}

/**
 * 指定シーンデータのシーンのドット単位の(幅、高さ）を返す。
 */
cocos2d::Size ProjectData::getSceneSize(SceneData *sceneData)
{
	CC_ASSERT(sceneData);
	auto horzScreenCount = sceneData->getHorzScreenCount();
	auto vertScreenCount = sceneData->getVertScreenCount();
	auto screenWidth = this->getScreenWidth();
	auto screenHeight = this->getScreenHeight();
	return cocos2d::Size(screenWidth * horzScreenCount, screenHeight * vertScreenCount);
}

bool ProjectData::getScreenEffect(int id, float *value)
{
	auto flag = dynamic_cast<cocos2d::__Bool *>(this->getScreenEffectFlagList()->getObjectAtIndex(id));
	auto v = dynamic_cast<cocos2d::__Double *>(this->getScreenEffectValueList()->getObjectAtIndex(id));
	if (value) {
		*value = v->getValue();
	}
	return flag->getValue();
}

int ProjectData::getScreenEffectCount()
{
	int count = 0;
	auto screenEffectFlagList = this->getScreenEffectFlagList();
	cocos2d::Ref *ref;
	CCARRAY_FOREACH(screenEffectFlagList, ref) {
		auto flag = dynamic_cast<cocos2d::__Bool *>(ref);
		if (flag->getValue()) {
			count++;
		}
	}
	return count;
}

SceneData *ProjectData::getSceneData(int id)
{
	std::function<SceneData *(int, cocos2d::__Dictionary *)> func = [&](int id, cocos2d::__Dictionary *children)->SceneData* {
		cocos2d::DictElement *el = nullptr;
		CCDICT_FOREACH(children, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto p = static_cast<agtk::data::SceneData *>(el->getObject());
#else
			auto p = dynamic_cast<agtk::data::SceneData *>(el->getObject());
#endif

			if (p->getId() == id) {
				return p;
			}
			if (p->getChildren()) {
				p = func(id, p->getChildren());
				if (p) {
					return p;
				}
			}
		}
		return nullptr;
	};
	return func(id, this->getSceneList());
}

SceneData *ProjectData::getSceneDataByName(const char *name)
{
	std::function<SceneData *(const char *, cocos2d::__Dictionary *)> func = [&](const char *name, cocos2d::__Dictionary *children)->SceneData* {
		cocos2d::DictElement *el = nullptr;
		CCDICT_FOREACH(children, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto p = static_cast<agtk::data::SceneData *>(el->getObject());
#else
			auto p = dynamic_cast<agtk::data::SceneData *>(el->getObject());
#endif
			if (strcmp(p->getName(), name) == 0) {
				return p;
			}
			if (p->getChildren()) {
				p = func(name, p->getChildren());
				if (p) {
					return p;
				}
			}
		}
		return nullptr;
	};
	return func(name, this->getSceneList());
}

SceneData *ProjectData::getMenuSceneData()
{
	return this->getSceneData(data::SceneData::kMenuSceneId);
}

TilesetData *ProjectData::getTilesetData(int id)
{
	std::function<TilesetData *(int, cocos2d::__Dictionary *)> func = [&](int id, cocos2d::__Dictionary *children)->TilesetData* {
		cocos2d::DictElement *el = nullptr;
		CCDICT_FOREACH(children, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto p = static_cast<agtk::data::TilesetData *>(el->getObject());
#else
			auto p = dynamic_cast<agtk::data::TilesetData *>(el->getObject());
#endif
			if (p->getId() == id) {
				return p;
			}
			if (p->getChildren()) {
				p = func(id, p->getChildren());
				if (p) {
					return p;
				}
			}
		}
		return nullptr;
	};
	return func(id, this->getTilesetList());
}

TilesetData *ProjectData::getTilesetDataByName(const char *name)
{
	std::function<TilesetData *(const char *, cocos2d::__Dictionary *)> func = [&](const char *name, cocos2d::__Dictionary *children)->TilesetData* {
		cocos2d::DictElement *el = nullptr;
		CCDICT_FOREACH(children, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto p = static_cast<agtk::data::TilesetData *>(el->getObject());
#else
			auto p = dynamic_cast<agtk::data::TilesetData *>(el->getObject());
#endif
			if (strcmp(p->getName(), name) == 0) {
				return p;
			}
			if (p->getChildren()) {
				p = func(name, p->getChildren());
				if (p) {
					return p;
				}
			}
		}
		return nullptr;
	};
	return func(name, this->getTilesetList());
}

FontData *ProjectData::getFontData(int id)
{
	std::function<FontData *(int, cocos2d::__Dictionary *)> func = [&](int id, cocos2d::__Dictionary *children)->FontData* {
		cocos2d::DictElement *el = nullptr;
		CCDICT_FOREACH(children, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto p = static_cast<agtk::data::FontData *>(el->getObject());
#else
			auto p = dynamic_cast<agtk::data::FontData *>(el->getObject());
#endif
			if (p->getId() == id) {
				return p;
			}
			if (p->getChildren()) {
				p = func(id, p->getChildren());
				if (p) {
					return p;
				}
			}
		}
		return nullptr;
	};
	return func(id, this->getFontList());
}

FontData *ProjectData::getFontDataByName(const char *name)
{
	std::function<FontData *(const char *, cocos2d::__Dictionary *)> func = [&](const char *name, cocos2d::__Dictionary *children)->FontData* {
		cocos2d::DictElement *el = nullptr;
		CCDICT_FOREACH(children, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto p = static_cast<agtk::data::FontData *>(el->getObject());
#else
			auto p = dynamic_cast<agtk::data::FontData *>(el->getObject());
#endif
			if (strcmp(p->getName(), name) == 0) {
				return p;
			}
			if (p->getChildren()) {
				p = func(name, p->getChildren());
				if (p) {
					return p;
				}
			}
		}
		return nullptr;
	};
	return func(name, this->getFontList());
}

ImageData *ProjectData::getImageData(int id)
{
	CC_ASSERT(this->getImageList());
	return this->getImageData(id, this->getImageList());
}

ImageData *ProjectData::getImageData(int id, cocos2d::__Dictionary *children)
{
	cocos2d::DictElement *el = nullptr;
	CCDICT_FOREACH(children, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		ImageData *p = static_cast<ImageData *>(el->getObject());
#else
		ImageData *p = dynamic_cast<ImageData *>(el->getObject());
#endif
		if (p->getId() == id) {
			return p;
		}
		if (p->getChildren()) {
			p = this->getImageData(id, p->getChildren());
			if (p) {
				return p;
			}
		}
	}
	return nullptr;
}

ImageData *ProjectData::getImageDataByName(const char *name)
{
	std::function<ImageData *(const char *, cocos2d::__Dictionary *)> func = [&](const char *name, cocos2d::__Dictionary *children)->ImageData* {
		cocos2d::DictElement *el = nullptr;
		CCDICT_FOREACH(children, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto p = static_cast<agtk::data::ImageData *>(el->getObject());
#else
			auto p = dynamic_cast<agtk::data::ImageData *>(el->getObject());
#endif
			if (strcmp(p->getName(), name) == 0) {
				return p;
			}
			if (p->getChildren()) {
				p = func(name, p->getChildren());
				if (p) {
					return p;
				}
			}
		}
		return nullptr;
	};
	return func(name, this->getImageList());
}

TextData *ProjectData::getTextData(int id)
{
	std::function<TextData *(int, cocos2d::__Dictionary *)> func = [&](int id, cocos2d::__Dictionary *children)->TextData* {
		cocos2d::DictElement *el = nullptr;
		CCDICT_FOREACH(children, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto p = static_cast<agtk::data::TextData *>(el->getObject());
#else
			auto p = dynamic_cast<agtk::data::TextData *>(el->getObject());
#endif
			if (p->getId() == id) {
				return p;
			}
			if (p->getChildren()) {
				p = func(id, p->getChildren());
				if (p) {
					return p;
				}
			}
		}
		return nullptr;
	};
	return func(id, this->getTextList());
}

TextData *ProjectData::getTextDataByName(const char *name)
{
	std::function<TextData *(const char *, cocos2d::__Dictionary *)> func = [&](const char *name, cocos2d::__Dictionary *children)->TextData* {
		cocos2d::DictElement *el = nullptr;
		CCDICT_FOREACH(children, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto p = static_cast<agtk::data::TextData *>(el->getObject());
#else
			auto p = dynamic_cast<agtk::data::TextData *>(el->getObject());
#endif
			if (strcmp(p->getName()->getCString(), name) == 0) {
				return p;
			}
			if (p->getChildren()) {
				p = func(name, p->getChildren());
				if (p) {
					return p;
				}
			}
		}
		return nullptr;
	};
	return func(name, this->getTextList());
}

MovieData *ProjectData::getMovieData(int id)
{
	CC_ASSERT(this->getMovieList());
	return this->getMovieData(id, this->getMovieList());
}

MovieData *ProjectData::getMovieData(int id, cocos2d::__Dictionary *children)
{
	cocos2d::DictElement *el = nullptr;
	CCDICT_FOREACH(children, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		MovieData *p = static_cast<MovieData *>(el->getObject());
#else
		MovieData *p = dynamic_cast<MovieData *>(el->getObject());
#endif
		if (p->getId() == id) {
			return p;
		}
		if (p->getChildren()) {
			p = this->getMovieData(id, p->getChildren());
			if (p) {
				return p;
			}
		}
	}
	return nullptr;
}

MovieData *ProjectData::getMovieDataByName(const char *name)
{
	std::function<MovieData *(const char *, cocos2d::__Dictionary *)> func = [&](const char *name, cocos2d::__Dictionary *children)->MovieData* {
		cocos2d::DictElement *el = nullptr;
		CCDICT_FOREACH(children, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto p = static_cast<agtk::data::MovieData *>(el->getObject());
#else
			auto p = dynamic_cast<agtk::data::MovieData *>(el->getObject());
#endif
			if (strcmp(p->getName(), name) == 0) {
				return p;
			}
			if (p->getChildren()) {
				p = func(name, p->getChildren());
				if (p) {
					return p;
				}
			}
		}
		return nullptr;
	};
	return func(name, this->getMovieList());
}

BgmData *ProjectData::getBgmData(int id)
{
	CC_ASSERT(this->getBgmList());
	return this->getBgmData(id, this->getBgmList());
}

BgmData *ProjectData::getBgmData(int id, cocos2d::__Dictionary *children)
{
	cocos2d::DictElement *el = nullptr;
	CCDICT_FOREACH(children, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		BgmData *p = static_cast<BgmData *>(el->getObject());
#else
		BgmData *p = dynamic_cast<BgmData *>(el->getObject());
#endif
		if (p->getId() == id) {
			return p;
		}
		if (p->getChildren()) {
			p = this->getBgmData(id, p->getChildren());
			if (p) {
				return p;
			}
		}
	}
	return nullptr;
}

BgmData *ProjectData::getBgmDataByName(const char *name)
{
	std::function<BgmData *(const char *, cocos2d::__Dictionary *)> func = [&](const char *name, cocos2d::__Dictionary *children)->BgmData* {
		cocos2d::DictElement *el = nullptr;
		CCDICT_FOREACH(children, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto p = static_cast<agtk::data::BgmData *>(el->getObject());
#else
			auto p = dynamic_cast<agtk::data::BgmData *>(el->getObject());
#endif
			if (strcmp(p->getName(), name) == 0) {
				return p;
			}
			if (p->getChildren()) {
				p = func(name, p->getChildren());
				if (p) {
					return p;
				}
			}
		}
		return nullptr;
	};
	return func(name, this->getBgmList());
}

SeData *ProjectData::getSeData(int id)
{
	CC_ASSERT(this->getSeList());
	return this->getSeData(id, this->getSeList());
}

SeData *ProjectData::getSeData(int id, cocos2d::__Dictionary *children)
{
	cocos2d::DictElement *el = nullptr;
	CCDICT_FOREACH(children, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		SeData *p = static_cast<SeData *>(el->getObject());
#else
		SeData *p = dynamic_cast<SeData *>(el->getObject());
#endif
		if (p->getId() == id) {
			return p;
		}
		if (p->getChildren()) {
			p = this->getSeData(id, p->getChildren());
			if (p) {
				return p;
			}
		}
	}
	return nullptr;
}

SeData *ProjectData::getSeDataByName(const char *name)
{
	std::function<SeData *(const char *, cocos2d::__Dictionary *)> func = [&](const char *name, cocos2d::__Dictionary *children)->SeData* {
		cocos2d::DictElement *el = nullptr;
		CCDICT_FOREACH(children, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto p = static_cast<agtk::data::SeData *>(el->getObject());
#else
			auto p = dynamic_cast<agtk::data::SeData *>(el->getObject());
#endif
			if (strcmp(p->getName(), name) == 0) {
				return p;
			}
			if (p->getChildren()) {
				p = func(name, p->getChildren());
				if (p) {
					return p;
				}
			}
		}
		return nullptr;
	};
	return func(name, this->getSeList());
}

VoiceData *ProjectData::getVoiceData(int id)
{
	CC_ASSERT(this->getVoiceList());
	return this->getVoiceData(id, this->getVoiceList());
}

VoiceData *ProjectData::getVoiceData(int id, cocos2d::__Dictionary *children)
{
	cocos2d::DictElement *el = nullptr;
	CCDICT_FOREACH(children, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto p = static_cast<VoiceData *>(el->getObject());
#else
		auto p = dynamic_cast<VoiceData *>(el->getObject());
#endif
		if (p->getId() == id) {
			return p;
		}
		if (p->getChildren()) {
			p = this->getVoiceData(id, p->getChildren());
			if (p) {
				return p;
			}
		}
	}
	return nullptr;
}

VoiceData *ProjectData::getVoiceDataByName(const char *name)
{
	std::function<VoiceData *(const char *, cocos2d::__Dictionary *)> func = [&](const char *name, cocos2d::__Dictionary *children)->VoiceData* {
		cocos2d::DictElement *el = nullptr;
		CCDICT_FOREACH(children, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto p = static_cast<agtk::data::VoiceData *>(el->getObject());
#else
			auto p = dynamic_cast<agtk::data::VoiceData *>(el->getObject());
#endif
			if (strcmp(p->getName(), name) == 0) {
				return p;
			}
			if (p->getChildren()) {
				p = func(name, p->getChildren());
				if (p) {
					return p;
				}
			}
		}
		return nullptr;
	};
	return func(name, this->getVoiceList());
}

VariableData *ProjectData::getVariableData(int id)
{
	CC_ASSERT(this->getVariableList());
	return this->getVariableData(id, this->getVariableList());
}

VariableData *ProjectData::getVariableData(int id, cocos2d::__Dictionary *children)
{
	cocos2d::DictElement *el = nullptr;
	CCDICT_FOREACH(children, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto p = static_cast<VariableData *>(el->getObject());
#else
		auto p = dynamic_cast<VariableData *>(el->getObject());
#endif
		if (p->getId() == id) {
			return p;
		}
		if (p->getChildren()) {
			p = this->getVariableData(id, p->getChildren());
			if (p) {
				return p;
			}
		}
	}
	return nullptr;
}

SwitchData *ProjectData::getSwitchData(int id)
{
	CC_ASSERT(this->getSwitchList());
	return this->getSwitchData(id, this->getSwitchList());
}

SwitchData *ProjectData::getSwitchData(int id, cocos2d::__Dictionary *children)
{
	cocos2d::DictElement *el = nullptr;
	CCDICT_FOREACH(children, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto p = static_cast<SwitchData *>(el->getObject());
#else
		auto p = dynamic_cast<SwitchData *>(el->getObject());
#endif
		if (p->getId() == id) {
			return p;
		}
		if (p->getChildren()) {
			p = this->getSwitchData(id, p->getChildren());
			if (p) {
				return p;
			}
		}
	}
	return nullptr;
}

AnimationData *ProjectData::getAnimationData(int id)
{
	CC_ASSERT(this->getAnimationList());
	return this->getAnimationData(id, this->getAnimationList());
}

AnimationData *ProjectData::getAnimationData(int id, cocos2d::__Dictionary *children)
{
	cocos2d::DictElement *el = nullptr;
	CCDICT_FOREACH(children, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto p = static_cast<AnimationData *>(el->getObject());
#else
		auto p = dynamic_cast<AnimationData *>(el->getObject());
#endif
		if (p->getId() == id) {
			return p;
		}
		if (p->getChildren()) {
			p = this->getAnimationData(id, p->getChildren());
			if (p) {
				return p;
			}
		}
	}
	return nullptr;
}

AnimationData *ProjectData::getAnimationDataByName(const char *name)
{
	std::function<AnimationData *(const char *, cocos2d::__Dictionary *)> func = [&](const char *name, cocos2d::__Dictionary *children)->AnimationData* {
		cocos2d::DictElement *el = nullptr;
		CCDICT_FOREACH(children, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto p = static_cast<agtk::data::AnimationData *>(el->getObject());
#else
			auto p = dynamic_cast<agtk::data::AnimationData *>(el->getObject());
#endif
			if (strcmp(p->getName(), name) == 0) {
				return p;
			}
			if (p->getChildren()) {
				p = func(name, p->getChildren());
				if (p) {
					return p;
				}
			}
		}
		return nullptr;
	};
	return func(name, this->getAnimationList());
}

cocos2d::__Array *ProjectData::getAnimationDataAllKeys()
{
	auto arr = cocos2d::__Array::create();
	std::function<void(cocos2d::__Dictionary *, cocos2d::__Array *)> func = [&](cocos2d::__Dictionary *children, cocos2d::__Array *arr) {
		cocos2d::DictElement *el = nullptr;
		CCDICT_FOREACH(children, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto p = static_cast<agtk::data::AnimationData *>(el->getObject());
#else
			auto p = dynamic_cast<agtk::data::AnimationData *>(el->getObject());
#endif
			arr->addObject(cocos2d::Integer::create(p->getId()));
			if (p->getChildren()) {
				func(p->getChildren(), arr);
			}
		}
	};
	func(this->getAnimationList(), arr);
	return arr;
}

AnimationOnlyData *ProjectData::getAnimationOnlyData(int id)
{
	std::function<AnimationOnlyData *(int, cocos2d::__Dictionary *)> func = [&](int id, cocos2d::__Dictionary *children)->AnimationOnlyData* {
		cocos2d::DictElement *el = nullptr;
		CCDICT_FOREACH(children, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto p = static_cast<agtk::data::AnimationOnlyData *>(el->getObject());
#else
			auto p = dynamic_cast<agtk::data::AnimationOnlyData *>(el->getObject());
#endif
			if (p->getId() == id) {
				return p;
			}
			if (p->getChildren()) {
				p = func(id, p->getChildren());
				if (p) {
					return p;
				}
			}
		}
		return nullptr;
	};
	return func(id, this->getAnimationOnlyList());
}

ObjectData *ProjectData::getObjectData(int id)
{
	std::function<ObjectData *(int, cocos2d::__Dictionary *)> func = [&](int id, cocos2d::__Dictionary *children)->ObjectData* {
		cocos2d::DictElement *el = nullptr;
		CCDICT_FOREACH(children, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto p = static_cast<agtk::data::ObjectData *>(el->getObject());
#else
			auto p = dynamic_cast<agtk::data::ObjectData *>(el->getObject());
#endif
			if (p->getId() == id) {
				return p;
			}
			if (p->getChildren()) {
				p = func(id, p->getChildren());
				if (p) {
					return p;
				}
			}
		}
		return nullptr;
	};
	return func(id, this->getObjectList());
}

ObjectData *ProjectData::getObjectDataByName(const char *name)
{
	std::function<ObjectData *(const char *, cocos2d::__Dictionary *)> func = [&](const char *name, cocos2d::__Dictionary *children)->ObjectData* {
		cocos2d::DictElement *el = nullptr;
		CCDICT_FOREACH(children, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto p = static_cast<agtk::data::ObjectData *>(el->getObject());
#else
			auto p = dynamic_cast<agtk::data::ObjectData *>(el->getObject());
#endif
			if (strcmp(p->getName(), name) == 0) {
				return p;
			}
			if (p->getChildren()) {
				p = func(name, p->getChildren());
				if (p) {
					return p;
				}
			}
		}
		return nullptr;
	};
	return func(name, this->getObjectList());
}

TransitionPortalData *ProjectData::getTransitionPortalData(int id)
{
	auto portalList = this->getTransitionPortalList();
	cocos2d::Ref *ref = nullptr;
	CCARRAY_FOREACH(portalList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto p = static_cast<agtk::data::TransitionPortalData *>(ref);
#else
		auto p = dynamic_cast<agtk::data::TransitionPortalData *>(ref);
#endif
		if (p->getId() == id) {
			return p;
		}
	}
	return nullptr;
}

TransitionPortalData *ProjectData::getTransitionPortalDataByName(const char *name)
{
	auto portalList = this->getTransitionPortalList();
	cocos2d::Ref *ref = nullptr;
	CCARRAY_FOREACH(portalList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto p = static_cast<agtk::data::TransitionPortalData *>(ref);
#else
		auto p = dynamic_cast<agtk::data::TransitionPortalData *>(ref);
#endif
		if (strcmp(p->getName()->getCString(), name) == 0) {
			return p;
		}
	}
	return nullptr;
}

DatabaseData *ProjectData::getDatabaseData(int id)
{
	CC_ASSERT(this->getDatabaseList());
	return this->getDatabaseData(id, this->getDatabaseList());
}

DatabaseData *ProjectData::getDatabaseData(int id, cocos2d::__Dictionary *children)
{
	cocos2d::DictElement *el = nullptr;
	CCDICT_FOREACH(children, el) {
		auto p = dynamic_cast<DatabaseData *>(el->getObject());
		if (p->getId() == id) {
			return p;
		}
		if (p->getFolder()) {
			p = this->getDatabaseData(id, p->getChildren());
			if (p) {
				return p;
			}
		}
	}
	return nullptr;
}

DatabaseData *ProjectData::getDatabaseDataByName(const char *name)
{
	std::function<DatabaseData *(const char *, cocos2d::__Dictionary *)> func = [&](const char *name, cocos2d::__Dictionary *children)->DatabaseData* {
		cocos2d::DictElement *el = nullptr;
		CCDICT_FOREACH(children, el) {
			auto p = dynamic_cast<agtk::data::DatabaseData *>(el->getObject());
			if (strcmp(p->getName()->getCString(), name) == 0) {
				return p;
			}
			if (p->getChildren()) {
				p = func(name, p->getChildren());
				if (p) {
					return p;
				}
			}
		}
		return nullptr;
	};
	return func(name, this->getDatabaseList());
}

bool ProjectData::isNumber(const char *str, int *pNum)
{
	*pNum = 0;
	if (!str || str[0] == '\0') {
		return false;
	}
	if (strcmp(str, "0") == 0) {
		return true;
	}
	char ch = str[0];
	if (ch >= '1' && ch <= '9') {
		*pNum = (ch - '0');
		str++;
		while (*str) {
			ch = str[0];
			if (ch >= '0' && ch <= '9') {
				*pNum = *pNum * 10 + (ch - '0');
				str++;
			}
			else {
				break;
			}
		}
		if (!*str) {
			return true;
		}
	}
	return false;
}

bool ProjectData::isHexNumber(const char *str, int *pNum)
{
	*pNum = 0;
	if (!str || str[0] == '\0') {
		return false;
	}

	while (*str) {
		auto ch = str[0];
		if (ch >= '0' && ch <= '9') {
			*pNum = *pNum * 16 + (ch - '0');
			str++;
		}
		else if (ch >= 'A' && ch <= 'F') {
			*pNum = *pNum * 16 + (ch - 'A' + 10);
			str++;
		}
		else if (ch >= 'a' && ch <= 'f') {
			*pNum = *pNum * 16 + (ch - 'a' + 10);
			str++;
		}
		else {
			return false;
		}
	}
	return true;
}

std::vector<std::string> ProjectData::stringSplit(const std::string &text, char delimiter)
{
	std::vector<std::string> list;
	int last = 0;
	int head = 0;
	while (head < static_cast<int>(text.length())) {
		if (text[head] == delimiter) {
			list.push_back(text.substr(last, head - last));
			head++;
			last = head;
		}
		else {
			head++;
		}
	}
	if (head > last) {
		list.push_back(text.substr(last, head - last));
	}
	return list;
}

//テキスト中のタグを展開したテキストを返す。
// _text: 展開したいテキスト
// textIdList: 参照されたtextIdリスト。※展開したいテキストがtextDataを基にしている場合は、textData->getId()を含めるようにする。
std::string ProjectData::getExpandedText(const char *locale, const std::string &_text, std::list<int> &textIdList)
{
	std::string text = _text;
	std::string before;
	std::string tag;
	std::string after;
	while (splitTextByTag(text, before, tag, after)) {
		auto head = tag.substr(0, 3);
		//if(head == "\\C["){
		//} else
		//} else if(head == "\\S["){
		//} else
		if (head == "\\V[") {
			auto varStr = tag.substr(3, tag.length() - 4);
			auto wordList = stringSplit(varStr, ',');
			if (wordList.size() < 2) {
				text = before + after;
			}
			else {
				int decimals = 0;
				if (wordList.size() > 2) {
					decimals = std::atoi(wordList[2].c_str());
				}
				auto ch = wordList[0][0];
				PlayVariableData *variableData = nullptr;
				int num = -1;
				int objectId = -1;
				std::string objectName;
				int variableId = -1;
				std::string variableName;
				if (isNumber(wordList[0].c_str(), &num)) {
					objectId = num;
				}
				else {
					objectName = wordList[0];
				}
				if (isNumber(wordList[1].c_str(), &num)) {
					variableId = num;
				}
				else {
					variableName = wordList[1];
				}
				if (objectId >= 0) {
					if (objectId == 0) {
						auto projectPlayData = GameManager::getInstance()->getPlayData();
						cocos2d::DictElement *el = nullptr;
						auto variableList = projectPlayData->getCommonVariableList();
						CCDICT_FOREACH(variableList, el) {
							auto playVariableData = dynamic_cast<agtk::data::PlayVariableData *>(el->getObject());
							if (!playVariableData) continue;
							if (variableId >= 0) {
								if (playVariableData->getId() == variableId) {
									variableData = playVariableData;
									break;
								}
							}
							else {
								auto name = playVariableData->getVariableData()->getName();
								if (strcmp(name, variableName.c_str()) == 0) {
									variableData = playVariableData;
									break;
								}
							}
						}
					}
					else {
						auto scene = GameManager::getInstance()->getCurrentScene();
						agtk::Object *objectInstance = scene->getObjectInstance(objectId, -1);
						if (objectInstance) {
							if (variableId >= 0) {
								variableData = objectInstance->getPlayObjectData()->getVariableData(variableId);
							} else {
								variableData = objectInstance->getPlayObjectData()->getVariableDataByName(variableName.c_str());
							}
						}
					}
				}
				else {
					auto scene = GameManager::getInstance()->getCurrentScene();
					agtk::Object *objectInstance = scene->getObjectInstanceByName(-1, objectName.c_str());
					if (objectInstance) {
						if (variableId >= 0) {
							variableData = objectInstance->getPlayObjectData()->getVariableData(variableId);
						}
						else {
							variableData = objectInstance->getPlayObjectData()->getVariableDataByName(variableName.c_str());
						}
					}
				}
				if (!variableData) {
					text = before + after;
				}
				else {
					std::string numStr;
					if (decimals == 0) {
						numStr = cocos2d::String::createWithFormat("%d", int(variableData->getValue()))->getCString();
					}
					else {
						double value = variableData->getValue();
						int digit = (int)std::pow(10, decimals);
						value = (value * digit) / digit;
						numStr = cocos2d::String::createWithFormat("%.*lf", decimals, value)->getCString();
					}
					text = before + numStr + after;
				}
			}
		}
		else if (head == "\\T[") {
			auto word = tag.substr(3, tag.length() - 4);
			TextData *textData = nullptr;
			int num = -1;
			if (isNumber(word.c_str(), &num)) {
				auto textId = num;
				textData = this->getTextData(textId);
			}
			else {
				textData = this->getTextDataByName(word.c_str());
			}
			if (!textData || std::find(textIdList.begin(), textIdList.end(), textData->getId()) != textIdList.end()) {
				text = before + after;
			}
			else {
				textIdList.push_back(textData->getId());
				text = before + getExpandedText(locale, textData->getText(locale), textIdList) + after;
			}
		}
		else if (head == "\\O[") {
			auto objectId = std::atoi(tag.substr(3).c_str());
			auto objectData = this->getObjectData(objectId);
			if (objectData) {
				text = before + objectData->getName() + after;
			}
			else {
				text = before + after;
			}
		}
	}
#if 0
	//\\を\に置き換える。
	int index = 0;
	while (true) {
		index = text.find("\\\\", index);
		if (index < 0) {
			break;
		}
		text = text.substr(0, index) + "\\" + text.substr(index + 2);
		index = index + 2;
	}
#endif
	return text;
}

// \\C[], \\S[]は展開用のタブとみなさない。
bool ProjectData::splitTextByTag(const std::string &text, std::string &before, std::string &tag, std::string &after)
{
	int head = 0;
	while (head + 4 < static_cast<int>(text.length())) {
		if (text[head] != '\\') {
			head++;
			continue;
		}
		auto ch = text[head + 1];
		if (ch == '\\') {
			head += 2;
			continue;
		}
		if (ch == 'V') {
		}
		else if (ch == 'T') {
		}
		else if (ch == 'O') {
		}
		else {
			head += 1;
			continue;
		}
		if (text[head + 2] != '[') {
			head += 2;
			continue;
		}
		if (ch == 'V') {
			int wl = 0;
			bool found = false;
			while (head + 3 + wl < static_cast<int>(text.length())) {
				if (text[head + 3 + wl] == ']') {
					if (wl > 0) {
						found = true;
					}
					break;
				}
				if (text[head + 3 + wl] == '\\') {
					break;
				}
				wl++;
			}
			if (found) {
				before = text.substr(0, head);
				tag = text.substr(head, 3 + wl + 1);
				after = text.substr(head + 3 + wl + 1);
				return true;
			}
			else {
				head += 3;
				continue;
			}
		}
		else if (ch == 'T') {
			int wl = 0;
			bool found = false;
			while (head + 3 + wl < static_cast<int>(text.length())) {
				if (text[head + 3 + wl] == ']') {
					if (wl > 0) {
						found = true;
					}
					break;
				}
				if (text[head + 3 + wl] == '\\') {
					break;
				}
				wl++;
			}
			if (found) {
				before = text.substr(0, head);
				tag = text.substr(head, 3 + wl + 1);
				after = text.substr(head + 3 + wl + 1);
				return true;
			}
			else {
				head += 3;
				continue;
			}
		}
		else if (ch == 'O') {
			int wl = 0;
			bool found = false;
			while (head + 3 + wl < static_cast<int>(text.length())) {
				if (text[head + 3 + wl] == ']') {
					if (wl > 0) {
						found = true;
					}
					break;
				}
				if (text[head + 3 + wl] == '\\') {
					break;
				}
				wl++;
			}
			if (found) {
				before = text.substr(0, head);
				tag = text.substr(head, 3 + wl + 1);
				after = text.substr(head + 3 + wl + 1);
				return true;
			}
			else {
				head += 3;
				continue;
			}
		}
		else {
			head += 2;
			continue;
		}
	}
	return false;
}


#ifdef USE_PREVIEW
void ProjectData::setObjectData(int id, const rapidjson::Value& json)
{
	//objectIdのデータを削除する再帰関数。
	std::function<void(int, cocos2d::__Dictionary *)> func = [&](int id, cocos2d::__Dictionary *children) {
		cocos2d::DictElement *el = nullptr;
		CCDICT_FOREACH(children, el) {
			auto p = dynamic_cast<agtk::data::ObjectData *>(el->getObject());
			if (p->getId() == id) {
				children->removeObjectForKey(id);
				return;
			}
			if (p->getChildren()) {
				func(id, p->getChildren());
			}
		}
		return;
	};
	auto objectList = this->getObjectList();
	func(id, objectList);
#if defined(AGTK_DEBUG)
	CC_ASSERT(objectList->objectForKey(id) == nullptr);
#endif
	auto objectData = ObjectData::create(json);
	objectList->setObject(objectData, id);
#ifdef USE_SCRIPT_PRECOMPILE
	GameManager::getInstance()->compileObjectScripts(objectData);
#endif
	//todo
}

void ProjectData::setSceneData(int id, const rapidjson::Value& json)
{
	//sceneIdのデータを削除する再帰関数。
	std::function<void(int, cocos2d::__Dictionary *)> func = [&](int id, cocos2d::__Dictionary *children) {
		cocos2d::DictElement *el = nullptr;
		CCDICT_FOREACH(children, el) {
			auto p = dynamic_cast<agtk::data::SceneData *>(el->getObject());
			if (p->getId() == id) {
				children->removeObjectForKey(id);
				return;
			}
			if (p->getChildren()) {
				func(id, p->getChildren());
			}
		}
		return;
	};
	auto sceneList = this->getSceneList();
	func(id, sceneList);
#if defined(AGTK_DEBUG)
	CC_ASSERT(sceneList->objectForKey(id) == nullptr);
#endif
	auto sceneData = SceneData::create(json, this->getTileSize());
	sceneList->setObject(sceneData, id);
	//todo
}

void ProjectData::setTilesetData(int id, const rapidjson::Value& json)
{
	//tilesetIdのデータを削除する再帰関数。
	std::function<void(int, cocos2d::__Dictionary *)> func = [&](int id, cocos2d::__Dictionary *children) {
		cocos2d::DictElement *el = nullptr;
		CCDICT_FOREACH(children, el) {
			auto p = dynamic_cast<agtk::data::TilesetData *>(el->getObject());
			if (p->getId() == id) {
				children->removeObjectForKey(id);
				return;
			}
			if (p->getChildren()) {
				func(id, p->getChildren());
			}
		}
		return;
	};
	auto tilesetList = this->getTilesetList();
	func(id, tilesetList);
#if defined(AGTK_DEBUG)
	CC_ASSERT(tilesetList->objectForKey(id) == nullptr);
#endif
	auto tilesetData = TilesetData::create(json);
	tilesetList->setObject(tilesetData, id);
	//todo
}

void ProjectData::setImageData(int id, const rapidjson::Value& json)
{
	//imageIdのデータを削除する再帰関数。
	std::function<void(int, cocos2d::__Dictionary *)> func = [&](int id, cocos2d::__Dictionary *children) {
		cocos2d::DictElement *el = nullptr;
		CCDICT_FOREACH(children, el) {
			auto p = dynamic_cast<agtk::data::ImageData *>(el->getObject());
			if (p->getId() == id) {
				children->removeObjectForKey(id);
				return;
			}
			if (p->getChildren()) {
				func(id, p->getChildren());
			}
		}
		return;
	};
	auto imageList = this->getImageList();
	func(id, imageList);
#if defined(AGTK_DEBUG)
	CC_ASSERT(imageList->objectForKey(id) == nullptr);
#endif
	auto projectPath = GameManager::getProjectPathFromProjectFile(GameManager::getInstance()->getProjectFilePath()->getCString());
	auto imageData = ImageData::create(json, projectPath);
	imageList->setObject(imageData, id);
	//todo
}

void ProjectData::setTextData(int id, const rapidjson::Value& json)
{
	//textIdのデータを削除する再帰関数。
	std::function<void(int, cocos2d::__Dictionary *)> func = [&](int id, cocos2d::__Dictionary *children) {
		cocos2d::DictElement *el = nullptr;
		CCDICT_FOREACH(children, el) {
			auto p = dynamic_cast<agtk::data::TextData *>(el->getObject());
			if (p->getId() == id) {
				children->removeObjectForKey(id);
				return;
			}
			if (p->getChildren()) {
				func(id, p->getChildren());
			}
		}
		return;
	};
	auto textList = this->getTextList();
	func(id, textList);
#if defined(AGTK_DEBUG)
	CC_ASSERT(textList->objectForKey(id) == nullptr);
#endif
	auto projectPath = GameManager::getProjectPathFromProjectFile(GameManager::getInstance()->getProjectFilePath()->getCString());
	auto textData = TextData::create(json, this->getGameInformation()->getLanguageList());
	textList->setObject(textData, id);
}
#endif

AnimParticleImageData *ProjectData::getParticleImageData(int id)
{
	CC_ASSERT(this->getAnimParticleImageList());
	return this->getParticleImageData(id, this->getAnimParticleImageList());
}

AnimParticleImageData *ProjectData::getParticleImageData(int id, cocos2d::__Dictionary *children)
{
	cocos2d::DictElement *el = nullptr;
	CCDICT_FOREACH(children, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto p = static_cast<AnimParticleImageData *>(el->getObject());
#else
		auto p = dynamic_cast<AnimParticleImageData *>(el->getObject());
#endif
		if (p->getId() == id) {
			return p;
		}
	}
	return nullptr;
}

PluginData *ProjectData::getPluginData(int id)
{
	CC_ASSERT(this->getPluginList());
	return this->getPluginData(id, this->getPluginList());
}

PluginData *ProjectData::getPluginData(int id, cocos2d::__Dictionary *children)
{
	cocos2d::DictElement *el = nullptr;
	CCDICT_FOREACH(children, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		PluginData *p = static_cast<PluginData *>(el->getObject());
#else
		PluginData *p = dynamic_cast<PluginData *>(el->getObject());
#endif
		if (p->getId() == id) {
			return p;
		}
		if (p->getChildren()) {
			p = this->getPluginData(id, p->getChildren());
			if (p) {
				return p;
			}
		}
	}
	return nullptr;
}

cocos2d::__Array *ProjectData::getPluginArray()
{
	std::function<void(cocos2d::__Dictionary *, cocos2d::__Array *)> func = [&](cocos2d::__Dictionary *children, cocos2d::__Array *arr) {
		cocos2d::DictElement *el = nullptr;
		CCDICT_FOREACH(children, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto p = static_cast<agtk::data::PluginData *>(el->getObject());
#else
			auto p = dynamic_cast<agtk::data::PluginData *>(el->getObject());
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
	func(this->getPluginList(), arr);
	return arr;
}

cocos2d::__Array *ProjectData::getVariableArray()
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

cocos2d::__Array *ProjectData::getSwitchArray()
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

/**
* 指定シーンの指定シーンレイヤに配置されるポータルオブジェクトリストの取得
* @param	portalDataList	ポータルデータリスト
* @param	sceneId			シーンID
* @param	scenelayerId	シーンレイヤーID
* @param	out				出力先
* @return					ポータルオブジェクトリスト
*/
void ProjectData::getPortalDataList(cocos2d::__Array * portalDataList, int sceneId, int sceneLayerId, cocos2d::__Array *out)
{
	CCASSERT(out, "output list is null .");
	CCASSERT(portalDataList, "portalDataList list is null .");

	cocos2d::Ref *ref = nullptr;
	CCARRAY_FOREACH(portalDataList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto p = static_cast<TransitionPortalData *>(ref);
#else
		auto p = dynamic_cast<TransitionPortalData *>(ref);
#endif

		// フォルダの場合
		if (p->getFolder()) {
			getPortalDataList(p->getChildren(), sceneId, sceneLayerId, out);
		}
		// データの場合
		else {
			cocos2d::Ref *ref2 = nullptr;
			CCARRAY_FOREACH(p->getAreaSettingList(), ref2) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto areaSettingData = static_cast<AreaSettingData *>(ref2);
#else
				auto areaSettingData = dynamic_cast<AreaSettingData *>(ref2);
#endif
				if (areaSettingData->getSceneId() == sceneId && areaSettingData->getLayerIndex() + 1 == sceneLayerId) {
					out->addObject(p);
					break;
				}
			}
		}
	}
}

cocos2d::__Array *ProjectData::getObjectArray()
{
	std::function<void(cocos2d::__Dictionary *, cocos2d::__Array *)> func = [&](cocos2d::__Dictionary *children, cocos2d::__Array *arr){
		cocos2d::DictElement *el = nullptr;
		CCDICT_FOREACH(children, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto p = static_cast<agtk::data::ObjectData *>(el->getObject());
#else
			auto p = dynamic_cast<agtk::data::ObjectData *>(el->getObject());
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
	func(this->getObjectList(), arr);
	return arr;
}

#if defined(AGTK_DEBUG)
void ProjectData::dump()
{
	CCLOG("gameIcon:%s", this->getGameIcon());
	CCLOG("gameType:%d", this->getGameType());
	CCLOG("playerCount:%d", this->getPlayerCount());
// #AGTK-NX, #AGTK-WIN
#ifdef USE_30FPS
	CCLOG("mode30Fps:%d", this->getMode30Fps());
#endif
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX) && defined(USE_NX60FPS)
#endif
	CCLOG("tileWidth:%d", this->getTileWidth());
	CCLOG("tileHeight:%d", this->getTileHeight());
	CCLOG("screenWidth:%d", this->getScreenWidth());
	CCLOG("screenHeight:%d", this->getScreenHeight());
	CCLOG("screenSettings:%d", this->getScreenSettings());
	CCLOG("magnifyWindow:%d", this->getMagnifyWindow());
	CCLOG("windowMagnification:%d", this->getWindowMagnification());
	CCLOG("adjustPixelMagnification:%d", this->getAdjustPixelMagnification());
	CCLOG("pixelMagnificationType:%d", this->getPixelMagnificationType());
	CCLOG("displayMenuBar:%d", this->getDisplayMenuBar());
	CCLOG("displayMousePointer:%d", this->getDisplayMousePointer());
	CCLOG("loadingSceneId:%d", this->getLoadingSceneId());
	CCLOG("wallDetectionOverlapMargin:%f", this->getWallDetectionOverlapMargin());
	CCLOG("multithreading:%d", this->getMultithreading());
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif
	cocos2d::Ref *ref = nullptr;
	std::string tmp = "screenEffectValueList:[";
	CCARRAY_FOREACH(this->getScreenEffectValueList(), ref) {
		auto i = dynamic_cast<cocos2d::Double *>(ref);
		tmp += std::to_string(i->getValue()) + std::string(",");
	}
	tmp += "]";
	CCLOG("screenEffectValueList:%s", tmp.c_str());
	tmp = "screenEffectFlagList:[";
	CCARRAY_FOREACH(this->getScreenEffectFlagList(), ref) {
		auto p = dynamic_cast<cocos2d::Bool *>(ref);
		tmp += p->getValue() ? "true" : "false";
	}
	tmp += "]";
	CCLOG("screenEffectFlagList:%s", tmp.c_str());
	this->getInputMapping()->dump();
	this->getGameInformation()->dump();
	this->getActionProgramSetting()->dump();
	//SceneList
	int cnt = 0;
	cocos2d::__Array *keys = this->getSceneList()->allKeys();
	CCARRAY_FOREACH(keys, ref) {
		auto id = dynamic_cast<cocos2d::Integer *>(ref);
		auto scene = dynamic_cast<SceneData *>(this->getSceneList()->objectForKey(id->getValue()));
		scene->dump();
		cnt++;
	}
	//TilesetList
	keys = this->getTilesetList()->allKeys();
	CCARRAY_FOREACH(keys, ref) {
		auto id = dynamic_cast<cocos2d::Integer *>(ref);
		auto tileset = dynamic_cast<TilesetData *>(this->getTilesetList()->objectForKey(id->getValue()));
		tileset->dump();
	}
	//FontList
	keys = this->getFontList()->allKeys();
	CCARRAY_FOREACH(keys, ref) {
		auto id = dynamic_cast<cocos2d::Integer *>(ref);
		auto data = dynamic_cast<FontData *>(this->getFontList()->objectForKey(id->getValue()));
		data->dump();
	}
	//ImageList
	keys = this->getImageList()->allKeys();
	CCARRAY_FOREACH(keys, ref) {
		auto id = dynamic_cast<cocos2d::Integer *>(ref);
		auto data = dynamic_cast<ImageData *>(this->getImageList()->objectForKey(id->getValue()));
		data->dump();
	}
	//bgmList
	CCLOG("bgmList ---------");
	keys = this->getBgmList()->allKeys();
	CCARRAY_FOREACH(keys, ref) {
		auto id = dynamic_cast<cocos2d::Integer *>(ref);
		auto data = dynamic_cast<BgmData *>(this->getBgmList()->objectForKey(id->getValue()));
		data->dump();
	}
	//seList
	CCLOG("seList ----------");
	keys = this->getSeList()->allKeys();
	CCARRAY_FOREACH(keys, ref) {
		auto id = dynamic_cast<cocos2d::Integer *>(ref);
		auto data = dynamic_cast<SeData *>(this->getSeList()->objectForKey(id->getValue()));
		data->dump();
	}
	//voiceList
	CCLOG("voiceList -------");
	keys = this->getVoiceList()->allKeys();
	CCARRAY_FOREACH(keys, ref) {
		auto id = dynamic_cast<cocos2d::Integer *>(ref);
		auto data = dynamic_cast<VoiceData *>(this->getVoiceList()->objectForKey(id->getValue()));
		data->dump();
	}
	//variableList
	CCLOG("variableList -------");
	keys = this->getVariableList()->allKeys();
	CCARRAY_FOREACH(keys, ref) {
		auto id = dynamic_cast<cocos2d::Integer *>(ref);
		auto data = dynamic_cast<VariableData *>(this->getVariableList()->objectForKey(id->getValue()));
		data->dump();
	}
	//switchList
	CCLOG("switchList -------");
	keys = this->getSwitchList()->allKeys();
	CCARRAY_FOREACH(keys, ref) {
		auto id = dynamic_cast<cocos2d::Integer *>(ref);
		auto data = dynamic_cast<SwitchData *>(this->getSwitchList()->objectForKey(id->getValue()));
		data->dump();
	}
	//animationOnlyList
	CCLOG("animationOnlyList -------");
	keys = this->getAnimationOnlyList()->allKeys();
	CCARRAY_FOREACH(keys, ref) {
		auto id = dynamic_cast<cocos2d::Integer *>(ref);
		auto data = dynamic_cast<AnimationOnlyData *>(this->getAnimationOnlyList()->objectForKey(id->getValue()));
		data->dump();
	}
	//objectList
	CCLOG("objectList -------");
	keys = this->getObjectList()->allKeys();
	CCARRAY_FOREACH(keys, ref){
		auto id = dynamic_cast<cocos2d::Integer *>(ref);
		auto data = dynamic_cast<ObjectData *>(this->getObjectList()->objectForKey(id->getValue()));
		data->dump();
	}
	//animationList
	CCLOG("animationList -------");
	keys = this->getAnimationList()->allKeys();
	CCARRAY_FOREACH(keys, ref) {
		auto id = dynamic_cast<cocos2d::Integer *>(ref);
		auto data = dynamic_cast<AnimationData *>(this->getAnimationList()->objectForKey(id->getValue()));
		data->dump();
	}
	CCLOG("objectActionProgramSettings -------");
	this->getObjectActionProgramSettings()->dump();

	CCLOG("AnimParticleImageDataList -------");
	keys = this->getAnimParticleImageList()->allKeys();
	CCARRAY_FOREACH(keys, ref) {
		auto id = dynamic_cast<cocos2d::Integer *>(ref);
		auto data = dynamic_cast<AnimParticleImageData *>(this->getAnimParticleImageList()->objectForKey(id->getValue()));
		data->dump();
	}

	CCLOG("transitionFlow -------");
	this->getTransitionFlow()->dump();

	CCLOG("transitionPortalList -------");
	ref = nullptr;
	CCARRAY_FOREACH(this->getTransitionPortalList(), ref) {
		auto data = dynamic_cast<TransitionPortalData *>(ref);
		data->dump();
	}
	CCLOG("playerCharacter -------");
	this->getPlayerCharacterData()->dump();

	CCLOG("pluginList ---------");
	keys = this->getPluginList()->allKeys();
	CCARRAY_FOREACH(keys, ref) {
		auto id = dynamic_cast<cocos2d::Integer *>(ref);
		auto data = dynamic_cast<PluginData *>(this->getPluginList()->objectForKey(id->getValue()));
		data->dump();
	}
}
#endif

NS_DATA_END
NS_AGTK_END
