/**
 * @brief オブジェクトコマンドデータ
 */
#include "ObjectCommandData.h"
#include "ObjectData.h"
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
FilterEffect::FilterEffect()
: _transparency(100)
, _blinkInterval300(0)
{
}

FilterEffect::~FilterEffect()
{
}

bool FilterEffect::init(const rapidjson::Value& json)
{
	CC_ASSERT(json.HasMember("effectType"));
	this->setEffectType((EnumEffectType)json["effectType"].GetInt());
	CC_ASSERT(json.HasMember("noise"));
	this->setNoise(json["noise"].GetInt());
	CC_ASSERT(json.HasMember("mosaic"));
	this->setMosaic(json["mosaic"].GetInt());
	CC_ASSERT(json.HasMember("monochrome"));
	this->setMonochrome(json["monochrome"].GetInt());
	CC_ASSERT(json.HasMember("sepia"));
	this->setSepia(json["sepia"].GetInt());
	CC_ASSERT(json.HasMember("negaPosiReverse"));
	this->setNegaPosiReverse(json["negaPosiReverse"].GetInt());
	CC_ASSERT(json.HasMember("defocus"));
	this->setDefocus(json["defocus"].GetInt());
	if (json.HasMember("chromaticAberration")) {
		this->setChromaticAberration(json["chromaticAberration"].GetInt());
	}
	else {
		this->setChromaticAberration(0);
	}
	CC_ASSERT(json.HasMember("darkness"));
	this->setDarkness(json["darkness"].GetInt());
	if (json.HasMember("transparency")) {
		this->setTransparency(json["transparency"].GetInt());
	}
	if (json.HasMember("blinkInterval300")) {
		this->setBlinkInterval300(json["blinkInterval300"].GetInt());
	}
	CC_ASSERT(json.HasMember("imageId"));
	this->setImageId(json["imageId"].GetInt());
	CC_ASSERT(json.HasMember("imageTransparency"));
	this->setImageTransparency(json["imageTransparency"].GetInt());
	CC_ASSERT(json.HasMember("fillA"));
	this->setFillA(json["fillA"].GetInt());
	CC_ASSERT(json.HasMember("fillR"));
	this->setFillR(json["fillR"].GetInt());
	CC_ASSERT(json.HasMember("fillG"));
	this->setFillG(json["fillG"].GetInt());
	CC_ASSERT(json.HasMember("fillB"));
	this->setFillB(json["fillB"].GetInt());
	CC_ASSERT(json.HasMember("duration300"));
	this->setDuration300(json["duration300"].GetInt());
	CC_ASSERT(json.HasMember("imagePlacement"));
	this->setImagePlacement((EnumPlacementType)json["imagePlacement"].GetInt());
	return true;
}

bool FilterEffect::init(void *jsCx, void *jsObj)
{
	//デフォルト値設定。
	this->setEffectType(kEffectNoise);
	this->setNoise(100);
	this->setMosaic(100);
	this->setMonochrome(100);
	this->setSepia(100);
	this->setNegaPosiReverse(100);
	this->setDefocus(100);
	this->setChromaticAberration(100);
	this->setDarkness(100);
	this->setTransparency(100);
	this->setBlinkInterval300(0);
	this->setImageId(-1);
	this->setImageTransparency(0);
	this->setImagePlacement(kPlacementCenter);
	this->setFillA(255);
	this->setFillR(255);
	this->setFillG(255);
	this->setFillB(255);
	this->setDuration300(300);

	if (jsObj) {
		auto cx = (JSContext *)jsCx;
		JS::RootedValue v(cx);
		JS::RootedObject rparams(cx, (JSObject *)jsObj);
		int iValue;
		if (getJsInt32(cx, rparams, "effectType", &iValue)) {
			this->setEffectType((EnumEffectType)iValue);
		}
		if (getJsInt32(cx, rparams, "noise", &iValue)) {
			this->setNoise(iValue);
		}
		if (getJsInt32(cx, rparams, "mosaic", &iValue)) {
			this->setMosaic(iValue);
		}
		if (getJsInt32(cx, rparams, "monochrome", &iValue)) {
			this->setMonochrome(iValue);
		}
		if (getJsInt32(cx, rparams, "sepia", &iValue)) {
			this->setSepia(iValue);
		}
		if (getJsInt32(cx, rparams, "negaPosiReverse", &iValue)) {
			this->setNegaPosiReverse(iValue);
		}
		if (getJsInt32(cx, rparams, "defocus", &iValue)) {
			this->setDefocus(iValue);
		}
		if (getJsInt32(cx, rparams, "chromaticAberration", &iValue)) {
			this->setChromaticAberration(iValue);
		}
		if (getJsInt32(cx, rparams, "darkness", &iValue)) {
			this->setDarkness(iValue);
		}
		if (getJsInt32(cx, rparams, "transparency", &iValue)) {
			this->setTransparency(iValue);
		}
		if (getJsInt32(cx, rparams, "blinkInterval300", &iValue)) {
			this->setBlinkInterval300(iValue);
		}
		if (getJsInt32(cx, rparams, "imageId", &iValue)) {
			this->setImageId(iValue);
		}
		if (getJsInt32(cx, rparams, "imageTransparency", &iValue)) {
			this->setImageTransparency(iValue);
		}
		if (getJsInt32(cx, rparams, "imagePlacement", &iValue)) {
			this->setImagePlacement((EnumPlacementType)iValue);
		}
		if (getJsInt32(cx, rparams, "fillA", &iValue)) {
			this->setFillA(iValue);
		}
		if (getJsInt32(cx, rparams, "fillR", &iValue)) {
			this->setFillR(iValue);
		}
		if (getJsInt32(cx, rparams, "fillG", &iValue)) {
			this->setFillG(iValue);
		}
		if (getJsInt32(cx, rparams, "fillB", &iValue)) {
			this->setFillB(iValue);
		}
		if (getJsInt32(cx, rparams, "duration300", &iValue)) {
			this->setDuration300(iValue);
		}
	}

	return true;
}

#if defined(AGTK_DEBUG)
void FilterEffect::dump()
{
	CCLOG("effectType:%d", this->getEffectType());
	CCLOG("noise:%d", this->getNoise());
	CCLOG("mosaic:%d", this->getMosaic());
	CCLOG("monochrome:%d", this->getMonochrome());
	CCLOG("sepia:%d", this->getSepia());
	CCLOG("negaPosiReverse:%d", this->getNegaPosiReverse());
	CCLOG("defocus:%d", this->getDefocus());
	CCLOG("chromaticAberration:%d", this->getChromaticAberration());
	CCLOG("darkness:%d", this->getDarkness());
	CCLOG("transparency:%d", this->getTransparency());
	CCLOG("blinkInterval300:%d", this->getBlinkInterval300());
	CCLOG("imageId:%d", this->getImageId());
	CCLOG("imageTransparency:%d", this->getImageTransparency());
	CCLOG("fillA:%d", this->getFillA());
	CCLOG("fillR:%d", this->getFillR());
	CCLOG("fillG:%d", this->getFillG());
	CCLOG("fillB:%d", this->getFillB());
	CCLOG("duration300:%d", this->getDuration300());
}
#endif

//-------------------------------------------------------------------------------------------------------------------
bool ObjectCommandSwitchVariableResetData::SwitchVariableData::init(void *jsCx, void *jsObj)
{
	//デフォルト値設定。
	this->setSwtch(true);
	this->setObjectId(-1);
	this->setItemId(-1);

	auto cx = (JSContext *)jsCx;
	JS::RootedValue v(cx);
	JS::RootedObject rparams(cx, (JSObject *)jsObj);
	bool bValue;
	int iValue;
	if (getJsBoolean(cx, rparams, "swtch", &bValue)) {
		this->setSwtch(bValue);
	}
	if (getJsInt32(cx, rparams, "objectId", &iValue)) {
		this->setObjectId(iValue);
	}
	if (getJsInt32(cx, rparams, "itemId", &iValue)) {
		this->setItemId(iValue);
	}

	return true;
}

//-------------------------------------------------------------------------------------------------------------------
ObjectCommandData::ObjectCommandData()
{
	_id = 0;
	_commandType = kMax;
	_ignored = false;
}

ObjectCommandData *ObjectCommandData::create(const rapidjson::Value& json)
{
	EnumObjCommandType type = (EnumObjCommandType)json["commandType"].GetInt();
	if (type >= kCustomHead) {
		return ObjectCommandCustomData::create(json);
	} else
	switch (type) {
	case kTemplateMove: return ObjectCommandTemplateMoveData::create(json);
	case kObjectLock: return ObjectCommandObjectLockData::create(json);
	case kObjectCreate: return ObjectCommandObjectCreateData::create(json);
	case kObjectChange: return ObjectCommandObjectChangeData::create(json);
	case kObjectMove: return ObjectCommandObjectMoveData::create(json);
	case kObjectPushPull: return ObjectCommandObjectPushPullData::create(json);
	case kParentChildChange: return ObjectCommandParentChildChangeData::create(json);
	case kLayerMove: return ObjectCommandLayerMoveData::create(json);
	case kAttackSetting: return ObjectCommandAttackSettingData::create(json);
	case kBulletFire: return ObjectCommandBulletFireData::create(json);
	case kDisappear: return ObjectCommandDisappearData::create(json);
	case kDisappearObjectRecover: return ObjectCommandDisappearObjectRecoverData::create(json);
	case kDisable: return ObjectCommandDisableData::create(json);
	case kDisableObjectEnable: return ObjectCommandDisableObjectEnableData::create(json);
	case kObjectFilterEffect: return ObjectCommandObjectFilterEffectData::create(json);
	case kObjectFilterEffectRemove: return ObjectCommandObjectFilterEffectRemoveData::create(json);
	case kSceneEffect: return ObjectCommandSceneEffectData::create(json);
	case kSceneEffectRemove: return ObjectCommandSceneEffectRemoveData::create(json);
	case kSceneGravityChange: return ObjectCommandSceneGravityChangeData::create(json);
	case kSceneRotateFlip: return ObjectCommandSceneRotateFlipData::create(json);
	case kCameraAreaChange: return ObjectCommandCameraAreaChangeData::create(json);
	case kSoundPlay: return ObjectCommandSoundPlayData::create(json);
	case kMessageShow: return ObjectCommandMessageShowData::create(json);
	case kScrollMessageShow: return ObjectCommandScrollMessageShowData::create(json);
	case kEffectShow: return ObjectCommandEffectShowData::create(json);
	case kMovieShow: return ObjectCommandMovieShowData::create(json);
	case kImageShow: return ObjectCommandImageShowData::create(json);
	case kSwitchVariableChange: return ObjectCommandSwitchVariableChangeData::create(json);
	case kSwitchVariableReset: return ObjectCommandSwitchVariableResetData::create(json);
	case kGameSpeedChange: return ObjectCommandGameSpeedChangeData::create(json);
	case kWait: return ObjectCommandWaitData::create(json);
	case kSceneTerminate: return ObjectCommandSceneTerminateData::create(json);
	case kDirectionMove: return ObjectCommandDirectionMoveData::create(json);
	case kForthBackMoveTurn: return ObjectCommandForthBackMoveTurnData::create(json);
	case kActionExec: return ObjectCommandActionExecData::create(json);
	case kParticleShow: return ObjectCommandParticleShowData::create(json);
	case kTimer: return ObjectCommandTimerData::create(json);
	case kSceneShake: return ObjectCommandSceneShakeData::create(json);
	case kEffectRemove: return ObjectCommandEffectRemoveData::create(json);
	case kParticleRemove: return ObjectCommandParticleRemoveData::create(json);
	case kLayerHide: return ObjectCommandLayerHide::create(json);
	case kLayerShow: return ObjectCommandLayerShow::create(json);
	case kLayerDisable: return ObjectCommandLayerDisable::create(json);
	case kLayerEnable: return ObjectCommandLayerEnable::create(json);
	case kScriptEvaluate: return ObjectCommandScriptEvaluate::create(json);
	case kSoundStop: return ObjectCommandSoundStopData::create(json);
	case kMenuShow: return ObjectCommandMenuShowData::create(json);
	case kMenuHide: return ObjectCommandMenuHideData::create(json);
	case kDisplayDirectionMove: return ObjectCommandDisplayDirectionMoveData::create(json);
	case kFileLoad: return ObjectCommandFileLoadData::create(json);
	case kSoundPositionRemember: return ObjectCommandSoundPositionRememberData::create(json);
	case kObjectUnlock: return ObjectCommandObjectUnlockData::create(json);
	case kResourceSetChange: return ObjectCommandResourceSetChangeData::create(json);
	case kDatabaseReflect: return ObjectCommandDatabaseReflectData::create(json);
	case kNXVibrateController: return ObjectCommandkNXVibrateControlData::create(json);	// #AGTK-NX
	case kNXShowControllerApplet: return ObjectCommandNXShowControllerAppletData::create(json); // #AGTK-NX
		CC_ASSERT(0);
	}
	CC_ASSERT(0);
	return nullptr;
}

bool ObjectCommandData::init(const rapidjson::Value& json)
{
	CC_ASSERT(json.HasMember("id"));
	this->setId(json["id"].GetInt());
	CC_ASSERT(json.HasMember("commandType"));
	this->setCommandType((EnumObjCommandType)json["commandType"].GetInt());
	CC_ASSERT(json.HasMember("ignored"));
	this->setIgnored(json["ignored"].GetBool());
	CC_ASSERT(json.HasMember("instanceConfigurable"));
	this->setInstanceConfigurable(json["instanceConfigurable"].GetBool());
	return true;
}

#if defined(AGTK_DEBUG)
void ObjectCommandData::dump()
{
	CCLOG("id:%d", this->getId());
	CCLOG("commandType:%d", this->getCommandType());
	CCLOG("ignroed:%d", this->getIgnored());
	CCLOG("instanceConfigurable:%d", this->getInstanceConfigurable());
}
#endif

const char *ObjectCommandData::getCommandTypeName(EnumObjCommandType type)
{
	static const char *commandTypeName[] = {
		"Template Move Settings",//kTemplateMove
		"Lock Object",//kObjectLock
		"Generate Object",//kObjectCreate
		"Change Object",//kObjectChange
		"Move Object",//kObjectMove
		"Push/Pull Object",//kObjectPushPull
		"",//kParentChildChange
		"Move Layer",//kLayerMove
		"Attack Settings",//kAttackSetting
		"Fire Bullet",//kBulletFire
		"Destroy Object",//kDisappear
		"Restore Destroyed Object",//kDisappearObjectRecover
		"Disable Object",//kDisable
		"Enable Disabled Object",//kDisableObjectEnable
		"",//kItemGive
		"",//kItemShow
		"",//kMenuPartShow
		"Apply Filter Effects on Object",//kObjectFilterEffect
		"Delete Filter Effects from Object",//kObjectFilterEffectRemove
		"Apply Screen Effects on Scene",//kSceneEffect
		"Delete Screen Effects from Scene",//kSceneEffectRemove
		"Change Scene Gravity Effects",//kSceneGravityChange
		"",//kSceneWaterChange
		"Rotate/Flip Scene",//kSceneRotateFlip
		"Change Camera Display Area",//kCameraAreaChange
		"Audio Playback",//kSoundPlay
		"Show Text",//kMessageShow
		"Show Scrolling Text",//kScrollMessageShow
		"Show Effect",//kEffectShow
		"Play Video",//kMovieShow
		"Display Image",//kImageShow
		"Change Switch/Variable",//kSwitchVariableChange
		"Reset Switch/Variable",//kSwitchVariableReset
		"Change Game Speed",//kGameSpeedChange
		"Wait",//kWait
		"End Scene",//kSceneTerminate,
		"",//kSaveScreenOpen
		"",//kQuickSave
		"Set Move Direction and Move",//kDirectionMove
		"Back and Forth Moving and Turning",//kFortBackMoveTurn
		"Execute Object Action",//kActionExec
		"Show Particles",//kParticleShow
		"Timer Function",//kTimer
		"Shake Scene",//kSceneShake
		"Hide Effects",//kEffectRemove
		"Hide Particles",//kParticleRemove
		"Disable Layer Display",//kLayerHide
		"Enable Layer Display",//kLayerShow
		"Disable Layer Motion",//kLayerDisable
		"Enable Layer Motion",//kLayerEnable
		"Execute Script",//kScriptEvaluate
		"Stop Audio Item",//kSoundStop
		"Show Menu Screen",//kMenuShow
		"Hide Menu Screen",//kMenuHide
		"Move Towards Display Direction",//kDisplayDirectionMove
		"Load a File",//kFileLoad
		"Save Playhead Start Time",//kSoundPositionRemember
		"Release Lock",//kObjectUnlock
		"Change Animation Set",//kResourceSetChange
		"Database Reflect",//kDatabaseReflect
		"Vibrate Controller",	// #AGTK-NX kNXVibrateController
		"Show Controller Applet",	// #AGTK-NX kNXShowControllerApplet
	};
// #AGTK-NX	
#if CC_TARGET_PLATFORM == CC_PLATFORM_NX
#else
	static_assert(ARRAYSIZE(commandTypeName) == EnumObjCommandType::kMax,"array size not match");
#endif

	if (type >= kCustomHead) {
		return "カスタム";
	}
	return commandTypeName[type];
}

//-------------------------------------------------------------------------------------------------------------------
ObjectCommandTemplateMoveData::ObjectCommandTemplateMoveData()
: ObjectCommandData()
, _nearObjectGroup(agtk::data::ObjectData::kObjGroupPlayer)
, _apartNearObjectGroup(agtk::data::ObjectData::kObjGroupPlayer)
{
}

bool ObjectCommandTemplateMoveData::init(const rapidjson::Value& json)
{
	if (!ObjectCommandData::init(json)) {
		return false;
	}
	CC_ASSERT(json.HasMember("templateMove"));
	const rapidjson::Value& cmd = json["templateMove"];
	this->setMoveType((EnumMoveType)cmd["moveType"].GetInt());
	this->setHorizontalMoveStartRight(cmd["horizontalMoveStartRight"].GetBool());
	this->setHorizontalMoveDuration300(cmd["horizontalMoveDuration300"].GetInt());
	this->setHorizontalInfinite(cmd["horizontalInfinite"].GetBool());
	this->setVerticalMoveStartDown(cmd["verticalMoveStartDown"].GetBool());
	this->setVerticalMoveDuration300(cmd["verticalMoveDuration300"].GetInt());
	this->setVerticalInfinite(cmd["verticalInfinite"].GetBool());
	this->setRandomMoveDuration300(cmd["randomMoveDuration300"].GetInt());
	this->setRandomMoveStop300(cmd["randomMoveStop300"].GetInt());
	if(cmd.HasMember("nearObjectGroup")){
		this->setNearObjectGroup(cmd["nearObjectGroup"].GetInt());
	}
	if (cmd.HasMember("nearObjectLockedObjectPrior")) {
		this->setNearObjectLockedObjectPrior(cmd["nearObjectLockedObjectPrior"].GetBool());
	}
	else {
		this->setNearObjectLockedObjectPrior(cmd["nearPlayerLockedPlayerPrior"].GetBool());
	}
	if (cmd.HasMember("apartNearObjectGroup")) {
		this->setApartNearObjectGroup(cmd["apartNearObjectGroup"].GetInt());
	}
	if (cmd.HasMember("apartNearObjectLockedObjectPrior")) {
		this->setApartNearObjectLockedObjectPrior(cmd["apartNearObjectLockedObjectPrior"].GetBool());
	}
	else {
		this->setApartNearObjectLockedObjectPrior(cmd["apartNearPlayerLockedPlayerPrior"].GetBool());
	}
	this->setFallFromStep(cmd["fallFromStep"].GetBool());
	this->setIgnoreOtherObjectWallArea(cmd["ignoreOtherObjectWallArea"].GetBool());
	this->setIgnoreWall(cmd["ignoreWall"].GetBool());
	return true;
}

bool ObjectCommandTemplateMoveData::init(void *jsCx, void *jsObj)
{
	_commandType = kTemplateMove;
	//デフォルト値設定。
	this->setMoveType(kMoveBound);
	this->setHorizontalMoveStartRight(false);
	this->setHorizontalMoveDuration300(300);
	this->setHorizontalInfinite(false);
	this->setVerticalMoveStartDown(false);
	this->setVerticalMoveDuration300(300);
	this->setVerticalInfinite(false);
	this->setRandomMoveDuration300(300);
	this->setRandomMoveStop300(300);
	this->setNearObjectGroup(agtk::data::ObjectData::kObjGroupPlayer);
	this->setNearObjectLockedObjectPrior(false);
	this->setApartNearObjectGroup(agtk::data::ObjectData::kObjGroupPlayer);
	this->setApartNearObjectLockedObjectPrior(false);
	this->setFallFromStep(true);
	this->setIgnoreOtherObjectWallArea(false);
	this->setIgnoreWall(false);

	auto cx = (JSContext *)jsCx;
	JS::RootedValue v(cx);
	JS::RootedObject rparams(cx, (JSObject *)jsObj);
	bool bValue;
	int iValue;
	if (getJsInt32(cx, rparams, "moveType", &iValue)) {
		this->setMoveType((EnumMoveType)iValue);
	}
	if (getJsBoolean(cx, rparams, "horizontalMoveStartRight", &bValue)) {
		this->setHorizontalMoveStartRight(bValue);
	}
	if (getJsInt32(cx, rparams, "horizontalMoveDuration300", &iValue)) {
		this->setHorizontalMoveDuration300(iValue);
	}
	if (getJsBoolean(cx, rparams, "horizontalInfinite", &bValue)) {
		this->setHorizontalInfinite(bValue);
	}
	if (getJsBoolean(cx, rparams, "verticalMoveStartDown", &bValue)) {
		this->setVerticalMoveStartDown(bValue);
	}
	if (getJsInt32(cx, rparams, "verticalMoveDuration300", &iValue)) {
		this->setVerticalMoveDuration300(iValue);
	}
	if (getJsBoolean(cx, rparams, "verticalInfinite", &bValue)) {
		this->setVerticalInfinite(bValue);
	}
	if (getJsInt32(cx, rparams, "randomMoveDuration300", &iValue)) {
		this->setRandomMoveDuration300(iValue);
	}
	if (getJsInt32(cx, rparams, "randomMoveStop300", &iValue)) {
		this->setRandomMoveStop300(iValue);
	}
	if (getJsInt32(cx, rparams, "nearObjectGroup", &iValue)) {
		this->setNearObjectGroup(iValue);
	}
	if (getJsBoolean(cx, rparams, "nearObjectLockedObjectPrior", &bValue)) {
		this->setNearObjectLockedObjectPrior(bValue);
	}
	// backward compatibility
	if (getJsBoolean(cx, rparams, "nearPlayerLockedPlayerPrior", &bValue)) {
		this->setNearObjectLockedObjectPrior(bValue);
	}
	if (getJsInt32(cx, rparams, "apartNearObjectGroup", &iValue)) {
		this->setApartNearObjectGroup(iValue);
	}
	if (getJsBoolean(cx, rparams, "apartNearObjectLockedObjectPrior", &bValue)) {
		this->setApartNearObjectLockedObjectPrior(bValue);
	}
	// backward compatibility
	if (getJsBoolean(cx, rparams, "apartNearPlayerLockedPlayerPrior", &bValue)) {
		this->setApartNearObjectLockedObjectPrior(bValue);
	}
	if (getJsBoolean(cx, rparams, "fallFromStep", &bValue)) {
		this->setFallFromStep(bValue);
	}
	if (getJsBoolean(cx, rparams, "ignoreOtherObjectWallArea", &bValue)) {
		this->setIgnoreOtherObjectWallArea(bValue);
	}
	if (getJsBoolean(cx, rparams, "ignoreWall", &bValue)) {
		this->setIgnoreWall(bValue);
	}
	return true;
}

#if defined(AGTK_DEBUG)
void ObjectCommandTemplateMoveData::dump()
{
	ObjectCommandData::dump();
	CCLOG("moveType:%d", this->getMoveType());
	CCLOG("horizontalMoveStartRight:%d", this->getHorizontalMoveStartRight());
	CCLOG("horizontalMoveDuration300:%d", this->getHorizontalMoveDuration300());
	CCLOG("horizontalInfinite:%d", this->getHorizontalInfinite());
	CCLOG("verticalMoveStartDown:%d", this->getVerticalMoveStartDown());
	CCLOG("verticalMoveDuration300:%d", this->getVerticalMoveDuration300());
	CCLOG("verticalInfinite:%d", this->getVerticalInfinite());
	CCLOG("randomMoveDuration300:%d", this->getRandomMoveDuration300());
	CCLOG("randomMoveStop300:%d", this->getRandomMoveStop300());
	CCLOG("nearObjectGroup:%d", this->getNearObjectGroup());
	CCLOG("nearObjectLockedObjectPrior:%d", this->getNearObjectLockedObjectPrior());
	CCLOG("apartNearObjectGroup:%d", this->getApartNearObjectGroup());
	CCLOG("apartNearObjectLockedObjectPrior:%d", this->getApartNearObjectLockedObjectPrior());
	CCLOG("fallFromStep:%d", this->getFallFromStep());
	CCLOG("ignoreOtherObjectWallArea:%d", this->getIgnoreOtherObjectWallArea());
	CCLOG("ignoreWall:%d", this->getIgnoreWall());
}
#endif

