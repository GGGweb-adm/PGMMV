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

static bool register_agtk_animations(JSContext *_cx,JS::HandleObject object);

//animations
static bool js_animations_get(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_animations_getIdByName(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_animations_getIdList(JSContext *cx, unsigned argc, JS::Value *vp);
//animation

//animation
static bool js_animation_getResourceSetIdList(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_animation_getResourceSetNameList(JSContext *cx, unsigned argc, JS::Value *vp);
//animation

//motions
static bool jsb_register_agtk_animation_motions(JSContext *_cx, JSObject *agtkObj);
static bool js_motions_get(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_motions_getIdByName(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_motions_getIdList(JSContext *cx, unsigned argc, JS::Value *vp);
//motion

//directions
static bool jsb_register_agtk_animation_motion_directions(JSContext *_cx, JSObject *agtkObj);
static bool js_directions_get(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_directions_getIdByName(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_directions_getIdList(JSContext *cx, unsigned argc, JS::Value *vp);
//direction

//tracks
static bool jsb_register_agtk_animation_motion_direction_tracks(JSContext *_cx, JSObject *agtkObj);
static bool js_tracks_get(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_tracks_getIdByName(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_tracks_getIdList(JSContext *cx, unsigned argc, JS::Value *vp);
//track

#include "Lib/ObjectCommand.h"
#include "Data/ObjectCommandData.h"
#include "Manager/DebugManager.h"
#include "Data/PlayData.h"
#include "JavascriptManager.h"

void jsb_register_agtk_animations(JSContext *cx, JS::HandleObject object)
{
	auto ret = register_agtk_animations(cx, object);
	CCASSERT(ret, "Failed to register_agtk_animations");
}

bool register_agtk_animations(JSContext *cx, JS::HandleObject object)
{
	JS::RootedObject robj(cx, object);

	static JSClass animations_class = {
		"animations",
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
	auto animations = JS_DefineObject(cx, robj, "animations", &animations_class, JS::NullPtr(), JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE);
	CCASSERT(animations, "animations should not be nullptr");
	JS::RootedObject ranimations(cx, animations);
	static JSFunctionSpec animations_methods[] = {
		{ "get", js_animations_get, 0, 0, 0 },
		{ "getIdByName", js_animations_getIdByName, 0, 0, 0 },
		{ "getIdList", js_animations_getIdList, 0, 0, 0 },
		{ NULL },
	};
	JS_DefineFunctions(cx, ranimations, animations_methods);

	JS::RootedValue rval(cx);
	rval.setObject(*animations);
	JS_SetProperty(cx, robj, "animations", rval);
	return true;
}

// animations
bool js_animations_get(JSContext *cx, unsigned argc, JS::Value *vp)
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
	auto animationId = JavascriptManager::getInt32(v);
	auto projectData = GameManager::getInstance()->getProjectData();
	auto animationData = projectData->getAnimationData(animationId);
	if (!animationData) {
		return true;
	}
	static JSClass animation_class = {
		"animation",
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
	auto animation = JS_DefineObject(cx, jsthis, "animation", &animation_class, JS::NullPtr(), JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE);
	CCASSERT(animation, "animation should not be nullptr");
	JS::RootedObject ranimation(cx, animation);
	static JSFunctionSpec animation_methods[] = {
		{ "getResourceSetIdList", js_animation_getResourceSetIdList, 0, 0, 0 },
		{ "getResourceSetNameList", js_animation_getResourceSetNameList, 0, 0, 0 },
		{ NULL },
	};
	JS_DefineFunctions(cx, ranimation, animation_methods);
	JS::RootedValue ranimationId(cx, JS::Int32Value(animationId));
	JS_DefineProperty(cx, ranimation, "id", ranimationId, JSPROP_ENUMERATE | JSPROP_PERMANENT);
	JS::RootedValue ranimationName(cx, std_string_to_jsval(cx, animationData->getName()));
	JS_DefineProperty(cx, ranimation, "name", ranimationName, JSPROP_ENUMERATE | JSPROP_PERMANENT);
	JS::RootedValue rtype(cx, JS::Int32Value(animationData->getType()));
	JS_DefineProperty(cx, ranimation, "type", rtype, JSPROP_ENUMERATE | JSPROP_PERMANENT);
	JS::RootedValue rval(cx);
	rval.setInt32(animationId);
	JS_SetProperty(cx, ranimation, "animationId", rval);
    if(animationData->getType() == agtk::data::AnimationData::kMotion){
        if (!jsb_register_agtk_animation_motions(cx, animation)) return false;
    }
	args.rval().set(OBJECT_TO_JSVAL(animation));
	return true;
}

bool js_animations_getIdByName(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//arg1: <string: オブジェクトインスタンス名>
	//ret: <int : オブジェクトインスタンスID>
	JS::CallArgs args = CallArgsFromVp(argc, vp);
    args.rval().set(JS::Int32Value(-1));
	auto len = args.length();
	if (len < 1) {
		return false;
	}
	std::string animationName;
	{
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
		auto &v2 = args[0];
#else
#endif
		if (!v2.isString()) {
			return false;
		}
		if (!jsval_to_std_string(cx, v2, &animationName)) {
			return false;
		}
	}

	int value = -1;
	auto projectData = GameManager::getInstance()->getProjectData();
	auto animationData = projectData->getAnimationDataByName(animationName.c_str());

	if(animationData){
		value = animationData->getId();
	}
	args.rval().set(JS::Int32Value(value));
	return true;
}

bool js_animations_getIdList(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//ret: <array : 弾IDの配列>
	JS::CallArgs args = CallArgsFromVp(argc, vp);

	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
	std::list<int> idList;
    do {
        auto projectData = GameManager::getInstance()->getProjectData();
        auto animationList = projectData->getAnimationList();
        cocos2d::DictElement *el = nullptr;
        CCDICT_FOREACH(animationList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto p = static_cast<agtk::data::AnimationData *>(el->getObject());
#else
			auto p = dynamic_cast<agtk::data::AnimationData *>(el->getObject());
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
// animations

// animation
bool js_animation_getResourceSetIdList(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//ret: <array : 弾IDの配列>
	JS::CallArgs args = CallArgsFromVp(argc, vp);

	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
	std::list<int> idList;
	do {
		int animationId = -1;
		{
			JS::RootedValue v(cx);
			JS_GetProperty(cx, jsthis, "animationId", &v);
			if (!v.isNumber()) {
				break;
			}
			animationId = JavascriptManager::getInt32(v);
		}
		auto projectData = GameManager::getInstance()->getProjectData();
		auto animationData = projectData->getAnimationData(animationId);
		if (animationData && animationData->getType() == agtk::data::AnimationData::kMotion) {
			for (auto id : animationData->getResourceSetIdList()) {
				idList.push_back(id);
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

bool js_animation_getResourceSetNameList(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//ret: <array : 弾IDの配列>
	JS::CallArgs args = CallArgsFromVp(argc, vp);

	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
	std::list<string> nameList;
	do {
		int animationId = -1;
		{
			JS::RootedValue v(cx);
			JS_GetProperty(cx, jsthis, "animationId", &v);
			if (!v.isNumber()) {
				break;
			}
			animationId = JavascriptManager::getInt32(v);
		}
		auto projectData = GameManager::getInstance()->getProjectData();
		auto animationData = projectData->getAnimationData(animationId);
		if (animationData && animationData->getType() == agtk::data::AnimationData::kMotion) {
			for (auto name : animationData->getResourceSetNameList()) {
				nameList.push_back(name);
			}
		}
	} while (0);

	JS::RootedObject jsarr(cx, JS_NewArrayObject(cx, nameList.size()));
	uint32_t i = 0;
	for (auto name : nameList) {
		JS::RootedValue element(cx, std_string_to_jsval(cx, name));
		JS_SetElement(cx, jsarr, i, element);
		i++;
	}

	args.rval().set(OBJECT_TO_JSVAL(jsarr));
	return true;
}

// animation

// motions
bool jsb_register_agtk_animation_motions(JSContext *cx, JSObject *agtkObj)
{
	JS::RootedObject robj(cx, agtkObj);

	static JSClass motions_class = {
		"motions",
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
	auto motions = JS_DefineObject(cx, robj, "motions", &motions_class, JS::NullPtr(), JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE);
	CCASSERT(motions, "motions should not be nullptr");
	JS::RootedObject rmotions(cx, motions);
	static JSFunctionSpec motions_methods[] = {
		{ "get", js_motions_get, 0, 0, 0 },
		{ "getIdByName", js_motions_getIdByName, 0, 0, 0 },
		{ "getIdList", js_motions_getIdList, 0, 0, 0 },
		{ NULL },
	};
	JS_DefineFunctions(cx, rmotions, motions_methods);

	JS::RootedValue rval(cx);
	rval.setObject(*motions);
	JS_SetProperty(cx, robj, "motions", rval);
	{
		JS::RootedValue v(cx);
		JS_GetProperty(cx, robj, "id", &v);
		if (!v.isNumber()) {
			return false;
		}
		JS_DefineProperty(cx, rmotions, "animationId", v, JSPROP_ENUMERATE | JSPROP_PERMANENT);
	}
	return true;
}

bool js_motions_get(JSContext *cx, unsigned argc, JS::Value *vp)
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
	auto motionId = JavascriptManager::getInt32(v);
	int animationId = -1;
	{
		JS::RootedValue v(cx);
		JS_GetProperty(cx, jsthis, "animationId", &v);
		if (!v.isNumber()) {
			return false;
		}
		animationId = JavascriptManager::getInt32(v);
	}
	auto projectData = GameManager::getInstance()->getProjectData();
	auto animationData = projectData->getAnimationData(animationId);
	if (!animationData) {
		return true;
	}
	auto motionData = animationData->getMotionData(motionId);
	if (!motionData) {
		return true;
	}
	static JSClass motion_class = {
		"motion",
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
	auto motion = JS_DefineObject(cx, jsthis, "motion", &motion_class, JS::NullPtr(), JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE);
	CCASSERT(motion, "motion should not be nullptr");
	JS::RootedObject rmotion(cx, motion);
#if 0
	static JSFunctionSpec motion_methods[] = {
		//{ "getValue", js_switch_getValue, 0, 0, 0 },
		{ NULL },
	};
	JS_DefineFunctions(cx, rmotion, motion_methods);
#endif
	JS::RootedValue ranimationId(cx, JS::Int32Value(animationId));
	JS_DefineProperty(cx, rmotion, "animationId", ranimationId, JSPROP_ENUMERATE | JSPROP_PERMANENT);
	JS::RootedValue rmotionId(cx, JS::Int32Value(motionId));
	JS_DefineProperty(cx, rmotion, "id", rmotionId, JSPROP_ENUMERATE | JSPROP_PERMANENT);
	JS::RootedValue rmotionName(cx, std_string_to_jsval(cx, motionData->getName()));
	JS_DefineProperty(cx, rmotion, "name", rmotionName, JSPROP_ENUMERATE | JSPROP_PERMANENT);
	JS::RootedValue rval(cx);
	rval.setInt32(motionId);
	JS_SetProperty(cx, rmotion, "motionId", rval);
	if (!jsb_register_agtk_animation_motion_directions(cx, motion)) return false;
	args.rval().set(OBJECT_TO_JSVAL(motion));
	return true;
}

bool js_motions_getIdList(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//ret: <array : 弾IDの配列>
	JS::CallArgs args = CallArgsFromVp(argc, vp);

	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
	std::list<int> idList;
    do {
        int animationId = -1;
        {
            JS::RootedValue v(cx);
            JS_GetProperty(cx, jsthis, "animationId", &v);
            if (!v.isNumber()) {
                break;
            }
            animationId = JavascriptManager::getInt32(v);
        }
        auto projectData = GameManager::getInstance()->getProjectData();
        auto animationData = projectData->getAnimationData(animationId);
        if (animationData) {
            auto motionList = animationData->getMotionList();
            cocos2d::DictElement *el = nullptr;
            CCDICT_FOREACH(motionList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto p = static_cast<agtk::data::MotionData *>(el->getObject());
#else
				auto p = dynamic_cast<agtk::data::MotionData *>(el->getObject());
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

bool js_motions_getIdByName(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//arg1: <string: 弾名>
	//ret: <int : 弾ID>、見つからなければ-1。
	JS::CallArgs args = CallArgsFromVp(argc, vp);
    args.rval().set(JS::Int32Value(-1));
	auto len = args.length();
	if (len < 1) {
		return false;
	}
	std::string motionName;
	{
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
		auto &v2 = args[0];
#else
#endif
		if (!v2.isString()) {
			return false;
		}
		if (!jsval_to_std_string(cx, v2, &motionName)) {
			return false;
		}
	}

	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
	int animationId = -1;
	{
		JS::RootedValue v(cx);
		JS_GetProperty(cx, jsthis, "animationId", &v);
		if (!v.isNumber()) {
			return false;
		}
		animationId = JavascriptManager::getInt32(v);
	}
	auto projectData = GameManager::getInstance()->getProjectData();
	auto animationData = projectData->getAnimationData(animationId);
	if (!animationData) {
		return true;
	}
	auto motionList = animationData->getMotionList();
	cocos2d::DictElement *el = nullptr;
	CCDICT_FOREACH(motionList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto p = static_cast<agtk::data::MotionData *>(el->getObject());
#else
		auto p = dynamic_cast<agtk::data::MotionData *>(el->getObject());
#endif
		if (strcmp(p->getName(), motionName.c_str()) == 0) {
			args.rval().set(JS::Int32Value(p->getId()));
			return true;
		}
	}

	return true;
}

// motion

// directions
bool jsb_register_agtk_animation_motion_directions(JSContext *cx, JSObject *agtkObj)
{
	JS::RootedObject robj(cx, agtkObj);

	static JSClass directions_class = {
		"directions",
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
	auto directions = JS_DefineObject(cx, robj, "directions", &directions_class, JS::NullPtr(), JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE);
	CCASSERT(directions, "directions should not be nullptr");
	JS::RootedObject rdirections(cx, directions);
	static JSFunctionSpec directions_methods[] = {
		{ "get", js_directions_get, 0, 0, 0 },
		{ "getIdByName", js_directions_getIdByName, 0, 0, 0 },
		{ "getIdList", js_directions_getIdList, 0, 0, 0 },
		{ NULL },
	};
	JS_DefineFunctions(cx, rdirections, directions_methods);

	JS::RootedValue rval(cx);
	rval.setObject(*directions);
	JS_SetProperty(cx, robj, "directions", rval);
	{
		JS::RootedValue v(cx);
		JS_GetProperty(cx, robj, "id", &v);
		if (!v.isNumber()) {
			return false;
		}
		JS_DefineProperty(cx, rdirections, "motionId", v, JSPROP_ENUMERATE | JSPROP_PERMANENT);
		JS_GetProperty(cx, robj, "animationId", &v);
		if (!v.isNumber()) {
			return false;
		}
		JS_DefineProperty(cx, rdirections, "animationId", v, JSPROP_ENUMERATE | JSPROP_PERMANENT);
	}
	return true;
}

bool js_directions_get(JSContext *cx, unsigned argc, JS::Value *vp)
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
	auto directionId = JavascriptManager::getInt32(v);
	int animationId = -1;
	int motionId = -1;
	{
		JS::RootedValue v(cx);
		JS_GetProperty(cx, jsthis, "animationId", &v);
		if (!v.isNumber()) {
			return false;
		}
		animationId = JavascriptManager::getInt32(v);
		JS_GetProperty(cx, jsthis, "motionId", &v);
		if (!v.isNumber()) {
			return false;
		}
		motionId = JavascriptManager::getInt32(v);
	}
	auto projectData = GameManager::getInstance()->getProjectData();
	auto animationData = projectData->getAnimationData(animationId);
	if (!animationData) {
		return true;
	}
	auto motionData = animationData->getMotionData(motionId);
	if (!motionData) {
		return true;
	}
	auto directionData = motionData->getDirectionData(directionId);
	if (!directionData) {
		return true;
	}
	static JSClass direction_class = {
		"direction",
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
	auto direction = JS_DefineObject(cx, jsthis, "direction", &direction_class, JS::NullPtr(), JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE);
	CCASSERT(direction, "direction should not be nullptr");
	JS::RootedObject rdirection(cx, direction);
#if 0
	static JSFunctionSpec direction_methods[] = {
		//{ "getValue", js_switch_getValue, 0, 0, 0 },
		{ NULL },
	};
	JS_DefineFunctions(cx, rdirection, direction_methods);
#endif
	JS::RootedValue ranimationId(cx, JS::Int32Value(animationId));
	JS_DefineProperty(cx, rdirection, "animationId", ranimationId, JSPROP_ENUMERATE | JSPROP_PERMANENT);
	JS::RootedValue rmotionId(cx, JS::Int32Value(motionId));
	JS_DefineProperty(cx, rdirection, "motionId", rmotionId, JSPROP_ENUMERATE | JSPROP_PERMANENT);
	JS::RootedValue rdirectionId(cx, JS::Int32Value(directionId));
	JS_DefineProperty(cx, rdirection, "id", rdirectionId, JSPROP_ENUMERATE | JSPROP_PERMANENT);
	JS::RootedValue rdirectionName(cx, std_string_to_jsval(cx, directionData->getName()));
	JS_DefineProperty(cx, rdirection, "name", rdirectionName, JSPROP_ENUMERATE | JSPROP_PERMANENT);
	JS::RootedValue rval(cx);
	rval.setInt32(directionId);
	JS_SetProperty(cx, rdirection, "directionId", rval);
	if (!jsb_register_agtk_animation_motion_direction_tracks(cx, direction)) return false;
	args.rval().set(OBJECT_TO_JSVAL(direction));
	return true;
}

bool js_directions_getIdList(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//ret: <array : 弾IDの配列>
	JS::CallArgs args = CallArgsFromVp(argc, vp);

	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
	std::list<int> idList;
    do {
        int animationId = -1;
        int motionId = -1;
        {
            JS::RootedValue v(cx);
            JS_GetProperty(cx, jsthis, "animationId", &v);
            if (!v.isNumber()) {
                break;
            }
            animationId = JavascriptManager::getInt32(v);
            JS_GetProperty(cx, jsthis, "motionId", &v);
            if (!v.isNumber()) {
                break;
            }
            motionId = JavascriptManager::getInt32(v);
        }
        auto projectData = GameManager::getInstance()->getProjectData();
        auto animationData = projectData->getAnimationData(animationId);
        if (animationData) {
            auto motionData = animationData->getMotionData(motionId);
            if (motionData) {
                auto directionList = motionData->getDirectionList();
                cocos2d::DictElement *el = nullptr;
                CCDICT_FOREACH(directionList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
					auto p = static_cast<agtk::data::DirectionData *>(el->getObject());
#else
					auto p = dynamic_cast<agtk::data::DirectionData *>(el->getObject());
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

bool js_directions_getIdByName(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//arg1: <string: 弾名>
	//ret: <int : 弾ID>、見つからなければ-1。
	JS::CallArgs args = CallArgsFromVp(argc, vp);
    args.rval().set(JS::Int32Value(-1));
	auto len = args.length();
	if (len < 1) {
		return false;
	}
	std::string directionName;
	{
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
		auto &v2 = args[0];
#else
#endif
		if (!v2.isString()) {
			return false;
		}
		if (!jsval_to_std_string(cx, v2, &directionName)) {
			return false;
		}
	}

	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
	int animationId = -1;
	int motionId = -1;
	{
		JS::RootedValue v(cx);
		JS_GetProperty(cx, jsthis, "animationId", &v);
		if (!v.isNumber()) {
			return false;
		}
		animationId = JavascriptManager::getInt32(v);
		JS_GetProperty(cx, jsthis, "motionId", &v);
		if (!v.isNumber()) {
			return false;
		}
		motionId = JavascriptManager::getInt32(v);
	}
	auto projectData = GameManager::getInstance()->getProjectData();
	auto animationData = projectData->getAnimationData(animationId);
	if (!animationData) {
		return true;
	}
	auto motionData = animationData->getMotionData(motionId);
	if (!motionData) {
		return true;
	}
	auto directionList = motionData->getDirectionList();
	cocos2d::DictElement *el = nullptr;
	CCDICT_FOREACH(directionList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto p = static_cast<agtk::data::DirectionData *>(el->getObject());
#else
		auto p = dynamic_cast<agtk::data::DirectionData *>(el->getObject());
#endif
		if (strcmp(p->getName(), directionName.c_str()) == 0) {
			args.rval().set(JS::Int32Value(p->getId()));
			return true;
		}
	}

	args.rval().set(JS::Int32Value(-1));
	return true;
}

// direction

// tracks
bool jsb_register_agtk_animation_motion_direction_tracks(JSContext *cx, JSObject *agtkObj)
{
	JS::RootedObject robj(cx, agtkObj);

	static JSClass tracks_class = {
		"tracks",
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
	auto tracks = JS_DefineObject(cx, robj, "tracks", &tracks_class, JS::NullPtr(), JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE);
	CCASSERT(tracks, "tracks should not be nullptr");
	JS::RootedObject rtracks(cx, tracks);
	static JSFunctionSpec tracks_methods[] = {
		{ "get", js_tracks_get, 0, 0, 0 },
		{ "getIdByName", js_tracks_getIdByName, 0, 0, 0 },
		{ "getIdList", js_tracks_getIdList, 0, 0, 0 },
		{ NULL },
	};
	JS_DefineFunctions(cx, rtracks, tracks_methods);

	JS::RootedValue rval(cx);
	rval.setObject(*tracks);
	JS_SetProperty(cx, robj, "tracks", rval);
	{
		JS::RootedValue v(cx);
		JS_GetProperty(cx, robj, "id", &v);
		if (!v.isNumber()) {
			return false;
		}
		JS_DefineProperty(cx, rtracks, "directionId", v, JSPROP_ENUMERATE | JSPROP_PERMANENT);
		JS_GetProperty(cx, robj, "animationId", &v);
		if (!v.isNumber()) {
			return false;
		}
		JS_DefineProperty(cx, rtracks, "animationId", v, JSPROP_ENUMERATE | JSPROP_PERMANENT);
		JS_GetProperty(cx, robj, "motionId", &v);
		if (!v.isNumber()) {
			return false;
		}
		JS_DefineProperty(cx, rtracks, "motionId", v, JSPROP_ENUMERATE | JSPROP_PERMANENT);
	}
	return true;
}

bool js_tracks_get(JSContext *cx, unsigned argc, JS::Value *vp)
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
	auto trackId = JavascriptManager::getInt32(v);
	int animationId = -1;
	int motionId = -1;
	int directionId = -1;
	{
		JS::RootedValue v(cx);
		JS_GetProperty(cx, jsthis, "animationId", &v);
		if (!v.isNumber()) {
			return false;
		}
		animationId = JavascriptManager::getInt32(v);
		JS_GetProperty(cx, jsthis, "motionId", &v);
		if (!v.isNumber()) {
			return false;
		}
		motionId = JavascriptManager::getInt32(v);
		JS_GetProperty(cx, jsthis, "directionId", &v);
		if (!v.isNumber()) {
			return false;
		}
		directionId = JavascriptManager::getInt32(v);
	}
	auto projectData = GameManager::getInstance()->getProjectData();
	auto animationData = projectData->getAnimationData(animationId);
	if (!animationData) {
		return true;
	}
	auto motionData = animationData->getMotionData(motionId);
	if (!motionData) {
		return true;
	}
	auto directionData = motionData->getDirectionData(directionId);
	if (!directionData) {
		return true;
	}
	auto timelineInfoData = directionData->getTimelineInfoData(trackId);
	if (!timelineInfoData) {
		return true;
	}
	static JSClass track_class = {
		"track",
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
	auto track = JS_DefineObject(cx, jsthis, "track", &track_class, JS::NullPtr(), JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE);
	CCASSERT(track, "track should not be nullptr");
	JS::RootedObject rtrack(cx, track);
#if 0
	static JSFunctionSpec track_methods[] = {
		//{ "getValue", js_switch_getValue, 0, 0, 0 },
		{ NULL },
	};
	JS_DefineFunctions(cx, rtrack, track_methods);
#endif
	JS::RootedValue ranimationId(cx, JS::Int32Value(animationId));
	JS_DefineProperty(cx, rtrack, "animationId", ranimationId, JSPROP_ENUMERATE | JSPROP_PERMANENT);
	JS::RootedValue rmotionId(cx, JS::Int32Value(motionId));
	JS_DefineProperty(cx, rtrack, "motionId", rmotionId, JSPROP_ENUMERATE | JSPROP_PERMANENT);
	JS::RootedValue rdirectionId(cx, JS::Int32Value(directionId));
	JS_DefineProperty(cx, rtrack, "directionId", rdirectionId, JSPROP_ENUMERATE | JSPROP_PERMANENT);
	JS::RootedValue rtrackId(cx, JS::Int32Value(trackId));
	JS_DefineProperty(cx, rtrack, "id", rtrackId, JSPROP_ENUMERATE | JSPROP_PERMANENT);
	JS::RootedValue rtrackName(cx, std_string_to_jsval(cx, timelineInfoData->getName()));
	JS_DefineProperty(cx, rtrack, "name", rtrackName, JSPROP_ENUMERATE | JSPROP_PERMANENT);
	JS::RootedValue rtimelineType(cx, JS::Int32Value(timelineInfoData->getTimelineType()));
	JS_DefineProperty(cx, rtrack, "timelineType", rtimelineType, JSPROP_ENUMERATE | JSPROP_PERMANENT);
	JS::RootedValue rval(cx);
	rval.setInt32(animationId);
	JS_SetProperty(cx, rtrack, "trackId", rval);
	args.rval().set(OBJECT_TO_JSVAL(track));
	return true;
}

bool js_tracks_getIdList(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//ret: <array : 弾IDの配列>
	JS::CallArgs args = CallArgsFromVp(argc, vp);

	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
	std::list<int> idList;
    do {
        int animationId = -1;
        int motionId = -1;
        int directionId = -1;
        {
            JS::RootedValue v(cx);
            JS_GetProperty(cx, jsthis, "animationId", &v);
            if (!v.isNumber()) {
                break;
            }
            animationId = JavascriptManager::getInt32(v);
            JS_GetProperty(cx, jsthis, "motionId", &v);
            if (!v.isNumber()) {
                break;
            }
            motionId = JavascriptManager::getInt32(v);
            JS_GetProperty(cx, jsthis, "directionId", &v);
            if (!v.isNumber()) {
                break;
            }
            directionId = JavascriptManager::getInt32(v);
        }
        auto projectData = GameManager::getInstance()->getProjectData();
        auto animationData = projectData->getAnimationData(animationId);
        if (animationData) {
            auto motionData = animationData->getMotionData(motionId);
            if (motionData) {
                auto directionData = motionData->getDirectionData(directionId);
                if (directionData) {
                    auto timelineInfoList = directionData->getTimelineInfoList();
                    cocos2d::DictElement *el = nullptr;
                    CCDICT_FOREACH(timelineInfoList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
						auto p = static_cast<agtk::data::TimelineInfoData *>(el->getObject());
#else
						auto p = dynamic_cast<agtk::data::TimelineInfoData *>(el->getObject());
#endif
                        idList.push_back(p->getId());
                    }
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

bool js_tracks_getIdByName(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//arg1: <string: 弾名>
	//ret: <int : 弾ID>、見つからなければ-1。
	JS::CallArgs args = CallArgsFromVp(argc, vp);
    args.rval().set(JS::Int32Value(-1));
	auto len = args.length();
	if (len < 1) {
		return false;
	}
	std::string trackName;
	{
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
		auto &v2 = args[0];
#else
#endif
		if (!v2.isString()) {
			return false;
		}
		if (!jsval_to_std_string(cx, v2, &trackName)) {
			return false;
		}
	}

	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
	int animationId = -1;
	int motionId = -1;
	int directionId = -1;
	{
		JS::RootedValue v(cx);
		JS_GetProperty(cx, jsthis, "animationId", &v);
		if (!v.isNumber()) {
			return false;
		}
		animationId = JavascriptManager::getInt32(v);
		JS_GetProperty(cx, jsthis, "motionId", &v);
		if (!v.isNumber()) {
			return false;
		}
		motionId = JavascriptManager::getInt32(v);
		JS_GetProperty(cx, jsthis, "directionId", &v);
		if (!v.isNumber()) {
			return false;
		}
		directionId = JavascriptManager::getInt32(v);
	}
	auto projectData = GameManager::getInstance()->getProjectData();
	auto animationData = projectData->getAnimationData(animationId);
	if (!animationData) {
		return true;
	}
	auto motionData = animationData->getMotionData(motionId);
	if (!motionData) {
		return true;
	}
	auto directionData = motionData->getDirectionData(directionId);
	if (!directionData) {
		return true;
	}
	auto timlineInfoList = directionData->getTimelineInfoList();
	cocos2d::DictElement *el = nullptr;
	CCDICT_FOREACH(timlineInfoList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto p = static_cast<agtk::data::TimelineInfoData *>(el->getObject());
#else
		auto p = dynamic_cast<agtk::data::TimelineInfoData *>(el->getObject());
#endif
		if (strcmp(p->getName(), trackName.c_str()) == 0) {
			args.rval().set(JS::Int32Value(p->getId()));
			return true;
		}
	}

	return true;
}

// track

//===============================================================================================//
