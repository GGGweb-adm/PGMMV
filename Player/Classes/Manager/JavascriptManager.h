#ifndef __JAVASCRIPT_MANAGER_H__
#define	__JAVASCRIPT_MANAGER_H__

#include "cocos2d.h"
#include "jsapi.h"
#include "js/RootingAPI.h"
#include "mozilla/Maybe.h"
#include "Lib/Macros.h"

USING_NS_CC;

/**
 * @brief JavaScriptデータを管理する
 */
class JavascriptManager : public cocos2d::Ref
{
public:
	static bool addObject(JSContext *cx, JS::HandleObject global);
	static void removeObject();
	static bool loadPlugins();
#ifdef USE_PREVIEW
	static bool loadAutoTestPlugins(std::string relativePath, std::string pluginInternalJson, std::string pluginParamValueJson);
#endif
	static void unloadPlugins();
	static void updatePlugins(float dt);
	static void setLocalePlugins(const char *locale);
	static std::string getPluginInternals();
	static void setPluginInternals(const std::string &internals);
	static void forceGC();

	static bool getBoolean(JSContext *cx, JS::RootedObject &rparams, const char *propertyName, bool *pValue);
	static bool getInt32(JSContext *cx, JS::RootedObject &rparams, const char *propertyName, int *pValue);
	static bool getDouble(JSContext *cx, JS::RootedObject &rparams, const char *propertyName, double *pValue);
	static bool getString(JSContext *cx, JS::RootedObject &rparams, const char *propertyName, cocos2d::String **pValue);
	static bool isDefined(JSContext *cx, JS::RootedObject &rparams, const char *propertyName);
	static int32_t getInt32(JS::MutableHandleValue &v);
	static int32_t getInt32(JS::RootedValue &v);

protected:
	static void reportError(JSContext *context, const char *message, JSErrorReport *report);
	static bool addActionCommands(JSContext *cx, JSObject *agtkObj);
	static bool addControllers(JSContext *cx, JSObject *agtkObj);
	static bool addSwitches(JSContext *cx, JSObject *agtkObj);
	static bool addVariables(JSContext *cx, JSObject *agtkObj);

protected:
	static bool _pluginLoaded;
};

#endif	//__JAVASCRIPT_MANAGER_H__
