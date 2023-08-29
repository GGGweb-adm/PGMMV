#ifndef __PARTICLE_H__
#define	__PARTICLE_H__

#include "Lib/Macros.h"
#include "Data/ProjectData.h"
//#include "collision/CollisionDetaction.hpp"
#include "Data/AnimationData.h"
#include "json/document.h"

NS_AGTK_BEGIN
//-------------------------------------------------------------------------------------------------------------------
//! パーティクルグループクラス
//  note: パーティクルをグルーピングしたクラス
class ObjectMovement;
class AGTKPLAYER_API ParticleGroup : public cocos2d::Node
{
public:
	// パーティクル処理タイプ
	enum PARTICLE_PROC_TYPE
	{
		PLAY,
		STOP,
		PAUSE,
		RESET
	};
private:
	ParticleGroup();
	virtual ~ParticleGroup();
public:
#ifdef USE_REDUCE_RENDER_TEXTURE
	static ParticleGroup *create(cocos2d::Node *followTarget, int sceneId, int sceneLayerId, int particleId, const cocos2d::Vec2 pos, int connectionId, float duration300, bool durationUnlimited, const cocos2d::__Dictionary * particleDataList, bool bForceBack, cocos2d::Node* objectNode = nullptr);
#else
	static ParticleGroup *create(cocos2d::Node *followTarget, int sceneId, int sceneLayerId, int particleId, const cocos2d::Vec2 pos, int connectionId, float duration300, bool durationUnlimited, const cocos2d::__Dictionary * particleDataList, cocos2d::Node* objectNode = nullptr);
#endif
protected:
#ifdef USE_REDUCE_RENDER_TEXTURE
	virtual bool init(cocos2d::Node *followTarget, int sceneId, int sceneLayerId, int particleId, const cocos2d::Vec2 pos, int connectionId, float duration300, bool durationUnlimited, const cocos2d::__Dictionary * particleDataList, bool bForceBack, cocos2d::Node* objectNode = nullptr);
#else
	virtual bool init(cocos2d::Node *followTarget, int sceneId, int sceneLayerId, int particleId, const cocos2d::Vec2 pos, int connectionId, float duration300, bool durationUnlimited, const cocos2d::__Dictionary * particleDataList, cocos2d::Node* objectNode = nullptr);
#endif
public:
	virtual void update(float dt) override;
	void changeProccess(PARTICLE_PROC_TYPE procType, bool isReset = true);
	bool isStopped();
	bool isActive();
	void remove();
	bool isUnlimited();
	void deleteParticle();
#ifdef USE_REDUCE_RENDER_TEXTURE
	virtual void visit(Renderer *renderer, const Mat4& parentTransform, uint32_t parentFlags);
#endif
#ifdef USE_REDUCE_RENDER_TEXTURE
	void updateBackside();
#endif

	// 残像用シェーダの設定を行う
	void setShaderAfterimage();
	// 残像用シェーダのカラーを設定
	void setAfterimageColor(cocos2d::Color4B color);
	// 残像用シェーダのαを設定
	void setAfterimageAlpha(float alpha);
private:
	CC_SYNTHESIZE(int, _id, Id);// パーティクルグループID
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _layerList, LayerList);//パーティクルレイヤーリスト
	CC_SYNTHESIZE(float, _duration300, Duration300);// 継続時間
	CC_SYNTHESIZE(bool, _isDurationUnlimited, IsDurationUnlimited);// 継続時間の無制限フラグ
	CC_SYNTHESIZE(int, _sceneId, SceneId);// 所属するシーンID
	CC_SYNTHESIZE(int, _sceneLayerId, SceneLayerId);// 所属するシーンレイヤーID
	CC_SYNTHESIZE_RETAIN(cocos2d::Node *, _followTarget, FollowTarget);//追従対象
	CC_SYNTHESIZE(cocos2d::Vec2, _followAdjustPos, FollowAdjustPos);//追従用補正座標
	CC_SYNTHESIZE(int, _connectionId, ConnectionId);//接続点ID
	CC_SYNTHESIZE(cocos2d::Vec2, _positionOffset, PositionOffset);//座標オフセット
	CC_SYNTHESIZE(bool, _targetObjectBackside, TargetObjctBackside);//「画像の裏側に設定」フラグ

	CC_SYNTHESIZE(cocos2d::GLProgramState *, _shaderStateAfterimage, ShaderStateAfterimage); // 残像用シェーダ
	CC_SYNTHESIZE(bool, _pauseEmissionFlag, PauseEmissionFlag);//パーティクル生成一時停止フラグ
	CC_SYNTHESIZE_RETAIN(agtk::data::SceneData *, _sceneData, SceneData);
#ifdef USE_REDUCE_RENDER_TEXTURE
	CC_SYNTHESIZE(bool, _forceBack, ForceBack);//表示優先度をオブジェクトの奥側にに強制
#endif
};

