#ifndef __PARTICLE_MANAGER_H__
#define	__PARTICLE_MANAGER_H__

#include "cocos2d.h"
#include "json/document.h"
#include "Data/ProjectData.h"
#include "Lib/Macros.h"
#include "Lib/Particle.h"

USING_NS_CC;

class GameManager;

//! パーティクルをテクスチャーにレンダリングするクラス
class AGTKPLAYER_API ParticleRenderTexture : public cocos2d::RenderTexture
{
public:
	ParticleRenderTexture();
	virtual ~ParticleRenderTexture();
	static ParticleRenderTexture * create(int w, int h);
	void capture(std::function<void(Image *, cocos2d::Node *node)> callback = nullptr);

protected:
	void onCapture();

protected:
	CustomCommand _captureCommand;
	std::function<void(Image *, cocos2d::Node *node)> _captureCallback;
	CC_SYNTHESIZE(cocos2d::Node *, _node, Node);
	CC_SYNTHESIZE(int, _width, Width);
	CC_SYNTHESIZE(int, _height, Height);
};

//! パーティクルを管理するクラス
class AGTKPLAYER_API ParticleManager : public cocos2d::Ref
{
private:
	ParticleManager();
	static ParticleManager *_particleManager;

public:
	virtual ~ParticleManager();
	static ParticleManager* getInstance();
	static void purge();
// #AGTK-NX
#if CC_TARGET_PLATFORM == CC_PLATFORM_NX
#else
	bool ParticleManager::init();
#endif
#ifdef USE_PREVIEW
	void runCommand(const rapidjson::Value &json);
	void update(float delta, GameManager *gameManager);
#endif
	void update(float delta);
	void setCapture(int offset, int size);

	bool existsParticleGroup(agtk::ParticleGroup* particle);
	void setVisible(int layerId, bool visible);
	cocos2d::Vec2 getScreenCenterPos(float delta);
	void setAlpha(cocos2d::Node *sceneLayer, float alpha);
#ifdef USE_PREVIEW
	void clearPreview();
#endif

public:
#ifdef USE_REDUCE_RENDER_TEXTURE
	agtk::ParticleGroup * addParticle(cocos2d::Node *followTarget, int sceneId, int sceneLayerId, int particleId, const cocos2d::Vec2 pos, int connectionId, float duration300, bool durationUnlimited, const cocos2d::__Dictionary * particleDataList, bool bForceBack, cocos2d::Node* objectNode = nullptr);// パーティクル追加
#else
	agtk::ParticleGroup * addParticle(cocos2d::Node *followTarget, int sceneId, int sceneLayerId, int particleId, const cocos2d::Vec2 pos, int connectionId, float duration300, bool durationUnlimited, const cocos2d::__Dictionary * particleDataList, cocos2d::Node* objectNode = nullptr);// パーティクル追加
#endif
#ifdef USE_REDUCE_RENDER_TEXTURE
	class ParticleGroupBackup : public cocos2d::Ref
	{
	public:
		ParticleGroupBackup();
		virtual ~ParticleGroupBackup();
		static ParticleGroupBackup *create(agtk::ParticleGroup *group);
		void init(agtk::ParticleGroup *group);
		int _particleId;
		cocos2d::Vec2 _pos;
		int _connectionId;
		float _duration300;
		bool _durationUnlimited;
		bool _forceBack;
	};
	agtk::ParticleGroup *addParticle(cocos2d::Node *followTarget, int sceneId, int sceneLayerId, ParticleGroupBackup *backup);// パーティクル追加
#endif
	void removeParticlesOfFollowed(const cocos2d::Node * dest, int targetParticleId);
	void removeAllParticles();
	void removeParticle(agtk::ParticleGroup *particleGroup);
	void stopEmitteParticlesOfFollowed(const cocos2d::Node * dest, int targetParticleId, bool isReset, bool bFollowedNull = false);
	void detachParticlesOfFollowed(const cocos2d::Node *dest, int targetParticleId);
	void addRemoveParticlesOfFollowed(const cocos2d::Node *dest, int targetParticleId, bool bAdd);
	cocos2d::__Array *getParticleArray(cocos2d::Node *object);
#ifdef USE_REDUCE_RENDER_TEXTURE
	cocos2d::__Array *getParticleGroupBackupList(cocos2d::Node *object);
#endif

private:
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _particleGroupList, ParticleGroupList);//パーティクルグループリスト
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _disabledLayerIdList, DisabledLayerIdList);

private:
	CC_SYNTHESIZE_RETAIN(cocos2d::Array *, _layerList, LayerList);
	CC_SYNTHESIZE_RETAIN(cocos2d::Array *, _boundingBoxList, BoundingBoxList);
	CC_SYNTHESIZE_RETAIN(ParticleRenderTexture *, _renderTexture, RenderTexture);
	CC_SYNTHESIZE_RETAIN(cocos2d::RenderTexture *, _sceneLayerRenderTexture, SceneLayerRenderTexture);
	CC_SYNTHESIZE(bool, _capture, Capture);
	CC_SYNTHESIZE(int, _captureOffset, CaptureOffset);
	CC_SYNTHESIZE(int, _captureSize, CaptureSize);
	CC_SYNTHESIZE(bool, _showBox, ShowBox);
	CC_SYNTHESIZE(int, _layerIndex, LayerIndex);
};

#endif	//__PARTICLE_MANAGER_H__
