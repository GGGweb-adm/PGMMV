#include "Lib/Macros.h"
#include "GameManager.h"

#include "jsapi.h"
#include "jsfriendapi.h"
#include "scripting/js-bindings/manual/js_bindings_config.h"
#include "scripting/js-bindings/manual/js_bindings_core.h"
#include "scripting/js-bindings/manual/spidermonkey_specifics.h"
#include "scripting/js-bindings/manual/js_manual_conversions.h"
#include "scripting/js-bindings/manual/cocos2d_specifics.hpp"
#include "mozilla/Maybe.h"
#include "scripting/js-bindings/manual/js-BindingsExport.h"

static bool register_agtk_sceneInstances(JSContext *_cx,JS::HandleObject object);

//sceneInstances
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_7
static bool register_agtk_sceneInstanceBase(JSContext *cx, JS::HandleObject object);
#endif
//static bool js_sceneInstances_get(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_sceneInstances_getCurrent(JSContext *cx, unsigned argc, JS::Value *vp);
//static bool js_sceneInstances_getIdByName(JSContext *cx, unsigned argc, JS::Value *vp);
//sceneInstance
//static bool js_sceneInstance_getValue(JSContext *cx, unsigned argc, JS::Value *vp);
//static bool js_sceneInstance_setValue(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_sceneInstance_getLayerByIndex(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_sceneInstance_getMenuLayerById(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_sceneInstance_getCurrentCameraTargetPos(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_sceneInstance_getCurrentCameraDisplayScale(JSContext *cx, unsigned argc, JS::Value *vp);

#include "Lib/ObjectCommand.h"
#include "Data/ObjectCommandData.h"
#include "Manager/DebugManager.h"
#include "Data/PlayData.h"
#include "JavascriptManager.h"

void jsb_register_agtk_sceneInstances(JSContext *cx, JS::HandleObject object)
{
	auto ret = register_agtk_sceneInstances(cx, object);
	CCASSERT(ret, "Failed to register_agtk_sceneInstances");
}

