#include "Lib/Macros.h"
#include "GameManager.h"
#include "Data/ObjectCommandData.h"

#include "jsapi.h"
#include "jsfriendapi.h"
#include "scripting/js-bindings/manual/js_bindings_config.h"
#include "scripting/js-bindings/manual/js_bindings_core.h"
#include "scripting/js-bindings/manual/spidermonkey_specifics.h"
#include "scripting/js-bindings/manual/js_manual_conversions.h"
#include "mozilla/Maybe.h"
#include "scripting/js-bindings/manual/js-BindingsExport.h"
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_7
#include "scripting/js-bindings/manual/ScriptingCore.h"
#endif

static bool register_agtk_objectInstances(JSContext *_cx,JS::HandleObject object);

//objectInstances
static bool js_objectInstances_get(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_objectInstances_getIdByName(JSContext *cx, unsigned argc, JS::Value *vp);
//objectInstance
static bool js_objectInstance_getProperty(JSContext *cx, JS::HandleObject obj, JS::HandleId id, JS::MutableHandleValue vp);
static bool js_objectInstance_setProperty(JSContext *cx, JS::HandleObject obj, JS::HandleId id, bool strict, JS::MutableHandleValue vp);
static bool js_objectInstance_getValue(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_objectInstance_setValue(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_objectInstance_getAttackerInstanceIdList(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_objectInstance_execCommandTemplateMove(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_objectInstance_execCommandObjectLock(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_objectInstance_execCommandObjectCreate(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_objectInstance_execCommandObjectChange(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_objectInstance_execCommandObjectMove(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_objectInstance_execCommandObjectPushPull(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_objectInstance_execCommandLayerMove(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_objectInstance_execCommandAttackSetting(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_objectInstance_execCommandBulletFire(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_objectInstance_execCommandDisappear(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_objectInstance_execCommandDisappearObjectRecover(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_objectInstance_execCommandDisable(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_objectInstance_execCommandDisableObjectEnable(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_objectInstance_execCommandObjectFilterEffect(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_objectInstance_execCommandObjectFilterEffectRemove(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_objectInstance_execCommandSceneEffect(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_objectInstance_execCommandSceneEffectRemove(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_objectInstance_execCommandSceneGravityChange(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_objectInstance_execCommandSceneRotateFlip(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_objectInstance_execCommandCameraAreaChange(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_objectInstance_execCommandSoundPlay(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_objectInstance_execCommandSoundStop(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_objectInstance_execCommandSoundPositionRemember(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_objectInstance_execCommandMessageShow(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_objectInstance_execCommandScrollMessageShow(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_objectInstance_execCommandEffectShow(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_objectInstance_execCommandEffectRemove(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_objectInstance_execCommandParticleShow(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_objectInstance_execCommandParticleRemove(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_objectInstance_execCommandMovieShow(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_objectInstance_execCommandImageShow(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_objectInstance_execCommandSwitchVariableChange(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_objectInstance_execCommandSwitchVariableReset(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_objectInstance_execCommandGameSpeedChange(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_objectInstance_execCommandTimer(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_objectInstance_execCommandSceneTerminate(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_objectInstance_execCommandDirectionMove(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_objectInstance_execCommandForthBackMoveTurn(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_objectInstance_execCommandActionExec(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_objectInstance_execCommandSceneShake(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_objectInstance_execCommandLayerHide(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_objectInstance_execCommandLayerShow(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_objectInstance_execCommandLayerDisable(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_objectInstance_execCommandLayerEnable(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_objectInstance_execCommandMenuShow(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_objectInstance_execCommandMenuHide(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_objectInstance_execCommandDisplayDirectionMove(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_objectInstance_execCommandFileLoad(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_objectInstance_execCommandObjectUnlock(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_objectInstance_execCommandResourceSetChange(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_objectInstance_execCommandDatabaseReflect(JSContext *cx, unsigned argc, JS::Value *vp);

static bool js_objectInstance_isWallTouched(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_objectInstance_isWallAhead(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_objectInstance_isObjectWallTouched(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_objectInstance_isAttackAreaTouched(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_objectInstance_isAttackAreaNear(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_objectInstance_isObjectNear(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_objectInstance_isObjectFacingEachOther(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_objectInstance_isObjectFacing(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_objectInstance_isObjectFound(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_objectInstance_isObjectFacingDirection(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_objectInstance_isHpZero(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_objectInstance_isCameraOutOfRange(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_objectInstance_isLocked(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_objectInstance_isProbability(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_objectInstance_isSwitchVariableChanged(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_objectInstance_isAnimationFinished(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_objectInstance_isJumpTop(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_objectInstance_isObjectActionChanged(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_objectInstance_isSlopeTouched(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_objectInstance_isBuriedInWall(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_objectInstance_isObjectHit(JSContext *cx, unsigned argc, JS::Value *vp);


//switches
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_7
static bool jsb_register_agtk_objectInstanceBase_switches(JSContext *cx, JSObject *agtkObj);
#endif
static bool jsb_register_agtk_objectInstance_switches(JSContext *cx, JSObject *agtkObj);
//switches
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_7
static bool jsb_register_agtk_switchBase(JSContext *cx, JSObject *agtkObj);
#endif
static bool js_switches_get(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_switches_getIdByName(JSContext *cx, unsigned argc, JS::Value *vp);
//switch
static bool js_switch_getValue(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_switch_setValue(JSContext *cx, unsigned argc, JS::Value *vp);

//variables
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_7
extern bool jsb_register_agtk_objectInstanceBase_variables(JSContext *_cx, JSObject *agtkObj);
#endif
extern bool jsb_register_agtk_objectInstance_variables(JSContext *_cx, JSObject *agtkObj);
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_7
static bool jsb_register_agtk_variableBase(JSContext *cx, JSObject *agtkObj);
#endif
static bool js_variables_get(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_variables_getIdByName(JSContext *cx, unsigned argc, JS::Value *vp);
//variable
static bool js_variable_getValue(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_variable_setValue(JSContext *cx, unsigned argc, JS::Value *vp);

#include "Lib/ObjectCommand.h"
#include "Data/ObjectCommandData.h"
#include "Manager/DebugManager.h"
#include "Data/PlayData.h"
#include "JavascriptManager.h"

// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_7
// objectInstances
enum {
	ObjectInstanceId,
	ObjectInstanceName,
	ObjectLayerId,
	ObjectLayerIndex,
};
#endif

void jsb_register_agtk_objectInstances(JSContext *cx, JS::HandleObject object)
{
	auto ret = register_agtk_objectInstances(cx, object);
	CCASSERT(ret, "Failed to register_agtk_objectInstances");
}

bool register_agtk_objectInstances(JSContext *cx, JS::HandleObject object)
{
	JS::RootedObject robj(cx, object);

	static JSClass objectInstances_class = {
		"objectInstances",
		0,
		/* All of these can be replaced with the corresponding JS_*Stub
		function pointers. */
		JS_PropertyStub,
		JS_DeletePropertyStub,
		JS_PropertyStub,
		JS_StrictPropertyStub,
		JS_EnumerateStub,
		JS_ResolveStub,
		JS_ConvertStub,
		nullptr, nullptr, nullptr, nullptr, nullptr
	};
	auto objectInstances = JS_DefineObject(cx, robj, "objectInstances", &objectInstances_class, JS::NullPtr(), JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE);
	CCASSERT(objectInstances, "objectInstances should not be nullptr");
	JS::RootedObject robjectInstances(cx, objectInstances);
	static JSFunctionSpec objectInstances_methods[] = {
		{ "get", js_objectInstances_get, 0, 0, 0 },
		{ "getIdByName", js_objectInstances_getIdByName, 0, 0, 0 },
		{ NULL },
	};
	JS_DefineFunctions(cx, robjectInstances, objectInstances_methods);

	JS::RootedValue rval(cx);
	rval.setObject(*objectInstances);
	JS_SetProperty(cx, robj, "objectInstances", rval);

// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_7
	static JSClass objectInstanceBase_class = {
		"objectInstanceBase",
		0,
		/* All of these can be replaced with the corresponding JS_*Stub
		function pointers. */
		JS_PropertyStub,
		JS_DeletePropertyStub,
		JS_PropertyStub,
		JS_StrictPropertyStub,
		JS_EnumerateStub,
		JS_ResolveStub,
		JS_ConvertStub,
		nullptr, nullptr, nullptr, nullptr, nullptr
	};
	auto objectInstanceBase = JS_DefineObject(cx, robj, "objectInstanceBase", &objectInstanceBase_class, JS::NullPtr(), JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE);
	CCASSERT(objectInstanceBase, "objectInstanceBase should not be nullptr");
	JS::RootedObject robjectInstanceBase(cx, objectInstanceBase);
#if 1
	static JSFunctionSpec objectInstance_methods[] = {
		{ "getAttackerInstanceIdList", js_objectInstance_getAttackerInstanceIdList, 0, 0, 0 },
		{ "execCommandTemplateMove", js_objectInstance_execCommandTemplateMove, 0, 0, 0 },
		{ "execCommandObjectLock", js_objectInstance_execCommandObjectLock, 0, 0, 0 },
		{ "execCommandObjectCreate", js_objectInstance_execCommandObjectCreate, 0, 0, 0 },
		{ "execCommandObjectChange", js_objectInstance_execCommandObjectChange, 0, 0, 0 },
		{ "execCommandObjectMove", js_objectInstance_execCommandObjectMove, 0, 0, 0 },
		{ "execCommandObjectPushPull", js_objectInstance_execCommandObjectPushPull, 0, 0, 0 },
		{ "execCommandLayerMove", js_objectInstance_execCommandLayerMove, 0, 0, 0 },
		{ "execCommandAttackSetting", js_objectInstance_execCommandAttackSetting, 0, 0, 0 },
		{ "execCommandBulletFire", js_objectInstance_execCommandBulletFire, 0, 0, 0 },
		{ "execCommandDisappear", js_objectInstance_execCommandDisappear, 0, 0, 0 },
		{ "execCommandDisappearObjectRecover", js_objectInstance_execCommandDisappearObjectRecover, 0, 0, 0 },
		{ "execCommandDisable", js_objectInstance_execCommandDisable, 0, 0, 0 },
		{ "execCommandDisableObjectEnable", js_objectInstance_execCommandDisableObjectEnable, 0, 0, 0 },
		{ "execCommandObjectFilterEffect", js_objectInstance_execCommandObjectFilterEffect, 0, 0, 0 },
		{ "execCommandObjectFilterEffectRemove", js_objectInstance_execCommandObjectFilterEffectRemove, 0, 0, 0 },
		{ "execCommandSceneEffect", js_objectInstance_execCommandSceneEffect, 0, 0, 0 },
		{ "execCommandSceneEffectRemove", js_objectInstance_execCommandSceneEffectRemove, 0, 0, 0 },
		{ "execCommandSceneGravityChange", js_objectInstance_execCommandSceneGravityChange, 0, 0, 0 },
		{ "execCommandSceneRotateFlip", js_objectInstance_execCommandSceneRotateFlip, 0, 0, 0 },
		{ "execCommandCameraAreaChange", js_objectInstance_execCommandCameraAreaChange, 0, 0, 0 },
		{ "execCommandSoundPlay", js_objectInstance_execCommandSoundPlay, 0, 0, 0 },
		{ "execCommandSoundStop", js_objectInstance_execCommandSoundStop, 0, 0, 0 },
		{ "execCommandSoundPositionRemember", js_objectInstance_execCommandSoundPositionRemember, 0, 0, 0 },
		{ "execCommandMessageShow", js_objectInstance_execCommandMessageShow, 0, 0, 0 },
		{ "execCommandScrollMessageShow", js_objectInstance_execCommandScrollMessageShow, 0, 0, 0 },
		{ "execCommandEffectShow", js_objectInstance_execCommandEffectShow, 0, 0, 0 },
		{ "execCommandEffectRemove", js_objectInstance_execCommandEffectRemove, 0, 0, 0 },
		{ "execCommandParticleShow", js_objectInstance_execCommandParticleShow, 0, 0, 0 },
		{ "execCommandParticleRemove", js_objectInstance_execCommandParticleRemove, 0, 0, 0 },
		{ "execCommandMovieShow", js_objectInstance_execCommandMovieShow, 0, 0, 0 },
		{ "execCommandImageShow", js_objectInstance_execCommandImageShow, 0, 0, 0 },
		{ "execCommandSwitchVariableChange", js_objectInstance_execCommandSwitchVariableChange, 0, 0, 0 },
		{ "execCommandSwitchVariableReset", js_objectInstance_execCommandSwitchVariableReset, 0, 0, 0 },
		{ "execCommandGameSpeedChange", js_objectInstance_execCommandGameSpeedChange, 0, 0, 0 },
		{ "execCommandTimer", js_objectInstance_execCommandTimer, 0, 0, 0 },
		{ "execCommandSceneTerminate", js_objectInstance_execCommandSceneTerminate, 0, 0, 0 },
		{ "execCommandDirectionMove", js_objectInstance_execCommandDirectionMove, 0, 0, 0 },
		{ "execCommandForthBackMoveTurn", js_objectInstance_execCommandForthBackMoveTurn, 0, 0, 0 },
		{ "execCommandActionExec", js_objectInstance_execCommandActionExec, 0, 0, 0 },
		{ "execCommandSceneShake", js_objectInstance_execCommandSceneShake, 0, 0, 0 },
		{ "execCommandLayerHide", js_objectInstance_execCommandLayerHide, 0, 0, 0 },
		{ "execCommandLayerShow", js_objectInstance_execCommandLayerShow, 0, 0, 0 },
		{ "execCommandLayerDisable", js_objectInstance_execCommandLayerDisable, 0, 0, 0 },
		{ "execCommandLayerEnable", js_objectInstance_execCommandLayerEnable, 0, 0, 0 },
		{ "execCommandMenuShow", js_objectInstance_execCommandMenuShow, 0, 0, 0 },
		{ "execCommandMenuHide", js_objectInstance_execCommandMenuHide, 0, 0, 0 },
		{ "execCommandDisplayDirectionMove", js_objectInstance_execCommandDisplayDirectionMove, 0, 0, 0 },
		{ "execCommandFileLoad", js_objectInstance_execCommandFileLoad, 0, 0, 0 },
		{ "execCommandObjectUnlock", js_objectInstance_execCommandObjectUnlock, 0, 0, 0 },
		{ "execCommandResourceSetChange", js_objectInstance_execCommandResourceSetChange, 0, 0, 0 },
		{ "execCommandDatabaseReflect", js_objectInstance_execCommandDatabaseReflect, 0, 0, 0 },
		{ "isWallTouched", js_objectInstance_isWallTouched, 0, 0, 0 },
		{ "isWallAhead", js_objectInstance_isWallAhead, 0, 0, 0 },
		{ "isObjectWallTouched", js_objectInstance_isObjectWallTouched, 0, 0, 0 },
		{ "isAttackAreaTouched", js_objectInstance_isAttackAreaTouched, 0, 0, 0 },
		{ "isAttackAreaNear", js_objectInstance_isAttackAreaNear, 0, 0, 0 },
		{ "isObjectNear", js_objectInstance_isObjectNear, 0, 0, 0 },
		{ "isObjectFacingEachOther", js_objectInstance_isObjectFacingEachOther, 0, 0, 0 },
		{ "isObjectFacing", js_objectInstance_isObjectFacing, 0, 0, 0 },
		{ "isObjectFound", js_objectInstance_isObjectFound, 0, 0, 0 },
		{ "isObjectFacingDirection", js_objectInstance_isObjectFacingDirection, 0, 0, 0 },
		{ "isHpZero", js_objectInstance_isHpZero, 0, 0, 0 },
		{ "isCameraOutOfRange", js_objectInstance_isCameraOutOfRange, 0, 0, 0 },
		{ "isLocked", js_objectInstance_isLocked, 0, 0, 0 },
		{ "isProbability", js_objectInstance_isProbability, 0, 0, 0 },
		{ "isSwitchVariableChanged", js_objectInstance_isSwitchVariableChanged, 0, 0, 0 },
		{ "isAnimationFinished", js_objectInstance_isAnimationFinished, 0, 0, 0 },
		{ "isJumpTop", js_objectInstance_isJumpTop, 0, 0, 0 },
		{ "isObjectActionChanged", js_objectInstance_isObjectActionChanged, 0, 0, 0 },
		{ "isSlopeTouched", js_objectInstance_isSlopeTouched, 0, 0, 0 },
		{ "isBuriedInWall", js_objectInstance_isBuriedInWall, 0, 0, 0 },
		{ "isObjectHit", js_objectInstance_isObjectHit, 0, 0, 0 },
		{ NULL },
	};
	JS_DefineFunctions(cx, robjectInstanceBase, objectInstance_methods);
#endif
	JS_DefineProperty(cx, robjectInstanceBase, "layerId", ObjectLayerId, JSPROP_ENUMERATE, js_objectInstance_getProperty, js_objectInstance_setProperty);
	JS_DefineProperty(cx, robjectInstanceBase, "layerIndex", ObjectLayerIndex, JSPROP_ENUMERATE, js_objectInstance_getProperty, js_objectInstance_setProperty);
	rval.setObject(*objectInstanceBase);
	JS_SetProperty(cx, robj, "objectInstanceBase", rval);

	if (!jsb_register_agtk_objectInstanceBase_switches(cx, object.get())) return false;
	if (!jsb_register_agtk_objectInstanceBase_variables(cx, object.get())) return false;

	if (!jsb_register_agtk_switchBase(cx, object.get())) return false;
	if (!jsb_register_agtk_variableBase(cx, object.get())) return false;
#endif
	return true;
}

// #AGTK-NX
#ifndef USE_SAR_OPTIMIZE_7
// objectInstances
enum {
	ObjectInstanceId,
	ObjectInstanceName,
	ObjectLayerId,
	ObjectLayerIndex,
};
#endif
bool js_objectInstances_get(JSContext *cx, unsigned argc, JS::Value *vp)
{
	JS::CallArgs args = CallArgsFromVp(argc, vp);
    args.rval().set(JS::NullValue());
	auto len = args.length();
	if (len < 1) {
		return false;
	}
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
	auto &v = args[0];
#else
#endif
	if (!v.isNumber()) {
		return false;
	}
	auto objectInstanceId = JavascriptManager::getInt32(v);
	auto scene = GameManager::getInstance()->getCurrentScene();
	agtk::Object *objectInstance = scene->getObjectInstance(-1, objectInstanceId);
	if (!objectInstance) {
		return true;
	}
	static JSClass objectInstance_class = {
		"objectInstance",
		0,
		/* All of these can be replaced with the corresponding JS_*Stub
		function pointers. */
		JS_PropertyStub,
		JS_DeletePropertyStub,
		JS_PropertyStub,
		JS_StrictPropertyStub,
		JS_EnumerateStub,
		JS_ResolveStub,
		JS_ConvertStub,
		nullptr, nullptr, nullptr, nullptr, nullptr
	};
	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_7
	auto sc = ScriptingCore::getInstance();
	auto _global = sc->getGlobalObject();
	JS::RootedObject gobj(cx, _global);
	JSAutoCompartment ac(cx, gobj);

	JS::RootedObject ns(cx);
	JS::MutableHandleObject jsObj = &ns;
	JS::RootedValue nsval(cx);
	JS_GetProperty(cx, gobj, "Agtk", &nsval);
	JS::RootedValue rv(cx);
	bool ret = false;
	if (nsval == JSVAL_VOID) {
		return false;
	}
	jsObj.set(nsval.toObjectOrNull());
	JS::RootedValue robjectInstanceBaseVal(cx);
	JS_GetProperty(cx, jsObj, "objectInstanceBase", &robjectInstanceBaseVal);
	if (!robjectInstanceBaseVal.isObject()) {
		return false;
	}

	JSObject* objectInstanceBase = robjectInstanceBaseVal.get().toObjectOrNull();
	JS::RootedObject robjectInstanceBase(cx, objectInstanceBase);

	auto objectInstanceObj = JS_DefineObject(cx, jsthis, "objectInstance", &objectInstance_class, robjectInstanceBase, JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE);
	CCASSERT(objectInstanceObj, "objectInstanceObj should not be nullptr");
	JS::RootedObject robjectInstance(cx, objectInstanceObj);
	JS::RootedValue robjectInstanceId(cx, JS::Int32Value(objectInstanceId));
	JS_DefineProperty(cx, robjectInstance, "id", robjectInstanceId, JSPROP_ENUMERATE | JSPROP_PERMANENT);
	JS::RootedValue rval(cx);
	rval.setInt32(objectInstance->getObjectData()->getId());
	JS_DefineProperty(cx, robjectInstance, "objectId", rval, JSPROP_ENUMERATE | JSPROP_PERMANENT);
	if (!jsb_register_agtk_objectInstance_switches(cx, objectInstanceObj)) return false;
	if (!jsb_register_agtk_objectInstance_variables(cx, objectInstanceObj)) return false;
#else
	auto objectInstanceObj = JS_DefineObject(cx, jsthis, "objectInstance", &objectInstance_class, JS::NullPtr(), JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE);
	CCASSERT(objectInstanceObj, "objectInstanceObj should not be nullptr");
	JS::RootedObject robjectInstance(cx, objectInstanceObj);
#if 1
	static JSFunctionSpec objectInstance_methods[] = {
		{ "getAttackerInstanceIdList", js_objectInstance_getAttackerInstanceIdList, 0, 0, 0 },
		{ "execCommandTemplateMove", js_objectInstance_execCommandTemplateMove, 0, 0, 0 },
		{ "execCommandObjectLock", js_objectInstance_execCommandObjectLock, 0, 0, 0 },
		{ "execCommandObjectCreate", js_objectInstance_execCommandObjectCreate, 0, 0, 0 },
		{ "execCommandObjectChange", js_objectInstance_execCommandObjectChange, 0, 0, 0 },
		{ "execCommandObjectMove", js_objectInstance_execCommandObjectMove, 0, 0, 0 },
		{ "execCommandObjectPushPull", js_objectInstance_execCommandObjectPushPull, 0, 0, 0 },
		{ "execCommandLayerMove", js_objectInstance_execCommandLayerMove, 0, 0, 0 },
		{ "execCommandAttackSetting", js_objectInstance_execCommandAttackSetting, 0, 0, 0 },
		{ "execCommandBulletFire", js_objectInstance_execCommandBulletFire, 0, 0, 0 },
		{ "execCommandDisappear", js_objectInstance_execCommandDisappear, 0, 0, 0 },
		{ "execCommandDisappearObjectRecover", js_objectInstance_execCommandDisappearObjectRecover, 0, 0, 0 },
		{ "execCommandDisable", js_objectInstance_execCommandDisable, 0, 0, 0 },
		{ "execCommandDisableObjectEnable", js_objectInstance_execCommandDisableObjectEnable, 0, 0, 0 },
		{ "execCommandObjectFilterEffect", js_objectInstance_execCommandObjectFilterEffect, 0, 0, 0 },
		{ "execCommandObjectFilterEffectRemove", js_objectInstance_execCommandObjectFilterEffectRemove, 0, 0, 0 },
		{ "execCommandSceneEffect", js_objectInstance_execCommandSceneEffect, 0, 0, 0 },
		{ "execCommandSceneEffectRemove", js_objectInstance_execCommandSceneEffectRemove, 0, 0, 0 },
		{ "execCommandSceneGravityChange", js_objectInstance_execCommandSceneGravityChange, 0, 0, 0 },
		{ "execCommandSceneRotateFlip", js_objectInstance_execCommandSceneRotateFlip, 0, 0, 0 },
		{ "execCommandCameraAreaChange", js_objectInstance_execCommandCameraAreaChange, 0, 0, 0 },
		{ "execCommandSoundPlay", js_objectInstance_execCommandSoundPlay, 0, 0, 0 },
		{ "execCommandSoundStop", js_objectInstance_execCommandSoundStop, 0, 0, 0 },
		{ "execCommandSoundPositionRemember", js_objectInstance_execCommandSoundPositionRemember, 0, 0, 0 },
		{ "execCommandMessageShow", js_objectInstance_execCommandMessageShow, 0, 0, 0 },
		{ "execCommandScrollMessageShow", js_objectInstance_execCommandScrollMessageShow, 0, 0, 0 },
		{ "execCommandEffectShow", js_objectInstance_execCommandEffectShow, 0, 0, 0 },
		{ "execCommandEffectRemove", js_objectInstance_execCommandEffectRemove, 0, 0, 0 },
		{ "execCommandParticleShow", js_objectInstance_execCommandParticleShow, 0, 0, 0 },
		{ "execCommandParticleRemove", js_objectInstance_execCommandParticleRemove, 0, 0, 0 },
		{ "execCommandMovieShow", js_objectInstance_execCommandMovieShow, 0, 0, 0 },
		{ "execCommandImageShow", js_objectInstance_execCommandImageShow, 0, 0, 0 },
		{ "execCommandSwitchVariableChange", js_objectInstance_execCommandSwitchVariableChange, 0, 0, 0 },
		{ "execCommandSwitchVariableReset", js_objectInstance_execCommandSwitchVariableReset, 0, 0, 0 },
		{ "execCommandGameSpeedChange", js_objectInstance_execCommandGameSpeedChange, 0, 0, 0 },
		{ "execCommandTimer", js_objectInstance_execCommandTimer, 0, 0, 0 },
		{ "execCommandSceneTerminate", js_objectInstance_execCommandSceneTerminate, 0, 0, 0 },
		{ "execCommandDirectionMove", js_objectInstance_execCommandDirectionMove, 0, 0, 0 },
		{ "execCommandForthBackMoveTurn", js_objectInstance_execCommandForthBackMoveTurn, 0, 0, 0 },
		{ "execCommandActionExec", js_objectInstance_execCommandActionExec, 0, 0, 0 },
		{ "execCommandSceneShake", js_objectInstance_execCommandSceneShake, 0, 0, 0 },
		{ "execCommandLayerHide", js_objectInstance_execCommandLayerHide, 0, 0, 0 },
		{ "execCommandLayerShow", js_objectInstance_execCommandLayerShow, 0, 0, 0 },
		{ "execCommandLayerDisable", js_objectInstance_execCommandLayerDisable, 0, 0, 0 },
		{ "execCommandLayerEnable", js_objectInstance_execCommandLayerEnable, 0, 0, 0 },
		{ "execCommandMenuShow", js_objectInstance_execCommandMenuShow, 0, 0, 0 },
		{ "execCommandMenuHide", js_objectInstance_execCommandMenuHide, 0, 0, 0 },
		{ "execCommandDisplayDirectionMove", js_objectInstance_execCommandDisplayDirectionMove, 0, 0, 0 },
		{ "execCommandFileLoad", js_objectInstance_execCommandFileLoad, 0, 0, 0 },
		{ "execCommandObjectUnlock", js_objectInstance_execCommandObjectUnlock, 0, 0, 0 },
		{ "execCommandResourceSetChange", js_objectInstance_execCommandResourceSetChange, 0, 0, 0 },
		{ "execCommandDatabaseReflect", js_objectInstance_execCommandDatabaseReflect, 0, 0, 0 },
		{ "isWallTouched", js_objectInstance_isWallTouched, 0, 0, 0 },
		{ "isWallAhead", js_objectInstance_isWallAhead, 0, 0, 0 },
		{ "isObjectWallTouched", js_objectInstance_isObjectWallTouched, 0, 0, 0 },
		{ "isAttackAreaTouched", js_objectInstance_isAttackAreaTouched, 0, 0, 0 },
		{ "isAttackAreaNear", js_objectInstance_isAttackAreaNear, 0, 0, 0 },
		{ "isObjectNear", js_objectInstance_isObjectNear, 0, 0, 0 },
		{ "isObjectFacingEachOther", js_objectInstance_isObjectFacingEachOther, 0, 0, 0 },
		{ "isObjectFacing", js_objectInstance_isObjectFacing, 0, 0, 0 },
		{ "isObjectFound", js_objectInstance_isObjectFound, 0, 0, 0 },
		{ "isObjectFacingDirection", js_objectInstance_isObjectFacingDirection, 0, 0, 0 },
		{ "isHpZero", js_objectInstance_isHpZero, 0, 0, 0 },
		{ "isCameraOutOfRange", js_objectInstance_isCameraOutOfRange, 0, 0, 0 },
		{ "isLocked", js_objectInstance_isLocked, 0, 0, 0 },
		{ "isProbability", js_objectInstance_isProbability, 0, 0, 0 },
		{ "isSwitchVariableChanged", js_objectInstance_isSwitchVariableChanged, 0, 0, 0 },
		{ "isAnimationFinished", js_objectInstance_isAnimationFinished, 0, 0, 0 },
		{ "isJumpTop", js_objectInstance_isJumpTop, 0, 0, 0 },
		{ "isObjectActionChanged", js_objectInstance_isObjectActionChanged, 0, 0, 0 },
		{ "isSlopeTouched", js_objectInstance_isSlopeTouched, 0, 0, 0 },
		{ "isBuriedInWall", js_objectInstance_isBuriedInWall, 0, 0, 0 },
		{ "isObjectHit", js_objectInstance_isObjectHit, 0, 0, 0 },
		{ NULL },
	};
	JS_DefineFunctions(cx, robjectInstance, objectInstance_methods);
#endif
	JS::RootedValue robjectInstanceId(cx, JS::Int32Value(objectInstanceId));
	JS_DefineProperty(cx, robjectInstance, "id", robjectInstanceId, JSPROP_ENUMERATE | JSPROP_PERMANENT);
	JS::RootedValue rval(cx);
	rval.setInt32(objectInstance->getObjectData()->getId());
	JS_DefineProperty(cx, robjectInstance, "objectId", rval, JSPROP_ENUMERATE | JSPROP_PERMANENT);
	JS_DefineProperty(cx, robjectInstance, "layerId", ObjectLayerId, JSPROP_ENUMERATE, js_objectInstance_getProperty, js_objectInstance_setProperty);
	JS_DefineProperty(cx, robjectInstance, "layerIndex", ObjectLayerIndex, JSPROP_ENUMERATE, js_objectInstance_getProperty, js_objectInstance_setProperty);
	//rval.setInt32(objectInstanceId);
	//JS_SetProperty(cx, robjectInstance, "objectInstanceId", rval);
	//JS_SetProperty(cx, robjectInstance, "objectId", rval);
	if (!jsb_register_agtk_objectInstance_switches(cx, objectInstanceObj)) return false;
	if (!jsb_register_agtk_objectInstance_variables(cx, objectInstanceObj)) return false;
#endif
	args.rval().set(OBJECT_TO_JSVAL(objectInstanceObj));
	return true;
}

bool js_objectInstances_getIdByName(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//arg1: <string: オブジェクトインスタンス名>
	//ret: <int : オブジェクトインスタンスID> or null
	JS::CallArgs args = CallArgsFromVp(argc, vp);
    args.rval().set(JS::Int32Value(-1));
	auto len = args.length();
	if (len < 2) {
		return false;
	}
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
	auto &v = args[0];
#else
#endif
	if (!v.isNumber()) {
		return false;
	}
	auto objectId = JavascriptManager::getInt32(v);
	std::string objectInstanceName;
	{
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
		auto &v2 = args[1];
#else
#endif
		if (!v2.isString()) {
			return false;
		}
		if (!jsval_to_std_string(cx, v2, &objectInstanceName)) {
			return false;
		}
	}

	int value = -1;
	auto scene = GameManager::getInstance()->getCurrentScene();
	agtk::Object *objectInstance = scene->getObjectInstanceByName(objectId, objectInstanceName.c_str());

	if(objectInstance){
		value = objectInstance->getInstanceId();
	}
	args.rval().set(JS::Int32Value(value));
	return true;
}

// objectInstance
bool js_objectInstance_getProperty(JSContext *cx, JS::HandleObject obj, JS::HandleId id, JS::MutableHandleValue vp)
{
	JS::RootedValue v(cx);
	vp.set(JS::NullValue());

	JS::RootedObject robj(cx, obj);
	JS_GetProperty(cx, robj, "id", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto objectInstanceId = JavascriptManager::getInt32(v);
	auto scene = GameManager::getInstance()->getCurrentScene();
	agtk::Object *objectInstance = scene->getObjectInstance(-1, objectInstanceId);
	if (!objectInstance) {
		return true;
	}

	JSString* jss = JSID_TO_STRING(id);
	char *p = JS_EncodeString(cx, jss);
	if (strcmp(p, "layerId") == 0) {
		vp.setInt32(objectInstance->getLayerId());
	}
	else if (strcmp(p, "layerIndex") == 0) {
		auto layerId = objectInstance->getLayerId();
		auto sceneInstance = GameManager::getInstance()->getCurrentScene();
		if (!sceneInstance) {
			return false;
		}
		auto layerList = sceneInstance->getSceneLayerList();
		agtk::SceneLayer *sceneLayer = nullptr;
		cocos2d::DictElement *el = nullptr;
		int index = 0;
		CCDICT_FOREACH(layerList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto p = static_cast<agtk::SceneLayer *>(el->getObject());
#else
			auto p = dynamic_cast<agtk::SceneLayer *>(el->getObject());
#endif
			if (p->getLayerId() == layerId) {
				break;
			}
			index++;
		}
		if (index >= layerList->count()) {
			return false;
		}
		vp.setInt32(layerList->count() - 1 - index);
	}
	else {
		CCLOG("Unimplemented property getter: %s", p);
		return false;
	}
	return true;
}

bool js_objectInstance_setProperty(JSContext *cx, JS::HandleObject obj, JS::HandleId id, bool strict, JS::MutableHandleValue vp)
{
	JS::RootedValue v(cx);

	JS::RootedObject robj(cx, obj);
	JS_GetProperty(cx, robj, "id", &v);
	if (!v.isNumber()) {
		vp.set(JS::NullValue());
		return false;
	}
	auto objectInstanceId = JavascriptManager::getInt32(v);
	auto scene = GameManager::getInstance()->getCurrentScene();
	agtk::Object *objectInstance = scene->getObjectInstance(-1, objectInstanceId);
	if (!objectInstance) {
		vp.set(JS::NullValue());
		return false;
	}

	JSString* jss = JSID_TO_STRING(id);
	char *p = JS_EncodeString(cx, jss);
	/*if (strcmp(p, "operatable") == 0) {
	bool operatable = false;
	if (vp.isBoolean()) {
	operatable = vp.toBoolean();
	}
	else if (vp.isInt32()) {
	operatable = vp.toInt32() != 0;
	}
	else if (vp.isDouble()) {
	operatable = vp.toDouble() != 0;
	}
	else {
	vp.set(JS::NullValue());
	return false;
	}
	objectData->setOperatable(operatable);
	}
	else*/
	{
		CCLOG("Unimplemented property setter: %s", p);
		vp.set(JS::NullValue());
		return false;
	}
	return true;
}

bool js_objectInstance_getAttackerInstanceIdList(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//ret: 実行に成功したか
	JS::CallArgs args = CallArgsFromVp(argc, vp);
	args.rval().set(JS::NullValue());

	JS::RootedValue v(cx);
	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
	JS_GetProperty(cx, jsthis, "id", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto instanceId = JavascriptManager::getInt32(v);
	JS_GetProperty(cx, jsthis, "objectId", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto objectId = JavascriptManager::getInt32(v);
	auto scene = GameManager::getInstance()->getCurrentScene();
	agtk::Object *objectInstance = scene->getObjectInstance(objectId, instanceId);
	if (!objectInstance) {
		return true;
	}

	std::list<int> idList;
	cocos2d::Ref *ref = nullptr;
	CCARRAY_FOREACH(objectInstance->getAttackerObjectInstanceIdList(), ref) {
		auto p = dynamic_cast<cocos2d::Integer *>(ref);
		idList.push_back(p->getValue());
	}

	JS::RootedObject jsarr(cx, JS_NewArrayObject(cx, idList.size()));
	uint32_t i = 0;
	for (auto id : idList) {
		JS::RootedValue element(cx, JS::Int32Value(id));
		JS_SetElement(cx, jsarr, i, element);
		i++;
	}

	args.rval().set(OBJECT_TO_JSVAL(jsarr));
	return true;
}

bool js_objectInstance_execCommandTemplateMove(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//ret: 実行に成功したか
	JS::CallArgs args = CallArgsFromVp(argc, vp);
    args.rval().set(JS::NullValue());
	auto len = args.length();
	if (len < 1) {
		return false;
	}
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
	auto &v2 = args[0];
#else
#endif
	if (!v2.isObject()) {
		return false;
	}
	auto &params = v2.toObject();
	JS::RootedObject rparams(cx, &params);

	JS::RootedValue v(cx);
	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
	JS_GetProperty(cx, jsthis, "id", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto instanceId = JavascriptManager::getInt32(v);
	JS_GetProperty(cx, jsthis, "objectId", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto objectId = JavascriptManager::getInt32(v);
	auto scene = GameManager::getInstance()->getCurrentScene();
	agtk::Object *objectInstance = scene->getObjectInstance(objectId, instanceId);
	if (!objectInstance) {
		return true;
	}

	auto commandData = agtk::data::ObjectCommandTemplateMoveData::create(cx, &params);
	auto currentObjectAction = objectInstance->getCurrentObjectAction();
	currentObjectAction->execActionTemplateMove(commandData);

	args.rval().set(JS::Int32Value(agtk::ObjectAction::kCommandBehaviorNext));
	return true;
}

bool js_objectInstance_execCommandObjectLock(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//ret: 実行に成功したか
	JS::CallArgs args = CallArgsFromVp(argc, vp);
    args.rval().set(JS::NullValue());
	auto len = args.length();
	if (len < 1) {
		return false;
	}
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
	auto &v2 = args[0];
#else
#endif
	if (!v2.isObject()) {
		return false;
	}
	auto &params = v2.toObject();
	JS::RootedObject rparams(cx, &params);

	JS::RootedValue v(cx);
	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
	JS_GetProperty(cx, jsthis, "id", &v);
	if (!v.isNumber()) {
		return false;
}
	auto instanceId = JavascriptManager::getInt32(v);
	JS_GetProperty(cx, jsthis, "objectId", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto objectId = JavascriptManager::getInt32(v);
	auto scene = GameManager::getInstance()->getCurrentScene();
	agtk::Object *objectInstance = scene->getObjectInstance(objectId, instanceId);
	if (!objectInstance) {
		return true;
	}

	auto commandData = agtk::data::ObjectCommandObjectLockData::create(cx, &params);
	auto currentObjectAction = objectInstance->getCurrentObjectAction();
	currentObjectAction->execActionObjectLock(commandData);

	args.rval().set(JS::Int32Value(agtk::ObjectAction::kCommandBehaviorNext));
	return true;
}

bool js_objectInstance_execCommandObjectCreate(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//ret: 実行に成功したか
	JS::CallArgs args = CallArgsFromVp(argc, vp);
    args.rval().set(JS::NullValue());
	auto len = args.length();
	if (len < 1) {
		return false;
	}
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
	auto &v2 = args[0];
#else
#endif
	if (!v2.isObject()) {
		return false;
	}
	auto &params = v2.toObject();
	JS::RootedObject rparams(cx, &params);

	JS::RootedValue v(cx);
	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
	JS_GetProperty(cx, jsthis, "id", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto instanceId = JavascriptManager::getInt32(v);
	JS_GetProperty(cx, jsthis, "objectId", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto objectId = JavascriptManager::getInt32(v);
	auto scene = GameManager::getInstance()->getCurrentScene();
	agtk::Object *objectInstance = scene->getObjectInstance(objectId, instanceId);
	if (!objectInstance) {
		return true;
	}

	auto commandData = agtk::data::ObjectCommandObjectCreateData::create(cx, &params);
	auto currentObjectAction = objectInstance->getCurrentObjectAction();
	currentObjectAction->execActionObjectCreate(commandData);

	args.rval().set(JS::Int32Value(agtk::ObjectAction::kCommandBehaviorNext));
	return true;
}

bool js_objectInstance_execCommandObjectChange(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//ret: 実行に成功したか
	JS::CallArgs args = CallArgsFromVp(argc, vp);
    args.rval().set(JS::NullValue());
	auto len = args.length();
	if (len < 1) {
		return false;
	}
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
	auto &v2 = args[0];
#else
#endif
	if (!v2.isObject()) {
		return false;
	}
	auto &params = v2.toObject();
	JS::RootedObject rparams(cx, &params);

	JS::RootedValue v(cx);
	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
	JS_GetProperty(cx, jsthis, "id", &v);
	if (!v.isNumber()) {
		return false;
    }
	auto instanceId = JavascriptManager::getInt32(v);
	JS_GetProperty(cx, jsthis, "objectId", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto objectId = JavascriptManager::getInt32(v);
	auto scene = GameManager::getInstance()->getCurrentScene();
	agtk::Object *objectInstance = scene->getObjectInstance(objectId, instanceId);
	if (!objectInstance) {
		return true;
	}

	auto commandData = agtk::data::ObjectCommandObjectChangeData::create(cx, &params);
	auto currentObjectAction = objectInstance->getCurrentObjectAction();
	currentObjectAction->execActionObjectChange(commandData);

	args.rval().set(JS::Int32Value(agtk::ObjectAction::kCommandBehaviorNext));
	return true;
}

bool js_objectInstance_execCommandObjectMove(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//ret: 実行に成功したか
	JS::CallArgs args = CallArgsFromVp(argc, vp);
    args.rval().set(JS::NullValue());
	auto len = args.length();
	if (len < 1) {
		return false;
	}
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
	auto &v2 = args[0];
#else
#endif
	if (!v2.isObject()) {
		return false;
	}
	auto &params = v2.toObject();
	JS::RootedObject rparams(cx, &params);

	JS::RootedValue v(cx);
	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
	JS_GetProperty(cx, jsthis, "id", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto instanceId = JavascriptManager::getInt32(v);
	JS_GetProperty(cx, jsthis, "objectId", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto objectId = JavascriptManager::getInt32(v);
	auto scene = GameManager::getInstance()->getCurrentScene();
	agtk::Object *objectInstance = scene->getObjectInstance(objectId, instanceId);
	if (!objectInstance) {
		return true;
	}

	auto commandData = agtk::data::ObjectCommandObjectMoveData::create(cx, &params);
	auto currentObjectAction = objectInstance->getCurrentObjectAction();
	currentObjectAction->execActionObjectMove(commandData);

	args.rval().set(JS::Int32Value(agtk::ObjectAction::kCommandBehaviorNext));
	return true;
}

bool js_objectInstance_execCommandObjectPushPull(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//ret: 実行に成功したか
	JS::CallArgs args = CallArgsFromVp(argc, vp);
    args.rval().set(JS::NullValue());
	auto len = args.length();
	if (len < 1) {
		return false;
	}
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
	auto &v2 = args[0];
#else
#endif
	if (!v2.isObject()) {
		return false;
	}
	auto &params = v2.toObject();
	JS::RootedObject rparams(cx, &params);

	JS::RootedValue v(cx);
	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
	JS_GetProperty(cx, jsthis, "id", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto instanceId = JavascriptManager::getInt32(v);
	JS_GetProperty(cx, jsthis, "objectId", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto objectId = JavascriptManager::getInt32(v);
	auto scene = GameManager::getInstance()->getCurrentScene();
	agtk::Object *objectInstance = scene->getObjectInstance(objectId, instanceId);
	if (!objectInstance) {
		return true;
	}

	auto commandData = agtk::data::ObjectCommandObjectPushPullData::create(cx, &params);
	auto currentObjectAction = objectInstance->getCurrentObjectAction();
	auto bContinuation = currentObjectAction->execActionObjectPushPull(commandData);

	args.rval().set(JS::Int32Value(bContinuation ? agtk::ObjectAction::kCommandBehaviorLoop : agtk::ObjectAction::kCommandBehaviorNext));
	return true;
}

bool js_objectInstance_execCommandLayerMove(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//ret: 実行に成功したか
	JS::CallArgs args = CallArgsFromVp(argc, vp);
    args.rval().set(JS::NullValue());
	auto len = args.length();
	if (len < 1) {
		return false;
	}
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
	auto &v2 = args[0];
#else
#endif
	if (!v2.isObject()) {
		return false;
	}
	auto &params = v2.toObject();
	JS::RootedObject rparams(cx, &params);

	JS::RootedValue v(cx);
	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
	JS_GetProperty(cx, jsthis, "id", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto instanceId = JavascriptManager::getInt32(v);
	JS_GetProperty(cx, jsthis, "objectId", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto objectId = JavascriptManager::getInt32(v);
	auto scene = GameManager::getInstance()->getCurrentScene();
	agtk::Object *objectInstance = scene->getObjectInstance(objectId, instanceId);
	if (!objectInstance) {
		return true;
	}

	auto commandData = agtk::data::ObjectCommandLayerMoveData::create(cx, &params);
	auto currentObjectAction = objectInstance->getCurrentObjectAction();
	currentObjectAction->execActionLayerMove(commandData);

	args.rval().set(JS::Int32Value(agtk::ObjectAction::kCommandBehaviorNext));
	return true;
}

bool js_objectInstance_execCommandAttackSetting(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//ret: 実行に成功したか
	JS::CallArgs args = CallArgsFromVp(argc, vp);
    args.rval().set(JS::NullValue());
	auto len = args.length();
	if (len < 1) {
		return false;
	}
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
	auto &v2 = args[0];
#else
#endif
	if (!v2.isObject()) {
		return false;
	}
	auto &params = v2.toObject();
	JS::RootedObject rparams(cx, &params);

	JS::RootedValue v(cx);
	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
	JS_GetProperty(cx, jsthis, "id", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto instanceId = JavascriptManager::getInt32(v);
	JS_GetProperty(cx, jsthis, "objectId", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto objectId = JavascriptManager::getInt32(v);
	auto scene = GameManager::getInstance()->getCurrentScene();
	agtk::Object *objectInstance = scene->getObjectInstance(objectId, instanceId);
	if (!objectInstance) {
		return true;
	}

	auto commandData = agtk::data::ObjectCommandAttackSettingData::create(cx, &params);
	auto currentObjectAction = objectInstance->getCurrentObjectAction();
	currentObjectAction->execActionAttackSetting(commandData);

	args.rval().set(JS::Int32Value(agtk::ObjectAction::kCommandBehaviorNext));
	return true;
}

bool js_objectInstance_execCommandBulletFire(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//ret: 実行に成功したか
	JS::CallArgs args = CallArgsFromVp(argc, vp);
    args.rval().set(JS::NullValue());
	auto len = args.length();
	if (len < 1) {
		return false;
	}
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
	auto &v2 = args[0];
#else
#endif
	if (!v2.isObject()) {
		return false;
	}
	auto &params = v2.toObject();
	JS::RootedObject rparams(cx, &params);

	JS::RootedValue v(cx);
	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
	JS_GetProperty(cx, jsthis, "id", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto instanceId = JavascriptManager::getInt32(v);
	JS_GetProperty(cx, jsthis, "objectId", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto objectId = JavascriptManager::getInt32(v);
	auto scene = GameManager::getInstance()->getCurrentScene();
	agtk::Object *objectInstance = scene->getObjectInstance(objectId, instanceId);
	if (!objectInstance) {
		return true;
	}

	auto commandData = agtk::data::ObjectCommandBulletFireData::create(cx, &params);
	auto currentObjectAction = objectInstance->getCurrentObjectAction();
	currentObjectAction->execActionBulletFire(commandData);

	args.rval().set(JS::Int32Value(agtk::ObjectAction::kCommandBehaviorNext));
	return true;
}

bool js_objectInstance_execCommandDisappear(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//ret: 実行に成功したか
	JS::CallArgs args = CallArgsFromVp(argc, vp);
    args.rval().set(JS::NullValue());

	JS::RootedValue v(cx);
	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
	JS_GetProperty(cx, jsthis, "id", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto instanceId = JavascriptManager::getInt32(v);
	JS_GetProperty(cx, jsthis, "objectId", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto objectId = JavascriptManager::getInt32(v);
	auto scene = GameManager::getInstance()->getCurrentScene();
	agtk::Object *objectInstance = scene->getObjectInstance(objectId, instanceId);
	if (!objectInstance) {
		return true;
	}

	auto commandData = agtk::data::ObjectCommandDisappearData::create(cx, nullptr);
	auto currentObjectAction = objectInstance->getCurrentObjectAction();
	currentObjectAction->execActionDisappear(commandData);

	args.rval().set(JS::Int32Value(agtk::ObjectAction::kCommandBehaviorNext));
	return true;
}

bool js_objectInstance_execCommandDisappearObjectRecover(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//ret: 実行に成功したか
	JS::CallArgs args = CallArgsFromVp(argc, vp);
    args.rval().set(JS::NullValue());
	auto len = args.length();
	if (len < 1) {
		return false;
	}
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
	auto &v2 = args[0];
#else
#endif
	if (!v2.isObject()) {
		return false;
	}
	auto &params = v2.toObject();
	JS::RootedObject rparams(cx, &params);

	JS::RootedValue v(cx);
	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
	JS_GetProperty(cx, jsthis, "id", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto instanceId = JavascriptManager::getInt32(v);
	JS_GetProperty(cx, jsthis, "objectId", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto objectId = JavascriptManager::getInt32(v);
	auto scene = GameManager::getInstance()->getCurrentScene();
	agtk::Object *objectInstance = scene->getObjectInstance(objectId, instanceId);
	if (!objectInstance) {
		return true;
	}

	auto commandData = agtk::data::ObjectCommandDisappearObjectRecoverData::create(cx, &params);
	auto currentObjectAction = objectInstance->getCurrentObjectAction();
	currentObjectAction->execActionDisappearObjectRecover(commandData);

	args.rval().set(JS::Int32Value(agtk::ObjectAction::kCommandBehaviorNext));
	return true;
}

bool js_objectInstance_execCommandDisable(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//ret: 実行に成功したか
	JS::CallArgs args = CallArgsFromVp(argc, vp);
    args.rval().set(JS::NullValue());
	auto len = args.length();

	JS::RootedValue v(cx);
	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
	JS_GetProperty(cx, jsthis, "id", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto instanceId = JavascriptManager::getInt32(v);
	JS_GetProperty(cx, jsthis, "objectId", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto objectId = JavascriptManager::getInt32(v);
	auto scene = GameManager::getInstance()->getCurrentScene();
	agtk::Object *objectInstance = scene->getObjectInstance(objectId, instanceId);
	if (!objectInstance) {
		return true;
	}

	auto commandData = agtk::data::ObjectCommandDisableData::create(cx, nullptr);
	auto currentObjectAction = objectInstance->getCurrentObjectAction();
	currentObjectAction->execActionDisable(commandData);

	args.rval().set(JS::Int32Value(agtk::ObjectAction::kCommandBehaviorNext));
	return true;
}

bool js_objectInstance_execCommandDisableObjectEnable(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//ret: 実行に成功したか
	JS::CallArgs args = CallArgsFromVp(argc, vp);
    args.rval().set(JS::NullValue());
	auto len = args.length();
	if (len < 1) {
		return false;
	}
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
	auto &v2 = args[0];
#else
#endif
	if (!v2.isObject()) {
		return false;
	}
	auto &params = v2.toObject();
	JS::RootedObject rparams(cx, &params);

	JS::RootedValue v(cx);
	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
	JS_GetProperty(cx, jsthis, "id", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto instanceId = JavascriptManager::getInt32(v);
	JS_GetProperty(cx, jsthis, "objectId", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto objectId = JavascriptManager::getInt32(v);
	auto scene = GameManager::getInstance()->getCurrentScene();
	agtk::Object *objectInstance = scene->getObjectInstance(objectId, instanceId);
	if (!objectInstance) {
		return true;
	}

	auto commandData = agtk::data::ObjectCommandDisableObjectEnableData::create(cx, &params);
	auto currentObjectAction = objectInstance->getCurrentObjectAction();
	currentObjectAction->execActionDisableObjectEnable(commandData);

	args.rval().set(JS::Int32Value(agtk::ObjectAction::kCommandBehaviorNext));
	return true;
}

bool js_objectInstance_execCommandObjectFilterEffect(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//ret: 実行に成功したか
	JS::CallArgs args = CallArgsFromVp(argc, vp);
    args.rval().set(JS::NullValue());
	auto len = args.length();
	if (len < 1) {
		return false;
	}
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
	auto &v2 = args[0];
#else
#endif
	if (!v2.isObject()) {
		return false;
	}
	auto &params = v2.toObject();
	JS::RootedObject rparams(cx, &params);

	JS::RootedValue v(cx);
	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
	JS_GetProperty(cx, jsthis, "id", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto instanceId = JavascriptManager::getInt32(v);
	JS_GetProperty(cx, jsthis, "objectId", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto objectId = JavascriptManager::getInt32(v);
	auto scene = GameManager::getInstance()->getCurrentScene();
	agtk::Object *objectInstance = scene->getObjectInstance(objectId, instanceId);
	if (!objectInstance) {
		return true;
	}

	auto commandData = agtk::data::ObjectCommandObjectFilterEffectData::create(cx, &params);
	auto currentObjectAction = objectInstance->getCurrentObjectAction();
	currentObjectAction->execActionObjectFilterEffect(commandData);

	args.rval().set(JS::Int32Value(agtk::ObjectAction::kCommandBehaviorNext));
	return true;
}


bool js_objectInstance_execCommandObjectFilterEffectRemove(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//ret: 実行に成功したか
	JS::CallArgs args = CallArgsFromVp(argc, vp);
    args.rval().set(JS::NullValue());
	auto len = args.length();
	if (len < 1) {
		return false;
	}
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
	auto &v2 = args[0];
#else
#endif
	if (!v2.isObject()) {
		return false;
	}
	auto &params = v2.toObject();
	JS::RootedObject rparams(cx, &params);

	JS::RootedValue v(cx);
	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
	JS_GetProperty(cx, jsthis, "id", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto instanceId = JavascriptManager::getInt32(v);
	JS_GetProperty(cx, jsthis, "objectId", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto objectId = JavascriptManager::getInt32(v);
	auto scene = GameManager::getInstance()->getCurrentScene();
	agtk::Object *objectInstance = scene->getObjectInstance(objectId, instanceId);
	if (!objectInstance) {
		return true;
	}

	auto commandData = agtk::data::ObjectCommandObjectFilterEffectRemoveData::create(cx, &params);
	auto currentObjectAction = objectInstance->getCurrentObjectAction();
	currentObjectAction->execActionObjectFilterEffectRemove(commandData);

	args.rval().set(JS::Int32Value(agtk::ObjectAction::kCommandBehaviorNext));
	return true;
}



bool js_objectInstance_execCommandSceneEffect(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//ret: 実行に成功したか
	JS::CallArgs args = CallArgsFromVp(argc, vp);
    args.rval().set(JS::NullValue());
	auto len = args.length();
	if (len < 1) {
		return false;
	}
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
	auto &v2 = args[0];
#else
#endif
	if (!v2.isObject()) {
		return false;
	}
	auto &params = v2.toObject();
	JS::RootedObject rparams(cx, &params);

	JS::RootedValue v(cx);
	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
	JS_GetProperty(cx, jsthis, "id", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto instanceId = JavascriptManager::getInt32(v);
	JS_GetProperty(cx, jsthis, "objectId", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto objectId = JavascriptManager::getInt32(v);
	auto scene = GameManager::getInstance()->getCurrentScene();
	agtk::Object *objectInstance = scene->getObjectInstance(objectId, instanceId);
	if (!objectInstance) {
		return true;
	}

	auto commandData = agtk::data::ObjectCommandSceneEffectData::create(cx, &params);
	auto currentObjectAction = objectInstance->getCurrentObjectAction();
	currentObjectAction->execActionSceneEffect(commandData);

	args.rval().set(JS::Int32Value(agtk::ObjectAction::kCommandBehaviorNext));
	return true;
}

bool js_objectInstance_execCommandSceneEffectRemove(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//ret: 実行に成功したか
	JS::CallArgs args = CallArgsFromVp(argc, vp);
    args.rval().set(JS::NullValue());
	auto len = args.length();
	if (len < 1) {
		return false;
	}
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
	auto &v2 = args[0];
#else
#endif
	if (!v2.isObject()) {
		return false;
	}
	auto &params = v2.toObject();
	JS::RootedObject rparams(cx, &params);

	JS::RootedValue v(cx);
	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
	JS_GetProperty(cx, jsthis, "id", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto instanceId = JavascriptManager::getInt32(v);
	JS_GetProperty(cx, jsthis, "objectId", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto objectId = JavascriptManager::getInt32(v);
	auto scene = GameManager::getInstance()->getCurrentScene();
	agtk::Object *objectInstance = scene->getObjectInstance(objectId, instanceId);
	if (!objectInstance) {
		return true;
	}

	auto commandData = agtk::data::ObjectCommandSceneEffectRemoveData::create(cx, &params);
	auto currentObjectAction = objectInstance->getCurrentObjectAction();
	currentObjectAction->execActionSceneEffectRemove(commandData);

	args.rval().set(JS::Int32Value(agtk::ObjectAction::kCommandBehaviorNext));
	return true;
}

bool js_objectInstance_execCommandSceneGravityChange(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//ret: 実行に成功したか
	JS::CallArgs args = CallArgsFromVp(argc, vp);
	auto len = args.length();
	if (len < 1) {
		return false;
	}
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
	auto &v2 = args[0];
#else
#endif
	if (!v2.isObject()) {
		return false;
	}
	auto &params = v2.toObject();
	JS::RootedObject rparams(cx, &params);

	JS::RootedValue v(cx);
	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
	JS_GetProperty(cx, jsthis, "id", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto instanceId = JavascriptManager::getInt32(v);
	JS_GetProperty(cx, jsthis, "objectId", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto objectId = JavascriptManager::getInt32(v);
	auto scene = GameManager::getInstance()->getCurrentScene();
	agtk::Object *objectInstance = scene->getObjectInstance(objectId, instanceId);
	if (!objectInstance) {
		return true;
	}

	auto commandData = agtk::data::ObjectCommandSceneGravityChangeData::create(cx, &params);
	auto currentObjectAction = objectInstance->getCurrentObjectAction();
	currentObjectAction->execActionSceneGravityChange(commandData);

	args.rval().set(JS::Int32Value(agtk::ObjectAction::kCommandBehaviorNext));
	return true;
}

bool js_objectInstance_execCommandSceneRotateFlip(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//ret: 実行に成功したか
	JS::CallArgs args = CallArgsFromVp(argc, vp);
    args.rval().set(JS::NullValue());
	auto len = args.length();
	if (len < 1) {
		return false;
	}
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
	auto &v2 = args[0];
#else
#endif
	if (!v2.isObject()) {
		return false;
	}
	auto &params = v2.toObject();
	JS::RootedObject rparams(cx, &params);

	JS::RootedValue v(cx);
	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
	JS_GetProperty(cx, jsthis, "id", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto instanceId = JavascriptManager::getInt32(v);
	JS_GetProperty(cx, jsthis, "objectId", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto objectId = JavascriptManager::getInt32(v);
	auto scene = GameManager::getInstance()->getCurrentScene();
	agtk::Object *objectInstance = scene->getObjectInstance(objectId, instanceId);
	if (!objectInstance) {
		return true;
	}

	auto commandData = agtk::data::ObjectCommandSceneRotateFlipData::create(cx, &params);
	auto currentObjectAction = objectInstance->getCurrentObjectAction();
	currentObjectAction->execActionSceneRotateFlip(commandData);

	args.rval().set(JS::Int32Value(agtk::ObjectAction::kCommandBehaviorNext));
	return true;
}

bool js_objectInstance_execCommandCameraAreaChange(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//ret: 実行に成功したか
	JS::CallArgs args = CallArgsFromVp(argc, vp);
    args.rval().set(JS::NullValue());
	auto len = args.length();
	if (len < 1) {
		return false;
	}
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
	auto &v2 = args[0];
#else
#endif
	if (!v2.isObject()) {
		return false;
	}
	auto &params = v2.toObject();
	JS::RootedObject rparams(cx, &params);

	JS::RootedValue v(cx);
	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
	JS_GetProperty(cx, jsthis, "id", &v);
	if (!v.isNumber()) {
		return false;
}
	auto instanceId = JavascriptManager::getInt32(v);
	JS_GetProperty(cx, jsthis, "objectId", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto objectId = JavascriptManager::getInt32(v);
	auto scene = GameManager::getInstance()->getCurrentScene();
	agtk::Object *objectInstance = scene->getObjectInstance(objectId, instanceId);
	if (!objectInstance) {
		return true;
	}

	auto commandData = agtk::data::ObjectCommandCameraAreaChangeData::create(cx, &params);
	auto currentObjectAction = objectInstance->getCurrentObjectAction();
	currentObjectAction->execActionCameraAreaChange(commandData);

	args.rval().set(JS::Int32Value(agtk::ObjectAction::kCommandBehaviorNext));
	return true;
}

bool js_objectInstance_execCommandSoundPlay(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//ret: 実行に成功したか
	JS::CallArgs args = CallArgsFromVp(argc, vp);
    args.rval().set(JS::NullValue());
	auto len = args.length();
	if (len < 1) {
		return false;
	}
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
	auto &v2 = args[0];
#else
#endif
	if (!v2.isObject()) {
		return false;
	}
	auto &params = v2.toObject();
	JS::RootedObject rparams(cx, &params);

	JS::RootedValue v(cx);
	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
	JS_GetProperty(cx, jsthis, "id", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto instanceId = JavascriptManager::getInt32(v);
	JS_GetProperty(cx, jsthis, "objectId", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto objectId = JavascriptManager::getInt32(v);
	auto scene = GameManager::getInstance()->getCurrentScene();
	agtk::Object *objectInstance = scene->getObjectInstance(objectId, instanceId);
	if (!objectInstance) {
		return true;
	}

	auto commandData = agtk::data::ObjectCommandSoundPlayData::create(cx, &params);
	auto currentObjectAction = objectInstance->getCurrentObjectAction();
	currentObjectAction->execActionSoundPlay(commandData);

	args.rval().set(JS::Int32Value(agtk::ObjectAction::kCommandBehaviorNext));
	return true;
}

bool js_objectInstance_execCommandSoundStop(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//ret: 実行に成功したか
	JS::CallArgs args = CallArgsFromVp(argc, vp);
    args.rval().set(JS::NullValue());
	auto len = args.length();
	if (len < 1) {
		return false;
	}
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
	auto &v2 = args[0];
#else
#endif
	if (!v2.isObject()) {
		return false;
	}
	auto &params = v2.toObject();
	JS::RootedObject rparams(cx, &params);

	JS::RootedValue v(cx);
	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
	JS_GetProperty(cx, jsthis, "id", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto instanceId = JavascriptManager::getInt32(v);
	JS_GetProperty(cx, jsthis, "objectId", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto objectId = JavascriptManager::getInt32(v);
	auto scene = GameManager::getInstance()->getCurrentScene();
	agtk::Object *objectInstance = scene->getObjectInstance(objectId, instanceId);
	if (!objectInstance) {
		return true;
	}

	auto commandData = agtk::data::ObjectCommandSoundStopData::create(cx, &params);
	auto currentObjectAction = objectInstance->getCurrentObjectAction();
	currentObjectAction->execActionSoundStop(commandData);

	args.rval().set(JS::Int32Value(agtk::ObjectAction::kCommandBehaviorNext));
	return true;
}

bool js_objectInstance_execCommandSoundPositionRemember(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//ret: 実行に成功したか
	JS::CallArgs args = CallArgsFromVp(argc, vp);
	args.rval().set(JS::NullValue());
	auto len = args.length();
	if (len < 1) {
		return false;
	}
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
	auto &v2 = args[0];
#else
#endif
	if (!v2.isObject()) {
		return false;
	}
	auto &params = v2.toObject();
	JS::RootedObject rparams(cx, &params);

	JS::RootedValue v(cx);
	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
	JS_GetProperty(cx, jsthis, "id", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto instanceId = JavascriptManager::getInt32(v);
	JS_GetProperty(cx, jsthis, "objectId", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto objectId = JavascriptManager::getInt32(v);
	auto scene = GameManager::getInstance()->getCurrentScene();
	agtk::Object *objectInstance = scene->getObjectInstance(objectId, instanceId);
	if (!objectInstance) {
		return true;
	}

	auto commandData = agtk::data::ObjectCommandSoundPositionRememberData::create(cx, &params);
	auto currentObjectAction = objectInstance->getCurrentObjectAction();
	currentObjectAction->execActionSoundPositionRemember(commandData);

	args.rval().set(JS::Int32Value(agtk::ObjectAction::kCommandBehaviorNext));
	return true;
}

bool js_objectInstance_execCommandMessageShow(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//ret: 実行に成功したか
	JS::CallArgs args = CallArgsFromVp(argc, vp);
    args.rval().set(JS::NullValue());
	auto len = args.length();
	if (len < 1) {
		return false;
	}
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
	auto &v2 = args[0];
#else
#endif
	if (!v2.isObject()) {
		return false;
	}
	auto &params = v2.toObject();
	JS::RootedObject rparams(cx, &params);

	JS::RootedValue v(cx);
	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
	JS_GetProperty(cx, jsthis, "id", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto instanceId = JavascriptManager::getInt32(v);
	JS_GetProperty(cx, jsthis, "objectId", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto objectId = JavascriptManager::getInt32(v);
	auto scene = GameManager::getInstance()->getCurrentScene();
	agtk::Object *objectInstance = scene->getObjectInstance(objectId, instanceId);
	if (!objectInstance) {
		return true;
	}

	auto commandData = agtk::data::ObjectCommandMessageShowData::create(cx, &params);
	auto currentObjectAction = objectInstance->getCurrentObjectAction();
	currentObjectAction->execActionMessageShow(commandData);

	args.rval().set(JS::Int32Value(agtk::ObjectAction::kCommandBehaviorNext));
	return true;
}

bool js_objectInstance_execCommandScrollMessageShow(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//ret: 実行に成功したか
	JS::CallArgs args = CallArgsFromVp(argc, vp);
    args.rval().set(JS::NullValue());
	auto len = args.length();
	if (len < 1) {
		return false;
	}
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
	auto &v2 = args[0];
#else
#endif
	if (!v2.isObject()) {
		return false;
	}
	auto &params = v2.toObject();
	JS::RootedObject rparams(cx, &params);

	JS::RootedValue v(cx);
	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
	JS_GetProperty(cx, jsthis, "id", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto instanceId = JavascriptManager::getInt32(v);
	JS_GetProperty(cx, jsthis, "objectId", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto objectId = JavascriptManager::getInt32(v);
	auto scene = GameManager::getInstance()->getCurrentScene();
	agtk::Object *objectInstance = scene->getObjectInstance(objectId, instanceId);
	if (!objectInstance) {
		return true;
	}

	auto commandData = agtk::data::ObjectCommandScrollMessageShowData::create(cx, &params);
	auto currentObjectAction = objectInstance->getCurrentObjectAction();
	currentObjectAction->execActionScrollMessageShow(commandData);

	args.rval().set(JS::Int32Value(agtk::ObjectAction::kCommandBehaviorNext));
	return true;
}

bool js_objectInstance_execCommandEffectShow(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//ret: 実行に成功したか
	JS::CallArgs args = CallArgsFromVp(argc, vp);
    args.rval().set(JS::NullValue());
	auto len = args.length();
	if (len < 1) {
		return false;
	}
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
	auto &v2 = args[0];
#else
#endif
	if (!v2.isObject()) {
		return false;
	}
	auto &params = v2.toObject();
	JS::RootedObject rparams(cx, &params);

	JS::RootedValue v(cx);
	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
	JS_GetProperty(cx, jsthis, "id", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto instanceId = JavascriptManager::getInt32(v);
	JS_GetProperty(cx, jsthis, "objectId", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto objectId = JavascriptManager::getInt32(v);
	auto scene = GameManager::getInstance()->getCurrentScene();
	agtk::Object *objectInstance = scene->getObjectInstance(objectId, instanceId);
	if (!objectInstance) {
		return true;
	}

	auto commandData = agtk::data::ObjectCommandEffectShowData::create(cx, &params);
	auto currentObjectAction = objectInstance->getCurrentObjectAction();
	currentObjectAction->execActionEffectShow(commandData);

	args.rval().set(JS::Int32Value(agtk::ObjectAction::kCommandBehaviorNext));
	return true;
}

bool js_objectInstance_execCommandEffectRemove(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//ret: 実行に成功したか
	JS::CallArgs args = CallArgsFromVp(argc, vp);
    args.rval().set(JS::NullValue());
	auto len = args.length();
	if (len < 1) {
		return false;
	}
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
	auto &v2 = args[0];
#else
#endif
	if (!v2.isObject()) {
		return false;
	}
	auto &params = v2.toObject();
	JS::RootedObject rparams(cx, &params);

	JS::RootedValue v(cx);
	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
	JS_GetProperty(cx, jsthis, "id", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto instanceId = JavascriptManager::getInt32(v);
	JS_GetProperty(cx, jsthis, "objectId", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto objectId = JavascriptManager::getInt32(v);
	auto scene = GameManager::getInstance()->getCurrentScene();
	agtk::Object *objectInstance = scene->getObjectInstance(objectId, instanceId);
	if (!objectInstance) {
		return true;
	}

	auto commandData = agtk::data::ObjectCommandEffectRemoveData::create(cx, &params);
	auto currentObjectAction = objectInstance->getCurrentObjectAction();
	currentObjectAction->execActionEffectRemove(commandData);

	args.rval().set(JS::Int32Value(agtk::ObjectAction::kCommandBehaviorNext));
	return true;
}

bool js_objectInstance_execCommandParticleShow(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//ret: 実行に成功したか
	JS::CallArgs args = CallArgsFromVp(argc, vp);
    args.rval().set(JS::NullValue());
	auto len = args.length();
	if (len < 1) {
		return false;
	}
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
	auto &v2 = args[0];
#else
#endif

	if (!v2.isObject()) {
		return false;
	}
	auto &params = v2.toObject();
	JS::RootedObject rparams(cx, &params);

	JS::RootedValue v(cx);
	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
	JS_GetProperty(cx, jsthis, "id", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto instanceId = JavascriptManager::getInt32(v);
	JS_GetProperty(cx, jsthis, "objectId", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto objectId = JavascriptManager::getInt32(v);
	auto scene = GameManager::getInstance()->getCurrentScene();
	agtk::Object *objectInstance = scene->getObjectInstance(objectId, instanceId);
	if (!objectInstance) {
		return true;
	}

	auto commandData = agtk::data::ObjectCommandParticleShowData::create(cx, &params);
	auto currentObjectAction = objectInstance->getCurrentObjectAction();
	currentObjectAction->execActionParticleShow(commandData);

	args.rval().set(JS::Int32Value(agtk::ObjectAction::kCommandBehaviorNext));
	return true;
}

bool js_objectInstance_execCommandParticleRemove(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//ret: 実行に成功したか
	JS::CallArgs args = CallArgsFromVp(argc, vp);
    args.rval().set(JS::NullValue());
	auto len = args.length();
	if (len < 1) {
		return false;
	}
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
	auto &v2 = args[0];
#else
#endif
	if (!v2.isObject()) {
		return false;
	}
	auto &params = v2.toObject();
	JS::RootedObject rparams(cx, &params);

	JS::RootedValue v(cx);
	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
	JS_GetProperty(cx, jsthis, "id", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto instanceId = JavascriptManager::getInt32(v);
	JS_GetProperty(cx, jsthis, "objectId", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto objectId = JavascriptManager::getInt32(v);
	auto scene = GameManager::getInstance()->getCurrentScene();
	agtk::Object *objectInstance = scene->getObjectInstance(objectId, instanceId);
	if (!objectInstance) {
		return true;
	}

	auto commandData = agtk::data::ObjectCommandParticleRemoveData::create(cx, &params);
	auto currentObjectAction = objectInstance->getCurrentObjectAction();
	currentObjectAction->execActionParticleRemove(commandData);

	args.rval().set(JS::Int32Value(agtk::ObjectAction::kCommandBehaviorNext));
	return true;
}

bool js_objectInstance_execCommandMovieShow(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//ret: 実行に成功したか
	JS::CallArgs args = CallArgsFromVp(argc, vp);
    args.rval().set(JS::NullValue());
	auto len = args.length();
	if (len < 1) {
		return false;
	}
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
	auto &v2 = args[0];
#else
#endif
	if (!v2.isObject()) {
		return false;
	}
	auto &params = v2.toObject();
	JS::RootedObject rparams(cx, &params);

	JS::RootedValue v(cx);
	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
	JS_GetProperty(cx, jsthis, "id", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto instanceId = JavascriptManager::getInt32(v);
	JS_GetProperty(cx, jsthis, "objectId", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto objectId = JavascriptManager::getInt32(v);
	auto scene = GameManager::getInstance()->getCurrentScene();
	agtk::Object *objectInstance = scene->getObjectInstance(objectId, instanceId);
	if (!objectInstance) {
		return true;
	}

	auto commandData = agtk::data::ObjectCommandMovieShowData::create(cx, &params);
	auto currentObjectAction = objectInstance->getCurrentObjectAction();
	currentObjectAction->execActionMovieShow(commandData);

	args.rval().set(JS::Int32Value(agtk::ObjectAction::kCommandBehaviorNext));
	return true;
}

bool js_objectInstance_execCommandImageShow(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//ret: 実行に成功したか
	JS::CallArgs args = CallArgsFromVp(argc, vp);
    args.rval().set(JS::NullValue());
	auto len = args.length();
	if (len < 1) {
		return false;
	}
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
	auto &v2 = args[0];
#else
#endif
	if (!v2.isObject()) {
		return false;
	}
	auto &params = v2.toObject();
	JS::RootedObject rparams(cx, &params);

	JS::RootedValue v(cx);
	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
	JS_GetProperty(cx, jsthis, "id", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto instanceId = JavascriptManager::getInt32(v);
	JS_GetProperty(cx, jsthis, "objectId", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto objectId = JavascriptManager::getInt32(v);
	auto scene = GameManager::getInstance()->getCurrentScene();
	agtk::Object *objectInstance = scene->getObjectInstance(objectId, instanceId);
	if (!objectInstance) {
		return true;
	}

	auto commandData = agtk::data::ObjectCommandImageShowData::create(cx, &params);
	auto currentObjectAction = objectInstance->getCurrentObjectAction();
	currentObjectAction->execActionImageShow(commandData);

	args.rval().set(JS::Int32Value(agtk::ObjectAction::kCommandBehaviorNext));
	return true;
}

bool js_objectInstance_execCommandSwitchVariableChange(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//ret: 実行に成功したか
	JS::CallArgs args = CallArgsFromVp(argc, vp);
    args.rval().set(JS::NullValue());
	auto len = args.length();
	if (len < 1) {
		return false;
	}
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
	auto &v2 = args[0];
#else
#endif
	if (!v2.isObject()) {
		return false;
	}
	auto &params = v2.toObject();
	JS::RootedObject rparams(cx, &params);

	JS::RootedValue v(cx);
	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
	JS_GetProperty(cx, jsthis, "id", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto instanceId = JavascriptManager::getInt32(v);
	JS_GetProperty(cx, jsthis, "objectId", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto objectId = JavascriptManager::getInt32(v);
	auto scene = GameManager::getInstance()->getCurrentScene();
	agtk::Object *objectInstance = scene->getObjectInstance(objectId, instanceId);
	if (!objectInstance) {
		return true;
	}

	auto commandData = agtk::data::ObjectCommandSwitchVariableChangeData::create(cx, &params);
	auto currentObjectAction = objectInstance->getCurrentObjectAction();
	currentObjectAction->execActionSwitchVariableChange(commandData);

	args.rval().set(JS::Int32Value(agtk::ObjectAction::kCommandBehaviorNext));
	return true;
}

bool js_objectInstance_execCommandSwitchVariableReset(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//ret: 実行に成功したか
	JS::CallArgs args = CallArgsFromVp(argc, vp);
    args.rval().set(JS::NullValue());
	auto len = args.length();
	if (len < 1) {
		return false;
	}
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
	auto &v2 = args[0];
#else
#endif
	if (!v2.isObject()) {
		return false;
	}
	auto &params = v2.toObject();
	JS::RootedObject rparams(cx, &params);


	JS::RootedValue v(cx);
	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
	JS_GetProperty(cx, jsthis, "id", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto instanceId = JavascriptManager::getInt32(v);
	JS_GetProperty(cx, jsthis, "objectId", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto objectId = JavascriptManager::getInt32(v);
	auto scene = GameManager::getInstance()->getCurrentScene();
	agtk::Object *objectInstance = scene->getObjectInstance(objectId, instanceId);
	if (!objectInstance) {
		return true;
	}

	auto commandData = agtk::data::ObjectCommandSwitchVariableResetData::create(cx, &params);
	if (commandData) {
		auto currentObjectAction = objectInstance->getCurrentObjectAction();
		currentObjectAction->execActionSwitchVariableReset(commandData);
	}

	args.rval().set(JS::Int32Value(agtk::ObjectAction::kCommandBehaviorNext));
	return true;
}

bool js_objectInstance_execCommandGameSpeedChange(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//ret: 実行に成功したか
	JS::CallArgs args = CallArgsFromVp(argc, vp);
    args.rval().set(JS::NullValue());
	auto len = args.length();
	if (len < 1) {
		return false;
	}
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
	auto &v2 = args[0];
#else
#endif
	if (!v2.isObject()) {
		return false;
	}
	auto &params = v2.toObject();
	JS::RootedObject rparams(cx, &params);

	JS::RootedValue v(cx);
	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
	JS_GetProperty(cx, jsthis, "id", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto instanceId = JavascriptManager::getInt32(v);
	JS_GetProperty(cx, jsthis, "objectId", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto objectId = JavascriptManager::getInt32(v);
	auto scene = GameManager::getInstance()->getCurrentScene();
	agtk::Object *objectInstance = scene->getObjectInstance(objectId, instanceId);
	if (!objectInstance) {
		return true;
	}

	auto commandData = agtk::data::ObjectCommandGameSpeedChangeData::create(cx, &params);
	auto currentObjectAction = objectInstance->getCurrentObjectAction();
	currentObjectAction->execActionGameSpeedChange(commandData);

	args.rval().set(JS::Int32Value(agtk::ObjectAction::kCommandBehaviorNext));
	return true;
}

bool js_objectInstance_execCommandTimer(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//ret: 実行に成功したか
	JS::CallArgs args = CallArgsFromVp(argc, vp);
    args.rval().set(JS::NullValue());
	auto len = args.length();
	if (len < 1) {
		return false;
	}
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
	auto &v2 = args[0];
#else
#endif
	if (!v2.isObject()) {
		return false;
	}
	auto &params = v2.toObject();
	JS::RootedObject rparams(cx, &params);

	JS::RootedValue v(cx);
	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
	JS_GetProperty(cx, jsthis, "id", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto instanceId = JavascriptManager::getInt32(v);
	JS_GetProperty(cx, jsthis, "objectId", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto objectId = JavascriptManager::getInt32(v);
	auto scene = GameManager::getInstance()->getCurrentScene();
	agtk::Object *objectInstance = scene->getObjectInstance(objectId, instanceId);
	if (!objectInstance) {
		return true;
	}

	auto commandData = agtk::data::ObjectCommandTimerData::create(cx, &params);
	auto currentObjectAction = objectInstance->getCurrentObjectAction();
	currentObjectAction->execActionTimer(commandData);

	args.rval().set(JS::Int32Value(agtk::ObjectAction::kCommandBehaviorNext));
	return true;
}

bool js_objectInstance_execCommandSceneTerminate(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//ret: 実行に成功したか
	JS::CallArgs args = CallArgsFromVp(argc, vp);
    args.rval().set(JS::NullValue());
	auto len = args.length();

	JS::RootedValue v(cx);
	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
	JS_GetProperty(cx, jsthis, "id", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto instanceId = JavascriptManager::getInt32(v);
	JS_GetProperty(cx, jsthis, "objectId", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto objectId = JavascriptManager::getInt32(v);
	auto scene = GameManager::getInstance()->getCurrentScene();
	agtk::Object *objectInstance = scene->getObjectInstance(objectId, instanceId);
	if (!objectInstance) {
		return true;
	}

	auto commandData = agtk::data::ObjectCommandSceneTerminateData::create(cx, nullptr);
	auto currentObjectAction = objectInstance->getCurrentObjectAction();
	currentObjectAction->execActionSceneTerminate(commandData);

	args.rval().set(JS::Int32Value(agtk::ObjectAction::kCommandBehaviorNext));
	return true;
}

bool js_objectInstance_execCommandDirectionMove(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//ret: 実行に成功したか
	JS::CallArgs args = CallArgsFromVp(argc, vp);
    args.rval().set(JS::NullValue());
	auto len = args.length();
	if (len < 1) {
		return false;
	}
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
	auto &v2 = args[0];
#else
#endif
	if (!v2.isObject()) {
		return false;
	}
	auto &params = v2.toObject();
	JS::RootedObject rparams(cx, &params);

	JS::RootedValue v(cx);
	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
	JS_GetProperty(cx, jsthis, "id", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto instanceId = JavascriptManager::getInt32(v);
	JS_GetProperty(cx, jsthis, "objectId", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto objectId = JavascriptManager::getInt32(v);
	auto scene = GameManager::getInstance()->getCurrentScene();
	agtk::Object *objectInstance = scene->getObjectInstance(objectId, instanceId);
	if (!objectInstance) {
		return true;
	}

	auto commandData = agtk::data::ObjectCommandDirectionMoveData::create(cx, &params);
	auto currentObjectAction = objectInstance->getCurrentObjectAction();
	currentObjectAction->execActionDirectionMove(commandData);

	args.rval().set(JS::Int32Value(agtk::ObjectAction::kCommandBehaviorNext));
	return true;
}

bool js_objectInstance_execCommandForthBackMoveTurn(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//ret: 実行に成功したか
	JS::CallArgs args = CallArgsFromVp(argc, vp);
    args.rval().set(JS::NullValue());
	auto len = args.length();
	if (len < 1) {
		return false;
	}
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
	auto &v2 = args[0];
#else
#endif
	if (!v2.isObject()) {
		return false;
	}
	auto &params = v2.toObject();
	JS::RootedObject rparams(cx, &params);

	JS::RootedValue v(cx);
	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
	JS_GetProperty(cx, jsthis, "id", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto instanceId = JavascriptManager::getInt32(v);
	JS_GetProperty(cx, jsthis, "objectId", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto objectId = JavascriptManager::getInt32(v);
	auto scene = GameManager::getInstance()->getCurrentScene();
	agtk::Object *objectInstance = scene->getObjectInstance(objectId, instanceId);
	if (!objectInstance) {
		return true;
	}

	auto commandData = agtk::data::ObjectCommandForthBackMoveTurnData::create(cx, &params);
	auto currentObjectAction = objectInstance->getCurrentObjectAction();
	currentObjectAction->execActionForthBackMoveTurn(commandData);

	args.rval().set(JS::Int32Value(agtk::ObjectAction::kCommandBehaviorNext));
	return true;
}

bool js_objectInstance_execCommandActionExec(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//ret: 実行に成功したか
	JS::CallArgs args = CallArgsFromVp(argc, vp);
    args.rval().set(JS::NullValue());
	auto len = args.length();
	if (len < 1) {
		return false;
	}
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
	auto &v2 = args[0];
#else
#endif
	if (!v2.isObject()) {
		return false;
	}
	auto &params = v2.toObject();
	JS::RootedObject rparams(cx, &params);

	JS::RootedValue v(cx);
	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
	JS_GetProperty(cx, jsthis, "id", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto instanceId = JavascriptManager::getInt32(v);
	JS_GetProperty(cx, jsthis, "objectId", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto objectId = JavascriptManager::getInt32(v);
	auto scene = GameManager::getInstance()->getCurrentScene();
	agtk::Object *objectInstance = scene->getObjectInstance(objectId, instanceId);
	if (!objectInstance) {
		return true;
	}

	auto commandData = agtk::data::ObjectCommandActionExecData::create(cx, &params);
	auto currentObjectAction = objectInstance->getCurrentObjectAction();
	auto isLoopBreak = currentObjectAction->execActionActionExec(commandData);

	args.rval().set(JS::Int32Value(isLoopBreak ? agtk::ObjectAction::kCommandBehaviorBreak : agtk::ObjectAction::kCommandBehaviorNext));
	return true;
}

bool js_objectInstance_execCommandSceneShake(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//ret: 実行に成功したか
	JS::CallArgs args = CallArgsFromVp(argc, vp);
    args.rval().set(JS::NullValue());
	auto len = args.length();
	if (len < 1) {
		return false;
	}
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
	auto &v2 = args[0];
#else
#endif
	if (!v2.isObject()) {
		return false;
	}
	auto &params = v2.toObject();
	JS::RootedObject rparams(cx, &params);

	JS::RootedValue v(cx);
	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
	JS_GetProperty(cx, jsthis, "id", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto instanceId = JavascriptManager::getInt32(v);
	JS_GetProperty(cx, jsthis, "objectId", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto objectId = JavascriptManager::getInt32(v);
	auto scene = GameManager::getInstance()->getCurrentScene();
	agtk::Object *objectInstance = scene->getObjectInstance(objectId, instanceId);
	if (!objectInstance) {
		return true;
	}

	auto commandData = agtk::data::ObjectCommandSceneShakeData::create(cx, &params);
	auto currentObjectAction = objectInstance->getCurrentObjectAction();
	currentObjectAction->execActionSceneShake(commandData);

	args.rval().set(JS::Int32Value(agtk::ObjectAction::kCommandBehaviorNext));
	return true;
}

bool js_objectInstance_execCommandLayerHide(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//ret: 実行に成功したか
	JS::CallArgs args = CallArgsFromVp(argc, vp);
    args.rval().set(JS::NullValue());
	auto len = args.length();
	if (len < 1) {
		return false;
	}
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
	auto &v2 = args[0];
#else
#endif
	if (!v2.isObject()) {
		return false;
	}
	auto &params = v2.toObject();
	JS::RootedObject rparams(cx, &params);

	JS::RootedValue v(cx);
	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
	JS_GetProperty(cx, jsthis, "id", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto instanceId = JavascriptManager::getInt32(v);
	JS_GetProperty(cx, jsthis, "objectId", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto objectId = JavascriptManager::getInt32(v);
	auto scene = GameManager::getInstance()->getCurrentScene();
	agtk::Object *objectInstance = scene->getObjectInstance(objectId, instanceId);
	if (!objectInstance) {
		return true;
	}

	auto commandData = agtk::data::ObjectCommandLayerHide::create(cx, &params);
	auto currentObjectAction = objectInstance->getCurrentObjectAction();
	currentObjectAction->execActionLayerHide(commandData);

	args.rval().set(JS::Int32Value(agtk::ObjectAction::kCommandBehaviorNext));
	return true;
}

bool js_objectInstance_execCommandLayerShow(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//ret: 実行に成功したか
	JS::CallArgs args = CallArgsFromVp(argc, vp);
    args.rval().set(JS::NullValue());
	auto len = args.length();
	if (len < 1) {
		return false;
	}
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
	auto &v2 = args[0];
#else
#endif
	if (!v2.isObject()) {
		return false;
	}
	auto &params = v2.toObject();
	JS::RootedObject rparams(cx, &params);

	JS::RootedValue v(cx);
	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
	JS_GetProperty(cx, jsthis, "id", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto instanceId = JavascriptManager::getInt32(v);
	JS_GetProperty(cx, jsthis, "objectId", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto objectId = JavascriptManager::getInt32(v);
	auto scene = GameManager::getInstance()->getCurrentScene();
	agtk::Object *objectInstance = scene->getObjectInstance(objectId, instanceId);
	if (!objectInstance) {
		return true;
	}

	auto commandData = agtk::data::ObjectCommandLayerShow::create(cx, &params);
	auto currentObjectAction = objectInstance->getCurrentObjectAction();
	currentObjectAction->execActionLayerShow(commandData);

	args.rval().set(JS::Int32Value(agtk::ObjectAction::kCommandBehaviorNext));
	return true;
}

bool js_objectInstance_execCommandLayerDisable(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//ret: 実行に成功したか
	JS::CallArgs args = CallArgsFromVp(argc, vp);
    args.rval().set(JS::NullValue());
	auto len = args.length();
	if (len < 1) {
		return false;
	}
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
	auto &v2 = args[0];
#else
#endif
	if (!v2.isObject()) {
		return false;
	}
	auto &params = v2.toObject();
	JS::RootedObject rparams(cx, &params);

	JS::RootedValue v(cx);
	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
	JS_GetProperty(cx, jsthis, "id", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto instanceId = JavascriptManager::getInt32(v);
	JS_GetProperty(cx, jsthis, "objectId", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto objectId = JavascriptManager::getInt32(v);
	auto scene = GameManager::getInstance()->getCurrentScene();
	agtk::Object *objectInstance = scene->getObjectInstance(objectId, instanceId);
	if (!objectInstance) {
		return true;
	}

	auto commandData = agtk::data::ObjectCommandLayerDisable::create(cx, &params);
	auto currentObjectAction = objectInstance->getCurrentObjectAction();
	currentObjectAction->execActionLayerDisable(commandData);

	args.rval().set(JS::Int32Value(agtk::ObjectAction::kCommandBehaviorNext));
	return true;
}

bool js_objectInstance_execCommandLayerEnable(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//ret: 実行に成功したか
	JS::CallArgs args = CallArgsFromVp(argc, vp);
    args.rval().set(JS::NullValue());
	auto len = args.length();
	if (len < 1) {
		return false;
	}
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
	auto &v2 = args[0];
#else
#endif
	if (!v2.isObject()) {
		return false;
	}
	auto &params = v2.toObject();
	JS::RootedObject rparams(cx, &params);

	JS::RootedValue v(cx);
	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
	JS_GetProperty(cx, jsthis, "id", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto instanceId = JavascriptManager::getInt32(v);
	JS_GetProperty(cx, jsthis, "objectId", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto objectId = JavascriptManager::getInt32(v);
	auto scene = GameManager::getInstance()->getCurrentScene();
	agtk::Object *objectInstance = scene->getObjectInstance(objectId, instanceId);
	if (!objectInstance) {
		return true;
	}

	auto commandData = agtk::data::ObjectCommandLayerEnable::create(cx, &params);
	auto currentObjectAction = objectInstance->getCurrentObjectAction();
	currentObjectAction->execActionLayerEnable(commandData);

	args.rval().set(JS::Int32Value(agtk::ObjectAction::kCommandBehaviorNext));
	return true;
}

bool js_objectInstance_execCommandMenuShow(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//ret: 実行に成功したか
	JS::CallArgs args = CallArgsFromVp(argc, vp);
    args.rval().set(JS::NullValue());
	auto len = args.length();
	if (len < 1) {
		return false;
	}
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
	auto &v2 = args[0];
#else
#endif
	if (!v2.isObject()) {
		return false;
	}
	auto &params = v2.toObject();
	JS::RootedObject rparams(cx, &params);

	JS::RootedValue v(cx);
	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
	JS_GetProperty(cx, jsthis, "id", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto instanceId = JavascriptManager::getInt32(v);
	JS_GetProperty(cx, jsthis, "objectId", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto objectId = JavascriptManager::getInt32(v);
	auto scene = GameManager::getInstance()->getCurrentScene();
	agtk::Object *objectInstance = scene->getObjectInstance(objectId, instanceId);
	if (!objectInstance) {
		return true;
	}

	auto commandData = agtk::data::ObjectCommandMenuShowData::create(cx, &params);
	auto currentObjectAction = objectInstance->getCurrentObjectAction();
	currentObjectAction->execActionMenuShow(commandData);

	args.rval().set(JS::Int32Value(agtk::ObjectAction::kCommandBehaviorNext));
	return true;
}

bool js_objectInstance_execCommandMenuHide(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//ret: 実行に成功したか
	JS::CallArgs args = CallArgsFromVp(argc, vp);
    args.rval().set(JS::NullValue());
	auto len = args.length();
	if (len < 1) {
		return false;
	}
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
	auto &v2 = args[0];
#else
#endif
	if (!v2.isObject()) {
		return false;
	}
	auto &params = v2.toObject();
	JS::RootedObject rparams(cx, &params);

	JS::RootedValue v(cx);
	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
	JS_GetProperty(cx, jsthis, "id", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto instanceId = JavascriptManager::getInt32(v);
	JS_GetProperty(cx, jsthis, "objectId", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto objectId = JavascriptManager::getInt32(v);
	auto scene = GameManager::getInstance()->getCurrentScene();
	agtk::Object *objectInstance = scene->getObjectInstance(objectId, instanceId);
	if (!objectInstance) {
		return true;
	}

	auto commandData = agtk::data::ObjectCommandMenuHideData::create(cx, &params);
	auto currentObjectAction = objectInstance->getCurrentObjectAction();
	currentObjectAction->execActionMenuHide(commandData);

	args.rval().set(JS::Int32Value(agtk::ObjectAction::kCommandBehaviorNext));
	return true;
}

bool js_objectInstance_execCommandDisplayDirectionMove(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//ret: 実行に成功したか
	JS::CallArgs args = CallArgsFromVp(argc, vp);
    args.rval().set(JS::NullValue());
	auto len = args.length();
	if (len < 1) {
		return false;
	}
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
	auto &v2 = args[0];
#else
#endif
	if (!v2.isObject()) {
		return false;
	}
	auto &params = v2.toObject();
	JS::RootedObject rparams(cx, &params);

	JS::RootedValue v(cx);
	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
	JS_GetProperty(cx, jsthis, "id", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto instanceId = JavascriptManager::getInt32(v);
	JS_GetProperty(cx, jsthis, "objectId", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto objectId = JavascriptManager::getInt32(v);
	auto scene = GameManager::getInstance()->getCurrentScene();
	agtk::Object *objectInstance = scene->getObjectInstance(objectId, instanceId);
	if (!objectInstance) {
		return true;
	}

	auto commandData = agtk::data::ObjectCommandDisplayDirectionMoveData::create(cx, &params);
	auto currentObjectAction = objectInstance->getCurrentObjectAction();
	currentObjectAction->execActionDisplayDirectionMove(commandData);

	args.rval().set(JS::Int32Value(agtk::ObjectAction::kCommandBehaviorNext));
	return true;
}

bool js_objectInstance_execCommandFileLoad(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//ret: 実行に成功したか
	JS::CallArgs args = CallArgsFromVp(argc, vp);
	args.rval().set(JS::NullValue());
	auto len = args.length();
	if (len < 1) {
		return false;
	}
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
	auto &v2 = args[0];
#else
#endif
	if (!v2.isObject()) {
		return false;
	}
	auto &params = v2.toObject();
	JS::RootedObject rparams(cx, &params);

	JS::RootedValue v(cx);
	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
	JS_GetProperty(cx, jsthis, "id", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto instanceId = JavascriptManager::getInt32(v);
	JS_GetProperty(cx, jsthis, "objectId", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto objectId = JavascriptManager::getInt32(v);
	auto scene = GameManager::getInstance()->getCurrentScene();
	agtk::Object *objectInstance = scene->getObjectInstance(objectId, instanceId);
	if (!objectInstance) {
		return true;
	}

	auto commandData = agtk::data::ObjectCommandFileLoadData::create(cx, &params);
	auto currentObjectAction = objectInstance->getCurrentObjectAction();
	currentObjectAction->execActionFileLoad(commandData);

	args.rval().set(JS::Int32Value(agtk::ObjectAction::kCommandBehaviorNext));
	return true;
}

bool js_objectInstance_execCommandObjectUnlock(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//ret: 実行に成功したか
	JS::CallArgs args = CallArgsFromVp(argc, vp);
	args.rval().set(JS::NullValue());
	auto len = args.length();
	if (len < 1) {
		return false;
	}
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
	auto &v2 = args[0];
#else
#endif
	if (!v2.isObject()) {
		return false;
	}
	auto &params = v2.toObject();
	JS::RootedObject rparams(cx, &params);

	JS::RootedValue v(cx);
	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
	JS_GetProperty(cx, jsthis, "id", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto instanceId = JavascriptManager::getInt32(v);
	JS_GetProperty(cx, jsthis, "objectId", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto objectId = JavascriptManager::getInt32(v);
	auto scene = GameManager::getInstance()->getCurrentScene();
	agtk::Object *objectInstance = scene->getObjectInstance(objectId, instanceId);
	if (!objectInstance) {
		return true;
	}

	auto commandData = agtk::data::ObjectCommandObjectUnlockData::create(cx, &params);
	auto currentObjectAction = objectInstance->getCurrentObjectAction();
	currentObjectAction->execActionObjectUnlock(commandData);

	args.rval().set(JS::Int32Value(agtk::ObjectAction::kCommandBehaviorNext));
	return true;
}

bool js_objectInstance_execCommandResourceSetChange(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//ret: 実行に成功したか
	JS::CallArgs args = CallArgsFromVp(argc, vp);
	args.rval().set(JS::NullValue());
	auto len = args.length();
	if (len < 1) {
		return false;
	}
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
	auto &v2 = args[0];
#else
#endif
	if (!v2.isObject()) {
		return false;
	}
	auto &params = v2.toObject();
	JS::RootedObject rparams(cx, &params);

	JS::RootedValue v(cx);
	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
	JS_GetProperty(cx, jsthis, "id", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto instanceId = JavascriptManager::getInt32(v);
	JS_GetProperty(cx, jsthis, "objectId", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto objectId = JavascriptManager::getInt32(v);
	auto scene = GameManager::getInstance()->getCurrentScene();
	agtk::Object *objectInstance = scene->getObjectInstance(objectId, instanceId);
	if (!objectInstance) {
		return true;
	}

	auto commandData = agtk::data::ObjectCommandResourceSetChangeData::create(cx, &params);
	auto currentObjectAction = objectInstance->getCurrentObjectAction();
	currentObjectAction->execActionResourceSetChange(commandData);

	args.rval().set(JS::Int32Value(agtk::ObjectAction::kCommandBehaviorNext));
	return true;
}

bool js_objectInstance_execCommandDatabaseReflect(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//ret: 実行に成功したか
	JS::CallArgs args = CallArgsFromVp(argc, vp);
	args.rval().set(JS::NullValue());
	auto len = args.length();
	if (len < 1) {
		return false;
	}
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
	auto &v2 = args[0];
#else
#endif
	if (!v2.isObject()) {
		return false;
	}
	auto &params = v2.toObject();
	JS::RootedObject rparams(cx, &params);

	JS::RootedValue v(cx);
	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
	JS_GetProperty(cx, jsthis, "id", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto instanceId = JavascriptManager::getInt32(v);
	JS_GetProperty(cx, jsthis, "objectId", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto objectId = JavascriptManager::getInt32(v);
	auto scene = GameManager::getInstance()->getCurrentScene();
	agtk::Object *objectInstance = scene->getObjectInstance(objectId, instanceId);
	if (!objectInstance) {
		return true;
	}

	auto commandData = agtk::data::ObjectCommandDatabaseReflectData::create(cx, &params);
	auto currentObjectAction = objectInstance->getCurrentObjectAction();
	currentObjectAction->execActionDatabaseReflect(commandData);

	args.rval().set(JS::Int32Value(agtk::ObjectAction::kCommandBehaviorNext));
	return true;
}

bool js_objectInstance_isWallTouched(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//ret: 成立しているか
	JS::CallArgs args = CallArgsFromVp(argc, vp);
    args.rval().set(JS::NullValue());
	auto len = args.length();
	if (len < 1) {
		return false;
	}
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
	auto &v2 = args[0];
#else
#endif
	if (!v2.isObject()) {
		return false;
	}
	auto &params = v2.toObject();
	JS::RootedObject rparams(cx, &params);

	JS::RootedValue v(cx);
	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
	JS_GetProperty(cx, jsthis, "id", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto instanceId = JavascriptManager::getInt32(v);
	JS_GetProperty(cx, jsthis, "objectId", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto objectId = JavascriptManager::getInt32(v);
	auto scene = GameManager::getInstance()->getCurrentScene();
	agtk::Object *objectInstance = scene->getObjectInstance(objectId, instanceId);
	if (!objectInstance) {
		return true;
	}

	auto linkConditionData = agtk::data::ObjectActionLinkConditionWallTouchedData::create(cx, &params);
	auto currentObjectAction = objectInstance->getCurrentObjectAction();
	auto result = currentObjectAction->checkLinkConditionWallTouched(linkConditionData);


	args.rval().set(JS::BooleanValue(result));
	return true;
}

bool js_objectInstance_isWallAhead(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//ret: 成立しているか
	JS::CallArgs args = CallArgsFromVp(argc, vp);
    args.rval().set(JS::NullValue());
	auto len = args.length();
	if (len < 1) {
		return false;
	}
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
	auto &v2 = args[0];
#else
#endif
	if (!v2.isObject()) {
		return false;
	}
	auto &params = v2.toObject();
	JS::RootedObject rparams(cx, &params);

	JS::RootedValue v(cx);
	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
	JS_GetProperty(cx, jsthis, "id", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto instanceId = JavascriptManager::getInt32(v);
	JS_GetProperty(cx, jsthis, "objectId", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto objectId = JavascriptManager::getInt32(v);
	auto scene = GameManager::getInstance()->getCurrentScene();
	agtk::Object *objectInstance = scene->getObjectInstance(objectId, instanceId);
	if (!objectInstance) {
		return true;
	}

	auto linkConditionData = agtk::data::ObjectActionLinkConditionWallAheadData::create(cx, &params);
	auto currentObjectAction = objectInstance->getCurrentObjectAction();
	auto result = currentObjectAction->checkLinkConditionWallAhead(linkConditionData);


	args.rval().set(JS::BooleanValue(result));
	return true;
}

bool js_objectInstance_isObjectWallTouched(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//ret: 成立しているか
	JS::CallArgs args = CallArgsFromVp(argc, vp);
    args.rval().set(JS::NullValue());
	auto len = args.length();
	if (len < 1) {
		return false;
	}
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
	auto &v2 = args[0];
#else
#endif
	if (!v2.isObject()) {
		return false;
	}
	auto &params = v2.toObject();
	JS::RootedObject rparams(cx, &params);

	JS::RootedValue v(cx);
	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
	JS_GetProperty(cx, jsthis, "id", &v);
	if (!v.isNumber()) {
		return false;
}
	auto instanceId = JavascriptManager::getInt32(v);
	JS_GetProperty(cx, jsthis, "objectId", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto objectId = JavascriptManager::getInt32(v);
	auto scene = GameManager::getInstance()->getCurrentScene();
	agtk::Object *objectInstance = scene->getObjectInstance(objectId, instanceId);
	if (!objectInstance) {
		return true;
	}

	auto linkConditionData = agtk::data::ObjectActionLinkConditionObjectWallTouchedData::create(cx, &params);
	auto currentObjectAction = objectInstance->getCurrentObjectAction();
	auto result = currentObjectAction->checkLinkConditionObjectWallTouched(linkConditionData);


	args.rval().set(JS::BooleanValue(result));
	return true;
}

bool js_objectInstance_isAttackAreaTouched(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//ret: 成立しているか
	JS::CallArgs args = CallArgsFromVp(argc, vp);
    args.rval().set(JS::NullValue());
	auto len = args.length();
	if (len < 1) {
		return false;
	}
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
	auto &v2 = args[0];
#else
#endif
	if (!v2.isObject()) {
		return false;
	}
	auto &params = v2.toObject();
	JS::RootedObject rparams(cx, &params);

	JS::RootedValue v(cx);
	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
	JS_GetProperty(cx, jsthis, "id", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto instanceId = JavascriptManager::getInt32(v);
	JS_GetProperty(cx, jsthis, "objectId", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto objectId = JavascriptManager::getInt32(v);
	auto scene = GameManager::getInstance()->getCurrentScene();
	agtk::Object *objectInstance = scene->getObjectInstance(objectId, instanceId);
	if (!objectInstance) {
		return true;
	}

	auto linkConditionData = agtk::data::ObjectActionLinkConditionAttackAreaTouchedData::create(cx, &params);
	auto currentObjectAction = objectInstance->getCurrentObjectAction();
	auto result = currentObjectAction->checkLinkConditionAttackAreaTouched(linkConditionData);


	args.rval().set(JS::BooleanValue(result));
	return true;
}

bool js_objectInstance_isAttackAreaNear(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//ret: 成立しているか
	JS::CallArgs args = CallArgsFromVp(argc, vp);
    args.rval().set(JS::NullValue());
	auto len = args.length();
	if (len < 1) {
		return false;
	}
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
	auto &v2 = args[0];
#else
#endif
	if (!v2.isObject()) {
		return false;
	}
	auto &params = v2.toObject();
	JS::RootedObject rparams(cx, &params);

	JS::RootedValue v(cx);
	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
	JS_GetProperty(cx, jsthis, "id", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto instanceId = JavascriptManager::getInt32(v);
	JS_GetProperty(cx, jsthis, "objectId", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto objectId = JavascriptManager::getInt32(v);
	auto scene = GameManager::getInstance()->getCurrentScene();
	agtk::Object *objectInstance = scene->getObjectInstance(objectId, instanceId);
	if (!objectInstance) {
		return true;
	}

	auto linkConditionData = agtk::data::ObjectActionLinkConditionAttackAreaNearData::create(cx, &params);
	auto currentObjectAction = objectInstance->getCurrentObjectAction();
	auto result = currentObjectAction->checkLinkConditionAttackAreaNear(linkConditionData);


	args.rval().set(JS::BooleanValue(result));
	return true;
}

bool js_objectInstance_isObjectNear(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//ret: 成立しているか
	JS::CallArgs args = CallArgsFromVp(argc, vp);
    args.rval().set(JS::NullValue());
	auto len = args.length();
	if (len < 1) {
		return false;
	}
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
	auto &v2 = args[0];
#else
#endif
	if (!v2.isObject()) {
		return false;
	}
	auto &params = v2.toObject();
	JS::RootedObject rparams(cx, &params);

	JS::RootedValue v(cx);
	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
	JS_GetProperty(cx, jsthis, "id", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto instanceId = JavascriptManager::getInt32(v);
	JS_GetProperty(cx, jsthis, "objectId", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto objectId = JavascriptManager::getInt32(v);
	auto scene = GameManager::getInstance()->getCurrentScene();
	agtk::Object *objectInstance = scene->getObjectInstance(objectId, instanceId);
	if (!objectInstance) {
		return true;
	}

	auto linkConditionData = agtk::data::ObjectActionLinkConditionObjectNearData::create(cx, &params);
	auto currentObjectAction = objectInstance->getCurrentObjectAction();
	auto result = currentObjectAction->checkLinkConditionObjectNear(linkConditionData);


	args.rval().set(JS::BooleanValue(result));
	return true;
}

bool js_objectInstance_isObjectFacingEachOther(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//ret: 成立しているか
	JS::CallArgs args = CallArgsFromVp(argc, vp);
    args.rval().set(JS::NullValue());
	auto len = args.length();
	if (len < 1) {
		return false;
	}
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
	auto &v2 = args[0];
#else
#endif
	if (!v2.isObject()) {
		return false;
	}
	auto &params = v2.toObject();
	JS::RootedObject rparams(cx, &params);

	JS::RootedValue v(cx);
	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
	JS_GetProperty(cx, jsthis, "id", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto instanceId = JavascriptManager::getInt32(v);
	JS_GetProperty(cx, jsthis, "objectId", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto objectId = JavascriptManager::getInt32(v);
	auto scene = GameManager::getInstance()->getCurrentScene();
	agtk::Object *objectInstance = scene->getObjectInstance(objectId, instanceId);
	if (!objectInstance) {
		return true;
	}

	auto linkConditionData = agtk::data::ObjectActionLinkConditionObjectFacingEachOtherData::create(cx, &params);
	auto currentObjectAction = objectInstance->getCurrentObjectAction();
	auto result = currentObjectAction->checkLinkConditionObjectFacingEachOther(linkConditionData);


	args.rval().set(JS::BooleanValue(result));
	return true;
}

bool js_objectInstance_isObjectFacing(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//ret: 成立しているか
	JS::CallArgs args = CallArgsFromVp(argc, vp);
    args.rval().set(JS::NullValue());
	auto len = args.length();
	if (len < 1) {
		return false;
	}
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
	auto &v2 = args[0];
#else
#endif
	if (!v2.isObject()) {
		return false;
	}
	auto &params = v2.toObject();
	JS::RootedObject rparams(cx, &params);

	JS::RootedValue v(cx);
	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
	JS_GetProperty(cx, jsthis, "id", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto instanceId = JavascriptManager::getInt32(v);
	JS_GetProperty(cx, jsthis, "objectId", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto objectId = JavascriptManager::getInt32(v);
	auto scene = GameManager::getInstance()->getCurrentScene();
	agtk::Object *objectInstance = scene->getObjectInstance(objectId, instanceId);
	if (!objectInstance) {
		return true;
	}

	auto linkConditionData = agtk::data::ObjectActionLinkConditionObjectFacingData::create(cx, &params);
	auto currentObjectAction = objectInstance->getCurrentObjectAction();
	auto result = currentObjectAction->checkLinkConditionObjectFacing(linkConditionData);


	args.rval().set(JS::BooleanValue(result));
	return true;
}

bool js_objectInstance_isObjectFound(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//ret: 成立しているか
	JS::CallArgs args = CallArgsFromVp(argc, vp);
    args.rval().set(JS::NullValue());
	auto len = args.length();
	if (len < 1) {
		return false;
	}
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
	auto &v2 = args[0];
#else
#endif
	if (!v2.isObject()) {
		return false;
	}
	auto &params = v2.toObject();
	JS::RootedObject rparams(cx, &params);

	JS::RootedValue v(cx);
	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
	JS_GetProperty(cx, jsthis, "id", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto instanceId = JavascriptManager::getInt32(v);
	JS_GetProperty(cx, jsthis, "objectId", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto objectId = JavascriptManager::getInt32(v);
	auto scene = GameManager::getInstance()->getCurrentScene();
	agtk::Object *objectInstance = scene->getObjectInstance(objectId, instanceId);
	if (!objectInstance) {
		return true;
	}

	auto linkConditionData = agtk::data::ObjectActionLinkConditionObjectFoundData::create(cx, &params);
	auto currentObjectAction = objectInstance->getCurrentObjectAction();
	auto result = currentObjectAction->checkLinkConditionObjectFound(linkConditionData);


	args.rval().set(JS::BooleanValue(result));
	return true;
}

bool js_objectInstance_isObjectFacingDirection(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//ret: 成立しているか
	JS::CallArgs args = CallArgsFromVp(argc, vp);
    args.rval().set(JS::NullValue());
	auto len = args.length();
	if (len < 1) {
		return false;
	}
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
	auto &v2 = args[0];
#else
#endif
	if (!v2.isObject()) {
		return false;
	}
	auto &params = v2.toObject();
	JS::RootedObject rparams(cx, &params);

	JS::RootedValue v(cx);
	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
	JS_GetProperty(cx, jsthis, "id", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto instanceId = JavascriptManager::getInt32(v);
	JS_GetProperty(cx, jsthis, "objectId", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto objectId = JavascriptManager::getInt32(v);
	auto scene = GameManager::getInstance()->getCurrentScene();
	agtk::Object *objectInstance = scene->getObjectInstance(objectId, instanceId);
	if (!objectInstance) {
		return true;
	}

	auto linkConditionData = agtk::data::ObjectActionLinkConditionObjectFacingDirectionData::create(cx, &params);
	auto currentObjectAction = objectInstance->getCurrentObjectAction();
	auto result = currentObjectAction->checkLinkConditionObjectFacingDirection(linkConditionData);


	args.rval().set(JS::BooleanValue(result));
	return true;
}

bool js_objectInstance_isHpZero(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//ret: 成立しているか
	JS::CallArgs args = CallArgsFromVp(argc, vp);
    args.rval().set(JS::NullValue());
	auto len = args.length();
	if (len < 1) {
		return false;
	}
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
	auto &v2 = args[0];
#else
#endif
	if (!v2.isObject()) {
		return false;
	}
	auto &params = v2.toObject();
	JS::RootedObject rparams(cx, &params);

	JS::RootedValue v(cx);
	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
	JS_GetProperty(cx, jsthis, "id", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto instanceId = JavascriptManager::getInt32(v);
	JS_GetProperty(cx, jsthis, "objectId", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto objectId = JavascriptManager::getInt32(v);
	auto scene = GameManager::getInstance()->getCurrentScene();
	agtk::Object *objectInstance = scene->getObjectInstance(objectId, instanceId);
	if (!objectInstance) {
		return true;
	}

	auto linkConditionData = agtk::data::ObjectActionLinkConditionHpZeroData::create(cx, &params);
	auto currentObjectAction = objectInstance->getCurrentObjectAction();
	auto result = currentObjectAction->checkLinkConditionHpZero(linkConditionData);


	args.rval().set(JS::BooleanValue(result));
	return true;
}

bool js_objectInstance_isCameraOutOfRange(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//ret: 成立しているか
	JS::CallArgs args = CallArgsFromVp(argc, vp);
    args.rval().set(JS::NullValue());
	auto len = args.length();
	if (len < 1) {
		return false;
	}
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
	auto &v2 = args[0];
#else
#endif
	if (!v2.isObject()) {
		return false;
	}
	auto &params = v2.toObject();
	JS::RootedObject rparams(cx, &params);

	JS::RootedValue v(cx);
	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
	JS_GetProperty(cx, jsthis, "id", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto instanceId = JavascriptManager::getInt32(v);
	JS_GetProperty(cx, jsthis, "objectId", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto objectId = JavascriptManager::getInt32(v);
	auto scene = GameManager::getInstance()->getCurrentScene();
	agtk::Object *objectInstance = scene->getObjectInstance(objectId, instanceId);
	if (!objectInstance) {
		return true;
	}

	auto linkConditionData = agtk::data::ObjectActionLinkConditionCameraOutOfRangeData::create(cx, &params);
	auto currentObjectAction = objectInstance->getCurrentObjectAction();
	auto result = currentObjectAction->checkLinkConditionCameraOutOfRange(linkConditionData);


	args.rval().set(JS::BooleanValue(result));
	return true;
}

bool js_objectInstance_isLocked(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//ret: 成立しているか
	JS::CallArgs args = CallArgsFromVp(argc, vp);
    args.rval().set(JS::NullValue());
	auto len = args.length();
	if (len < 1) {
		return false;
	}
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
	auto &v2 = args[0];
#else
#endif
	if (!v2.isObject()) {
		return false;
	}
	auto &params = v2.toObject();
	JS::RootedObject rparams(cx, &params);

	JS::RootedValue v(cx);
	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
	JS_GetProperty(cx, jsthis, "id", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto instanceId = JavascriptManager::getInt32(v);
	JS_GetProperty(cx, jsthis, "objectId", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto objectId = JavascriptManager::getInt32(v);
	auto scene = GameManager::getInstance()->getCurrentScene();
	agtk::Object *objectInstance = scene->getObjectInstance(objectId, instanceId);
	if (!objectInstance) {
		return true;
	}

	auto linkConditionData = agtk::data::ObjectActionLinkConditionLockedData::create(cx, &params);
	auto currentObjectAction = objectInstance->getCurrentObjectAction();
	auto result = currentObjectAction->checkLinkConditionLocked(linkConditionData);


	args.rval().set(JS::BooleanValue(result));
	return true;
}

bool js_objectInstance_isProbability(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//ret: 成立しているか
	JS::CallArgs args = CallArgsFromVp(argc, vp);
    args.rval().set(JS::NullValue());
	auto len = args.length();
	if (len < 1) {
		return false;
	}
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
	auto &v2 = args[0];
#else
#endif
	if (!v2.isObject()) {
		return false;
	}
	auto &params = v2.toObject();
	JS::RootedObject rparams(cx, &params);

	JS::RootedValue v(cx);
	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
	JS_GetProperty(cx, jsthis, "id", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto instanceId = JavascriptManager::getInt32(v);
	JS_GetProperty(cx, jsthis, "objectId", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto objectId = JavascriptManager::getInt32(v);
	auto scene = GameManager::getInstance()->getCurrentScene();
	agtk::Object *objectInstance = scene->getObjectInstance(objectId, instanceId);
	if (!objectInstance) {
		return true;
	}

	auto linkConditionData = agtk::data::ObjectActionLinkConditionProbabilityData::create(cx, &params);
	auto currentObjectAction = objectInstance->getCurrentObjectAction();
	auto result = currentObjectAction->checkLinkConditionProbability(linkConditionData);


	args.rval().set(JS::BooleanValue(result));
	return true;
}

bool js_objectInstance_isSwitchVariableChanged(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//ret: 成立しているか
	JS::CallArgs args = CallArgsFromVp(argc, vp);
    args.rval().set(JS::NullValue());
	auto len = args.length();
	if (len < 1) {
		return false;
	}
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
	auto &v2 = args[0];
#else
#endif
	if (!v2.isObject()) {
		return false;
	}
	auto &params = v2.toObject();
	JS::RootedObject rparams(cx, &params);

	JS::RootedValue v(cx);
	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
	JS_GetProperty(cx, jsthis, "id", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto instanceId = JavascriptManager::getInt32(v);
	JS_GetProperty(cx, jsthis, "objectId", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto objectId = JavascriptManager::getInt32(v);
	auto scene = GameManager::getInstance()->getCurrentScene();
	agtk::Object *objectInstance = scene->getObjectInstance(objectId, instanceId);
	if (!objectInstance) {
		return true;
	}

	auto linkConditionData = agtk::data::ObjectActionLinkConditionSwitchVariableChangedData::create(cx, &params);
	auto currentObjectAction = objectInstance->getCurrentObjectAction();
	auto result = currentObjectAction->checkLinkConditionSwitchVariableChanged(linkConditionData);


	args.rval().set(JS::BooleanValue(result));
	return true;
}

bool js_objectInstance_isAnimationFinished(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//ret: 成立しているか
	JS::CallArgs args = CallArgsFromVp(argc, vp);
    args.rval().set(JS::NullValue());

	JS::RootedValue v(cx);
	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
	JS_GetProperty(cx, jsthis, "id", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto instanceId = JavascriptManager::getInt32(v);
	JS_GetProperty(cx, jsthis, "objectId", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto objectId = JavascriptManager::getInt32(v);
	auto scene = GameManager::getInstance()->getCurrentScene();
	agtk::Object *objectInstance = scene->getObjectInstance(objectId, instanceId);
	if (!objectInstance) {
		return true;
	}

	auto linkConditionData = agtk::data::ObjectActionLinkConditionAnimationFinishedData::create(cx, nullptr);
	auto currentObjectAction = objectInstance->getCurrentObjectAction();
	auto result = currentObjectAction->checkLinkConditionAnimationFinished(linkConditionData);


	args.rval().set(JS::BooleanValue(result));
	return true;
}

bool js_objectInstance_isJumpTop(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//ret: 成立しているか
	JS::CallArgs args = CallArgsFromVp(argc, vp);
    args.rval().set(JS::NullValue());

	JS::RootedValue v(cx);
	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
	JS_GetProperty(cx, jsthis, "id", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto instanceId = JavascriptManager::getInt32(v);
	JS_GetProperty(cx, jsthis, "objectId", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto objectId = JavascriptManager::getInt32(v);
	auto scene = GameManager::getInstance()->getCurrentScene();
	agtk::Object *objectInstance = scene->getObjectInstance(objectId, instanceId);
	if (!objectInstance) {
		return true;
	}

	auto linkConditionData = agtk::data::ObjectActionLinkConditionJumpTopData::create(cx, nullptr);
	auto currentObjectAction = objectInstance->getCurrentObjectAction();
	auto result = currentObjectAction->checkLinkConditionJumpTop(linkConditionData);


	args.rval().set(JS::BooleanValue(result));
	return true;
}

bool js_objectInstance_isObjectActionChanged(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//ret: 成立しているか
	JS::CallArgs args = CallArgsFromVp(argc, vp);
    args.rval().set(JS::NullValue());
	auto len = args.length();
	if (len < 1) {
		return false;
	}
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
	auto &v2 = args[0];
#else
#endif
	if (!v2.isObject()) {
		return false;
	}
	auto &params = v2.toObject();
	JS::RootedObject rparams(cx, &params);

	JS::RootedValue v(cx);
	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
	JS_GetProperty(cx, jsthis, "id", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto instanceId = JavascriptManager::getInt32(v);
	JS_GetProperty(cx, jsthis, "objectId", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto objectId = JavascriptManager::getInt32(v);
	auto scene = GameManager::getInstance()->getCurrentScene();
	agtk::Object *objectInstance = scene->getObjectInstance(objectId, instanceId);
	if (!objectInstance) {
		return true;
	}

	auto linkConditionData = agtk::data::ObjectActionLinkConditionObjectActionChangedData::create(cx, &params);
	auto currentObjectAction = objectInstance->getCurrentObjectAction();
	auto result = currentObjectAction->checkLinkConditionObjectActionChanged(linkConditionData);


	args.rval().set(JS::BooleanValue(result));
	return true;
}

bool js_objectInstance_isSlopeTouched(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//ret: 成立しているか
	JS::CallArgs args = CallArgsFromVp(argc, vp);
    args.rval().set(JS::NullValue());
	auto len = args.length();
	if (len < 1) {
		return false;
	}
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
	auto &v2 = args[0];
#else
#endif
	if (!v2.isObject()) {
		return false;
	}
	auto &params = v2.toObject();
	JS::RootedObject rparams(cx, &params);

	JS::RootedValue v(cx);
	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
	JS_GetProperty(cx, jsthis, "id", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto instanceId = JavascriptManager::getInt32(v);
	JS_GetProperty(cx, jsthis, "objectId", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto objectId = JavascriptManager::getInt32(v);
	auto scene = GameManager::getInstance()->getCurrentScene();
	agtk::Object *objectInstance = scene->getObjectInstance(objectId, instanceId);
	if (!objectInstance) {
		return true;
	}

	auto linkConditionData = agtk::data::ObjectActionLinkConditionSlopeTouchedData::create(cx, &params);
	auto currentObjectAction = objectInstance->getCurrentObjectAction();
	auto result = currentObjectAction->checkLinkConditionSlopeTouched(linkConditionData);


	args.rval().set(JS::BooleanValue(result));
	return true;
}

bool js_objectInstance_isBuriedInWall(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//ret: 成立しているか
	JS::CallArgs args = CallArgsFromVp(argc, vp);
    args.rval().set(JS::NullValue());
	auto len = args.length();
	if (len < 1) {
		return false;
	}
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
	auto &v2 = args[0];
#else
#endif
	if (!v2.isObject()) {
		return false;
	}
	auto &params = v2.toObject();
	JS::RootedObject rparams(cx, &params);

	JS::RootedValue v(cx);
	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
	JS_GetProperty(cx, jsthis, "id", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto instanceId = JavascriptManager::getInt32(v);
	JS_GetProperty(cx, jsthis, "objectId", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto objectId = JavascriptManager::getInt32(v);
	auto scene = GameManager::getInstance()->getCurrentScene();
	agtk::Object *objectInstance = scene->getObjectInstance(objectId, instanceId);
	if (!objectInstance) {
		return true;
	}

	auto linkConditionData = agtk::data::ObjectActionLinkConditionBuriedInWallData::create(cx, &params);
	auto currentObjectAction = objectInstance->getCurrentObjectAction();
	auto result = currentObjectAction->checkLinkConditionBuriedInWallData(linkConditionData);

	args.rval().set(JS::BooleanValue(result));
	return true;
}

bool js_objectInstance_isObjectHit(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//ret: 成立しているか
	JS::CallArgs args = CallArgsFromVp(argc, vp);
    args.rval().set(JS::NullValue());
	auto len = args.length();
	if (len < 1) {
		return false;
	}
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
	auto &v2 = args[0];
#else
#endif
	if (!v2.isObject()) {
		return false;
	}
	auto &params = v2.toObject();
	JS::RootedObject rparams(cx, &params);

	JS::RootedValue v(cx);
	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
	JS_GetProperty(cx, jsthis, "id", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto instanceId = JavascriptManager::getInt32(v);
	JS_GetProperty(cx, jsthis, "objectId", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto objectId = JavascriptManager::getInt32(v);
	auto scene = GameManager::getInstance()->getCurrentScene();
	agtk::Object *objectInstance = scene->getObjectInstance(objectId, instanceId);
	if (!objectInstance) {
		return true;
	}

	auto linkConditionData = agtk::data::ObjectActionLinkConditionObjectHitData::create(cx, &params);
	auto currentObjectAction = objectInstance->getCurrentObjectAction();
	auto result = currentObjectAction->checkLinkConditionObjectHit(linkConditionData);


	args.rval().set(JS::BooleanValue(result));
	return true;
}

#if 0
bool js_objectInstance_getValue(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//ret: <bool : 値>
	JS::CallArgs args = CallArgsFromVp(argc, vp);
    args.rval().set(JS::NullValue());

	JS::RootedValue v(cx);
	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
	JS_GetProperty(cx, jsthis, "objectInstanceId", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto objectInstanceId = JavascriptManager::getInt32(v);

	auto projectPlayData = GameManager::getInstance()->getPlayData();
	auto playSwitchData = projectPlayData->getSwitchData(agtk::data::PlayData::COMMON_PLAY_DATA_ID, switchId);
	if (playSwitchData) {
		auto value = playSwitchData->getValue();
		args.rval().set(JS::BooleanValue(value));
	}
	else {
		args.rval().set(JS::NullValue());
	}
	return true;
}

bool js_objectInstance_setValue(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//arg1: <bool : 設定値>
	JS::CallArgs args = CallArgsFromVp(argc, vp);
    args.rval().set(JS::NullValue());
	auto len = args.length();
	if (len < 1) {
		return false;
	}
	bool value = 0;
	{
		auto &v2 = args[0];
		if (v2.isBoolean()) {
			value = v2.toBoolean();
		} else if (v2.isInt32()) {
			value = v2.toInt32() != 0;
		} else {
			return false;
		}
	}

	JS::RootedValue v(cx);
	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
	JS_GetProperty(cx, jsthis, "objectInstanceId", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto objectInstanceId = JavascriptManager::getInt32(v);

	auto projectPlayData = GameManager::getInstance()->getPlayData();
	auto playSwitchData = projectPlayData->getSwitchData(agtk::data::PlayData::COMMON_PLAY_DATA_ID, switchId);
	if (playSwitchData) {
		playSwitchData->setValue(value);
		args.rval().set(JS::TrueValue());
	}
	else {
		args.rval().set(JS::NullValue());
	}
	return true;
}
#endif

// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_7
bool jsb_register_agtk_objectInstanceBase_switches(JSContext *cx, JSObject *agtkObj)
#else
bool jsb_register_agtk_objectInstance_switches(JSContext *cx, JSObject *agtkObj)
#endif
{
	JS::RootedObject robj(cx, agtkObj);

	static JSClass switches_class = {
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_7
		"switchesBase",
#else
		"switches",
#endif
		0,
		/* All of these can be replaced with the corresponding JS_*Stub
		function pointers. */
		JS_PropertyStub,
		JS_DeletePropertyStub,
		JS_PropertyStub,
		JS_StrictPropertyStub,
		JS_EnumerateStub,
		JS_ResolveStub,
		JS_ConvertStub,
		nullptr, nullptr, nullptr, nullptr, nullptr
	};
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_7
	auto switches = JS_DefineObject(cx, robj, "switchesBase", &switches_class, JS::NullPtr(), JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE);
#else
	auto switches = JS_DefineObject(cx, robj, "switches", &switches_class, JS::NullPtr(), JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE);
#endif
	CCASSERT(switches, "switches should not be nullptr");
	JS::RootedObject rswitches(cx, switches);
	static JSFunctionSpec switches_methods[] = {
		{ "get", js_switches_get, 0, 0, 0 },
		{ "getIdByName", js_switches_getIdByName, 0, 0, 0 },
		{ NULL },
	};
	JS_DefineFunctions(cx, rswitches, switches_methods);
	{
		JS::RootedValue rval(cx);
		rval.setInt32(agtk::data::kObjectSystemSwitchInvincible);
		JS_SetProperty(cx, rswitches, "InvincibleId", rval);
		rval.setInt32(agtk::data::kObjectSystemSwitchFreeMove);
		JS_SetProperty(cx, rswitches, "FreeMoveId", rval);
		rval.setInt32(agtk::data::kObjectSystemSwitchLockTarget);
		JS_SetProperty(cx, rswitches, "LockTargetId", rval);
		rval.setInt32(agtk::data::kObjectSystemSwitchPortalTouched);
		JS_SetProperty(cx, rswitches, "PortalTouchedId", rval);
		rval.setInt32(agtk::data::kObjectSystemSwitchCriticalDamaged);
		JS_SetProperty(cx, rswitches, "CriticalDamagedId", rval);
		rval.setInt32(agtk::data::kObjectSystemSwitchDisabled);
		JS_SetProperty(cx, rswitches, "DisabledId", rval);
		rval.setInt32(agtk::data::kObjectSystemSwitchSlipOnSlope);
		JS_SetProperty(cx, rswitches, "SlipOnSlopeId", rval);
		rval.setInt32(agtk::data::kObjectSystemSwitchAffectOtherObjects);
		JS_SetProperty(cx, rswitches, "AffectOtherObjectsId", rval);
		rval.setInt32(agtk::data::kObjectSystemSwitchAffectedByOtherObjects);
		JS_SetProperty(cx, rswitches, "AffectedByOtherObjectsId", rval);
		rval.setInt32(agtk::data::kObjectSystemSwitchFollowConnectedPhysics);
		JS_SetProperty(cx, rswitches, "FollowConnectedPhysicsId", rval);
		rval.setInt32(agtk::data::kObjectSystemSwitchDisplayAfterimage);
		JS_SetProperty(cx, rswitches, "DisplayAfterimageId", rval);
	}

// #AGTK-NX
#ifndef USE_SAR_OPTIMIZE_7
	JS::RootedValue rval(cx);
	rval.setObject(*switches);
	JS_SetProperty(cx, robj, "switches", rval);
#if 1
	{
		JS::RootedValue v(cx);
		JS_GetProperty(cx, robj, "id", &v);
		if (!v.isNumber()) {
			return false;
		}
		auto objectInstanceId = JavascriptManager::getInt32(v);
		JS::RootedValue robjectInstanceId(cx, JS::Int32Value(objectInstanceId));
		//JS_DefineProperty(cx, rvariables, "instanceId", robjectInstanceId, JSPROP_ENUMERATE | JSPROP_PERMANENT);
		JS_DefineProperty(cx, rswitches, "instanceId", v, JSPROP_ENUMERATE | JSPROP_PERMANENT);

		JS_GetProperty(cx, robj, "objectId", &v);
		if (!v.isNumber()) {
			return false;
		}
		JS_DefineProperty(cx, rswitches, "objectId", v, JSPROP_ENUMERATE | JSPROP_PERMANENT);
	}
#endif
#endif
	return true;
}

// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_7
bool jsb_register_agtk_objectInstance_switches(JSContext *cx, JSObject *agtkObj)
{
	JS::RootedObject robj(cx, agtkObj);

	static JSClass switches_class = {
		"switches",
		0,
		/* All of these can be replaced with the corresponding JS_*Stub
		function pointers. */
		JS_PropertyStub,
		JS_DeletePropertyStub,
		JS_PropertyStub,
		JS_StrictPropertyStub,
		JS_EnumerateStub,
		JS_ResolveStub,
		JS_ConvertStub,
		nullptr, nullptr, nullptr, nullptr, nullptr
	};

	auto sc = ScriptingCore::getInstance();
	auto _global = sc->getGlobalObject();
	JS::RootedObject gobj(cx, _global);
	JSAutoCompartment ac(cx, gobj);

	JS::RootedObject ns(cx);
	JS::MutableHandleObject jsObj = &ns;
	JS::RootedValue nsval(cx);
	JS_GetProperty(cx, gobj, "Agtk", &nsval);
	JS::RootedValue rv(cx);
	bool ret = false;
	if (nsval == JSVAL_VOID) {
		return false;
	}
	jsObj.set(nsval.toObjectOrNull());
	JS::RootedValue rswitchesBaseVal(cx);
	JS_GetProperty(cx, jsObj, "switchesBase", &rswitchesBaseVal);
	if (!rswitchesBaseVal.isObject()) {
		return false;
	}

	JSObject* switchesBase = rswitchesBaseVal.get().toObjectOrNull();
	JS::RootedObject rswitchesBase(cx, switchesBase);

	auto switches = JS_DefineObject(cx, robj, "switches", &switches_class, rswitchesBase, JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE);
	CCASSERT(switches, "switches should not be nullptr");
	JS::RootedObject rswitches(cx, switches);
	{
		JS::RootedValue v(cx);
		JS_GetProperty(cx, robj, "id", &v);
		if (!v.isNumber()) {
			return false;
		}
		auto objectInstanceId = JavascriptManager::getInt32(v);
		JS::RootedValue robjectInstanceId(cx, JS::Int32Value(objectInstanceId));
		//JS_DefineProperty(cx, rvariables, "instanceId", robjectInstanceId, JSPROP_ENUMERATE | JSPROP_PERMANENT);
		JS_DefineProperty(cx, rswitches, "instanceId", v, JSPROP_ENUMERATE | JSPROP_PERMANENT);

		JS_GetProperty(cx, robj, "objectId", &v);
		if (!v.isNumber()) {
			return false;
		}
		JS_DefineProperty(cx, rswitches, "objectId", v, JSPROP_ENUMERATE | JSPROP_PERMANENT);
	}
	return true;
}
#endif

// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_7
bool jsb_register_agtk_objectInstanceBase_variables(JSContext *cx, JSObject *agtkObj)
#else
bool jsb_register_agtk_objectInstance_variables(JSContext *cx, JSObject *agtkObj)
#endif
{
	JS::RootedObject robj(cx, agtkObj);

	static JSClass variables_class = {
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_7
		"variablesBase",
#else
		"variables",
#endif
		0,
		/* All of these can be replaced with the corresponding JS_*Stub
		function pointers. */
		JS_PropertyStub,
		JS_DeletePropertyStub,
		JS_PropertyStub,
		JS_StrictPropertyStub,
		JS_EnumerateStub,
		JS_ResolveStub,
		JS_ConvertStub,
		nullptr, nullptr, nullptr, nullptr, nullptr
	};
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_7
	auto variables = JS_DefineObject(cx, robj, "variablesBase", &variables_class, JS::NullPtr(), JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE);
#else
	auto variables = JS_DefineObject(cx, robj, "variables", &variables_class, JS::NullPtr(), JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE);
#endif
	CCASSERT(variables, "variables should not be nullptr");
	JS::RootedObject rvariables(cx, variables);
	static JSFunctionSpec variables_methods[] = {
		{ "get", js_variables_get, 0, 0, 0 },
		{ "getIdByName", js_variables_getIdByName, 0, 0, 0 },
		{ NULL },
	};
	JS_DefineFunctions(cx, rvariables, variables_methods);
	{
		JS::RootedValue rval(cx);
		rval.setInt32(agtk::data::kObjectSystemVariableObjectID);
		JS_SetProperty(cx, rvariables, "ObjectIDId", rval);
		rval.setInt32(agtk::data::kObjectSystemVariableHP);
		JS_SetProperty(cx, rvariables, "HPId", rval);
		rval.setInt32(agtk::data::kObjectSystemVariableMaxHP);
		JS_SetProperty(cx, rvariables, "MaxHPId", rval);
		rval.setInt32(agtk::data::kObjectSystemVariableMinimumAttack);
		JS_SetProperty(cx, rvariables, "MinimumAttackId", rval);
		rval.setInt32(agtk::data::kObjectSystemVariableMaximumAttack);
		JS_SetProperty(cx, rvariables, "MaximumAttackId", rval);
		rval.setInt32(agtk::data::kObjectSystemVariableDamageRatio);
		JS_SetProperty(cx, rvariables, "DamageRatioId", rval);
		rval.setInt32(agtk::data::kObjectSystemVariableDamageVariationValue);
		JS_SetProperty(cx, rvariables, "DamageVariationValueId", rval);
		rval.setInt32(agtk::data::kObjectSystemVariableAttackAttribute);
		JS_SetProperty(cx, rvariables, "AttackAttributeId", rval);
		rval.setInt32(agtk::data::kObjectSystemVariableAreaAttribute);
		JS_SetProperty(cx, rvariables, "AreaAttributeId", rval);
		rval.setInt32(agtk::data::kObjectSystemVariableX);
		JS_SetProperty(cx, rvariables, "XId", rval);
		rval.setInt32(agtk::data::kObjectSystemVariableY);
		JS_SetProperty(cx, rvariables, "YId", rval);
		rval.setInt32(agtk::data::kObjectSystemVariableDisplayDirection);
		JS_SetProperty(cx, rvariables, "DisplayDirectionId", rval);
		rval.setInt32(agtk::data::kObjectSystemVariableParentObjectInstanceID);
		JS_SetProperty(cx, rvariables, "ParentObjectInstanceIDId", rval);
		rval.setInt32(agtk::data::kObjectSystemVariableVelocityX);
		JS_SetProperty(cx, rvariables, "VelocityXId", rval);
		rval.setInt32(agtk::data::kObjectSystemVariableVelocityY);
		JS_SetProperty(cx, rvariables, "VelocityYId", rval);
		rval.setInt32(agtk::data::kObjectSystemVariablePlayerID);
		JS_SetProperty(cx, rvariables, "PlayerIDId", rval);
		rval.setInt32(agtk::data::kObjectSystemVariableDamageValue);
		JS_SetProperty(cx, rvariables, "DamageValueId", rval);
		rval.setInt32(agtk::data::kObjectSystemVariableCriticalRatio);
		JS_SetProperty(cx, rvariables, "CriticalRatioId", rval);
		rval.setInt32(agtk::data::kObjectSystemVariableCriticalIncidence);
		JS_SetProperty(cx, rvariables, "CriticalIncidenceId", rval);
		rval.setInt32(agtk::data::kObjectSystemVariableInvincibleDuration);
		JS_SetProperty(cx, rvariables, "InvincibleDurationId", rval);
		rval.setInt32(agtk::data::kObjectSystemVariableFixedAnimationFrame);
		JS_SetProperty(cx, rvariables, "FixedAnimationFrameId", rval);
		rval.setInt32(agtk::data::kObjectSystemVariableInstanceID);
		JS_SetProperty(cx, rvariables, "InstanceIDId", rval);
		rval.setInt32(agtk::data::kObjectSystemVariableInstanceCount);
		JS_SetProperty(cx, rvariables, "InstanceCountId", rval);
		rval.setInt32(agtk::data::kObjectSystemVariableSingleInstanceID);
		JS_SetProperty(cx, rvariables, "SingleInstanceIDId", rval);
		rval.setInt32(agtk::data::kObjectSystemVariableControllerID);
		JS_SetProperty(cx, rvariables, "ControllerIDId", rval);
		rval.setInt32(agtk::data::kObjectSystemVariableHorizontalMove);
		JS_SetProperty(cx, rvariables, "HorizontalMoveId", rval);
		rval.setInt32(agtk::data::kObjectSystemVariableVerticalMove);
		JS_SetProperty(cx, rvariables, "VerticalMoveId", rval);
		rval.setInt32(agtk::data::kObjectSystemVariableHorizontalAccel);
		JS_SetProperty(cx, rvariables, "HorizontalAccelId", rval);
		rval.setInt32(agtk::data::kObjectSystemVariableVerticalAccel);
		JS_SetProperty(cx, rvariables, "VerticalAccelId", rval);
		rval.setInt32(agtk::data::kObjectSystemVariableHorizontalMaxMove);
		JS_SetProperty(cx, rvariables, "HorizontalMaxMoveId", rval);
		rval.setInt32(agtk::data::kObjectSystemVariableVerticalMaxMove);
		JS_SetProperty(cx, rvariables, "VerticalMaxMoveId", rval);
		rval.setInt32(agtk::data::kObjectSystemVariableHorizontalDecel);
		JS_SetProperty(cx, rvariables, "HorizontalDecelId", rval);
		rval.setInt32(agtk::data::kObjectSystemVariableVerticalDecel);
		JS_SetProperty(cx, rvariables, "VerticalDecelId", rval);
		rval.setInt32(agtk::data::kObjectSystemVariableDurationToTakeOverAccelerationMoveSpeed);
		JS_SetProperty(cx, rvariables, "DurationToTakeOverAccelerationMoveSpeedId", rval);
		rval.setInt32(agtk::data::kObjectSystemVariableScalingX);
		JS_SetProperty(cx, rvariables, "ScalingXId", rval);
		rval.setInt32(agtk::data::kObjectSystemVariableScalingY);
		JS_SetProperty(cx, rvariables, "ScalingYId", rval);
		rval.setInt32(agtk::data::kObjectSystemVariableDispPriority);
		JS_SetProperty(cx, rvariables, "DispPriorityId", rval);
		rval.setInt32(agtk::data::kObjectSystemVariableInitialJumpSpeed);
		JS_SetProperty(cx, rvariables, "InitialJumpSpeedId", rval);
	}

// #AGTK-NX
#ifndef USE_SAR_OPTIMIZE_7
	JS::RootedValue rval(cx);
	rval.setObject(*variables);
	JS_SetProperty(cx, robj, "variables", rval);
#if 1
	{
		JS::RootedValue v(cx);
		JS_GetProperty(cx, robj, "id", &v);
		if (!v.isNumber()) {
			return false;
		}
		auto objectInstanceId = JavascriptManager::getInt32(v);
		JS::RootedValue robjectInstanceId(cx, JS::Int32Value(objectInstanceId));
		//JS_DefineProperty(cx, rvariables, "instanceId", robjectInstanceId, JSPROP_ENUMERATE | JSPROP_PERMANENT);
		JS_DefineProperty(cx, rvariables, "instanceId", v, JSPROP_ENUMERATE | JSPROP_PERMANENT);

		JS_GetProperty(cx, robj, "objectId", &v);
		if (!v.isNumber()) {
			return false;
		}
		JS_DefineProperty(cx, rvariables, "objectId", v, JSPROP_ENUMERATE | JSPROP_PERMANENT);
	}
#endif
#endif
	return true;
}

// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_7
bool jsb_register_agtk_objectInstance_variables(JSContext *cx, JSObject *agtkObj)
{
	JS::RootedObject robj(cx, agtkObj);

	static JSClass variables_class = {
		"variables",
		0,
		/* All of these can be replaced with the corresponding JS_*Stub
		function pointers. */
		JS_PropertyStub,
		JS_DeletePropertyStub,
		JS_PropertyStub,
		JS_StrictPropertyStub,
		JS_EnumerateStub,
		JS_ResolveStub,
		JS_ConvertStub,
		nullptr, nullptr, nullptr, nullptr, nullptr
	};

	auto sc = ScriptingCore::getInstance();
	auto _global = sc->getGlobalObject();
	JS::RootedObject gobj(cx, _global);
	JSAutoCompartment ac(cx, gobj);

	JS::RootedObject ns(cx);
	JS::MutableHandleObject jsObj = &ns;
	JS::RootedValue nsval(cx);
	JS_GetProperty(cx, gobj, "Agtk", &nsval);
	JS::RootedValue rv(cx);
	bool ret = false;
	if (nsval == JSVAL_VOID) {
		return false;
	}
	jsObj.set(nsval.toObjectOrNull());
	JS::RootedValue rvariablesBaseVal(cx);
	JS_GetProperty(cx, jsObj, "variablesBase", &rvariablesBaseVal);
	if (!rvariablesBaseVal.isObject()) {
		return false;
	}

	JSObject* variablesBase = rvariablesBaseVal.get().toObjectOrNull();
	JS::RootedObject rvariablesBase(cx, variablesBase);

	auto variables = JS_DefineObject(cx, robj, "variables", &variables_class, rvariablesBase, JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE);
	CCASSERT(variables, "variables should not be nullptr");
	JS::RootedObject rvariables(cx, variables);
	{
		JS::RootedValue v(cx);
		JS_GetProperty(cx, robj, "id", &v);
		if (!v.isNumber()) {
			return false;
		}
		auto objectInstanceId = JavascriptManager::getInt32(v);
		JS::RootedValue robjectInstanceId(cx, JS::Int32Value(objectInstanceId));
		JS_DefineProperty(cx, rvariables, "instanceId", v, JSPROP_ENUMERATE | JSPROP_PERMANENT);

		JS_GetProperty(cx, robj, "objectId", &v);
		if (!v.isNumber()) {
			return false;
		}
		JS_DefineProperty(cx, rvariables, "objectId", v, JSPROP_ENUMERATE | JSPROP_PERMANENT);
	}
	return true;
}
#endif

// switches
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_7
bool jsb_register_agtk_switchBase(JSContext *cx, JSObject *agtkObj)
{
	JS::RootedObject robj(cx, agtkObj);

	static JSClass switch_class = {
		"switchBase",
		0,
		/* All of these can be replaced with the corresponding JS_*Stub
		function pointers. */
		JS_PropertyStub,
		JS_DeletePropertyStub,
		JS_PropertyStub,
		JS_StrictPropertyStub,
		JS_EnumerateStub,
		JS_ResolveStub,
		JS_ConvertStub,
		nullptr, nullptr, nullptr, nullptr, nullptr
	};
	auto swtch = JS_DefineObject(cx, robj, "switchBase", &switch_class, JS::NullPtr(), JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE);
	CCASSERT(swtch, "switch should not be nullptr");
	JS::RootedObject rswitch(cx, swtch);
	static JSFunctionSpec switch_methods[] = {
		{ "getValue", js_switch_getValue, 0, 0, 0 },
		{ "setValue", js_switch_setValue, 0, 0, 0 },
		{ NULL },
	};
	JS_DefineFunctions(cx, rswitch, switch_methods);
	return true;
}
#endif

bool js_switches_get(JSContext *cx, unsigned argc, JS::Value *vp)
{
	JS::CallArgs args = CallArgsFromVp(argc, vp);
    args.rval().set(JS::NullValue());
	auto len = args.length();
	if (len < 1) {
		return false;
	}
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
	auto &v = args[0];
#else
#endif
	if (!v.isNumber()) {
		return false;
	}
	auto switchId = JavascriptManager::getInt32(v);
	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
	static JSClass switch_class = {
		"switch",
		0,
		/* All of these can be replaced with the corresponding JS_*Stub
		function pointers. */
		JS_PropertyStub,
		JS_DeletePropertyStub,
		JS_PropertyStub,
		JS_StrictPropertyStub,
		JS_EnumerateStub,
		JS_ResolveStub,
		JS_ConvertStub,
		nullptr, nullptr, nullptr, nullptr, nullptr
	};
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_7
	auto sc = ScriptingCore::getInstance();
	auto _global = sc->getGlobalObject();
	JS::RootedObject gobj(cx, _global);
	JSAutoCompartment ac(cx, gobj);

	JS::RootedObject ns(cx);
	JS::MutableHandleObject jsObj = &ns;
	JS::RootedValue nsval(cx);
	JS_GetProperty(cx, gobj, "Agtk", &nsval);
	JS::RootedValue rv(cx);
	bool ret = false;
	if (nsval == JSVAL_VOID) {
		return false;
	}
	jsObj.set(nsval.toObjectOrNull());
	JS::RootedValue rswitchBaseVal(cx);
	JS_GetProperty(cx, jsObj, "switchBase", &rswitchBaseVal);
	if (!rswitchBaseVal.isObject()) {
		return false;
	}

	JSObject* switchBase = rswitchBaseVal.get().toObjectOrNull();
	JS::RootedObject rswitchBase(cx, switchBase);
#endif
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_7
	auto swtch = JS_DefineObject(cx, jsthis, "switch", &switch_class, rswitchBase, JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE);
#else
	auto swtch = JS_DefineObject(cx, jsthis, "switch", &switch_class, JS::NullPtr(), JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE);
#endif
	CCASSERT(swtch, "switch should not be nullptr");
	JS::RootedObject rswitch(cx, swtch);
// #AGTK-NX
#ifndef USE_SAR_OPTIMIZE_7
	static JSFunctionSpec switch_methods[] = {
		{ "getValue", js_switch_getValue, 0, 0, 0 },
		{ "setValue", js_switch_setValue, 0, 0, 0 },
		{ NULL },
	};
	JS_DefineFunctions(cx, rswitch, switch_methods);
#endif
#if 1
	{
		JS::RootedValue v(cx);
		JS_GetProperty(cx, jsthis, "instanceId", &v);
		if (!v.isNumber()) {
			return false;
		}
		JS_DefineProperty(cx, rswitch, "instanceId", v, JSPROP_ENUMERATE | JSPROP_PERMANENT);
		auto instanceId = v.toInt32();
		JS_GetProperty(cx, jsthis, "objectId", &v);
		if (!v.isNumber()) {
			return false;
		}
		JS_DefineProperty(cx, rswitch, "objectId", v, JSPROP_ENUMERATE | JSPROP_PERMANENT);
		auto objectId = v.toInt32();

		auto scene = GameManager::getInstance()->getCurrentScene();
		agtk::Object *objectInstance = scene->getObjectInstance(objectId, instanceId);
		if (!objectInstance) {
			return true;
		}
		auto switchData = objectInstance->getPlayObjectData()->getSwitchData(switchId);
		if (!switchData) {
			return true;
		}
	}
#endif
	JS::RootedValue rval(cx);
	rval.setInt32(switchId);
	JS_SetProperty(cx, rswitch, "switchId", rval);
	args.rval().set(OBJECT_TO_JSVAL(swtch));
	return true;
}

bool js_switches_getIdByName(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//arg1: <string: スイッチ名>
	//ret: <int : スイッチID>
	JS::CallArgs args = CallArgsFromVp(argc, vp);
    args.rval().set(JS::Int32Value(-1));
	auto len = args.length();
	if (len < 1) {
		return false;
	}
	std::string switchName;
	{
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
		auto &v2 = args[0];
#else
#endif
		if (!v2.isString()) {
			return false;
		}
		if (!jsval_to_std_string(cx, v2, &switchName)) {
			return false;
		}
	}

#if 1
	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
	JS::RootedValue v(cx);
	JS_GetProperty(cx, jsthis, "instanceId", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto instanceId = JavascriptManager::getInt32(v);
	JS_GetProperty(cx, jsthis, "objectId", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto objectId = JavascriptManager::getInt32(v);

	auto scene = GameManager::getInstance()->getCurrentScene();
	agtk::Object *objectInstance = scene->getObjectInstance(objectId, instanceId);
	if (!objectInstance) {
		return true;
	}
#endif
	auto switchData = objectInstance->getPlayObjectData()->getSwitchDataByName(switchName.c_str());
	if (!switchData) {
		return true;
	}
	args.rval().set(JS::Int32Value(switchData->getId()));
	return true;
}

// switch
bool js_switch_getValue(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//ret: <bool : 値>
	JS::CallArgs args = CallArgsFromVp(argc, vp);
    args.rval().set(JS::NullValue());

	JS::RootedValue v(cx);
	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
	JS_GetProperty(cx, jsthis, "switchId", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto switchId = JavascriptManager::getInt32(v);
#if 1
	JS_GetProperty(cx, jsthis, "instanceId", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto instanceId = JavascriptManager::getInt32(v);
	JS_GetProperty(cx, jsthis, "objectId", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto objectId = JavascriptManager::getInt32(v);

	auto scene = GameManager::getInstance()->getCurrentScene();
	agtk::Object *objectInstance = scene->getObjectInstance(objectId, instanceId);
	if (!objectInstance) {
		return true;
	}
#endif
	auto switchData = objectInstance->getPlayObjectData()->getSwitchData(switchId);
	if (switchData) {
		auto value = switchData->getValue();
		args.rval().set(JS::BooleanValue(value));
	} else {
		args.rval().set(JS::NullValue());
	}
	return true;
}

bool js_switch_setValue(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//arg1: <bool : 設定値>
	JS::CallArgs args = CallArgsFromVp(argc, vp);
    args.rval().set(JS::NullValue());
	auto len = args.length();
	if (len < 1) {
		return false;
	}
	bool value = 0;
	{
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
		auto &v2 = args[0];
#else
#endif
		if (v2.isBoolean()) {
			value = v2.toBoolean();
		}
		else if (v2.isInt32()) {
			value = v2.toInt32() != 0;
		}
		else {
			return false;
		}
	}

	JS::RootedValue v(cx);
	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
	JS_GetProperty(cx, jsthis, "switchId", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto switchId = JavascriptManager::getInt32(v);
#if 1
	JS_GetProperty(cx, jsthis, "instanceId", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto instanceId = JavascriptManager::getInt32(v);
	JS_GetProperty(cx, jsthis, "objectId", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto objectId = JavascriptManager::getInt32(v);

	auto scene = GameManager::getInstance()->getCurrentScene();
	agtk::Object *objectInstance = scene->getObjectInstance(objectId, instanceId);
	if (!objectInstance) {
		return true;
	}
#endif
	auto switchData = objectInstance->getPlayObjectData()->getSwitchData(switchId);

	if (switchData) {
		switchData->setValue(value);
		args.rval().set(JS::TrueValue());
	}
	else {
		args.rval().set(JS::NullValue());
	}
	return true;
}

// variables
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_7
bool jsb_register_agtk_variableBase(JSContext *cx, JSObject *agtkObj)
{
	JS::RootedObject robj(cx, agtkObj);

	static JSClass variable_class = {
		"variableBase",
		0,
		/* All of these can be replaced with the corresponding JS_*Stub
		function pointers. */
		JS_PropertyStub,
		JS_DeletePropertyStub,
		JS_PropertyStub,
		JS_StrictPropertyStub,
		JS_EnumerateStub,
		JS_ResolveStub,
		JS_ConvertStub,
		nullptr, nullptr, nullptr, nullptr, nullptr
	};
	auto variable = JS_DefineObject(cx, robj, "variableBase", &variable_class, JS::NullPtr(), JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE);
	CCASSERT(variable, "variable should not be nullptr");
	JS::RootedObject rvariable(cx, variable);
	static JSFunctionSpec variable_methods[] = {
		{ "getValue", js_variable_getValue, 0, 0, 0 },
		{ "setValue", js_variable_setValue, 0, 0, 0 },
		{ NULL },
	};
	JS_DefineFunctions(cx, rvariable, variable_methods);
	return true;
}
#endif

bool js_variables_get(JSContext *cx, unsigned argc, JS::Value *vp)
{
	JS::CallArgs args = CallArgsFromVp(argc, vp);
    args.rval().set(JS::NullValue());
	auto len = args.length();
	if (len < 1) {
		return false;
	}
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
	auto &v = args[0];
#else
#endif
	if (!v.isNumber()) {
		return false;
	}
	auto variableId = JavascriptManager::getInt32(v);
	static JSClass variable_class = {
		"variable",
		0,
		/* All of these can be replaced with the corresponding JS_*Stub
		function pointers. */
		JS_PropertyStub,
		JS_DeletePropertyStub,
		JS_PropertyStub,
		JS_StrictPropertyStub,
		JS_EnumerateStub,
		JS_ResolveStub,
		JS_ConvertStub,
		nullptr, nullptr, nullptr, nullptr, nullptr
	};
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_7
	auto sc = ScriptingCore::getInstance();
	auto _global = sc->getGlobalObject();
	JS::RootedObject gobj(cx, _global);
	JSAutoCompartment ac(cx, gobj);

	JS::RootedObject ns(cx);
	JS::MutableHandleObject jsObj = &ns;
	JS::RootedValue nsval(cx);
	JS_GetProperty(cx, gobj, "Agtk", &nsval);
	JS::RootedValue rv(cx);
	bool ret = false;
	if (nsval == JSVAL_VOID) {
		return false;
	}
	jsObj.set(nsval.toObjectOrNull());
	JS::RootedValue rvariableBaseVal(cx);
	JS_GetProperty(cx, jsObj, "variableBase", &rvariableBaseVal);
	if (!rvariableBaseVal.isObject()) {
		return false;
	}

	JSObject* variableBase = rvariableBaseVal.get().toObjectOrNull();
	JS::RootedObject rvariableBase(cx, variableBase);
#endif
	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_7
	auto variable = JS_DefineObject(cx, jsthis, "variable", &variable_class, rvariableBase, JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE);
#else
	auto variable = JS_DefineObject(cx, jsthis, "variable", &variable_class, JS::NullPtr(), JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE);
#endif
	CCASSERT(variable, "variable should not be nullptr");
	JS::RootedObject rvariable(cx, variable);
// #AGTK-NX
#ifndef USE_SAR_OPTIMIZE_7
	static JSFunctionSpec variable_methods[] = {
		{ "getValue", js_variable_getValue, 0, 0, 0 },
		{ "setValue", js_variable_setValue, 0, 0, 0 },
		{ NULL },
	};
	JS_DefineFunctions(cx, rvariable, variable_methods);
#endif
#if 1
	{
		JS::RootedValue v(cx);
		JS_GetProperty(cx, jsthis, "instanceId", &v);
		if (!v.isNumber()) {
			return false;
		}
		JS_DefineProperty(cx, rvariable, "instanceId", v, JSPROP_ENUMERATE | JSPROP_PERMANENT);
		auto instanceId = v.toInt32();
		JS_GetProperty(cx, jsthis, "objectId", &v);
		if (!v.isNumber()) {
			return false;
		}
		JS_DefineProperty(cx, rvariable, "objectId", v, JSPROP_ENUMERATE | JSPROP_PERMANENT);
		auto objectId = v.toInt32();

		auto scene = GameManager::getInstance()->getCurrentScene();
		agtk::Object *objectInstance = scene->getObjectInstance(objectId, instanceId);
		if (!objectInstance) {
			return true;
		}
		auto variableData = objectInstance->getPlayObjectData()->getVariableData(variableId);
		if (!variableData) {
			return true;
		}
	}
#endif
	JS::RootedValue rval(cx);
	rval.setInt32(variableId);
	JS_SetProperty(cx, rvariable, "variableId", rval);
	args.rval().set(OBJECT_TO_JSVAL(variable));
	return true;
}

bool js_variables_getIdByName(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//arg1: <string: 変数名>
	//ret: <int : 変数ID>
	JS::CallArgs args = CallArgsFromVp(argc, vp);
    args.rval().set(JS::Int32Value(-1));
	auto len = args.length();
	if (len < 1) {
		return false;
	}
	std::string variableName;
	{
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
		auto &v2 = args[0];
#else
#endif
		if (!v2.isString()) {
			return false;
		}
		if (!jsval_to_std_string(cx, v2, &variableName)) {
			return false;
		}
	}

#if 1
	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
	JS::RootedValue v(cx);
	JS_GetProperty(cx, jsthis, "instanceId", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto instanceId = JavascriptManager::getInt32(v);
	JS_GetProperty(cx, jsthis, "objectId", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto objectId = JavascriptManager::getInt32(v);

	auto scene = GameManager::getInstance()->getCurrentScene();
	agtk::Object *objectInstance = scene->getObjectInstance(objectId, instanceId);
	if (!objectInstance) {
		return true;
	}
#endif
	auto variableData = objectInstance->getPlayObjectData()->getVariableDataByName(variableName.c_str());
	if (!variableData) {
		return true;
	}
	args.rval().set(JS::Int32Value(variableData->getId()));
	return true;
}

// variable
bool js_variable_getValue(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//ret: <double : 値>
	JS::CallArgs args = CallArgsFromVp(argc, vp);
    args.rval().set(JS::NullValue());

	JS::RootedValue v(cx);
	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
	JS_GetProperty(cx, jsthis, "variableId", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto variableId = JavascriptManager::getInt32(v);
#if 1
	JS_GetProperty(cx, jsthis, "instanceId", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto instanceId = JavascriptManager::getInt32(v);
	JS_GetProperty(cx, jsthis, "objectId", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto objectId = JavascriptManager::getInt32(v);

	auto scene = GameManager::getInstance()->getCurrentScene();
	agtk::Object *objectInstance = scene->getObjectInstance(objectId, instanceId);
	if (!objectInstance) {
		return true;
	}
#endif
	auto variableData = objectInstance->getPlayObjectData()->getVariableData(variableId);
	if (variableData) {
		auto value = variableData->getValue();
		args.rval().set(JS::DoubleValue(value));
	}
	else {
		args.rval().set(JS::NullValue());
	}

	return true;
}

bool js_variable_setValue(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//arg1: <double : 設定値>
	JS::CallArgs args = CallArgsFromVp(argc, vp);
    args.rval().set(JS::NullValue());
	auto len = args.length();
	if (len < 1) {
		return false;
	}
	double value = 0;
	{
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
		auto &v2 = args[0];
#else
#endif
		if (v2.isDouble()) {
			value = v2.toDouble();
		}
		else if (v2.isInt32()) {
			value = (double)v2.toInt32();
		}
		else {
			return false;
		}
	}

	JS::RootedValue v(cx);
	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
	JS_GetProperty(cx, jsthis, "variableId", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto variableId = JavascriptManager::getInt32(v);
#if 1
	JS_GetProperty(cx, jsthis, "instanceId", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto instanceId = JavascriptManager::getInt32(v);
	JS_GetProperty(cx, jsthis, "objectId", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto objectId = JavascriptManager::getInt32(v);

	auto scene = GameManager::getInstance()->getCurrentScene();
	agtk::Object *objectInstance = scene->getObjectInstance(objectId, instanceId);
	if (!objectInstance) {
		return true;
	}
#endif
	auto playData = objectInstance->getPlayObjectData();
	auto variableData = playData->getVariableData(variableId);

	if (variableData) {
		variableData->setValue(value);
		playData->adjustVariableData(variableData);
		args.rval().set(JS::TrueValue());
	}
	else {
		args.rval().set(JS::NullValue());
	}

	return true;
}


//===============================================================================================//
