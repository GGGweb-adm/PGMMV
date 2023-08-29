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

static bool register_agtk_objects(JSContext *_cx,JS::HandleObject object);

//objects
static bool js_objects_get(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_objects_getIdByName(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_objects_getIdList(JSContext *cx, unsigned argc, JS::Value *vp);
//object
static bool js_object_getProperty(JSContext *cx, JS::HandleObject obj, JS::HandleId id, JS::MutableHandleValue vp);
static bool js_object_setProperty(JSContext *cx, JS::HandleObject obj, JS::HandleId id, bool strict, JS::MutableHandleValue vp);
//static bool js_object_getValue(JSContext *cx, unsigned argc, JS::Value *vp);
//static bool js_object_setValue(JSContext *cx, unsigned argc, JS::Value *vp);
//bullets
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_7
static bool jsb_register_agtk_object_bulletsBase(JSContext *cx, JSObject *agtkObj);
#endif
static bool jsb_register_agtk_object_bullets(JSContext *cx, JSObject *agtkObj);
static bool js_bullets_get(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_bullets_getIdByName(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_bullets_getIdList(JSContext *cx, unsigned argc, JS::Value *vp);
//bullet
//static bool js_bullet_getProperty(JSContext *cx, JS::HandleObject obj, JS::HandleId id, JS::MutableHandleValue vp);
//static bool js_bullet_setProperty(JSContext *cx, JS::HandleObject obj, JS::HandleId id, bool strict, JS::MutableHandleValue vp);
//actions
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_7
static bool jsb_register_agtk_object_actionsBase(JSContext *cx, JSObject *agtkObj);
#endif
static bool jsb_register_agtk_object_actions(JSContext *cx, JSObject *agtkObj);
static bool js_actions_get(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_actions_getIdByName(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_actions_getIdList(JSContext *cx, unsigned argc, JS::Value *vp);
//action
//viewports
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_7
static bool jsb_register_agtk_object_viewportsBase(JSContext *cx, JSObject *agtkObj);
#endif
static bool jsb_register_agtk_object_viewports(JSContext *cx, JSObject *agtkObj);
static bool js_viewports_get(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_viewports_getIdByName(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_viewports_getIdList(JSContext *cx, unsigned argc, JS::Value *vp);
//viewport
//switches
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_7
static bool jsb_register_agtk_object_switchesBase(JSContext *cx, JSObject *agtkObj);
#endif
static bool jsb_register_agtk_object_switches(JSContext *cx, JSObject *agtkObj);
static bool js_switches_get(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_switches_getIdByName(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_switches_getIdList(JSContext *cx, unsigned argc, JS::Value *vp);
//switch
//variables
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_7
static bool jsb_register_agtk_object_variablesBase(JSContext *cx, JSObject *agtkObj);
#endif
static bool jsb_register_agtk_object_variables(JSContext *cx, JSObject *agtkObj);
static bool js_variables_get(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_variables_getIdByName(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_variables_getIdList(JSContext *cx, unsigned argc, JS::Value *vp);
//variable

#include "Lib/ObjectCommand.h"
#include "Data/ObjectCommandData.h"
#include "Manager/DebugManager.h"
#include "Data/PlayData.h"
#include "JavascriptManager.h"

void jsb_register_agtk_objects(JSContext *cx, JS::HandleObject object)
{
	auto ret = register_agtk_objects(cx, object);
	CCASSERT(ret, "Failed to register_agtk_objects");
}

bool register_agtk_objects(JSContext *cx, JS::HandleObject object)
{
	JS::RootedObject robj(cx, object);

	static JSClass objects_class = {
		"objects",
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
	auto objects = JS_DefineObject(cx, robj, "objects", &objects_class, JS::NullPtr(), JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE);
	CCASSERT(objects, "objects should not be nullptr");
	JS::RootedObject robjects(cx, objects);
	static JSFunctionSpec objects_methods[] = {
		{ "get", js_objects_get, 0, 0, 0 },
		{ "getIdByName", js_objects_getIdByName, 0, 0, 0 },
		{ "getIdList", js_objects_getIdList, 0, 0, 0 },
		{ NULL },
	};
	JS_DefineFunctions(cx, robjects, objects_methods);

	JS::RootedValue rval(cx);
	rval.setObject(*objects);
	JS_SetProperty(cx, robj, "objects", rval);

// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_7
	if (!jsb_register_agtk_object_bulletsBase(cx, object.get())) return false;
	if (!jsb_register_agtk_object_actionsBase(cx, object.get())) return false;
	if (!jsb_register_agtk_object_viewportsBase(cx, object.get())) return false;
	if (!jsb_register_agtk_object_switchesBase(cx, object.get())) return false;
	if (!jsb_register_agtk_object_variablesBase(cx, object.get())) return false;
#endif

	return true;
}

// objects
enum {
	ObjectOperatable,
};
bool js_objects_get(JSContext *cx, unsigned argc, JS::Value *vp)
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
	auto objectId = JavascriptManager::getInt32(v);
	auto projectData = GameManager::getInstance()->getProjectData();
	auto objectData = projectData->getObjectData(objectId);
	if (!objectData) {
		return true;
	}
	static JSClass object_class = {
		"object",
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
	auto object = JS_DefineObject(cx, jsthis, "object", &object_class, JS::NullPtr(), JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE);
	CCASSERT(object, "object should not be nullptr");
	JS::RootedObject robject(cx, object);
#if 0
	static JSFunctionSpec object_methods[] = {
		{ "get", js_bullet_get, 0, 0, 0 },
		{ "getIdList", js_bullet_getIdList, 0, 0, 0 },
		{ "getIdByName", js_bullet_getIdByName, 0, 0, 0 },
		{ NULL },
	};
	JS_DefineFunctions(cx, robject, object_methods);
#endif
	JS::RootedValue robjectId(cx, JS::Int32Value(objectId));
	JS_DefineProperty(cx, robject, "id", robjectId, JSPROP_ENUMERATE | JSPROP_PERMANENT);
	JS::RootedValue robjectName(cx, std_string_to_jsval(cx, objectData->getName()));
	JS_DefineProperty(cx, robject, "name", robjectName, JSPROP_ENUMERATE | JSPROP_PERMANENT);
	JS_DefineProperty(cx, robject, "operatable", ObjectOperatable, JSPROP_ENUMERATE, js_object_getProperty, js_object_setProperty);
	JS::RootedValue ranimationId(cx, JS::Int32Value(objectData->getAnimationId()));
	JS_DefineProperty(cx, robject, "animationId", ranimationId, JSPROP_ENUMERATE | JSPROP_PERMANENT);
	JS::RootedValue rval(cx);
	rval.setInt32(objectId);
	JS_SetProperty(cx, robject, "objectId", rval);
	if (!jsb_register_agtk_object_bullets(cx, object)) return false;
	if (!jsb_register_agtk_object_actions(cx, object)) return false;
	if (!jsb_register_agtk_object_viewports(cx, object)) return false;
	if (!jsb_register_agtk_object_switches(cx, object)) return false;
	if (!jsb_register_agtk_object_variables(cx, object)) return false;
	args.rval().set(OBJECT_TO_JSVAL(object));
	return true;
}

bool js_objects_getIdByName(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//arg1: <string: オブジェクトインスタンス名>
	//ret: <int : オブジェクトインスタンスID>
	JS::CallArgs args = CallArgsFromVp(argc, vp);
    args.rval().set(JS::Int32Value(-1));
	auto len = args.length();
	if (len < 1) {
		return false;
	}
	std::string objectName;
	{
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
		auto &v2 = args[0];
#else
#endif
		if (!v2.isString()) {
			return false;
		}
		if (!jsval_to_std_string(cx, v2, &objectName)) {
			return false;
		}
	}

	int value = -1;
	auto projectData = GameManager::getInstance()->getProjectData();
	auto objectData = projectData->getObjectDataByName(objectName.c_str());

	if(objectData){
		value = objectData->getId();
	}
	args.rval().set(JS::Int32Value(value));
	return true;
}

bool js_objects_getIdList(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//ret: <array : 弾IDの配列>
	JS::CallArgs args = CallArgsFromVp(argc, vp);

	std::list<int> idList;
    do {
        JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
        auto projectData = GameManager::getInstance()->getProjectData();
        auto objectList = projectData->getObjectList();
        cocos2d::DictElement *el = nullptr;
        CCDICT_FOREACH(objectList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto p = static_cast<agtk::data::ObjectData *>(el->getObject());
#else
			auto p = dynamic_cast<agtk::data::ObjectData *>(el->getObject());
#endif
            idList.push_back(p->getId());
        }
    } while(0);

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

// object
bool js_object_getProperty(JSContext *cx, JS::HandleObject obj, JS::HandleId id, JS::MutableHandleValue vp)
{
	JS::RootedValue v(cx);
    vp.set(JS::NullValue());

	JS::RootedObject robj(cx, obj);
	JS_GetProperty(cx, robj, "objectId", &v);
    if (!v.isNumber()){
        return false;
    }
	auto objectId = JavascriptManager::getInt32(v);
	auto projectData = GameManager::getInstance()->getProjectData();
	auto objectData = projectData->getObjectData(objectId);
	if (!objectData){
        return false;
    }

	JSString* jss = JSID_TO_STRING(id);
	char *p = JS_EncodeString(cx, jss);
	if (strcmp(p, "operatable") == 0) {
		vp.setBoolean(objectData->getOperatable());
	} else {
		CCLOG("Unimplemented property getter: %s", p);
		return false;
	}
	return true;
}

bool js_object_setProperty(JSContext *cx, JS::HandleObject obj, JS::HandleId id, bool strict, JS::MutableHandleValue vp)
{
	JS::RootedValue v(cx);

	JS::RootedObject robj(cx, obj);
	JS_GetProperty(cx, robj, "objectId", &v);
    if (!v.isNumber()){
		vp.set(JS::NullValue());
		return false;
    }
	auto objectId = JavascriptManager::getInt32(v);
	auto projectData = GameManager::getInstance()->getProjectData();
	auto objectData = projectData->getObjectData(objectId);
	if (!objectData){
		vp.set(JS::NullValue());
		return false;
    }

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
		objectData->setOperatable(operatable);
	} else {
		CCLOG("Unimplemented property setter: %s", p);
		vp.set(JS::NullValue());
		return false;
	}
	return true;
}

// bullets
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_7
bool jsb_register_agtk_object_bulletsBase(JSContext *cx, JSObject *agtkObj)
{
	JS::RootedObject robj(cx, agtkObj);

	static JSClass bullets_class = {
		"bulletsBase",
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
	auto bullets = JS_DefineObject(cx, robj, "bulletsBase", &bullets_class, JS::NullPtr(), JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE);
	CCASSERT(bullets, "bulletsBase should not be nullptr");
	JS::RootedObject rbullets(cx, bullets);
	static JSFunctionSpec bullets_methods[] = {
		{ "get", js_bullets_get, 0, 0, 0 },
		{ "getIdByName", js_bullets_getIdByName, 0, 0, 0 },
		{ "getIdList", js_bullets_getIdList, 0, 0, 0 },
		{ NULL },
	};
	JS_DefineFunctions(cx, rbullets, bullets_methods);
	return true;
}
#endif

bool jsb_register_agtk_object_bullets(JSContext *cx, JSObject *agtkObj)
{
	JS::RootedObject robj(cx, agtkObj);

	static JSClass bullets_class = {
		"bullets",
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
	JS::RootedValue rbulletsBaseVal(cx);
	JS_GetProperty(cx, jsObj, "bulletsBase", &rbulletsBaseVal);
	if (!rbulletsBaseVal.isObject()) {
		return false;
	}

	JSObject* bulletsBase = rbulletsBaseVal.get().toObjectOrNull();
	JS::RootedObject rbulletsBase(cx, bulletsBase);
#endif
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_7
	auto bullets = JS_DefineObject(cx, robj, "bullets", &bullets_class, rbulletsBase, JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE);
#else
	auto bullets = JS_DefineObject(cx, robj, "bullets", &bullets_class, JS::NullPtr(), JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE);
#endif
	CCASSERT(bullets, "bullets should not be nullptr");
	JS::RootedObject rbullets(cx, bullets);
// #AGTK-NX
#ifndef USE_SAR_OPTIMIZE_7
	static JSFunctionSpec bullets_methods[] = {
		{ "get", js_bullets_get, 0, 0, 0 },
		{ "getIdByName", js_bullets_getIdByName, 0, 0, 0 },
		{ "getIdList", js_bullets_getIdList, 0, 0, 0 },
		{ NULL },
	};
	JS_DefineFunctions(cx, rbullets, bullets_methods);

	JS::RootedValue rval(cx);
	rval.setObject(*bullets);
	JS_SetProperty(cx, robj, "bullets", rval);
#endif
	{
		JS::RootedValue v(cx);
		JS_GetProperty(cx, robj, "id", &v);
		if (!v.isNumber()) {
			return false;
		}
		JS_DefineProperty(cx, rbullets, "objectId", v, JSPROP_ENUMERATE | JSPROP_PERMANENT);
	}
	return true;
}

bool js_bullets_get(JSContext *cx, unsigned argc, JS::Value *vp)
{
	JS::CallArgs args = CallArgsFromVp(argc, vp);
    args.rval().set(JS::NullValue());
	auto len = args.length();
	if (len < 1) {
		return false;
	}
	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
	auto &v = args[0];
#else
#endif
	if (!v.isNumber()) {
		return false;
	}
	auto bulletId = JavascriptManager::getInt32(v);
	int objectId = -1;
	{
		JS::RootedValue v(cx);
		JS_GetProperty(cx, jsthis, "objectId", &v);
		if (!v.isNumber()) {
			return false;
		}
		objectId = JavascriptManager::getInt32(v);
	}
	auto projectData = GameManager::getInstance()->getProjectData();
	auto objectData = projectData->getObjectData(objectId);
	if (!objectData) {
		return true;
	}
	if (!objectData->getFireBulletSettingFlag()) {
		return true;
	}
	auto fireBulletSettingData = objectData->getFireBulletSettingData(bulletId);
	//auto bulletData = projectData->getBulletData(bulletId);
	//	ObjectFireBulletSettingData *getFireBulletSettingData(int id);
#if 0
	CC_SYNTHESIZE(bool, _fireBulletSettingFlag, FireBulletSettingFlag);//bool
	CC_SYNTHESIZE_RETAIN(cocos2d::__Dictionary *, _fireBulletSettingList, FireBulletSettingList);//->ObjectFireBulletSettingData
#endif
	if (!fireBulletSettingData) {
		return true;
	}
	static JSClass bullet_class = {
		"bullet",
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
	auto bullet = JS_DefineObject(cx, jsthis, "bullet", &bullet_class, JS::NullPtr(), JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE);
	CCASSERT(bullet, "bullet should not be nullptr");
	JS::RootedObject rbullet(cx, bullet);
#if 0
	static JSFunctionSpec bullet_methods[] = {
		//{ "getValue", js_switch_getValue, 0, 0, 0 },
		{ NULL },
	};
	JS_DefineFunctions(cx, rbullet, bullet_methods);
#endif
	JS::RootedValue rbulletId(cx, JS::Int32Value(bulletId));
	JS_DefineProperty(cx, rbullet, "id", rbulletId, JSPROP_ENUMERATE | JSPROP_PERMANENT);
	JS::RootedValue rbulletName(cx, std_string_to_jsval(cx, fireBulletSettingData->getName()));
	JS_DefineProperty(cx, rbullet, "name", rbulletName, JSPROP_ENUMERATE | JSPROP_PERMANENT);
	JS::RootedValue rval(cx);
	rval.setInt32(bulletId);
	JS_SetProperty(cx, rbullet, "bulletId", rval);
	args.rval().set(OBJECT_TO_JSVAL(bullet));
	return true;
}

bool js_bullets_getIdList(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//ret: <array : 弾IDの配列>
	JS::CallArgs args = CallArgsFromVp(argc, vp);

	std::list<int> idList;
    do {
        JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
        int objectId = -1;
        {
            JS::RootedValue v(cx);
            JS_GetProperty(cx, jsthis, "objectId", &v);
            if (!v.isNumber()) {
                break;
            }
            objectId = JavascriptManager::getInt32(v);
        }
        auto projectData = GameManager::getInstance()->getProjectData();
        auto objectData = projectData->getObjectData(objectId);
        if (objectData) {
            if (objectData->getFireBulletSettingFlag()) {
                auto fireBulletSettingList = objectData->getFireBulletSettingList();
                cocos2d::DictElement *el = nullptr;
                CCDICT_FOREACH(fireBulletSettingList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
					auto p = static_cast<agtk::data::ObjectFireBulletSettingData *>(el->getObject());
#else
					auto p = dynamic_cast<agtk::data::ObjectFireBulletSettingData *>(el->getObject());
#endif
                    idList.push_back(p->getId());
                }
            }
        }
    } while(0);

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

bool js_bullets_getIdByName(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//arg1: <string: 弾名>
	//ret: <int : 弾ID>、見つからなければ-1。
	JS::CallArgs args = CallArgsFromVp(argc, vp);
    args.rval().set(JS::Int32Value(-1));
	auto len = args.length();
	if (len < 1) {
		return false;
	}
	std::string bulletName;
	{
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
		auto &v2 = args[0];
#else
#endif
		if (!v2.isString()) {
			return false;
		}
		if (!jsval_to_std_string(cx, v2, &bulletName)) {
			return false;
		}
	}

	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
	int objectId = -1;
	{
		JS::RootedValue v(cx);
		JS_GetProperty(cx, jsthis, "objectId", &v);
		if (!v.isNumber()) {
			return false;
		}
		objectId = JavascriptManager::getInt32(v);
	}
	auto projectData = GameManager::getInstance()->getProjectData();
	auto objectData = projectData->getObjectData(objectId);
	if (!objectData) {
		return true;
	}
	if (!objectData->getFireBulletSettingFlag()) {
		return true;
	}
	auto fireBulletSettingList = objectData->getFireBulletSettingList();
	cocos2d::DictElement *el = nullptr;
	CCDICT_FOREACH(fireBulletSettingList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto p = static_cast<agtk::data::ObjectFireBulletSettingData *>(el->getObject());
#else
		auto p = dynamic_cast<agtk::data::ObjectFireBulletSettingData *>(el->getObject());
#endif
		if (strcmp(p->getName(), bulletName.c_str()) == 0) {
			args.rval().set(JS::Int32Value(p->getId()));
			return true;
		}
	}

	return true;
}

// bullet
#if 0
bool js_bullet_getProperty(JSContext *cx, JS::HandleObject obj, JS::HandleId id, JS::MutableHandleValue vp)
{
	JS::RootedValue v(cx);
    vp.set(JS::NullValue());

	JS::RootedObject robj(cx, obj);
	JS_GetProperty(cx, robj, "bulletId", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto bulletId = JavascriptManager::getInt32(v);
	auto projectData = GameManager::getInstance()->getProjectData();
	auto bulletData = projectData->getBulletData(bulletId);
	if (!bulletData) {
		return false;
	}

#if 0
	JSString* jss = JSID_TO_STRING(id);
	char *p = JS_EncodeString(cx, jss);
	if (strcmp(p, "operatable") == 0) {
		vp.setBoolean(bulletData->getOperatable());
	}
	else {
		CCLOG("Unimplemented property getter: %s", p);
		return false;
	}
#endif
	return true;
}

bool js_bullet_setProperty(JSContext *cx, JS::HandleObject obj, JS::HandleId id, bool strict, JS::MutableHandleValue vp)
{
	JS::RootedValue v(cx);
    vp.set(JS::NullValue());

	JS::RootedObject robj(cx, obj);
	JS_GetProperty(cx, robj, "bulletId", &v);
	if (!v.isNumber()) {
		return false;
	}
	auto bulletId = JavascriptManager::getInt32(v);
	auto projectData = GameManager::getInstance()->getProjectData();
	auto bulletData = projectData->getBulletData(bulletId);
	if (!bulletData) {
		return false;
	}

#if 0
	JSString* jss = JSID_TO_STRING(id);
	char *p = JS_EncodeString(cx, jss);
	if (strcmp(p, "operatable") == 0) {
		bool operatable = false;
		if (vp.isBoolean()) {
			operatable = vp.toBoolean();
		}
		else if (vp.isInt32()) {
			operatable = vp.toInt32() != 0;
		}
		else if (vp.isDouble()) {
			operatable = vp.toDouble() != 0;
		}
		else {
			return false;
		}
		bulletData->setOperatable(operatable);
	}
	else {
		CCLOG("Unimplemented property setter: %s", p);
		return false;
	}
#endif
	return true;
}
#endif

// actions
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_7
bool jsb_register_agtk_object_actionsBase(JSContext *cx, JSObject *agtkObj)
{
	JS::RootedObject robj(cx, agtkObj);

	static JSClass actions_class = {
		"actionsBase",
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
	auto actions = JS_DefineObject(cx, robj, "actionsBase", &actions_class, JS::NullPtr(), JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE);
	CCASSERT(actions, "actionsBase should not be nullptr");
	JS::RootedObject ractions(cx, actions);
	static JSFunctionSpec actions_methods[] = {
		{ "get", js_actions_get, 0, 0, 0 },
		{ "getIdByName", js_actions_getIdByName, 0, 0, 0 },
		{ "getIdList", js_actions_getIdList, 0, 0, 0 },
		{ NULL },
	};
	JS_DefineFunctions(cx, ractions, actions_methods);
	return true;
}
#endif

bool jsb_register_agtk_object_actions(JSContext *cx, JSObject *agtkObj)
{
	JS::RootedObject robj(cx, agtkObj);

	static JSClass actions_class = {
		"actions",
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
	JS::RootedValue ractionsBaseVal(cx);
	JS_GetProperty(cx, jsObj, "actionsBase", &ractionsBaseVal);
	if (!ractionsBaseVal.isObject()) {
		return false;
	}

	JSObject* actionsBase = ractionsBaseVal.get().toObjectOrNull();
	JS::RootedObject ractionsBase(cx, actionsBase);
#endif
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_7
	auto actions = JS_DefineObject(cx, robj, "actions", &actions_class, ractionsBase, JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE);
#else
	auto actions = JS_DefineObject(cx, robj, "actions", &actions_class, JS::NullPtr(), JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE);
#endif
	CCASSERT(actions, "actions should not be nullptr");
	JS::RootedObject ractions(cx, actions);
// #AGTK-NX
#ifndef USE_SAR_OPTIMIZE_7
	static JSFunctionSpec actions_methods[] = {
		{ "get", js_actions_get, 0, 0, 0 },
		{ "getIdByName", js_actions_getIdByName, 0, 0, 0 },
		{ "getIdList", js_actions_getIdList, 0, 0, 0 },
		{ NULL },
	};
	JS_DefineFunctions(cx, ractions, actions_methods);

	JS::RootedValue rval(cx);
	rval.setObject(*actions);
	JS_SetProperty(cx, robj, "actions", rval);
#endif
	{
		JS::RootedValue v(cx);
		JS_GetProperty(cx, robj, "id", &v);
		if (!v.isNumber()) {
			return false;
		}
		JS_DefineProperty(cx, ractions, "objectId", v, JSPROP_ENUMERATE | JSPROP_PERMANENT);
	}
	return true;
}

bool js_actions_get(JSContext *cx, unsigned argc, JS::Value *vp)
{
	JS::CallArgs args = CallArgsFromVp(argc, vp);
	args.rval().set(JS::NullValue());
	auto len = args.length();
	if (len < 1) {
		return false;
	}
	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
	auto &v = args[0];
#else
#endif
	if (!v.isNumber()) {
		return false;
	}
	auto actionId = JavascriptManager::getInt32(v);
	int objectId = -1;
	{
		JS::RootedValue v(cx);
		JS_GetProperty(cx, jsthis, "objectId", &v);
		if (!v.isNumber()) {
			return false;
		}
		objectId = JavascriptManager::getInt32(v);
	}
	auto projectData = GameManager::getInstance()->getProjectData();
	auto objectData = projectData->getObjectData(objectId);
	if (!objectData) {
		return true;
	}
	auto actionData = objectData->getActionData(actionId);
	if (!actionData) {
		return true;
	}
	static JSClass action_class = {
		"action",
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
	auto action = JS_DefineObject(cx, jsthis, "action", &action_class, JS::NullPtr(), JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE);
	CCASSERT(action, "action should not be nullptr");
	JS::RootedObject raction(cx, action);
#if 0
	static JSFunctionSpec action_methods[] = {
		//{ "getValue", js_action_getValue, 0, 0, 0 },
		{ NULL },
	};
	JS_DefineFunctions(cx, raction, action_methods);
#endif
	JS::RootedValue ractionId(cx, JS::Int32Value(actionId));
	JS_DefineProperty(cx, raction, "id", ractionId, JSPROP_ENUMERATE | JSPROP_PERMANENT);
	JS::RootedValue ractionName(cx, std_string_to_jsval(cx, actionData->getName()));
	JS_DefineProperty(cx, raction, "name", ractionName, JSPROP_ENUMERATE | JSPROP_PERMANENT);
	JS::RootedValue rval(cx);
	rval.setInt32(actionId);
	JS_SetProperty(cx, raction, "actionId", rval);
	args.rval().set(OBJECT_TO_JSVAL(action));
	return true;
}

bool js_actions_getIdList(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//ret: <array : 弾IDの配列>
	JS::CallArgs args = CallArgsFromVp(argc, vp);

	std::list<int> idList;
	do {
		JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
		int objectId = -1;
		{
			JS::RootedValue v(cx);
			JS_GetProperty(cx, jsthis, "objectId", &v);
			if (!v.isNumber()) {
				break;
			}
			objectId = JavascriptManager::getInt32(v);
		}
		auto projectData = GameManager::getInstance()->getProjectData();
		auto objectData = projectData->getObjectData(objectId);
		if (objectData) {
			auto actionList = objectData->getActionList();
			cocos2d::DictElement *el = nullptr;
			CCDICT_FOREACH(actionList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto p = static_cast<agtk::data::ObjectActionData *>(el->getObject());
#else
				auto p = dynamic_cast<agtk::data::ObjectActionData *>(el->getObject());
#endif
				idList.push_back(p->getId());
			}
		}
	} while (0);

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

bool js_actions_getIdByName(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//arg1: <string: アクション名>
	//ret: <int : アクションID>、見つからなければ-1。
	JS::CallArgs args = CallArgsFromVp(argc, vp);
	args.rval().set(JS::Int32Value(-1));
	auto len = args.length();
	if (len < 1) {
		return false;
	}
	std::string actionName;
	{
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
		auto &v2 = args[0];
#else
#endif
		if (!v2.isString()) {
			return false;
		}
		if (!jsval_to_std_string(cx, v2, &actionName)) {
			return false;
		}
	}

	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
	int objectId = -1;
	{
		JS::RootedValue v(cx);
		JS_GetProperty(cx, jsthis, "objectId", &v);
		if (!v.isNumber()) {
			return false;
		}
		objectId = JavascriptManager::getInt32(v);
	}
	auto projectData = GameManager::getInstance()->getProjectData();
	auto objectData = projectData->getObjectData(objectId);
	if (!objectData) {
		return true;
	}
	auto actionList = objectData->getActionList();
	cocos2d::DictElement *el = nullptr;
	CCDICT_FOREACH(actionList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto p = static_cast<agtk::data::ObjectActionData *>(el->getObject());
#else
		auto p = dynamic_cast<agtk::data::ObjectActionData *>(el->getObject());
#endif
		if (strcmp(p->getName(), actionName.c_str()) == 0) {
			args.rval().set(JS::Int32Value(p->getId()));
			return true;
		}
	}

	return true;
}

// action

// viewports
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_7
bool jsb_register_agtk_object_viewportsBase(JSContext *cx, JSObject *agtkObj)
{
	JS::RootedObject robj(cx, agtkObj);

	static JSClass viewports_class = {
		"viewportsBase",
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
	auto viewports = JS_DefineObject(cx, robj, "viewportsBase", &viewports_class, JS::NullPtr(), JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE);
	CCASSERT(viewports, "viewportsBase should not be nullptr");
	JS::RootedObject rviewports(cx, viewports);
	static JSFunctionSpec viewports_methods[] = {
		{ "get", js_viewports_get, 0, 0, 0 },
		{ "getIdByName", js_viewports_getIdByName, 0, 0, 0 },
		{ "getIdList", js_viewports_getIdList, 0, 0, 0 },
		{ NULL },
	};
	JS_DefineFunctions(cx, rviewports, viewports_methods);

	return true;
}
#endif

bool jsb_register_agtk_object_viewports(JSContext *cx, JSObject *agtkObj)
{
	JS::RootedObject robj(cx, agtkObj);

	static JSClass viewports_class = {
		"viewports",
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
	JS::RootedValue rviewportsBaseVal(cx);
	JS_GetProperty(cx, jsObj, "viewportsBase", &rviewportsBaseVal);
	if (!rviewportsBaseVal.isObject()) {
		return false;
	}

	JSObject* viewportsBase = rviewportsBaseVal.get().toObjectOrNull();
	JS::RootedObject rviewportsBase(cx, viewportsBase);
#endif
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_7
	auto viewports = JS_DefineObject(cx, robj, "viewports", &viewports_class, rviewportsBase, JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE);
#else
	auto viewports = JS_DefineObject(cx, robj, "viewports", &viewports_class, JS::NullPtr(), JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE);
#endif
	CCASSERT(viewports, "viewports should not be nullptr");
	JS::RootedObject rviewports(cx, viewports);
// #AGTK-NX
#ifndef USE_SAR_OPTIMIZE_7
	static JSFunctionSpec viewports_methods[] = {
		{ "get", js_viewports_get, 0, 0, 0 },
		{ "getIdByName", js_viewports_getIdByName, 0, 0, 0 },
		{ "getIdList", js_viewports_getIdList, 0, 0, 0 },
		{ NULL },
	};
	JS_DefineFunctions(cx, rviewports, viewports_methods);

	JS::RootedValue rval(cx);
	rval.setObject(*viewports);
	JS_SetProperty(cx, robj, "viewports", rval);
#endif
	{
		JS::RootedValue v(cx);
		JS_GetProperty(cx, robj, "id", &v);
		if (!v.isNumber()) {
			return false;
		}
		JS_DefineProperty(cx, rviewports, "objectId", v, JSPROP_ENUMERATE | JSPROP_PERMANENT);
	}
	return true;
}

bool js_viewports_get(JSContext *cx, unsigned argc, JS::Value *vp)
{
	JS::CallArgs args = CallArgsFromVp(argc, vp);
    args.rval().set(JS::NullValue());
	auto len = args.length();
	if (len < 1) {
		return false;
	}
	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
	auto &v = args[0];
#else
#endif
	if (!v.isNumber()) {
		return false;
	}
	auto viewportId = JavascriptManager::getInt32(v);
	int objectId = -1;
	{
		JS::RootedValue v(cx);
		JS_GetProperty(cx, jsthis, "objectId", &v);
		if (!v.isNumber()) {
			return false;
		}
		objectId = JavascriptManager::getInt32(v);
	}
	auto projectData = GameManager::getInstance()->getProjectData();
	auto objectData = projectData->getObjectData(objectId);
	if (!objectData) {
		return true;
	}
	if (!objectData->getViewportLightSettingFlag()) {
		return true;
	}
	auto viewportSettingData = objectData->getViewportLightSettingData(viewportId);
	if (!viewportSettingData) {
		return true;
	}
	static JSClass viewport_class = {
		"viewport",
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
	auto viewport = JS_DefineObject(cx, jsthis, "viewport", &viewport_class, JS::NullPtr(), JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE);
	CCASSERT(viewport, "viewport should not be nullptr");
	JS::RootedObject rviewport(cx, viewport);
#if 0
	static JSFunctionSpec viewport_methods[] = {
		//{ "getValue", js_switch_getValue, 0, 0, 0 },
		{ NULL },
	};
	JS_DefineFunctions(cx, rviewport, viewport_methods);
#endif
	JS::RootedValue rviewportId(cx, JS::Int32Value(viewportId));
	JS_DefineProperty(cx, rviewport, "id", rviewportId, JSPROP_ENUMERATE | JSPROP_PERMANENT);
	JS::RootedValue rviewportName(cx, std_string_to_jsval(cx, viewportSettingData->getName()));
	JS_DefineProperty(cx, rviewport, "name", rviewportName, JSPROP_ENUMERATE | JSPROP_PERMANENT);
	JS::RootedValue rval(cx);
	rval.setInt32(viewportId);
	JS_SetProperty(cx, rviewport, "viewportId", rval);
	args.rval().set(OBJECT_TO_JSVAL(viewport));
	return true;
}

bool js_viewports_getIdList(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//ret: <array : 弾IDの配列>
	JS::CallArgs args = CallArgsFromVp(argc, vp);

	std::list<int> idList;
    do {
        JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
        int objectId = -1;
        {
            JS::RootedValue v(cx);
            JS_GetProperty(cx, jsthis, "objectId", &v);
            if (!v.isNumber()) {
                break;
            }
            objectId = JavascriptManager::getInt32(v);
        }
        auto projectData = GameManager::getInstance()->getProjectData();
        auto objectData = projectData->getObjectData(objectId);
        if (objectData) {
            if (objectData->getViewportLightSettingFlag()) {
                auto viewportSettingList = objectData->getViewportLightSettingList();
                cocos2d::DictElement *el = nullptr;
                CCDICT_FOREACH(viewportSettingList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
					auto p = static_cast<agtk::data::ObjectViewportLightSettingData *>(el->getObject());
#else
					auto p = dynamic_cast<agtk::data::ObjectViewportLightSettingData *>(el->getObject());
#endif
                    idList.push_back(p->getId());
                }
            }
        }
    } while(0);

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

bool js_viewports_getIdByName(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//arg1: <string: 弾名>
	//ret: <int : 弾ID>、見つからなければ-1。
	JS::CallArgs args = CallArgsFromVp(argc, vp);
    args.rval().set(JS::Int32Value(-1));
	auto len = args.length();
	if (len < 1) {
		return false;
	}
	std::string viewportName;
	{
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
		auto &v2 = args[0];
#else
#endif
		if (!v2.isString()) {
			return false;
		}
		if (!jsval_to_std_string(cx, v2, &viewportName)) {
			return false;
		}
	}

	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
	int objectId = -1;
	{
		JS::RootedValue v(cx);
		JS_GetProperty(cx, jsthis, "objectId", &v);
		if (!v.isNumber()) {
			return false;
		}
		objectId = JavascriptManager::getInt32(v);
	}
	auto projectData = GameManager::getInstance()->getProjectData();
	auto objectData = projectData->getObjectData(objectId);
	if (!objectData) {
		return true;
	}
	if (!objectData->getViewportLightSettingFlag()) {
		return true;
	}
	auto viewportSettingList = objectData->getViewportLightSettingList();
	cocos2d::DictElement *el = nullptr;
	CCDICT_FOREACH(viewportSettingList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto p = static_cast<agtk::data::ObjectViewportLightSettingData *>(el->getObject());
#else
		auto p = dynamic_cast<agtk::data::ObjectViewportLightSettingData *>(el->getObject());
#endif
		if (strcmp(p->getName(), viewportName.c_str()) == 0) {
			args.rval().set(JS::Int32Value(p->getId()));
			return true;
		}
	}

	return true;
}

// viewport

// switches
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_7
bool jsb_register_agtk_object_switchesBase(JSContext *cx, JSObject *agtkObj)
{
	JS::RootedObject robj(cx, agtkObj);

	static JSClass switches_class = {
		"switchesBase",
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
	auto switches = JS_DefineObject(cx, robj, "objectSwitchesBase", &switches_class, JS::NullPtr(), JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE);
	CCASSERT(switches, "objectSwitchesBase should not be nullptr");
	JS::RootedObject rswitches(cx, switches);
	static JSFunctionSpec switches_methods[] = {
		{ "get", js_switches_get, 0, 0, 0 },
		{ "getIdByName", js_switches_getIdByName, 0, 0, 0 },
		{ "getIdList", js_switches_getIdList, 0, 0, 0 },
		{ NULL },
	};
	JS_DefineFunctions(cx, rswitches, switches_methods);
	return true;
}
#endif

bool jsb_register_agtk_object_switches(JSContext *cx, JSObject *agtkObj)
{
	JS::RootedObject robj(cx, agtkObj);

	static JSClass switches_class = {
		"switches",
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
	JS::RootedValue rswitchesBaseVal(cx);
	JS_GetProperty(cx, jsObj, "objectSwitchesBase", &rswitchesBaseVal);
	if (!rswitchesBaseVal.isObject()) {
		return false;
	}

	JSObject* switchesBase = rswitchesBaseVal.get().toObjectOrNull();
	JS::RootedObject rswitchesBase(cx, switchesBase);
#endif
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_7
	auto switches = JS_DefineObject(cx, robj, "switches", &switches_class, rswitchesBase, JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE);
#else
	auto switches = JS_DefineObject(cx, robj, "switches", &switches_class, JS::NullPtr(), JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE);
#endif
	CCASSERT(switches, "switches should not be nullptr");
	JS::RootedObject rswitches(cx, switches);
// #AGTK-NX
#ifndef USE_SAR_OPTIMIZE_7
	static JSFunctionSpec switches_methods[] = {
		{ "get", js_switches_get, 0, 0, 0 },
		{ "getIdByName", js_switches_getIdByName, 0, 0, 0 },
		{ "getIdList", js_switches_getIdList, 0, 0, 0 },
		{ NULL },
	};
	JS_DefineFunctions(cx, rswitches, switches_methods);

	JS::RootedValue rval(cx);
	rval.setObject(*switches);
	JS_SetProperty(cx, robj, "switches", rval);
#endif
	{
		JS::RootedValue v(cx);
		JS_GetProperty(cx, robj, "id", &v);
		if (!v.isNumber()) {
			return false;
		}
		JS_DefineProperty(cx, rswitches, "objectId", v, JSPROP_ENUMERATE | JSPROP_PERMANENT);
	}
	return true;
}

bool js_switches_get(JSContext *cx, unsigned argc, JS::Value *vp)
{
	JS::CallArgs args = CallArgsFromVp(argc, vp);
    args.rval().set(JS::NullValue());
	auto len = args.length();
	if (len < 1) {
		return false;
	}
	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
	auto &v = args[0];
#else
#endif
	if (!v.isNumber()) {
		return false;
	}
	auto switchId = JavascriptManager::getInt32(v);
	int objectId = -1;
	{
		JS::RootedValue v(cx);
		JS_GetProperty(cx, jsthis, "objectId", &v);
		if (!v.isNumber()) {
			return false;
		}
		objectId = JavascriptManager::getInt32(v);
	}
	auto projectData = GameManager::getInstance()->getProjectData();
	auto objectData = projectData->getObjectData(objectId);
	if (!objectData) {
		return true;
	}
	if (!objectData->getSwitchList()) {
		return true;
	}
	auto switchData = objectData->getSwitchData(switchId);
	if (!switchData) {
		return true;
	}
	static JSClass switch_class = {
		"switch",
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
	auto swtch = JS_DefineObject(cx, jsthis, "switch", &switch_class, JS::NullPtr(), JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE);
	CCASSERT(swtch, "switch should not be nullptr");
	JS::RootedObject rswitch(cx, swtch);
#if 0
	static JSFunctionSpec switch_methods[] = {
		//{ "getValue", js_switch_getValue, 0, 0, 0 },
		{ NULL },
	};
	JS_DefineFunctions(cx, rswitch, switch_methods);
#endif
	JS::RootedValue rswitchId(cx, JS::Int32Value(switchId));
	JS_DefineProperty(cx, rswitch, "id", rswitchId, JSPROP_ENUMERATE | JSPROP_PERMANENT);
	JS::RootedValue rswitchName(cx, std_string_to_jsval(cx, switchData->getName()));
	JS_DefineProperty(cx, rswitch, "name", rswitchName, JSPROP_ENUMERATE | JSPROP_PERMANENT);
	JS::RootedValue rval(cx);
	rval.setInt32(switchId);
	JS_SetProperty(cx, rswitch, "switchId", rval);
	args.rval().set(OBJECT_TO_JSVAL(swtch));
	return true;
}

bool js_switches_getIdList(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//ret: <array : 弾IDの配列>
	JS::CallArgs args = CallArgsFromVp(argc, vp);

	std::list<int> idList;
    do {
        JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
        int objectId = -1;
        {
            JS::RootedValue v(cx);
            JS_GetProperty(cx, jsthis, "objectId", &v);
            if (!v.isNumber()) {
                break;
            }
            objectId = JavascriptManager::getInt32(v);
        }
        auto projectData = GameManager::getInstance()->getProjectData();
        auto objectData = projectData->getObjectData(objectId);
        if (objectData) {
            auto switchList = objectData->getSwitchArray();
            cocos2d::Ref *ref = nullptr;
            CCARRAY_FOREACH(switchList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto p = static_cast<agtk::data::SwitchData *>(ref);
#else
				auto p = dynamic_cast<agtk::data::SwitchData *>(ref);
#endif
                idList.push_back(p->getId());
            }
        }
    } while(0);

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

bool js_switches_getIdByName(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//arg1: <string: 弾名>
	//ret: <int : 弾ID>、見つからなければ-1。
	JS::CallArgs args = CallArgsFromVp(argc, vp);
    args.rval().set(JS::Int32Value(-1));
	auto len = args.length();
	if (len < 1) {
		return false;
	}
	std::string switchName;
	{
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
		auto &v2 = args[0];
#else
#endif
		if (!v2.isString()) {
			return false;
		}
		if (!jsval_to_std_string(cx, v2, &switchName)) {
			return false;
		}
	}

	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
	int objectId = -1;
	{
		JS::RootedValue v(cx);
		JS_GetProperty(cx, jsthis, "objectId", &v);
		if (!v.isNumber()) {
			return false;
		}
		objectId = JavascriptManager::getInt32(v);
	}
	auto projectData = GameManager::getInstance()->getProjectData();
	auto objectData = projectData->getObjectData(objectId);
	if (!objectData) {
		return true;
	}
	auto switchData = objectData->getSwitchDataByName(switchName.c_str());
	if (!switchData) {
		return true;
	}
	args.rval().set(JS::Int32Value(switchData->getId()));
	return true;
}

// switch

// variables
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_7
bool jsb_register_agtk_object_variablesBase(JSContext *cx, JSObject *agtkObj)
{
	JS::RootedObject robj(cx, agtkObj);

	static JSClass variables_class = {
		"variablesBase",
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
	auto variables = JS_DefineObject(cx, robj, "objectVariablesBase", &variables_class, JS::NullPtr(), JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE);
	CCASSERT(variables, "objectVariablesBase should not be nullptr");
	JS::RootedObject rvariables(cx, variables);
	static JSFunctionSpec variables_methods[] = {
		{ "get", js_variables_get, 0, 0, 0 },
		{ "getIdByName", js_variables_getIdByName, 0, 0, 0 },
		{ "getIdList", js_variables_getIdList, 0, 0, 0 },
		{ NULL },
	};
	JS_DefineFunctions(cx, rvariables, variables_methods);
	return true;
}
#endif

bool jsb_register_agtk_object_variables(JSContext *cx, JSObject *agtkObj)
{
	JS::RootedObject robj(cx, agtkObj);

	static JSClass variables_class = {
		"variables",
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
	JS::RootedValue rvariablesBaseVal(cx);
	JS_GetProperty(cx, jsObj, "objectVariablesBase", &rvariablesBaseVal);
	if (!rvariablesBaseVal.isObject()) {
		return false;
	}

	JSObject* variablesBase = rvariablesBaseVal.get().toObjectOrNull();
	JS::RootedObject rvariablesBase(cx, variablesBase);
#endif
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_7
	auto variables = JS_DefineObject(cx, robj, "variables", &variables_class, rvariablesBase, JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE);
#else
	auto variables = JS_DefineObject(cx, robj, "variables", &variables_class, JS::NullPtr(), JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE);
#endif
	CCASSERT(variables, "variables should not be nullptr");
	JS::RootedObject rvariables(cx, variables);
// #AGTK-NX
#ifndef USE_SAR_OPTIMIZE_7
	static JSFunctionSpec variables_methods[] = {
		{ "get", js_variables_get, 0, 0, 0 },
		{ "getIdByName", js_variables_getIdByName, 0, 0, 0 },
		{ "getIdList", js_variables_getIdList, 0, 0, 0 },
		{ NULL },
	};
	JS_DefineFunctions(cx, rvariables, variables_methods);

	JS::RootedValue rval(cx);
	rval.setObject(*variables);
	JS_SetProperty(cx, robj, "variables", rval);
#endif
	{
		JS::RootedValue v(cx);
		JS_GetProperty(cx, robj, "id", &v);
		if (!v.isNumber()) {
			return false;
		}
		JS_DefineProperty(cx, rvariables, "objectId", v, JSPROP_ENUMERATE | JSPROP_PERMANENT);
	}
	return true;
}

bool js_variables_get(JSContext *cx, unsigned argc, JS::Value *vp)
{
	JS::CallArgs args = CallArgsFromVp(argc, vp);
    args.rval().set(JS::NullValue());
	auto len = args.length();
	if (len < 1) {
		return false;
	}
	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
	auto &v = args[0];
#else
#endif
	if (!v.isNumber()) {
		return false;
	}
	auto variableId = JavascriptManager::getInt32(v);
	int objectId = -1;
	{
		JS::RootedValue v(cx);
		JS_GetProperty(cx, jsthis, "objectId", &v);
		if (!v.isNumber()) {
			return false;
		}
		objectId = JavascriptManager::getInt32(v);
	}
	auto projectData = GameManager::getInstance()->getProjectData();
	auto objectData = projectData->getObjectData(objectId);
	if (!objectData) {
		return true;
	}
	auto variableData = objectData->getVariableData(variableId);
	if (!variableData) {
		return true;
	}
	static JSClass variable_class = {
		"variable",
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
	auto variable = JS_DefineObject(cx, jsthis, "variable", &variable_class, JS::NullPtr(), JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE);
	CCASSERT(variable, "variable should not be nullptr");
	JS::RootedObject rvariable(cx, variable);
#if 0
	static JSFunctionSpec variable_methods[] = {
		//{ "getValue", js_switch_getValue, 0, 0, 0 },
		{ NULL },
	};
	JS_DefineFunctions(cx, rvariable, variable_methods);
#endif
	JS::RootedValue rvariableId(cx, JS::Int32Value(variableId));
	JS_DefineProperty(cx, rvariable, "id", rvariableId, JSPROP_ENUMERATE | JSPROP_PERMANENT);
	JS::RootedValue rvariableName(cx, std_string_to_jsval(cx, variableData->getName()));
	JS_DefineProperty(cx, rvariable, "name", rvariableName, JSPROP_ENUMERATE | JSPROP_PERMANENT);
	JS::RootedValue rval(cx);
	rval.setInt32(variableId);
	JS_SetProperty(cx, rvariable, "variableId", rval);
	args.rval().set(OBJECT_TO_JSVAL(variable));
	return true;
}

bool js_variables_getIdList(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//ret: <array : 弾IDの配列>
	JS::CallArgs args = CallArgsFromVp(argc, vp);

	std::list<int> idList;
    do {
        JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
        int objectId = -1;
        {
            JS::RootedValue v(cx);
            JS_GetProperty(cx, jsthis, "objectId", &v);
            if (!v.isNumber()) {
                break;
            }
            objectId = JavascriptManager::getInt32(v);
        }
        auto projectData = GameManager::getInstance()->getProjectData();
        auto objectData = projectData->getObjectData(objectId);
        if (objectData) {
            auto variableList = objectData->getVariableArray();
            cocos2d::Ref *ref = nullptr;
            CCARRAY_FOREACH(variableList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto p = static_cast<agtk::data::VariableData *>(ref);
#else
				auto p = dynamic_cast<agtk::data::VariableData *>(ref);
#endif
                idList.push_back(p->getId());
            }
        }
    } while(0);

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

bool js_variables_getIdByName(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//arg1: <string: 弾名>
	//ret: <int : 弾ID>、見つからなければ-1。
	JS::CallArgs args = CallArgsFromVp(argc, vp);
    args.rval().set(JS::Int32Value(-1));
	auto len = args.length();
	if (len < 1) {
		return false;
	}
	std::string variableName;
	{
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
		auto &v2 = args[0];
#else
#endif
		if (!v2.isString()) {
			return false;
		}
		if (!jsval_to_std_string(cx, v2, &variableName)) {
			return false;
		}
	}

	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
	int objectId = -1;
	{
		JS::RootedValue v(cx);
		JS_GetProperty(cx, jsthis, "objectId", &v);
		if (!v.isNumber()) {
			return false;
		}
		objectId = JavascriptManager::getInt32(v);
	}
	auto projectData = GameManager::getInstance()->getProjectData();
	auto objectData = projectData->getObjectData(objectId);
	if (!objectData) {
		return true;
	}
	auto variableData = objectData->getVariableDataByName(variableName.c_str());
	if (!variableData) {
		return true;
	}
	args.rval().set(JS::Int32Value(variableData->getId()));
	return true;
}

// variable

#if 0
bool js_object_getValue(JSContext *cx, unsigned argc, JS::Value *vp)
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
