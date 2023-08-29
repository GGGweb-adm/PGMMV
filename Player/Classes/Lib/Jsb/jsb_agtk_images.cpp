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

static bool register_agtk_images(JSContext *_cx,JS::HandleObject object);

//images
static bool js_images_get(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_images_getIdByName(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_images_getIdList(JSContext *cx, unsigned argc, JS::Value *vp);
//image
static bool js_image_getProperty(JSContext *cx, JS::HandleObject obj, JS::HandleId id, JS::MutableHandleValue vp);
static bool js_image_setProperty(JSContext *cx, JS::HandleObject obj, JS::HandleId id, bool strict, JS::MutableHandleValue vp);
//static bool js_image_getValue(JSContext *cx, unsigned argc, JS::Value *vp);
//static bool js_image_setValue(JSContext *cx, unsigned argc, JS::Value *vp);

#include "Lib/ObjectCommand.h"
#include "Data/ObjectCommandData.h"
#include "Manager/DebugManager.h"
#include "Data/PlayData.h"
#include "JavascriptManager.h"

void jsb_register_agtk_images(JSContext *cx, JS::HandleObject object)
{
	auto ret = register_agtk_images(cx, object);
	CCASSERT(ret, "Failed to register_agtk_images");
}

bool register_agtk_images(JSContext *cx, JS::HandleObject object)
{
	JS::RootedObject robj(cx, object);

	static JSClass images_class = {
		"images",
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
	auto images = JS_DefineObject(cx, robj, "images", &images_class, JS::NullPtr(), JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE);
	CCASSERT(images, "images should not be nullptr");
	JS::RootedObject rimages(cx, images);
	static JSFunctionSpec images_methods[] = {
		{ "get", js_images_get, 0, 0, 0 },
		{ "getIdList", js_images_getIdList, 0, 0, 0 },
		{ "getIdByName", js_images_getIdByName, 0, 0, 0 },
		{ NULL },
	};
	JS_DefineFunctions(cx, rimages, images_methods);

	JS::RootedValue rval(cx);
	rval.setObject(*images);
	JS_SetProperty(cx, robj, "images", rval);
	return true;
}

// images
enum {
	ObjectOperatable,
};
bool js_images_get(JSContext *cx, unsigned argc, JS::Value *vp)
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
	auto imageId = JavascriptManager::getInt32(v);
	auto projectData = GameManager::getInstance()->getProjectData();
	auto imageData = projectData->getImageData(imageId);
	if (!imageData) {
		return true;
	}
	static JSClass image_class = {
		"image",
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
	auto image = JS_DefineObject(cx, jsthis, "image", &image_class, JS::NullPtr(), JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE);
	CCASSERT(image, "image should not be nullptr");
	JS::RootedObject rimage(cx, image);
#if 0
	static JSFunctionSpec image_methods[] = {
		//{ "getValue", js_switch_getValue, 0, 0, 0 },
		{ NULL },
	};
	JS_DefineFunctions(cx, rimage, image_methods);
#endif
	JS::RootedValue rimageId(cx, JS::Int32Value(imageId));
	JS_DefineProperty(cx, rimage, "id", rimageId, JSPROP_ENUMERATE | JSPROP_PERMANENT);
	JS::RootedValue rimageName(cx, std_string_to_jsval(cx, imageData->getName()));
	JS_DefineProperty(cx, rimage, "name", rimageName, JSPROP_ENUMERATE | JSPROP_PERMANENT);
	JS::RootedValue rimageFilename(cx, std_string_to_jsval(cx, imageData->getFilename()));
	JS_DefineProperty(cx, rimage, "filename", rimageFilename, JSPROP_ENUMERATE | JSPROP_PERMANENT);
	JS::RootedValue rval(cx);
	rval.setInt32(imageId);
	JS_SetProperty(cx, rimage, "imageId", rval);
	args.rval().set(OBJECT_TO_JSVAL(image));
	return true;
}

bool js_images_getIdList(JSContext *cx, unsigned argc, JS::Value *vp)
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
			auto p = static_cast<agtk::data::ImageData *>(el->getObject());
#else
			auto p = dynamic_cast<agtk::data::ImageData *>(el->getObject());
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
	scanRecur(projectData->getImageList());

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

bool js_images_getIdByName(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//arg1: <string: 画像名>
	//ret: <int : 画像ID>
	JS::CallArgs args = CallArgsFromVp(argc, vp);
    args.rval().set(JS::Int32Value(-1));
	auto len = args.length();
	if (len < 1) {
		return false;
	}
	std::string imageName;
	{
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
		auto &v2 = args[0];
#else
#endif
		if (!v2.isString()) {
			return false;
		}
		if (!jsval_to_std_string(cx, v2, &imageName)) {
			return false;
		}
	}

	int value = -1;
	auto projectData = GameManager::getInstance()->getProjectData();
	auto imageData = projectData->getImageDataByName(imageName.c_str());

	if(imageData){
		value = imageData->getId();
	}
	args.rval().set(JS::Int32Value(value));
	return true;
}

// image
bool js_image_getProperty(JSContext *cx, JS::HandleObject obj, JS::HandleId id, JS::MutableHandleValue vp)
{
	JS::RootedValue v(cx);

	JS::RootedObject robj(cx, obj);
	JS_GetProperty(cx, robj, "imageId", &v);
    if (!v.isNumber()){
        vp.set(JS::NullValue());
        return false;
    }
	auto imageId = JavascriptManager::getInt32(v);
	auto projectData = GameManager::getInstance()->getProjectData();
	auto imageData = projectData->getImageData(imageId);
	if (!imageData){
        vp.set(JS::NullValue());
        return false;
    }

#if 0
	JSString* jss = JSID_TO_STRING(id);
	char *p = JS_EncodeString(cx, jss);
	if (strcmp(p, "operatable") == 0) {
		vp.setBoolean(imageData->getOperatable());
	} else {
		CCLOG("Unimplemented property getter: %s", p);
		return false;
	}
#endif
	return true;
}

bool js_image_setProperty(JSContext *cx, JS::HandleObject obj, JS::HandleId id, bool strict, JS::MutableHandleValue vp)
{
	JS::RootedValue v(cx);

	JS::RootedObject robj(cx, obj);
	JS_GetProperty(cx, robj, "imageId", &v);
    if (!v.isNumber()){
        vp.set(JS::NullValue());
        return false;
    }
	auto imageId = JavascriptManager::getInt32(v);
	auto projectData = GameManager::getInstance()->getProjectData();
	auto imageData = projectData->getImageData(imageId);
	if (!imageData){
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
		imageData->setOperatable(operatable);
	} else {
		CCLOG("Unimplemented property setter: %s", p);
		return false;
	}
#endif
	return true;
}

#if 0
bool js_image_getValue(JSContext *cx, unsigned argc, JS::Value *vp)
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
