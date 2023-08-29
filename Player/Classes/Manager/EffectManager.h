#ifndef __EFFECT_MANAGER_H__
#define __EFFECT_MANAGER_H__

#include "cocos2d.h"
#include "Lib/Macros.h"
#include "Lib/Effect.h"
#include "Lib/Animation.h"

USING_NS_CC;

class AGTKPLAYER_API EffectManager : public cocos2d::Ref
{
private:
	EffectManager();
	static EffectManager *_effectManager;
	agtk::EffectAnimation* addEffectAnimation(int sceneLayerId, int duration300, agtk::data::AnimationData * animationData, int zOrder); // エフェクトアニメーションを生成
#ifdef USE_REDUCE_RENDER_TEXTURE
	agtk::EffectAnimation* addEffectAnimation(agtk::Object *object, int duration300, agtk::data::AnimationData * animationData, int zOrder, bool bForceBack); // オブジェクトにエフェクトアニメーションを生成
#endif

public:
	virtual ~EffectManager();
	static EffectManager* getInstance();
	static void purge();
	bool init();

	void update(float delta);

	agtk::EffectAnimation* addEffectAnimation(cocos2d::Vec2 pos, int sceneLayerId, int duration300, agtk::data::AnimationData *animationData); // 指定位置にエフェクトアニメーションを生成する
#ifdef USE_REDUCE_RENDER_TEXTURE
	agtk::EffectAnimation* addEffectAnimation(agtk::Object *targetObject, cocos2d::Vec2 offset, int duration300, agtk::data::AnimationData *animationData, bool bForceBack); // エフェクトアニメーションを生成（対象オブジェクトにエフェクトを追従させる）
	agtk::EffectAnimation* addEffectAnimation(agtk::Object *targetObject, cocos2d::Vec2 offset, int connectId, int duration300, agtk::data::AnimationData *animationData, bool bForceBack); // エフェクトアニメーションを生成（対象オブジェクトにエフェクトを追従させる）
#else
	agtk::EffectAnimation* addEffectAnimation(agtk::Object *targetObject, cocos2d::Vec2 offset, int duration300, agtk::data::AnimationData *animationData); // エフェクトアニメーションを生成（対象オブジェクトにエフェクトを追従させる）
	agtk::EffectAnimation* addEffectAnimation(agtk::Object *targetObject, cocos2d::Vec2 offset, int connectId, int duration300, agtk::data::AnimationData *animationData); // エフェクトアニメーションを生成（対象オブジェクトにエフェクトを追従させる）
#endif
#ifdef USE_REDUCE_RENDER_TEXTURE
	class EffectBackup : public cocos2d::Ref
	{
	public:
		EffectBackup();
		virtual ~EffectBackup();
		static EffectBackup *create(agtk::EffectAnimation *effect);
		void init(agtk::EffectAnimation *effect);
		int _effectId;
		cocos2d::Vec2 _offset;
		int _connectionId;
		int _duration300;
		bool _forceBack;
	};
	agtk::EffectAnimation *addEffectAnimation(agtk::Object *targetObject, EffectBackup *backup);// パーティクル追加
#endif

	void removeEffect(agtk::Object *object, int effectId, bool bRemoveInstance = false); // 指定オブジェクトに追従しているエフェクトを削除する。
	void removeEffectAll(agtk::Object *object, bool bRemoveInstance = false);// 指定オブジェクトに追従するエフェクトをすべて削除する。
	
	void clearEffect(); // 全てのエフェクトを削除

	bool existsEffect(agtk::EffectAnimation* effect); // 指定エフェクトが存在するか？
	cocos2d::__Array *getEffectArray(agtk::Object *object);
	cocos2d::__Array *getEffectArray(agtk::SceneLayer *sceneLayer);
#ifdef USE_REDUCE_RENDER_TEXTURE
	cocos2d::__Array *getEffectBackupList(agtk::Object *object);
#endif

	void setAlpha(agtk::SceneLayer *sceneLayer, float alpha);

private:
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _effectList, EffectList);
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _disabledLayerIdList, DisabledLayerIdList);
};

#endif	//__EFFECT_MANAGER_H__