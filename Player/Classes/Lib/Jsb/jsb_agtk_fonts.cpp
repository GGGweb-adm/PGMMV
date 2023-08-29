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

static bool register_agtk_fonts(JSContext *_cx,JS::HandleObject object);

//fonts
static bool js_fonts_get(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_fonts_getIdByName(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_fonts_getIdList(JSContext *cx, unsigned argc, JS::Value *vp);
//font
static bool js_font_getProperty(JSContext *cx, JS::HandleObject obj, JS::HandleId id, JS::MutableHandleValue vp);
static bool js_font_setProperty(JSContext *cx, JS::HandleObject obj, JS::HandleId id, bool strict, JS::MutableHandleValue vp);
//static bool js_font_getValue(JSContext *cx, unsigned argc, JS::Value *vp);
//static bool js_font_setValue(JSContext *cx, unsigned argc, JS::Value *vp);

#include "Lib/ObjectCommand.h"
#include "Data/ObjectCommandData.h"
#include "Manager/DebugManager.h"
#include "Data/PlayData.h"
#include "JavascriptManager.h"

void jsb_register_agtk_fonts(JSContext *cx, JS::HandleObject object)
{
	auto ret = register_agtk_fonts(cx, object);
	CCASSERT(ret, "Failed to register_agtk_fonts");
}

bool register_agtk_fonts(JSContext *cx, JS::HandleObject object)
{
	JS::RootedObject robj(cx, object);

	static JSClass fonts_class = {
		"fonts",
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
	auto fonts = JS_DefineObject(cx, robj, "fonts", &fonts_class, JS::NullPtr(), JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE);
	CCASSERT(fonts, "fonts should not be nullptr");
	JS::RootedObject rfonts(cx, fonts);
	static JSFunctionSpec fonts_methods[] = {
		{ "get", js_fonts_get, 0, 0, 0 },
		{ "getIdList", js_fonts_getIdList, 0, 0, 0 },
		{ "getIdByName", js_fonts_getIdByName, 0, 0, 0 },
		{ NULL },
	};
	JS_DefineFunctions(cx, rfonts, fonts_methods);

	JS::RootedValue rval(cx);
	rval.setObject(*fonts);
	JS_SetProperty(cx, robj, "fonts", rval);
	return true;
}

// fonts
enum {
	ObjectOperatable,
};
bool js_fonts_get(JSContext *cx, unsigned argc, JS::Value *vp)
{
	JS::CallArgs args = CallArgsFromVp(argc, vp);
	auto len = args.length();
	if (len < 1) {
		args.rval().set(JS::NullValue());
		return false;
	}
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
	auto &v = args[0];
#else
#endif
	if (!v.isNumber()) {
		args.rval().set(JS::NullValue());
		return false;
	}
	auto fontId = JavascriptManager::getInt32(v);
	auto projectData = GameManager::getInstance()->getProjectData();
	auto fontData = projectData->getFontData(fontId);
	if (!fontData) {
		args.rval().set(JS::NullValue());
		return true;
	}
	static JSClass font_class = {
		"font",
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
	auto font = JS_DefineObject(cx, jsthis, "font", &font_class, JS::NullPtr(), JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE);
	CCASSERT(font, "font should not be nullptr");
	JS::RootedObject rfont(cx, font);
#if 0
	static JSFunctionSpec font_methods[] = {
		//{ "getValue", js_switch_getValue, 0, 0, 0 },
		{ NULL },
	};
	JS_DefineFunctions(cx, rfont, font_methods);
#endif
	JS::RootedValue rfontId(cx, JS::Int32Value(fontId));
	JS_DefineProperty(cx, rfont, "id", rfontId, JSPROP_ENUMERATE | JSPROP_PERMANENT);
	JS::RootedValue rname(cx, std_string_to_jsval(cx, fontData->getName()));
	JS_DefineProperty(cx, rfont, "name", rname, JSPROP_ENUMERATE | JSPROP_PERMANENT);

	JS::RootedValue rimageFontFlag(cx, JS::BooleanValue(fontData->getMainFontSetting()->getImageFontFlag()));
	JS_DefineProperty(cx, rfont, "imageFontFlag", rimageFontFlag, JSPROP_ENUMERATE | JSPROP_PERMANENT);

	JS::RootedValue rimageId(cx, JS::Int32Value(fontData->getMainFontSetting()->getImageId()));
	JS_DefineProperty(cx, rfont, "imageId", rimageId, JSPROP_ENUMERATE | JSPROP_PERMANENT);

	JS::RootedValue rfontName(cx, std_string_to_jsval(cx, fontData->getFontName()));
	JS_DefineProperty(cx, rfont, "fontName", rfontName, JSPROP_ENUMERATE | JSPROP_PERMANENT);

	JS::RootedValue rttfName(cx, std_string_to_jsval(cx, fontData->getTTFName()));
	JS_DefineProperty(cx, rfont, "ttfName", rttfName, JSPROP_ENUMERATE | JSPROP_PERMANENT);
	JS::RootedValue rfontSize(cx, JS::Int32Value(fontData->getMainFontSetting()->getFontSize()));
	JS_DefineProperty(cx, rfont, "fontSize", rfontSize, JSPROP_ENUMERATE | JSPROP_PERMANENT);
	JS::RootedValue rantialiasDisabled(cx, JS::BooleanValue(fontData->getMainFontSetting()->getAntialiasDisabled()));
	JS_DefineProperty(cx, rfont, "antialiasDisabled", rantialiasDisabled, JSPROP_ENUMERATE | JSPROP_PERMANENT);
	JS::RootedValue raliasThreshold(cx, JS::Int32Value(fontData->getMainFontSetting()->getAliasThreshold()));
	JS_DefineProperty(cx, rfont, "aliasThreshold", raliasThreshold, JSPROP_ENUMERATE | JSPROP_PERMANENT);
	JS::RootedValue rfixedWidth(cx, JS::BooleanValue(fontData->getMainFontSetting()->getFixedWidth()));
	JS_DefineProperty(cx, rfont, "fixedWidth", rfixedWidth, JSPROP_ENUMERATE | JSPROP_PERMANENT);
	JS::RootedValue rhankakuWidth(cx, JS::Int32Value(fontData->getMainFontSetting()->getHankakuWidth()));
	JS_DefineProperty(cx, rfont, "hankakuWidth", rhankakuWidth, JSPROP_ENUMERATE | JSPROP_PERMANENT);
	JS::RootedValue rzenkakuWidth(cx, JS::Int32Value(fontData->getMainFontSetting()->getZenkakuWidth()));
	JS_DefineProperty(cx, rfont, "zenkakuWidth", rzenkakuWidth, JSPROP_ENUMERATE | JSPROP_PERMANENT);
	JS::RootedValue rletterLayout(cx, std_string_to_jsval(cx, fontData->getLetterLayout()));
	JS_DefineProperty(cx, rfont, "letterLayout", rletterLayout, JSPROP_ENUMERATE | JSPROP_PERMANENT);

	JS::RootedValue rval(cx);
	rval.setInt32(fontId);
	JS_SetProperty(cx, rfont, "fontId", rval);
	args.rval().set(OBJECT_TO_JSVAL(font));
	return true;
}

bool js_fonts_getIdList(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//ret: <array : フォントIDの配列>
	JS::CallArgs args = CallArgsFromVp(argc, vp);

	int value = -1;
	auto projectData = GameManager::getInstance()->getProjectData();
	std::list<int> idList;
	std::function<void(cocos2d::Dictionary *)> scanRecur = [&scanRecur, &idList, projectData](cocos2d::Dictionary *children){
		cocos2d::DictElement *el = nullptr;
		CCDICT_FOREACH(children, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto p = static_cast<agtk::data::FontData *>(el->getObject());
#else
			auto p = dynamic_cast<agtk::data::FontData *>(el->getObject());
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
	scanRecur(projectData->getFontList());

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

bool js_fonts_getIdByName(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//arg1: <string: 画像名>
	//ret: <int : 画像ID>
	JS::CallArgs args = CallArgsFromVp(argc, vp);
	auto len = args.length();
	if (len < 1) {
		args.rval().set(JS::NullValue());
		return false;
	}
	std::string fontName;
	{
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
		auto &v2 = args[0];
#else
#endif
		if (!v2.isString()) {
			args.rval().set(JS::Int32Value(-1));
			return false;
		}
		if (!jsval_to_std_string(cx, v2, &fontName)) {
			args.rval().set(JS::Int32Value(-1));
			return false;
		}
	}

	int value = -1;
	auto projectData = GameManager::getInstance()->getProjectData();
	auto fontData = projectData->getFontDataByName(fontName.c_str());

	if(fontData){
		value = fontData->getId();
	}
	args.rval().set(JS::Int32Value(value));
	return true;
}

// font
bool js_font_getProperty(JSContext *cx, JS::HandleObject obj, JS::HandleId id, JS::MutableHandleValue vp)
{
	JS::RootedValue v(cx);

	JS::RootedObject robj(cx, obj);
	JS_GetProperty(cx, robj, "fontId", &v);
    if (!v.isNumber()){
        vp.set(JS::NullValue());
        return false;
    }
	auto fontId = JavascriptManager::getInt32(v);
	auto projectData = GameManager::getInstance()->getProjectData();
	auto fontData = projectData->getFontData(fontId);
	if (!fontData){
        vp.set(JS::NullValue());
        return false;
    }

#if 0
	JSString* jss = JSID_TO_STRING(id);
	char *p = JS_EncodeString(cx, jss);
	if (strcmp(p, "operatable") == 0) {
		vp.setBoolean(fontData->getOperatable());
	} else {
		CCLOG("Unimplemented property getter: %s", p);
		return false;
	}
#endif
	return true;
}

bool js_font_setProperty(JSContext *cx, JS::HandleObject obj, JS::HandleId id, bool strict, JS::MutableHandleValue vp)
{
	JS::RootedValue v(cx);

	JS::RootedObject robj(cx, obj);
	JS_GetProperty(cx, robj, "fontId", &v);
    if (!v.isNumber()){
        vp.set(JS::NullValue());
        return false;
    }
	auto fontId = JavascriptManager::getInt32(v);
	auto projectData = GameManager::getInstance()->getProjectData();
	auto fontData = projectData->getFontData(fontId);
	if (!fontData){
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
		fontData->setOperatable(operatable);
	} else {
		CCLOG("Unimplemented property setter: %s", p);
		return false;
	}
#endif
	return true;
}

#if 0
bool js_font_getValue(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//ret: <bool : 値>
	JS::CallArgs args = CallArgsFromVp(argc, vp);

	JS::RootedValue v(cx);
	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
	JS_GetProperty(cx, jsthis, "switchId", &v);
	if (!v.isInt32()) {
		args.rval().set(JS::NullValue());
		return false;
	}
	auto switchId = v.toInt32();

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
	auto len = args.length();
	if (len < 1) {
		args.rval().set(JS::NullValue());
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
			args.rval().set(JS::FalseValue());
			return false;
		}
	}

	JS::RootedValue v(cx);
	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
	JS_GetProperty(cx, jsthis, "switchId", &v);
	if (!v.isInt32()) {
		args.rval().set(JS::NullValue());
		return false;
	}
	auto switchId = v.toInt32();

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
