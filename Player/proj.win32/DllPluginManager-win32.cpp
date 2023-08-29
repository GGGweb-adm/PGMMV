//#include <windows.h>
#include "jsapi.h"
#include "js/RootingAPI.h"
#include "mozilla/Maybe.h"
#include "scripting/js-bindings/manual/ScriptingCore.h"
#include "DllPluginManager-win32.h"

DllPluginManagerWin32::DllPluginManagerWin32()
: DllPluginManager()
, mDllEntryList()
{
}

DllPluginManagerWin32::~DllPluginManagerWin32()
{
	unloadPlugins();
}

DllPluginManager *DllPluginManager::getInstance()
{
    //CCLOG("%s: %d: %s\n", __FILE__, __LINE__, __FUNCTION__);
	if (!_dllPluginManager) {
		_dllPluginManager = new DllPluginManagerWin32();
	}
	return _dllPluginManager;
}

bool DllPluginManagerWin32::loadPlugins()
{
	CCLOG("%s: %d: %s\n", __FILE__, __LINE__, __FUNCTION__);
	//プロジェクトデータのplugins/win32/*.dllをロードする。
	std::string projectFilePath;
#ifdef USE_RUNTIME
	projectFilePath = FileUtils::getInstance()->getDefaultResourceRootPath() + "plugins/win32/";
#else
	auto gameManager = GameManager::getInstance();
	projectFilePath = gameManager->getProjectPathFromProjectFile(gameManager->getProjectFilePath()->getCString()) + "plugins/win32/";
#endif
	auto list = getDirectoryDllFilenameList(projectFilePath);
	bool ret = true;
	auto scriptingCore = ScriptingCore::getInstance();
	auto context = scriptingCore->getGlobalContext();
	auto _global = scriptingCore->getGlobalObject();
	JS::RootedObject global(context, _global);
	JSAutoCompartment ac(context, global);
	for (auto filename: list) {
		if (!loadPlugin((projectFilePath + filename).c_str(), context, global)) {
			ret = false;
			continue;
		}

	}
	return ret;
}
void DllPluginManagerWin32::unloadPlugins()
{
#ifdef USE_AGTK
	if (ScriptingCore::isCreateInstance()) {
		CCLOG("%s: %d: %s\n", __FILE__, __LINE__, __FUNCTION__);
		auto scriptingCore = ScriptingCore::getInstance();
		auto context = scriptingCore->getGlobalContext();
		auto _global = scriptingCore->getGlobalObject();
		JS::RootedObject global(context, _global);
		JSAutoCompartment ac(context, global);
		for (auto entry : mDllEntryList) {
			entry.finalFunc(context, global);
			auto dll = (HINSTANCE)entry.dll;
			auto code = FreeLibrary(dll);
			CCLOG("%s: %d: %s: %d\n", __FILE__, __LINE__, __FUNCTION__, code);
		}
	}
#else
	CCLOG("%s: %d: %s\n", __FILE__, __LINE__, __FUNCTION__);
	auto scriptingCore = ScriptingCore::getInstance();
	auto context = scriptingCore->getGlobalContext();
	auto _global = scriptingCore->getGlobalObject();
	JS::RootedObject global(context, _global);
	JSAutoCompartment ac(context, global);
	for (auto entry : mDllEntryList) {
		entry.finalFunc(context, global);
		auto dll = (HINSTANCE)entry.dll;
		auto code =
		FreeLibrary(dll);
		CCLOG("%s: %d: %s: %d\n", __FILE__, __LINE__, __FUNCTION__, code);
	}
#endif
	mDllEntryList.clear();

}

std::wstring DllPluginManagerWin32::getWstring(const char *str)
{
	auto len = MultiByteToWideChar(CP_UTF8, 0, str, (int)strlen(str), NULL, 0);
	std::wstring wstr(len, 0);
	MultiByteToWideChar(CP_UTF8, 0, str, (int)strlen(str), &wstr[0], len);
	return wstr;
}

std::string DllPluginManagerWin32::getString(const wchar_t *wstr)
{
	std::string str;
	int len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, nullptr, 0, nullptr, FALSE);
	if (!len) {
		return str;
	}
	str.resize(len + 1);
	WideCharToMultiByte(CP_UTF8, 0, wstr, -1, &str[0], len + 1, nullptr, FALSE);
	return str;
}

#ifdef DLLTEST_EXPORTS
#define DLLTEST_API __declspec(dllexport)
#else
#define DLLTEST_API __declspec(dllimport)
#endif

bool DllPluginManagerWin32::loadPlugin(const char *dllFilename, JSContext *cx, JS::HandleObject object)
{
    CCLOG("%s: %d: %s\n", __FILE__, __LINE__, dllFilename);
	auto wFilename = getWstring(dllFilename);
	auto dll = LoadLibrary(wFilename.c_str());
	if (!dll) {
		CCLOG("LoadLibrary failed: %s", dllFilename);
		return false;
	}
	auto initFunc = (bool(*)(const char *, JSContext *, JS::HandleObject))GetProcAddress(dll, "initialize");
	if (!initFunc) {
		CCLOG("initialize not found");
	}
	auto updateFunc = (void(*)(float, JSContext *, JS::HandleObject))GetProcAddress(dll, "update");
	auto finalFunc = (void(*)(JSContext *, JS::HandleObject))GetProcAddress(dll, "finalize");
	if (!finalFunc) {
		CCLOG("finalize not found");
	}
	if (!initFunc || !finalFunc) {
		FreeLibrary(dll);
		return false;
	}
	auto gameManager = GameManager::getInstance();
	auto code = initFunc(cocos2d::String::createWithFormat("%s %s", gameManager->getAppName()->getCString(), gameManager->getAppVersion()->getCString())->getCString(), cx, object);
	if (!code) {
		CCLOG("%s.initialize returns false, so unloaded.", dllFilename);
		FreeLibrary(dll);
		return false;
	}
	DllEntry entry;
	entry.dll = dll;
	entry.initFunc = initFunc;
	entry.updateFunc = updateFunc;
	entry.finalFunc = finalFunc;
	mDllEntryList.push_back(entry);
	return true;
}

void DllPluginManagerWin32::updatePlugins(float dt)
{
	//CCLOG("%s: %d: %s: %f\n", __FILE__, __LINE__, __FUNCTION__, dt);
	auto scriptingCore = ScriptingCore::getInstance();
	auto context = scriptingCore->getGlobalContext();
	auto _global = scriptingCore->getGlobalObject();
	JS::RootedObject global(context, _global);
	JSAutoCompartment ac(context, global);
	for (auto entry : mDllEntryList) {
		auto updateFunc = entry.updateFunc;
		if (updateFunc) {
			updateFunc(dt, context, global);
		}
	}
}

std::vector<std::string> DllPluginManagerWin32::getDirectoryDllFilenameList(const std::string &dir)
{
	std::vector<std::string> list;

	WIN32_FIND_DATA fd;
	auto hnd = FindFirstFile(getWstring((dir + "*.dll").c_str()).c_str(), &fd);
	if (hnd == INVALID_HANDLE_VALUE) {
		CCLOG("Failed to FindFirstFile: %s", dir.c_str());
		return list;
	}
	do {
		if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
			CCLOG("%s", fd.cFileName);
			if (wcsicmp(fd.cFileName + wcslen(fd.cFileName) - 4, L".dll") == 0) {
				list.push_back(getString(fd.cFileName));
			}
		}
	} while (FindNextFile(hnd, &fd));
	FindClose(hnd);

	return list;
}

