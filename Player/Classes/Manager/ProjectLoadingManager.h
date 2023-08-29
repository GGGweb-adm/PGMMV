#ifndef __PROJECT_LOADING_MANAGER_H__
#define	__PROJECT_LOADING_MANAGER_H__

#include "Lib/Macros.h"

#if defined(USE_BG_PROJECT_LOAD) || defined(USE_PRELOAD_TEX)
#include <thread>
#include <json/document.h>
#include "platform\CCImage.h"


class ProjectLoadingManager
{
private:
	ProjectLoadingManager();
public:
	struct CacheTask
	{
		Image* image;
		std::string key;
	};

	~ProjectLoadingManager();
	static ProjectLoadingManager* getInstance();
	void load(std::string projectFilePath);
	bool isDone();
	void pause();
	bool isPaused();
	void restart();
	void join();
	bool execOneTask();
	void addCacheTask(Image* image, std::string key);
	void preparePreloadTex(rapidjson::Value& json, std::string projectPath);
	void postProcessPreloadTex();
	void postProcessCacheTex();
	void removePreloadTex();

private:
	static ProjectLoadingManager* _projectLoadingManager;
	std::map<std::string, int> _preloadTexKeys;
	CC_SYNTHESIZE_RETAIN(cocos2d::__Dictionary *, _preloadTexList, PreloadTexList);
	std::vector<CacheTask> _cacheTaskList;
};

#endif

#endif // __PROJECT_LOADING_MANAGER_H__