//-------------------------------------------------------------------------------------------------------------------
bool ObjectCommandObjectLockData::init(const rapidjson::Value& json)
{
	if (!ObjectCommandData::init(json)) {
		return false;
	}
	CC_ASSERT(json.HasMember("objectLock"));
	const rapidjson::Value& cmd = json["objectLock"];
	this->setLockTouchedObject(cmd["lockTouchedObject"].GetBool());
	this->setLockViewportLightObject(cmd["lockViewportLightObject"].GetBool());
	if (cmd.HasMember("lockViewportLightOfAcrossLayerObject")) {
		this->setLockViewportLightOfAcrossLayerObject(cmd["lockViewportLightOfAcrossLayerObject"].GetBool());
	}
	else {
		this->setLockViewportLightOfAcrossLayerObject(false);
	}
	
	this->setLockObjectOnScreen(cmd["lockObjectOnScreen"].GetBool());
	if (cmd.HasMember("lockObjectTouchedByThisObjectAttack")) {
		this->setLockObjectTouchedByThisObjectAttack(cmd["lockObjectTouchedByThisObjectAttack"].GetBool());
	}
	else {
		this->setLockObjectTouchedByThisObjectAttack(false);
	}
	this->setObjectType(cmd["objectType"].GetInt());
	this->setObjectGroup(cmd["objectGroup"].GetInt());
	this->setObjectId(cmd["objectId"].GetInt());
	this->setUseType((EnumUseSwitchVariable)cmd["useType"].GetInt());
	this->setSwitchId(cmd["switchId"].GetInt());
	this->setSwitchCondition((EnumSwitchCondition)cmd["switchCondition"].GetInt());
	this->setVariableId(cmd["variableId"].GetInt());
	this->setCompareVariableOperator(cmd["compareVariableOperator"].GetInt());
	this->setCompareValueType((EnumVariableCompareValueType)cmd["compareValueType"].GetInt());
	this->setCompareValue(cmd["compareValue"].GetDouble());
	this->setCompareVariableObjectId(cmd["compareVariableObjectId"].GetInt());
	this->setCompareVariableId(cmd["compareVariableId"].GetInt());
	this->setCompareVariableQualifierId((EnumQualifierType)cmd["compareVariableQualifierId"].GetInt());
	return true;
}

