#ifndef __SS_PLAYER_H__
#define	__SS_PLAYER_H__

#include "Lib/Macros.h"
#include "Lib/Player/BasePlayer.h"
#include "Lib/Animation.h"
#include "Data/ProjectData.h"
#include "Manager/PrimitiveManager.h"

#include "External/SSPlayer/SS6Player.h"
#include "External/SSPlayer/SS6PlayerData.h"

NS_AGTK_BEGIN

/**
 * @brief SpriteStudioアニメの表示を管理するクラス。
 */
class SSPlayer : public ss::SSPlayerControl, public virtual agtk::BasePlayer
{
private:
	SSPlayer();
	virtual ~SSPlayer();
public:
	static SSPlayer *createWithFilename(std::string path, ss::ResourceManager* resman = nullptr);
	static SSPlayer *createWithAnimationData(agtk::data::AnimationData *animationData, agtk::data::ResourceInfoData *resourceInfoData, ss::ResourceManager* resman = nullptr);
	virtual void update(float dt);
	virtual void play(const std::string& animeName, int loop = 0, int startFrameNo = 0);
	virtual void play(int actionNo, int actionDirectNo, float seconds = 0.0f, bool bIgnoredSound = false, bool bReverse = false);
	const char *getFilename();
	virtual void draw(cocos2d::Renderer *renderer, const cocos2d::Mat4 &transform, uint32_t flags);
	virtual void onDraw(cocos2d::Renderer *renderer, const cocos2d::Mat4 &transform, uint32_t flags);
	virtual void onRenderingDraw(cocos2d::Renderer *renderer, const cocos2d::Mat4 &transform, uint32_t flags);
	bool createTexture2D(int width, int height);
	void removeTexture2D();
	void updateTexture2D(int offsetX, int offsetY, int width, int height);
	cocos2d::Texture2D *getTexture2D() { return _texture2d; }
public:
	virtual void setPosition(const cocos2d::Vec2& pos) override;
	virtual void setPosition(float x, float y) override;
	virtual const cocos2d::Vec2& getPosition() const;
	virtual void getPosition(float *x, float *y) const;
public:
	virtual void setScale(const cocos2d::Vec2& pos);
	virtual void setScale(float scaleX, float scaleY) override;
	virtual float getScaleX() const;
	virtual float getScaleY() const;
public:
	virtual void setRotation(float rotation) override;
	virtual float getRotation() const;
#ifdef AGTK_DEBUG
public:
	void setVisibleDebugDisplay(bool bVisible);
	PrimitiveNode *getDebugNode();
#endif
private:
	virtual bool initWithFilename(std::string path, ss::ResourceManager *resman);
	virtual bool initWithAnimationData(agtk::data::AnimationData *animationData, agtk::data::ResourceInfoData *resourceInfoData, ss::ResourceManager* resman);
private:
	std::string extractFilename(std::string path);
private:
	CC_SYNTHESIZE_RETAIN(cocos2d::__String *, _filename, Filename);
	CC_SYNTHESIZE_RETAIN(agtk::data::AnimationOnlyData *, _animationOnlyData, AnimationOnlyData);
	cocos2d::Texture2D *_texture2d;
	unsigned char *_buffer;
	CC_SYNTHESIZE(cocos2d::Rect, _animationOnlyRect, AnimationOnlyRect);
	CC_SYNTHESIZE(bool, _nodeToWorldTransformFlag, NodeToWorldTransformFlag);	//独自のNodeToWorldTransformを使用するかどうか
	CC_SYNTHESIZE(cocos2d::Mat4, _nodeToWorldTransform, NodeToWorldTransform);	//独自のNodeToWorldTransform
	CC_SYNTHESIZE(cocos2d::Vec2, _originOrg, OriginOrg);	//プロジェクトデータから読み込んだSSアニメのオリジン
	CC_SYNTHESIZE(bool, _flipX, FlipX);
	CC_SYNTHESIZE(bool, _flipY, FlipY);
public:
	static bool _bPlatformInit;
};

NS_AGTK_END

#endif	//__SS_PLAYER_H__
