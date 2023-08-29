#ifndef __RENDER_TEXTURE_H__
#define	__RENDER_TEXTURE_H__

#include "Lib/Macros.h"
#include "Lib/Shader.h"

NS_AGTK_BEGIN

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API RenderTexture: public cocos2d::RenderTexture
{
public:
	RenderTexture();
	virtual ~RenderTexture();
public:
	static RenderTexture *create(int w, int h, cocos2d::Texture2D::PixelFormat format = cocos2d::Texture2D::PixelFormat::RGBA8888, bool withDepthStencilBuffer = false);
	void update(float delta, cocos2d::Node *node, cocos2d::Mat4 m);
	virtual void update(float delta);
	const cocos2d::Size& getContentSize();
	cocos2d::Node *getRenderTextureNode();
	void addRenderTexture(RenderTexture *renderTexture);
	void removeRenderTexture(RenderTexture *renderTexture);
	void removeAllRenderTexture();
	agtk::Shader *addShader(Shader::ShaderKind kind, float value, float seconds = 0.0f);
	void pauseShader() { if (this->getShader()) this->getShader()->pause(); }
	void resumeShader() { if (this->getShader()) this->getShader()->resume(); }
	void reset();
private:
	cocos2d::Layer *getLayer();
private:
	bool init(int w, int h, cocos2d::Texture2D::PixelFormat format, bool withDepthStencilBuffer);
	cocos2d::__Array *_renderTextureArray;
private:
	CC_SYNTHESIZE(bool, _ignoredLayerPosition, IgnoredLayerPosition);
	CC_SYNTHESIZE_RETAIN(agtk::Shader *, _shader, Shader);
};

//-------------------------------------------------------------------------------------------------------------------
class SceneBackground;
class SceneLayer;
class SceneTopMost;
class AGTKPLAYER_API RenderTextureCtrl : public cocos2d::Node
{
public:
	enum EnumType {
		kTypeNormal,
		kTypeSceneBackground,
		kTypeSceneLayer,
		kTypeSceneTopMost,
	};
public:
	RenderTextureCtrl();
	virtual ~RenderTextureCtrl();
public:
	static RenderTextureCtrl *create(cocos2d::Node *node, cocos2d::Size renderTextureSize, int zOrder = 0, bool withDepthStencilBuffer = false);
	static RenderTextureCtrl *createForSceneLayer(agtk::SceneLayer *sceneLayer, cocos2d::Size renderTextureSize, int zOrder = 0);
	static RenderTextureCtrl *createForSceneBackground(agtk::SceneBackground *sceneBackground, cocos2d::Size renderTextureSize, int zOrder = 0);
	static RenderTextureCtrl *createForSceneTopMost(agtk::SceneTopMost *sceneTopMost, cocos2d::Size renderTextureSize, int zOrder = 0);
	virtual void update(float delta, cocos2d::Mat4 *viewMatrix = nullptr, cocos2d::Camera * viewCamera = nullptr);
public:
	void setClearColor(cocos2d::Color4F color);
	cocos2d::Color4F getClearColor();
	agtk::RenderTexture *getLastRenderTexture();
	agtk::RenderTexture *getFirstRenderTexture();
	agtk::RenderTexture *addRenderTexture(cocos2d::Size size, bool withDepthStencilBuffer = false);
	agtk::Shader *addShader(Shader::ShaderKind kind, float value, float seconds = 0.0f);
	agtk::Shader *addShader(Shader::ShaderKind kind, cocos2d::Size sizse, float value, float seconds = 0.0f);
	bool isUseShader();
	bool isShader(Shader::ShaderKind kind);
	void removeShader(Shader::ShaderKind kind);
	void removeShader(agtk::Shader *shader);
	void removeShaderAll();
	agtk::Shader *getShader(Shader::ShaderKind kind);
	int getShaderCount();
	void pauseShader();
	void resumeShader();
private:
	virtual bool init(cocos2d::Node *node, cocos2d::Size renderTextureSize, int zOrder, bool withDepthStencilBuffer);
	bool initForSceneLayer(agtk::SceneLayer *sceneLayer, cocos2d::Size renderTextureSize, int zOrder);
	bool initForSceneBackground(agtk::SceneBackground *sceneBackground, cocos2d::Size renderTextureSize, int zOrder);
	bool initForSceneTopMost(agtk::SceneTopMost *sceneTopMost, cocos2d::Size renderTextureSize, int zOrder);
private:
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _renderTextureList, RenderTextureList);
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _shaderList, ShaderList);
	CC_SYNTHESIZE_RETAIN(cocos2d::Node *, _baseNode, BaseNode);
	CC_SYNTHESIZE(cocos2d::Size, _renderTextureSize, RenderTextureSize);
	cocos2d::Color4F _clearColor;
	CC_SYNTHESIZE(EnumType, _type, Type);
	CC_SYNTHESIZE_RETAIN(cocos2d::Layer *, _layer, Layer);

	// --------------------------------------------------------------------------------
	// 重なり演出用
public:
	virtual void update(float delta, cocos2d::Mat4 *viewMatrix, cocos2d::__Array *maskList, cocos2d::__Array *objList, cocos2d::__Array *tileMapList, bool ignoreVisibleObject);
	virtual void updateTopMost(float delta, cocos2d::Mat4 *viewMatrix = nullptr, cocos2d::Camera * viewCamera = nullptr);
	virtual void updateTopMostWithMenu(float delta, cocos2d::Mat4 *viewMatrix = nullptr, cocos2d::Camera * viewCamera = nullptr);
	virtual void updateTopMostWithMenuIgnoreMenu(float delta, cocos2d::Mat4 *viewMatrix = nullptr, cocos2d::Camera * viewCamera = nullptr);
	virtual void updateTopMostWithMenuOnlyMenu(float delta, cocos2d::Mat4 *viewMatrix = nullptr, cocos2d::Camera * viewCamera = nullptr);
	void updateIsRenderTile(cocos2d::__Array *objList, bool ignoreVisibleObject);
	void updateIsRenderObj(cocos2d::__Array *objList, bool ignoreVisibleObject);

	CC_SYNTHESIZE_RETAIN(RenderTexture *, _tileRenderTexture, TileRenderTexture);
	CC_SYNTHESIZE_RETAIN(RenderTexture *, _objRenderTexture, ObjRenderTexture);

	CC_SYNTHESIZE(bool, _isRenderTile, IsRenderTile);
	CC_SYNTHESIZE(bool, _isRenderObject, IsRenderObject);
};

NS_AGTK_END

#endif	//__RENDER_TEXTURE_H__