bool register_agtk_sceneInstances(JSContext *cx, JS::HandleObject object)
{
	JS::RootedObject robj(cx, object);

	static JSClass sceneInstances_class = {
		"sceneInstances",
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
	auto sceneInstances = JS_DefineObject(cx, robj, "sceneInstances", &sceneInstances_class, JS::NullPtr(), JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE);
	CCASSERT(sceneInstances, "sceneInstances should not be nullptr");
	JS::RootedObject rsceneInstances(cx, sceneInstances);
	static JSFunctionSpec sceneInstances_methods[] = {
		//{ "get", js_sceneInstances_get, 0, 0, 0 },
		{ "getCurrent", js_sceneInstances_getCurrent, 0, 0, 0 },
		//{ "getIdByName", js_sceneInstances_getIdByName, 0, 0, 0 },
		{ NULL },
	};
	JS_DefineFunctions(cx, rsceneInstances, sceneInstances_methods);

	JS::RootedValue rval(cx);
	rval.setObject(*sceneInstances);
	JS_SetProperty(cx, robj, "sceneInstances", rval);

// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_7
	if (!register_agtk_sceneInstanceBase(cx, object)) return false;
#endif

	return true;
}

// sceneInstances
enum {
	SceneInstanceId,
	SceneInstanceName,
};
#if 0
bool js_sceneInstances_get(JSContext *cx, unsigned argc, JS::Value *vp)
{
	JS::CallArgs args = CallArgsFromVp(argc, vp);
    args.rval().set(JS::NullValue());
	auto len = args.length();
	if (len < 1) {
		return false;
	}
	auto &v = args[0];
	if (!v.isNumber()) {
		return false;
	}
	auto sceneInstanceId = JavascriptManager::getInt32(v);
	auto scene = GameManager::getInstance()->getCurrentScene();
	agtk::Object *sceneInstance = scene->getSceneInstance(-1, sceneInstanceId);
	if (!sceneInstance) {
		return true;
	}
	static JSClass sceneInstance_class = {
		"sceneInstance",
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
	auto sceneInstanceObj = JS_DefineObject(cx, jsthis, "sceneInstance", &sceneInstance_class, JS::NullPtr(), JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE);
	CCASSERT(sceneInstanceObj, "sceneInstanceObj should not be nullptr");
	JS::RootedObject rsceneInstance(cx, sceneInstanceObj);
#if 0
	static JSFunctionSpec sceneInstance_methods[] = {
		//{ "getValue", js_sceneInstance_getValue, 0, 0, 0 },
		{ NULL },
	};
	JS_DefineFunctions(cx, rsceneInstance, sceneInstance_methods);
#endif
	JS::RootedValue rsceneInstanceId(cx, JS::Int32Value(sceneInstanceId));
	JS_DefineProperty(cx, rsceneInstance, "id", rsceneInstanceId, JSPROP_ENUMERATE | JSPROP_PERMANENT);
	JS::RootedValue rval(cx);
	rval.setInt32(sceneInstance->getObjectData()->getId());
	JS_DefineProperty(cx, rsceneInstance, "sceneId", rval, JSPROP_ENUMERATE | JSPROP_PERMANENT);
	//rval.setInt32(sceneInstanceId);
	//JS_SetProperty(cx, rsceneInstance, "sceneInstanceId", rval);
	//JS_SetProperty(cx, rsceneInstance, "sceneId", rval);
	args.rval().set(OBJECT_TO_JSVAL(sceneInstanceObj));
	return true;
}
#endif

// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_7
bool register_agtk_sceneInstanceBase(JSContext *cx, JS::HandleObject object)
{
	JS::RootedObject robj(cx, object);

	static JSClass sceneInstance_class = {
		"sceneInstanceBase",
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
	auto sceneInstanceObj = JS_DefineObject(cx, robj, "sceneInstanceBase", &sceneInstance_class, JS::NullPtr(), JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE);
	CCASSERT(sceneInstanceObj, "sceneInstanceBase should not be nullptr");
	JS::RootedObject rsceneInstance(cx, sceneInstanceObj);
	static JSFunctionSpec sceneInstance_methods[] = {
		{ "getLayerByIndex", js_sceneInstance_getLayerByIndex, 0, 0, 0 },
		{ "getMenuLayerById", js_sceneInstance_getMenuLayerById, 0, 0, 0 },
		{ "getCurrentCameraTargetPos", js_sceneInstance_getCurrentCameraTargetPos, 0, 0, 0 },
		{ "getCurrentCameraDisplayScale", js_sceneInstance_getCurrentCameraDisplayScale, 0, 0, 0 },
		{ NULL },
	};
	JS_DefineFunctions(cx, rsceneInstance, sceneInstance_methods);

	return true;
}
#endif

bool js_sceneInstances_getCurrent(JSContext *cx, unsigned argc, JS::Value *vp)
{
	JS::CallArgs args = CallArgsFromVp(argc, vp);
    args.rval().set(JS::NullValue());
#if 0
	auto len = args.length();
	if (len < 1) {
		return false;
	}
	auto &v = args[0];
	if (!v.isNumber()) {
		return false;
	}
#endif
	auto sceneInstance = GameManager::getInstance()->getCurrentScene();
	if (!sceneInstance) {
		return true;
	}
	static JSClass sceneInstance_class = {
		"sceneInstance",
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
	JS::RootedValue rsceneInstanceBaseVal(cx);
	JS_GetProperty(cx, jsObj, "sceneInstanceBase", &rsceneInstanceBaseVal);
	if (!rsceneInstanceBaseVal.isObject()) {
		return false;
	}

	JSObject* sceneInstanceBase = rsceneInstanceBaseVal.get().toObjectOrNull();
	JS::RootedObject rsceneInstanceBase(cx, sceneInstanceBase);
#endif
	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_7
	auto sceneInstanceObj = JS_DefineObject(cx, jsthis, "sceneInstance", &sceneInstance_class, rsceneInstanceBase, JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE);
#else
	auto sceneInstanceObj = JS_DefineObject(cx, jsthis, "sceneInstance", &sceneInstance_class, JS::NullPtr(), JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE);
#endif
	CCASSERT(sceneInstanceObj, "sceneInstanceObj should not be nullptr");
	JS::RootedObject rsceneInstance(cx, sceneInstanceObj);
// #AGTK-NX
#ifndef USE_SAR_OPTIMIZE_7
	static JSFunctionSpec sceneInstance_methods[] = {
		{ "getLayerByIndex", js_sceneInstance_getLayerByIndex, 0, 0, 0 },
		{ "getMenuLayerById", js_sceneInstance_getMenuLayerById, 0, 0, 0 },
		{ "getCurrentCameraTargetPos", js_sceneInstance_getCurrentCameraTargetPos, 0, 0, 0 },
		{ "getCurrentCameraDisplayScale", js_sceneInstance_getCurrentCameraDisplayScale, 0, 0, 0 },
		{ NULL },
	};
	JS_DefineFunctions(cx, rsceneInstance, sceneInstance_methods);
#endif
	//JS::RootedValue rsceneInstanceId(cx, JS::Int32Value(sceneInstance->id));
	//JS_DefineProperty(cx, rsceneInstance, "id", rsceneInstanceId, JSPROP_ENUMERATE | JSPROP_PERMANENT);
	JS::RootedValue rval(cx);
	rval.setInt32(sceneInstance->getSceneData()->getId());
	JS_DefineProperty(cx, rsceneInstance, "sceneId", rval, JSPROP_ENUMERATE | JSPROP_PERMANENT);
	//rval.setInt32(sceneInstanceId);
	//JS_SetProperty(cx, rsceneInstance, "sceneInstanceId", rval);
	//JS_SetProperty(cx, rsceneInstance, "sceneId", rval);
	args.rval().set(OBJECT_TO_JSVAL(sceneInstanceObj));
	return true;
}

// sceneInstance
bool js_sceneInstance_getLayerByIndex(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//arg1: <int: レイヤーインデックス>
	//ret: <シーンレイヤー> or null
	JS::CallArgs args = CallArgsFromVp(argc, vp);
    args.rval().set(JS::NullValue());
	auto len = args.length();
	if (len < 1) {
		return false;
	}
	int layerIndex = -1;
	{
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
		auto &v2 = args[0];
#else
#endif
		if (!v2.isInt32()) {
			return false;
		}
		layerIndex = v2.toInt32();
	}

	auto sceneInstance = GameManager::getInstance()->getCurrentScene();
	if (!sceneInstance) {
		return true;
	}
	auto sceneLayer = sceneInstance->getSceneLayer(layerIndex + 1);
	if (!sceneLayer) {
		return true;
	}
	//agtk::SceneLayer *Scene::getSceneLayer(int layerId)
#if 1
	JS::RootedValue jsret(cx);
	if (sceneLayer) {
		jsret = OBJECT_TO_JSVAL(js_get_or_create_jsobject<cocos2d::Layer>(cx, (cocos2d::Layer*)sceneLayer));
	}
	else {
		jsret = JSVAL_NULL;
	};
	args.rval().set(jsret);
	return true;
#endif
}

bool js_sceneInstance_getMenuLayerById(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//arg1: <int: メニューレイヤーID>
	//ret: <メニューレイヤー> or null
	JS::CallArgs args = CallArgsFromVp(argc, vp);
	args.rval().set(JS::NullValue());
	auto len = args.length();
	if (len < 1) {
		return false;
	}
	int menuId = -1;
	{
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
		auto &v2 = args[0];
#else
#endif
		if (!v2.isInt32()) {
			return false;
		}
		menuId = v2.toInt32();
	}

	auto sceneInstance = GameManager::getInstance()->getCurrentScene();
	if (!sceneInstance) {
		return true;
	}

	auto layerList = sceneInstance->getMenuLayerList();
	agtk::SceneLayer *menuLayer = nullptr;
	cocos2d::DictElement *el = nullptr;
	CCDICT_FOREACH(layerList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto p = static_cast<agtk::SceneLayer *>(el->getObject());
#else
		auto p = dynamic_cast<agtk::SceneLayer *>(el->getObject());
#endif
		if (p->getLayerId() == menuId){
			menuLayer = p;
			break;
		}
	}
	if (!menuLayer) {
		return true;
	}
#if 1
	JS::RootedValue jsret(cx);
	if (menuLayer) {
		jsret = OBJECT_TO_JSVAL(js_get_or_create_jsobject<cocos2d::Layer>(cx, (cocos2d::Layer*)menuLayer));
	}
	else {
		jsret = JSVAL_NULL;
	};
	args.rval().set(jsret);
	return true;
#endif
}

bool js_sceneInstance_getCurrentCameraTargetPos(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//ret: <カメラ位置> or null
	JS::CallArgs args = CallArgsFromVp(argc, vp);
	args.rval().set(JS::NullValue());

	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
	JS::RootedValue v(cx);

	static JSClass posInfo_class = {
		"posInfo",
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
	auto posInfo = JS_DefineObject(cx, jsthis, "posInfo", &posInfo_class, JS::NullPtr(), JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE);
	CCASSERT(posInfo, "posInfo should not be nullptr");
	JS::RootedObject rposInfo(cx, posInfo);
	//{x: <X位置>, y: <Y位置>}

	auto scene = GameManager::getInstance()->getCurrentScene();
	auto pos = agtk::Scene::getPositionSceneFromCocos2d(scene->getCamera()->getPosition());
	JS::RootedValue rx(cx, JS::DoubleValue(pos.x));
	JS_DefineProperty(cx, rposInfo, "x", rx, JSPROP_ENUMERATE | JSPROP_PERMANENT);
	JS::RootedValue ry(cx, JS::DoubleValue(pos.y));
	JS_DefineProperty(cx, rposInfo, "y", ry, JSPROP_ENUMERATE | JSPROP_PERMANENT);
	args.rval().set(OBJECT_TO_JSVAL(posInfo));
	return true;
}

bool js_sceneInstance_getCurrentCameraDisplayScale(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//ret: <カメラ画面スケール> or null
	JS::CallArgs args = CallArgsFromVp(argc, vp);
	args.rval().set(JS::NullValue());

	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
	JS::RootedValue v(cx);

	static JSClass scaleInfo_class = {
		"scaleInfo",
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
	auto scaleInfo = JS_DefineObject(cx, jsthis, "scaleInfo", &scaleInfo_class, JS::NullPtr(), JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE);
	CCASSERT(scaleInfo, "scaleInfo should not be nullptr");
	JS::RootedObject rscaleInfo(cx, scaleInfo);
	//{x: <Xスケール>, y: <Yスケール>}

	auto scene = GameManager::getInstance()->getCurrentScene();
	auto camera = scene->getCamera();
	auto displaySize = camera->getDisplaySize();
	auto cameraScale = camera->getZoom();
	auto projectData = GameManager::getInstance()->getProjectData();
	auto scale = cocos2d::Vec2(displaySize.width * cameraScale.x / projectData->getScreenSize().width, displaySize.height * cameraScale.y / projectData->getScreenSize().height);
	JS::RootedValue rx(cx, JS::DoubleValue(scale.x));
	JS_DefineProperty(cx, rscaleInfo, "x", rx, JSPROP_ENUMERATE | JSPROP_PERMANENT);
	JS::RootedValue ry(cx, JS::DoubleValue(scale.y));
	JS_DefineProperty(cx, rscaleInfo, "y", ry, JSPROP_ENUMERATE | JSPROP_PERMANENT);
	args.rval().set(OBJECT_TO_JSVAL(scaleInfo));
	return true;
}

#if 0
bool js_sceneInstances_getIdByName(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//arg1: <string: オブジェクトインスタンス名>
	//ret: <int : オブジェクトインスタンスID> or null
	JS::CallArgs args = CallArgsFromVp(argc, vp);
    args.rval().set(JS::Int32Value(-1));
	auto len = args.length();
	if (len < 1) {
		args.rval().set(JS::NullValue());
		return false;
	}
	std::string sceneInstanceName;
	{
		auto &v2 = args[0];
		if (!v2.isString()) {
			return false;
		}
		if (!jsval_to_std_string(cx, v2, &sceneInstanceName)) {
			return false;
		}
	}

	int value = -1;
	auto scene = GameManager::getInstance()->getCurrentScene();
	agtk::Object *sceneInstance = scene->getSceneInstanceByName(-1, sceneInstanceName.c_str());

	if(sceneInstance){
		value = sceneInstance->getId();
	}
	args.rval().set(JS::Int32Value(value));
	return true;
}
#endif

#if 0
// sceneInstance
bool js_sceneInstance_getValue(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//ret: <bool : 値>
	JS::CallArgs args = CallArgsFromVp(argc, vp);
    args.rval().set(JS::NullValue());

	JS::RootedValue v(cx);
	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
	JS_GetProperty(cx, jsthis, "sceneInstanceId", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto sceneInstanceId = JavascriptManager::getInt32(v);

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

bool js_sceneInstance_setValue(JSContext *cx, unsigned argc, JS::Value *vp)
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
	JS_GetProperty(cx, jsthis, "sceneInstanceId", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto sceneInstanceId = JavascriptManager::getInt32(v);

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

//===============================================================================================//
