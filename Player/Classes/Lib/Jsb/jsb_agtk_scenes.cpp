#include "Lib/Macros.h"
#include "GameManager.h"

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

static bool register_agtk_scenes(JSContext *_cx,JS::HandleObject object);

//scenes
static bool js_scenes_get(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_scenes_getIdByName(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_scenes_getIdList(JSContext *cx, unsigned argc, JS::Value *vp);
//scene
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_7
bool jsb_register_agtk_sceneBase(JSContext *cx, JSObject *agtkObj);
#endif
static bool js_scene_getLayerIdList(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_scene_getLayerIdByName(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_scene_getLayerIndexById(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_scene_getLayerById(JSContext *cx, unsigned argc, JS::Value *vp);
//layer
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_7
bool jsb_register_agtk_layerBase(JSContext *cx, JSObject *agtkObj);
#endif
static bool js_layer_getTileInfo(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_layer_getSlopeIdList(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_layer_getSlopeById(JSContext *cx, unsigned argc, JS::Value *vp);

#include "Lib/ObjectCommand.h"
#include "Data/ObjectCommandData.h"
#include "Manager/DebugManager.h"
#include "Data/PlayData.h"
#include "JavascriptManager.h"

void jsb_register_agtk_scenes(JSContext *cx, JS::HandleObject object)
{
	auto ret = register_agtk_scenes(cx, object);
	CCASSERT(ret, "Failed to register_agtk_scenes");
}

bool register_agtk_scenes(JSContext *cx, JS::HandleObject object)
{
	JS::RootedObject robj(cx, object);

	static JSClass scenes_class = {
		"scenes",
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
	auto scenes = JS_DefineObject(cx, robj, "scenes", &scenes_class, JS::NullPtr(), JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE);
	CCASSERT(scenes, "scenes should not be nullptr");
	JS::RootedObject rscenes(cx, scenes);
	static JSFunctionSpec scenes_methods[] = {
		{ "get", js_scenes_get, 0, 0, 0 },
		{ "getIdList", js_scenes_getIdList, 0, 0, 0 },
		{ "getIdByName", js_scenes_getIdByName, 0, 0, 0 },
		{ NULL },
	};
	JS_DefineFunctions(cx, rscenes, scenes_methods);

	JS::RootedValue rval(cx);
	rval.setObject(*scenes);
	JS_SetProperty(cx, robj, "scenes", rval);

// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_7
	if (!jsb_register_agtk_sceneBase(cx, object.get())) return false;
	if (!jsb_register_agtk_layerBase(cx, object.get())) return false;
#endif

	return true;
}

// scenes
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_7
bool jsb_register_agtk_sceneBase(JSContext *cx, JSObject *agtkObj)
{
	JS::RootedObject robj(cx, agtkObj);

	static JSClass scene_class = {
		"sceneBase",
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
	auto scene = JS_DefineObject(cx, robj, "sceneBase", &scene_class, JS::NullPtr(), JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE);
	CCASSERT(scene, "sceneBase should not be nullptr");
	JS::RootedObject rscene(cx, scene);
	static JSFunctionSpec scene_methods[] = {
		{ "getLayerIdList", js_scene_getLayerIdList, 0, 0, 0 },
		{ "getLayerIdByName", js_scene_getLayerIdByName, 0, 0, 0 },
		{ "getLayerIndexById", js_scene_getLayerIndexById, 0, 0, 0 },
		{ "getLayerById", js_scene_getLayerById, 0, 0, 0 },
		{ NULL },
	};
	JS_DefineFunctions(cx, rscene, scene_methods);
	return true;
}
#endif

bool js_scenes_get(JSContext *cx, unsigned argc, JS::Value *vp)
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
	auto sceneId = JavascriptManager::getInt32(v);
	auto projectData = GameManager::getInstance()->getProjectData();
	auto sceneData = projectData->getSceneData(sceneId);
	if (!sceneData) {
		return true;
	}
	static JSClass scene_class = {
		"scene",
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
	JS::RootedValue rsceneBaseVal(cx);
	JS_GetProperty(cx, jsObj, "sceneBase", &rsceneBaseVal);
	if (!rsceneBaseVal.isObject()) {
		return false;
	}

	JSObject* sceneBase = rsceneBaseVal.get().toObjectOrNull();
	JS::RootedObject rsceneBase(cx, sceneBase);
#endif
	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_7
	auto scene = JS_DefineObject(cx, jsthis, "scene", &scene_class, rsceneBase, JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE);
#else
	auto scene = JS_DefineObject(cx, jsthis, "scene", &scene_class, JS::NullPtr(), JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE);
#endif
	CCASSERT(scene, "scene should not be nullptr");
	JS::RootedObject rscene(cx, scene);
// #AGTK-NX
#ifndef USE_SAR_OPTIMIZE_7
	static JSFunctionSpec scene_methods[] = {
		{ "getLayerIdList", js_scene_getLayerIdList, 0, 0, 0 },
		{ "getLayerIdByName", js_scene_getLayerIdByName, 0, 0, 0 },
		{ "getLayerIndexById", js_scene_getLayerIndexById, 0, 0, 0 },
		{ "getLayerById", js_scene_getLayerById, 0, 0, 0 },
		{ NULL },
	};
	JS_DefineFunctions(cx, rscene, scene_methods);
#endif
	JS::RootedValue rsceneId(cx, JS::Int32Value(sceneId));
	JS_DefineProperty(cx, rscene, "id", rsceneId, JSPROP_ENUMERATE | JSPROP_PERMANENT);
	JS::RootedValue rsceneName(cx, std_string_to_jsval(cx, sceneData->getName()));
	JS_DefineProperty(cx, rscene, "name", rsceneName, JSPROP_ENUMERATE | JSPROP_PERMANENT);
	JS::RootedValue rhorzScreenCount(cx, JS::Int32Value(sceneData->getHorzScreenCount()));
	JS_DefineProperty(cx, rscene, "horzScreenCount", rhorzScreenCount, JSPROP_ENUMERATE | JSPROP_PERMANENT);
	JS::RootedValue rvertScreenCount(cx, JS::Int32Value(sceneData->getVertScreenCount()));
	JS_DefineProperty(cx, rscene, "vertScreenCount", rvertScreenCount, JSPROP_ENUMERATE | JSPROP_PERMANENT);
	JS::RootedValue rval(cx);
	rval.setInt32(sceneId);
	JS_SetProperty(cx, rscene, "sceneId", rval);
	args.rval().set(OBJECT_TO_JSVAL(scene));
	return true;
}

bool js_scenes_getIdList(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//ret: <array : シーンIDの配列>
	JS::CallArgs args = CallArgsFromVp(argc, vp);

	int value = -1;
	auto projectData = GameManager::getInstance()->getProjectData();
	std::list<int> idList;
	std::function<void(cocos2d::Dictionary *)> scanRecur = [&scanRecur, &idList, projectData](cocos2d::Dictionary *children){
		cocos2d::DictElement *el = nullptr;
		CCDICT_FOREACH(children, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto p = static_cast<agtk::data::SceneData *>(el->getObject());
#else
			auto p = dynamic_cast<agtk::data::SceneData *>(el->getObject());
#endif
			if (!p->getFolder()) {
				idList.push_back(p->getId());
			}
			if (p->getChildren()) {
				scanRecur(p->getChildren());
			}
		}
		return;
	};
	scanRecur(projectData->getSceneList());

	JS::RootedObject jsarr(cx, JS_NewArrayObject(cx, idList.size()));
	uint32_t i = 0;
	for(auto id: idList){
		JS::RootedValue element(cx, JS::Int32Value(id));
		JS_SetElement(cx, jsarr, i, element);
		i++;
	}

	args.rval().set(OBJECT_TO_JSVAL(jsarr));
	return true;
}

bool js_scenes_getIdByName(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//arg1: <string: シーン名>
	//ret: <int : シーンID>
	JS::CallArgs args = CallArgsFromVp(argc, vp);
    args.rval().set(JS::Int32Value(-1));
	auto len = args.length();
	if (len < 1) {
		return false;
	}
	std::string sceneName;
	{
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
		auto &v2 = args[0];
#else
#endif
		if (!v2.isString()) {
			return false;
		}
		if (!jsval_to_std_string(cx, v2, &sceneName)) {
			return false;
		}
	}

	int value = -1;
	auto projectData = GameManager::getInstance()->getProjectData();
	auto sceneData = projectData->getSceneDataByName(sceneName.c_str());

	if(sceneData){
		value = sceneData->getId();
	}
	args.rval().set(JS::Int32Value(value));
	return true;
}

// scene
bool js_scene_getLayerIdList(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//ret: <array : レイヤーIDの配列>
	JS::RootedValue v(cx);
	JS::CallArgs args = CallArgsFromVp(argc, vp);
	args.rval().set(JS::NullValue());

	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
	JS_GetProperty(cx, jsthis, "sceneId", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto sceneId = JavascriptManager::getInt32(v);
	int value = -1;
	auto projectData = GameManager::getInstance()->getProjectData();
	auto sceneData = projectData->getSceneData(sceneId);
	if (!sceneData) {
		return false;
	}
	auto layerList = sceneData->getLayerList();
	cocos2d::DictElement *el = nullptr;
	std::list<int> idList;
	CCDICT_FOREACH(layerList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto p = static_cast<agtk::data::LayerData *>(el->getObject());
#else
		auto p = dynamic_cast<agtk::data::LayerData *>(el->getObject());
#endif
		idList.push_back(p->getLayerId());
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

bool js_scene_getLayerIdByName(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//arg1: <string: レイヤー名>
	//ret: <int : レイヤーID>
	JS::CallArgs args = CallArgsFromVp(argc, vp);
	args.rval().set(JS::Int32Value(-1));
	auto len = args.length();
	if (len < 1) {
		return false;
	}
	std::string layerName;
	{
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
		auto &v2 = args[0];
#else
#endif
		if (!v2.isString()) {
			return false;
		}
		if (!jsval_to_std_string(cx, v2, &layerName)) {
			return false;
		}
	}
	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
	JS::RootedValue v(cx);
	JS_GetProperty(cx, jsthis, "sceneId", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto sceneId = JavascriptManager::getInt32(v);

	auto projectData = GameManager::getInstance()->getProjectData();
	auto sceneData = projectData->getSceneData(sceneId);
	if (!sceneData) {
		return true;
	}
	int layerId = -1;
	auto layerList = sceneData->getLayerList();
	cocos2d::DictElement *el = nullptr;
	CCDICT_FOREACH(layerList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto p = static_cast<agtk::data::LayerData *>(el->getObject());
#else
		auto p = dynamic_cast<agtk::data::LayerData *>(el->getObject());
#endif
		if (strcmp(p->getName(), layerName.c_str()) == 0) {
			layerId = p->getLayerId();
			break;
		}
	}
	args.rval().set(JS::Int32Value(layerId));
	return true;
}

bool js_scene_getLayerIndexById(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//arg1: <int: レイヤーID>
	//ret: <レイヤー> or null
	JS::CallArgs args = CallArgsFromVp(argc, vp);
	args.rval().set(JS::NullValue());
	auto len = args.length();
	if (len < 1) {
		return false;
	}
	int layerId = -1;
	{
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
		auto &v2 = args[0];
#else
#endif
		if (!v2.isNumber()) {
			return false;
		}
		layerId = JavascriptManager::getInt32(v2);
	}
	if (layerId < 0) {
		return true;
	}

	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
	JS::RootedValue v(cx);
	JS_GetProperty(cx, jsthis, "sceneId", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto sceneId = JavascriptManager::getInt32(v);

	auto projectData = GameManager::getInstance()->getProjectData();
	auto sceneData = projectData->getSceneData(sceneId);
	if (!sceneData) {
		return true;
	}
	int layerIndex = -1;
	auto layerDic = sceneData->getLayerList();
	cocos2d::Ref *ref = nullptr;
	auto allKeys = layerDic->allKeys();
	CCARRAY_FOREACH(allKeys, ref) {
		auto key = dynamic_cast<cocos2d::Integer *>(ref)->getValue();
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto layerData = static_cast<agtk::data::LayerData *>(layerDic->objectForKey(key));
#else
		auto layerData = dynamic_cast<agtk::data::LayerData *>(layerDic->objectForKey(key));
#endif
		if (layerData->getLayerId() == layerId) {
			layerIndex = key - 1;
			break;
		}
	}
	args.rval().set(JS::Int32Value(layerIndex));
	return true;
}

// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_7
bool jsb_register_agtk_layerBase(JSContext *cx, JSObject *agtkObj)
{
	JS::RootedObject robj(cx, agtkObj);

	static JSClass layer_class = {
		"layerBase",
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
	auto layer = JS_DefineObject(cx, robj, "layerBase", &layer_class, JS::NullPtr(), JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE);
	CCASSERT(layer, "layerBase should not be nullptr");
	JS::RootedObject rlayer(cx, layer);
	static JSFunctionSpec layer_methods[] = {
		{ "getTileInfo", js_layer_getTileInfo, 0, 0, 0 },
		{ "getSlopeIdList", js_layer_getSlopeIdList, 0, 0, 0 },
		{ "getSlopeById", js_layer_getSlopeById, 0, 0, 0 },
		{ NULL },
	};
	JS_DefineFunctions(cx, rlayer, layer_methods);

	return true;
}
#endif

bool js_scene_getLayerById(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//arg1: <int: レイヤーID>
	//ret: <レイヤー> or null
	JS::CallArgs args = CallArgsFromVp(argc, vp);
	args.rval().set(JS::NullValue());
	auto len = args.length();
	if (len < 1) {
		return false;
	}
	int layerId = -1;
	{
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
		auto &v2 = args[0];
#else
#endif
		if (!v2.isNumber()) {
			return false;
		}
		layerId = JavascriptManager::getInt32(v2);
	}
	if (layerId < 0) {
		return true;
	}

	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
	JS::RootedValue v(cx);
	JS_GetProperty(cx, jsthis, "sceneId", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto sceneId = JavascriptManager::getInt32(v);

	auto projectData = GameManager::getInstance()->getProjectData();
	auto sceneData = projectData->getSceneData(sceneId);
	if (!sceneData) {
		return true;
	}
	int layerIndex = -1;
	auto layerDic = sceneData->getLayerList();
	cocos2d::Ref *ref = nullptr;
	auto allKeys = layerDic->allKeys();
	agtk::data::LayerData *layerData = nullptr;
	CCARRAY_FOREACH(allKeys, ref) {
		auto key = dynamic_cast<cocos2d::Integer *>(ref)->getValue();
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto ld = static_cast<agtk::data::LayerData *>(layerDic->objectForKey(key));
#else
		auto ld = dynamic_cast<agtk::data::LayerData *>(layerDic->objectForKey(key));
#endif
		if (ld->getLayerId() == layerId) {
			layerData = ld;
			break;
		}
	}
	if (!layerData) {
		return true;
	}
	static JSClass layer_class = {
		"layer",
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
	JS::RootedValue rlayerBaseVal(cx);
	JS_GetProperty(cx, jsObj, "layerBase", &rlayerBaseVal);
	if (!rlayerBaseVal.isObject()) {
		return false;
	}

	JSObject* layerBase = rlayerBaseVal.get().toObjectOrNull();
	JS::RootedObject rlayerBase(cx, layerBase);
#endif
	// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_7
	auto layer = JS_DefineObject(cx, jsthis, "layer", &layer_class, rlayerBase, JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE);
#else
	auto layer = JS_DefineObject(cx, jsthis, "layer", &layer_class, JS::NullPtr(), JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE);
#endif
	CCASSERT(layer, "layer should not be nullptr");
	JS::RootedObject rlayer(cx, layer);
// #AGTK-NX
#ifndef USE_SAR_OPTIMIZE_7
	static JSFunctionSpec layer_methods[] = {
		{ "getTileInfo", js_layer_getTileInfo, 0, 0, 0 },
		{ "getSlopeIdList", js_layer_getSlopeIdList, 0, 0, 0 },
		{ "getSlopeById", js_layer_getSlopeById, 0, 0, 0 },
		{ NULL },
	};
	JS_DefineFunctions(cx, rlayer, layer_methods);
#endif
	JS::RootedValue rlayerId(cx, JS::Int32Value(layerId));
	JS_DefineProperty(cx, rlayer, "id", rlayerId, JSPROP_ENUMERATE | JSPROP_PERMANENT);
	JS::RootedValue rlayerName(cx, std_string_to_jsval(cx, layerData->getName()));
	JS_DefineProperty(cx, rlayer, "name", rlayerName, JSPROP_ENUMERATE | JSPROP_PERMANENT);
	JS::RootedValue rsceneId(cx, JS::Int32Value(sceneId));
	JS_DefineProperty(cx, rlayer, "sceneId", rsceneId, JSPROP_ENUMERATE | JSPROP_PERMANENT);
	JS::RootedValue rval(cx);
	rval.setInt32(layerId);
	JS_SetProperty(cx, rlayer, "layerId", rval);
	args.rval().set(OBJECT_TO_JSVAL(layer));
	return true;
}

//layer
bool js_layer_getTileInfo(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//arg1: <int: x>
	//arg2: <int: y>
	//ret: <タイル情報> or null
	JS::CallArgs args = CallArgsFromVp(argc, vp);
	args.rval().set(JS::NullValue());
	auto len = args.length();
	if (len < 2) {
		return false;
	}
	int x = -1;
	int y = -1;
	{
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
		auto &v2 = args[0];
#else
#endif
		if (!v2.isNumber()) {
			return false;
		}
		x = JavascriptManager::getInt32(v2);
	}
	{
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
		auto &v2 = args[1];
#else
#endif
		if (!v2.isNumber()) {
			return false;
		}
		y = JavascriptManager::getInt32(v2);
	}

	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
	JS::RootedValue v(cx);
	JS_GetProperty(cx, jsthis, "sceneId", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto sceneId = JavascriptManager::getInt32(v);
	JS_GetProperty(cx, jsthis, "layerId", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto layerId = JavascriptManager::getInt32(v);

	auto projectData = GameManager::getInstance()->getProjectData();
	auto sceneData = projectData->getSceneData(sceneId);
	if (!sceneData) {
		return true;
	}
	auto layerDic = sceneData->getLayerList();
	cocos2d::Ref *ref = nullptr;
	auto allKeys = layerDic->allKeys();
	agtk::data::LayerData *layerData = nullptr;
	CCARRAY_FOREACH(allKeys, ref) {
		auto key = dynamic_cast<cocos2d::Integer *>(ref)->getValue();
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto ld = static_cast<agtk::data::LayerData *>(layerDic->objectForKey(key));
#else
		auto ld = dynamic_cast<agtk::data::LayerData *>(layerDic->objectForKey(key));
#endif
		if (ld->getLayerId() == layerId) {
			layerData = ld;
			break;
		}
	}
	if (!layerData) {
		return true;
	}
	auto horzScreenCount = sceneData->getHorzScreenCount();
	auto vertScreenCount = sceneData->getVertScreenCount();
	auto const screen = GameManager::getInstance()->getProjectData()->getScreenSize();
	auto tileWidth = projectData->getTileWidth();
	auto tileHeight = projectData->getTileHeight();
	auto sceneHorzTileCount = (int)((screen.width * horzScreenCount + tileWidth - 1) / tileWidth);
	auto sceneVertTileCount = (int)((screen.height * vertScreenCount + tileHeight - 1) / tileHeight);

	if (x < 0 || x >= sceneHorzTileCount || y < 0 || y >= sceneVertTileCount) {
		return true;
	}
	agtk::data::Tile *tileData = nullptr;
	CCARRAY_FOREACH(layerData->getTileList(), ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto tile = static_cast<agtk::data::Tile *>(ref);
#else
		auto tile = dynamic_cast<agtk::data::Tile *>(ref);
#endif
		auto position = tile->getPosition();
		if (position.x == x && position.y == y) {
			tileData = tile;
			break;
		}
#if 0
		auto tile = dynamic_cast<agtk::data::Tile *>(ref);
		auto tileset = projectData->getTilesetData(tile->getTilesetId());
		auto tileData = tileset->getTileData(tile->getX() + tile->getY() * tileset->getHorzTileCount());
		if (tileData && tileData->getPlaySe() && tileData->getPlaySeId() > 0) {
			audioManager->preloadSe(tileData->getPlaySeId());
		}
#endif
	}
	if (!tileData) {
		return true;
	}
	static JSClass tileInfo_class = {
		"tileInfo",
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
	auto tileInfo = JS_DefineObject(cx, jsthis, "tileInfo", &tileInfo_class, JS::NullPtr(), JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE);
	CCASSERT(tileInfo, "tileInfo should not be nullptr");
	JS::RootedObject rtileInfo(cx, tileInfo);
#if 0
	static JSFunctionSpec tileInfo_methods[] = {
		{ "getTileInfo", js_tileInfo_getTileInfo, 0, 0, 0 },
		{ NULL },
	};
	JS_DefineFunctions(cx, rtileInfo, tileInfo_methods);
#endif
	//{tilesetId: <タイルセットID>, x: <X位置>, y: <Y位置>}

	JS::RootedValue rtilesetId(cx, JS::Int32Value(tileData->getTilesetId()));
	JS_DefineProperty(cx, rtileInfo, "tilesetId", rtilesetId, JSPROP_ENUMERATE | JSPROP_PERMANENT);
	JS::RootedValue rx(cx, JS::Int32Value(tileData->getX()));
	JS_DefineProperty(cx, rtileInfo, "x", rx, JSPROP_ENUMERATE | JSPROP_PERMANENT);
	JS::RootedValue ry(cx, JS::Int32Value(tileData->getY()));
	JS_DefineProperty(cx, rtileInfo, "y", ry, JSPROP_ENUMERATE | JSPROP_PERMANENT);
	args.rval().set(OBJECT_TO_JSVAL(tileInfo));
	return true;
}

bool js_layer_getSlopeIdList(JSContext *cx, unsigned argc, JS::Value *vp)
{
	JS::CallArgs args = CallArgsFromVp(argc, vp);
	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
	JS::RootedValue v(cx);
	JS_GetProperty(cx, jsthis, "sceneId", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto sceneId = JavascriptManager::getInt32(v);
	JS_GetProperty(cx, jsthis, "layerId", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto layerId = JavascriptManager::getInt32(v);

	auto projectData = GameManager::getInstance()->getProjectData();
	auto sceneData = projectData->getSceneData(sceneId);
	if (!sceneData) {
		return true;
	}
	auto othersList = sceneData->getScenePartOthersList(layerId - 1);
	cocos2d::Ref *ref = nullptr;
	std::list<int> idList;
	CCARRAY_FOREACH(othersList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto p = static_cast<agtk::data::ScenePartData *>(ref);
#else
		auto p = dynamic_cast<agtk::data::ScenePartData *>(ref);
#endif
		if (p->getPartType() != agtk::data::ScenePartData::kPartOthers) continue;
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto othersData = static_cast<agtk::data::ScenePartOthersData *>(p);
#else
		auto othersData = dynamic_cast<agtk::data::ScenePartOthersData *>(p);
#endif
		if (othersData->getOthersType() != agtk::data::ScenePartOthersData::kOthersSlope) continue;
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		// slopeData は不要
#else
		auto slopeData = dynamic_cast<agtk::data::OthersSlopeData *>(othersData->getPart());
#endif
		idList.push_back(p->getId());
	}

	JS::RootedObject jsarr(cx, JS_NewArrayObject(cx, idList.size()));
	uint32_t i = 0;
	for(auto id: idList){
		JS::RootedValue element(cx, JS::Int32Value(id));
		JS_SetElement(cx, jsarr, i, element);
		i++;
	}

	args.rval().set(OBJECT_TO_JSVAL(jsarr));
	return true;
}

bool js_layer_getSlopeById(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//arg1: <int: id>
	//ret: <タイル情報> or null
	JS::CallArgs args = CallArgsFromVp(argc, vp);
	args.rval().set(JS::NullValue());
	auto len = args.length();
	if (len < 1) {
		return false;
	}
	int slopeId = -1;
	{
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
		auto &v2 = args[0];
#else
#endif
		if (!v2.isNumber()) {
			return false;
		}
		slopeId = JavascriptManager::getInt32(v2);
	}

	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
	JS::RootedValue v(cx);
	JS_GetProperty(cx, jsthis, "sceneId", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto sceneId = JavascriptManager::getInt32(v);
	JS_GetProperty(cx, jsthis, "layerId", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto layerId = JavascriptManager::getInt32(v);

	auto projectData = GameManager::getInstance()->getProjectData();
	auto sceneData = projectData->getSceneData(sceneId);
	if (!sceneData) {
		return true;
	}
	auto othersList = sceneData->getScenePartOthersList(layerId - 1);
	cocos2d::Ref *ref = nullptr;
	agtk::data::ScenePartData *scenePartData = nullptr;
    agtk::data::OthersSlopeData *slopeData = nullptr;
	CCARRAY_FOREACH(othersList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto p = static_cast<agtk::data::ScenePartData *>(ref);
#else
		auto p = dynamic_cast<agtk::data::ScenePartData *>(ref);
#endif
		if (p->getPartType() != agtk::data::ScenePartData::kPartOthers) continue;
		auto othersData = dynamic_cast<agtk::data::ScenePartOthersData *>(p);
        if(othersData != nullptr){
			scenePartData = p;
			slopeData = dynamic_cast<agtk::data::OthersSlopeData *>(othersData->getPart());
            break;
        }
	}
    if(!slopeData){
		return false;
    }
	static JSClass slopeInfo_class = {
		"slopeInfo",
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
	auto slopeInfo = JS_DefineObject(cx, jsthis, "slopeInfo", &slopeInfo_class, JS::NullPtr(), JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE);
	CCASSERT(slopeInfo, "slopeInfo should not be nullptr");
	JS::RootedObject rslopeInfo(cx, slopeInfo);
#if 0
	static JSFunctionSpec slopeInfo_methods[] = {
		{ "getTileInfo", js_slopeInfo_getTileInfo, 0, 0, 0 },
		{ NULL },
	};
	JS_DefineFunctions(cx, rslopeInfo, slopeInfo_methods);
#endif
	JS::RootedValue rstartX(cx, JS::Int32Value(slopeData->getStartX()));
	JS_DefineProperty(cx, rslopeInfo, "startX", rstartX, JSPROP_ENUMERATE | JSPROP_PERMANENT);
	JS::RootedValue rstartY(cx, JS::Int32Value(slopeData->getStartY()));
	JS_DefineProperty(cx, rslopeInfo, "startY", rstartY, JSPROP_ENUMERATE | JSPROP_PERMANENT);
	JS::RootedValue rendX(cx, JS::Int32Value(slopeData->getEndX()));
	JS_DefineProperty(cx, rslopeInfo, "endX", rendX, JSPROP_ENUMERATE | JSPROP_PERMANENT);
	JS::RootedValue rendY(cx, JS::Int32Value(slopeData->getEndY()));
	JS_DefineProperty(cx, rslopeInfo, "endY", rendY, JSPROP_ENUMERATE | JSPROP_PERMANENT);
	JS::RootedValue rname(cx, std_string_to_jsval(cx, scenePartData->getName()));
	JS_DefineProperty(cx, rslopeInfo, "name", rname, JSPROP_ENUMERATE | JSPROP_PERMANENT);
	args.rval().set(OBJECT_TO_JSVAL(slopeInfo));
	return true;
}

//===============================================================================================//
