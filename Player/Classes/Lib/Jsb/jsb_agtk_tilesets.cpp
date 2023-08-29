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

static bool register_agtk_tilesets(JSContext *_cx,JS::HandleObject object);

//tilesets
static bool js_tilesets_get(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_tilesets_getIdByName(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_tilesets_getIdList(JSContext *cx, unsigned argc, JS::Value *vp);
//tileset
static bool js_tileset_getWallBits(JSContext *cx, unsigned argc, JS::Value *vp);

#include "Lib/ObjectCommand.h"
#include "Data/ObjectCommandData.h"
#include "Manager/DebugManager.h"
#include "Data/PlayData.h"
#include "JavascriptManager.h"

void jsb_register_agtk_tilesets(JSContext *cx, JS::HandleObject object)
{
	auto ret = register_agtk_tilesets(cx, object);
	CCASSERT(ret, "Failed to register_agtk_tilesets");
}

bool register_agtk_tilesets(JSContext *cx, JS::HandleObject object)
{
	JS::RootedObject robj(cx, object);

	static JSClass tilesets_class = {
		"tilesets",
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
	auto tilesets = JS_DefineObject(cx, robj, "tilesets", &tilesets_class, JS::NullPtr(), JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE);
	CCASSERT(tilesets, "tilesets should not be nullptr");
	JS::RootedObject rtilesets(cx, tilesets);
	static JSFunctionSpec tilesets_methods[] = {
		{ "get", js_tilesets_get, 0, 0, 0 },
		{ "getIdList", js_tilesets_getIdList, 0, 0, 0 },
		{ "getIdByName", js_tilesets_getIdByName, 0, 0, 0 },
		{ NULL },
	};
	JS_DefineFunctions(cx, rtilesets, tilesets_methods);

	JS::RootedValue rval(cx);
	rval.setObject(*tilesets);
	JS_SetProperty(cx, robj, "tilesets", rval);
	return true;
}

// tilesets
enum {
	ObjectOperatable,
};
bool js_tilesets_get(JSContext *cx, unsigned argc, JS::Value *vp)
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
	auto tilesetId = JavascriptManager::getInt32(v);
	auto projectData = GameManager::getInstance()->getProjectData();
	auto tilesetData = projectData->getTilesetData(tilesetId);
	if (!tilesetData) {
		return true;
	}
	static JSClass tileset_class = {
		"tileset",
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
	auto tileset = JS_DefineObject(cx, jsthis, "tileset", &tileset_class, JS::NullPtr(), JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE);
	CCASSERT(tileset, "tileset should not be nullptr");
	JS::RootedObject rtileset(cx, tileset);
	static JSFunctionSpec tileset_methods[] = {
		{ "getWallBits", js_tileset_getWallBits, 0, 0, 0 },
		{ NULL },
	};
	JS_DefineFunctions(cx, rtileset, tileset_methods);
	JS::RootedValue rtilesetId(cx, JS::Int32Value(tilesetId));
	JS_DefineProperty(cx, rtileset, "id", rtilesetId, JSPROP_ENUMERATE | JSPROP_PERMANENT);
	JS::RootedValue rtilesetName(cx, std_string_to_jsval(cx, tilesetData->getName()));
	JS_DefineProperty(cx, rtileset, "name", rtilesetName, JSPROP_ENUMERATE | JSPROP_PERMANENT);
	JS::RootedValue rtilesetImageId(cx, JS::Int32Value(tilesetData->getImageId()));
	JS_DefineProperty(cx, rtileset, "imageId", rtilesetImageId, JSPROP_ENUMERATE | JSPROP_PERMANENT);
	JS::RootedValue rval(cx);
	rval.setInt32(tilesetId);
	JS_SetProperty(cx, rtileset, "tilesetId", rval);
	args.rval().set(OBJECT_TO_JSVAL(tileset));
	return true;
}

bool js_tilesets_getIdList(JSContext *cx, unsigned argc, JS::Value *vp)
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
			auto p = static_cast<agtk::data::TilesetData *>(el->getObject());
#else
			auto p = dynamic_cast<agtk::data::TilesetData *>(el->getObject());
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
	scanRecur(projectData->getTilesetList());

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

bool js_tilesets_getIdByName(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//arg1: <string: 画像名>
	//ret: <int : 画像ID>
	JS::CallArgs args = CallArgsFromVp(argc, vp);
    args.rval().set(JS::Int32Value(-1));
	auto len = args.length();
	if (len < 1) {
		return false;
	}
	std::string tilesetName;
	{
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
		auto &v2 = args[0];
#else
#endif
		if (!v2.isString()) {
			return false;
		}
		if (!jsval_to_std_string(cx, v2, &tilesetName)) {
			return false;
		}
	}

	int value = -1;
	auto projectData = GameManager::getInstance()->getProjectData();
	auto tilesetData = projectData->getTilesetDataByName(tilesetName.c_str());

	if(tilesetData){
		value = tilesetData->getId();
	}
	args.rval().set(JS::Int32Value(value));
	return true;
}

// tileset
bool js_tileset_getWallBits(JSContext *cx, unsigned argc, JS::Value *vp)
{
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
		auto &v = args[0];
#else
#endif
		if (!v.isNumber()) {
            return false;
        }
        x = JavascriptManager::getInt32(v);
    }
    {
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
		auto &v = args[1];
#else
#endif
		if (!v.isNumber()) {
            return false;
        }
        y = JavascriptManager::getInt32(v);
    }
	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
	JS::RootedValue v(cx);
	JS_GetProperty(cx, jsthis, "tilesetId", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto tilesetId = JavascriptManager::getInt32(v);
	auto projectData = GameManager::getInstance()->getProjectData();
	auto tilesetData = projectData->getTilesetData(tilesetId);
	if (!tilesetData) {
		return true;
	}
    if(x < 0 || x >= tilesetData->getHorzTileCount() || y < 0){
        return true;
    }
	auto id = x + y * tilesetData->getHorzTileCount();
    auto wallSetting = tilesetData->getWallSetting(id);
	JS::RootedValue rval(cx);
	rval.setInt32(wallSetting);
	args.rval().set(rval);
	return true;
}

//===============================================================================================//
