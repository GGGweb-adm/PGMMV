#ifndef __VIEWER_SCENE_H__
#define __VIEWER_SCENE_H__

#include "Lib/Macros.h"
#include "Lib/BaseLayer.h"

class ViewerScene : public agtk::BaseLayer
{
public:
	ViewerScene();
	virtual ~ViewerScene();
    virtual bool init(int id);
	virtual void onEnter();
	virtual void onEnterTranslationDidFinish();
	virtual void onExit();
public:
    CREATE_FUNC_PARAM(ViewerScene, int, id);
protected:
	virtual void update(float delta);
private:
	CC_SYNTHESIZE(int, _sceneId, SceneId);
	float _counter;
};

#endif // __VIEWER_SCENE_H__
