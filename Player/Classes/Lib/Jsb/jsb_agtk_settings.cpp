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

static bool register_agtk_settings(JSContext *_cx,JS::HandleObject object);
static bool register_agtk_playerCharacters(JSContext *cx, JSObject *agtkObj);

//settings

//playerCharacters
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_7
bool jsb_register_agtk_playerCharacterBase(JSContext *cx, JSObject *agtkObj);
#endif
static bool js_playerCharacters_getCount(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_playerCharacters_get(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_playerCharacter_getCount(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_playerCharacter_get(JSContext *cx, unsigned argc, JS::Value *vp);

#include "Lib/ObjectCommand.h"
#include "Data/ObjectCommandData.h"
#include "Manager/DebugManager.h"
#include "Data/PlayData.h"
#include "JavascriptManager.h"

void jsb_register_agtk_settings(JSContext *cx, JS::HandleObject object)
{
	auto ret = register_agtk_settings(cx, object);
	CCASSERT(ret, "Failed to register_agtk_settings");
}

bool register_agtk_settings(JSContext *cx, JS::HandleObject object)
{
	JS::RootedObject robj(cx, object);

	static JSClass settings_class = {
		"settings",
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
	auto settings = JS_DefineObject(cx, robj, "settings", &settings_class, JS::NullPtr(), JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE);
	CCASSERT(settings, "settings should not be nullptr");
	JS::RootedObject rsettings(cx, settings);
#if 0
	static JSFunctionSpec settings_methods[] = {
		{ "get", js_settings_get, 0, 0, 0 },
		{ "getIdList", js_settings_getIdList, 0, 0, 0 },
		{ "getIdByName", js_settings_getIdByName, 0, 0, 0 },
		{ NULL },
	};
	JS_DefineFunctions(cx, rsettings, settings_methods);
#endif

	register_agtk_playerCharacters(cx, settings);

	auto projectData = GameManager::getInstance()->getProjectData();
	JS::RootedValue rtileWidth(cx, JS::Int32Value(projectData->getTileWidth()));
	JS_DefineProperty(cx, rsettings, "tileWidth", rtileWidth, JSPROP_ENUMERATE | JSPROP_PERMANENT);
	JS::RootedValue rtileHeight(cx, JS::Int32Value(projectData->getTileHeight()));
	JS_DefineProperty(cx, rsettings, "tileHeight", rtileHeight, JSPROP_ENUMERATE | JSPROP_PERMANENT);
	JS::RootedValue rscreenWidth(cx, JS::Int32Value(projectData->getScreenWidth()));
	JS_DefineProperty(cx, rsettings, "screenWidth", rscreenWidth, JSPROP_ENUMERATE | JSPROP_PERMANENT);
	JS::RootedValue rscreenHeight(cx, JS::Int32Value(projectData->getScreenHeight()));
	JS_DefineProperty(cx, rsettings, "screenHeight", rscreenHeight, JSPROP_ENUMERATE | JSPROP_PERMANENT);
	JS::RootedValue rprojectPath(cx, std_string_to_jsval(cx, GameManager::getProjectPathFromProjectFile(GameManager::getInstance()->getProjectFilePath()->getCString())));
	JS_DefineProperty(cx, rsettings, "projectPath", rprojectPath, JSPROP_ENUMERATE | JSPROP_PERMANENT);

	JS::RootedValue rval(cx);
	rval.setObject(*settings);
	JS_SetProperty(cx, robj, "settings", rval);

// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_7
	if (!jsb_register_agtk_playerCharacterBase(cx, object.get())) return false;
#endif

	return true;
}

bool register_agtk_playerCharacters(JSContext *cx, JSObject *agtkObj)
{
	JS::RootedObject robj(cx, agtkObj);

	static JSClass playerCharacters_class = {
		"playerCharacters",
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
	auto playerCharacters = JS_DefineObject(cx, robj, "playerCharacters", &playerCharacters_class, JS::NullPtr(), JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE);
	CCASSERT(playerCharacters, "playerCharacters should not be nullptr");
	JS::RootedObject rplayerCharacters(cx, playerCharacters);
	static JSFunctionSpec playerCharacters_methods[] = {
		{ "getCount", js_playerCharacters_getCount, 0, 0, 0 },
		{ "get", js_playerCharacters_get, 0, 0, 0 },
		{ NULL },
	};
	JS_DefineFunctions(cx, rplayerCharacters, playerCharacters_methods);
	//auto nsval = OBJECT_TO_JSVAL(playerCharacters);
	JS::RootedValue rval(cx);
	rval.setObject(*playerCharacters);
	JS_SetProperty(cx, robj, "playerCharacters", rval);
	return true;
}

bool js_playerCharacters_getCount(JSContext *cx, unsigned argc, JS::Value *vp)
{
	JS::CallArgs args = CallArgsFromVp(argc, vp);
	auto projectData = GameManager::getInstance()->getProjectData();
	if (!projectData) {
		args.rval().set(JS::NullValue());
		return false;
	}
#define MAX_CHARACTERS  4
	args.rval().set(JS::Int32Value(MAX_CHARACTERS));
	return true;
}

// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_7
bool jsb_register_agtk_playerCharacterBase(JSContext *cx, JSObject *agtkObj)
{
	JS::RootedObject robj(cx, agtkObj);

	static JSClass playerCharacter_class = {
		"playerCharacterBase",
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
	auto playerCharacter = JS_DefineObject(cx, robj, "playerCharacterBase", &playerCharacter_class, JS::NullPtr(), JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE);
	CCASSERT(playerCharacter, "playerCharacterBase should not be nullptr");
	JS::RootedObject rplayerCharacter(cx, playerCharacter);
	static JSFunctionSpec playerCharacter_methods[] = {
		{ "getCount", js_playerCharacter_getCount, 0, 0, 0 },
		{ "get", js_playerCharacter_get, 0, 0, 0 },
		{ NULL },
	};
	JS_DefineFunctions(cx, rplayerCharacter, playerCharacter_methods);

	return true;
}
#endif

bool js_playerCharacters_get(JSContext *cx, unsigned argc, JS::Value *vp)
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
	auto projectData = GameManager::getInstance()->getProjectData();
	if (!projectData) {
		args.rval().set(JS::NullValue());
		return false;
	}
	int playerCount = projectData->getPlayerCount();
	auto index = JavascriptManager::getInt32(v);
	if (index < 0 || index >= MAX_CHARACTERS) {
		args.rval().set(JS::NullValue());
		return false;
	}
	static JSClass playerCharacter_class = {
		"playerCharacter",
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
	JS::RootedValue rplayerCharacterBaseVal(cx);
	JS_GetProperty(cx, jsObj, "playerCharacterBase", &rplayerCharacterBaseVal);
	if (!rplayerCharacterBaseVal.isObject()) {
		return false;
	}

	JSObject* playerCharacterBase = rplayerCharacterBaseVal.get().toObjectOrNull();
	JS::RootedObject rplayerCharacterBase(cx, playerCharacterBase);
#endif
	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_7
	auto playerCharacter = JS_DefineObject(cx, jsthis, "playerCharacter", &playerCharacter_class, rplayerCharacterBase, JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE);
#else
	auto playerCharacter = JS_DefineObject(cx, jsthis, "playerCharacter", &playerCharacter_class, JS::NullPtr(), JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE);
#endif
	CCASSERT(playerCharacter, "playerCharacter should not be nullptr");
	JS::RootedObject rplayerCharacter(cx, playerCharacter);
// #AGTK-NX
#ifndef USE_SAR_OPTIMIZE_7
	static JSFunctionSpec playerCharacter_methods[] = {
		{ "getCount", js_playerCharacter_getCount, 0, 0, 0 },
		{ "get", js_playerCharacter_get, 0, 0, 0 },
		{ NULL },
	};
	JS_DefineFunctions(cx, rplayerCharacter, playerCharacter_methods);
#endif
	JS::RootedValue rval(cx);
	rval.setInt32(index);
	JS_SetProperty(cx, rplayerCharacter, "index", rval);
	args.rval().set(OBJECT_TO_JSVAL(playerCharacter));
	return true;
}

bool js_playerCharacter_getCount(JSContext *cx, unsigned argc, JS::Value *vp)
{
	JS::CallArgs args = CallArgsFromVp(argc, vp);
	auto projectData = GameManager::getInstance()->getProjectData();
	if (!projectData) {
		args.rval().set(JS::NullValue());
		return false;
	}
	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
	JS::RootedValue v(cx);
	JS_GetProperty(cx, jsthis, "index", &v);
	if (!v.isNumber()) {
		args.rval().set(JS::NullValue());
		return false;
	}
	auto index = JavascriptManager::getInt32(v);
	auto playerCharacterList = projectData->getPlayerCharacterData()->getPlayerCharacterList(index);
	args.rval().set(JS::Int32Value(playerCharacterList->count()));
	return true;
}

bool js_playerCharacter_get(JSContext *cx, unsigned argc, JS::Value *vp)
{
	JS::CallArgs args = CallArgsFromVp(argc, vp);
	auto len = args.length();
	if (len < 1) {
		args.rval().set(JS::NullValue());
		return false;
	}
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
	auto &v2 = args[0];
#else
#endif
	if (!v2.isInt32()) {
		args.rval().set(JS::NullValue());
		return false;
	}
	auto index2 = v2.toInt32();
	auto projectData = GameManager::getInstance()->getProjectData();
	if (!projectData) {
		args.rval().set(JS::NullValue());
		return false;
	}
	JS::RootedValue v(cx);
	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
	JS_GetProperty(cx, jsthis, "index", &v);
	if (!v.isNumber()) {
		args.rval().set(JS::NullValue());
		return false;
	}
	auto index = JavascriptManager::getInt32(v);
	auto playerCharacterList = projectData->getPlayerCharacterData()->getPlayerCharacterList(index);
	if (playerCharacterList == nullptr) {
		args.rval().set(JS::NullValue());
		return false;
	}
	if (index2 < 0 || index2 >= playerCharacterList->count()) {
		args.rval().set(JS::NullValue());
		return false;
	}
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	auto objectIdPtr = static_cast<cocos2d::Integer *>(playerCharacterList->getObjectAtIndex(index2));
#else
	auto objectIdPtr = dynamic_cast<cocos2d::Integer *>(playerCharacterList->getObjectAtIndex(index2));
#endif
	auto objectId = objectIdPtr->getValue();
	args.rval().set(JS::Int32Value(objectId));
	return true;
}

//===============================================================================================//
