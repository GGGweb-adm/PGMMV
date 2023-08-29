#ifndef __DLL_PLUGIN_MANAGER_H__
#define	__DLL_PLUGIN_MANAGER_H__

#include "cocos2d.h"
#include "GameManager.h"

USING_NS_CC;

class DllPluginManager : public cocos2d::Ref
{
protected:
	DllPluginManager();
	virtual ~DllPluginManager();
	static DllPluginManager *_dllPluginManager;
public:
	static DllPluginManager *getInstance();
	static void purge();
public:
	virtual bool loadPlugins() = 0;
	virtual void unloadPlugins() = 0;
	virtual void updatePlugins(float dt) = 0;
	//virtual bool loadPlugin(const char *dllFilename) = 0;
};

#endif	//__DLL_PLUGIN_MANAGER_H__
