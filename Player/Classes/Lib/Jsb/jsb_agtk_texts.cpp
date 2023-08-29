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

static bool register_agtk_texts(JSContext *_cx,JS::HandleObject object);

//texts
static bool js_texts_get(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_texts_getIdByName(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_texts_getIdList(JSContext *cx, unsigned argc, JS::Value *vp);
//text
static bool js_text_getText(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_text_getProperty(JSContext *cx, JS::HandleObject obj, JS::HandleId id, JS::MutableHandleValue vp);
static bool js_text_setProperty(JSContext *cx, JS::HandleObject obj, JS::HandleId id, bool strict, JS::MutableHandleValue vp);
//static bool js_text_getValue(JSContext *cx, unsigned argc, JS::Value *vp);
//static bool js_text_setValue(JSContext *cx, unsigned argc, JS::Value *vp);

#include "Lib/ObjectCommand.h"
#include "Data/ObjectCommandData.h"
#include "Manager/DebugManager.h"
#include "Data/PlayData.h"
#include "JavascriptManager.h"

void jsb_register_agtk_texts(JSContext *cx, JS::HandleObject object)
{
	auto ret = register_agtk_texts(cx, object);
	CCASSERT(ret, "Failed to register_agtk_texts");
}

bool register_agtk_texts(JSContext *cx, JS::HandleObject object)
{
	JS::RootedObject robj(cx, object);

	static JSClass texts_class = {
		"texts",
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
	auto texts = JS_DefineObject(cx, robj, "texts", &texts_class, JS::NullPtr(), JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE);
	CCASSERT(texts, "texts should not be nullptr");
	JS::RootedObject rtexts(cx, texts);
	static JSFunctionSpec texts_methods[] = {
		{ "get", js_texts_get, 0, 0, 0 },
		{ "getIdList", js_texts_getIdList, 0, 0, 0 },
		{ "getIdByName", js_texts_getIdByName, 0, 0, 0 },
		{ NULL },
	};
	JS_DefineFunctions(cx, rtexts, texts_methods);

	JS::RootedValue rval(cx);
	rval.setObject(*texts);
	JS_SetProperty(cx, robj, "texts", rval);
	return true;
}

// texts
enum {
	ObjectOperatable,
};
bool js_texts_get(JSContext *cx, unsigned argc, JS::Value *vp)
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
	auto textId = JavascriptManager::getInt32(v);
	auto projectData = GameManager::getInstance()->getProjectData();
	auto textData = projectData->getTextData(textId);
	if (!textData) {
		return true;
	}
	static JSClass text_class = {
		"text",
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
	auto text = JS_DefineObject(cx, jsthis, "text", &text_class, JS::NullPtr(), JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE);
	CCASSERT(text, "text should not be nullptr");
	JS::RootedObject rtext(cx, text);
	static JSFunctionSpec text_methods[] = {
		{ "getText", js_text_getText, 0, 0, 0 },
		{ NULL },
	};
	JS_DefineFunctions(cx, rtext, text_methods);
	JS::RootedValue rtextId(cx, JS::Int32Value(textId));
	JS_DefineProperty(cx, rtext, "id", rtextId, JSPROP_ENUMERATE | JSPROP_PERMANENT);
	JS::RootedValue rtextName(cx, std_string_to_jsval(cx, textData->getName()->getCString()));
	JS_DefineProperty(cx, rtext, "name", rtextName, JSPROP_ENUMERATE | JSPROP_PERMANENT);

	JS::RootedValue rfontId(cx, JS::Int32Value(textData->getFontId()));
	JS_DefineProperty(cx, rtext, "fontId", rfontId, JSPROP_ENUMERATE | JSPROP_PERMANENT);

	JS::RootedValue rletterSpacing(cx, JS::Int32Value(textData->getLetterSpacing()));
	JS_DefineProperty(cx, rtext, "letterSpacing", rletterSpacing, JSPROP_ENUMERATE | JSPROP_PERMANENT);
	JS::RootedValue rlineSpacing(cx, JS::Int32Value(textData->getLineSpacing()));
	JS_DefineProperty(cx, rtext, "lineSpacing", rlineSpacing, JSPROP_ENUMERATE | JSPROP_PERMANENT);
	static JSClass localeText_class = {
		"localeText",
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
	auto localeText = JS_DefineObject(cx, rtext, "localeText", &localeText_class, JS::NullPtr(), JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE);
	CCASSERT(localeText, "localeText should not be nullptr");
	JS::RootedObject rlocaleText(cx, localeText);
    auto textDic = textData->getTextList();
	cocos2d::Ref *ref = nullptr;
	auto allKeys = textDic->allKeys();
	CCARRAY_FOREACH(allKeys, ref) {
		auto locale = std::string(dynamic_cast<cocos2d::String *>(ref)->getCString());
		auto text = dynamic_cast<cocos2d::String *>(textDic->objectForKey(locale));
        JS::RootedValue rtext(cx, std_string_to_jsval(cx, text->getCString()));
        JS_DefineProperty(cx, rlocaleText, locale.c_str(), rtext, JSPROP_ENUMERATE | JSPROP_PERMANENT);
    }

	JS::RootedValue rval(cx);
	rval.setInt32(textId);
	JS_SetProperty(cx, rtext, "textId", rval);
	args.rval().set(OBJECT_TO_JSVAL(text));
	return true;
}

bool js_texts_getIdList(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//ret: <array : テキストIDの配列>
	JS::CallArgs args = CallArgsFromVp(argc, vp);

	int value = -1;
	auto projectData = GameManager::getInstance()->getProjectData();
	std::list<int> idList;
	std::function<void(cocos2d::Dictionary *)> scanRecur = [&scanRecur, &idList, projectData](cocos2d::Dictionary *children){
		cocos2d::DictElement *el = nullptr;
		CCDICT_FOREACH(children, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto p = static_cast<agtk::data::TextData *>(el->getObject());
#else
			auto p = dynamic_cast<agtk::data::TextData *>(el->getObject());
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
	scanRecur(projectData->getTextList());

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

bool js_texts_getIdByName(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//arg1: <string: 画像名>
	//ret: <int : 画像ID>
	JS::CallArgs args = CallArgsFromVp(argc, vp);
    args.rval().set(JS::Int32Value(-1));
	auto len = args.length();
	if (len < 1) {
		return false;
	}
	std::string textName;
	{
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
		auto &v2 = args[0];
#else
#endif
		if (!v2.isString()) {
			return false;
		}
		if (!jsval_to_std_string(cx, v2, &textName)) {
			return false;
		}
	}

	int value = -1;
	auto projectData = GameManager::getInstance()->getProjectData();
	auto textData = projectData->getTextDataByName(textName.c_str());

	if(textData){
		value = textData->getId();
	}
	args.rval().set(JS::Int32Value(value));
	return true;
}

// text
bool js_text_getText(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//arg1: <string: locale>
	//ret: <string : テキスト>
	JS::CallArgs args = CallArgsFromVp(argc, vp);
	args.rval().set(JS::NullValue());

	auto len = args.length();
	if (len < 1) {
		return false;
	}
	std::string locale;
	{
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
		auto &v2 = args[0];
#else
#endif
		if (!v2.isString()) {
			return false;
		}
		if (!jsval_to_std_string(cx, v2, &locale)) {
			return false;
		}
	}

	JS::RootedValue v(cx);
	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
	JS_GetProperty(cx, jsthis, "textId", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto textId = JavascriptManager::getInt32(v);

	auto projectData = GameManager::getInstance()->getProjectData();
	auto textData = projectData->getTextData(textId);
	if (!textData) {
		return true;
	}
	std::string text;
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
	text = projectData->getExpandedText(locale.c_str(), textData->getText(locale.c_str()), std::list<int>(1, textId)).c_str();
#else
#endif
	args.rval().set(std_string_to_jsval(cx, text));
	return true;
}

bool js_text_getProperty(JSContext *cx, JS::HandleObject obj, JS::HandleId id, JS::MutableHandleValue vp)
{
	JS::RootedValue v(cx);
    vp.set(JS::NullValue());

	JS::RootedObject robj(cx, obj);
	JS_GetProperty(cx, robj, "textId", &v);
    if (!v.isNumber()){
        return false;
    }
	auto textId = JavascriptManager::getInt32(v);
	auto projectData = GameManager::getInstance()->getProjectData();
	auto textData = projectData->getTextData(textId);
	if (!textData){
        return false;
    }

#if 0
	JSString* jss = JSID_TO_STRING(id);
	char *p = JS_EncodeString(cx, jss);
	if (strcmp(p, "operatable") == 0) {
		vp.setBoolean(textData->getOperatable());
	} else {
		CCLOG("Unimplemented property getter: %s", p);
		return false;
	}
#endif
	return true;
}

bool js_text_setProperty(JSContext *cx, JS::HandleObject obj, JS::HandleId id, bool strict, JS::MutableHandleValue vp)
{
	JS::RootedValue v(cx);
    vp.set(JS::NullValue());

	JS::RootedObject robj(cx, obj);
	JS_GetProperty(cx, robj, "textId", &v);
    if (!v.isNumber()){
        return false;
    }
	auto textId = JavascriptManager::getInt32(v);
	auto projectData = GameManager::getInstance()->getProjectData();
	auto textData = projectData->getTextData(textId);
	if (!textData){
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
			return false;
		}
		textData->setOperatable(operatable);
	} else {
		CCLOG("Unimplemented property setter: %s", p);
		return false;
	}
#endif
	return true;
}

#if 0
bool js_text_getValue(JSContext *cx, unsigned argc, JS::Value *vp)
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
	JS_GetProperty(cx, jsthis, "switchId", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto switchId = JavascriptManager::getInt32(v);

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
