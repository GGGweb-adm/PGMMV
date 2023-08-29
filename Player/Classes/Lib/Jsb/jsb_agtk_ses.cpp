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

static bool register_agtk_ses(JSContext *_cx,JS::HandleObject object);

//ses
static bool js_ses_get(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_ses_getIdByName(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_ses_getIdList(JSContext *cx, unsigned argc, JS::Value *vp);
//se
static bool js_se_getProperty(JSContext *cx, JS::HandleObject obj, JS::HandleId id, JS::MutableHandleValue vp);
static bool js_se_setProperty(JSContext *cx, JS::HandleObject obj, JS::HandleId id, bool strict, JS::MutableHandleValue vp);

#include "Lib/ObjectCommand.h"
#include "Data/ObjectCommandData.h"
#include "Manager/DebugManager.h"
#include "Data/PlayData.h"
#include "JavascriptManager.h"

void jsb_register_agtk_ses(JSContext *cx, JS::HandleObject object)
{
	auto ret = register_agtk_ses(cx, object);
	CCASSERT(ret, "Failed to register_agtk_ses");
}

bool register_agtk_ses(JSContext *cx, JS::HandleObject object)
{
	JS::RootedObject robj(cx, object);

	static JSClass ses_class = {
		"ses",
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
	auto ses = JS_DefineObject(cx, robj, "ses", &ses_class, JS::NullPtr(), JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE);
	CCASSERT(ses, "ses should not be nullptr");
	JS::RootedObject rses(cx, ses);
	static JSFunctionSpec ses_methods[] = {
		{ "get", js_ses_get, 0, 0, 0 },
		{ "getIdList", js_ses_getIdList, 0, 0, 0 },
		{ "getIdByName", js_ses_getIdByName, 0, 0, 0 },
		{ NULL },
	};
	JS_DefineFunctions(cx, rses, ses_methods);

	JS::RootedValue rval(cx);
	rval.setObject(*ses);
	JS_SetProperty(cx, robj, "ses", rval);
	return true;
}

// ses
enum {
	ObjectOperatable,
};
bool js_ses_get(JSContext *cx, unsigned argc, JS::Value *vp)
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
	auto seId = JavascriptManager::getInt32(v);
	auto projectData = GameManager::getInstance()->getProjectData();
	auto seData = projectData->getSeData(seId);
	if (!seData) {
		return true;
	}
	static JSClass se_class = {
		"se",
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
	auto se = JS_DefineObject(cx, jsthis, "se", &se_class, JS::NullPtr(), JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE);
	CCASSERT(se, "se should not be nullptr");
	JS::RootedObject rse(cx, se);
#if 0
	static JSFunctionSpec se_methods[] = {
		//{ "getValue", js_switch_getValue, 0, 0, 0 },
		{ NULL },
	};
	JS_DefineFunctions(cx, rse, se_methods);
#endif
	JS::RootedValue rseId(cx, JS::Int32Value(seId));
	JS_DefineProperty(cx, rse, "id", rseId, JSPROP_ENUMERATE | JSPROP_PERMANENT);
	JS::RootedValue rseName(cx, std_string_to_jsval(cx, seData->getName()));
	JS_DefineProperty(cx, rse, "name", rseName, JSPROP_ENUMERATE | JSPROP_PERMANENT);
	JS::RootedValue rseFilename(cx, std_string_to_jsval(cx, seData->getFilename()));
	JS_DefineProperty(cx, rse, "filename", rseFilename, JSPROP_ENUMERATE | JSPROP_PERMANENT);
	JS::RootedValue rval(cx);
	rval.setInt32(seId);
	JS_SetProperty(cx, rse, "seId", rval);
	args.rval().set(OBJECT_TO_JSVAL(se));
	return true;
}

bool js_ses_getIdList(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//ret: <array : 画像IDの配列>
	JS::CallArgs args = CallArgsFromVp(argc, vp);

	int value = -1;
	auto projectData = GameManager::getInstance()->getProjectData();
	std::list<int> idList;
	std::function<void(cocos2d::Dictionary *)> scanRecur = [&scanRecur, &idList, projectData](cocos2d::Dictionary *children){
		cocos2d::DictElement *el = nullptr;
		CCDICT_FOREACH(children, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto p = static_cast<agtk::data::SeData *>(el->getObject());
#else
			auto p = dynamic_cast<agtk::data::SeData *>(el->getObject());
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
	scanRecur(projectData->getSeList());

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

bool js_ses_getIdByName(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//arg1: <string: 画像名>
	//ret: <int : 画像ID>
	JS::CallArgs args = CallArgsFromVp(argc, vp);
    args.rval().set(JS::Int32Value(-1));
	auto len = args.length();
	if (len < 1) {
		return false;
	}
	std::string seName;
	{
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
		auto &v2 = args[0];
#else
#endif
		if (!v2.isString()) {
			return false;
		}
		if (!jsval_to_std_string(cx, v2, &seName)) {
			return false;
		}
	}

	int value = -1;
	auto projectData = GameManager::getInstance()->getProjectData();
	auto seData = projectData->getSeDataByName(seName.c_str());

	if(seData){
		value = seData->getId();
	}
	args.rval().set(JS::Int32Value(value));
	return true;
}

// se
bool js_se_getProperty(JSContext *cx, JS::HandleObject obj, JS::HandleId id, JS::MutableHandleValue vp)
{
	JS::RootedValue v(cx);

	JS::RootedObject robj(cx, obj);
	JS_GetProperty(cx, robj, "seId", &v);
    if (!v.isNumber()){
        vp.set(JS::NullValue());
        return false;
    }
	auto seId = JavascriptManager::getInt32(v);
	auto projectData = GameManager::getInstance()->getProjectData();
	auto seData = projectData->getSeData(seId);
	if (!seData){
        vp.set(JS::NullValue());
        return false;
    }

#if 0
	JSString* jss = JSID_TO_STRING(id);
	char *p = JS_EncodeString(cx, jss);
	if (strcmp(p, "operatable") == 0) {
		vp.setBoolean(seData->getOperatable());
	} else {
		CCLOG("Unimplemented property getter: %s", p);
		return false;
	}
#endif
	return true;
}

bool js_se_setProperty(JSContext *cx, JS::HandleObject obj, JS::HandleId id, bool strict, JS::MutableHandleValue vp)
{
	JS::RootedValue v(cx);

	JS::RootedObject robj(cx, obj);
	JS_GetProperty(cx, robj, "seId", &v);
    if (!v.isNumber()){
        vp.set(JS::NullValue());
        return false;
    }
	auto seId = JavascriptManager::getInt32(v);
	auto projectData = GameManager::getInstance()->getProjectData();
	auto seData = projectData->getSeData(seId);
	if (!seData){
        vp.set(JS::NullValue());
        return false;
    }

#if 0
	JSString* jss = JSID_TO_STRING(id);
	char *p = JS_EncodeString(cx, jss);
	if (strcmp(p, "operatable") == 0){
		bool operatable = false;
		if(vp.isBoolean()){
			operatable = vp.toBoolean();
		} else if(vp.isInt32()){
			operatable = vp.toInt32() != 0;
		} else if(vp.isDouble()){
			operatable = vp.toDouble() != 0;
		} else {
			vp.set(JS::NullValue());
			return false;
		}
		seData->setOperatable(operatable);
	} else {
		CCLOG("Unimplemented property setter: %s", p);
		return false;
	}
#endif
	return true;
}

//===============================================================================================//
