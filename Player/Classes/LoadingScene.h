#ifndef __LOADING_SCENE_H__
#define __LOADING_SCENE_H__

#include "Lib/Macros.h"
#include "Lib/BaseLayer.h"

class AGTKPLAYER_API LoadingScene : public agtk::BaseLayer
{
public:
	LoadingScene();
	virtual ~LoadingScene();
    virtual bool init();
public:
	void changeScene();
    CREATE_FUNC(LoadingScene);
protected:
	virtual void update(float delta);
};

#endif // __LOADING_SCENE_H__
