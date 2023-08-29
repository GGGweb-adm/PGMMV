#ifndef __VIEWPORTLIGHT_H__
#define __VIEWPORTLIGHT_H__

#include "Lib/Macros.h"
#include "Lib/Collision.h"
#include "Data/ObjectData.h"

NS_AGTK_BEGIN

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API TimerColor : public agtk::EventTimer {
private:
	TimerColor();
public:
	CREATE_FUNC_PARAM(TimerColor, cocos2d::Color3B, value);
private:
	virtual bool init(cocos2d::Color3B value);
public:
	cocos2d::Color3B setValue(cocos2d::Color3B value, float seconds = 0.0f);
	cocos2d::Color3B addValue(cocos2d::Color3B value, float seconds = 0.0f);
	cocos2d::Color3B getValue() { return _value; }
	cocos2d::Color3B getPreValue() { return _prevValue; }
	bool isChanged() { return _value != _oldValue ? true : false; }
	void reset();
private:
	cocos2d::Color3B _initValue;
	cocos2d::Color3B _value;
	cocos2d::Color3B _nextValue;
	cocos2d::Color3B _prevValue;
	cocos2d::Color3B _oldValue;
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API ViewportLightTexture : public cocos2d::Texture2D
{
private:
	ViewportLightTexture();
	virtual ~ViewportLightTexture();
public:
	CREATE_FUNC_PARAM(ViewportLightTexture, agtk::data::ObjectViewportLightSettingData *, objectViewportLightSettingData);
	bool checkSameTexture(agtk::data::ObjectViewportLightSettingData *objectViewportLightSettingData);
private:
	virtual bool init(agtk::data::ObjectViewportLightSettingData *objectViewportLightSettingData);
	bool createTexture(agtk::data::ObjectViewportLightSettingData *objectViewportLightSettingData);
private:
	unsigned char *_buffer;
	CC_SYNTHESIZE_RETAIN(agtk::data::ObjectViewportLightSettingData *, _objectViewportLightSettingData, ObjectViewportLightSettingData);
};

//-------------------------------------------------------------------------------------------------------------------
class Object;
class ViewportLight;
class AGTKPLAYER_API ViewportLightSprite : public cocos2d::Sprite
{
private:
	ViewportLightSprite();
	virtual ~ViewportLightSprite();
public:
	CREATE_FUNC_PARAM2(ViewportLightSprite, agtk::Object *, object, agtk::data::ObjectViewportLightSettingData *, objectViewportLightSettingData);
	virtual void update(float delta);
	void updateDarkShader(float intensity);
	agtk::Object *getObject() { return _object; };
	bool intersects(agtk::Object *targetObject);
	void setViewportLightTexture(agtk::ViewportLightTexture *viewportLightTexture);
	void setViewportLight(agtk::ViewportLight *viewportLight);
	bool checkSwitch();
private:
	virtual bool init(agtk::Object *object, agtk::data::ObjectViewportLightSettingData *objectViewportLightSettingData);
private:
	agtk::Object *_object;
	CC_SYNTHESIZE_READONLY(agtk::ViewportLight *, _viewportLight, ViewportLight);
	CC_SYNTHESIZE_READONLY(agtk::ViewportLightTexture *, _viewportLightTexture, ViewportLightTexture);
	CC_SYNTHESIZE_RETAIN(agtk::data::ObjectViewportLightSettingData *, _objectViewportLightSettingData, ObjectViewportLightSettingData);
	CC_SYNTHESIZE_RETAIN(agtk::PolygonShape *, _lightShape, LightShape);
	float _seconds;
	CC_SYNTHESIZE_RETAIN(agtk::TimerColor *, _intensityColor, IntensityColor);
};

//-------------------------------------------------------------------------------------------------------------------
class SceneLayer;
class AGTKPLAYER_API ViewportLightObject : public cocos2d::Ref
{
private:
	ViewportLightObject();
	virtual ~ViewportLightObject();
public:
	static ViewportLightObject *create(agtk::Object *object, agtk::ViewportLight *viewportLight, agtk::SceneLayer *sceneLayer);
	virtual void update(float delta);
	void updateDarkShader(float intensity);
	void visit();
	agtk::Object *getObject() { return _object; };
	agtk::ViewportLightSprite *getViewportLightSprite(int id);
	void removeSprite(agtk::SceneLayer *sceneLayer);
	bool checkSwitch();
	bool containsVisibleViewportLight();
	bool containsVisibleViewportLightSwitch();
private:
	virtual bool init(agtk::Object *object, agtk::ViewportLight *viewportLight, agtk::SceneLayer *sceneLayer);
private:
	agtk::Object *_object;
	agtk::ViewportLight *_viewportLight;
	CC_SYNTHESIZE_RETAIN(cocos2d::__Dictionary *, _viewportLightSpriteList, ViewportLightSpriteList);
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API ViewportLightSceneLayer : public cocos2d::Ref
{
private:
	ViewportLightSceneLayer();
	virtual ~ViewportLightSceneLayer();
public:
	CREATE_FUNC_PARAM2(ViewportLightSceneLayer, agtk::SceneLayer *, sceneLayer, agtk::ViewportLight *, viewportLight);
	virtual void update(float delta);
	agtk::SceneLayer *getSceneLayer() { return _sceneLayer; };
	bool isSwitch();
	bool containsVisibleViewportLightSwitch();
	bool containsVisibleViewportLight();//表示物設定を含んでいるか
	void createRenderTexture();
	void removeRenderTexture();
	void removeObject(agtk::Object *object);
	agtk::ViewportLightObject *getViewportLightObject(agtk::Object *object);
	void createShader(cocos2d::Layer *layer);
	void removeShader();
private:
	virtual bool init(agtk::SceneLayer *sceneLayer, agtk::ViewportLight *viewportLight);
private:
	agtk::SceneLayer *_sceneLayer;
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _viewportLightObjectList, ViewportLightObjectList);
	CC_SYNTHESIZE_RETAIN(cocos2d::RenderTexture *, _renderTexture, RenderTexture);
	bool _isPreviousSwitch;
};

//-------------------------------------------------------------------------------------------------------------------
class Scene;
class AGTKPLAYER_API ViewportLight : public cocos2d::Ref
{
private:
	ViewportLight();
	virtual ~ViewportLight();
public:
	CREATE_FUNC_PARAM(ViewportLight, agtk::Scene *, scene);
private:
	virtual bool init(agtk::Scene *scene);
public:
	virtual void update(float delta);
	void createViewportLight();
	void removeViewportLight();
	void removeShader();
	agtk::Scene *getScene() { return _scene; };
	agtk::ViewportLightSceneLayer *getViewportLightSceneLayer(int id);
	agtk::ViewportLightTexture *getViewportLightTexture(agtk::data::ObjectViewportLightSettingData *objectViewportLightSettingData);
private:
	agtk::Scene *_scene;
	CC_SYNTHESIZE_RETAIN(cocos2d::__Dictionary *, _viewportLightSceneLayerList, ViewportLightSceneLayerList);
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _viewportLightTextureList, ViewportLightTextureList);
	CC_SYNTHESIZE(cocos2d::Layer *, _baseLayer, BaseLayer);
};

NS_AGTK_END

#endif // __VIEWPORTLIGHT_H__
