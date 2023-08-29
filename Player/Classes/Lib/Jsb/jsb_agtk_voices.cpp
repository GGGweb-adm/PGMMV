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

static bool register_agtk_voices(JSContext *_cx,JS::HandleObject object);

//voices
static bool js_voices_get(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_voices_getIdByName(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_voices_getIdList(JSContext *cx, unsigned argc, JS::Value *vp);
//voice
static bool js_voice_getProperty(JSContext *cx, JS::HandleObject obj, JS::HandleId id, JS::MutableHandleValue vp);
static bool js_voice_setProperty(JSContext *cx, JS::HandleObject obj, JS::HandleId id, bool strict, JS::MutableHandleValue vp);

#include "Lib/ObjectCommand.h"
#include "Data/ObjectCommandData.h"
#include "Manager/DebugManager.h"
#include "Data/PlayData.h"
#include "JavascriptManager.h"

void jsb_register_agtk_voices(JSContext *cx, JS::HandleObject object)
{
	auto ret = register_agtk_voices(cx, object);
	CCASSERT(ret, "Failed to register_agtk_voices");
}

bool register_agtk_voices(JSContext *cx, JS::HandleObject object)
{
	JS::RootedObject robj(cx, object);

	static JSClass voices_class = {
		"voices",
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
	auto voices = JS_DefineObject(cx, robj, "voices", &voices_class, JS::NullPtr(), JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE);
	CCASSERT(voices, "voices should not be nullptr");
	JS::RootedObject rvoices(cx, voices);
	static JSFunctionSpec voices_methods[] = {
		{ "get", js_voices_get, 0, 0, 0 },
		{ "getIdList", js_voices_getIdList, 0, 0, 0 },
		{ "getIdByName", js_voices_getIdByName, 0, 0, 0 },
		{ NULL },
	};
	JS_DefineFunctions(cx, rvoices, voices_methods);

	JS::RootedValue rval(cx);
	rval.setObject(*voices);
	JS_SetProperty(cx, robj, "voices", rval);
	return true;
}

// voices
enum {
	ObjectOperatable,
};
bool js_voices_get(JSContext *cx, unsigned argc, JS::Value *vp)
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
	auto voiceId = JavascriptManager::getInt32(v);
	auto projectData = GameManager::getInstance()->getProjectData();
	auto voiceData = projectData->getVoiceData(voiceId);
	if (!voiceData) {
		return true;
	}
	static JSClass voice_class = {
		"voice",
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
	auto voice = JS_DefineObject(cx, jsthis, "voice", &voice_class, JS::NullPtr(), JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE);
	CCASSERT(voice, "voice should not be nullptr");
	JS::RootedObject rvoice(cx, voice);
#if 0
	static JSFunctionSpec voice_methods[] = {
		//{ "getValue", js_switch_getValue, 0, 0, 0 },
		{ NULL },
	};
	JS_DefineFunctions(cx, rvoice, voice_methods);
#endif
	JS::RootedValue rvoiceId(cx, JS::Int32Value(voiceId));
	JS_DefineProperty(cx, rvoice, "id", rvoiceId, JSPROP_ENUMERATE | JSPROP_PERMANENT);
	JS::RootedValue rvoiceName(cx, std_string_to_jsval(cx, voiceData->getName()));
	JS_DefineProperty(cx, rvoice, "name", rvoiceName, JSPROP_ENUMERATE | JSPROP_PERMANENT);
	JS::RootedValue rvoiceFilename(cx, std_string_to_jsval(cx, voiceData->getFilename()));
	JS_DefineProperty(cx, rvoice, "filename", rvoiceFilename, JSPROP_ENUMERATE | JSPROP_PERMANENT);
	JS::RootedValue rval(cx);
	rval.setInt32(voiceId);
	JS_SetProperty(cx, rvoice, "voiceId", rval);
	args.rval().set(OBJECT_TO_JSVAL(voice));
	return true;
}

bool js_voices_getIdList(JSContext *cx, unsigned argc, JS::Value *vp)
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
			auto p = static_cast<agtk::data::VoiceData *>(el->getObject());
#else
			auto p = dynamic_cast<agtk::data::VoiceData *>(el->getObject());
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
	scanRecur(projectData->getVoiceList());

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

bool js_voices_getIdByName(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//arg1: <string: 画像名>
	//ret: <int : 画像ID>
	JS::CallArgs args = CallArgsFromVp(argc, vp);
    args.rval().set(JS::Int32Value(-1));
	auto len = args.length();
	if (len < 1) {
		return false;
	}
	std::string voiceName;
	{
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
		auto &v2 = args[0];
#else
#endif
		if (!v2.isString()) {
			return false;
		}
		if (!jsval_to_std_string(cx, v2, &voiceName)) {
			return false;
		}
	}

	int value = -1;
	auto projectData = GameManager::getInstance()->getProjectData();
	auto voiceData = projectData->getVoiceDataByName(voiceName.c_str());

	if(voiceData){
		value = voiceData->getId();
	}
	args.rval().set(JS::Int32Value(value));
	return true;
}

// voice
bool js_voice_getProperty(JSContext *cx, JS::HandleObject obj, JS::HandleId id, JS::MutableHandleValue vp)
{
	JS::RootedValue v(cx);

	JS::RootedObject robj(cx, obj);
	JS_GetProperty(cx, robj, "voiceId", &v);
    if (!v.isNumber()){
        vp.set(JS::NullValue());
        return false;
    }
	auto voiceId = JavascriptManager::getInt32(v);
	auto projectData = GameManager::getInstance()->getProjectData();
	auto voiceData = projectData->getVoiceData(voiceId);
	if (!voiceData){
        vp.set(JS::NullValue());
        return false;
    }

#if 0
	JSString* jss = JSID_TO_STRING(id);
	char *p = JS_EncodeString(cx, jss);
	if (strcmp(p, "operatable") == 0) {
		vp.setBoolean(voiceData->getOperatable());
	} else {
		CCLOG("Unimplemented property getter: %s", p);
		return false;
	}
#endif
	return true;
}

bool js_voice_setProperty(JSContext *cx, JS::HandleObject obj, JS::HandleId id, bool strict, JS::MutableHandleValue vp)
{
	JS::RootedValue v(cx);

	JS::RootedObject robj(cx, obj);
	JS_GetProperty(cx, robj, "voiceId", &v);
    if (!v.isNumber()){
        vp.set(JS::NullValue());
        return false;
    }
	auto voiceId = JavascriptManager::getInt32(v);
	auto projectData = GameManager::getInstance()->getProjectData();
	auto voiceData = projectData->getVoiceData(voiceId);
	if (!voiceData){
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
		voiceData->setOperatable(operatable);
	} else {
		CCLOG("Unimplemented property setter: %s", p);
		return false;
	}
#endif
	return true;
}

//===============================================================================================//
