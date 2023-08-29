#include "Lib/Macros.h"
#include "AudioManager.h"
#include "GameManager.h"
#include "InputManager.h"
#include "jsapi.h"
#include "scripting/js-bindings/manual/ScriptingCore.h"

// test js
#include "jsapi.h"
#include "jsfriendapi.h"
#include "scripting/js-bindings/manual/js_bindings_config.h"
#include "scripting/js-bindings/manual/js_bindings_core.h"
#include "scripting/js-bindings/manual/spidermonkey_specifics.h"
#include "scripting/js-bindings/manual/js_manual_conversions.h"
#include "mozilla/Maybe.h"
#include "scripting/js-bindings/manual/js-BindingsExport.h"
#include "Lib/Jsb/jsb_agtk_settings.h"
#include "Lib/Jsb/jsb_agtk_scenes.h"
#include "Lib/Jsb/jsb_agtk_tilesets.h"
#include "Lib/Jsb/jsb_agtk_objects.h"
#include "Lib/Jsb/jsb_agtk_images.h"
#include "Lib/Jsb/jsb_agtk_fonts.h"
#include "Lib/Jsb/jsb_agtk_texts.h"
#include "Lib/Jsb/jsb_agtk_movies.h"
#include "Lib/Jsb/jsb_agtk_bgms.h"
#include "Lib/Jsb/jsb_agtk_ses.h"
#include "Lib/Jsb/jsb_agtk_voices.h"
#include "Lib/Jsb/jsb_agtk_animations.h"
#include "Lib/Jsb/jsb_agtk_portals.h"
#include "Lib/Jsb/jsb_agtk_objectInstances.h"
#include "Lib/Jsb/jsb_agtk_sceneInstances.h"
#include "Lib/Jsb/jsb_agtk_databases.h"
// #AGTK-NX #AGTK-WIN
#if 1
#include "Lib/Jsb/jsb_agtk_systems.h"
#endif
//getScriptType()
//ScriptingCore
//rootScriptObject
//static void enableTracing(JSContext *cx);
//static void disableTracing(JSContext *cx);
//static bool addObject(JSContext *cx, JS::HandleObject global);
static bool js_log(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_reset(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_actionCommands_objectCreate(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_actionCommands_objectDestroy(JSContext *cx, unsigned argc, JS::Value *vp);
//controllers
static bool js_controllers_getOperationKeyPressed(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_controllers_getOperationKeyValue(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_controllers_getKeyValue(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_controllers_isConnected(JSContext *cx, unsigned argc, JS::Value *vp);

// #AGTK-NX #AGTK-WIN
#if 1
static bool js_controllers_isJoyCon(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_controllers_isSeparatableJoyCon(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_controllers_isJoinableJoyCon(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_controllers_setAssignmentJoyCon(JSContext *cx, unsigned argc, JS::Value *vp);
#endif

//switches
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_7
bool jsb_register_agtk_projectSwitchBase(JSContext *cx, JSObject *agtkObj);
#endif
static bool js_switches_get(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_switches_getIdByName(JSContext *cx, unsigned argc, JS::Value *vp);
//switch
static bool js_switch_getValue(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_switch_setValue(JSContext *cx, unsigned argc, JS::Value *vp);

//variables
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_7
bool jsb_register_agtk_projectVariableBase(JSContext *cx, JSObject *agtkObj);
#endif
static bool js_variables_get(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_variables_getIdByName(JSContext *cx, unsigned argc, JS::Value *vp);
//variable
static bool js_variable_getValue(JSContext *cx, unsigned argc, JS::Value *vp);
static bool js_variable_setValue(JSContext *cx, unsigned argc, JS::Value *vp);

#include "Lib/ObjectCommand.h"
#include "Data/ObjectCommandData.h"
#include "Manager/DebugManager.h"
#include "Data/PlayData.h"
#include "JavascriptManager.h"

//using namespace JS;

bool JavascriptManager::_pluginLoaded = false;


#if 0
//--------------------------------------------------------------------------------------------------------------------
// シングルトンのインスタンス
//--------------------------------------------------------------------------------------------------------------------
JavascriptManager* JavascriptManager::_jsManager = NULL;

static JSClass global_class = {
    "global",
    JSCLASS_GLOBAL_FLAGS,
    // [SpiderMonkey 38] Replace all stubs but JS_GlobalObjectTraceHook with nullptr.
    JS_PropertyStub,
    JS_DeletePropertyStub,
    JS_PropertyStub,
    JS_StrictPropertyStub,
    JS_EnumerateStub,
    JS_ResolveStub,
    JS_ConvertStub,
    // [SpiderMonkey 24] doesn't need the bits after this; there is
    // no JS_GlobalObjectTraceHook there.
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    JS_GlobalObjectTraceHook
};

//--------------------------------------------------------------------------------------------------------------------
// コンストラクタ
//--------------------------------------------------------------------------------------------------------------------
JavascriptManager::JavascriptManager()
//: _global((JSContext *)nullptr, (JSObject *)nullptr)
{
	_runtime = NULL;
	_context = NULL;
	_initialized = false;
    //_globalObject = nullptr;
    //_global = nullptr;
}

//--------------------------------------------------------------------------------------------------------------------
// デストラクタ
//--------------------------------------------------------------------------------------------------------------------
JavascriptManager::~JavascriptManager()
{
	term();
}

//--------------------------------------------------------------------------------------------------------------------
// インスタンスを取得する。
//--------------------------------------------------------------------------------------------------------------------
JavascriptManager* JavascriptManager::getInstance()
{
	if(!_jsManager){
		_jsManager = new JavascriptManager();
	}
	return _jsManager;
}

//--------------------------------------------------------------------------------------------------------------------
// シングルトンを破棄する。
//--------------------------------------------------------------------------------------------------------------------
void JavascriptManager::purge()
{
	if(!_jsManager){
		return;
	}
	JavascriptManager *jsm = _jsManager;
	_jsManager = NULL;
	jsm->release();
}

bool JavascriptManager::init(unsigned int maxbytes, unsigned int stackChunkSize)
{
	if(_initialized){
		return true;
	}
	JS_Init();
	_runtime = JS_NewRuntime(maxbytes);
	if(!_runtime){
		return false;
	}
	_context = JS_NewContext(_runtime, stackChunkSize);
	if(!_context){
		return false;
	}
    JS_BeginRequest(_context);
    //JS_SetOptions
    //JS_SetVersion
	JS_SetErrorReporter(_context, JavascriptManager::reportError);
#if 0//def USE_PREVIEW
	enableTracing(_context);
#endif
	//_global = JS::RootedObject(_context, JS_NewGlobalObject(_context, &global_class, nullptr, JS::FireOnNewGlobalHook));
    //_globalObject = JS_NewGlobalObject(_context, &global_class, nullptr, JS::FireOnNewGlobalHook);
    //JS::AddNamedObjectRoot(_context, &_globalObject, "test-global");
    _global.construct(_context);
    _global.ref() = JS_NewGlobalObject(_context, &global_class, nullptr, JS::FireOnNewGlobalHook);
	_initialized = true;
	return true;
}

void JavascriptManager::term()
{
	if(!_initialized){
		return;
	}
    //_global = nullptr;
    //if(_globalObject != nullptr){
        //JS::RemoveObjectRoot(_context, &_globalObject);
        //_globalObject = nullptr;
    //}
    _global.ref() = nullptr;
	if(_context != NULL){
		JS_EndRequest(_context);
		JS_DestroyContext(_context);
		_context = NULL;
	}
	if(_runtime != NULL){
		JS_DestroyRuntime(_runtime);
		_runtime = NULL;
	}
	JS_ShutDown();
	_initialized = false;
}
#endif

void JavascriptManager::reportError(JSContext *context, const char *message, JSErrorReport *report)
{
	CCLOG("%s\n%s:%u:%s\n",
		report->linebuf,
		report->filename ? report->filename : "<no filename=\"filename\">",
		(unsigned int)report->lineno,
		message
	);
	auto debugManager = agtk::DebugManager::getInstance();
	debugManager->getDebugExecuteLogWindow()->addLog("%s:%u:%s\n",
		report->filename ? report->filename : "no filename",
		(unsigned int)report->lineno,
		message
	);
#if defined(USE_PREVIEW)
	auto fp = GameManager::getScriptLogFp();
	if (fp) {
		fprintf(fp, "%s: %u: %s\n",
			report->filename ? report->filename : "no filename",
			(unsigned int)report->lineno,
			message
		);
		fflush(fp);
	}
#endif
	CCASSERT(0, "JavaScript error");
}

#if 0
enum my_tinyid {
	MY_COLOR, MY_HEIGHT, MY_WIDTH, MY_FUNNY, MY_ARRAY, MY_RDONLY
};
/*
* Given the above definitions and call to JS_DefineProperties, obj will
* need this sort of "getter" method in its class (my_class, above).  See
* the example for the "It" class in js.c.
*/
static bool 
my_getProperty(JSContext *cx, JS::HandleObject obj, JS::HandleId id, JS::MutableHandleValue vp)
{
	if (true) {
		JS::RootedValue v(cx);
		JS::RootedString someString(cx, JS_NewStringCopyZ(cx, "Dave"));
		//CCLOG("id: JSID_IS_STRING: %s, JSID_IS_INT: %s", JSID_IS_STRING(id) ? "true" : "false", JSID_IS_INT(id) ? "true" : "false");
		JSString* jss = JSID_TO_STRING(id);
		char *p = JS_EncodeString(cx, jss);
#if 1
		if (strcmp(p, "color") == 0){
			vp.setInt32(123);
		} else if (strcmp(p, "height") == 0){
			vp.setInt32(456);
		} else if (strcmp(p, "width") == 0){
			vp.setInt32(789);
		} else if (strcmp(p, "funny") == 0){
			vp.setDouble(0.5);
		} else if (strcmp(p, "array") == 0){
			vp.setString(someString);
		} else if (strcmp(p, "rdonly") == 0){
			vp.setUndefined();
		} else {
			CCLOG("Unimplemented property getter: %s", p);
			return false;
		}
#else
		switch (id.get().asBits) {
		case MY_COLOR:  vp.setInt32(123); break;
		case MY_HEIGHT: vp.setInt32(456); break;
		case MY_WIDTH:  vp.setInt32(789); break;
		case MY_FUNNY:  vp.setDouble(0.5); break;
		case MY_ARRAY:  vp.setString(someString); break;
		case MY_RDONLY: vp.setUndefined(); break;
		default: return false;
		}
#endif
	}
	return true;
}
#endif

bool JavascriptManager::addObject(JSContext* cx, JS::HandleObject global)
{
	JS_SetErrorReporter(cx, JavascriptManager::reportError);

	/* Statically initialize a class to make "one-off" objects. */
#if 0
	static JSClass agtk_class = {
		"Agtk",
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
	static JSClass my_class = {
		"MyClass",
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
#endif

	JSObject *obj;

	{
#if 0
		/*
		* Define an object named in the global scope that can be enumerated by
		* for/in loops.  The parent object is passed as the second argument, as
		* with all other API calls that take an object/name pair.  The prototype
		* passed in is null, so the default object prototype will be used.
		*/
		obj = JS_DefineObject(cx, global, "console", &my_class, JS::NullPtr(), JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE);
		//	obj = JS_DefineObject(cx, global, "myObject", nullptr, JS::NullPtr(), JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE);
		//JS_NewObject(cx, JS::NullPtr(), JS::NullPtr(), JS::NullPtr());
		CCASSERT(obj, "obj should not be nullptr");
#endif
#if 1
		ScriptingCore* sc = ScriptingCore::getInstance();
		JS::RootedObject gobj(cx, sc->getGlobalObject());
		JS::RootedObject ns(cx);
		std::string name("Agtk");
		JS::MutableHandleObject jsObj = &ns;
		JS::RootedValue nsval(cx);
		JS_GetProperty(cx, gobj, name.c_str(), &nsval);
		if (nsval == JSVAL_VOID) {
			jsObj.set(JS_NewObject(cx, nullptr, JS::NullPtr(), JS::NullPtr()));
			nsval = OBJECT_TO_JSVAL(jsObj);
			JS_SetProperty(cx, gobj, name.c_str(), nsval);
		}
		else {
			jsObj.set(nsval.toObjectOrNull());
		}
		obj = jsObj;
#else
		ScriptingCore* sc = ScriptingCore::getInstance();
		JS::RootedObject obj(cx, sc->getGlobalObject());
		JS::RootedObject ns(cx);
		sc->get_or_create_js_obj(cx, obj, "Agtk", &ns);
		void get_or_create_js_obj(JSContext* cx, JS::HandleObject obj, const std::string &name, JS::MutableHandleObject jsObj)
		{
			JS::RootedValue nsval(cx);
			JS_GetProperty(cx, obj, name.c_str(), &nsval);
			if (nsval == JSVAL_VOID) {
				jsObj.set(JS_NewObject(cx, nullptr, JS::NullPtr(), JS::NullPtr()));
				nsval = OBJECT_TO_JSVAL(jsObj);
				JS_SetProperty(cx, obj, name.c_str(), nsval);
			}
			else {
				jsObj.set(nsval.toObjectOrNull());
			}
		}
#endif

#if 0
		/*
		* Define a bunch of properties with a JSPropertySpec array statically
		* initialized and terminated with a null-name entry.  Besides its name,
		* each property has a "tiny" identifier (MY_COLOR, e.g.) that can be used
		* in switch statements (in a common my_getProperty function, for example).
		*/

		JS::RootedObject robj(cx, obj);
		JSFunctionSpec console_methods[] = {
			{ "log", js_log, 0, 0, 0 },
			{ NULL },
		};
		JS_DefineFunctions(cx, robj, console_methods);
#endif
	}

#if 0
	obj = JS_DefineObject(cx, global, "Agtk", &agtk_class, JS::NullPtr(), JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE);
	CCASSERT(obj, "obj should not be nullptr");
#endif

	JS::RootedObject robj(cx, obj);
	{
		auto gameManager = GameManager::getInstance();
		JS::RootedValue rversion(cx, std_string_to_jsval(cx, cocos2d::String::createWithFormat("%s %s", gameManager->getAppName()->getCString(), gameManager->getAppVersion()->getCString())->getCString()));
		JS_SetProperty(cx, robj, "version", rversion);
	}
	JSFunctionSpec agtk_methods[] = {
		{ "log", js_log, 0, 0, 0 },
		{ "reset", js_reset, 0, 0, 0 },
		{ NULL },
	};
	JS_DefineFunctions(cx, robj, agtk_methods);
#if 0
	static JSPropertySpec my_props[] = {
		{ "color", MY_COLOR, JSPROP_ENUMERATE },
		{ "height", MY_HEIGHT, JSPROP_ENUMERATE },
		{ "width", MY_WIDTH, JSPROP_ENUMERATE },
		{ "funny", MY_FUNNY, JSPROP_ENUMERATE },
		{ "array", MY_ARRAY, JSPROP_ENUMERATE },
		{ "rdonly", MY_RDONLY, JSPROP_READONLY },
		{ 0 }
	};

	JS_DefineProperty(cx, robj, "color", MY_COLOR, JSPROP_ENUMERATE, my_getProperty, JS_StrictPropertyStub);
	JS_DefineProperty(cx, robj, "height", MY_HEIGHT, JSPROP_ENUMERATE, my_getProperty, JS_StrictPropertyStub);
	JS_DefineProperty(cx, robj, "width", MY_WIDTH, JSPROP_ENUMERATE, my_getProperty, JS_StrictPropertyStub);
	JS_DefineProperty(cx, robj, "funny", MY_FUNNY, JSPROP_ENUMERATE, my_getProperty, JS_StrictPropertyStub);
	JS_DefineProperty(cx, robj, "array", MY_ARRAY, JSPROP_ENUMERATE, my_getProperty, JS_StrictPropertyStub);
	JS_DefineProperty(cx, robj, "rdonly", MY_RDONLY, JSPROP_ENUMERATE, my_getProperty, JS_StrictPropertyStub);
#endif

	if (!addActionCommands(cx, obj)) return false;
	if (!addControllers(cx, obj)) return false;
	if (!addSwitches(cx, obj)) return false;
	if (!addVariables(cx, obj)) return false;
	jsb_register_agtk_settings(cx, robj);
	jsb_register_agtk_scenes(cx, robj);
	jsb_register_agtk_tilesets(cx, robj);
	jsb_register_agtk_objects(cx, robj);
	jsb_register_agtk_images(cx, robj);
	jsb_register_agtk_fonts(cx, robj);
	jsb_register_agtk_texts(cx, robj);
	jsb_register_agtk_movies(cx, robj);
	jsb_register_agtk_bgms(cx, robj);
	jsb_register_agtk_ses(cx, robj);
	jsb_register_agtk_voices(cx, robj);
	jsb_register_agtk_animations(cx, robj);
	jsb_register_agtk_portals(cx, robj);
	jsb_register_agtk_objectInstances(cx, robj);
	jsb_register_agtk_sceneInstances(cx, robj);
	jsb_register_agtk_databases(cx, robj);
// #AGTK-NX #AGTK-WIN
#if 1
	jsb_register_agtk_systems(cx, robj);
#endif

	return true;
}

void JavascriptManager::removeObject()
{
	if (!ScriptingCore::isCreateInstance()) {
		return;
	}

	ScriptingCore::getInstance()->evalString("if(Agtk){ Agtk.log(\"Agtk: \" + Agtk); delete Agtk; Agtk = null; }");
}

bool JavascriptManager::loadPlugins()
{
	auto projectData = GameManager::getInstance()->getProjectData();
	auto projectPath = GameManager::getProjectPathFromProjectFile(GameManager::getInstance()->getProjectFilePath()->getCString());
	auto pluginList = projectData->getPluginArray();
	cocos2d::Ref *ref;
	auto gameInformation = projectData->getGameInformation();
	auto locale = gameInformation->getMainLanguage();
	CCARRAY_FOREACH(pluginList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto pluginData = static_cast<agtk::data::PluginData *>(ref);
#else
		auto pluginData = dynamic_cast<agtk::data::PluginData *>(ref);
#endif
		CC_ASSERT(pluginData->getFolder() == false);
		if (!pluginData->getEnabled()) {
			continue;
		}
		std::string filename = pluginData->getFilename();
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
		if (stricmp(filename.substr(filename.length() - 7).c_str(), ".coffee") == 0) {
#else
#endif
			filename = filename.substr(0, filename.length() - 7) + ".js";
		}
		filename = agtk::data::getFullFilename(filename, projectPath);
		auto jsData = FileUtils::getInstance()->getStringFromFile(filename);
		//CCLOG("jsData: %s", jsData.c_str());
		if (jsData.length() == 0) {
			auto errorStr = String::createWithFormat("Runtime error: %s is not found or empty.", filename.c_str())->getCString();
			agtk::DebugManager::getInstance()->getDebugExecuteLogWindow()->addLog(errorStr);
#if defined(USE_PREVIEW)
			auto fp = GameManager::getScriptLogFp();
			if (fp) {
				fwrite(errorStr, 1, strlen(errorStr), fp);
				fwrite("\n", 1, 1, fp);
			}
#endif
			continue;
		}
		auto script = String::createWithFormat("(function(){ var obj = %s; if(obj != null) Agtk.plugins.reload(obj, %d, \"%s\", %s, %s); })()", jsData.c_str(), pluginData->getId(), locale, pluginData->getInternalJson(), pluginData->getParamValueJson());
		if (!ScriptingCore::getInstance()->evalString(script->getCString())) {
			auto errorStr = String::createWithFormat("Runtime error in Agtk.plugins.reload(script: %s, pluginId: %s, locale: %s, internal: %s, param: %s).", jsData.c_str(), pluginData->getId(), locale, pluginData->getInternalJson(), pluginData->getParamValueJson())->getCString();
			agtk::DebugManager::getInstance()->getDebugExecuteLogWindow()->addLog(errorStr);
#if defined(USE_PREVIEW)
			auto fp = GameManager::getScriptLogFp();
			if (fp) {
				fwrite(errorStr, 1, strlen(errorStr), fp);
				fwrite("\n", 1, 1, fp);
			}
#endif
		}
	}
	_pluginLoaded = true;
	return true;
}

#ifdef USE_PREVIEW
// 自動テストプラグイン読み込み
bool JavascriptManager::loadAutoTestPlugins(std::string relativePath, std::string pluginInternalJson, std::string pluginParamValueJson)
{
	if (relativePath.compare("") != 0) {
		auto projectData = GameManager::getInstance()->getProjectData();
		auto projectPath = GameManager::getProjectPathFromProjectFile(GameManager::getInstance()->getProjectFilePath()->getCString());

		// 設定値
		auto pluginList = projectData->getPluginArray();
		int pluginId = pluginList->count() + 1;	// プラグインIDが被らないよう最大値＋１の値を設定

		// プラグイン読み込み
		auto gameInformation = projectData->getGameInformation();
		auto locale = gameInformation->getMainLanguage();
		if (stricmp(relativePath.substr(relativePath.length() - 7).c_str(), ".coffee") == 0) {
			relativePath = relativePath.substr(0, relativePath.length() - 7) + ".js";
		}
		relativePath = agtk::data::getFullFilename(relativePath, projectPath);
		auto jsData = FileUtils::getInstance()->getStringFromFile(relativePath);
		auto script = String::createWithFormat("(function(){ var obj = %s; if(obj != null) Agtk.plugins.reload(obj, %d, \"%s\", %s, %s); })()", jsData.c_str(), pluginId, locale, pluginInternalJson.c_str(), pluginParamValueJson.c_str());
		if (!ScriptingCore::getInstance()->evalString(script->getCString())) {
			auto errorStr = String::createWithFormat("Runtime error in Agtk.plugins.reload(script: %s, pluginId: %s, locale: %s, internal: %s, param: %s).", jsData.c_str(), pluginId, locale, pluginInternalJson.c_str(), pluginParamValueJson.c_str())->getCString();
			agtk::DebugManager::getInstance()->getDebugExecuteLogWindow()->addLog(errorStr);
#if defined(USE_PREVIEW)
			auto fp = GameManager::getScriptLogFp();
			if (fp) {
				fwrite(errorStr, 1, strlen(errorStr), fp);
				fwrite("\n", 1, 1, fp);
			}
#endif
		}
	}

	return true;
}
#endif

void JavascriptManager::unloadPlugins()
{
	if (!_pluginLoaded) return;
#if 1
	if (!ScriptingCore::isCreateInstance()) {
		return;
	}

	auto sc = ScriptingCore::getInstance();
	auto cx = sc->getGlobalContext();
	auto _global = sc->getGlobalObject();
	JS::RootedObject gobj(cx, _global);
	JSAutoCompartment ac(cx, gobj);

	JS::RootedObject ns(cx);
	std::string name("Agtk");
	JS::MutableHandleObject jsObj = &ns;
	JS::RootedValue nsval(cx);
	JS_GetProperty(cx, gobj, name.c_str(), &nsval);
	if (nsval != JSVAL_VOID) {
		jsObj.set(nsval.toObjectOrNull());
		JS::RootedValue v(cx);
		JS_GetProperty(cx, jsObj, "plugins", &v);
		if (v.isObject()) {
			JS::RootedObject rplugins(cx, &v.toObject());
			JS_GetProperty(cx, rplugins, "unloadPlugins", &v);
			if (v.isObject()) {
				JS::RootedValue runloadPlugins(cx, v);
				//jsval args[1];
				JS::RootedValue rval(cx);
				auto result = JS_CallFunctionValue(cx, rplugins, runloadPlugins, JS::HandleValueArray::fromMarkedLocation(0, nullptr), &rval);
			}

		}
	}
#else
	auto script = String::createWithFormat("(function(){ var dt = %f; for(var i = Agtk.plugins.length - 1; i >= 0; i--){ var plugin = Agtk.plugins[i]; Agtk.plugins.unload(plugin.id); } })()");
	ScriptingCore::getInstance()->evalString(script->getCString());
#endif
	_pluginLoaded = false;
}

void JavascriptManager::updatePlugins(float dt)
{
	if (!_pluginLoaded) return;
#if 1
	auto sc = ScriptingCore::getInstance();
	auto cx = sc->getGlobalContext();
	auto _global = sc->getGlobalObject();
	JS::RootedObject gobj(cx, _global);
	JSAutoCompartment ac(cx, gobj);

	JS::RootedObject ns(cx);
	std::string name("Agtk");
	JS::MutableHandleObject jsObj = &ns;
	JS::RootedValue nsval(cx);
	JS_GetProperty(cx, gobj, name.c_str(), &nsval);
	if (nsval != JSVAL_VOID) {
		jsObj.set(nsval.toObjectOrNull());
		JS::RootedValue v(cx);
		JS_GetProperty(cx, jsObj, "plugins", &v);
		if (v.isObject()) {
			//auto plugins = v.toObject();
			JS::RootedObject rplugins(cx, &v.toObject());
			JS_GetProperty(cx, rplugins, "updatePlugins", &v);
			if (v.isObject()) {
				//auto updatePlugins = v.toObject();
				JS::RootedValue rupdatePlugins(cx, v);
				jsval args[1];
				args[0] = JS::DoubleValue(dt);
				JS::RootedValue rval(cx);
				auto result = JS_CallFunctionValue(cx, rplugins, rupdatePlugins, JS::HandleValueArray::fromMarkedLocation(1, args), &rval);
#if 0
				if (handler->is_oo) {
					JS::RootedObject arbiterProto(handler->cx, JSB_cpArbiter_object);
					JS::RootedObject spaceProto(handler->cx, JSB_cpSpace_object);
					args[0] = c_class_to_jsval(handler->cx, arb, arbiterProto, JSB_cpArbiter_class, "cpArbiter");
					args[1] = c_class_to_jsval(handler->cx, space, spaceProto, JSB_cpSpace_class, "cpArbiter");
				}
				else {
					args[0] = opaque_to_jsval(handler->cx, arb);
					args[1] = opaque_to_jsval(handler->cx, space);
				}

				JS::RootedValue rval(handler->cx);
				JS::RootedObject jsthis(handler->cx, handler->jsthis);
				JS::RootedValue jsbegin(handler->cx, OBJECT_TO_JSVAL(handler->begin));
				bool ok = JS_CallFunctionValue(handler->cx, jsthis, jsbegin, JS::HandleValueArray::fromMarkedLocation(2, args), &rval);

#endif
			}

		}
	}
#else
	auto script = String::createWithFormat("(function(){ var dt = %f; for(var i = 0; i < Agtk.plugins.length; i++){ var plugin = Agtk.plugins[i]; if(plugin.update){ plugin.update(dt); } } })()", dt);
	ScriptingCore::getInstance()->evalString(script->getCString());
#endif
}

void JavascriptManager::setLocalePlugins(const char *locale)
{
	if (!_pluginLoaded) return;

	auto sc = ScriptingCore::getInstance();
	auto cx = sc->getGlobalContext();
	auto _global = sc->getGlobalObject();
	JS::RootedObject gobj(cx, _global);
	JSAutoCompartment ac(cx, gobj);

	JS::RootedObject ns(cx);
	std::string name("Agtk");
	JS::MutableHandleObject jsObj = &ns;
	JS::RootedValue nsval(cx);
	JS_GetProperty(cx, gobj, name.c_str(), &nsval);
	if (nsval != JSVAL_VOID) {
		jsObj.set(nsval.toObjectOrNull());
		JS::RootedValue v(cx);
		JS_GetProperty(cx, jsObj, "plugins", &v);
		if (v.isObject()) {
			//auto plugins = v.toObject();
			JS::RootedObject rplugins(cx, &v.toObject());
			JS_GetProperty(cx, rplugins, "setLocalePlugins", &v);
			if (v.isObject()) {
				JS::RootedValue rsetLocalePlugins(cx, v);
				jsval args[1];
				args[0] = std_string_to_jsval(cx, std::string(locale));;
				JS::RootedValue rval(cx);
				auto result = JS_CallFunctionValue(cx, rplugins, rsetLocalePlugins, JS::HandleValueArray::fromMarkedLocation(1, args), &rval);
			}

		}
	}
}

std::string JavascriptManager::getPluginInternals()
{
	if (!_pluginLoaded) return "{}";
	auto sc = ScriptingCore::getInstance();
	auto cx = sc->getGlobalContext();
	auto _global = sc->getGlobalObject();
	JS::RootedObject gobj(cx, _global);
	JSAutoCompartment ac(cx, gobj);

	JS::RootedObject ns(cx);
	std::string name("Agtk");
	JS::MutableHandleObject jsObj = &ns;
	JS::RootedValue nsval(cx);
	JS_GetProperty(cx, gobj, name.c_str(), &nsval);
	if (nsval != JSVAL_VOID) {
		jsObj.set(nsval.toObjectOrNull());
		JS::RootedValue v(cx);
		JS_GetProperty(cx, jsObj, "plugins", &v);
		if (v.isObject()) {
			JS::RootedObject rplugins(cx, &v.toObject());
			JS_GetProperty(cx, rplugins, "getPluginInternals", &v);
			if (v.isObject()) {
				JS::RootedValue rgetPluginInternals(cx, v);
				JS::RootedValue rval(cx);
				auto result = JS_CallFunctionValue(cx, rplugins, rgetPluginInternals, JS::HandleValueArray::fromMarkedLocation(0, nullptr), &rval);
				if (result) {
					if (rval.isString()) {
						std::string str;
						if (jsval_to_std_string(cx, rval, &str)) {
							return str;
						}
					}
				}
			}
		}
	}
	return "{}";
}

void JavascriptManager::setPluginInternals(const std::string &internals)
{
	if (!_pluginLoaded) return;
	auto sc = ScriptingCore::getInstance();
	auto cx = sc->getGlobalContext();
	auto _global = sc->getGlobalObject();
	JS::RootedObject gobj(cx, _global);
	JSAutoCompartment ac(cx, gobj);

	JS::RootedObject ns(cx);
	std::string name("Agtk");
	JS::MutableHandleObject jsObj = &ns;
	JS::RootedValue nsval(cx);
	JS_GetProperty(cx, gobj, name.c_str(), &nsval);
	if (nsval != JSVAL_VOID) {
		jsObj.set(nsval.toObjectOrNull());
		JS::RootedValue v(cx);
		JS_GetProperty(cx, jsObj, "plugins", &v);
		if (v.isObject()) {
			JS::RootedObject rplugins(cx, &v.toObject());
			JS_GetProperty(cx, rplugins, "setPluginInternals", &v);
			if (v.isObject()) {
				JS::RootedValue rsetPluginInternals(cx, v);
				jsval args[1];
				args[0] = std_string_to_jsval(cx, internals);
				JS::RootedValue rval(cx);
				auto result = JS_CallFunctionValue(cx, rplugins, rsetPluginInternals, JS::HandleValueArray::fromMarkedLocation(1, args), &rval);
			}
		}
	}
}

bool JavascriptManager::addActionCommands(JSContext *cx, JSObject *agtkObj)
{
	JS::RootedObject robj(cx, agtkObj);

	static JSClass actionCommands_class = {
		"actionCommands",
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
	auto actionCommands = JS_DefineObject(cx, robj, "actionCommands", &actionCommands_class, JS::NullPtr(), JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE);
	CCASSERT(actionCommands, "actionCommands should not be nullptr");
	JS::RootedObject ractionCommands(cx, actionCommands);
	static JSFunctionSpec actionCommands_methods[] = {
		{ "objectCreate", js_actionCommands_objectCreate, 0, 0, 0 },
		{ "objectDestroy", js_actionCommands_objectDestroy, 0, 0, 0 },
		{ NULL },
	};
	JS_DefineFunctions(cx, ractionCommands, actionCommands_methods);
	JS::RootedValue rval(cx);
	rval.setObject(*actionCommands);
	JS_SetProperty(cx, robj, "actionCommands", rval);
	return true;
}

bool JavascriptManager::addControllers(JSContext *cx, JSObject *agtkObj)
{
	JS::RootedObject robj(cx, agtkObj);

	static JSClass controllers_class = {
		"controllers",
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
	auto controllers = JS_DefineObject(cx, robj, "controllers", &controllers_class, JS::NullPtr(), JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE);
	CCASSERT(controllers, "controllers should not be nullptr");
	JS::RootedObject rcontrollers(cx, controllers);
	static JSFunctionSpec controllers_methods[] = {
		{ "getOperationKeyPressed", js_controllers_getOperationKeyPressed, 0, 0, 0 },
		{ "getOperationKeyValue", js_controllers_getOperationKeyValue, 0, 0, 0 },
		{ "getKeyValue", js_controllers_getKeyValue, 0, 0, 0 },
		{ "isConnected", js_controllers_isConnected, 0, 0, 0 },
// #AGTK-NX #AGTK-WIN
#if 1
		{ "isJoyCon", js_controllers_isJoyCon, 0, 0, 0 },
		{ "isSeparatableJoyCon", js_controllers_isSeparatableJoyCon, 0, 0, 0 },
		{ "isJoinableJoyCon", js_controllers_isJoinableJoyCon, 0, 0, 0 },
		{ "setAssinmentJoyCon", js_controllers_setAssignmentJoyCon, 0, 0, 0 },
#endif
		{ NULL },
	};
	JS_DefineFunctions(cx, rcontrollers, controllers_methods);
	{
		JS::RootedValue rval(cx);
		rval.setInt32(InputDataRaw::MAX_GAMEPAD);
		JS_SetProperty(cx, rcontrollers, "MaxControllerId", rval);
	}

	JS::RootedValue rval(cx);
	rval.setObject(*controllers);
	JS_SetProperty(cx, robj, "controllers", rval);
	return true;
}

bool JavascriptManager::addSwitches(JSContext *cx, JSObject *agtkObj)
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
	auto switches = JS_DefineObject(cx, robj, "switches", &switches_class, JS::NullPtr(), JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE);
	CCASSERT(switches, "switches should not be nullptr");
	JS::RootedObject rswitches(cx, switches);
	static JSFunctionSpec switches_methods[] = {
		{ "get", js_switches_get, 0, 0, 0 },
		{ "getIdByName", js_switches_getIdByName, 0, 0, 0 },
		{ NULL },
	};
	JS_DefineFunctions(cx, rswitches, switches_methods);
	{
		JS::RootedValue rval(cx);
		rval.setInt32(agtk::data::kProjectSystemSwitchInit);
		JS_SetProperty(cx, rswitches, "InitId", rval);
		rval.setInt32(agtk::data::kProjectSystemSwitchReset);
		JS_SetProperty(cx, rswitches, "ResetId", rval);
		rval.setInt32(agtk::data::kProjectSystemSwitchSaveFile);
		JS_SetProperty(cx, rswitches, "SaveFileId", rval);
		rval.setInt32(agtk::data::kProjectSystemSwitchLoadFile);
		JS_SetProperty(cx, rswitches, "LoadFileId", rval);
		rval.setInt32(agtk::data::kProjectSystemSwitchCopyFile);
		JS_SetProperty(cx, rswitches, "CopyFileId", rval);
		rval.setInt32(agtk::data::kProjectSystemSwitchDeleteFile);
		JS_SetProperty(cx, rswitches, "DeleteFileId", rval);
		rval.setInt32(agtk::data::kProjectSystemSwitchInitialCamera);
		JS_SetProperty(cx, rswitches, "InitialCameraId", rval);
		rval.setInt32(agtk::data::kProjectSystemSwitchLoadingScene);
		JS_SetProperty(cx, rswitches, "LoadingSceneId", rval);
		rval.setInt32(agtk::data::kProjectSystemSwitchQuitTheGame);
		JS_SetProperty(cx, rswitches, "QuitTheGameId", rval);
		rval.setInt32(agtk::data::kProjectSystemSwitchFileExists);
		JS_SetProperty(cx, rswitches, "FileExistsId", rval);
	}

	JS::RootedValue rval(cx);
	rval.setObject(*switches);
	JS_SetProperty(cx, robj, "switches", rval);

// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_7
	if (!jsb_register_agtk_projectSwitchBase(cx, agtkObj)) return false;
#endif

	return true;
}

bool JavascriptManager::addVariables(JSContext *cx, JSObject *agtkObj)
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
	auto variables = JS_DefineObject(cx, robj, "variables", &variables_class, JS::NullPtr(), JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE);
	CCASSERT(variables, "variables should not be nullptr");
	JS::RootedObject rvariables(cx, variables);
	static JSFunctionSpec variables_methods[] = {
		{ "get", js_variables_get, 0, 0, 0 },
		{ "getIdByName", js_variables_getIdByName, 0, 0, 0 },
		{ NULL },
	};
	JS_DefineFunctions(cx, rvariables, variables_methods);
	{
		JS::RootedValue rval(cx);
		rval.setInt32(agtk::data::kProjectSystemVariablePlayerCount);
		JS_SetProperty(cx, rvariables, "PlayerCountId", rval);
		rval.setInt32(agtk::data::kProjectSystemVariable1PCharacter);
		JS_SetProperty(cx, rvariables, "_1PObjectId", rval);
		rval.setInt32(agtk::data::kProjectSystemVariable2PCharacter);
		JS_SetProperty(cx, rvariables, "_2PObjectId", rval);
		rval.setInt32(agtk::data::kProjectSystemVariable3PCharacter);
		JS_SetProperty(cx, rvariables, "_3PObjectId", rval);
		rval.setInt32(agtk::data::kProjectSystemVariable4PCharacter);
		JS_SetProperty(cx, rvariables, "_4PObjectId", rval);
		rval.setInt32(agtk::data::kProjectSystemVariable1PInstance);
		JS_SetProperty(cx, rvariables, "_1PInstanceId", rval);
		rval.setInt32(agtk::data::kProjectSystemVariable2PInstance);
		JS_SetProperty(cx, rvariables, "_2PInstanceId", rval);
		rval.setInt32(agtk::data::kProjectSystemVariable3PInstance);
		JS_SetProperty(cx, rvariables, "_3PInstanceId", rval);
		rval.setInt32(agtk::data::kProjectSystemVariable4PInstance);
		JS_SetProperty(cx, rvariables, "_4PInstanceId", rval);
		rval.setInt32(agtk::data::kProjectSystemVariable1PController);
		JS_SetProperty(cx, rvariables, "_1PControllerId", rval);
		rval.setInt32(agtk::data::kProjectSystemVariable2PController);
		JS_SetProperty(cx, rvariables, "_2PControllerId", rval);
		rval.setInt32(agtk::data::kProjectSystemVariable3PController);
		JS_SetProperty(cx, rvariables, "_3PControllerId", rval);
		rval.setInt32(agtk::data::kProjectSystemVariable4PController);
		JS_SetProperty(cx, rvariables, "_4PControllerId", rval);
		rval.setInt32(agtk::data::kProjectSystemVariablePortalMoveStartTime);
		JS_SetProperty(cx, rvariables, "PortalMoveStartTimeId", rval);
		rval.setInt32(agtk::data::kProjectSystemVariableFileSlot);
		JS_SetProperty(cx, rvariables, "FileSlotId", rval);
		rval.setInt32(agtk::data::kProjectSystemVariableCopyDestinationFileSlot);
		JS_SetProperty(cx, rvariables, "CopyDestinationFileSlotId", rval);
		rval.setInt32(agtk::data::kProjectSystemVariableMouseX);
		JS_SetProperty(cx, rvariables, "MouseXId", rval);
		rval.setInt32(agtk::data::kProjectSystemVariableMouseY);
		JS_SetProperty(cx, rvariables, "MouseYId", rval);
		rval.setInt32(agtk::data::kProjectSystemVariableBgmVolumeAdjust);
		JS_SetProperty(cx, rvariables, "BgmVolume", rval);
		rval.setInt32(agtk::data::kProjectSystemVariableSeVolumeAdjust);
		JS_SetProperty(cx, rvariables, "SeVolume", rval);
		rval.setInt32(agtk::data::kProjectSystemVariableVoiceVolumeAdjust);
		JS_SetProperty(cx, rvariables, "VoiceVolume", rval);
	}

	JS::RootedValue rval(cx);
	rval.setObject(*variables);
	JS_SetProperty(cx, robj, "variables", rval);

// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_7
	if (!jsb_register_agtk_projectVariableBase(cx, agtkObj)) return false;
#endif

	return true;
}

int32_t JavascriptManager::getInt32(JS::MutableHandleValue &v)
{
	if (v.isInt32()) {
		return v.toInt32();
	}
	return (int32_t)v.toDouble();
}

int32_t JavascriptManager::getInt32(JS::RootedValue &v)
{
	if (v.isInt32()) {
		return v.toInt32();
	}
	return (int32_t)v.toDouble();
}


#if 0
namespace AGTK {
	class VariableMgr {
	public:
		VariableMgr(){
			CCLOG("VariableMgr()");
		}
		~VariableMgr(){
			CCLOG("~VariableMgr()");
		}
		static VariableMgr *create(){
			return new VariableMgr();
		}
		void functionTest();
	};
}


//#include “cocos2d.h”
//#include “cocos2d_specifics.hpp”
// Binding specific object by defining JSClass
static JSClass** agtk_class;
static JSObject* agtk_prototype;

// This function is mapping the function “functionTest”.
bool js_functionTest(JSContext* cx, uint32_t argc, JS::Value* vp)
{
	bool ok = true;
#if 1
	if(argc == 1){
		const char *arg0;
		std::string arg0_tmp;
		ok &= jsval_to_std_string(cx, vp[0], &arg0_tmp);
		arg0 = arg0_tmp.c_str();
		CCLOG("functionTest(%s)", arg0);
		//todo set return value for JS;
		return true;
	}
#else
	JSObject* obj = NULL;
	AGTK::VariableMgr* cobj = NULL;
	obj = JS_THIS_OBJECT(cx, vp);
	js_proxy_t* proxy = jsb_get_js_proxy(obj);
	cobj = (AGTK::VariableMgr*)(proxy ? proxy~~>ptr : NULL);
	JSB_PRECONDITION2;
	if {
		//cobj->functionTest();
		CCLOG("functionTest()");
		JS_SET_RVAL(cx, vp, JSVAL_VOID);
		return ok;
	}
#endif
	JS_ReportError(cx, "Wrong number of arguments");
	return false;
}

bool js_constructor(JSContext* cx, uint32_t argc, JS::Value* vp){
	cocos2d::CCLog("JS Constructor…");
	if (argc > 0) {
		AGTK::VariableMgr* cobj = new AGTK::VariableMgr();
		cocos2d::CCObject* ccobj = dynamic_cast<cocos2d::CCObject *>(cobj);
		if (ccobj) {
			ccobj->autorelease();
		}
		TypeTest t;
		js_type_class_t* typeClass;
		uint32_t typeId = t.s_id();
		HASH_FIND_INT(_js_global_type_ht, &typeId, typeClass);
		assert(typeClass);
		JSObject* obj = JS_NewObject(cx, typeClass->jsclass, typeClass->proto, typeClass->parentProto);
		//JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(obj));
		JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
		args.rval().setObject(*obj);

		js_proxy_t* p = jsb_new_proxy(cobj, obj);
		JS_AddNamedObjectRoot(cx, &p->obj, "AGTK::VariableMgr");

		return true;
	}

	JS_ReportError(cx, "Wrong number of arguments: %d, was expecting: %d", argc, 0);

	return false;
}

// This function is mapping the function “create” when using JavaScript code
bool js_create(JSContext* cx, uint32_t argc, JS::Value* vp){
	cocos2d::CCLog("js is creating...");
	if (argc  0) {
		AGTK::VariableMgr* ret = AGTK::VariableMgr::create();
        JS::Value jsret;
		do{
			if (ret) {
				js_proxy_t* proxy = js_get_or_create_proxy(cx, ret);
				jsret = OBJECT_TO_JSVAL(proxy->obj);
			} else{
				jsret = JSVAL_NULL;
			}
		} while;
		JS_SET_RVAL;
		return false;
	}
	JS_ReportError;
	return false;
}

void js_finalize()
{
	CCLOGINFO("JSBindings: finallizing JS object %p AGTK", obj);
}

// Binding AGTK type
void js_register()
{
	jsb_class = calloc();
	jsb_class->name = "VariableMgr";
	jsb_class->addProperty = JS_PropertyStub;
	jsb_class->delProperty = JS_PropertyStub;
	jsb_class->getProperty = JS_PropertyStub;
	jsb_class->setProperty = JS_StrictPropertyStub;
	jsb_class->enumerate = JS_EnumerateStub;
	jsb_class->resolve = JS_ResolveStub;
	jsb_class->convert = JS_ConvertStub;
	jsb_class->finalize = js_finalize;
	jsb_class->flags = JSCLASS_HAS_RESERVED_SLOTS(2);

	static JSPropertySpec properties[] = {
		{ 0, 0, 0, JSOP_NULLWRAPPER, JSOP_NULLWRAPPER }
	};

	// Binding functionTest function

	static JSFunctionSpec funcs[] = {
		JS_FN(“functionTest”, js_functionTest, 1, JSPROP_PERMANENT | JSPROP_ENUMERATE),
		JS_FS_END
	};

	// Binding create() function

	static JSFunctionSpec st_funcs[] = {
		JS_FN(“create”, js_create, 0, JSPROP_PERMANENT | JSPROP_ENUMERATE),
		JS_FS_END
	};

	// Binding constructor function and prototype
	jsb_prototype = JS_InitClass(
		cx, global,
		NULL,
		jsb_class,
		js_constructor, 0,
		properties,
		funcs,
		NULL,
		st_funcs);
	JSBool found;
	JS_SetPropertyAttributes(cx, global, "AGTK", JSPROP_ENUMERATE | JSPROP_READONLY, &found);

	TypeTest t;
	js_type_class_t* p;
	uint32_t typeId = t.s_id();
	HASH_FIND_INT(_js_global_type_ht, &typeId, p);

	if (!p) {
		p = (js_type_class_t*)malloc(sizeof(_js_global_type_ht));
		p->type = typeId;
		p->jsclass = jsb_class;
		p->proto = jsb_prototype;
		p->parentProto = NULL;
		HASH_ADD_INT(_js_global_type_ht, type, p);
	}
}

// Binding AGTK namespace so in JavaScript code AGTK namespce can be recognized
void register_all(JSContext* cx, JSObject* obj){
    JS::Value nsval;
	JSObject* ns;
	JS_GetProperty(cx, obj, “JS”, &nsval);

	if (nsval == JSVAL_VOID) {
		ns = JS_NewObject(cx, NULL, NULL, NULL);
		nsval = OBJECT_TO_JSVAL(ns);
		JS_SetProperty(cx, obj, "AGTK", &nsval);
	} else{
		JS_ValueToObject(cx, nsval, &ns);
	}
	obj = ns;
	js_register(cx, obj);
}
#endif

#if 0
bool JavascriptManager::initScript(std::string script)
{
	CCASSERT(_initialized, "is not initialized.");
	JSAutoRequest ar(_context);
	//JS::RootedObject global(_context, JS_NewGlobalObject(_context, &global_class, nullptr, JS::FireOnNewGlobalHook));
    //JS::RootedObject global(_context);
	//global = JS_NewGlobalObject(_context, &global_class, nullptr, JS::DontFireOnNewGlobalHool);
	//JS::RootedObject global(_context, _globalObject);
    //JS::HandleObject global = JS::HandleObject::fromMarkedLocation(&_globalObject);
	//JSObject* globalObj = _global.ref().get();
    JS::RootedObject global(_context, _global.ref().get());
	if(!global){
		return false;
	}

	JS::RootedValue rval(_context);
	{
		JSAutoCompartment ac(_context, global);
		if(!JS_InitStandardClasses(_context, global)){
            return false;
        }

		if(!this->registerFunction(global)){
			return false;
		}
		if (!addObject(_context, global)){
			return false;
		}

        script = std::string("Agtk = ") + script;
        CCLOG("script: %s", script.c_str());
		const char *filename = "noname";
		unsigned int lineno = 0;
		bool ok = JS_EvaluateScript((JSContext *)_context, global, script.c_str(), script.length(), (const char*)filename, (unsigned int)lineno, &rval);
		if(!ok){
			return false;
		}
		if(rval.isBoolean() && !rval.toBoolean()){
			return false;
		}
	}
	return true;
}

JS::Value JavascriptManager::evaluateJavaScript(std::string script)
{
#if 0
	//JS::RootedObject global(_context, _globalObject);
	JS::RootedObject global(_context, JS_NewGlobalObject(_context, &global_class, nullptr, JS::FireOnNewGlobalHook));
	if(!global){
		return JS::BooleanValue(false);
	}
	JS::RootedValue rval(_context);
	const char *filename = "noname";
	unsigned int lineno = 0;
	bool ok = JS_EvaluateScript((JSContext *)_context, global, script.c_str(), script.length(), (const char*)filename, (unsigned int)lineno, &rval);
	if (!ok) {
		return JS::BooleanValue(false);
	}
	return rval;
#else
	CCASSERT(_initialized, "is not initialized.");
	JSAutoRequest ar(_context);
	//JS::RootedObject global(_context, JS_NewGlobalObject(_context, &global_class, nullptr, JS::FireOnNewGlobalHook));
    //JS::HandleObject global = JS::HandleObject::fromMarkedLocation(&_globalObject);
	//JS::RootedObject global(_context, _globalObject);
    //auto global = _global.ref().get();
	JS::RootedObject global(_context, _global.ref().get());
	if(!global){
        return JS::BooleanValue(false);
	}

	JS::RootedValue rval(_context);
	{
		JSAutoCompartment ac(_context, global);
		//JS_InitStandardClasses(_context, global);

		//if(!this->registerFunction(global)){
			//return false;
		//}
		//if (!addObject(_context, global)){
			//return false;
		//}

		const char *filename = "noname";
		unsigned int lineno = 0;
		bool ok = JS_EvaluateScript((JSContext *)_context, global, script.c_str(), script.length(), (const char*)filename, (unsigned int)lineno, &rval);
		if(!ok){
            return JS::BooleanValue(false);
		}
	}
	return rval;
#endif
}

//===============================================================================================//
static bool js_playBgm(JSContext *cx, unsigned argc, JS::Value *vp)
{
	JS::CallArgs args = CallArgsFromVp(argc, vp);
	if(argc == 1){
		int bgmId = args[0].toInt32();
		AudioManager::getInstance()->playBgm(bgmId);
		return true;
	}
	return false;
}

static bool js_playSe(JSContext *cx, unsigned argc, JS::Value *vp)
{
	JS::CallArgs args = CallArgsFromVp(argc, vp);
	if(argc == 1){
		int seId = args[0].toInt32();
		AudioManager::getInstance()->playSe(seId);
		return true;
	}
	return false;
}
#endif

bool js_log(JSContext *cx, unsigned argc, JS::Value *vp)
{
	JS::CallArgs args = CallArgsFromVp(argc, vp);
	bool ok = true;
	auto len = args.length();
	auto debugManager = agtk::DebugManager::getInstance();
	std::string line;
	for (unsigned int i = 0; i < len; i++){
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
		auto &v = args[i];
#else
#endif
		if (v.isString()){
			std::string arg0_tmp;
			if (jsval_to_std_string(cx, v, &arg0_tmp)){
				CCLOG("js_log(\"%s\")", arg0_tmp.c_str());
				line += ", " + arg0_tmp;
			}
		} else if (v.isInt32()){
			CCLOG("js_log(int: %d)", v.toInt32());
			line += String::createWithFormat(", %d", v.toInt32())->getCString();
		} else if (v.isDouble()){
			CCLOG("js_log(int: %f)", v.toDouble());
			line += String::createWithFormat(", %lf", v.toDouble())->getCString();
			
		} else if (v.isNull()){
			CCLOG("js_log(null)");
			line += ", null";
		} else if (v.isBoolean()){
			CCLOG("js_log(int: %s)", v.toBoolean() ? "true" : "false");
			line += String::createWithFormat(", %s", (v.toBoolean() ? "true" : "false"))->getCString();
		} else if (v.isUndefined()){
			CCLOG("js_log(undefined)");
			line += ", undefined";
		} else if (v.isObject()){
			CCLOG("js_log(Object)");
			line += ", Object";
		} else {
			CCLOG("js_log(): Unknown value");
			line += ", Unknown value";
			ok = false;
		}
	}
	debugManager->getDebugExecuteLogWindow()->addLog("%s", line.substr(2).c_str());
#if defined(USE_PREVIEW)
	auto fp = GameManager::getScriptLogFp();
	if (fp) {
		fwrite("Agtk: ", 1, 6, fp);
		fwrite(line.substr(2).c_str(), 1, line.size() - 2, fp);
		fwrite("\n", 1, 1, fp);
	}
#endif
	return ok;
}

bool js_reset(JSContext *cx, unsigned argc, JS::Value *vp)
{
	GameManager::getInstance()->restartScene();
	return true;
}

bool js_actionCommands_objectCreate(JSContext *cx, unsigned argc, JS::Value *vp)
{
	JS::CallArgs args = CallArgsFromVp(argc, vp);
	auto len = args.length();
	if (len < 4) {
		args.rval().set(JS::NullValue());
		return false;
	}
#if 1
#if 0
	arg1: <int: オブジェクトID>
	arg2: <int : 生成位置X>
	arg3 : <int : 生成位置Y>
	arg4 : <int : レイヤー番号>
#endif
	int objectId = -1;
	int x = 0;
	int y = 0;
	int layerIndex = -1;

	{
// #AGTK-NX
#if C_TARGET_PLATFORM != CC_PLATFORM_NX
		auto &v2 = args[0];
#else
#endif
		if (!v2.isNumber()) {
			args.rval().set(JS::Int32Value(-1));
			return false;
		}
		objectId = JavascriptManager::getInt32(v2);
	}
	{
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
		auto &v2 = args[1];
#else
#endif
		if (v2.isInt32()) {
			x = v2.toInt32();
		} else if (v2.isDouble()) {
			x = (int)v2.toDouble();
		} else {
			args.rval().set(JS::Int32Value(-1));
			return false;
		}
	}
	{
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
		auto &v2 = args[2];
#else
#endif
		if (v2.isInt32()) {
			y = v2.toInt32();
		}
		else if (v2.isDouble()) {
			y = (int)v2.toDouble();
		}
		else {
			args.rval().set(JS::Int32Value(-1));
			return false;
		}
	}
	{
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
		auto &v2 = args[3];
#else
#endif
		if (!v2.isNumber()) {
			args.rval().set(JS::Int32Value(-1));
			return false;
		}
		layerIndex = JavascriptManager::getInt32(v2);
	}
#else
#if 0
	"arg1: <int: オブジェクトID>
		arg2: <int : 生成位置>
		arg3 : <bool : 接続点を使用>
		arg4 : <int : 接続点ID>
		arg5 : <int : 調整位置X>
		arg6 : <int : 調整位置Y>
		arg7 : <doub : e 生成確率>
		arg8 : <bool : 子オブジェクトとして生成する>
		arg9 : <bool : 生成時のオブジェクトの向きを、このオブジェクトに合わせる>
		arg10 : <bool : このオブジェクトより表示の優先度を下げる>
		arg11 : <bool : 生成したオブジェクトがグリッドに吸着する>"
#endif
	int objectId = -1;
	int createPosition = 0;
	bool useConnect = false;
	int connectId = -1;
	int adjustX = 0;
	int adjustY = 0;
	double probability = 100.0;
	bool childObject = false;
	bool useRotation = false;
	bool lowerPriority = false;
	bool gridMagnet = false;
	auto &v2 = args[0];
	if (!v2.isInt32()) {
		args.rval().set(JS::Int32Value(-1));
		return false;
	}
	objectId = v2.toInt32();
	v2 = args[1];
	if (!v2.isInt32()) {
		args.rval().set(JS::Int32Value(-1));
		return false;
	}
	createPosition = v2.toInt32();
	v2 = args[2];
	if (!v2.isBoolean()) {
		args.rval().set(JS::Int32Value(-1));
		return false;
	}
	useConnect = v2.toBoolean();
	v2 = args[3];
	if (!v2.isInt32()) {
		args.rval().set(JS::Int32Value(-1));
		return false;
	}
	connectId = v2.toInt32();
	v2 = args[4];
	if (!v2.isInt32()) {
		args.rval().set(JS::Int32Value(-1));
		return false;
	}
	adjustX = v2.toInt32();
	v2 = args[5];
	if (!v2.isInt32()) {
		args.rval().set(JS::Int32Value(-1));
		return false;
	}
	adjustY = v2.toInt32();
	v2 = args[6];
	if (!v2.isDouble()) {
		args.rval().set(JS::Int32Value(-1));
		return false;
	}
	probability = v2.toDouble();
	v2 = args[7];
	if (!v2.isBoolean()) {
		args.rval().set(JS::Int32Value(-1));
		return false;
	}
	childObject = v2.toBoolean();
	v2 = args[8];
	if (!v2.isBoolean()) {
		args.rval().set(JS::Int32Value(-1));
		return false;
	}
	useRotation = v2.toBoolean();
	v2 = args[9];
	if (!v2.isBoolean()) {
		args.rval().set(JS::Int32Value(-1));
		return false;
	}
	lowerPriority = v2.toBoolean();
	v2 = args[10];
	if (!v2.isBoolean()) {
		args.rval().set(JS::Int32Value(-1));
		return false;
	}
	gridMagnet = v2.toBoolean();
#endif

	auto projectData = GameManager::getInstance()->getProjectData();
	if (objectId < 0 || !projectData->getObjectData(objectId)) {
		args.rval().set(JS::Int32Value(-1));
		return true;
	}
	auto scene = GameManager::getInstance()->getCurrentScene();
	auto sceneLayer = scene->getSceneLayer(layerIndex + 1);
	if (!sceneLayer) {
		args.rval().set(JS::Int32Value(-1));
		return true;
	}
	auto objectAction = agtk::ObjectAction::createForScript();
	auto instanceId = objectAction->execActionObjectCreateForScript(objectId, x, y, layerIndex + 1);
	args.rval().set(JS::Int32Value(instanceId));
#if 0
	auto projectData = GameManager::getInstance()->getProjectData();
	if (!projectData) {
		args.rval().set(JS::Int32Value(-1));
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
	auto objectIdPtr = dynamic_cast<cocos2d::Integer *>(playerCharacterList->getObjectAtIndex(index2));
	auto objectId = objectIdPtr->getValue();
	args.rval().set(JS::Int32Value(objectId));
#endif
	return true;
}

bool js_actionCommands_objectDestroy(JSContext *cx, unsigned argc, JS::Value *vp)
{
	JS::CallArgs args = CallArgsFromVp(argc, vp);
	auto len = args.length();
	if (len < 1) {
		args.rval().set(JS::NullValue());
		return false;
	}

	int instanceId = -1;

	{
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
		auto &v2 = args[0];
#else
#endif
		if (!v2.isInt32()) {
			args.rval().set(JS::Int32Value(-1));
			return false;
		}
		instanceId = v2.toInt32();
	}
	auto scene = GameManager::getInstance()->getCurrentScene();
	if (instanceId >= 0) {
		agtk::Object *object = scene->getObjectInstance(-1, instanceId);
		if (object) {
			object->removeSelf();
		}
	}
	return true;
}


bool js_controllers_getOperationKeyPressed(JSContext *cx, unsigned argc, JS::Value *vp)
{
	JS::CallArgs args = CallArgsFromVp(argc, vp);
	auto len = args.length();
	if (len < 1) {
		args.rval().set(JS::NullValue());
		return false;
	}
#if 0
	"arg1: <int: コントローラID>
		arg2: <操作キーID>
		ret : <int : キー値>"
#endif
	int controllerId = -1;
	int operationKeyId = -1;
	{
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
		auto &v2 = args[0];
#else
#endif
		if (!v2.isInt32()) {
			args.rval().set(JS::Int32Value(0));
			return false;
		}
		controllerId = v2.toInt32();
	}
	{
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
		auto &v2 = args[1];
#else
#endif
		if (!v2.isInt32()) {
			args.rval().set(JS::Int32Value(0));
			return false;
		}
		operationKeyId = v2.toInt32();
	}

	bool pressed = false;
	auto inputManager = InputManager::getInstance();					// インプットマネージャ
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
	auto controller = dynamic_cast<InputController *>(inputManager->getInputControllerList()->objectForKey(controllerId));
#endif
	if (controller) {
		pressed = controller->isPressed(operationKeyId);
	}
	args.rval().set(JS::BooleanValue(pressed));
	return true;
}

bool js_controllers_getOperationKeyValue(JSContext *cx, unsigned argc, JS::Value *vp)
{
	JS::CallArgs args = CallArgsFromVp(argc, vp);
	auto len = args.length();
	if (len < 1) {
		args.rval().set(JS::NullValue());
		return false;
	}
#if 0
	"arg1: <int: コントローラID>
		arg2: <操作キーID>
		ret : <int : キー値>"
#endif
		int controllerId = -1;
	int operationKeyId = -1;
	{
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
		auto &v2 = args[0];
#else
#endif
		if (!v2.isInt32()) {
			args.rval().set(JS::Int32Value(0));
			return false;
		}
		controllerId = v2.toInt32();
	}
	{
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
		auto &v2 = args[1];
#else
#endif
		if (!v2.isInt32()) {
			args.rval().set(JS::Int32Value(0));
			return false;
		}
		operationKeyId = v2.toInt32();
	}

	float value = 0;
	auto inputManager = InputManager::getInstance();					// インプットマネージャ
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
	auto controller = dynamic_cast<InputController *>(inputManager->getInputControllerList()->objectForKey(controllerId));
#endif
	if (controller) {
		value = controller->getOperationKeyValue(operationKeyId);
	}
	args.rval().set(JS::DoubleValue(value));
	return true;
}

bool js_controllers_getKeyValue(JSContext *cx, unsigned argc, JS::Value *vp)
{
	JS::CallArgs args = CallArgsFromVp(argc, vp);
	auto len = args.length();
	if (len < 1) {
		args.rval().set(JS::NullValue());
		return false;
	}
#if 0
	"arg1: <int: コントローラID>
		arg2: <キーコード>
		ret : <double : 値>"
#endif
	int controllerId = -1;
	int keyCode = -1;
	{
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
		auto &v2 = args[0];
#else
#endif
		if (!v2.isInt32()) {
			args.rval().set(JS::Int32Value(0));
			return false;
		}
		controllerId = v2.toInt32();
	}
	{
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
		auto &v2 = args[1];
#else
#endif
		if (!v2.isInt32()) {
			args.rval().set(JS::Int32Value(0));
			return false;
		}
		keyCode = v2.toInt32();
	}

	float value = 0;
	auto inputManager = InputManager::getInstance();
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
	auto controller = dynamic_cast<InputController *>(inputManager->getInputControllerList()->objectForKey(controllerId));
#endif
	if (controller) {
		value = controller->getValue(keyCode);
	}
	args.rval().set(JS::DoubleValue(value));
	return true;
}

bool js_controllers_isConnected(JSContext *cx, unsigned argc, JS::Value *vp)
{
	JS::CallArgs args = CallArgsFromVp(argc, vp);
	auto len = args.length();
	if (len < 1) {
		args.rval().set(JS::NullValue());
		return false;
	}
#if 0
	arg1: <int : コントローラID>
		ret : <bool : 接続されているか>
#endif
	int controllerId = -1;
	{
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
		auto &v2 = args[0];
#else
#endif
		if (!v2.isInt32()) {
			args.rval().set(JS::Int32Value(0));
			return false;
		}
		controllerId = v2.toInt32();
	}

	bool connected = false;
	auto inputManager = InputManager::getInstance();
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
	auto controller = dynamic_cast<InputController *>(inputManager->getInputControllerList()->objectForKey(controllerId));
#endif
	if (controller) {
		if (controller->getController() == InputController::kControllerPc) {
			connected = true;
		}
		else {
			auto gamepad = inputManager->getGamepad(controller->getGamepadNo());
			if (gamepad && gamepad->getConnected()) {
				connected = true;
			}
		}
	}
	args.rval().set(JS::BooleanValue(connected));
	return true;
}

// #AGTK-NX #AGTK-WIN
#if 1
bool js_controllers_isJoyCon(JSContext *cx, unsigned argc, JS::Value *vp)
{
	JS::CallArgs args = CallArgsFromVp(argc, vp);
	auto len = args.length();
	if (len < 1) {
		args.rval().set(JS::NullValue());
		return false;
	}
	/*
	arg1: <int : コントローラID>
	ret : <bool : Joy-Conであるかどうか>
	*/
	int controllerId = -1;
	{
		auto v2 = args[0];
		if (!v2.isInt32()) {
			args.rval().set(JS::Int32Value(0));
			return false;
		}
		controllerId = v2.toInt32();
	}

	bool isJoyCon = false;
	auto inputManager = InputManager::getInstance();
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
	auto controller = dynamic_cast<InputController *>(inputManager->getInputControllerList()->objectForKey(controllerId));
#endif
	if (controller) {
		if (controller->getController() == InputController::kControllerPc) {
			isJoyCon = false;
		}
		else {
			auto gamepad = inputManager->getGamepad(controller->getGamepadNo());
			if (gamepad && gamepad->getConnected()) {
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
				isJoyCon = false;
#endif
			}
		}
	}
	args.rval().set(JS::BooleanValue(isJoyCon));
	return true;
}

bool js_controllers_isSeparatableJoyCon(JSContext *cx, unsigned argc, JS::Value *vp)
{
	JS::CallArgs args = CallArgsFromVp(argc, vp);
	auto len = args.length();
	if (len < 1) {
		args.rval().set(JS::NullValue());
		return false;
	}
	/*
	arg1: <int : コントローラID>
	ret : <bool : おすそ分け可能なJoy-Conであるかどうか>
	*/
	int controllerId = -1;
	{
		auto v2 = args[0];
		if (!v2.isInt32()) {
			args.rval().set(JS::Int32Value(0));
			return false;
		}
		controllerId = v2.toInt32();
	}

	bool isSeparatable = false;
	auto inputManager = InputManager::getInstance();
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
	auto controller = dynamic_cast<InputController *>(inputManager->getInputControllerList()->objectForKey(controllerId));
#endif
	if (controller) {
		if (controller->getController() == InputController::kControllerPc) {
			isSeparatable = false;
		}
		else {
			auto gamepad = inputManager->getGamepad(controller->getGamepadNo());
			if (gamepad && gamepad->getConnected()) {
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
				isSeparatable = false;
#endif
			}
		}
	}
	args.rval().set(JS::BooleanValue(isSeparatable));
	return true;
}

bool js_controllers_isJoinableJoyCon(JSContext *cx, unsigned argc, JS::Value *vp)
{
	JS::CallArgs args = CallArgsFromVp(argc, vp);
	auto len = args.length();
	if (len < 1) {
		args.rval().set(JS::NullValue());
		return false;
	}
	/*
	arg1: <int : コントローラID>
	ret : <bool : おすそ分けを解除可能なJoy-Conであるかどうか>
	*/
	int controllerId = -1;
	{
		auto v2 = args[0];
		if (!v2.isInt32()) {
			args.rval().set(JS::Int32Value(0));
			return false;
		}
		controllerId = v2.toInt32();
	}

	bool isJoinable = false;
	auto inputManager = InputManager::getInstance();
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
	auto controller = dynamic_cast<InputController *>(inputManager->getInputControllerList()->objectForKey(controllerId));
#endif
	if (controller) {
		if (controller->getController() == InputController::kControllerPc) {
			isJoinable = false;
		}
		else {
			auto gamepad = inputManager->getGamepad(controller->getGamepadNo());
			if (gamepad && gamepad->getConnected()) {
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
				isJoinable = false;
#endif
			}
		}
	}
	args.rval().set(JS::BooleanValue(isJoinable));
	return true;
}

bool js_controllers_setAssignmentJoyCon(JSContext *cx, unsigned argc, JS::Value *vp)
{
	JS::CallArgs args = CallArgsFromVp(argc, vp);
	auto len = args.length();
	if (len < 1) {
		args.rval().set(JS::NullValue());
		return false;
	}
	/*
	arg1: <int : コントローラID>
	arg2: <bool : trueの時おすそ分けを行う。falseの時おすそ分けを解除>
	*/
	int controllerId = -1;
	bool doSeparate = false;
	{
		auto v2 = args[0];
		if (!v2.isInt32()) {
			args.rval().set(JS::Int32Value(0));
			return false;
		}
		controllerId = v2.toInt32();

		auto v3 = args[1];
		if (!v3.isBoolean()) {
			args.rval().set(JS::Int32Value(0));
			return false;
		}
		doSeparate = v3.toBoolean();
	}

	auto inputManager = InputManager::getInstance();
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
	auto controller = dynamic_cast<InputController *>(inputManager->getInputControllerList()->objectForKey(controllerId));
#endif
	if (controller) {
		if (controller->getController() == InputController::kControllerPc) {
			return true;
		}
		else {
			auto gamepad = inputManager->getGamepad(controller->getGamepadNo());
			if (gamepad && gamepad->getConnected()) {
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif
			}
		}
	}
	return true;
}
#endif

// switches
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_7
bool jsb_register_agtk_projectSwitchBase(JSContext *cx, JSObject *agtkObj)
{
	JS::RootedObject robj(cx, agtkObj);

	static JSClass switch_class = {
		"projectSwitchBase",
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
	auto swtch = JS_DefineObject(cx, robj, "projectSwitchBase", &switch_class, JS::NullPtr(), JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE);
	CCASSERT(swtch, "projectSwitchBase should not be nullptr");
	JS::RootedObject rswitch(cx, swtch);
	static JSFunctionSpec switch_methods[] = {
		{ "getValue", js_switch_getValue, 0, 0, 0 },
		{ "setValue", js_switch_setValue, 0, 0, 0 },
		{ NULL },
	};
	JS_DefineFunctions(cx, rswitch, switch_methods);
	return true;
}
#endif

bool js_switches_get(JSContext *cx, unsigned argc, JS::Value *vp)
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
	auto switchId = JavascriptManager::getInt32(v);
	auto projectPlayData = GameManager::getInstance()->getPlayData();
	auto playSwitchData = projectPlayData->getSwitchData(agtk::data::PlayData::COMMON_PLAY_DATA_ID, switchId);
	if (!playSwitchData) {
		args.rval().set(JS::NullValue());
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
	JS::RootedValue rswitchBaseVal(cx);
	JS_GetProperty(cx, jsObj, "projectSwitchBase", &rswitchBaseVal);
	if (!rswitchBaseVal.isObject()) {
		return false;
	}

	JSObject* switchBase = rswitchBaseVal.get().toObjectOrNull();
	JS::RootedObject rswitchBase(cx, switchBase);
#endif
	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_7
	auto swtch = JS_DefineObject(cx, jsthis, "switch", &switch_class, rswitchBase, JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE);
#else
	auto swtch = JS_DefineObject(cx, jsthis, "switch", &switch_class, JS::NullPtr(), JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE);
#endif
	CCASSERT(swtch, "switch should not be nullptr");
	JS::RootedObject rswitch(cx, swtch);
// #AGTK-NX
#ifndef USE_SAR_OPTIMIZE_7
	static JSFunctionSpec switch_methods[] = {
		{ "getValue", js_switch_getValue, 0, 0, 0 },
		{ "setValue", js_switch_setValue, 0, 0, 0 },
		{ NULL },
	};
	JS_DefineFunctions(cx, rswitch, switch_methods);
#endif
	JS::RootedValue rval(cx);
	rval.setInt32(switchId);
	JS_SetProperty(cx, rswitch, "switchId", rval);
	args.rval().set(OBJECT_TO_JSVAL(swtch));
	return true;
}

bool js_switches_getIdByName(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//arg1: <string: スイッチ名>
	//ret: <int : スイッチID>
	JS::CallArgs args = CallArgsFromVp(argc, vp);
	auto len = args.length();
	if (len < 1) {
		args.rval().set(JS::NullValue());
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
			args.rval().set(JS::Int32Value(-1));
			return false;
		}
		if (!jsval_to_std_string(cx, v2, &switchName)) {
			args.rval().set(JS::Int32Value(-1));
			return false;
		}
	}

	int value = -1;
	auto projectPlayData = GameManager::getInstance()->getPlayData();
	
	cocos2d::DictElement *el = nullptr;
	auto switchList = projectPlayData->getCommonSwitchList();
	CCDICT_FOREACH(switchList, el) {
		auto playSwitchData = dynamic_cast<agtk::data::PlaySwitchData *>(el->getObject());
		if (!playSwitchData) continue;
		auto name = playSwitchData->getSwitchData()->getName();
		if (strcmp(name, switchName.c_str()) == 0) {
			value = playSwitchData->getId();
			break;
		}
	}
	args.rval().set(JS::Int32Value(value));
	return true;
}

// switch
bool js_switch_getValue(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//ret: <bool : 値>
	JS::CallArgs args = CallArgsFromVp(argc, vp);

	JS::RootedValue v(cx);
	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
	JS_GetProperty(cx, jsthis, "switchId", &v);
	if (!v.isNumber()) {
		args.rval().set(JS::NullValue());
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
	auto len = args.length();
	if (len < 1) {
		args.rval().set(JS::NullValue());
		return false;
	}
	bool value = 0;
	{
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
		auto &v2 = args[0];
#else
#endif
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
	if (!v.isNumber()) {
		args.rval().set(JS::NullValue());
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

// variables
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_7
bool jsb_register_agtk_projectVariableBase(JSContext *cx, JSObject *agtkObj)
{
	JS::RootedObject robj(cx, agtkObj);

	static JSClass variable_class = {
		"projectVariableBase",
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
	auto variable = JS_DefineObject(cx, robj, "projectVariableBase", &variable_class, JS::NullPtr(), JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE);
	CCASSERT(variable, "variable should not be nullptr");
	JS::RootedObject rvariable(cx, variable);
	static JSFunctionSpec variable_methods[] = {
		{ "getValue", js_variable_getValue, 0, 0, 0 },
		{ "setValue", js_variable_setValue, 0, 0, 0 },
		{ NULL },
	};
	JS_DefineFunctions(cx, rvariable, variable_methods);
	return true;
}
#endif

bool js_variables_get(JSContext *cx, unsigned argc, JS::Value *vp)
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
	auto variableId = JavascriptManager::getInt32(v);
	auto projectPlayData = GameManager::getInstance()->getPlayData();
	auto playVariableData = projectPlayData->getVariableData(agtk::data::PlayData::COMMON_PLAY_DATA_ID, variableId);
	if (!playVariableData) {
		args.rval().set(JS::NullValue());
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
	JS::RootedValue rvariableBaseVal(cx);
	JS_GetProperty(cx, jsObj, "projectVariableBase", &rvariableBaseVal);
	if (!rvariableBaseVal.isObject()) {
		return false;
	}

	JSObject* variableBase = rvariableBaseVal.get().toObjectOrNull();
	JS::RootedObject rvariableBase(cx, variableBase);
#endif
	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_7
	auto variable = JS_DefineObject(cx, jsthis, "variable", &variable_class, rvariableBase, JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE);
#else
	auto variable = JS_DefineObject(cx, jsthis, "variable", &variable_class, JS::NullPtr(), JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE);
#endif
	CCASSERT(variable, "variable should not be nullptr");
	JS::RootedObject rvariable(cx, variable);
// #AGTK-NX
#ifndef USE_SAR_OPTIMIZE_7
	static JSFunctionSpec variable_methods[] = {
		{ "getValue", js_variable_getValue, 0, 0, 0 },
		{ "setValue", js_variable_setValue, 0, 0, 0 },
		{ NULL },
	};
	JS_DefineFunctions(cx, rvariable, variable_methods);
#endif
	JS::RootedValue rval(cx);
	rval.setInt32(variableId);
	JS_SetProperty(cx, rvariable, "variableId", rval);
	args.rval().set(OBJECT_TO_JSVAL(variable));
	return true;
}

bool js_variables_getIdByName(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//arg1: <string: 変数名>
	//ret: <int : 変数ID>
	JS::CallArgs args = CallArgsFromVp(argc, vp);
	auto len = args.length();
	if (len < 1) {
		args.rval().set(JS::NullValue());
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
			args.rval().set(JS::Int32Value(-1));
			return false;
		}
		if (!jsval_to_std_string(cx, v2, &variableName)) {
			args.rval().set(JS::Int32Value(-1));
			return false;
		}
	}

	int value = -1;
	auto projectPlayData = GameManager::getInstance()->getPlayData();

	cocos2d::DictElement *el = nullptr;
	auto variableList = projectPlayData->getCommonVariableList();
	CCDICT_FOREACH(variableList, el) {
		auto playVariableData = dynamic_cast<agtk::data::PlayVariableData *>(el->getObject());
		if (!playVariableData) continue;
		auto name = playVariableData->getVariableData()->getName();
		if (strcmp(name, variableName.c_str()) == 0) {
			value = playVariableData->getId();
			break;
		}
	}
	args.rval().set(JS::Int32Value(value));
	return true;
}

// variable
bool js_variable_getValue(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//ret: <double : 値>
	JS::CallArgs args = CallArgsFromVp(argc, vp);

	JS::RootedValue v(cx);
	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
	JS_GetProperty(cx, jsthis, "variableId", &v);
	if (!v.isNumber()) {
		args.rval().set(JS::NullValue());
		return false;
	}
	auto variableId = JavascriptManager::getInt32(v);

	auto projectPlayData = GameManager::getInstance()->getPlayData();
	auto playVariableData = projectPlayData->getVariableData(agtk::data::PlayData::COMMON_PLAY_DATA_ID, variableId);
	if (playVariableData) {
		auto value = playVariableData->getValue();
		args.rval().set(JS::DoubleValue(value));
	}
	else {
		args.rval().set(JS::NullValue());
	}
	return true;
}

bool js_variable_setValue(JSContext *cx, unsigned argc, JS::Value *vp)
{
	//arg1: <double : 設定値>
	JS::CallArgs args = CallArgsFromVp(argc, vp);
	auto len = args.length();
	if (len < 1) {
		args.rval().set(JS::NullValue());
		return false;
	}
	double value = 0;
	{
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
		auto &v2 = args[0];
#else
#endif
		if (v2.isDouble()) {
			value = v2.toDouble();
		}
		else if (v2.isInt32()) {
			value = (double)v2.toInt32();
		}
		else {
			args.rval().set(JS::NullValue());
			return false;
		}
	}

	JS::RootedValue v(cx);
	JS::RootedObject jsthis(cx, args.thisv().toObjectOrNull());
	JS_GetProperty(cx, jsthis, "variableId", &v);
	if (!v.isNumber()) {
		args.rval().set(JS::NullValue());
		return false;
	}
	auto variableId = JavascriptManager::getInt32(v);

	auto projectPlayData = GameManager::getInstance()->getPlayData();
	auto playVariableData = projectPlayData->getVariableData(agtk::data::PlayData::COMMON_PLAY_DATA_ID, variableId);
	if (playVariableData) {
		playVariableData->setValue(value);
		args.rval().set(JS::TrueValue());
	}
	else {
		args.rval().set(JS::NullValue());
	}
	return true;
}


//===============================================================================================//

bool JavascriptManager::getBoolean(JSContext *cx, JS::RootedObject &rparams, const char *propertyName, bool *pValue)
{
	JS::RootedValue v(cx);
	JS_GetProperty(cx, rparams, propertyName, &v);
	if (v.isUndefined()) {
		return false;
	}
	if (v.isBoolean()) {
		*pValue = v.toBoolean();
		return true;
	}
	if (v.isInt32()) {
		*pValue = v.toInt32() != 0;
		return true;
	}
	if (v.isDouble()) {
		*pValue = v.toDouble() != 0;
		return true;
	}
	if (v.isNullOrUndefined()) {
		*pValue = false;
		return true;
	}
	return false;
}

bool JavascriptManager::getInt32(JSContext *cx, JS::RootedObject &rparams, const char *propertyName, int *pValue)
{
	JS::RootedValue v(cx);
	JS_GetProperty(cx, rparams, propertyName, &v);
	if (v.isUndefined()) {
		return false;
	}
	if (v.isInt32()) {
		*pValue = v.toInt32();
		return true;
	}
	if (v.isDouble()) {
		*pValue = (int)v.toDouble();
		return true;
	}
	if (v.isBoolean()) {
		*pValue = v.toBoolean() ? 1 : 0;
		return true;
	}
	if (v.isNullOrUndefined()) {
		*pValue = 0;
		return true;
	}
	return false;
}

bool JavascriptManager::getDouble(JSContext *cx, JS::RootedObject &rparams, const char *propertyName, double *pValue)
{
	JS::RootedValue v(cx);
	JS_GetProperty(cx, rparams, propertyName, &v);
	if (v.isUndefined()) {
		return false;
	}
	if (v.isDouble()) {
		*pValue = v.toDouble();
		return true;
	}
	if (v.isInt32()) {
		*pValue = v.toInt32();
		return true;
	}
	if (v.isBoolean()) {
		*pValue = v.toBoolean() ? 1 : 0;
		return true;
	}
	if (v.isNullOrUndefined()) {
		*pValue = 0;
		return true;
	}
	return false;
}

bool JavascriptManager::getString(JSContext *cx, JS::RootedObject &rparams, const char *propertyName, cocos2d::String **pValue)
{
	JS::RootedValue v(cx);
	JS_GetProperty(cx, rparams, propertyName, &v);
	if (v.isUndefined()) {
		return false;
	}
	std::string str;
	if (jsval_to_std_string(cx, v, &str)) {
		*pValue = cocos2d::String::create(str.c_str());
		return true;
	}
	if (v.isDouble()) {
		*pValue = cocos2d::String::createWithFormat("%f", v.toDouble());
		return true;
	}
	if (v.isInt32()) {
		*pValue = cocos2d::String::createWithFormat("%d", v.toInt32());
		return true;
	}
	if (v.isBoolean()) {
		*pValue = cocos2d::String::create(v.toBoolean() ? "true" : "false");
		return true;
	}
	if (v.isNull()) {
		*pValue = cocos2d::String::create("null");
		return true;
	}
	if (v.isUndefined()) {
		*pValue = cocos2d::String::create("undefined");
		return true;
	}
	*pValue = cocos2d::String::create(JS_EncodeString(cx, v.toString()));
	return true;
}

bool JavascriptManager::isDefined(JSContext *cx, JS::RootedObject &rparams, const char *propertyName)
{
	JS::RootedValue v(cx);
	JS_GetProperty(cx, rparams, propertyName, &v);
	if (v.isUndefined()) {
		return false;
	}
	return true;
}

void JavascriptManager::forceGC()
{
	auto sc = ScriptingCore::getInstance();
	auto cx = sc->getGlobalContext();
	sc->forceGC(cx, 0, nullptr);
}

#if 0
bool JavascriptManager::registerFunction(JS::HandleObject global)
{
	static JSFunctionSpec _functions[] = {
		JS_FN("playBgm", js_playBgm, 1, 0),
		JS_FN("playSe", js_playSe, 1, 0),
#ifdef USE_PREVIEW	//test js
		JS_FN("log", js_log, 1, 0),
#endif
		JS_FS_END
	};
	bool ret = JS_DefineFunctions(_context, global, _functions);
	if(!ret){
		return false;
	}
	return true;
}
#endif

#if 0//def USE_PREVIEW	//test js
void funcTransition(const JSFunction *func,
	const JSScript *scr,
	const JSContext *const_cx,
	bool entering)
{
	JSContext *cx = const_cast<JSContext*>(const_cx);
	JSString *name = JS_GetFunctionId((JSFunction*)func);
	const char *entExit;
	const char *nameStr;

	/* build a C string for the function's name */

	if (!name) {
		nameStr = "Unnamed function";
	} else {
		nameStr = JS_EncodeString(cx, name);
	}

	/* build a string for whether we're entering or exiting */

	if (entering) {
		entExit = "Entering";
	} else {
		entExit = "Exiting";
	}

	/* output information about the trace */

	printf("%s JavaScript function: %s at time: %ld", entExit, nameStr, clock());
}

void enableTracing(JSContext *cx) {
	JS_SetFunctionCallback(cx, funcTransition);
}

void disableTracing(JSContext *cx) {
	JS_SetFunctionCallback(cx, nullptr);
}
#endif
