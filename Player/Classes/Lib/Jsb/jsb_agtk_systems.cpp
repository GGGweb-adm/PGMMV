// #AGTK-NX #AGTK-WIN
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

static bool register_agtk_systems(JSContext *_cx,JS::HandleObject object);

//systems
static bool js_system_getGameDeltaTime(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_system_getScaledGameDeltaTime(JSContext *cx, unsigned argc, JS::Value *vp);

#include "JavascriptManager.h"

void jsb_register_agtk_systems(JSContext *cx, JS::HandleObject object)
{
	auto ret = register_agtk_systems(cx, object);
	CCASSERT(ret, "Failed to register_agtk_systems");
}

bool register_agtk_systems(JSContext *cx, JS::HandleObject object)
{
	JS::RootedObject robj(cx, object);

	static JSClass systems_class = {
		"system",
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
	auto system = JS_DefineObject(cx, robj, "system", &systems_class, JS::NullPtr(), JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE);
	CCASSERT(system, "system should not be nullptr");
	JS::RootedObject rsystem(cx, system);

	static JSFunctionSpec system_methods[] = {
		{ "getGameDeltaTime", js_system_getGameDeltaTime, 0, 0, 0 },
		{ "getScaledGameDeltaTime", js_system_getScaledGameDeltaTime, 0, 0, 0 },
		{ NULL },
	};
	JS_DefineFunctions(cx, rsystem, system_methods);
	return true;
}

bool js_system_getGameDeltaTime(JSContext *cx, unsigned argc, JS::Value *vp)
{
	JS::CallArgs args = CallArgsFromVp(argc, vp);
	auto delta = Director::getInstance()->getAnimationInterval();
	args.rval().set(JS::Float32Value(delta));
	return true;
}

bool js_system_getScaledGameDeltaTime(JSContext *cx, unsigned argc, JS::Value *vp)
{
	JS::CallArgs args = CallArgsFromVp(argc, vp);
	auto delta = Director::getInstance()->getAnimationInterval();
	auto timeScale = Director::getInstance()->getScheduler()->getTimeScale();
	delta *= timeScale;
	args.rval().set(JS::Float32Value(delta));
	return true;
}

//===============================================================================================//