//-------------------------------------------------------------------------------------------------------------------
//! パーティクルを処理するクラス
class AGTKPLAYER_API Particle : public cocos2d::ParticleSystemQuad
{
private:
	const int TEXTURE_ID_NONE = -9999;// テクスチャIDデフォルト値
	const int SOUND_ID_NONE = -1;// サウンドIDデフォルト値
	const int VOICE_ID_NONE = -1;// ボイスIDデフォルト値
	const float INCREASE_REPULTION = 1.5f;//反発係数強化割合
public:
	static Particle *create(int seed);
	static Particle *create(int seed, agtk::data::AnimeParticleData *animeParticleData, cocos2d::Dictionary *animParticleImageList, cocos2d::Node* objectNode = nullptr);

CC_CONSTRUCTOR_ACCESS:
	Particle(int seed);
	virtual ~Particle();
	bool init() override { return initWithTotalParticles(250); }
	bool initWithAnimeParticleData(agtk::data::AnimeParticleData *animeParticleData, cocos2d::Dictionary *animParticleImageList, cocos2d::Node* objectNode = nullptr);
	virtual void play();
	void pause();
	virtual void stop(bool isReset);
	virtual void reset();
	void setParameters(const rapidjson::Value &json, bool boundingBox);
	virtual void update(float dt) override;
	virtual void onEnter() override;
	//virtual void draw(Renderer *renderer, const Mat4 &transform, uint32_t flags) override;
#ifdef USE_REDUCE_RENDER_TEXTURE
	virtual void visit(Renderer *renderer, const Mat4& parentTransform, uint32_t parentFlags);
#endif

	void deleteParticle();
	void setAlpha(float alpha);

protected:
	std::string getTextureKey(int id);
	cocos2d::Texture2D *getTexture(int id, const char *base64, bool isPreview = false);
	void removeTexture(int id);
	void procInitialEmit();// 事前発生、同時発生処理
	void getParticleBound(cocos2d::Vec2 sceneSize, Point *boundMin, Point *boundMax);// パーティクルのバウンディング取得
	void calcCollision(cocos2d::Node *target, ObjectMovement *objectMovement, int wallBit, float dt);// パーティクルの衝突処理
	bool checkCollision(float dt);//パーティクルの衝突処理
	void updateParticleData(int idx, const cocos2d::Vec2 &crossPos, const cocos2d::Vec2 &reflection);
	bool isActuallyPreviousEmit();
protected:
	CC_SYNTHESIZE(int, _id, Id);
	CC_SYNTHESIZE(int, _textureId, TextureId);
	CC_SYNTHESIZE(bool, _paused, Paused);
	CC_SYNTHESIZE(bool, _stopped, Stopped);
	CC_SYNTHESIZE(int, _duration300, Duration300);
	CC_SYNTHESIZE(bool, _loop, Loop);
	CC_SYNTHESIZE(bool, _instantEmit, InstantEmit);
	CC_SYNTHESIZE(bool, _previousEmit, PreviousEmit);
	CC_SYNTHESIZE(float, _gravityDirection, GravityDirection);
	CC_SYNTHESIZE(float, _gravity, Gravity);
	CC_SYNTHESIZE(bool, _isCheck, IsCheck);// 判定の有無
	CC_SYNTHESIZE(float, _repulsion, Repulsion);// 反発係数
	CC_SYNTHESIZE(bool, _touchBound, TouchBound);// 壁や当たり判定で跳ねるか？
	CC_SYNTHESIZE(bool, _touchDisappear, TouchDisappear);// 壁や当たり判定で消えるか？
	CC_SYNTHESIZE(int, _disappearCount, DisappearCount);// 壁や当たり判定で消えるまでのヒット回数
	CC_SYNTHESIZE(bool, _isDisableAntiAlias, IsDisableAntiAlias);// アンチエイリアス無効の有無
	CC_SYNTHESIZE(std::string, _base64Cache, Base64Cache);// base64エンコードされた画像データキャッシュ

	CC_SYNTHESIZE(bool, _isPlaySe, IsPlaySe);// サウンド再生の有無
	CC_SYNTHESIZE(int, _playSeId, PlaySeId);// 再生するサウンドID
	CC_SYNTHESIZE(bool, _isPlayVoice, IsPlayVoice);// 音声再生の有無
	CC_SYNTHESIZE(int, _playVoiceId, PlayVoiceId);// 再生する音声ID
#ifdef USE_PREVIEW
	CC_SYNTHESIZE(cocos2d::Vec2, _emitPos, EmitPos);
#endif
	CC_SYNTHESIZE(int, _sceneId, SceneId);//シーンID
	CC_SYNTHESIZE(int, _sceneLayerId, SceneLayerId);//シーンレイヤーID
	CC_SYNTHESIZE_RETAIN(agtk::data::AnimeParticleData *, _animeParticleData, AnimeParticleData);
	float _alpha;
#ifdef USE_PREVIEW
	CC_SYNTHESIZE(double, _lastDirection, LastDirection);
	CC_SYNTHESIZE(double, _lastEndRadius, LastEndRadius);
#endif
	CC_SYNTHESIZE(cocos2d::Node *, _parentBeforeRemoveChild, ParentBeforeRemoveChild);// removeChild()される直前のparentノード。

protected:
	CC_DISALLOW_COPY_AND_ASSIGN(Particle);

	CC_SYNTHESIZE(cocos2d::Node*, _objectNode, objectNode);
};

NS_AGTK_END

#endif	//__PARTICLE_H__
