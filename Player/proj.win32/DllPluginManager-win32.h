#ifndef __DLL_PLUGIN_MANAGER_WIN32_H__
#define	__DLL_PLUGIN_MANAGER_WIN32_H__

#include <vector>
#include "cocos2d.h"
#include "Manager/DllPluginManager.h"

USING_NS_CC;

class DllPluginManagerWin32 : public DllPluginManager
{
	friend class DllPluginManager;
protected:
	DllPluginManagerWin32();
	std::vector<std::string> getDirectoryDllFilenameList(const std::string &dir);
	std::wstring getWstring(const char *str);
	std::string getString(const wchar_t *str);
public:
	~DllPluginManagerWin32();
	virtual bool loadPlugins();
	virtual void unloadPlugins();
	virtual void updatePlugins(float dt);
	bool loadPlugin(const char *dllFilename, JSContext *cx, JS::HandleObject object);
public:
	class DllEntry {
	public:
		void *dll;
		bool(*initFunc)(const char *version, JSContext *cx, JS::HandleObject object);
		void(*updateFunc)(float dt, JSContext *cx, JS::HandleObject object);
		void(*finalFunc)(JSContext *cx, JS::HandleObject object);
	};
	std::vector<DllEntry> mDllEntryList;
};

#endif	//__DLL_PLUGIN_MANAGER_WIN32_H__