bool ObjectCommandObjectLockData::init(void *jsCx, void *jsObj)
{
	_commandType = kObjectLock;
	//デフォルト値設定。
	this->setLockTouchedObject(false);
	this->setLockViewportLightObject(false);
	this->setLockViewportLightOfAcrossLayerObject(false);
	this->setLockObjectOnScreen(false);
	this->setLockObjectTouchedByThisObjectAttack(false);
	this->setObjectType(kObjectByGroup);
	this->setObjectGroup(kObjectGroupAll);
	this->setObjectId(-1);
	this->setUseType(kUseNone);
	this->setSwitchId(-1);
	this->setSwitchCondition(kSwitchConditionOn);
	this->setVariableId(-1);
	this->setCompareVariableOperator(kOperatorEqual);
	this->setCompareValueType(kVariableCompareValue);
	this->setCompareValue(0);
	this->setCompareVariableObjectId(-1);
	this->setCompareVariableId(-1);
	this->setCompareVariableQualifierId(kQualifierSingle);

	auto cx = (JSContext *)jsCx;
	JS::RootedValue v(cx);
	JS::RootedObject rparams(cx, (JSObject *)jsObj);
	bool bValue;
	int iValue;
	double dValue;
	if (getJsBoolean(cx, rparams, "lockTouchedObject", &bValue)) {
		this->setLockTouchedObject(bValue);
	}
	if (getJsBoolean(cx, rparams, "lockViewportLightObject", &bValue)) {
		this->setLockViewportLightObject(bValue);
	}
	if (getJsBoolean(cx, rparams, "lockViewportLightOfAcrossLayerObject", &bValue)) {
		this->setLockViewportLightOfAcrossLayerObject(bValue);
	}
	if (getJsBoolean(cx, rparams, "lockObjectOnScreen", &bValue)) {
		this->setLockObjectOnScreen(bValue);
	}
	if (getJsBoolean(cx, rparams, "lockObjectTouchedByThisObjectAttack", &bValue)) {
		this->setLockObjectTouchedByThisObjectAttack(bValue);
	}
	if (isJsDefined(cx, rparams, "objectGroup")) {
		if (getJsInt32(cx, rparams, "objectGroup", &iValue)) {
			this->setObjectGroup(iValue);
		}
	}
	else {
		if (getJsInt32(cx, rparams, "objectTypeByType", &iValue)) {
			this->setObjectGroup(iValue - kObjectGroupAllOffset);
		}
	}
	if (getJsInt32(cx, rparams, "objectType", &iValue)) {
		this->setObjectType(iValue);
	}
	if (getJsInt32(cx, rparams, "objectId", &iValue)) {
		this->setObjectId(iValue);
	}
	if (getJsInt32(cx, rparams, "useType", &iValue)) {
		this->setUseType((EnumUseSwitchVariable)iValue);
	}
	if (getJsInt32(cx, rparams, "switchId", &iValue)) {
		this->setSwitchId(iValue);
	}
	if (getJsInt32(cx, rparams, "switchCondition", &iValue)) {
		this->setSwitchCondition((EnumSwitchCondition)iValue);
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
void ObjectCommandObjectLockData::dump()
{
	ObjectCommandData::dump();
	CCLOG("lockTouchedObject:%d", this->getLockTouchedObject());
	CCLOG("lockViewportLightObject:%d", this->getLockViewportLightObject());
	CCLOG("lockViewportLightOfAcrossLayerObject:%d", this->getLockViewportLightOfAcrossLayerObject());
	CCLOG("lockObjectOnScreen:%d", this->getLockObjectOnScreen());
	CCLOG("lockObjectTouchedByThisObjectAttack:%d", this->getLockObjectTouchedByThisObjectAttack());
	CCLOG("objectGroup:%d", this->getObjectGroup());
	CCLOG("objectType:%d", this->getObjectType());
	CCLOG("objectId:%d", this->getObjectId());
	CCLOG("useType:%d", this->getUseType());
	CCLOG("switchId:%d", this->getSwitchId());
	CCLOG("switchCondition:%d", this->getSwitchCondition());
	CCLOG("variableId:%d", this->getVariableId());
	CCLOG("compareVariableOperator:%d", this->getCompareVariableOperator());
	CCLOG("compareValueType:%d", this->getCompareValueType());
	CCLOG("compareValue:%d", this->getCompareValue());
	CCLOG("compareVariableObjectId:%d", this->getCompareVariableObjectId());
	CCLOG("compareVariableId:%d", this->getCompareVariableId());
	CCLOG("compareVariableQualifierId:%d", this->getCompareVariableQualifierId());
}
#endif

ObjectCommandObjectCreateData *ObjectCommandObjectCreateData::createForScript()
{
	return new ObjectCommandObjectCreateData();
}

//-------------------------------------------------------------------------------------------------------------------
bool ObjectCommandObjectCreateData::init(const rapidjson::Value& json)
{
	if (!ObjectCommandData::init(json)) {
		return false;
	}
	CC_ASSERT(json.HasMember("objectCreate"));
	const rapidjson::Value& cmd = json["objectCreate"];
	CC_ASSERT(cmd.HasMember("objectId"));
	this->setObjectId(cmd["objectId"].GetInt());
	if (cmd.HasMember("actionId")) {
		this->setActionId(cmd["actionId"].GetInt());
	}
	CC_ASSERT(cmd.HasMember("createPosition"));
	this->setCreatePosition((EnumCreatePosition)cmd["createPosition"].GetInt());
	CC_ASSERT(cmd.HasMember("useConnect"));
	this->setUseConnect(cmd["useConnect"].GetBool());
	CC_ASSERT(cmd.HasMember("connectId"));
	this->setConnectId(cmd["connectId"].GetInt());
	CC_ASSERT(cmd.HasMember("adjustX"));
	this->setAdjustX(cmd["adjustX"].GetInt());
	CC_ASSERT(cmd.HasMember("adjustY"));
	this->setAdjustY(cmd["adjustY"].GetInt());
	CC_ASSERT(cmd.HasMember("probability"));
	this->setProbability(cmd["probability"].GetDouble());
	CC_ASSERT(cmd.HasMember("childObject"));
	this->setChildObject(cmd["childObject"].GetBool());
	CC_ASSERT(cmd.HasMember("useRotation"));
	this->setUseRotation(cmd["useRotation"].GetBool());
	CC_ASSERT(cmd.HasMember("lowerPriority"));
	this->setLowerPriority(cmd["lowerPriority"].GetBool());
	CC_ASSERT(cmd.HasMember("gridMagnet"));
	this->setGridMagnet(cmd["gridMagnet"].GetBool());
	return true;
}

bool ObjectCommandObjectCreateData::init(void *jsCx, void *jsObj)
{
	_commandType = kObjectCreate;
	//デフォルト値設定。
	this->setObjectId(-1);
	this->setActionId(-1);
	this->setCreatePosition(kPositionCenter);
	this->setUseConnect(false);
	this->setConnectId(-1);
	this->setAdjustX(0);
	this->setAdjustY(0);
	this->setProbability(100);
	this->setChildObject(false);
	this->setUseRotation(false);
	this->setLowerPriority(false);
	this->setGridMagnet(false);

	auto cx = (JSContext *)jsCx;
	JS::RootedValue v(cx);
	JS::RootedObject rparams(cx, (JSObject *)jsObj);
	bool bValue;
	int iValue;
	double dValue;
	if (getJsInt32(cx, rparams, "objectId", &iValue)) {
		this->setObjectId(iValue);
	}
	if (getJsInt32(cx, rparams, "actionId", &iValue)) {
		this->setActionId(iValue);
	}
	if (getJsInt32(cx, rparams, "createPosition", &iValue)) {
		this->setCreatePosition((EnumCreatePosition)iValue);
	}
	if (getJsBoolean(cx, rparams, "useConnect", &bValue)) {
		this->setUseConnect(bValue);
	}
	if (getJsInt32(cx, rparams, "connectId", &iValue)) {
		this->setConnectId(iValue);
	}
	if (getJsInt32(cx, rparams, "adjustX", &iValue)) {
		this->setAdjustX(iValue);
	}
	if (getJsInt32(cx, rparams, "adjustY", &iValue)) {
		this->setAdjustY(iValue);
	}
	if (getJsDouble(cx, rparams, "probability", &dValue)) {
		this->setProbability(dValue);
	}
	if (getJsBoolean(cx, rparams, "childObject", &bValue)) {
		this->setChildObject(bValue);
	}
	if (getJsBoolean(cx, rparams, "useRotation", &bValue)) {
		this->setUseRotation(bValue);
	}
	if (getJsBoolean(cx, rparams, "lowerPriority", &bValue)) {
		this->setLowerPriority(bValue);
	}
	if (getJsBoolean(cx, rparams, "gridMagnet", &bValue)) {
		this->setGridMagnet(bValue);
	}

	return true;
}

#if defined(AGTK_DEBUG)
void ObjectCommandObjectCreateData::dump()
{
	ObjectCommandData::dump();
	CCLOG("objectId:%d",  this->getObjectId());
	CCLOG("actionId:%d", this->getActionId());
	CCLOG("createPosition:%d", this->getCreatePosition());
	CCLOG("useConnect:%d", this->getUseConnect());
	CCLOG("connectId:%d", this->getConnectId());
	CCLOG("adjustX:%d", this->getAdjustX());
	CCLOG("adjustY:%d", this->getAdjustY());
	CCLOG("probability:%f", this->getProbability());
	CCLOG("childObject:%d", this->getChildObject());
	CCLOG("useRotation:%d", this->getUseRotation());
	CCLOG("lowerPriority:%d", this->getLowerPriority());
	CCLOG("gridMagnet:%d", this->getGridMagnet());
}
#endif

//-------------------------------------------------------------------------------------------------------------------
bool ObjectCommandObjectChangeData::init(const rapidjson::Value& json)
{
	if (!ObjectCommandData::init(json)) {
		return false;
	}
	CC_ASSERT(json.HasMember("objectChange"));
	const rapidjson::Value& cmd = json["objectChange"];
	CC_ASSERT(cmd.HasMember("objectId"));
	this->setObjectId(cmd["objectId"].GetInt());
	if (cmd.HasMember("actionId")) {
		this->setActionId(cmd["actionId"].GetInt());
	}
	CC_ASSERT(cmd.HasMember("createPosition"));
	this->setCreatePosition(cmd["createPosition"].GetInt());
	CC_ASSERT(cmd.HasMember("useConnect"));
	this->setUseConnect(cmd["useConnect"].GetBool());
	CC_ASSERT(cmd.HasMember("connectId"));
	this->setConnectId(cmd["connectId"].GetInt());
	CC_ASSERT(cmd.HasMember("adjustX"));
	this->setAdjustX(cmd["adjustX"].GetInt());
	CC_ASSERT(cmd.HasMember("adjustY"));
	this->setAdjustY(cmd["adjustY"].GetInt());
	CC_ASSERT(cmd.HasMember("probability"));
	this->setProbability(cmd["probability"].GetDouble());
	CC_ASSERT(cmd.HasMember("takeOverSwitches"));
	this->setTakeOverSwitches(cmd["takeOverSwitches"].GetBool());
	CC_ASSERT(cmd.HasMember("takeOverVariables"));
	this->setTakeOverVariables(cmd["takeOverVariables"].GetBool());
	CC_ASSERT(cmd.HasMember("takeOverParentChild"));
	this->setTakeOverParentChild(cmd["takeOverParentChild"].GetBool());
	return true;
}

bool ObjectCommandObjectChangeData::init(void *jsCx, void *jsObj)
{
	_commandType = kObjectChange;
	//デフォルト値設定。
	this->setObjectId(-1);
	this->setActionId(-1);
	this->setCreatePosition(kPositionCenter);
	this->setUseConnect(false);
	this->setConnectId(-1);
	this->setAdjustX(0);
	this->setAdjustY(0);
	this->setProbability(100);
	this->setTakeOverSwitches(true);
	this->setTakeOverVariables(true);
	this->setTakeOverParentChild(true);

	auto cx = (JSContext *)jsCx;
	JS::RootedValue v(cx);
	JS::RootedObject rparams(cx, (JSObject *)jsObj);
	bool bValue;
	int iValue;
	double dValue;
	if (getJsInt32(cx, rparams, "objectId", &iValue)) {
		this->setObjectId(iValue);
	}
	if (getJsInt32(cx, rparams, "actionId", &iValue)) {
		this->setActionId(iValue);
	}
	if (getJsBoolean(cx, rparams, "takeOverVariables", &bValue)) {
		this->setTakeOverVariables(bValue);
	}
	if (getJsBoolean(cx, rparams, "takeOverSwitches", &bValue)) {
		this->setTakeOverSwitches(bValue);
	}
	if (getJsBoolean(cx, rparams, "takeOverParentChild", &bValue)) {
		this->setTakeOverParentChild(bValue);
	}
	if (getJsInt32(cx, rparams, "createPosition", &iValue)) {
		this->setCreatePosition(iValue);
	}
	if (getJsBoolean(cx, rparams, "useConnect", &bValue)) {
		this->setUseConnect(bValue);
	}
	if (getJsInt32(cx, rparams, "connectId", &iValue)) {
		this->setConnectId(iValue);
	}
	if (getJsInt32(cx, rparams, "adjustX", &iValue)) {
		this->setAdjustX(iValue);
	}
	if (getJsInt32(cx, rparams, "adjustY", &iValue)) {
		this->setAdjustY(iValue);
	}
	if (getJsDouble(cx, rparams, "probability", &dValue)) {
		this->setProbability(dValue);
	}
	if (getJsBoolean(cx, rparams, "takeOverSwitches", &bValue)) {
		this->setTakeOverSwitches(bValue);
	}
	if (getJsBoolean(cx, rparams, "takeOverVariables", &bValue)) {
		this->setTakeOverVariables(bValue);
	}
	if (getJsBoolean(cx, rparams, "takeOverParentChild", &bValue)) {
		this->setTakeOverParentChild(bValue);
	}

	return true;
}

#if defined(AGTK_DEBUG)
void ObjectCommandObjectChangeData::dump()
{
	ObjectCommandData::dump();
	CCLOG("objectId:%d", this->getObjectId());
	CCLOG("actionId:%d", this->getActionId());
	CCLOG("createPosition:%d", this->getCreatePosition());
	CCLOG("useConnect:%d", this->getUseConnect());
	CCLOG("connectId:%d", this->getConnectId());
	CCLOG("adjustX:%d", this->getAdjustX());
	CCLOG("adjustY:%d", this->getAdjustY());
	CCLOG("probability:%f", this->getProbability());
	CCLOG("takeOverSwitches:%d", this->getTakeOverSwitches());
	CCLOG("takeOverVariables:%d", this->getTakeOverVariables());
	CCLOG("takeOverParentChild:%d", this->getTakeOverParentChild());
}
#endif

//-------------------------------------------------------------------------------------------------------------------
bool ObjectCommandObjectMoveData::init(const rapidjson::Value& json)
{
	if (!ObjectCommandData::init(json)) {
		return false;
	}
	CC_ASSERT(json.HasMember("objectMove"));
	const rapidjson::Value& cmd = json["objectMove"];
	CC_ASSERT(cmd.HasMember("moveType"));
	this->setMoveType((EnumMoveType)cmd["moveType"].GetInt());
	CC_ASSERT(cmd.HasMember("angle"));
	this->setAngle(cmd["angle"].GetDouble());
	CC_ASSERT(cmd.HasMember("moveDistance"));
	this->setMoveDistance(cmd["moveDistance"].GetInt());
	CC_ASSERT(cmd.HasMember("posX"));
	this->setPosX(cmd["posX"].GetInt());
	CC_ASSERT(cmd.HasMember("posY"));
	this->setPosY(cmd["posY"].GetInt());
	if (cmd.HasMember("moveInDisplayCoordinates")) {
		this->setMoveInDisplayCoordinates(cmd["moveInDisplayCoordinates"].GetBool());
	}
	if (cmd.HasMember("followCameraMoving")) {
		this->setFollowCameraMoving(cmd["followCameraMoving"].GetBool());
	}
	CC_ASSERT(cmd.HasMember("centerObjectId"));
	this->setCenterObjectId(cmd["centerObjectId"].GetInt());
	CC_ASSERT(cmd.HasMember("centerQualifierId"));
	this->setCenterQualifierId((EnumQualifierType)cmd["centerQualifierId"].GetInt());
	CC_ASSERT(cmd.HasMember("centerAdjustX"));
	this->setCenterAdjustX(cmd["centerAdjustX"].GetInt());
	CC_ASSERT(cmd.HasMember("centerAdjustY"));
	this->setCenterAdjustY(cmd["centerAdjustY"].GetInt());
	if (cmd.HasMember("connectId")) {
		this->setConnectId(cmd["connectId"].GetInt());
	}
	CC_ASSERT(cmd.HasMember("useObjectParameter"));
	this->setUseObjectParameter(cmd["useObjectParameter"].GetBool());
	CC_ASSERT(cmd.HasMember("changeMoveSpeed"));
	this->setChangeMoveSpeed(cmd["changeMoveSpeed"].GetDouble());
	CC_ASSERT(cmd.HasMember("moveDuration300"));
	this->setMoveDuration300(cmd["moveDuration300"].GetInt());
	CC_ASSERT(cmd.HasMember("targettingType"));
	this->setTargettingType((EnumTargettingType)cmd["targettingType"].GetInt());
	CC_ASSERT(cmd.HasMember("targetObjectGroup"));
	this->setTargetObjectGroup(cmd["targetObjectGroup"].GetInt());
	CC_ASSERT(cmd.HasMember("targetObjectId"));
	this->setTargetObjectId(cmd["targetObjectId"].GetInt());
	CC_ASSERT(cmd.HasMember("targetQualifierId"));
	this->setTargetQualifierId((EnumQualifierType)cmd["targetQualifierId"].GetInt());
	CC_ASSERT(cmd.HasMember("excludeObjectGroupBit"));
	this->setExcludeObjectGroupBit(cmd["excludeObjectGroupBit"].GetInt());
	CC_ASSERT(cmd.HasMember("fitDispDirToMoveDir"));
	this->setFitDispDirToMoveDir(cmd["fitDispDirToMoveDir"].GetBool());
	CC_ASSERT(cmd.HasMember("dispWhileMoving"));
	this->setDispWhileMoving(cmd["dispWhileMoving"].GetBool());
	CC_ASSERT(cmd.HasMember("stopActionWhileMoving"));
	this->setStopActionWhileMoving(cmd["stopActionWhileMoving"].GetBool());
	CC_ASSERT(cmd.HasMember("stopAnimWhileMoving"));
	this->setStopAnimWhileMoving(cmd["stopAnimWhileMoving"].GetBool());
	CC_ASSERT(cmd.HasMember("finalGridMagnet"));
	this->setFinalGridMagnet(cmd["finalGridMagnet"].GetBool());
	return true;
}

bool ObjectCommandObjectMoveData::init(void *jsCx, void *jsObj)
{
	_commandType = kObjectMove;
	//デフォルト値設定。
	this->setMoveType(kMoveWithDirection);
	this->setAngle(0);
	this->setMoveDistance(128);
	this->setPosX(0);
	this->setPosY(0);
	this->setMoveInDisplayCoordinates(false);
	this->setFollowCameraMoving(false);
	this->setCenterObjectId(-1);
	this->setCenterQualifierId(kQualifierSingle);
	this->setCenterAdjustX(0);
	this->setCenterAdjustY(0);
	this->setConnectId(-1);
	this->setUseObjectParameter(true);
	this->setChangeMoveSpeed(100);
	this->setMoveDuration300(300);
	this->setTargettingType(kTargettingByGroup);
	this->setTargetObjectGroup(kObjectTypeAll);
	this->setTargetObjectId(-1);
	this->setTargetQualifierId(kQualifierSingle);
	this->setExcludeObjectGroupBit(0);
	this->setFitDispDirToMoveDir(false);
	this->setDispWhileMoving(true);
	this->setStopActionWhileMoving(false);
	this->setStopAnimWhileMoving(false);
	this->setFinalGridMagnet(false);

	auto cx = (JSContext *)jsCx;
	JS::RootedValue v(cx);
	JS::RootedObject rparams(cx, (JSObject *)jsObj);
	JS_GetProperty(cx, rparams, "moveType", &v);
	bool bValue;
	int iValue;
	double dValue;
	if (getJsInt32(cx, rparams, "moveType", &iValue)) {
		this->setMoveType((EnumMoveType)iValue);
	}
	if (getJsDouble(cx, rparams, "angle", &dValue)) {
		this->setAngle(dValue);
	}
	if (getJsInt32(cx, rparams, "moveDistance", &iValue)) {
		this->setMoveDistance(iValue);
	}
	if (getJsInt32(cx, rparams, "posX", &iValue)) {
		this->setPosX(iValue);
	}
	if (getJsInt32(cx, rparams, "posY", &iValue)) {
		this->setPosY(iValue);
	}
	if (getJsBoolean(cx, rparams, "moveInDisplayCoordinates", &bValue)) {
		this->setMoveInDisplayCoordinates(bValue);
	}
	if (getJsBoolean(cx, rparams, "followCameraMoving", &bValue)) {
		this->setFollowCameraMoving(bValue);
	}
	if (getJsInt32(cx, rparams, "centerObjectId", &iValue)) {
		this->setCenterObjectId(iValue);
	}
	if (getJsInt32(cx, rparams, "centerQualifierId", &iValue)) {
		this->setCenterQualifierId((EnumQualifierType)iValue);
	}
	if (getJsInt32(cx, rparams, "centerAdjustX", &iValue)) {
		this->setCenterAdjustX(iValue);
	}
	if (getJsInt32(cx, rparams, "centerAdjustY", &iValue)) {
		this->setCenterAdjustY(iValue);
	}
	if (getJsInt32(cx, rparams, "connectId", &iValue)) {
		this->setConnectId(iValue);
	}
	if (getJsBoolean(cx, rparams, "useObjectParameter", &bValue)) {
		this->setUseObjectParameter(bValue);
	}
	if (getJsDouble(cx, rparams, "changeMoveSpeed", &dValue)) {
		this->setChangeMoveSpeed(dValue);
	}
	if (getJsInt32(cx, rparams, "moveDuration300", &iValue)) {
		this->setMoveDuration300(iValue);
	}
	if (getJsInt32(cx, rparams, "targettingType", &iValue)) {
		this->setTargettingType((EnumTargettingType)iValue);
	}
	if (isJsDefined(cx, rparams, "targetObjectGroup")) {
		if (getJsInt32(cx, rparams, "targetObjectGroup", &iValue)) {
			this->setTargetObjectGroup(iValue);
		}
	}
	else {
		if (getJsInt32(cx, rparams, "targetObjectType", &iValue)) {
			this->setTargetObjectGroup(iValue - kObjectGroupAllOffset);
		}
	}
	if (getJsInt32(cx, rparams, "targetObjectId", &iValue)) {
		this->setTargetObjectId(iValue);
	}
	if (getJsInt32(cx, rparams, "targetQualifierId", &iValue)) {
		this->setTargetQualifierId((EnumQualifierType)iValue);
	}
	if (isJsDefined(cx, rparams, "excludeObjectGroupBit")) {
		if (getJsInt32(cx, rparams, "excludeObjectGroupBit", &iValue)) {
			this->setExcludeObjectGroupBit(iValue);
		}
	}
	else {
		int excludeObjectGroupBit = 0;
		if (getJsBoolean(cx, rparams, "playerObject", &bValue)) {
			excludeObjectGroupBit |= !bValue ? (1 << kObjectGroupPlayer) : 0;
		}
		if (getJsBoolean(cx, rparams, "enemyObject", &bValue)) {
			excludeObjectGroupBit |= !bValue ? (1 << kObjectGroupEnemy) : 0;
		}
		this->setExcludeObjectGroupBit(excludeObjectGroupBit);
	}
	if (getJsBoolean(cx, rparams, "fitDispDirToMoveDir", &bValue)) {
		this->setFitDispDirToMoveDir(bValue);
	}
	if (getJsBoolean(cx, rparams, "dispWhileMoving", &bValue)) {
		this->setDispWhileMoving(bValue);
	}
	if (getJsBoolean(cx, rparams, "stopActionWhileMoving", &bValue)) {
		this->setStopActionWhileMoving(bValue);
	}
	if (getJsBoolean(cx, rparams, "stopAnimWhileMoving", &bValue)) {
		this->setStopAnimWhileMoving(bValue);
	}
	if (getJsBoolean(cx, rparams, "finalGridMagnet", &bValue)) {
		this->setFinalGridMagnet(bValue);
	}

	return true;
}

#if defined(AGTK_DEBUG)
void ObjectCommandObjectMoveData::dump()
{
	ObjectCommandData::dump();
	CCLOG("moveType:%d", this->getMoveType());
	CCLOG("angle:%f", this->getAngle());
	CCLOG("moveDistance:%d", this->getMoveDistance());
	CCLOG("posX:%d", this->getPosX());
	CCLOG("posY:%d", this->getPosY());
	CCLOG("moveInDisplayCoordinates:%d", this->getMoveInDisplayCoordinates());
	CCLOG("followCameraMoving:%d", this->getFollowCameraMoving());
	CCLOG("centerObjectId:%d", this->getCenterObjectId());
	CCLOG("centerQualifierId:%d", this->getCenterQualifierId());
	CCLOG("centerAdjustX:%d", this->getCenterAdjustX());
	CCLOG("centerAdjustY:%d", this->getCenterAdjustY());
	CCLOG("connectId:%d", this->getConnectId());
	CCLOG("useObjectParameter:%d", this->getUseObjectParameter());
	CCLOG("changeMoveSpeed:%f", this->getChangeMoveSpeed());
	CCLOG("moveDuration300:%d", this->getMoveDuration300());
	CCLOG("targettingType:%d", this->getTargettingType());
	CCLOG("targetObjectGroup:%d", this->getTargetObjectGroup());
	CCLOG("targetObjectId:%d", this->getTargetObjectId());
	CCLOG("targetQualifierId:%d", this->getTargetQualifierId());
	CCLOG("excludeObjectGroupBit:%d", this->getExcludeObjectGroupBit());
	CCLOG("fitDispDirToMoveDir:%d", this->getFitDispDirToMoveDir());
	CCLOG("dispWhileMoving:%d", this->getDispWhileMoving());
	CCLOG("stopActionWhileMoving:%d", this->getStopActionWhileMoving());
	CCLOG("stopAnimWhileMoving:%d", this->getStopAnimWhileMoving());
	CCLOG("finalGridMagnet:%d", this->getFinalGridMagnet());
}
#endif

//-------------------------------------------------------------------------------------------------------------------
bool ObjectCommandObjectPushPullData::init(const rapidjson::Value& json)
{
	if (!ObjectCommandData::init(json)) {
		return false;
	}
	CC_ASSERT(json.HasMember("objectPushPull"));
	const rapidjson::Value& cmd = json["objectPushPull"];
	this->setDirectionType((EnumDirection)cmd["directionType"].GetInt());
	this->setAngle(cmd["angle"].GetDouble());
	this->setConnectId(cmd["connectId"].GetInt());
	this->setRectangle(cmd["rectangle"].GetBool());
	this->setRectangleDistance(cmd["rectangleDistance"].GetInt());
	this->setRectangleHeight(cmd["rectangleHeight"].GetInt());
	this->setCircleDistance(cmd["circleDistance"].GetInt());
	this->setArcAngle(cmd["arcAngle"].GetInt());
	this->setEffectRangeBaseConnect(cmd["effectRangeBaseConnect"].GetBool());
	this->setEffectRangeBaseConnectId(cmd["effectRangeBaseConnectId"].GetInt());
	this->setEffectDirectionType((EnumEffectDirection)cmd["effectDirectionType"].GetInt());
	this->setEffectDirection(cmd["effectDirection"].GetDouble());
	this->setEffectValue(cmd["effectValue"].GetInt());
	this->setDistanceEffect(cmd["distanceEffect"].GetBool());
	this->setNearValue(cmd["nearValue"].GetDouble());
	this->setFarValue(cmd["farValue"].GetDouble());
	this->setOneTimeEffect(cmd["oneTimeEffect"].GetBool());
	this->setTargettingType((EnumTargettingType)cmd["targettingType"].GetInt());
	this->setTargetObjectGroup(cmd["targetObjectGroup"].GetInt());
	this->setTargetObjectId(cmd["targetObjectId"].GetInt());
	this->setTargetQualifierId((EnumQualifierType)cmd["targetQualifierId"].GetInt());
	this->setExcludeObjectGroupBit(cmd["excludeObjectGroupBit"].GetInt());
	return true;
}

bool ObjectCommandObjectPushPullData::init(void *jsCx, void *jsObj)
{
	_commandType = kObjectPushPull;
	//デフォルト値設定。
	this->setEffectRangeBaseConnect(false);
	this->setEffectRangeBaseConnectId(-1);
	this->setRectangle(true);
	this->setRectangleDistance(64);
	this->setRectangleHeight(64);
	this->setCircleDistance(64);
	this->setArcAngle(360);
	this->setEffectDirectionType(kEffectDirectionAngle);
	this->setEffectDirection(0);
	this->setDirectionType(kDirectionAngle);
	this->setAngle(0);
	this->setConnectId(-1);
	this->setEffectValue(100);
	this->setDistanceEffect(false);
	this->setNearValue(100);
	this->setFarValue(100);
	this->setOneTimeEffect(false);
	this->setTargettingType(kTargettingByGroup);
	this->setTargetObjectGroup(kObjectTypeAll);
	this->setTargetObjectId(-1);
	this->setTargetQualifierId(kQualifierSingle);
	this->setExcludeObjectGroupBit(0);

	auto cx = (JSContext *)jsCx;
	JS::RootedValue v(cx);
	JS::RootedObject rparams(cx, (JSObject *)jsObj);
	bool bValue;
	int iValue;
	double dValue;
	if (getJsBoolean(cx, rparams, "effectRangeBaseConnect", &bValue)) {
		this->setEffectRangeBaseConnect(bValue);
	}
	if (getJsInt32(cx, rparams, "effectRangeBaseConnectId", &iValue)) {
		this->setEffectRangeBaseConnectId(iValue);
	}
	if (getJsBoolean(cx, rparams, "rectangle", &bValue)) {
		this->setRectangle(bValue);
	}
	if (getJsInt32(cx, rparams, "rectangleDistance", &iValue)) {
		this->setRectangleDistance(iValue);
	}
	if (getJsInt32(cx, rparams, "rectangleHeight", &iValue)) {
		this->setRectangleHeight(iValue);
	}
	if (getJsInt32(cx, rparams, "circleDistance", &iValue)) {
		this->setCircleDistance(iValue);
	}
	if (getJsInt32(cx, rparams, "arcAngle", &iValue)) {
		this->setArcAngle(iValue);
	}
	if (getJsInt32(cx, rparams, "effectDirectionType", &iValue)) {
		this->setEffectDirectionType((EnumEffectDirection)iValue);
	}
	if (getJsDouble(cx, rparams, "effectDirection", &dValue)) {
		this->setEffectDirection(dValue);
	}
	if (getJsInt32(cx, rparams, "directionType", &iValue)) {
		this->setDirectionType((EnumDirection)iValue);
	}
	if (getJsDouble(cx, rparams, "angle", &dValue)) {
		this->setAngle(dValue);
	}
	if (getJsInt32(cx, rparams, "connectId", &iValue)) {
		this->setConnectId(iValue);
	}
	if (getJsInt32(cx, rparams, "effectValue", &iValue)) {
		this->setEffectValue(iValue);
	}
	if (getJsBoolean(cx, rparams, "distanceEffect", &bValue)) {
		this->setDistanceEffect(bValue);
	}
	if (getJsDouble(cx, rparams, "nearValue", &dValue)) {
		this->setNearValue(dValue);
	}
	if (getJsDouble(cx, rparams, "farValue", &dValue)) {
		this->setFarValue(dValue);
	}
	if (getJsBoolean(cx, rparams, "oneTimeEffect", &bValue)) {
		this->setOneTimeEffect(bValue);
	}
	if (getJsInt32(cx, rparams, "targettingType", &iValue)) {
		this->setTargettingType((EnumTargettingType)iValue);
	}
	if (isJsDefined(cx, rparams, "targetObjectGroup")) {
		if (getJsInt32(cx, rparams, "targetObjectGroup", &iValue)) {
			this->setTargetObjectGroup(iValue);
		}
	}
	else {
		if (getJsInt32(cx, rparams, "targetObjectType", &iValue)) {
			this->setTargetObjectGroup(iValue - kObjectGroupAllOffset);
		}
	}
	if (getJsInt32(cx, rparams, "targetObjectId", &iValue)) {
		this->setTargetObjectId(iValue);
	}
	if (getJsInt32(cx, rparams, "targetQualifierId", &iValue)) {
		this->setTargetQualifierId((EnumQualifierType)iValue);
	}
	if (isJsDefined(cx, rparams, "excludeObjectGroupBit")) {
		if (getJsInt32(cx, rparams, "excludeObjectGroupBit", &iValue)) {
			this->setExcludeObjectGroupBit(iValue);
		}
	}
	else {
		int excludeObjectGroupBit = 0;
		if (getJsBoolean(cx, rparams, "playerObject", &bValue)) {
			excludeObjectGroupBit |= !bValue ? (1 << kObjectGroupPlayer) : 0;
		}
		if (getJsBoolean(cx, rparams, "enemyObject", &bValue)) {
			excludeObjectGroupBit |= !bValue ? (1 << kObjectGroupEnemy) : 0;
		}
		this->setExcludeObjectGroupBit(excludeObjectGroupBit);
	}
	
	return true;
}

#if defined(AGTK_DEBUG)
void ObjectCommandObjectPushPullData::dump()
{
	ObjectCommandData::dump();

	CCLOG("directionType:%d", this->getDirectionType());
	CCLOG("angle:%f", this->getAngle());
	CCLOG("connectId:%d", this->getConnectId());
	CCLOG("rectangle:%d", this->getRectangle());
	CCLOG("rectangleDistance:%d", this->getRectangleDistance());
	CCLOG("rectangleHeight:%d", this->getRectangleHeight());
	CCLOG("circleDistance:%d", this->getCircleDistance());
	CCLOG("arcAngle:%d", this->getArcAngle());
	CCLOG("effectRangeBaseConnect:%d", this->getEffectRangeBaseConnect());
	CCLOG("effectRangeBaseConnectId:%d", this->getEffectRangeBaseConnectId());
	CCLOG("effectDirectionType:%d", this->getEffectDirectionType());
	CCLOG("effectDirection:%f", this->getEffectDirection());
	CCLOG("effectValue:%d", this->getEffectValue());
	CCLOG("distanceEffect:%d", this->getDistanceEffect());
	CCLOG("nearValue:%f", this->getNearValue());
	CCLOG("farValue:%f", this->getFarValue());
	CCLOG("oneTimeEffect:%d", this->getOneTimeEffect());
	CCLOG("targettingType:%d", this->getTargettingType());
	CCLOG("targetObjectGroup:%d", this->getTargetObjectGroup());
	CCLOG("targetObjectId:%d", this->getTargetObjectId());
	CCLOG("targetQualifierId:%d", this->getTargetQualifierId());
	CCLOG("excludeObjectGroupBit:%d", this->getExcludeObjectGroupBit());
}
#endif

//-------------------------------------------------------------------------------------------------------------------
bool ObjectCommandParentChildChangeData::init(const rapidjson::Value& json)
{
	if (!ObjectCommandData::init(json)) {
		return false;
	}
	CC_ASSERT(json.HasMember("parentChildChange"));
	const rapidjson::Value& cmd = json["parentChildChange"];
	this->setRelation(cmd["relation"].GetInt());
	this->setTargettingType(cmd["targettingType"].GetInt());
	this->setTargetObjectId(cmd["targetObjectId"].GetInt());
	this->setIgnorePlayerObject(cmd["ignorePlayerObject"].GetBool());
	this->setIgnoreEnemyObject(cmd["ignoreEnemyObject"].GetBool());
	this->setChangeTimingPosition(cmd["changeTimingPosition"].GetBool());
	this->setConnectId(cmd["connectId"].GetInt());
	this->setAdjustX(cmd["adjustX"].GetInt());
	this->setAdjustY(cmd["adjustY"].GetInt());
	return true;
}

#if defined(AGTK_DEBUG)
void ObjectCommandParentChildChangeData::dump()
{
	ObjectCommandData::dump();
	CCLOG("relation:%d", this->getRelation());
	CCLOG("targettingType:%d", this->getTargettingType());
	CCLOG("targetObjectId:%d", this->getTargetObjectId());
	CCLOG("ignorePlayerObject:%d", this->getIgnorePlayerObject());
	CCLOG("ignoreEnemyObject:%d", this->getIgnoreEnemyObject());
	CCLOG("changeTimingPosition:%d", this->getChangeTimingPosition());
	CCLOG("connectId:%d", this->getConnectId());
	CCLOG("adjustX:%d", this->getAdjustX());
	CCLOG("adjustY:%d", this->getAdjustY());
}
#endif

//-------------------------------------------------------------------------------------------------------------------
bool ObjectCommandLayerMoveData::init(const rapidjson::Value& json)
{
	if (!ObjectCommandData::init(json)) {
		return false;
	}
	CC_ASSERT(json.HasMember("layerMove"));
	const rapidjson::Value& cmd = json["layerMove"];
	this->setLayerIndex(cmd["layerIndex"].GetInt());
	return true;
}

bool ObjectCommandLayerMoveData::init(void *jsCx, void *jsObj)
{
	_commandType = kLayerMove;
	//デフォルト値設定。
	this->setLayerIndex(-1);

	auto cx = (JSContext *)jsCx;
	JS::RootedValue v(cx);
	JS::RootedObject rparams(cx, (JSObject *)jsObj);
	int iValue;
	if (getJsInt32(cx, rparams, "layerIndex", &iValue)) {
		this->setLayerIndex(iValue);
	}

	return true;
}

#if defined(AGTK_DEBUG)
void ObjectCommandLayerMoveData::dump()
{
	ObjectCommandData::dump();
	CCLOG("layerIndex:%d", this->getLayerIndex());
}
#endif

//-------------------------------------------------------------------------------------------------------------------
bool ObjectCommandAttackSettingData::init(const rapidjson::Value& json)
{
	if (!ObjectCommandData::init(json)) {
		return false;
	}
	CC_ASSERT(json.HasMember("attackSetting"));
	const rapidjson::Value& cmd = json["attackSetting"];
	this->setAttackChange(cmd["attackChange"].GetDouble());
	this->setHitObjectFlag(cmd["hitObjectFlag"].GetBool());
	this->setObjectGroupBit(cmd["objectGroupBit"].GetInt());
	this->setHitTileFlag(cmd["hitTileFlag"].GetBool());
	this->setTileGroupBit(cmd["tileGroupBit"].GetInt());
	this->setAttributeType(cmd["attributeType"].GetInt());
	this->setAttributePresetId(cmd["attributePresetId"].GetInt());
	this->setAttributeValue(cmd["attributeValue"].GetInt());
	return true;
}

bool ObjectCommandAttackSettingData::init(void *jsCx, void *jsObj)
{
	_commandType = kAttackSetting;
	//デフォルト値設定。
	this->setAttackChange(100);
	this->setHitObjectFlag(false);
	this->setObjectGroupBit(0);
	this->setHitTileFlag(false);
	this->setTileGroupBit(0);
	this->setAttributeType(kNone);
	this->setAttributePresetId(1);
	this->setAttributeValue(0);

	auto cx = (JSContext *)jsCx;
	JS::RootedValue v(cx);
	JS::RootedObject rparams(cx, (JSObject *)jsObj);
	bool bValue;
	int iValue;
	double dValue;
	if (getJsDouble(cx, rparams, "attackChange", &dValue)) {
		this->setAttackChange(dValue);
	}
	if (getJsBoolean(cx, rparams, "hitObjectFlag", &bValue)) {
		this->setHitObjectFlag(bValue);
	}
	if (isJsDefined(cx, rparams, "objectGroupBit")) {
		if (getJsInt32(cx, rparams, "objectGroupBit", &iValue)) {
			this->setObjectGroupBit(iValue);
		}
	}
	else {
		int objectGroupBit = 0;
		if (getJsBoolean(cx, rparams, "playerAttackArea", &bValue)) {
			objectGroupBit |= !bValue ? (1 << kObjectGroupPlayer) : 0;
		}
		if (getJsBoolean(cx, rparams, "enemyAttackArea", &bValue)) {
			objectGroupBit |= !bValue ? (1 << kObjectGroupEnemy) : 0;
		}
		this->setObjectGroupBit(objectGroupBit);
		if (!isJsDefined(cx, rparams, "objectGroupBit")) {
			this->setHitObjectFlag(objectGroupBit ? true : false);
		}
	}
	if (getJsBoolean(cx, rparams, "hitTileFlag", &bValue)) {
		this->setHitTileFlag(bValue);
	}
	if (isJsDefined(cx, rparams, "tileGroupBit")) {
		if (getJsInt32(cx, rparams, "tileGroupBit", &iValue)) {
			this->setTileGroupBit(iValue);
		}
	}
	else {
		int tileGroupBit = 0;
		if (getJsBoolean(cx, rparams, "tileAttackArea:", &bValue)) {
			tileGroupBit |= !bValue ? (1 << kTileGroupDefault) : 0;
		}
		this->setTileGroupBit(tileGroupBit);
		if (!isJsDefined(cx, rparams, "hitTileFlag")) {
			this->setHitTileFlag(tileGroupBit ? true : false);
		}
	}
	if (getJsInt32(cx, rparams, "attributeType", &iValue)) {
		this->setAttributeType(iValue);
	}
	if (getJsInt32(cx, rparams, "attributePresetId", &iValue)) {
		this->setAttributePresetId(iValue);
	}
	if (getJsInt32(cx, rparams, "attributeValue", &iValue)) {
		this->setAttributeValue(iValue);
	}

	return true;
}

#if defined(AGTK_DEBUG)
void ObjectCommandAttackSettingData::dump()
{
	ObjectCommandData::dump();
	CCLOG("attackChange:%f", this->getAttackChange());
	CCLOG("hitObjectFlag:%d", this->getHitObjectFlag());
	CCLOG("objectGroupBit:%d", this->getObjectGroupBit());
	CCLOG("hitTileFlag:%d", this->getHitTileFlag());
	CCLOG("tileGroupBit:%d", this->getTileGroupBit());
	CCLOG("attributeType:%d", this->getAttributeType());
	CCLOG("attributePresetId:%d", this->getAttributePresetId());
	CCLOG("attributeValue:%d", this->getAttributeValue());
}
#endif

//-------------------------------------------------------------------------------------------------------------------
bool ObjectCommandBulletFireData::init(const rapidjson::Value& json)
{
	if (!ObjectCommandData::init(json)) {
		return false;
	}
	CC_ASSERT(json.HasMember("bulletFire"));
	const rapidjson::Value& cmd = json["bulletFire"];
	this->setBulletId(cmd["bulletId"].GetInt());
	this->setConnectId(cmd["connectId"].GetInt());
	return true;
}

bool ObjectCommandBulletFireData::init(void *jsCx, void *jsObj)
{
	_commandType = kBulletFire;
	//デフォルト値設定。
	this->setBulletId(-1);
	this->setConnectId(-1);

	auto cx = (JSContext *)jsCx;
	JS::RootedValue v(cx);
	JS::RootedObject rparams(cx, (JSObject *)jsObj);
	int iValue;
	if (getJsInt32(cx, rparams, "bulletId", &iValue)) {
		this->setBulletId(iValue);
	}
	if (getJsInt32(cx, rparams, "connectId", &iValue)) {
		this->setConnectId(iValue);
	}

	return true;
}

#if defined(AGTK_DEBUG)
void ObjectCommandBulletFireData::dump()
{
	ObjectCommandData::dump();
	CCLOG("bulletId:%d", this->getBulletId());
	CCLOG("connectId:%d", this->getConnectId());
}
#endif

//-------------------------------------------------------------------------------------------------------------------
bool ObjectCommandDisappearData::init(const rapidjson::Value& json)
{
	if (!ObjectCommandData::init(json)) {
		return false;
	}
	CC_ASSERT(json.HasMember("disappear"));;
	return true;
}

bool ObjectCommandDisappearData::init(void *jsCx, void *jsObj)
{
	_commandType = kDisappear;

	return true;
}

#if defined(AGTK_DEBUG)
void ObjectCommandDisappearData::dump()
{
	ObjectCommandData::dump();
}
#endif

//-------------------------------------------------------------------------------------------------------------------
bool ObjectCommandDisappearObjectRecoverData::init(const rapidjson::Value& json)
{
	if (!ObjectCommandData::init(json)) {
		return false;
	}
	CC_ASSERT(json.HasMember("disappearObjectRecover"));
	const rapidjson::Value& cmd = json["disappearObjectRecover"];
	if (cmd.HasMember("objectId")) {
		this->setObjectId(cmd["objectId"].GetInt());
	}
	return true;
}

bool ObjectCommandDisappearObjectRecoverData::init(void *jsCx, void *jsObj)
{
	_commandType = kDisappearObjectRecover;
	//デフォルト値設定。
	this->setObjectId(-1);

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
void ObjectCommandDisappearObjectRecoverData::dump()
{
	ObjectCommandData::dump();
	CCLOG("objectId:%d", this->getObjectId());
}
#endif

//-------------------------------------------------------------------------------------------------------------------
bool ObjectCommandDisableData::init(const rapidjson::Value& json)
{
	if (!ObjectCommandData::init(json)) {
		return false;
	}
	CC_ASSERT(json.HasMember("disable"));
	return true;
}

bool ObjectCommandDisableData::init(void *jsCx, void *jsObj)
{
	_commandType = kDisable;

	return true;
}

#if defined(AGTK_DEBUG)
void ObjectCommandDisableData::dump()
{
	ObjectCommandData::dump();
}
#endif

//-------------------------------------------------------------------------------------------------------------------
bool ObjectCommandDisableObjectEnableData::init(const rapidjson::Value& json)
{
	if (!ObjectCommandData::init(json)) {
		return false;
	}
	CC_ASSERT(json.HasMember("disableObjectEnable"));
	const rapidjson::Value& cmd = json["disableObjectEnable"];
	if (cmd.HasMember("objectId")) {
		this->setObjectId(cmd["objectId"].GetInt());
	}
	return true;
}

bool ObjectCommandDisableObjectEnableData::init(void *jsCx, void *jsObj)
{
	_commandType = kDisableObjectEnable;
	//デフォルト値設定。
	this->setObjectId(-1);

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
void ObjectCommandDisableObjectEnableData::dump()
{
	ObjectCommandData::dump();
	CCLOG("objectId:%d", this->getObjectId());
}
#endif

//-------------------------------------------------------------------------------------------------------------------
ObjectCommandObjectFilterEffectData::ObjectCommandObjectFilterEffectData() : ObjectCommandData()
{
	_filterEffect = nullptr;
}

ObjectCommandObjectFilterEffectData::~ObjectCommandObjectFilterEffectData()
{
	CC_SAFE_RELEASE_NULL(_filterEffect);
}

bool ObjectCommandObjectFilterEffectData::init(const rapidjson::Value& json)
{
	if (!ObjectCommandData::init(json)) {
		return false;
	}
	CC_ASSERT(json.HasMember("objectFilterEffect"));
	const rapidjson::Value& cmd = json["objectFilterEffect"];
	this->setFilterEffect(FilterEffect::create(cmd));
	return true;
}

bool ObjectCommandObjectFilterEffectData::init(void *jsCx, void *jsObj)
{
	_commandType = kObjectFilterEffect;

	this->setFilterEffect(FilterEffect::create(jsCx, jsObj));

	return true;
}

#if defined(AGTK_DEBUG)
void ObjectCommandObjectFilterEffectData::dump()
{
	ObjectCommandData::dump();
	CCLOG("objectFilterEffect");
	this->getFilterEffect()->dump();
};
#endif

//-------------------------------------------------------------------------------------------------------------------
bool ObjectCommandObjectFilterEffectRemoveData::init(const rapidjson::Value& json)
{
	if (!ObjectCommandData::init(json)) {
		return false;
	}
	CC_ASSERT(json.HasMember("objectFilterEffectRemove"));
	const rapidjson::Value& cmd = json["objectFilterEffectRemove"];
	this->setRemoveBit(cmd["removeBit"].GetInt());
	this->setDuration300(cmd["duration300"].GetInt());
	return true;
}

bool ObjectCommandObjectFilterEffectRemoveData::init(void *jsCx, void *jsObj)
{
	_commandType = kObjectFilterEffectRemove;
	//デフォルト値設定。
	this->setRemoveBit(0);
	this->setDuration300(300);

	auto cx = (JSContext *)jsCx;
	JS::RootedValue v(cx);
	JS::RootedObject rparams(cx, (JSObject *)jsObj);
	int iValue;
	if (getJsInt32(cx, rparams, "removeBit", &iValue)) {
		this->setRemoveBit(iValue);
	}
	if (getJsInt32(cx, rparams, "duration300", &iValue)) {
		this->setDuration300(iValue);
	}

	return true;
}

#if defined(AGTK_DEBUG)
void ObjectCommandObjectFilterEffectRemoveData::dump()
{
	ObjectCommandData::dump();
	CCLOG("removeBit:%d", this->getRemoveBit());
	CCLOG("duration300:%d", this->getDuration300());
}
#endif

//-------------------------------------------------------------------------------------------------------------------
ObjectCommandSceneEffectData::ObjectCommandSceneEffectData() : ObjectCommandData()
{
	_filterEffect = nullptr;
}

ObjectCommandSceneEffectData::~ObjectCommandSceneEffectData()
{
	CC_SAFE_RELEASE_NULL(_filterEffect);
}

bool ObjectCommandSceneEffectData::init(const rapidjson::Value& json)
{
	if (!ObjectCommandData::init(json)) {
		return false;
	}
	CC_ASSERT(json.HasMember("sceneEffect"));
	const rapidjson::Value& cmd = json["sceneEffect"];
	this->setLayerIndex(cmd["layerIndex"].GetInt());
	auto filterEffect = FilterEffect::create(cmd["filterEffect"]);
	if (filterEffect == nullptr) {
		return false;
	}
	this->setFilterEffect(filterEffect);
	return true;
}

bool ObjectCommandSceneEffectData::init(void *jsCx, void *jsObj)
{
	_commandType = kSceneEffect;
	//デフォルト値設定。
	this->setLayerIndex(-1);

	auto cx = (JSContext *)jsCx;
	JS::RootedValue v(cx);
	JS::RootedObject rparams(cx, (JSObject *)jsObj);
	int iValue;
	if (getJsInt32(cx, rparams, "layerIndex", &iValue)) {
		this->setLayerIndex(iValue);
	}
	JS_GetProperty(cx, rparams, "filterEffect", &v);
	auto &obj = v.toObject();
	auto filterEffect = FilterEffect::create(jsCx, &obj);
	if (filterEffect == nullptr) {
		return false;
	}
	this->setFilterEffect(filterEffect);

	return true;
}

#if defined(AGTK_DEBUG)
void ObjectCommandSceneEffectData::dump()
{
	ObjectCommandData::dump();
	CCLOG("layerIndex:%d", this->getLayerIndex());
	this->getFilterEffect()->dump();
}
#endif

//-------------------------------------------------------------------------------------------------------------------
bool ObjectCommandSceneEffectRemoveData::init(const rapidjson::Value& json)
{
	if (!ObjectCommandData::init(json)) {
		return false;
	}
	CC_ASSERT(json.HasMember("sceneEffectRemove"));
	const rapidjson::Value& cmd = json["sceneEffectRemove"];
	this->setLayerIndex(cmd["layerIndex"].GetInt());
	this->setRemoveBit(cmd["removeBit"].GetInt());
	this->setDuration300(cmd["duration300"].GetInt());
	return true;
}

bool ObjectCommandSceneEffectRemoveData::init(void *jsCx, void *jsObj)
{
	_commandType = kSceneEffectRemove;
	//デフォルト値設定。
	this->setLayerIndex(-1);
	this->setRemoveBit(0);
	this->setDuration300(300);

	auto cx = (JSContext *)jsCx;
	JS::RootedValue v(cx);
	JS::RootedObject rparams(cx, (JSObject *)jsObj);
	int iValue;
	if (getJsInt32(cx, rparams, "layerIndex", &iValue)) {
		this->setLayerIndex(iValue);
	}
	if (getJsInt32(cx, rparams, "removeBit", &iValue)) {
		this->setRemoveBit(iValue);
	}
	if (getJsInt32(cx, rparams, "duration300", &iValue)) {
		this->setDuration300(iValue);
	}

	return true;
}

#if defined(AGTK_DEBUG)
void ObjectCommandSceneEffectRemoveData::dump()
{
	ObjectCommandData::dump();
	CCLOG("layerIndex:%d", this->getLayerIndex());
	CCLOG("removeBit:%d", this->getRemoveBit());
	CCLOG("duration300:%d", this->getDuration300());
}
#endif

//-------------------------------------------------------------------------------------------------------------------
bool ObjectCommandSceneGravityChangeData::init(const rapidjson::Value& json)
{
	if (!ObjectCommandData::init(json)) {
		return false;
	}
	CC_ASSERT(json.HasMember("sceneGravityChange"));
	const rapidjson::Value& cmd = json["sceneGravityChange"];
	this->setGravity(cmd["gravity"].GetDouble());
	this->setDirection(cmd["direction"].GetDouble());
	this->setDuration300(cmd["duration300"].GetInt());
	this->setDurationUnlimited(cmd["durationUnlimited"].GetBool());
	return true;
}

bool ObjectCommandSceneGravityChangeData::init(void *jsCx, void *jsObj)
{
	_commandType = kSceneGravityChange;
	//デフォルト値設定。
	this->setGravity(100);
	this->setDirection(180);
	this->setDuration300(300);
	this->setDurationUnlimited(false);

	auto cx = (JSContext *)jsCx;
	JS::RootedValue v(cx);
	JS::RootedObject rparams(cx, (JSObject *)jsObj);
	bool bValue;
	int iValue;
	double dValue;
	if (getJsDouble(cx, rparams, "gravity", &dValue)) {
		this->setGravity(dValue);
	}
	if (getJsDouble(cx, rparams, "direction", &dValue)) {
		this->setDirection(dValue);
	}
	if (getJsInt32(cx, rparams, "duration300", &iValue)) {
		this->setDuration300(iValue);
	}
	if (getJsBoolean(cx, rparams, "durationUnlimited", &bValue)) {
		this->setDurationUnlimited(bValue);
	}

	return true;
}

#if defined(AGTK_DEBUG)
void ObjectCommandSceneGravityChangeData::dump()
{
	ObjectCommandData::dump();
	CCLOG("gravity:%f", this->getGravity());
	CCLOG("direction:%f", this->getDirection());
	CCLOG("duration300:%d", this->getDuration300());
	CCLOG("durationUnlimited:%d", this->getDurationUnlimited());
};
#endif

//-------------------------------------------------------------------------------------------------------------------
bool ObjectCommandSceneRotateFlipData::init(const rapidjson::Value& json)
{
	if (!ObjectCommandData::init(json)) {
		return false;
	}
	CC_ASSERT(json.HasMember("sceneRotateFlip"));
	const rapidjson::Value& cmd = json["sceneRotateFlip"];
	if (cmd.HasMember("type")) {
		this->setType(cmd["type"].GetInt());
	}
	this->setRotationFlag(cmd["rotationFlag"].GetBool());
	this->setRotation(cmd["rotation"].GetDouble());
	if (cmd.HasMember("absoluteRotation")) {
		this->setAbsoluteRotation(cmd["absoluteRotation"].GetBool());
	}
	this->setFlipX(cmd["flipX"].GetBool());
	this->setFlipY(cmd["flipY"].GetBool());
	this->setDuration300(cmd["duration300"].GetInt());
	//this->setPlayerObject(cmd["playerObject"].GetBool());
	//this->setEnemyObject(cmd["enemyObject"].GetBool());
	//this->setGravityDirection(cmd["gravityDirection"].GetBool());
	return true;
}

bool ObjectCommandSceneRotateFlipData::init(void *jsCx, void *jsObj)
{
	_commandType = kSceneRotateFlip;
	//デフォルト値設定。
	this->setType(kTypeRotationFlip);
	this->setRotationFlag(false);
	this->setRotation(0);
	this->setAbsoluteRotation(false);
	this->setFlipX(false);
	this->setFlipY(false);
	this->setDuration300(300);

	auto cx = (JSContext *)jsCx;
	JS::RootedValue v(cx);
	JS::RootedObject rparams(cx, (JSObject *)jsObj);
	bool bValue;
	int iValue;
	double dValue;
	if (getJsInt32(cx, rparams, "type", &iValue)) {
		this->setType(iValue);
	}
	if (getJsBoolean(cx, rparams, "rotationFlag", &bValue)) {
		this->setRotationFlag(bValue);
	}
	if (getJsDouble(cx, rparams, "rotation", &dValue)) {
		this->setRotation(dValue);
	}
	if (getJsBoolean(cx, rparams, "absoluteRotation", &bValue)) {
		this->setAbsoluteRotation(bValue);
	}
	if (getJsBoolean(cx, rparams, "flipX", &bValue)) {
		this->setFlipX(bValue);
	}
	if (getJsBoolean(cx, rparams, "flipY", &bValue)) {
		this->setFlipY(bValue);
	}
	if (getJsInt32(cx, rparams, "duration300", &iValue)) {
		this->setDuration300(iValue);
	}

	return true;
}

#if defined(AGTK_DEBUG)
void ObjectCommandSceneRotateFlipData::dump()
{
	ObjectCommandData::dump();
	CCLOG("type:%d", this->getType());
	CCLOG("rotationFlag:%d", this->getRotationFlag());
	CCLOG("rotation:%f", this->getRotation());
	CCLOG("absoluteRotation:%d", this->getAbsoluteRotation());
	CCLOG("flipX:%d", this->getFlipX());
	CCLOG("flipY:%d", this->getFlipY());
	CCLOG("duration300:%d", this->getDuration300());
	//CCLOG("playerObject:%d", this->getPlayerObject());
	//CCLOG("enemyObject:%d", this->getEnemyObject());
	//CCLOG("gravityDirection:%d", this->getGravityDirection());
}
#endif

//-------------------------------------------------------------------------------------------------------------------
bool ObjectCommandCameraAreaChangeData::init(const rapidjson::Value& json)
{
	if (!ObjectCommandData::init(json)) {
		return false;
	}
	CC_ASSERT(json.HasMember("cameraAreaChange"));
	const rapidjson::Value& cmd = json["cameraAreaChange"];
	this->setXRate(cmd["xRate"].GetDouble());
	this->setYRate(cmd["yRate"].GetDouble());
	this->setDuration300(cmd["duration300"].GetInt());
	this->setPositionType(agtk::data::ObjectCommandCameraAreaChangeData::kPositionCenter);
	return true;
}

bool ObjectCommandCameraAreaChangeData::init(void *jsCx, void *jsObj)
{
	_commandType = kCameraAreaChange;
	//デフォルト値設定。
	this->setXRate(1);
	this->setYRate(1);
	this->setDuration300(300);
	this->setPositionType(agtk::data::ObjectCommandCameraAreaChangeData::kPositionCenter);

	auto cx = (JSContext *)jsCx;
	JS::RootedValue v(cx);
	JS::RootedObject rparams(cx, (JSObject *)jsObj);
	int iValue;
	double dValue;
	if (getJsDouble(cx, rparams, "xRate", &dValue)) {
		this->setXRate(dValue);
	}
	if (getJsDouble(cx, rparams, "yRate", &dValue)) {
		this->setYRate(dValue);
	}
	if (getJsInt32(cx, rparams, "duration300", &iValue)) {
		this->setDuration300(iValue);
	}

	return true;
}

#if defined(AGTK_DEBUG)
void ObjectCommandCameraAreaChangeData::dump()
{
	ObjectCommandData::dump();
	CCLOG("xRate:%f", this->getXRate());
	CCLOG("yRate:%f", this->getYRate());
	CCLOG("duration300:%d", this->getDuration300());
	CCLOG("positionType:%d", this->getPositionType());
}
#endif

//-------------------------------------------------------------------------------------------------------------------
bool ObjectCommandSoundPlayData::init(const rapidjson::Value& json)
{
	if (!ObjectCommandData::init(json)) {
		return false;
	}
	CC_ASSERT(json.HasMember("soundPlay"));
	const rapidjson::Value& cmd = json["soundPlay"];
	this->setSoundType((EnumSound)cmd["soundType"].GetInt());
	this->setSeId(cmd["seId"].GetInt());
	this->setVoiceId(cmd["voiceId"].GetInt());
	this->setBgmId(cmd["bgmId"].GetInt());
	if (cmd.HasMember("loop")) {
		this->setLoop(cmd["loop"].GetBool());
	}
	if (cmd.HasMember("fadein")) {
		this->setFadein(cmd["fadein"].GetBool());
	}
	if (cmd.HasMember("duration300")) {
		this->setDuration300(cmd["duration300"].GetInt());
	}
	if (cmd.HasMember("volume")) {
		this->setVolume(cmd["volume"].GetInt());
	}
	if (cmd.HasMember("pan")) {
		this->setPan(cmd["pan"].GetInt());
	}
	if (cmd.HasMember("pitch")) {
		this->setPitch(cmd["pitch"].GetInt());
	}
	if (cmd.HasMember("specifyAudioPosition")) {
		this->setSpecifyAudioPosition(cmd["specifyAudioPosition"].GetBool());
	}
	if (cmd.HasMember("audioPositionVariableObjectId")) {
		this->setAudioPositionVariableObjectId(cmd["audioPositionVariableObjectId"].GetInt());
	}
	if (cmd.HasMember("audioPositionVariableQualifierId")) {
		this->setAudioPositionVariableQualifierId((EnumQualifierType)cmd["audioPositionVariableQualifierId"].GetInt());
	}
	if (cmd.HasMember("audioPositionVariableId")) {
		this->setAudioPositionVariableId(cmd["audioPositionVariableId"].GetInt());
	}
	return true;
}

bool ObjectCommandSoundPlayData::init(void *jsCx, void *jsObj)
{
	_commandType = kSoundPlay;
	//デフォルト値設定。
	this->setSoundType(kSoundSe);
	this->setSeId(-1);
	this->setVoiceId(-1);
	this->setBgmId(-1);
	this->setLoop(false);
	this->setFadein(false);
	this->setDuration300(300);
	this->setVolume(100);
	this->setPan(0);
	this->setPitch(0);
	this->setSpecifyAudioPosition(false);
	this->setAudioPositionVariableObjectId(-1);
	this->setAudioPositionVariableQualifierId(kQualifierSingle);
	this->setAudioPositionVariableId(-1);

	auto cx = (JSContext *)jsCx;
	JS::RootedValue v(cx);
	JS::RootedObject rparams(cx, (JSObject *)jsObj);
	bool bValue;
	int iValue;
	//double dValue;
	if (getJsInt32(cx, rparams, "soundType", &iValue)) {
		this->setSoundType((EnumSound)iValue);
	}
	if (getJsInt32(cx, rparams, "seId", &iValue)) {
		this->setSeId(iValue);
	}
	if (getJsInt32(cx, rparams, "voiceId", &iValue)) {
		this->setVoiceId(iValue);
	}
	if (getJsInt32(cx, rparams, "bgmId", &iValue)) {
		this->setBgmId(iValue);
	}
	if (getJsBoolean(cx, rparams, "loop", &bValue)) {
		this->setLoop(bValue);
	}
	if (getJsBoolean(cx, rparams, "fadein", &bValue)) {
		this->setFadein(bValue);
	}
	if (getJsInt32(cx, rparams, "duration300", &iValue)) {
		this->setDuration300(iValue);
	}
	if (getJsInt32(cx, rparams, "volume", &iValue)) {
		this->setVolume(iValue);
	}
	if (getJsInt32(cx, rparams, "pan", &iValue)) {
		this->setPan(iValue);
	}
	if (getJsInt32(cx, rparams, "pitch", &iValue)) {
		this->setPitch(iValue);
	}
	if (getJsBoolean(cx, rparams, "specifyAudioPosition", &bValue)) {
		this->setSpecifyAudioPosition(bValue);
	}
	if (getJsInt32(cx, rparams, "audioPositionVariableObjectId", &iValue)) {
		this->setAudioPositionVariableObjectId(iValue);
	}
	if (getJsInt32(cx, rparams, "audioPositionVariableQualifierId", &iValue)) {
		this->setAudioPositionVariableQualifierId((EnumQualifierType)iValue);
	}
	if (getJsInt32(cx, rparams, "audioPositionVariableId", &iValue)) {
		this->setAudioPositionVariableId(iValue);
	}

	return true;
}

#if defined(AGTK_DEBUG)
void ObjectCommandSoundPlayData::dump()
{
	ObjectCommandData::dump();
	CCLOG("soundType:%d", this->getSoundType());
	CCLOG("seId:%d", this->getSeId());
	CCLOG("voiceId:%d", this->getVoiceId());
	CCLOG("bgmId:%d", this->getBgmId());
	CCLOG("loop:%d", this->getLoop());
	CCLOG("fadein:%d", this->getFadein());
	CCLOG("duration300:%d", this->getDuration300());
	CCLOG("volume:%d", this->getVolume());
	CCLOG("pan:%d", this->getPan());
	CCLOG("pitch:%d", this->getPitch());
	CCLOG("specifyAudioPosition:%d", this->getSpecifyAudioPosition());
	CCLOG("audioPositionVariableObjectId:%d", this->getAudioPositionVariableObjectId());
	CCLOG("audioPositionVariableQualifierId:%d", this->getAudioPositionVariableQualifierId());
	CCLOG("audioPositionVariableId:%d", this->getAudioPositionVariableId());
}
#endif

//-------------------------------------------------------------------------------------------------------------------
bool ObjectCommandMessageShowData::init(const rapidjson::Value& json)
{
	if (!ObjectCommandData::init(json)) {
		return false;
	}
	CC_ASSERT(json.HasMember("messageShow"));
	const rapidjson::Value& cmd = json["messageShow"];
	this->setTextFlag(cmd["textFlag"].GetBool());
	this->setTextId(cmd["textId"].GetInt());
	this->setVariableObjectId(cmd["variableObjectId"].GetInt());
	this->setVariableQualifierId(cmd["variableQualifierId"].GetInt());
	this->setVariableId(cmd["variableId"].GetInt());
	this->setFontId(cmd["fontId"].GetInt());
	this->setDigitFlag(cmd["digitFlag"].GetBool());
	this->setDigits(cmd["digits"].GetInt());
	this->setZeroPadding(cmd["zeroPadding"].GetBool());
	this->setComma(cmd["comma"].GetBool());
	if (cmd.HasMember("withoutDecimalPlaces")) {
		this->setWithoutDecimalPlaces(cmd["withoutDecimalPlaces"].GetBool());
	}
	this->setColorA(cmd["colorA"].GetInt());
	this->setColorR(cmd["colorR"].GetInt());
	this->setColorG(cmd["colorG"].GetInt());
	this->setColorB(cmd["colorB"].GetInt());
	this->setWindowType((EnumWindow)cmd["windowType"].GetInt());
	this->setTemplateId((EnumWindowTemplate)cmd["templateId"].GetInt());
	this->setImageId(cmd["imageId"].GetInt());
	this->setWindowTransparency(cmd["windowTransparency"].GetInt());
	this->setWindowWidth(cmd["windowWidth"].GetInt());
	this->setWindowHeight(cmd["windowHeight"].GetInt());
	this->setPositionType((EnumPosition)cmd["positionType"].GetInt());
	this->setUseConnect(cmd["useConnect"].GetBool());
	this->setConnectId(cmd["connectId"].GetInt());
	this->setAdjustX(cmd["adjustX"].GetInt());
	this->setAdjustY(cmd["adjustY"].GetInt());
	if (cmd.HasMember("topBottomMargin")) {
		this->setTopBottomMargin(cmd["topBottomMargin"].GetInt());
	}
	if (cmd.HasMember("leftRightMargin")) {
		this->setLeftRightMargin(cmd["leftRightMargin"].GetInt());
	}
	this->setHorzAlign((EnumHorzAlign)cmd["horzAlign"].GetInt());
	this->setVertAlign((EnumVertAlign)cmd["vertAlign"].GetInt());
	this->setDuration300(cmd["duration300"].GetInt());
	this->setDurationUnlimited(cmd["durationUnlimited"].GetBool());
	this->setActionChangeHide(cmd["actionChangeHide"].GetBool());
	this->setCloseByKey(cmd["closeByKey"].GetBool());
	this->setKeyId(cmd["keyId"].GetInt());
	this->setObjectStop(cmd["objectStop"].GetBool());
	this->setGameStop(cmd["gameStop"].GetBool());
	if (cmd.HasMember("priority")) {
		this->setPriority(cmd["priority"].GetBool());
	}
	if (cmd.HasMember("priorityType")) {
		this->setPriorityType(cmd["priorityType"].GetInt());
	}
	else {
		this->setPriority(cmd["priorityMostFront"].GetBool());
		this->setPriorityType(kPriorityMostFront);
	}
	
	return true;
}

bool ObjectCommandMessageShowData::init(void *jsCx, void *jsObj)
{
	_commandType = kMessageShow;
	//デフォルト値設定。
	this->setTextFlag(true);
	this->setTextId(-1);
	this->setVariableObjectId(-1);
	this->setVariableQualifierId(kQualifierSingle);
	this->setVariableId(-1);
	this->setFontId(-1);
	this->setDigitFlag(false);
	this->setDigits(0);
	this->setZeroPadding(false);
	this->setComma(false);
	this->setWithoutDecimalPlaces(false);
	this->setDuration300(300);
	this->setDurationUnlimited(false);
	this->setColorA(255);
	this->setColorR(255);
	this->setColorG(255);
	this->setColorB(255);
	this->setWindowWidth(256);
	this->setWindowHeight(256);
	this->setWindowType(kWindowTemplate);
	this->setTemplateId(kWindowTemplateWhiteFrame);
	this->setImageId(-1);
	this->setWindowTransparency(0);
	this->setPositionType(kPositionCenter);
	this->setUseConnect(false);
	this->setConnectId(-1);
	this->setAdjustX(0);
	this->setAdjustY(0);
	this->setTopBottomMargin(10);
	this->setLeftRightMargin(10);
	this->setHorzAlign(kHorzAlignLeft);
	this->setVertAlign(kVertAlignTop);
	this->setActionChangeHide(false);
	this->setCloseByKey(false);
	this->setKeyId(-1);
	this->setObjectStop(false);
	this->setGameStop(false);
	this->setPriority(false);
	this->setPriorityType(kPriorityMostFront);

	auto cx = (JSContext *)jsCx;
	JS::RootedValue v(cx);
	JS::RootedObject rparams(cx, (JSObject *)jsObj);
	bool bValue;
	int iValue;
	if (getJsBoolean(cx, rparams, "textFlag", &bValue)) {
		this->setTextFlag(bValue);
	}
	if (getJsInt32(cx, rparams, "textId", &iValue)) {
		this->setTextId(iValue);
	}
	if (getJsInt32(cx, rparams, "variableObjectId", &iValue)) {
		this->setVariableObjectId(iValue);
	}
	if (getJsInt32(cx, rparams, "variableQualifierId", &iValue)) {
		this->setVariableQualifierId(iValue);
	}
	if (getJsInt32(cx, rparams, "variableId", &iValue)) {
		this->setVariableId(iValue);
	}
	if (getJsInt32(cx, rparams, "fontId", &iValue)) {
		this->setFontId(iValue);
	}
	if (getJsBoolean(cx, rparams, "digitFlag", &bValue)) {
		this->setDigitFlag(bValue);
	}
	if (getJsInt32(cx, rparams, "digits", &iValue)) {
		this->setDigits(iValue);
	}
	if (getJsBoolean(cx, rparams, "zeroPadding", &bValue)) {
		this->setZeroPadding(bValue);
	}
	if (getJsBoolean(cx, rparams, "comma", &bValue)) {
		this->setComma(bValue);
	}
	if (getJsBoolean(cx, rparams, "withoutDecimalPlaces", &bValue)) {
		this->setWithoutDecimalPlaces(bValue);
	}
	if (getJsInt32(cx, rparams, "duration300", &iValue)) {
		this->setDuration300(iValue);
	}
	if (getJsBoolean(cx, rparams, "durationUnlimited", &bValue)) {
		this->setDurationUnlimited(bValue);
	}
	if (getJsInt32(cx, rparams, "colorA", &iValue)) {
		this->setColorA(iValue);
	}
	if (getJsInt32(cx, rparams, "colorR", &iValue)) {
		this->setColorR(iValue);
	}
	if (getJsInt32(cx, rparams, "colorG", &iValue)) {
		this->setColorG(iValue);
	}
	if (getJsInt32(cx, rparams, "colorB", &iValue)) {
		this->setColorB(iValue);
	}
	if (getJsInt32(cx, rparams, "windowWidth", &iValue)) {
		this->setWindowWidth(iValue);
	}
	if (getJsInt32(cx, rparams, "windowHeight", &iValue)) {
		this->setWindowHeight(iValue);
	}
	if (getJsInt32(cx, rparams, "windowType", &iValue)) {
		this->setWindowType((EnumWindow)iValue);
	}
	if (getJsInt32(cx, rparams, "templateId", &iValue)) {
		this->setTemplateId((EnumWindowTemplate)iValue);
	}
	if (getJsInt32(cx, rparams, "imageId", &iValue)) {
		this->setImageId(iValue);
	}
	if (getJsInt32(cx, rparams, "windowTransparency", &iValue)) {
		this->setWindowTransparency(iValue);
	}
	if (getJsInt32(cx, rparams, "positionType", &iValue)) {
		this->setPositionType((EnumPosition)iValue);
	}
	if (getJsBoolean(cx, rparams, "useConnect", &bValue)) {
		this->setUseConnect(bValue);
	}
	if (getJsInt32(cx, rparams, "connectId", &iValue)) {
		this->setConnectId(iValue);
	}
	if (getJsInt32(cx, rparams, "adjustX", &iValue)) {
		this->setAdjustX(iValue);
	}
	if (getJsInt32(cx, rparams, "adjustY", &iValue)) {
		this->setAdjustY(iValue);
	}
	if (getJsInt32(cx, rparams, "topBottomMargin", &iValue)) {
		this->setTopBottomMargin(iValue);
	}
	if (getJsInt32(cx, rparams, "leftRightMargin", &iValue)) {
		this->setLeftRightMargin(iValue);
	}
	if (getJsInt32(cx, rparams, "horzAlign", &iValue)) {
		this->setHorzAlign((EnumHorzAlign)iValue);
	}
	if (getJsInt32(cx, rparams, "vertAlign", &iValue)) {
		this->setVertAlign((EnumVertAlign)iValue);
	}
	if (getJsBoolean(cx, rparams, "actionChangeHide", &bValue)) {
		this->setActionChangeHide(bValue);
	}
	if (getJsBoolean(cx, rparams, "closeByKey", &bValue)) {
		this->setCloseByKey(bValue);
	}
	if (getJsInt32(cx, rparams, "keyId", &iValue)) {
		this->setKeyId(iValue);
	}
	if (getJsBoolean(cx, rparams, "objectStop", &bValue)) {
		this->setObjectStop(bValue);
	}
	if (getJsBoolean(cx, rparams, "gameStop", &bValue)) {
		this->setGameStop(bValue);
	}
	if (getJsBoolean(cx, rparams, "priority", &bValue)) {
		this->setPriority(bValue);
	}
	if (getJsInt32(cx, rparams, "priorityType", &iValue)) {
		this->setPriorityType(iValue);
	}
	else {
		if (getJsBoolean(cx, rparams, "priorityMostFront", &bValue)) {
			this->setPriority(bValue);
			this->setPriorityType(kPriorityMostFront);
		}
	}

	return true;
}

#if defined(AGTK_DEBUG)
void ObjectCommandMessageShowData::dump()
{
	ObjectCommandData::dump();
	CCLOG("textFlag:%d", this->getTextFlag());
	CCLOG("textId:%d", this->getTextId());
	CCLOG("variableObjectId:%d", this->getVariableObjectId());
	CCLOG("variableQualifierId:%d", this->getVariableQualifierId());
	CCLOG("variableId:%d", this->getVariableId());
	CCLOG("fontId:%d", this->getFontId());
	CCLOG("digitFlag:%d", this->getDigitFlag());
	CCLOG("digits:%d", this->getDigits());
	CCLOG("zeroPadding:%d", this->getZeroPadding());
	CCLOG("comma:%d", this->getComma());
	CCLOG("withoutDecimalPlaces:%d", this->getWithoutDecimalPlaces());
	CCLOG("colorA:%d", this->getColorA());
	CCLOG("colorR:%d", this->getColorR());
	CCLOG("colorG:%d", this->getColorG());
	CCLOG("colorB:%d", this->getColorB());
	CCLOG("windowType:%d", this->getWindowType());
	CCLOG("templateId:%d", this->getTemplateId());
	CCLOG("imageId:%d", this->getImageId());
	CCLOG("windowTransparency:%d", this->getWindowTransparency());
	CCLOG("windowWidth:%d", this->getWindowWidth());
	CCLOG("windowHeight:%d", this->getWindowHeight());
	CCLOG("positionType:%d", this->getPositionType());
	CCLOG("useConnect:%d", this->getUseConnect());
	CCLOG("connectId:%d", this->getConnectId());
	CCLOG("adjustX:%d", this->getAdjustX());
	CCLOG("adjustY:%d", this->getAdjustY());
	CCLOG("topBottomMargin:%d", this->getTopBottomMargin());
	CCLOG("leftRightMargin:%d", this->getLeftRightMargin());
	CCLOG("horzAlign:%d", this->getHorzAlign());
	CCLOG("vertAlign:%d", this->getVertAlign());
	CCLOG("duration300:%d", this->getDuration300());
	CCLOG("durationUnlimited:%d", this->getDurationUnlimited());
	CCLOG("actionChangeHide:%d", this->getActionChangeHide());
	CCLOG("closeByKey:%d", this->getCloseByKey());
	CCLOG("keyId:%d", this->getKeyId());
	CCLOG("objectStop:%d", this->getObjectStop());
	CCLOG("gameStop:%d", this->getGameStop());
	CCLOG("priority:%d", this->getPriority());
	CCLOG("priorityType:%d", this->getPriorityType());
}
#endif

//-------------------------------------------------------------------------------------------------------------------
bool ObjectCommandScrollMessageShowData::init(const rapidjson::Value& json)
{
	if (!ObjectCommandData::init(json)) {
		return false;
	}
	CC_ASSERT(json.HasMember("scrollMessageShow"));
	const rapidjson::Value& cmd = json["scrollMessageShow"];
	this->setTextId(cmd["textId"].GetInt());
	this->setColorA(cmd["colorA"].GetInt());
	this->setColorR(cmd["colorR"].GetInt());
	this->setColorG(cmd["colorG"].GetInt());
	this->setColorB(cmd["colorB"].GetInt());
	this->setBackgroundType((EnumBackground)cmd["backgroundType"].GetInt());
	this->setTemplateId((EnumBackgroundTemplate)cmd["templateId"].GetInt());
	this->setImageId(cmd["imageId"].GetInt());
	this->setBackgroundTransparency(cmd["backgroundTransparency"].GetInt());
	this->setBackgroundWidth(cmd["backgroundWidth"].GetInt());
	this->setBackgroundHeight(cmd["backgroundHeight"].GetInt());
	this->setPositionType((EnumPosition)cmd["positionType"].GetInt());
	this->setUseConnect(cmd["useConnect"].GetBool());
	this->setConnectId(cmd["connectId"].GetInt());
	this->setAdjustX(cmd["adjustX"].GetInt());
	this->setAdjustY(cmd["adjustY"].GetInt());
	if (cmd.HasMember("topBottomMargin")) {
		this->setTopBottomMargin(cmd["topBottomMargin"].GetInt());
	}
	if (cmd.HasMember("leftRightMargin")) {
		this->setLeftRightMargin(cmd["leftRightMargin"].GetInt());
	}
	this->setHorzAlign((EnumHorzAlign)cmd["horzAlign"].GetInt());
	this->setScrollSpeed(cmd["scrollSpeed"].GetDouble());
	this->setScrollUp(cmd["scrollUp"].GetBool());
	this->setActionChangeHide(cmd["actionChangeHide"].GetBool());
	this->setSpeedUpByKey(cmd["speedUpByKey"].GetBool());
	this->setKeyId(cmd["keyId"].GetInt());
	this->setObjectStop(cmd["objectStop"].GetBool());
	this->setGameStop(cmd["gameStop"].GetBool());
	if (cmd.HasMember("priority")) {
		this->setPriority(cmd["priority"].GetBool());
	}
	if (cmd.HasMember("priorityType")) {
		this->setPriorityType(cmd["priorityType"].GetInt());
	}
	else {
		this->setPriority(cmd["priorityMostFront"].GetBool());
		this->setPriorityType(kPriorityMostFront);
	}
	return true;
}

bool ObjectCommandScrollMessageShowData::init(void *jsCx, void *jsObj)
{
	_commandType = kScrollMessageShow;
	//デフォルト値設定。
	this->setTextId(-1);
	this->setScrollSpeed(1.0);
	this->setScrollUp(true);
	this->setColorA(255);
	this->setColorR(255);
	this->setColorG(255);
	this->setColorB(255);
	this->setBackgroundWidth(256);
	this->setBackgroundHeight(256);
	this->setBackgroundType(kBackgroundTemplate);
	this->setTemplateId(kBackgroundTemplateBlack);
	this->setImageId(-1);
	this->setBackgroundTransparency(0);
	this->setTopBottomMargin(10);
	this->setLeftRightMargin(10);
	this->setHorzAlign(kHorzAlignLeft);
	this->setPositionType(kPositionCenter);
	this->setUseConnect(false);
	this->setConnectId(-1);
	this->setAdjustX(0);
	this->setAdjustY(0);
	this->setActionChangeHide(false);
	this->setSpeedUpByKey(false);
	this->setKeyId(-1);
	this->setObjectStop(false);
	this->setGameStop(false);
	this->setPriority(false);
	this->setPriorityType(kPriorityMostFront);

	auto cx = (JSContext *)jsCx;
	JS::RootedValue v(cx);
	JS::RootedObject rparams(cx, (JSObject *)jsObj);
	bool bValue;
	int iValue;
	double dValue;
	if (getJsInt32(cx, rparams, "textId", &iValue)) {
		this->setTextId(iValue);
	}
	if (getJsDouble(cx, rparams, "scrollSpeed", &dValue)) {
		this->setScrollSpeed(dValue);
	}
	if (getJsBoolean(cx, rparams, "scrollUp", &bValue)) {
		this->setScrollUp(bValue);
	}
	if (getJsInt32(cx, rparams, "colorA", &iValue)) {
		this->setColorA(iValue);
	}
	if (getJsInt32(cx, rparams, "colorR", &iValue)) {
		this->setColorR(iValue);
	}
	if (getJsInt32(cx, rparams, "colorG", &iValue)) {
		this->setColorG(iValue);
	}
	if (getJsInt32(cx, rparams, "colorB", &iValue)) {
		this->setColorB(iValue);
	}
	if (getJsInt32(cx, rparams, "backgroundWidth", &iValue)) {
		this->setBackgroundWidth(iValue);
	}
	if (getJsInt32(cx, rparams, "backgroundHeight", &iValue)) {
		this->setBackgroundHeight(iValue);
	}
	if (getJsInt32(cx, rparams, "backgroundType", &iValue)) {
		this->setBackgroundType((EnumBackground)iValue);
	}
	if (getJsInt32(cx, rparams, "templateId", &iValue)) {
		this->setTemplateId((EnumBackgroundTemplate)iValue);
	}
	if (getJsInt32(cx, rparams, "imageId", &iValue)) {
		this->setImageId(iValue);
	}
	if (getJsInt32(cx, rparams, "backgroundTransparency", &iValue)) {
		this->setBackgroundTransparency(iValue);
	}
	if (getJsInt32(cx, rparams, "topBottomMargin", &iValue)) {
		this->setTopBottomMargin(iValue);
	}
	if (getJsInt32(cx, rparams, "leftRightMargin", &iValue)) {
		this->setLeftRightMargin(iValue);
	}
	if (getJsInt32(cx, rparams, "horzAlign", &iValue)) {
		this->setHorzAlign((EnumHorzAlign)iValue);
	}
	if (getJsInt32(cx, rparams, "positionType", &iValue)) {
		this->setPositionType((EnumPosition)iValue);
	}
	if (getJsBoolean(cx, rparams, "useConnect", &bValue)) {
		this->setUseConnect(bValue);
	}
	if (getJsInt32(cx, rparams, "connectId", &iValue)) {
		this->setConnectId(iValue);
	}
	if (getJsInt32(cx, rparams, "adjustX", &iValue)) {
		this->setAdjustX(iValue);
	}
	if (getJsInt32(cx, rparams, "adjustY", &iValue)) {
		this->setAdjustY(iValue);
	}
	if (getJsBoolean(cx, rparams, "actionChangeHide", &bValue)) {
		this->setActionChangeHide(bValue);
	}
	if (getJsBoolean(cx, rparams, "speedUpByKey", &bValue)) {
		this->setSpeedUpByKey(bValue);
	}
	if (getJsInt32(cx, rparams, "keyId", &iValue)) {
		this->setKeyId(iValue);
	}
	if (getJsBoolean(cx, rparams, "objectStop", &bValue)) {
		this->setObjectStop(bValue);
	}
	if (getJsBoolean(cx, rparams, "gameStop", &bValue)) {
		this->setGameStop(bValue);
	}
	if (getJsBoolean(cx, rparams, "priority", &bValue)) {
		this->setPriority(bValue);
	}
	if (getJsInt32(cx, rparams, "priorityType", &iValue)) {
		this->setPriorityType(iValue);
	}
	else {
		if (getJsBoolean(cx, rparams, "priorityMostFront", &bValue)) {
			this->setPriority(bValue);
			this->setPriorityType(kPriorityMostFront);
		}
	}

	return true;
}

#if defined(AGTK_DEBUG)
void ObjectCommandScrollMessageShowData::dump()
{
	ObjectCommandData::dump();
	CCLOG("textId:%d", this->getTextId());
	CCLOG("colorA:%d", this->getColorA());
	CCLOG("colorR:%d", this->getColorR());
	CCLOG("colorG:%d", this->getColorG());
	CCLOG("colorB:%d", this->getColorB());
	CCLOG("backgroundType:%d", this->getBackgroundType());
	CCLOG("templateId:%d", this->getTemplateId());
	CCLOG("imageId:%d", this->getImageId());
	CCLOG("backgroundTransparency:%d", this->getBackgroundTransparency());
	CCLOG("backgroundWidth:%d", this->getBackgroundWidth());
	CCLOG("backgroundHeight:%d", this->getBackgroundHeight());
	CCLOG("positionType:%d", this->getPositionType());
	CCLOG("useConnect:%d", this->getUseConnect());
	CCLOG("connectId:%d", this->getConnectId());
	CCLOG("adjustX:%d", this->getAdjustX());
	CCLOG("adjustY:%d", this->getAdjustY());
	CCLOG("horzAlign:%d", this->getHorzAlign());
	CCLOG("topBottomMargin:%d", this->getTopBottomMargin());
	CCLOG("leftRightMargin:%d", this->getLeftRightMargin());
	CCLOG("scrollSpeed:%f", this->getScrollSpeed());
	CCLOG("scrollUp:%d", this->getScrollUp());
	CCLOG("actionChangeHide:%d", this->getActionChangeHide());
	CCLOG("speedUpByKey:%d", this->getSpeedUpByKey());
	CCLOG("keyId:%d", this->getKeyId());
	CCLOG("objectStop:%d", this->getObjectStop());
	CCLOG("gameStop:%d", this->getGameStop());
	CCLOG("priority:%d", this->getPriority());
	CCLOG("priorityType:%d", this->getPriorityType());
}
#endif

//-------------------------------------------------------------------------------------------------------------------
bool ObjectCommandEffectShowData::init(const rapidjson::Value& json)
{
	if (!ObjectCommandData::init(json)) {
		return false;
	}
	CC_ASSERT(json.HasMember("effectShow"));
	const rapidjson::Value& cmd = json["effectShow"];
	this->setEffectId(cmd["effectId"].GetInt());
	this->setPositionType((EnumPosition)cmd["positionType"].GetInt());
	this->setUseConnect(cmd["useConnect"].GetBool());
	this->setConnectId(cmd["connectId"].GetInt());
	this->setAdjustX(cmd["adjustX"].GetInt());
	this->setAdjustY(cmd["adjustY"].GetInt());
	this->setDuration300(cmd["duration300"].GetInt());
	this->setDurationUnlimited(cmd["durationUnlimited"].GetBool());
	return true;
}

bool ObjectCommandEffectShowData::init(void *jsCx, void *jsObj)
{
	_commandType = kEffectShow;
	//デフォルト値設定。
	this->setEffectId(-1);
	this->setPositionType(kPositionCenter);
	this->setUseConnect(false);
	this->setConnectId(-1);
	this->setAdjustX(0);
	this->setAdjustY(0);
	this->setDuration300(300);
	this->setDurationUnlimited(false);

	auto cx = (JSContext *)jsCx;
	JS::RootedValue v(cx);
	JS::RootedObject rparams(cx, (JSObject *)jsObj);
	bool bValue;
	int iValue;
	if (getJsInt32(cx, rparams, "effectId", &iValue)) {
		this->setEffectId(iValue);
	}
	if (getJsInt32(cx, rparams, "positionType", &iValue)) {
		this->setPositionType((EnumPosition)iValue);
	}
	if (getJsBoolean(cx, rparams, "useConnect", &bValue)) {
		this->setUseConnect(bValue);
	}
	if (getJsInt32(cx, rparams, "connectId", &iValue)) {
		this->setConnectId(iValue);
	}
	if (getJsInt32(cx, rparams, "adjustX", &iValue)) {
		this->setAdjustX(iValue);
	}
	if (getJsInt32(cx, rparams, "adjustY", &iValue)) {
		this->setAdjustY(iValue);
	}
	if (getJsInt32(cx, rparams, "duration300", &iValue)) {
		this->setDuration300(iValue);
	}
	if (getJsBoolean(cx, rparams, "durationUnlimited", &bValue)) {
		this->setDurationUnlimited(bValue);
	}

	return true;
}

#if defined(AGTK_DEBUG)
void ObjectCommandEffectShowData::dump()
{
	ObjectCommandData::dump();
	CCLOG("effectId:%d", this->getEffectId());
	CCLOG("positionType:%d", this->getPositionType());
	CCLOG("useConnect:%d", this->getUseConnect());
	CCLOG("connectId:%d", this->getConnectId());
	CCLOG("adjustX:%d", this->getAdjustX());
	CCLOG("adjustY:%d", this->getAdjustY());
	CCLOG("duration300:%d", this->getDuration300());
	CCLOG("durationUnlimited:%d", this->getDurationUnlimited());
}
#endif

//-------------------------------------------------------------------------------------------------------------------
bool ObjectCommandMovieShowData::init(const rapidjson::Value& json)
{
	if (!ObjectCommandData::init(json)) {
		return false;
	}
	CC_ASSERT(json.HasMember("movieShow"));
	const rapidjson::Value& cmd = json["movieShow"];
	this->setMovieId(cmd["movieId"].GetInt());
	this->setLoop(cmd["loop"].GetBool());
	this->setVolume(cmd["volume"].GetInt());
	this->setDefaultSize(cmd["defaultSize"].GetBool());
	this->setWidth(cmd["width"].GetInt());
	this->setHeight(cmd["height"].GetInt());
	this->setPositionType((EnumPosition)cmd["positionType"].GetInt());
	this->setUseConnect(cmd["useConnect"].GetBool());
	this->setConnectId(cmd["connectId"].GetInt());
	this->setVertAlign((EnumVertAlign)cmd["vertAlign"].GetInt());
	this->setHorzAlign((EnumHorzAlign)cmd["horzAlign"].GetInt());
	this->setAdjustX(cmd["adjustX"].GetInt());
	this->setAdjustY(cmd["adjustY"].GetInt());
	if (cmd.HasMember("hideOnObjectActionChange")) {
		this->setHideOnObjectActionChange(cmd["hideOnObjectActionChange"].GetBool());
	}
	this->setStopObject(cmd["stopObject"].GetBool());
	this->setStopGame(cmd["stopGame"].GetBool());
	this->setFillBlack(cmd["fillBlack"].GetBool());
	this->setPriority(cmd["priority"].GetBool());
	if (cmd.HasMember("priorityType")) {
		this->setPriorityType(cmd["priorityType"].GetInt());
	}
	else {
		this->setPriorityType(cmd["priorityMostFront"].GetBool() ? kPriorityMostFront : kPriorityBackground);
	}
	return true;
}

bool ObjectCommandMovieShowData::init(void *jsCx, void *jsObj)
{
	_commandType = kMovieShow;
	//デフォルト値設定。
	this->setMovieId(-1);
	this->setLoop(false);
	this->setVolume(100);
	this->setDefaultSize(true);
	this->setWidth(256);
	this->setHeight(256);
	this->setPositionType(kPositionCenter);
	this->setUseConnect(false);
	this->setConnectId(-1);
	this->setVertAlign(kVertAlignCenter);
	this->setHorzAlign(kHorzAlignCenter);
	this->setAdjustX(0);
	this->setAdjustY(0);
	this->setHideOnObjectActionChange(false);
	this->setStopObject(false);
	this->setStopGame(false);
	this->setFillBlack(false);
	this->setPriority(false);
	this->setPriorityType(kPriorityMostFront);

	auto cx = (JSContext *)jsCx;
	JS::RootedValue v(cx);
	JS::RootedObject rparams(cx, (JSObject *)jsObj);
	bool bValue;
	int iValue;
	if (getJsInt32(cx, rparams, "movieId", &iValue)) {
		this->setMovieId(iValue);
	}
	if (getJsBoolean(cx, rparams, "loop", &bValue)) {
		this->setLoop(bValue);
	}
	if (getJsInt32(cx, rparams, "volume", &iValue)) {
		this->setVolume(iValue);
	}
	if (getJsBoolean(cx, rparams, "defaultSize", &bValue)) {
		this->setDefaultSize(bValue);
	}
	if (getJsInt32(cx, rparams, "width", &iValue)) {
		this->setWidth(iValue);
	}
	if (getJsInt32(cx, rparams, "height", &iValue)) {
		this->setHeight(iValue);
	}
	if (getJsInt32(cx, rparams, "positionType", &iValue)) {
		this->setPositionType((EnumPosition)iValue);
	}
	if (getJsBoolean(cx, rparams, "useConnect", &bValue)) {
		this->setUseConnect(bValue);
	}
	if (getJsInt32(cx, rparams, "connectId", &iValue)) {
		this->setConnectId(iValue);
	}
	if (getJsInt32(cx, rparams, "vertAlign", &iValue)) {
		this->setVertAlign((EnumVertAlign)iValue);
	}
	if (getJsInt32(cx, rparams, "horzAlign", &iValue)) {
		this->setHorzAlign((EnumHorzAlign)iValue);
	}
	if (getJsInt32(cx, rparams, "adjustX", &iValue)) {
		this->setAdjustX(iValue);
	}
	if (getJsInt32(cx, rparams, "adjustY", &iValue)) {
		this->setAdjustY(iValue);
	}
	if (getJsBoolean(cx, rparams, "hideOnObjectActionChange", &bValue)) {
		this->setHideOnObjectActionChange(bValue);
	}
	if (getJsBoolean(cx, rparams, "stopObject", &bValue)) {
		this->setStopObject(bValue);
	}
	if (getJsBoolean(cx, rparams, "stopGame", &bValue)) {
		this->setStopGame(bValue);
	}
	if (getJsBoolean(cx, rparams, "fillBlack", &bValue)) {
		this->setFillBlack(bValue);
	}
	if (getJsBoolean(cx, rparams, "priority", &bValue)) {
		this->setPriority(bValue);
	}
	if (getJsInt32(cx, rparams, "priorityType", &iValue)) {
		this->setPriorityType(iValue);
	}
	else {
		if (getJsBoolean(cx, rparams, "priorityMostFront", &bValue)) {
			this->setPriorityType(bValue ? kPriorityMostFront : kPriorityBackground);
		}
	}

	return true;
}

#if defined(AGTK_DEBUG)
void ObjectCommandMovieShowData::dump()
{
	ObjectCommandData::dump();
	CCLOG("movieId:%d", this->getMovieId());
	CCLOG("loop:%d", this->getLoop());
	CCLOG("volume:%d", this->getVolume());
	CCLOG("defaultSize:%d", this->getDefaultSize());
	CCLOG("width:%d", this->getWidth());
	CCLOG("height:%d", this->getHeight());
	CCLOG("positionType:%d", this->getPositionType());
	CCLOG("useConnect:%d", this->getUseConnect());
	CCLOG("connectId:%d", this->getConnectId());
	CCLOG("vertAlign:%d", this->getVertAlign());
	CCLOG("horzAlign:%d", this->getHorzAlign());
	CCLOG("adjustX:%d", this->getAdjustX());
	CCLOG("adjustY:%d", this->getAdjustY());
	CCLOG("hideOnObjectActionChange:%d", this->getHideOnObjectActionChange());
	CCLOG("stopObject:%d", this->getStopObject());
	CCLOG("stopGame:%d", this->getStopGame());
	CCLOG("fillBlack:%d", this->getFillBlack());
	CCLOG("priority:%d", this->getPriority());
	CCLOG("priorityType:%d", this->getPriorityType());
}
#endif

//-------------------------------------------------------------------------------------------------------------------
bool ObjectCommandImageShowData::init(const rapidjson::Value& json)
{
	if (!ObjectCommandData::init(json)) {
		return false;
	}
	CC_ASSERT(json.HasMember("imageShow"));
	const rapidjson::Value& cmd = json["imageShow"];
	this->setImageId(cmd["imageId"].GetInt());
	this->setDefaultSize(cmd["defaultSize"].GetBool());
	this->setWidth(cmd["width"].GetInt());
	this->setHeight(cmd["height"].GetInt());
	this->setPositionType((EnumPosition)cmd["positionType"].GetInt());
	this->setUseConnect(cmd["useConnect"].GetBool());
	this->setConnectId(cmd["connectId"].GetInt());
	if (cmd.HasMember("vertAlign")) {
		this->setVertAlign((EnumVertAlign)cmd["vertAlign"].GetInt());
	}
	if (cmd.HasMember("horzAlign")) {
		this->setHorzAlign((EnumHorzAlign)cmd["horzAlign"].GetInt());
	}
	this->setAdjustX(cmd["adjustX"].GetInt());
	this->setAdjustY(cmd["adjustY"].GetInt());
	this->setDuration300(cmd["duration300"].GetInt());
	this->setDurationUnlimited(cmd["durationUnlimited"].GetBool());
	if (cmd.HasMember("hideOnObjectActionChange")) {
		this->setHideOnObjectActionChange(cmd["hideOnObjectActionChange"].GetBool());
	}
	this->setCloseByOk(cmd["closeByOk"].GetBool());
	this->setStopObject(cmd["stopObject"].GetBool());
	this->setStopGame(cmd["stopGame"].GetBool());
	this->setPriority(cmd["priority"].GetBool());
	if (cmd.HasMember("priorityType")) {
		this->setPriorityType(cmd["priorityType"].GetInt());
	}
	else {
		this->setPriorityType(cmd["priorityMostFront"].GetBool() ? kPriorityMostFront : kPriorityBackground);
	}
	return true;
}

bool ObjectCommandImageShowData::init(void *jsCx, void *jsObj)
{
	_commandType = kImageShow;
	//デフォルト値設定。
	this->setImageId(-1);
	this->setDuration300(300);
	this->setDurationUnlimited(false);
	this->setDefaultSize(true);
	this->setWidth(256);
	this->setHeight(256);
	this->setPositionType(kPositionCenter);
	this->setUseConnect(false);
	this->setConnectId(-1);
	this->setVertAlign(kVertAlignCenter);
	this->setHorzAlign(kHorzAlignCenter);
	this->setAdjustX(0);
	this->setAdjustY(0);
	this->setHideOnObjectActionChange(false);
	this->setCloseByOk(false);
	this->setStopObject(false);
	this->setStopGame(false);
	this->setPriority(false);
	this->setPriorityType(kPriorityMostFront);

	auto cx = (JSContext *)jsCx;
	JS::RootedValue v(cx);
	JS::RootedObject rparams(cx, (JSObject *)jsObj);
	bool bValue;
	int iValue;
	if (getJsInt32(cx, rparams, "imageId", &iValue)) {
		this->setImageId(iValue);
	}
	if (getJsInt32(cx, rparams, "duration300", &iValue)) {
		this->setDuration300(iValue);
	}
	if (getJsBoolean(cx, rparams, "durationUnlimited", &bValue)) {
		this->setDurationUnlimited(bValue);
	}
	if (getJsBoolean(cx, rparams, "defaultSize", &bValue)) {
		this->setDefaultSize(bValue);
	}
	if (getJsInt32(cx, rparams, "width", &iValue)) {
		this->setWidth(iValue);
	}
	if (getJsInt32(cx, rparams, "height", &iValue)) {
		this->setHeight(iValue);
	}
	if (getJsInt32(cx, rparams, "positionType", &iValue)) {
		this->setPositionType((EnumPosition)iValue);
	}
	if (getJsBoolean(cx, rparams, "useConnect", &bValue)) {
		this->setUseConnect(bValue);
	}
	if (getJsInt32(cx, rparams, "connectId", &iValue)) {
		this->setConnectId(iValue);
	}
	if (getJsInt32(cx, rparams, "vertAlign", &iValue)) {
		this->setVertAlign((EnumVertAlign)iValue);
	}
	if (getJsInt32(cx, rparams, "horzAlign", &iValue)) {
		this->setHorzAlign((EnumHorzAlign)iValue);
	}
	if (getJsInt32(cx, rparams, "adjustX", &iValue)) {
		this->setAdjustX(iValue);
	}
	if (getJsInt32(cx, rparams, "adjustY", &iValue)) {
		this->setAdjustY(iValue);
	}
	if (getJsBoolean(cx, rparams, "hideOnObjectActionChange", &bValue)) {
		this->setHideOnObjectActionChange(bValue);
	}
	if (getJsBoolean(cx, rparams, "closeByOk", &bValue)) {
		this->setCloseByOk(bValue);
	}
	if (getJsBoolean(cx, rparams, "stopObject", &bValue)) {
		this->setStopObject(bValue);
	}
	if (getJsBoolean(cx, rparams, "stopGame", &bValue)) {
		this->setStopGame(bValue);
	}
	if (getJsBoolean(cx, rparams, "priority", &bValue)) {
		this->setPriority(bValue);
	}
	if (getJsInt32(cx, rparams, "priorityType", &iValue)) {
		this->setPriorityType(iValue);
	}
	else {
		if (getJsBoolean(cx, rparams, "priorityMostFront", &bValue)) {
			this->setPriorityType(bValue ? kPriorityMostFront : kPriorityBackground);
		}
	}

	return true;
}

#if defined(AGTK_DEBUG)
void ObjectCommandImageShowData::dump()
{
	ObjectCommandData::dump();
	CCLOG("imageId:%d", this->getImageId());
	CCLOG("defaultSize:%d", this->getDefaultSize());
	CCLOG("width:%d", this->getWidth());
	CCLOG("height:%d", this->getHeight());
	CCLOG("positionType:%d", this->getPositionType());
	CCLOG("useConnect:%d", this->getUseConnect());
	CCLOG("connectId:%d", this->getConnectId());
	CCLOG("vertAlign:%d", this->getVertAlign());
	CCLOG("horzAlign:%d", this->getHorzAlign());
	CCLOG("adjustX:%d", this->getAdjustX());
	CCLOG("adjustY:%d", this->getAdjustY());
	CCLOG("duration300:%d", this->getDuration300());
	CCLOG("durationUnlimited:%d", this->getDurationUnlimited());
	CCLOG("hideOnObjectActionChange:%d", this->getHideOnObjectActionChange());
	CCLOG("closeByOk:%d", this->getCloseByOk());
	CCLOG("stopObject:%d", this->getStopObject());
	CCLOG("stopGame:%d", this->getStopGame());
	CCLOG("priority:%d", this->getPriority());
	CCLOG("priorityType:%d", this->getPriorityType());
}
#endif

//-------------------------------------------------------------------------------------------------------------------
ObjectCommandSwitchVariableChangeData::ObjectCommandSwitchVariableChangeData()
{
	_assignScript = nullptr;
}

ObjectCommandSwitchVariableChangeData::~ObjectCommandSwitchVariableChangeData()
{
	CC_SAFE_RELEASE_NULL(_assignScript);
}

bool ObjectCommandSwitchVariableChangeData::init(const rapidjson::Value& json)
{
	if (!ObjectCommandData::init(json)) {
		return false;
	}
	CC_ASSERT(json.HasMember("switchVariableChange"));
	const rapidjson::Value& cmd = json["switchVariableChange"];
	this->setSwtch(cmd["swtch"].GetBool());
	this->setSwitchObjectId(cmd["switchObjectId"].GetInt());
	this->setSwitchId(cmd["switchId"].GetInt());
	this->setSwitchQualifierId((EnumQualifierType)cmd["switchQualifierId"].GetInt());
	this->setSwitchValue((EnumSwitchAssignValue)cmd["switchValue"].GetInt());
	this->setVariableObjectId(cmd["variableObjectId"].GetInt());
	this->setVariableId(cmd["variableId"].GetInt());
	this->setVariableQualifierId((EnumQualifierType)cmd["variableQualifierId"].GetInt());
	this->setVariableAssignOperator((EnumVariableAssignOperatorType)cmd["variableAssignOperator"].GetInt());
	this->setVariableAssignValueType((EnumVariableAssignValueType)cmd["variableAssignValueType"].GetInt());
	this->setAssignValue(cmd["assignValue"].GetDouble());
	this->setAssignVariableObjectId(cmd["assignVariableObjectId"].GetInt());
	this->setAssignVariableId(cmd["assignVariableId"].GetInt());
	this->setAssignVariableQualifierId((EnumQualifierType)cmd["assignVariableQualifierId"].GetInt());
	this->setRandomMin(cmd["randomMin"].GetInt());
	this->setRandomMax(cmd["randomMax"].GetInt());
	if (cmd.HasMember("useCoffeeScript") && cmd["useCoffeeScript"].GetBool()) {
		this->setAssignScript(cocos2d::__String::create(cmd["javaScript"].GetString()));
	}
	else {
		this->setAssignScript(cocos2d::__String::create(cmd["assignScript"].GetString()));
	}
	return true;
}

bool ObjectCommandSwitchVariableChangeData::init(void *jsCx, void *jsObj)
{
	_commandType = kSwitchVariableChange;
	//デフォルト値設定。
	this->setSwtch(true);
	this->setSwitchObjectId(-1);
	this->setSwitchQualifierId(kQualifierSingle);
	this->setSwitchId(-1);
	this->setSwitchValue(kSwitchAssignOn);
	this->setVariableObjectId(-1);
	this->setVariableQualifierId(kQualifierSingle);
	this->setVariableId(-1);
	this->setVariableAssignOperator(kVariableAssignOperatorSet);
	this->setVariableAssignValueType(kVariableAssignValue);
	this->setAssignValue(0);
	this->setAssignVariableObjectId(-1);
	this->setAssignVariableQualifierId(kQualifierSingle);
	this->setAssignVariableId(-1);
	this->setRandomMin(0);
	this->setRandomMax(100);
	this->setAssignScript(cocos2d::__String::create(""));

	auto cx = (JSContext *)jsCx;
	JS::RootedValue v(cx);
	JS::RootedObject rparams(cx, (JSObject *)jsObj);
	bool bValue;
	int iValue;
	double dValue;
	cocos2d::String *sValue;
	if (getJsBoolean(cx, rparams, "swtch", &bValue)) {
		this->setSwtch(bValue);
	}
	if (getJsInt32(cx, rparams, "switchQualifierId", &iValue)) {
		this->setSwitchQualifierId((EnumQualifierType)iValue);
	}
	if (getJsInt32(cx, rparams, "switchObjectId", &iValue)) {
		this->setSwitchObjectId(iValue);
	}
	if (getJsInt32(cx, rparams, "switchId", &iValue)) {
		this->setSwitchId(iValue);
	}
	if (getJsInt32(cx, rparams, "switchValue", &iValue)) {
		this->setSwitchValue((EnumSwitchAssignValue)iValue);
	}
	if (getJsInt32(cx, rparams, "variableQualifierId", &iValue)) {
		this->setVariableQualifierId((EnumQualifierType)iValue);
	}
	if (getJsInt32(cx, rparams, "variableObjectId", &iValue)) {
		this->setVariableObjectId(iValue);
	}
	if (getJsInt32(cx, rparams, "variableId", &iValue)) {
		this->setVariableId(iValue);
	}
	if (getJsInt32(cx, rparams, "variableAssignOperator", &iValue)) {
		this->setVariableAssignOperator((EnumVariableAssignOperatorType)iValue);
	}
	if (getJsInt32(cx, rparams, "variableAssignValueType", &iValue)) {
		this->setVariableAssignValueType((EnumVariableAssignValueType)iValue);
	}
	if (getJsDouble(cx, rparams, "assignValue", &dValue)) {
		this->setAssignValue(dValue);
	}
	if (getJsInt32(cx, rparams, "assignVariableObjectId", &iValue)) {
		this->setAssignVariableObjectId(iValue);
	}
	if (getJsInt32(cx, rparams, "assignVariableQualifierId", &iValue)) {
		this->setAssignVariableQualifierId((EnumQualifierType)iValue);
	}
	if (getJsInt32(cx, rparams, "assignVariableId", &iValue)) {
		this->setAssignVariableId(iValue);
	}
	if (getJsInt32(cx, rparams, "randomMin", &iValue)) {
		this->setRandomMin(iValue);
	}
	if (getJsInt32(cx, rparams, "randomMax", &iValue)) {
		this->setRandomMax(iValue);
	}
	if (getJsString(cx, rparams, "assignScript", &sValue)) {
		this->setAssignScript(sValue);
	}

	return true;
}

const char *ObjectCommandSwitchVariableChangeData::getAssignScript()
{
	CC_ASSERT(_assignScript);
	return _assignScript->getCString();
}

#if defined(AGTK_DEBUG)
void ObjectCommandSwitchVariableChangeData::dump()
{
	ObjectCommandData::dump();
	CCLOG("swtch;%d", this->getSwtch());
	CCLOG("switchObjectId:%d", this->getSwitchObjectId());
	CCLOG("switchId:%d", this->getSwitchId());
	CCLOG("switchQualifierId:%d", this->getSwitchQualifierId());
	CCLOG("switchValue:%d", this->getSwitchValue());
	CCLOG("variableId:%d", this->getVariableId());
	CCLOG("variableObjectId:%d", this->getVariableObjectId());
	CCLOG("variableQualifierId:%d", this->getVariableQualifierId());
	CCLOG("variableAssignOperator:%d", this->getVariableAssignOperator());
	CCLOG("variableAssignValueType:%d", this->getVariableAssignValueType());
	CCLOG("assignValue:%d", this->getAssignValue());
	CCLOG("assignVariableObjectId:%d", this->getAssignVariableObjectId());
	CCLOG("assignVariableId:%d", this->getAssignVariableId());
	CCLOG("assignVariableQualifierId:%d", this->getAssignVariableQualifierId());
	CCLOG("randomMin:%d", this->getRandomMin());
	CCLOG("randomMax:%d", this->getRandomMax());
	CCLOG("assignScript:%s", this->getAssignScript());
}
#endif

//-------------------------------------------------------------------------------------------------------------------
ObjectCommandSwitchVariableResetData::ObjectCommandSwitchVariableResetData() : ObjectCommandData()
{
	_switchVariableResetList = nullptr;
}

ObjectCommandSwitchVariableResetData::~ObjectCommandSwitchVariableResetData()
{
	CC_SAFE_RELEASE_NULL(_switchVariableResetList);
}

bool ObjectCommandSwitchVariableResetData::init(const rapidjson::Value& json)
{
	if (!ObjectCommandData::init(json)) {
		return false;
	}
	CC_ASSERT(json.HasMember("switchVariableReset"));
	const rapidjson::Value& cmd = json["switchVariableReset"];
	auto switchVariableResetList = cocos2d::__Array::create();
	for (rapidjson::SizeType i = 0; i < cmd.Size(); i++) {
		auto switchVariableData = SwitchVariableData::create(cmd[i]);
		switchVariableResetList->addObject(switchVariableData);
	}
	this->setSwitchVariableResetList(switchVariableResetList);
	return true;
}

bool ObjectCommandSwitchVariableResetData::init(void *jsCx, void *jsObj)
{
	_commandType = kSwitchVariableReset;
	//デフォルト値設定。
	auto switchVariableResetList = cocos2d::__Array::create();
	this->setSwitchVariableResetList(switchVariableResetList);

	auto cx = (JSContext *)jsCx;
	JS::RootedValue v(cx);
	JS::RootedObject rparams(cx, (JSObject *)jsObj);
//	bool bValue;
//	int iValue;
//	double dValue;
	if (!JS_IsArrayObject(cx, rparams)) {
		return false;
	}
	uint32_t length;
	JS_GetArrayLength(cx, rparams, &length);
	for (uint32_t i = 0; i < length; i++) {
		JS_GetElement(cx, rparams, i, &v);
		auto &params2 = v.toObject();
		auto switchVariableData = SwitchVariableData::create(cx, &params2);
		switchVariableResetList->addObject(switchVariableData);
	}
	this->setSwitchVariableResetList(switchVariableResetList);

	return true;
}

#if defined(AGTK_DEBUG)
void ObjectCommandSwitchVariableResetData::dump()
{
	ObjectCommandData::dump();
	cocos2d::Ref *ref = nullptr;
	CCARRAY_FOREACH(this->getSwitchVariableResetList(), ref) {
		auto switchVariableData = dynamic_cast<SwitchVariableData *>(ref);
		switchVariableData->dump();
	}
}
#endif

//-------------------------------------------------------------------------------------------------------------------
bool ObjectCommandGameSpeedChangeData::init(const rapidjson::Value& json)
{
	if (!ObjectCommandData::init(json)) {
		return false;
	}
	CC_ASSERT(json.HasMember("gameSpeedChange"));
	const rapidjson::Value& cmd = json["gameSpeedChange"];
	this->setGameSpeed(cmd["gameSpeed"].GetDouble());
	this->setDuration300(cmd["duration300"].GetInt());
	this->setDurationUnlimited(cmd["durationUnlimited"].GetBool());
	this->setTargetObject(cmd["targetObject"].GetBool());
	this->setTargetEffect(cmd["targetEffect"].GetBool());
	this->setTargetTile(cmd["targetTile"].GetBool());
	this->setTargetMenu(cmd["targetMenu"].GetBool());
	this->setTargettingType((EnumTargettingType)cmd["targettingType"].GetInt());
	this->setTargetObjectGroup(cmd["targetObjectGroup"].GetInt());
	this->setTargetObjectId(cmd["targetObjectId"].GetInt());
	this->setTargetQualifierId((EnumQualifierType)cmd["targetQualifierId"].GetInt());
	this->setExcludeObjectGroupBit(cmd["excludeObjectGroupBit"].GetInt());
	return true;
}

bool ObjectCommandGameSpeedChangeData::init(void *jsCx, void *jsObj)
{
	_commandType = kGameSpeedChange;
	//デフォルト値設定。
	this->setGameSpeed(100);
	this->setDuration300(300);
	this->setDurationUnlimited(false);
	this->setTargetObject(false);
	this->setTargetEffect(false);
	this->setTargetTile(false);
	this->setTargetMenu(false);
	this->setTargettingType(kTargettingByGroup);
	this->setTargetObjectGroup(kObjectGroupPlayer);
	this->setTargetObjectId(-1);
	this->setTargetQualifierId(kQualifierSingle);
	this->setExcludeObjectGroupBit(0);

	auto cx = (JSContext *)jsCx;
	JS::RootedValue v(cx);
	JS::RootedObject rparams(cx, (JSObject *)jsObj);
	bool bValue;
	int iValue;
	double dValue;
	if (getJsDouble(cx, rparams, "gameSpeed", &dValue)) {
		this->setGameSpeed(dValue);
	}
	if (getJsInt32(cx, rparams, "duration300", &iValue)) {
		this->setDuration300(iValue);
	}
	if (getJsBoolean(cx, rparams, "durationUnlimited", &bValue)) {
		this->setDurationUnlimited(bValue);
	}
	if (getJsBoolean(cx, rparams, "targetObject", &bValue)) {
		this->setTargetObject(bValue);
	}
	if (getJsBoolean(cx, rparams, "targetEffect", &bValue)) {
		this->setTargetEffect(bValue);
	}
	if (getJsBoolean(cx, rparams, "targetTile", &bValue)) {
		this->setTargetTile(bValue);
	}
	if (getJsBoolean(cx, rparams, "targetMenu", &bValue)) {
		this->setTargetMenu(bValue);
	}
	if (getJsInt32(cx, rparams, "targettingType", &iValue)) {
		this->setTargettingType((EnumTargettingType)iValue);
	}
	if (isJsDefined(cx, rparams, "targetObjectGroup")) {
		if (getJsInt32(cx, rparams, "targetObjectGroup", &iValue)) {
			this->setTargetObjectGroup(iValue);
		}
	}
	else {
		if (getJsInt32(cx, rparams, "targetObjectType", &iValue)) {
			this->setTargetObjectGroup(iValue - kObjectGroupAllOffset);
		}
	}
	if (getJsInt32(cx, rparams, "targetObjectId", &iValue)) {
		this->setTargetObjectId(iValue);
	}
	if (getJsInt32(cx, rparams, "targetQualifierId", &iValue)) {
		this->setTargetQualifierId((EnumQualifierType)iValue);
	}
	if (isJsDefined(cx, rparams, "excludeObjectGroupBit")) {
		if (getJsInt32(cx, rparams, "excludeObjectGroupBit", &iValue)) {
			this->setExcludeObjectGroupBit(iValue);
		}
	}
	else {
		int excludeObjectGroupBit = 0;
		if (getJsBoolean(cx, rparams, "excludePlayerObject", &bValue)) {
			excludeObjectGroupBit |= bValue ? (1 << kObjectGroupPlayer) : 0;
		}
		if (getJsBoolean(cx, rparams, "excludeEnemyObject", &bValue)) {
			excludeObjectGroupBit |= bValue ? (1 << kObjectGroupEnemy) : 0;
		}
		this->setExcludeObjectGroupBit(excludeObjectGroupBit);
	}

	return true;
}

#if defined(AGTK_DEBUG)
void ObjectCommandGameSpeedChangeData::dump()
{
	ObjectCommandData::dump();
	CCLOG("gameSpeed:%f", this->getGameSpeed());
	CCLOG("duration300:%d", this->getDuration300());
	CCLOG("durationUnlimited:%d", this->getDurationUnlimited());
	CCLOG("targetObject:%d", this->getTargetObject());
	CCLOG("targetEffect:%d", this->getTargetEffect());
	CCLOG("targetTile:%d", this->getTargetTile());
	CCLOG("targetMenu:%d", this->getTargetMenu());
	CCLOG("targettingType:%d", this->getTargettingType());
	CCLOG("targetObjectGroup:%d", this->getTargetObjectGroup());
	CCLOG("targetObjectId:%d", this->getTargetObjectId());
	CCLOG("targetQualifierId:%d", this->getTargetQualifierId());
	CCLOG("excludeObjectGroupBit:%d", this->getExcludeObjectGroupBit());
}
#endif

//-------------------------------------------------------------------------------------------------------------------
bool ObjectCommandWaitData::init(const rapidjson::Value& json)
{
	if (!ObjectCommandData::init(json)) {
		return false;
	}
	CC_ASSERT(json.HasMember("wait"));
	const rapidjson::Value& cmd = json["wait"];
	this->setDuration300(cmd["duration300"].GetInt());
	this->setStopGame(cmd["stopGame"].GetBool());
	return true;
}

#if defined(AGTK_DEBUG)
void ObjectCommandWaitData::dump()
{
	ObjectCommandData::dump();
	CCLOG("duration300:%d", this->getDuration300());
	CCLOG("stopGame:%d", this->getStopGame());
}
#endif

//-------------------------------------------------------------------------------------------------------------------
bool ObjectCommandSceneTerminateData::init(const rapidjson::Value& json)
{
	if (!ObjectCommandData::init(json)) {
		return false;
	}
	CC_ASSERT(json.HasMember("sceneTerminate"));
	return true;
}

bool ObjectCommandSceneTerminateData::init(void *jsCx, void *jsObj)
{
	_commandType = kSceneTerminate;

	return true;
}

#if defined(AGTK_DEBUG)
void ObjectCommandSceneTerminateData::dump()
{
	ObjectCommandData::dump();
}
#endif

//-------------------------------------------------------------------------------------------------------------------
bool ObjectCommandDirectionMoveData::init(const rapidjson::Value& json)
{
	if (!ObjectCommandData::init(json)) {
		return false;
	}
	CC_ASSERT(json.HasMember("directionMove"));
	const rapidjson::Value& cmd = json["directionMove"];
	this->setDirection(cmd["direction"].GetDouble());
	this->setDirectionId(cmd["directionId"].GetInt());
	if (cmd.HasMember("moveDistance")) {
		this->setMoveDistance(cmd["moveDistance"].GetInt());
	}
	if (cmd.HasMember("moveDistanceEnabled")) {
		this->setMoveDistanceEnabled(cmd["moveDistanceEnabled"].GetBool());
	}
	return true;
}

bool ObjectCommandDirectionMoveData::init(void *jsCx, void *jsObj)
{
	_commandType = kDirectionMove;
	//デフォルト値設定。
	this->setDirection(270);
	this->setDirectionId(kAccordingToMoveDirection);
	this->setMoveDistance(0);
	this->setMoveDistanceEnabled(false);

	auto cx = (JSContext *)jsCx;
	JS::RootedValue v(cx);
	JS::RootedObject rparams(cx, (JSObject *)jsObj);
	int iValue;
	double dValue;
	bool bValue;
	if (getJsDouble(cx, rparams, "direction", &dValue)) {
		this->setDirection(dValue);
	}
	if (getJsInt32(cx, rparams, "directionId", &iValue)) {
		this->setDirectionId(iValue);
	}
	if (getJsInt32(cx, rparams, "moveDistance", &iValue)) {
		this->setMoveDistance(iValue);
	}
	if (getJsBoolean(cx, rparams, "moveDistanceEnabled", &bValue)) {
		this->setMoveDistanceEnabled(bValue);
	}

	return true;
}

#if defined(AGTK_DEBUG)
void ObjectCommandDirectionMoveData::dump()
{
	ObjectCommandData::dump();
	CCLOG("direction:%f", this->getDirection());
	CCLOG("directionId:%d", this->getDirectionId());
	CCLOG("moveDistance:%d", this->getMoveDistance());
	CCLOG("moveDistanceEnabled:%d", this->getMoveDistanceEnabled());
}
#endif

//-------------------------------------------------------------------------------------------------------------------
bool ObjectCommandForthBackMoveTurnData::init(const rapidjson::Value& json)
{
	if (!ObjectCommandData::init(json)) {
		return false;
	}
	CC_ASSERT(json.HasMember("forthBackMoveTurn"));
	const rapidjson::Value& cmd = json["forthBackMoveTurn"];
	this->setMoveType(cmd["moveType"].GetInt());
	this->setTurnType(cmd["turnType"].GetInt());
	this->setDirectionId(cmd["directionId"].GetInt());
	return true;
}

bool ObjectCommandForthBackMoveTurnData::init(void *jsCx, void *jsObj)
{
	_commandType = kForthBackMoveTurn;
	//デフォルト値設定。
	this->setMoveType(kMoveNone);
	this->setTurnType(kTurnNone);
	this->setDirectionId(kAccordingToMoveDirection);

	auto cx = (JSContext *)jsCx;
	JS::RootedValue v(cx);
	JS::RootedObject rparams(cx, (JSObject *)jsObj);
	int iValue;
	if (getJsInt32(cx, rparams, "moveType", &iValue)) {
		this->setMoveType(iValue);
	}
	if (getJsInt32(cx, rparams, "turnType", &iValue)) {
		this->setTurnType(iValue);
	}
	if (getJsInt32(cx, rparams, "directionId", &iValue)) {
		this->setDirectionId(iValue);
	}

	return true;
}

#if defined(AGTK_DEBUG)
void ObjectCommandForthBackMoveTurnData::dump()
{
	ObjectCommandData::dump();
	CCLOG("moveType:%d", this->getMoveType());
	CCLOG("turnType:%d", this->getTurnType());
	CCLOG("directionId:%d", this->getDirectionId());
}
#endif

//-------------------------------------------------------------------------------------------------------------------
bool ObjectCommandActionExecData::init(const rapidjson::Value& json)
{
	if (!ObjectCommandData::init(json)) {
		return false;
	}
	CC_ASSERT(json.HasMember("actionExec"));
	const rapidjson::Value& cmd = json["actionExec"];
	this->setObjectId(cmd["objectId"].GetInt());
	this->setQualifierId((EnumQualifierType)cmd["qualifierId"].GetInt());
	this->setActionId(cmd["actionId"].GetInt());
	return true;
}

bool ObjectCommandActionExecData::init(void *jsCx, void *jsObj)
{
	_commandType = kActionExec;
	//デフォルト値設定。
	this->setObjectId(-1);
	this->setQualifierId(kQualifierSingle);
	this->setActionId(-1);

	auto cx = (JSContext *)jsCx;
	JS::RootedValue v(cx);
	JS::RootedObject rparams(cx, (JSObject *)jsObj);
	int iValue;
	if (getJsInt32(cx, rparams, "objectId", &iValue)) {
		this->setObjectId(iValue);
	}
	if (getJsInt32(cx, rparams, "qualifierId", &iValue)) {
		this->setQualifierId((EnumQualifierType)iValue);
	}
	if (getJsInt32(cx, rparams, "actionId", &iValue)) {
		this->setActionId(iValue);
	}

	return true;
}

#if defined(AGTK_DEBUG)
void ObjectCommandActionExecData::dump()
{
	ObjectCommandData::dump();
	CCLOG("objectId:%d", this->getObjectId());
	CCLOG("qualifierId:%d", this->getQualifierId());
	CCLOG("actionId:%d", this->getActionId());
}
#endif

//-------------------------------------------------------------------------------------------------------------------
bool ObjectCommandParticleShowData::init(const rapidjson::Value& json)
{
	if (!ObjectCommandData::init(json)) {
		return false;
	}
	CC_ASSERT(json.HasMember("particleShow"));
	const rapidjson::Value& cmd = json["particleShow"];
	CC_ASSERT(cmd.HasMember("particleId"));
	this->setParticleId(cmd["particleId"].GetInt());
	CC_ASSERT(cmd.HasMember("positionType"));
	this->setPositionType(cmd["positionType"].GetInt());
	CC_ASSERT(cmd.HasMember("useConnect"));
	this->setUseConnect(cmd["useConnect"].GetBool());
	CC_ASSERT(cmd.HasMember("connectId"));
	this->setConnectId(cmd["connectId"].GetInt());
	CC_ASSERT(cmd.HasMember("adjustX"));
	this->setAdjustX(cmd["adjustX"].GetInt());
	CC_ASSERT(cmd.HasMember("adjustY"));
	this->setAdjustY(cmd["adjustY"].GetInt());
	CC_ASSERT(cmd.HasMember("duration300"));
	this->setDuration300(cmd["duration300"].GetInt());
	CC_ASSERT(cmd.HasMember("durationUnlimited"));
	this->setDurationUnlimited(cmd["durationUnlimited"].GetBool());

	return true;
}

bool ObjectCommandParticleShowData::init(void *jsCx, void *jsObj)
{
	_commandType = kParticleShow;
	//デフォルト値設定。
	this->setParticleId(-1);
	this->setPositionType(kPositionCenter);
	this->setUseConnect(false);
	this->setConnectId(-1);
	this->setAdjustX(0);
	this->setAdjustY(0);
	this->setDuration300(300);
	this->setDurationUnlimited(false);

	auto cx = (JSContext *)jsCx;
	JS::RootedValue v(cx);
	JS::RootedObject rparams(cx, (JSObject *)jsObj);
	bool bValue;
	int iValue;
	if (getJsInt32(cx, rparams, "particleId", &iValue)) {
		this->setParticleId(iValue);
	}
	if (getJsInt32(cx, rparams, "positionType", &iValue)) {
		this->setPositionType(iValue);
	}
	if (getJsBoolean(cx, rparams, "useConnect", &bValue)) {
		this->setUseConnect(bValue);
	}
	if (getJsInt32(cx, rparams, "connectId", &iValue)) {
		this->setConnectId(iValue);
	}
	if (getJsInt32(cx, rparams, "adjustX", &iValue)) {
		this->setAdjustX(iValue);
	}
	if (getJsInt32(cx, rparams, "adjustY", &iValue)) {
		this->setAdjustY(iValue);
	}
	if (getJsInt32(cx, rparams, "duration300", &iValue)) {
		this->setDuration300(iValue);
	}
	if (getJsBoolean(cx, rparams, "durationUnlimited", &bValue)) {
		this->setDurationUnlimited(bValue);
	}

	return true;
}

#if defined(AGTK_DEBUG)
void ObjectCommandParticleShowData::dump()
{
	ObjectCommandData::dump();
	CCLOG("particleId", this->getParticleId());
	CCLOG("positionType", this->getPositionType());
	CCLOG("useConnect", this->getUseConnect());
	CCLOG("connectId", this->getConnectId());
	CCLOG("adjustX", this->getAdjustX());
	CCLOG("adjustY", this->getAdjustY());
	CCLOG("duration300", this->getDuration300());
	CCLOG("durationUnlimited", this->getDurationUnlimited());
}
#endif

//-------------------------------------------------------------------------------------------------------------------
bool ObjectCommandTimerData::init(const rapidjson::Value& json)
{
	if (!ObjectCommandData::init(json)) {
		return false;
	}
	CC_ASSERT(json.HasMember("timer"));
	const rapidjson::Value& cmd = json["timer"];
	CC_ASSERT(cmd.HasMember("start"));
	this->setStart(cmd["start"].GetBool());
	CC_ASSERT(cmd.HasMember("timerVariableObjectId"));
	this->setTimerVariableObjectId(cmd["timerVariableObjectId"].GetInt());
	if (cmd.HasMember("timerVariableQualifierId")) {
		this->setTimerVariableQualifierId(cmd["timerVariableQualifierId"].GetInt());
	}
	CC_ASSERT(cmd.HasMember("timerVariableId"));
	this->setTimerVariableId(cmd["timerVariableId"].GetInt());
	CC_ASSERT(cmd.HasMember("countUp"));
	this->setCountUp(cmd["countUp"].GetBool());
	CC_ASSERT(cmd.HasMember("secondType"));
	this->setSecondType(cmd["secondType"].GetInt());
	CC_ASSERT(cmd.HasMember("second300"));
	this->setSecond300(cmd["second300"].GetInt());
	CC_ASSERT(cmd.HasMember("secondVariableObjectId"));
	this->setSecondVariableObjectId(cmd["secondVariableObjectId"].GetInt());
	if (cmd.HasMember("secondVariableQualifierId")) {
		this->setSecondVariableQualifierId(cmd["secondVariableQualifierId"].GetInt());
	}
	CC_ASSERT(cmd.HasMember("secondVariableId"));
	this->setSecondVariableId(cmd["secondVariableId"].GetInt());
	return true;
}

bool ObjectCommandTimerData::init(void *jsCx, void *jsObj)
{
	_commandType = kTimer;
	//デフォルト値設定。
	this->setStart(true);
	this->setTimerVariableObjectId(-1);
	this->setTimerVariableQualifierId(kQualifierSingle);
	this->setTimerVariableId(-1);
	this->setCountUp(true);
	this->setSecondType(kSecondByValue);
	this->setSecond300(300);
	this->setSecondVariableObjectId(-1);
	this->setSecondVariableQualifierId(kQualifierSingle);
	this->setSecondVariableId(-1);

	auto cx = (JSContext *)jsCx;
	JS::RootedValue v(cx);
	JS::RootedObject rparams(cx, (JSObject *)jsObj);
	bool bValue;
	int iValue;
	if (getJsBoolean(cx, rparams, "start", &bValue)) {
		this->setStart(bValue);
	}
	if (getJsInt32(cx, rparams, "timerVariableObjectId", &iValue)) {
		this->setTimerVariableObjectId(iValue);
	}
	if (getJsInt32(cx, rparams, "timerVariableQualifierId", &iValue)) {
		this->setTimerVariableQualifierId(iValue);
	}
	if (getJsInt32(cx, rparams, "timerVariableId", &iValue)) {
		this->setTimerVariableId(iValue);
	}
	if (getJsBoolean(cx, rparams, "countUp", &bValue)) {
		this->setCountUp(bValue);
	}
	if (getJsInt32(cx, rparams, "secondType", &iValue)) {
		this->setSecondType(iValue);
	}
	if (getJsInt32(cx, rparams, "second300", &iValue)) {
		this->setSecond300(iValue);
	}
	if (getJsInt32(cx, rparams, "secondVariableObjectId", &iValue)) {
		this->setSecondVariableObjectId(iValue);
	}
	if (getJsInt32(cx, rparams, "secondVariableQualifierId", &iValue)) {
		this->setSecondVariableQualifierId(iValue);
	}
	if (getJsInt32(cx, rparams, "secondVariableId", &iValue)) {
		this->setSecondVariableId(iValue);
	}

	return true;
}

#if defined(AGTK_DEBUG)
void ObjectCommandTimerData::dump()
{
	ObjectCommandData::dump();
	CCLOG("start:%d", this->getStart());
	CCLOG("timerVariableObjectId:%d", this->getTimerVariableObjectId());
	CCLOG("timerVariableId:%d", this->getTimerVariableId());
	CCLOG("countUp:%d", this->getCountUp());
	CCLOG("secondType:%d", this->getSecondType());
	CCLOG("second300:%d", this->getSecond300());
	CCLOG("secondVariableObjectId:%d", this->getSecondVariableObjectId());
	CCLOG("secondVariableId:%d", this->getSecondVariableId());
}
#endif

//-------------------------------------------------------------------------------------------------------------------
bool ObjectCommandSceneShakeData::init(const rapidjson::Value& json)
{
	if (!ObjectCommandData::init(json)) {
		return false;
	}
	CC_ASSERT(json.HasMember("sceneShake"));
	const rapidjson::Value& cmd = json["sceneShake"];
	this->setDuration300(cmd["duration300"].GetInt());
	this->setFadeIn(cmd["fadeIn"].GetBool());
	this->setFadeOut(cmd["fadeOut"].GetBool());
	this->setInterval300(cmd["interval300"].GetInt());
	this->setHeight(cmd["height"].GetInt());
	this->setHeightDispersion(cmd["heightDispersion"].GetInt());
	this->setWidth(cmd["width"].GetInt());
	this->setWidthDispersion(cmd["widthDispersion"].GetInt());
	return true;
}

bool ObjectCommandSceneShakeData::init(void *jsCx, void *jsObj)
{
	_commandType = kSceneShake;
	//デフォルト値設定。
	this->setDuration300(300);
	this->setFadeIn(false);
	this->setFadeOut(false);
	this->setInterval300(30);
	this->setHeight(16);
	this->setHeightDispersion(16);
	this->setWidth(8);
	this->setWidthDispersion(8);

	auto cx = (JSContext *)jsCx;
	JS::RootedValue v(cx);
	JS::RootedObject rparams(cx, (JSObject *)jsObj);
	bool bValue;
	int iValue;
	if (getJsInt32(cx, rparams, "duration300", &iValue)) {
		this->setDuration300(iValue);
	}
	if (getJsBoolean(cx, rparams, "fadeIn", &bValue)) {
		this->setFadeIn(bValue);
	}
	if (getJsBoolean(cx, rparams, "fadeOut", &bValue)) {
		this->setFadeOut(bValue);
	}
	if (getJsInt32(cx, rparams, "interval300", &iValue)) {
		this->setInterval300(iValue);
	}
	if (getJsInt32(cx, rparams, "height", &iValue)) {
		this->setHeight(iValue);
	}
	if (getJsInt32(cx, rparams, "heightDispersion", &iValue)) {
		this->setHeightDispersion(iValue);
	}
	if (getJsInt32(cx, rparams, "width", &iValue)) {
		this->setWidth(iValue);
	}
	if (getJsInt32(cx, rparams, "widthDispersion", &iValue)) {
		this->setWidthDispersion(iValue);
	}

	return true;
}

#if defined(AGTK_DEBUG)
void ObjectCommandSceneShakeData::dump()
{
	ObjectCommandData::dump();
	CCLOG("duration300:%d", this->getDuration300());
	CCLOG("fadeIn:%d", this->getFadeIn());
	CCLOG("fadeOut:%d", this->getFadeOut());
	CCLOG("interval300:%d", this->getInterval300());
	CCLOG("height:%d", this->getHeight());
	CCLOG("heightDispersion:%d", this->getHeightDispersion());
	CCLOG("width:%d", this->getWidth());
	CCLOG("widthDispersion:%d", this->getWidthDispersion());
}
#endif

//-------------------------------------------------------------------------------------------------------------------
bool ObjectCommandEffectRemoveData::init(const rapidjson::Value& json)
{
	if (!ObjectCommandData::init(json)) {
		return false;
	}
	CC_ASSERT(json.HasMember("effectRemove"));
	const rapidjson::Value& cmd = json["effectRemove"];
	this->setEffectId(cmd["effectId"].GetInt());
	this->setTargetObjectId(cmd["targetObjectId"].GetInt());
	this->setTargetObjectGroup(cmd["targetObjectGroup"].GetInt());
	this->setTargettingType((EnumTargettingType)cmd["targettingType"].GetInt());
	return true;
}

bool ObjectCommandEffectRemoveData::init(void *jsCx, void *jsObj)
{
	_commandType = kEffectRemove;
	//デフォルト値設定。
	this->setEffectId(kAllParticles);
	this->setTargettingType(kTargettingSceneEffect);
	this->setTargetObjectGroup(kObjectGroupAll);
	this->setTargetObjectId(-1);

	auto cx = (JSContext *)jsCx;
	JS::RootedValue v(cx);
	JS::RootedObject rparams(cx, (JSObject *)jsObj);
	int iValue;
	if (getJsInt32(cx, rparams, "effectId", &iValue)) {
		this->setEffectId(iValue);
	}
	if (getJsInt32(cx, rparams, "targettingType", &iValue)) {
		this->setTargettingType((EnumTargettingType)iValue);
	}
	if (isJsDefined(cx, rparams, "targetObjectGroup")) {
		if (getJsInt32(cx, rparams, "targetObjectGroup", &iValue)) {
			this->setTargetObjectGroup(iValue);
		}
	}
	else {
		if (getJsInt32(cx, rparams, "targetObjectType", &iValue)) {
			this->setTargetObjectGroup(iValue - kObjectGroupAllOffset);
		}
	}
	if (getJsInt32(cx, rparams, "targetObjectId", &iValue)) {
		this->setTargetObjectId(iValue);
	}

	return true;
}

#if defined(AGTK_DEBUG)
void ObjectCommandEffectRemoveData::dump()
{
	ObjectCommandData::dump();
	CCLOG("effectId:%d", this->getEffectId());
	CCLOG("targetObjectId:%d", this->getTargetObjectId());
	CCLOG("targetObjectGroup:%d", this->getTargetObjectGroup());
	CCLOG("targettingType:%d", this->getTargettingType());
}
#endif

//-------------------------------------------------------------------------------------------------------------------
bool ObjectCommandParticleRemoveData::init(const rapidjson::Value& json)
{
	if (!ObjectCommandData::init(json)) {
		return false;
	}
	CC_ASSERT(json.HasMember("particleRemove"));
	const rapidjson::Value& cmd = json["particleRemove"];
	this->setParticleId(cmd["particleId"].GetInt());
	this->setTargetObjectId(cmd["targetObjectId"].GetInt());
	this->setTargetObjectGroup(cmd["targetObjectGroup"].GetInt());
	this->setTargettingType((EnumTargettingType)cmd["targettingType"].GetInt());
	return true;
}

bool ObjectCommandParticleRemoveData::init(void *jsCx, void *jsObj)
{
	_commandType = kParticleRemove;
	//デフォルト値設定。
	this->setParticleId(kAllParticles);
	this->setTargettingType(kTargettingSceneEffect);
	this->setTargetObjectGroup(kObjectTypeAll);
	this->setTargetObjectId(-1);

	auto cx = (JSContext *)jsCx;
	JS::RootedValue v(cx);
	JS::RootedObject rparams(cx, (JSObject *)jsObj);
	int iValue;
	if (getJsInt32(cx, rparams, "particleId", &iValue)) {
		this->setParticleId(iValue);
	}
	if (getJsInt32(cx, rparams, "targettingType", &iValue)) {
		this->setTargettingType((EnumTargettingType)iValue);
	}
	if (isJsDefined(cx, rparams, "targetObjectGroup")) {
		if (getJsInt32(cx, rparams, "targetObjectGroup", &iValue)) {
			this->setTargetObjectGroup(iValue);
		}
	}
	else {
		if (getJsInt32(cx, rparams, "targetObjectType", &iValue)) {
			this->setTargetObjectGroup(iValue - kObjectGroupAllOffset);
		}
	}
	if (getJsInt32(cx, rparams, "targetObjectId", &iValue)) {
		this->setTargetObjectId(iValue);
	}

	return true;
}

#if defined(AGTK_DEBUG)
void ObjectCommandParticleRemoveData::dump()
{
	ObjectCommandData::dump();
	CCLOG("particleId:%d", this->getParticleId());
	CCLOG("targetObjectId:%d", this->getTargetObjectId());
	CCLOG("targetObjectGroup:%d", this->getTargetObjectGroup());
	CCLOG("targettingType:%d", this->getTargettingType());
}
#endif

//-------------------------------------------------------------------------------------------------------------------
bool ObjectCommandLayerHide::init(const rapidjson::Value& json)
{
	if (!ObjectCommandData::init(json)) {
		return false;
	}
	CC_ASSERT(json.HasMember("layerHide"));
	const rapidjson::Value& cmd = json["layerHide"];
	this->setExceptFlag(cmd["exceptFlag"].GetBool());
	this->setLayerIndex(cmd["layerIndex"].GetInt());
	return true;
}

bool ObjectCommandLayerHide::init(void *jsCx, void *jsObj)
{
	_commandType = kLayerHide;
	//デフォルト値設定。
	this->setLayerIndex(-1);
	this->setExceptFlag(false);

	auto cx = (JSContext *)jsCx;
	JS::RootedValue v(cx);
	JS::RootedObject rparams(cx, (JSObject *)jsObj);
	bool bValue;
	int iValue;
	if (getJsInt32(cx, rparams, "layerIndex", &iValue)) {
		this->setLayerIndex(iValue);
	}
	if (getJsBoolean(cx, rparams, "exceptFlag", &bValue)) {
		this->setExceptFlag(bValue);
	}

	return true;
}

#if defined(AGTK_DEBUG)
void ObjectCommandLayerHide::dump()
{
	CCLOG("exceptFlag:%s", DUMP_BOOLTEXT(this->getExceptFlag()));
	CCLOG("layerIndex:%d", this->getLayerIndex());
}
#endif
//-------------------------------------------------------------------------------------------------------------------
bool ObjectCommandLayerShow::init(const rapidjson::Value& json)
{
	if (!ObjectCommandData::init(json)) {
		return false;
	}
	CC_ASSERT(json.HasMember("layerShow"));
	const rapidjson::Value& cmd = json["layerShow"];
	this->setExceptFlag(cmd["exceptFlag"].GetBool());
	this->setLayerIndex(cmd["layerIndex"].GetInt());
	return true;
}

bool ObjectCommandLayerShow::init(void *jsCx, void *jsObj)
{
	_commandType = kLayerShow;
	//デフォルト値設定。
	this->setLayerIndex(-1);
	this->setExceptFlag(false);

	auto cx = (JSContext *)jsCx;
	JS::RootedValue v(cx);
	JS::RootedObject rparams(cx, (JSObject *)jsObj);
	bool bValue;
	int iValue;
	if (getJsInt32(cx, rparams, "layerIndex", &iValue)) {
		this->setLayerIndex(iValue);
	}
	if (getJsBoolean(cx, rparams, "exceptFlag", &bValue)) {
		this->setExceptFlag(bValue);
	}

	return true;
}

#if defined(AGTK_DEBUG)
void ObjectCommandLayerShow::dump()
{
	CCLOG("exceptFlag:%s", DUMP_BOOLTEXT(this->getExceptFlag()));
	CCLOG("layerIndex:%d", this->getLayerIndex());
}
#endif
//-------------------------------------------------------------------------------------------------------------------
bool ObjectCommandLayerDisable::init(const rapidjson::Value& json)
{
	if (!ObjectCommandData::init(json)) {
		return false;
	}
	CC_ASSERT(json.HasMember("layerDisable"));
	const rapidjson::Value& cmd = json["layerDisable"];
	this->setExceptFlag(cmd["exceptFlag"].GetBool());
	this->setLayerIndex(cmd["layerIndex"].GetInt());
	return true;
}

bool ObjectCommandLayerDisable::init(void *jsCx, void *jsObj)
{
	_commandType = kLayerDisable;
	//デフォルト値設定。
	this->setLayerIndex(-1);
	this->setExceptFlag(false);

	auto cx = (JSContext *)jsCx;
	JS::RootedValue v(cx);
	JS::RootedObject rparams(cx, (JSObject *)jsObj);
	bool bValue;
	int iValue;
	if (getJsInt32(cx, rparams, "layerIndex", &iValue)) {
		this->setLayerIndex(iValue);
	}
	if (getJsBoolean(cx, rparams, "exceptFlag", &bValue)) {
		this->setExceptFlag(bValue);
	}

	return true;
}

#if defined(AGTK_DEBUG)
void ObjectCommandLayerDisable::dump()
{
	CCLOG("exceptFlag:%s", DUMP_BOOLTEXT(this->getExceptFlag()));
	CCLOG("layerIndex:%d", this->getLayerIndex());
}
#endif
//-------------------------------------------------------------------------------------------------------------------
bool ObjectCommandLayerEnable::init(const rapidjson::Value& json)
{
	if (!ObjectCommandData::init(json)) {
		return false;
	}
	CC_ASSERT(json.HasMember("layerEnable"));
	const rapidjson::Value& cmd = json["layerEnable"];
	this->setExceptFlag(cmd["exceptFlag"].GetBool());
	this->setLayerIndex(cmd["layerIndex"].GetInt());
	return true;
}

bool ObjectCommandLayerEnable::init(void *jsCx, void *jsObj)
{
	_commandType = kLayerEnable;
	//デフォルト値設定。
	this->setLayerIndex(-1);
	this->setExceptFlag(false);

	auto cx = (JSContext *)jsCx;
	JS::RootedValue v(cx);
	JS::RootedObject rparams(cx, (JSObject *)jsObj);
	bool bValue;
	int iValue;
	if (getJsInt32(cx, rparams, "layerIndex", &iValue)) {
		this->setLayerIndex(iValue);
	}
	if (getJsBoolean(cx, rparams, "exceptFlag", &bValue)) {
		this->setExceptFlag(bValue);
	}

	return true;
}

#if defined(AGTK_DEBUG)
void ObjectCommandLayerEnable::dump()
{
	CCLOG("exceptFlag:%s", DUMP_BOOLTEXT(this->getExceptFlag()));
	CCLOG("layerIndex:%d", this->getLayerIndex());
}
#endif

//-------------------------------------------------------------------------------------------------------------------
ObjectCommandScriptEvaluate::ObjectCommandScriptEvaluate()
{
	_script = nullptr;
}

ObjectCommandScriptEvaluate::~ObjectCommandScriptEvaluate()
{
	CC_SAFE_RELEASE_NULL(_script);
}

bool ObjectCommandScriptEvaluate::init(const rapidjson::Value& json)
{
	if (!ObjectCommandData::init(json)) {
		return false;
	}
	CC_ASSERT(json.HasMember("scriptEvaluate"));
	const rapidjson::Value& cmd = json["scriptEvaluate"];
	if (cmd["useCoffeeScript"].GetBool()) {
		this->setScript(cocos2d::__String::create(cmd["javaScript"].GetString()));
	}
	else {
		this->setScript(cocos2d::__String::create(cmd["script"].GetString()));
	}
	return true;
}

const char *ObjectCommandScriptEvaluate::getScript()
{
	CC_ASSERT(_script);
	return _script->getCString();
}

#if defined(AGTK_DEBUG)
void ObjectCommandScriptEvaluate::dump()
{
	ObjectCommandData::dump();
	CCLOG("script:%s", this->getScript());
}
#endif

//-------------------------------------------------------------------------------------------------------------------
bool ObjectCommandSoundStopData::init(const rapidjson::Value& json)
{
	if (!ObjectCommandData::init(json)) {
		return false;
	}
	CC_ASSERT(json.HasMember("soundStop"));
	const rapidjson::Value& cmd = json["soundStop"];
	this->setSoundType((EnumSound)cmd["soundType"].GetInt());
	if (cmd.HasMember("fadeout")) {
		this->setFadeout(cmd["fadeout"].GetBool());
	}
	if (cmd.HasMember("duration300")) {
		this->setDuration300(cmd["duration300"].GetInt());
	}
	if (cmd.HasMember("seId")) {
		this->setSeId(cmd["seId"].GetInt());
	}
	if (cmd.HasMember("voiceId")) {
		this->setVoiceId(cmd["voiceId"].GetInt());
	}
	if (cmd.HasMember("bgmId")) {
		this->setBgmId(cmd["bgmId"].GetInt());
	}
	if (cmd.HasMember("stopOnlySoundByThisObject")) {
		this->setStopOnlySoundByThisObject(cmd["stopOnlySoundByThisObject"].GetBool());
	}
	return true;
}

bool ObjectCommandSoundStopData::init(void *jsCx, void *jsObj)
{
	_commandType = kSoundStop;
	//デフォルト値設定。
	this->setSoundType(kSoundSe);
	this->setFadeout(false);
	this->setDuration300(300);
	this->setSeId(-1);
	this->setVoiceId(-1);
	this->setBgmId(-1);
	this->setStopOnlySoundByThisObject(false);

	auto cx = (JSContext *)jsCx;
	JS::RootedValue v(cx);
	JS::RootedObject rparams(cx, (JSObject *)jsObj);
	bool bValue;
	int iValue;
	//double dValue;
	if (getJsInt32(cx, rparams, "soundType", &iValue)) {
		this->setSoundType((EnumSound)iValue);
	}
	if (getJsBoolean(cx, rparams, "fadeout", &bValue)) {
		this->setFadeout(bValue);
	}
	if (getJsInt32(cx, rparams, "duration300", &iValue)) {
		this->setDuration300(iValue);
	}
	if (getJsInt32(cx, rparams, "seId", &iValue)) {
		this->setSeId(iValue);
	}
	if (getJsInt32(cx, rparams, "voiceId", &iValue)) {
		this->setVoiceId(iValue);
	}
	if (getJsInt32(cx, rparams, "bgmId", &iValue)) {
		this->setBgmId(iValue);
	}
	if (getJsBoolean(cx, rparams, "stopOnlySoundByThisObject", &bValue)) {
		this->setStopOnlySoundByThisObject(bValue);
	}

	return true;
}

#if defined(AGTK_DEBUG)
void ObjectCommandSoundStopData::dump()
{
	ObjectCommandData::dump();
	CCLOG("soundType:%d", this->getSoundType());
	CCLOG("fadeout:%d", this->getFadeout());
	CCLOG("duration300:%d", this->getDuration300());
	CCLOG("seId:%d", this->getSeId());
	CCLOG("voiceId:%d", this->getVoiceId());
	CCLOG("bgmId:%d", this->getBgmId());
	CCLOG("stopOnlySoundByThisObject:%d", this->getStopOnlySoundByThisObject());
}
#endif

//-------------------------------------------------------------------------------------------------------------------
ObjectCommandMenuShowData *ObjectCommandMenuShowData::create(int layerId, bool useEffect, int effectType, bool fadein, int duration300)
{
	auto p = new (std::nothrow) ObjectCommandMenuShowData();
	if (p && p->init(layerId, useEffect, effectType, fadein, duration300)) {
		p->autorelease();
		return p;
	}
	CC_SAFE_DELETE(p);
	return nullptr;
}

bool ObjectCommandMenuShowData::init(int layerId, bool useEffect, int effectType, bool fadein, int duration300)
{
	this->setLayerId(layerId);
	this->setUseEffect(useEffect);
	this->setEffectType(effectType);
	this->setFadein(fadein);
	this->setDuration300(duration300);
	return true;
}

bool ObjectCommandMenuShowData::init(const rapidjson::Value& json)
{
	if (!ObjectCommandData::init(json)) {
		return false;
	}
	CC_ASSERT(json.HasMember("menuShow"));
	const rapidjson::Value& cmd = json["menuShow"];
	this->setLayerId(cmd["layerId"].GetInt());
	this->setUseEffect(cmd["useEffect"].GetBool());
	this->setEffectType(cmd["effectType"].GetInt());
	this->setFadein(cmd["fadein"].GetBool());
	this->setDuration300(cmd["duration300"].GetInt());
	return true;
}

bool ObjectCommandMenuShowData::init(void *jsCx, void *jsObj)
{
	_commandType = kMenuShow;
	//デフォルト値設定。
	this->setLayerId(-1);
	this->setUseEffect(false);
	this->setEffectType(kNone);
	this->setFadein(false);
	this->setDuration300(300);

	auto cx = (JSContext *)jsCx;
	JS::RootedValue v(cx);
	JS::RootedObject rparams(cx, (JSObject *)jsObj);
	bool bValue;
	int iValue;
	if (getJsInt32(cx, rparams, "layerId", &iValue)) {
		this->setLayerId(iValue);
	}
	if (getJsBoolean(cx, rparams, "useEffect", &bValue)) {
		this->setUseEffect(bValue);
	}
	if (getJsInt32(cx, rparams, "effectType", &iValue)) {
		this->setEffectType(iValue);
	}
	if (getJsBoolean(cx, rparams, "fadein", &bValue)) {
		this->setFadein(bValue);
	}
	if (getJsInt32(cx, rparams, "duration300", &iValue)) {
		this->setDuration300(iValue);
	}
	return true;
}

#if defined(AGTK_DEBUG)
void ObjectCommandMenuShowData::dump()
{
	ObjectCommandData::dump();
	CCLOG("layerId:%f", this->getLayerId());
	CCLOG("useEffect:%d", this->getUseEffect());
	CCLOG("effectType:%d", this->getEffectType());
	CCLOG("fadein:%d", this->getFadein());
	CCLOG("duratino300:%d", this->getDuration300());
}
#endif

//-------------------------------------------------------------------------------------------------------------------
ObjectCommandMenuHideData *ObjectCommandMenuHideData::create(bool hideExceptInitial, bool useEffect, int effectType, bool fadeout, int duration300, bool disableObjects)
{
	auto p = new (std::nothrow) ObjectCommandMenuHideData();
	if (p && p->init(hideExceptInitial, useEffect, effectType, fadeout, duration300, disableObjects)) {
		p->autorelease();
		return p;
	}
	CC_SAFE_DELETE(p);
	return nullptr;
}

bool ObjectCommandMenuHideData::init(bool hideExceptInitial, bool useEffect, int effectType, bool fadeout, int duration300, bool disableObjects)
{
	this->setHideExceptInitial(hideExceptInitial);
	this->setUseEffect(useEffect);
	this->setEffectType(effectType);
	this->setFadeout(fadeout);
	this->setDuration300(duration300);
	this->setDisableObjects(disableObjects);
	return true;
}

bool ObjectCommandMenuHideData::init(const rapidjson::Value& json)
{
	if (!ObjectCommandData::init(json)) {
		return false;
	}
	CC_ASSERT(json.HasMember("menuHide"));
	const rapidjson::Value& cmd = json["menuHide"];
	this->setHideExceptInitial(cmd["hideExceptInitial"].GetBool());
	this->setUseEffect(cmd["useEffect"].GetBool());
	this->setEffectType(cmd["effectType"].GetInt());
	this->setFadeout(cmd["fadeout"].GetBool());
	this->setDuration300(cmd["duration300"].GetInt());
	if (cmd.HasMember("disableObjects")) {
		this->setDisableObjects(cmd["disableObjects"].GetBool());
	}
	return true;
}

bool ObjectCommandMenuHideData::init(void *jsCx, void *jsObj)
{
	_commandType = kMenuHide;
	//デフォルト値設定。
	this->setHideExceptInitial(false);
	this->setUseEffect(false);
	this->setEffectType(kNone);
	this->setFadeout(false);
	this->setDuration300(300);
	this->setDisableObjects(true);

	auto cx = (JSContext *)jsCx;
	JS::RootedValue v(cx);
	JS::RootedObject rparams(cx, (JSObject *)jsObj);
	bool bValue;
	int iValue;
	if (getJsBoolean(cx, rparams, "hideExceptInitial", &bValue)) {
		this->setHideExceptInitial(bValue);
	}
	if (getJsBoolean(cx, rparams, "useEffect", &bValue)) {
		this->setUseEffect(bValue);
	}
	if (getJsInt32(cx, rparams, "effectType", &iValue)) {
		this->setEffectType(iValue);
	}
	if (getJsBoolean(cx, rparams, "fadeout", &bValue)) {
		this->setFadeout(bValue);
	}
	if (getJsInt32(cx, rparams, "duration300", &iValue)) {
		this->setDuration300(iValue);
	}
	if (getJsBoolean(cx, rparams, "disableObjects", &bValue)) {
		this->setDisableObjects(bValue);
	}

	return true;
}

#if defined(AGTK_DEBUG)
void ObjectCommandMenuHideData::dump()
{
	ObjectCommandData::dump();
	CCLOG("hideExceptInitial:%f", this->getHideExceptInitial());
	CCLOG("useEffect:%d", this->getUseEffect());
	CCLOG("effectType:%d", this->getEffectType());
	CCLOG("fadeout:%d", this->getFadeout());
	CCLOG("duratino300:%d", this->getDuration300());
	CCLOG("disableObjects:%d", this->getDisableObjects());
}
#endif

//-------------------------------------------------------------------------------------------------------------------
ObjectCommandDisplayDirectionMoveData *ObjectCommandDisplayDirectionMoveData::create(int moveDistance, bool reverse, bool distanceOverride)
{
	auto p = new (std::nothrow) ObjectCommandDisplayDirectionMoveData();
	if (p && p->init(moveDistance, reverse, distanceOverride)) {
		p->autorelease();
		return p;
	}
	CC_SAFE_DELETE(p);
	return nullptr;
}

bool ObjectCommandDisplayDirectionMoveData::init(int moveDistance, bool reverse, bool distanceOverride)
{
	this->setMoveDistance(moveDistance);
	this->setReverse(reverse);
	this->setDistanceOverride(distanceOverride);
	return true;
}

bool ObjectCommandDisplayDirectionMoveData::init(const rapidjson::Value& json)
{
	if (!ObjectCommandData::init(json)) {
		return false;
	}
	CC_ASSERT(json.HasMember("displayDirectionMove"));
	const rapidjson::Value& cmd = json["displayDirectionMove"];
	this->setMoveDistance(cmd["moveDistance"].GetInt());
	this->setReverse(cmd["reverse"].GetBool());
	if (cmd.HasMember("distanceOverride")) {
		this->setDistanceOverride(cmd["distanceOverride"].GetBool());
	}

	return true;
}

bool ObjectCommandDisplayDirectionMoveData::init(void *jsCx, void *jsObj)
{
	_commandType = kDisplayDirectionMove;
	//デフォルト値設定。
	this->setMoveDistance(0);
	this->setReverse(false);
	this->setDistanceOverride(false);

	auto cx = (JSContext *)jsCx;
	JS::RootedValue v(cx);
	JS::RootedObject rparams(cx, (JSObject *)jsObj);
	bool bValue;
	int iValue;
	if (getJsInt32(cx, rparams, "moveDistance", &iValue)) {
		this->setMoveDistance(iValue);
	}
	if (getJsBoolean(cx, rparams, "reverse", &bValue)) {
		this->setReverse(bValue);
	}
	if (getJsBoolean(cx, rparams, "distanceOverride", &bValue)) {
		this->setDistanceOverride(bValue);
	}

	return true;
}

#if defined(AGTK_DEBUG)
void ObjectCommandDisplayDirectionMoveData::dump()
{
	ObjectCommandData::dump();
	CCLOG("moveDistance:%f", this->getMoveDistance());
	CCLOG("reverse:%d", this->getReverse());
	CCLOG("distanceOverride:%d", this->getDistanceOverride());
}
#endif

//-------------------------------------------------------------------------------------------------------------------

ObjectCommandFileLoadData::ObjectCommandFileLoadData()
	: ObjectCommandData()
	, _projectCommonVariables(false)
	, _projectCommonSwitches(false)
	, _sceneAtTimeOfSave(false)
	, _objectsStatesInSceneAtTimeOfSave(false)
	, _effectType(kEffectTypeNone)
	, _duration300(300)
{
	_dataSetterList.setter = {
		{ FUNC_BOOL("projectCommonVariables",          _projectCommonVariables) },
		{ FUNC_BOOL("projectCommonSwitches",           _projectCommonSwitches) },
		{ FUNC_BOOL("sceneAtTimeOfSave",               _sceneAtTimeOfSave) },
		{ FUNC_BOOL("objectsStatesInSceneAtTimeOfSave",_objectsStatesInSceneAtTimeOfSave) },
		{ FUNC_INT("effectType",_effectType) },
		{ FUNC_INT("duration300",_duration300) },
	};
}

bool ObjectCommandFileLoadData::init(const rapidjson::Value& json)
{
	if (!ObjectCommandData::init(json)) {
		return false;
	}
	_dataSetterList.execJson(json["loadFile"]);
	if (_effectType == kEffectTypeNone) {
		_duration300 = 300;
	}

	return true;
}

bool ObjectCommandFileLoadData::init(void *jsCx, void *jsObj)
{
	_commandType = kFileLoad;
	_dataSetterList.execJs(jsCx, jsObj);
	if (_effectType == kEffectTypeNone) {
		_duration300 = 300;
	}
	return true;
}

#if defined(AGTK_DEBUG)
void ObjectCommandFileLoadData::dump()
{
	_dataSetterList.execDump();
}
#endif

//-------------------------------------------------------------------------------------------------------------------
// 音の再生位置を保存
ObjectCommandSoundPositionRememberData::ObjectCommandSoundPositionRememberData()
	: ObjectCommandData()
	, _soundType(kSoundSe)
	, _variableObjectId(-1)
	, _variableQualifierId(kQualifierSingle)
	, _variableId(-1)
{
}

ObjectCommandSoundPositionRememberData::~ObjectCommandSoundPositionRememberData()
{
}

bool ObjectCommandSoundPositionRememberData::init(const rapidjson::Value& json)
{
	if (!ObjectCommandData::init(json)) {
		return false;
	}
	CC_ASSERT(json.HasMember("soundPositionRemember"));
	const rapidjson::Value& cmd = json["soundPositionRemember"];
	this->setSoundType((EnumSound)cmd["soundType"].GetInt());
	this->setVariableObjectId(cmd["variableObjectId"].GetInt());
	this->setVariableQualifierId((EnumQualifierType)cmd["variableQualifierId"].GetInt());
	this->setVariableId(cmd["variableId"].GetInt());
	return true;
}

bool ObjectCommandSoundPositionRememberData::init(void *jsCx, void *jsObj)
{
	_commandType = kSoundPositionRemember;
	this->setSoundType(kSoundSe);
	this->setVariableObjectId(-1);
	this->setVariableQualifierId(kQualifierSingle);
	this->setVariableId(-1);

	auto cx = (JSContext *)jsCx;
	JS::RootedValue v(cx);
	JS::RootedObject rparams(cx, (JSObject *)jsObj);
	int iValue;
	if (getJsInt32(cx, rparams, "soundType", &iValue)) {
		this->setSoundType((EnumSound)iValue);
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
	return true;
}

#if defined(AGTK_DEBUG)
void ObjectCommandSoundPositionRememberData::dump()
{
	ObjectCommandData::dump();
	CCLOG("soundType:%d", this->getSoundType());
	CCLOG("variableObjectId:%d", this->getVariableObjectId());
	CCLOG("variableQualifierId:%d", this->getVariableQualifierId());
	CCLOG("variableId:%d", this->getVariableId());
}
#endif

//-------------------------------------------------------------------------------------------------------------------
bool ObjectCommandObjectUnlockData::init(const rapidjson::Value& json)
{
	if (!ObjectCommandData::init(json)) {
		return false;
	}
	CC_ASSERT(json.HasMember("objectUnlock"));
	const rapidjson::Value& cmd = json["objectUnlock"];
	this->setObjectType(cmd["objectType"].GetInt());
	this->setObjectGroup(cmd["objectGroup"].GetInt());
	this->setObjectId(cmd["objectId"].GetInt());
	return true;
}

bool ObjectCommandObjectUnlockData::init(void *jsCx, void *jsObj)
{
	_commandType = kObjectUnlock;
	//デフォルト値設定。
	this->setObjectType(kObjectByGroup);
	this->setObjectGroup(kObjectGroupAll);
	this->setObjectId(-1);

	auto cx = (JSContext *)jsCx;
	JS::RootedValue v(cx);
	JS::RootedObject rparams(cx, (JSObject *)jsObj);
	bool bValue;
	int iValue;
	double dValue;
	if (getJsInt32(cx, rparams, "objectGroup", &iValue)) {
		this->setObjectGroup(iValue);
	}
	if (getJsInt32(cx, rparams, "objectType", &iValue)) {
		this->setObjectType(iValue);
	}
	if (getJsInt32(cx, rparams, "objectId", &iValue)) {
		this->setObjectId(iValue);
	}
	return true;
}

#if defined(AGTK_DEBUG)
void ObjectCommandObjectUnlockData::dump()
{
	ObjectCommandData::dump();
	CCLOG("objectGroup:%d", this->getObjectGroup());
	CCLOG("objectType:%d", this->getObjectType());
	CCLOG("objectId:%d", this->getObjectId());
}
#endif

//-------------------------------------------------------------------------------------------------------------------
bool ObjectCommandResourceSetChangeData::init(const rapidjson::Value& json)
{
	if (!ObjectCommandData::init(json)) {
		return false;
	}
	CC_ASSERT(json.HasMember("resourceSetChange"));
	const rapidjson::Value& cmd = json["resourceSetChange"];
	this->setObjectId(cmd["objectId"].GetInt());
	this->setQualifierId((EnumQualifierType)cmd["qualifierId"].GetInt());
	this->setResourceSetId(cmd["resourceSetId"].GetInt());
	return true;
}

bool ObjectCommandResourceSetChangeData::init(void *jsCx, void *jsObj)
{
	_commandType = kActionExec;
	//デフォルト値設定。
	this->setObjectId(-1);
	this->setQualifierId(kQualifierSingle);
	this->setResourceSetId(-1);

	auto cx = (JSContext *)jsCx;
	JS::RootedValue v(cx);
	JS::RootedObject rparams(cx, (JSObject *)jsObj);
	int iValue;
	if (getJsInt32(cx, rparams, "objectId", &iValue)) {
		this->setObjectId(iValue);
	}
	if (getJsInt32(cx, rparams, "qualifierId", &iValue)) {
		this->setQualifierId((EnumQualifierType)iValue);
	}
	if (getJsInt32(cx, rparams, "resourceSetId", &iValue)) {
		this->setResourceSetId(iValue);
	}

	return true;
}

#if defined(AGTK_DEBUG)
void ObjectCommandResourceSetChangeData::dump()
{
	ObjectCommandData::dump();
	CCLOG("objectId:%d", this->getObjectId());
	CCLOG("qualifierId:%d", this->getQualifierId());
	CCLOG("resourceSetId:%d", this->getResourceSetId());
}
#endif

//-------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------
ObjectCommandDatabaseReflectData::ObjectCommandDatabaseReflectData() : ObjectCommandData()
{

}

ObjectCommandDatabaseReflectData::~ObjectCommandDatabaseReflectData()
{
}

bool ObjectCommandDatabaseReflectData::init(const rapidjson::Value& json)
{

	if (!ObjectCommandData::init(json)) {
		return false;
	}
	CC_ASSERT(json.HasMember("databaseReflect"));
	const rapidjson::Value& cmd = json["databaseReflect"];
	this->setObjectId(cmd["objectId"].GetInt());
	this->setDatabaseId(cmd["databaseId"].GetInt());
	if (cmd.HasMember("fromObject")) {
		this->setFromObject(cmd["fromObject"].GetBool());
	}
	if (cmd.HasMember("fromRow")) {
		this->setFromRow(cmd["fromRow"].GetBool());
	}
	this->setColumnIndex(cmd["columnIndex"].GetInt());
	this->setRowIndex(cmd["rowIndex"].GetInt());
	if (cmd.HasMember("rowIndexFromName")) {
		this->setRowIndexFromName(cmd["rowIndexFromName"].GetBool());
	}
	if (cmd.HasMember("columnIndexFromName")) {
		this->setColumnIndexFromName(cmd["columnIndexFromName"].GetBool());
	}
	if (cmd.HasMember("rowNumberFromValue")) {
		this->setRowNumberFromValue(cmd["rowNumberFromValue"].GetBool());
	}
	if (cmd.HasMember("columnNumberFromValue")) {
		this->setColumnNumberFromValue(cmd["columnNumberFromValue"].GetBool());
	}
	this->setRowVariableObjectId(cmd["rowVariableObjectId"].GetInt());
	this->setRowVariableId(cmd["rowVariableId"].GetInt());
	this->setRowVariableQualifierId((EnumQualifierType)cmd["rowVariableQualifierId"].GetInt());
	this->setColumnVariableObjectId(cmd["columnVariableObjectId"].GetInt());
	this->setColumnVariableId(cmd["columnVariableId"].GetInt());
	this->setColumnVariableQualifierId((EnumQualifierType)cmd["columnVariableQualifierId"].GetInt());

	this->setReflectObjectId(cmd["reflectObjectId"].GetInt());
	this->setReflectVariableId(cmd["reflectVariableId"].GetInt());
	this->setReflectQualifierId((EnumQualifierType)cmd["reflectQualifierId"].GetInt());
	this->setReflectVariableAssignOperator((EnumVariableAssignOperatorType)cmd["reflectVariableAssignOperator"].GetInt());
	
	return true;
}

bool ObjectCommandDatabaseReflectData::init(void *jsCx, void *jsObj)
{
	_commandType = kActionExec;
	//デフォルト値設定。
	this->setObjectId(-1);
	this->setDatabaseId(-1);
	this->setFromObject(false);
	this->setFromRow(false);
	this->setColumnIndex(-1);
	this->setRowIndex(-1);
	this->setRowIndexFromName(false);
	this->setColumnIndexFromName(false);
	this->setRowNumberFromValue(false);
	this->setColumnNumberFromValue(false);
	this->setRowVariableObjectId(-1);
	this->setRowVariableId(-1);
	this->setRowVariableQualifierId(-1);
	this->setColumnVariableObjectId(-1);
	this->setColumnVariableId(-1);
	this->setColumnVariableQualifierId(-1);

	this->setReflectVariableId(-1);
	this->setReflectQualifierId(kQualifierSingle);
	this->setReflectVariableAssignOperator(kVariableAssignOperatorSet);

	auto cx = (JSContext *)jsCx;
	JS::RootedValue v(cx);
	JS::RootedObject rparams(cx, (JSObject *)jsObj);
	int iValue;
	bool bValue;
	cocos2d::String *sValue;
	if (getJsInt32(cx, rparams, "objectId", &iValue)) {
		this->setObjectId(iValue);
	}
	if (getJsInt32(cx, rparams, "databaseId", &iValue)) {
		this->setDatabaseId(iValue);
	}
	if (getJsBoolean(cx, rparams, "fromObject", &bValue)) {
		this->setFromObject(bValue);
	}
	if (getJsBoolean(cx, rparams, "fromRow", &bValue)) {
		this->setFromRow(bValue);
	}
	if (getJsInt32(cx, rparams, "columnIndex", &iValue)) {
		this->setColumnIndex(iValue);
	}
	if (getJsInt32(cx, rparams, "rowIndex", &iValue)) {
		this->setRowIndex(iValue);
	}
	if (getJsBoolean(cx, rparams, "rowIndexFromName", &bValue)) {
		this->setRowIndexFromName(bValue);
	}
	if (getJsBoolean(cx, rparams, "columnIndexFromName", &bValue)) {
		this->setColumnIndexFromName(bValue);
	}
	if (getJsBoolean(cx, rparams, "rowNumberFromValue", &bValue)) {
		this->setRowNumberFromValue(bValue);
	}
	if (getJsBoolean(cx, rparams, "columnNumberFromValue", &bValue)) {
		this->setColumnNumberFromValue(bValue);
	}
	if (getJsInt32(cx, rparams, "rowVariableObjectId", &iValue)) {
		this->setRowVariableObjectId(iValue);
	}
	if (getJsInt32(cx, rparams, "rowVariableId", &iValue)) {
		this->setRowVariableId(iValue);
	}
	if (getJsInt32(cx, rparams, "rowVariableQualifierId", &iValue)) {
		this->setRowVariableQualifierId((EnumQualifierType)iValue);
	}
	if (getJsInt32(cx, rparams, "columnVariableObjectId", &iValue)) {
		this->setColumnVariableObjectId(iValue);
	}
	if (getJsInt32(cx, rparams, "columnVariableId", &iValue)) {
		this->setColumnVariableId(iValue);
	}
	if (getJsInt32(cx, rparams, "columnVariableQualifierId", &iValue)) {
		this->setColumnVariableQualifierId((EnumQualifierType)iValue);
	}

	if (getJsInt32(cx, rparams, "reflectObjectId", &iValue)) {
		this->setReflectObjectId(iValue);
	}
	if (getJsInt32(cx, rparams, "reflectVariableId", &iValue)) {
		this->setReflectVariableId(iValue);
	}
	if (getJsInt32(cx, rparams, "reflectQualifierId", &iValue)) {
		this->setReflectQualifierId((EnumQualifierType)iValue);
	}
	if (getJsInt32(cx, rparams, "reflectVariableAssignOperator", &iValue)) {
		this->setReflectVariableAssignOperator((EnumVariableAssignOperatorType)iValue);
	}

	return true;
}

#if defined(AGTK_DEBUG)
void ObjectCommandDatabaseReflectData::dump()
{
	ObjectCommandData::dump();
	CCLOG("objectId:%d", this->getObjectId());
	CCLOG("databaseId:%d", this->getDatabaseId());
	CCLOG("fromObject:%d", this->getFromObject());
	CCLOG("fromRow:%d", this->getFromRow());
	CCLOG("columnIndex:%d", this->getColumnIndex());
	CCLOG("rowIndex:%d", this->getRowIndex());
	CCLOG("rowIndexFromName:%d", this->getRowIndexFromName());
	CCLOG("columnIndexFromName:%d", this->getColumnIndexFromName());
	CCLOG("rowNumberFromValue:%d", this->getRowNumberFromValue());
	CCLOG("columnNumberFromValue:%d", this->getColumnNumberFromValue());
	CCLOG("rowVariableObjectId:%d", this->getRowVariableObjectId());
	CCLOG("rowVariableId:%d", this->getRowVariableId());
	CCLOG("rowVariableQualifierId:%d", this->getRowVariableQualifierId());
	CCLOG("columnVariableObjectId:%d", this->getColumnVariableObjectId());
	CCLOG("columnVariableId:%d", this->getColumnVariableId());
	CCLOG("columnVariableQualifierId:%d", this->getColumnVariableQualifierId());

	CCLOG("reflectObjectId:%d", this->getReflectObjectId());
	CCLOG("reflectVariableId:%d", this->getReflectVariableId());
	CCLOG("reflectQualifierId:%d", this->getReflectQualifierId());
	CCLOG("reflectVariableAssignOperator:%d", this->getReflectVariableAssignOperator());
}
#endif
//-------------------------------------------------------------------------------------------------------------------
// #AGTK-NX
bool ObjectCommandkNXVibrateControlData::init(const rapidjson::Value& json)
{
	if (!ObjectCommandData::init(json)) {
		return false;
	}
	CC_ASSERT(json.HasMember("nxVibrateController"));
	const rapidjson::Value& cmd = json["nxVibrateController"];
	CC_ASSERT(cmd.HasMember("vibrationPattern"));
	this->setVibrationPattern(cmd["vibrationPattern"].GetInt());

	if (cmd.HasMember("vibrationTargetController"))
	{
		const rapidjson::Value& target_controller = cmd["vibrationTargetController"];
		CC_ASSERT(target_controller.HasMember("variableId"));
		this->setVariableId(target_controller["variableId"].GetInt());
	}

	return true;
}

bool ObjectCommandkNXVibrateControlData::init(void *jsCx, void *jsObj)
{
	_commandType = kNXVibrateController;
	//デフォルト値設定。
	this->setVibrationPattern(0);
	this->setVariableId(-1);

	auto cx = (JSContext *)jsCx;
	JS::RootedValue v(cx);
	JS::RootedObject rparams(cx, (JSObject *)jsObj);
	int iValue;
	if (getJsInt32(cx, rparams, "vibrationPattern", &iValue)) {
		this->setVibrationPattern(iValue);
	}
	// TODO variableId



	return true;
}

#if defined(AGTK_DEBUG)
void ObjectCommandkNXVibrateControlData::dump()
{
	ObjectCommandData::dump();
	CCLOG("vibrationPattern:%d", this->getVibrationPattern());
	CCLOG("variableId:%d", this->getVariableId());
}
#endif

//-------------------------------------------------------------------------------------------------------------------
// #AGTK-NX
bool ObjectCommandNXShowControllerAppletData::init(const rapidjson::Value& json)
{
	if (!ObjectCommandData::init(json)) {
		return false;
	}
	CC_ASSERT(json.HasMember("nxShowControllerSupportApplet"));
	const rapidjson::Value& cmd = json["nxShowControllerSupportApplet"];
	CC_ASSERT(cmd.HasMember("playerCount"));
	this->setPlayerCount(cmd["playerCount"].GetInt());
	return true;
}

bool ObjectCommandNXShowControllerAppletData::init(void *jsCx, void *jsObj)
{
	_commandType = kNXShowControllerApplet;
	//デフォルト値設定。
	this->setPlayerCount(4);

	auto cx = (JSContext *)jsCx;
	JS::RootedValue v(cx);
	JS::RootedObject rparams(cx, (JSObject *)jsObj);
	int iValue;
	if (getJsInt32(cx, rparams, "playerCount", &iValue)) {
		this->setPlayerCount(iValue);
	}

	return true;
}

#if defined(AGTK_DEBUG)
void ObjectCommandNXShowControllerAppletData::dump()
{
	ObjectCommandData::dump();
	CCLOG("playerCount:%d", this->getPlayerCount());
}
#endif

//-------------------------------------------------------------------------------------------------------------------

ObjectCommandCustomData::ObjectCommandCustomData()
{
	_customId = -1;
	_valueJson = nullptr;
}

ObjectCommandCustomData::~ObjectCommandCustomData()
{
	CC_SAFE_RELEASE_NULL(_valueJson);
}

bool ObjectCommandCustomData::init(const rapidjson::Value& json)
{
	if (!ObjectCommandData::init(json)) {
		return false;
	}
	CC_ASSERT(json.HasMember("custom"));
	const rapidjson::Value& cmd = json["custom"];
	this->setCustomId(cmd["customId"].GetInt());
	this->setValueJson(cocos2d::__String::create(cmd["valueJson"].GetString()));
	return true;
}

const char *ObjectCommandCustomData::getValueJson()
{
	CC_ASSERT(_valueJson);
	return _valueJson->getCString();
}

#if defined(AGTK_DEBUG)
void ObjectCommandCustomData::dump()
{
	ObjectCommandData::dump();
	CCLOG("customId:%d", this->getCustomId());
	CCLOG("valueJson:%s", this->getValueJson());
}
#endif

NS_DATA_END
NS_AGTK_END
