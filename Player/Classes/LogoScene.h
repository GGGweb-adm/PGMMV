#ifndef __LOGO_SCENE_H__
#define __LOGO_SCENE_H__

#include "cocos2d.h"
#include "Lib/Macros.h"
#include "Manager/InputManager.h"

class LogoScene : public cocos2d::Layer, public InputEventListener
{
public:
#ifdef USE_BG_PROJECT_LOAD
	static cocos2d::Scene* createScene(const std::string &locale, const std::string &projectFilePath);
#else
	static cocos2d::Scene* createScene(const std::string &locale);
#endif

	LogoScene();
    virtual bool init(const std::string &locale);
	virtual void onEnter();
	virtual void update(float delta);

	CREATE_FUNC_PARAM(LogoScene, const std::string &, locale);

protected:
	bool mLeaving;
// #AGTK-NX
#ifdef USE_LOGO_ACT2_6098
	int phase;
	Sprite* gggSprite;
	Sprite* logoSprite;
	cocos2d::Size orgDesignSize;
	ResolutionPolicy orgPolicy;
	float orgFrameZoomFactor;
#ifdef USE_BG_PROJECT_LOAD
	Sprite* loadingSprite1;
	Sprite* loadingSprite2;
	float loadingDelta;
	int loadingFrameIndex;
	std::string _projectFilePath;
	bool _loadStarted;

	void ApplyProjectData();
#endif
#endif
};

#endif // __LOGO_SCENE_H__
