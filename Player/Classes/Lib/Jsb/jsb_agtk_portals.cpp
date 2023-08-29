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

static bool register_agtk_portals(JSContext *_cx,JS::HandleObject object);

//portals
static bool js_portals_get(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_portals_getIdByName(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_portals_getIdList(JSContext *cx, unsigned argc, JS::Value *vp);
//portal
static bool jsb_register_agtk_portal(JSContext *cx, JSObject *agtkObj, const char *name, int index);

#include "Lib/ObjectCommand.h"
#include "Data/ObjectCommandData.h"
#include "Manager/DebugManager.h"
#include "Data/PlayData.h"
#include "JavascriptManager.h"

void jsb_register_agtk_portals(JSContext *cx, JS::HandleObject object)
{
	auto ret = register_agtk_portals(cx, object);
	CCASSERT(ret, "Failed to register_agtk_portals");
}

bool register_agtk_portals(JSContext *cx, JS::HandleObject object)
{
	JS::RootedObject robj(cx, object);

	static JSClass portals_class = {
		"portals",
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
	auto portals = JS_DefineObject(cx, robj, "portals", &portals_class, JS::NullPtr(), JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE);
	CCASSERT(portals, "portals should not be nullptr");
	JS::RootedObject rportals(cx, portals);
	static JSFunctionSpec portals_methods[] = {
		{ "get", js_portals_get, 0, 0, 0 },
		{ "getIdList", js_portals_getIdList, 0, 0, 0 },
		{ "getIdByName", js_portals_getIdByName, 0, 0, 0 },
		{ NULL },
	};
	JS_DefineFunctions(cx, rportals, portals_methods);

	JS::RootedValue rval(cx);
	rval.setObject(*portals);
	JS_SetProperty(cx, robj, "portals", rval);
	return true;
}

// portals
bool js_portals_get(JSContext *cx, unsigned argc, JS::Value *vp)
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
	auto portalId = JavascriptManager::getInt32(v);
	auto projectData = GameManager::getInstance()->getProjectData();
	auto portalData = projectData->getTransitionPortalData(portalId);
	if (!portalData) {
		return true;
	}
	static JSClass portal_class = {
		"portal",
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
	auto portal = JS_DefineObject(cx, jsthis, "portal", &portal_class, JS::NullPtr(), JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE);
	CCASSERT(portal, "portal should not be nullptr");
	JS::RootedObject rportal(cx, portal);
#if 0
	static JSFunctionSpec portal_methods[] = {
		{ "getLayerIdList", js_portal_getLayerIdList, 0, 0, 0 },
		{ "getLayerIdByName", js_portal_getLayerIdByName, 0, 0, 0 },
		{ "getLayerIndexById", js_portal_getLayerIndexById, 0, 0, 0 },
		{ NULL },
	};
	JS_DefineFunctions(cx, rportal, portal_methods);
#endif
	JS::RootedValue rportalId(cx, JS::Int32Value(portalId));
	JS_DefineProperty(cx, rportal, "id", rportalId, JSPROP_ENUMERATE | JSPROP_PERMANENT);
	JS::RootedValue rportalName(cx, std_string_to_jsval(cx, portalData->getName()->getCString()));
	JS_DefineProperty(cx, rportal, "name", rportalName, JSPROP_ENUMERATE | JSPROP_PERMANENT);
    //todo a, bを追加する。
	if (!jsb_register_agtk_portal(cx, portal, "a", 0)) return false;
	if (!jsb_register_agtk_portal(cx, portal, "b", 1)) return false;
#if 0
	JS::RootedValue rhorzScreenCount(cx, JS::Int32Value(portalData->getHorzScreenCount()));
	JS_DefineProperty(cx, rportal, "horzScreenCount", rhorzScreenCount, JSPROP_ENUMERATE | JSPROP_PERMANENT);
	JS::RootedValue rvertScreenCount(cx, JS::Int32Value(portalData->getVertScreenCount()));
	JS_DefineProperty(cx, rportal, "vertScreenCount", rvertScreenCount, JSPROP_ENUMERATE | JSPROP_PERMANENT);
#endif
	JS::RootedValue rval(cx);
	rval.setInt32(portalId);
	JS_SetProperty(cx, rportal, "portalId", rval);
	args.rval().set(OBJECT_TO_JSVAL(portal));
	return true;
}

bool js_portals_getIdList(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//ret: <array : シーンIDの配列>
	JS::CallArgs args = CallArgsFromVp(argc, vp);

	int value = -1;
	auto projectData = GameManager::getInstance()->getProjectData();
	std::list<int> idList;
	auto portalList = projectData->getTransitionPortalList();
	cocos2d::Ref *ref = nullptr;
	CCARRAY_FOREACH(portalList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto p = static_cast<agtk::data::TransitionPortalData *>(ref);
#else
		auto p = dynamic_cast<agtk::data::TransitionPortalData *>(ref);
#endif
		if (p->getFolder()) {
			continue;
		}
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

bool js_portals_getIdByName(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//arg1: <string: シーン名>
	//ret: <int : シーンID>
	JS::CallArgs args = CallArgsFromVp(argc, vp);
    args.rval().set(JS::Int32Value(-1));
	auto len = args.length();
	if (len < 1) {
		return false;
	}
	std::string portalName;
	{
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
		auto &v2 = args[0];
#else
#endif
		if (!v2.isString()) {
			return false;
		}
		if (!jsval_to_std_string(cx, v2, &portalName)) {
			return false;
		}
	}

	int value = -1;
	auto projectData = GameManager::getInstance()->getProjectData();
	auto portalData = projectData->getTransitionPortalDataByName(portalName.c_str());

	if(portalData){
		value = portalData->getId();
	}
	args.rval().set(JS::Int32Value(value));
	return true;
}

// portal a/b
bool jsb_register_agtk_portal(JSContext *cx, JSObject *agtkObj, const char *name, int index)
{
	JS::RootedObject robj(cx, agtkObj);

	static JSClass portal_class = {
		"portal",
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
	auto portal = JS_DefineObject(cx, robj, name, &portal_class, JS::NullPtr(), JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE);
	CCASSERT(portal, "portal should not be nullptr");
	JS::RootedObject rportal(cx, portal);
#if 0
	static JSFunctionSpec portal_methods[] = {
		{ "get", js_portal_get, 0, 0, 0 },
		{ "getIdByName", js_portal_getIdByName, 0, 0, 0 },
		{ "getIdList", js_portal_getIdList, 0, 0, 0 },
		{ NULL },
	};
	JS_DefineFunctions(cx, rportal, portal_methods);
#endif
	int portalId = -1;
	{
		JS::RootedValue v(cx);
		JS_GetProperty(cx, robj, "id", &v);
		if (!v.isNumber()) {
			return false;
		}
		portalId = JavascriptManager::getInt32(v);
	}
	auto projectData = GameManager::getInstance()->getProjectData();
	auto portalData = projectData->getTransitionPortalData(portalId);
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	auto areaSettingData = static_cast<agtk::data::AreaSettingData *>(portalData->getAreaSettingList()->getObjectAtIndex(index));
	auto movable = static_cast<cocos2d::Bool *>(portalData->getMovableList()->getObjectAtIndex(index))->getValue();
#else
	auto areaSettingData = dynamic_cast<agtk::data::AreaSettingData *>(portalData->getAreaSettingList()->getObjectAtIndex(index));
	auto movable = dynamic_cast<cocos2d::Bool *>(portalData->getMovableList()->getObjectAtIndex(index))->getValue();
#endif
#if 0
	auto areaSettingDataList = data->getAreaSettingList();

	// ポータルAとポータルBのデータチェック
	for (int i = 0; i < agtk::data::TransitionPortalData::EnumPortalType::MAX; i++) {
		auto areaSettingData = dynamic_cast<agtk::data::AreaSettingData *>(areaSettingDataList->getObjectAtIndex(i));

		// エリア設定データでシーンIDとシーンレイヤーIDが一致する場合
		if (areaSettingData->getSceneId() == sceneId && areaSettingData->getLayerIndex() == layerId - 1) {
#endif
	JS::RootedValue rsceneId(cx, JS::Int32Value(areaSettingData->getSceneId()));
	JS_DefineProperty(cx, rportal, "sceneId", rsceneId, JSPROP_ENUMERATE | JSPROP_PERMANENT);
	JS::RootedValue rx(cx, JS::Int32Value(areaSettingData->getX()));
	JS_DefineProperty(cx, rportal, "x", rx, JSPROP_ENUMERATE | JSPROP_PERMANENT);
	JS::RootedValue ry(cx, JS::Int32Value(areaSettingData->getY()));
	JS_DefineProperty(cx, rportal, "y", ry, JSPROP_ENUMERATE | JSPROP_PERMANENT);
	JS::RootedValue rwidth(cx, JS::Int32Value(areaSettingData->getWidth()));
	JS_DefineProperty(cx, rportal, "width", rwidth, JSPROP_ENUMERATE | JSPROP_PERMANENT);
	JS::RootedValue rheight(cx, JS::Int32Value(areaSettingData->getHeight()));
	JS_DefineProperty(cx, rportal, "height", rheight, JSPROP_ENUMERATE | JSPROP_PERMANENT);
	JS::RootedValue rmovable(cx, JS::Int32Value(movable));
	JS_DefineProperty(cx, rportal, "movable", rmovable, JSPROP_ENUMERATE | JSPROP_PERMANENT);

	//JS::RootedValue rval(cx);
	//rval.setObject(*portal);
	//JS_SetProperty(cx, robj, name, rval);
	{
		JS::RootedValue v(cx);
		JS_GetProperty(cx, robj, "id", &v);
		if (!v.isNumber()) {
			return false;
		}
		JS_DefineProperty(cx, rportal, "portalId", v, JSPROP_ENUMERATE | JSPROP_PERMANENT);
	}
	return true;
}

//===============================================================================================//
