#ifndef __IMAGE_PLAYER_H__
#define	__IMAGE_PLAYER_H__

#include "Lib/Macros.h"
#include "Lib/Player/BasePlayer.h"
#include "Data/AnimationData.h"

class PrimitiveNode;
NS_AGTK_BEGIN
/**
 * @brief 画像素材アニメの表示を管理するクラス。
 */
class AGTKPLAYER_API ImagePlayer : public cocos2d::Sprite, public agtk::BasePlayer
{
protected:
	ImagePlayer();
	virtual ~ImagePlayer();
public:
	static ImagePlayer *(*createWithAnimationData)(agtk::data::AnimationData *animationData, agtk::data::ResourceInfoData *resourceInfoData, int imageId);
	static void setCreateWithAnimationData(ImagePlayer *(*createWithAnimationData)(agtk::data::AnimationData *animationData, agtk::data::ResourceInfoData *resourceInfoData, int imageId));
	static ImagePlayer *_createWithAnimationData(agtk::data::AnimationData *animationData, agtk::data::ResourceInfoData *resourceInfoData, int imageId);
	virtual void update(float dt);
	virtual void play(int actionNo, int actionDirectNo, float seconds = 0.0f, bool bIgnoredSound = false, bool bReverse = false);
	 void setResourceSetId(int resourceSetId);
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
protected:
	virtual bool initWithAnimationData(agtk::data::AnimationData *animationData, agtk::data::ResourceInfoData *resourceInfoData, int imageId);
protected:
	//cocos2d::Vec2 _leftupAnchorPoint;
};

NS_AGTK_END

#endif	//__IMAGE_PLAYER_H__
