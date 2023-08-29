#ifndef __EFFECT_H__
#define __EFFECT_H__

#include "cocos2d.h"
#include "Lib/Macros.h"
#include "Lib/Player.h"
#include "Lib/Object.h"
#include "Lib/Particle.h"

NS_AGTK_BEGIN

//-------------------------------------------------------------------------------------------------------------------
/**
 * @brief エフェクトアニメーションを管理するクラス。
 */
class AGTKPLAYER_API EffectAnimation : public cocos2d::Node
{
private:
	EffectAnimation();
	virtual ~EffectAnimation();

	virtual bool init(int sceneLayerId, agtk::data::AnimationData * animationData, int zOrder);
#ifdef USE_REDUCE_RENDER_TEXTURE
	virtual bool init(agtk::Object *object, agtk::data::AnimationData * animationData, int zOrder, bool bForceBack);
#endif

public:
	CREATE_FUNC_PARAM3(EffectAnimation, int, sceneLayerId, agtk::data::AnimationData *, animationData, int, zOrder);
#ifdef USE_REDUCE_RENDER_TEXTURE
	CREATE_FUNC_PARAM4(EffectAnimation, agtk::Object *, sceneLayerId, agtk::data::AnimationData *, animationData, int, zOrder, bool, bForceBack);
#endif
	virtual void update(float delta);
	void updatePosition(); // 座標の更新
	void remove();
	void deleteEffect();
	void stopEffect();
	bool isStopped() { return _isStop; }

	bool isDeletable(); // 更新が終了し、削除してもよいか？
	void setTargetObject(agtk::Object *object); // 追従対象のオブジェクトを設定
	agtk::Object* getTargetObject(); // 追従対象のオブジェクトを取得
	void setFillColor(cocos2d::Color4B color); // 指定色で塗りつぶす（残像エフェクト用）
	void setAlpha(float alpha);	// α値の設定（残像エフェクト用）
	void setMainAlpha(float alpha);//メインα値の設定。
	float getAlpha() { return _alpha * _mainAlpha; }
#ifdef USE_REDUCE_RENDER_TEXTURE
	void updateBackside();
#endif

protected:
	void setInnerAlpha(float alpha);

private:
	CC_SYNTHESIZE_RETAIN(agtk::Player *, _player, Player);
	CC_SYNTHESIZE(cocos2d::Vec2, _offset, Offset);
	CC_SYNTHESIZE(int, _targetObjectConnectId, TargetObjectConnectId);
	CC_SYNTHESIZE(int, _duration300, Duration300); // 生存時間
	CC_SYNTHESIZE(bool, _isCheckDuration, IsCheckDuration); // 生存時間をチェックするか？
	CC_SYNTHESIZE(bool, _targetObjectBackside, TargetObjctBackside);
#ifdef USE_REDUCE_RENDER_TEXTURE
	CC_SYNTHESIZE(bool, _forceBack, ForceBack);//表示優先度をオブジェクトの奥側にに強制
#endif

	bool _isDeletable;
	bool _isStop;
	int _sceneLayerId; // シーンレイヤーID
	agtk::Object* _targetObject; // 追従対象のオブジェクト
	float _alpha;
	float _mainAlpha;
};

//-------------------------------------------------------------------------------------------------------------------
/**
 * @brief 「オブジェクトにエフェクトを表示」の情報を保持する
 */
class AGTKPLAYER_API ObjectEffect : public cocos2d::Ref
{
private:
	ObjectEffect();
	virtual ~ObjectEffect();
	virtual bool init(agtk::data::ObjectEffectSettingData* effectData);

public:
	CREATE_FUNC_PARAM(ObjectEffect, agtk::data::ObjectEffectSettingData*, effectData);

public:
	// エフェクトの設定データ
	CC_SYNTHESIZE_RETAIN(agtk::data::ObjectEffectSettingData*, _effectSettingData, EffectSettingData);
	// 前のフレームのスイッチの値
	CC_SYNTHESIZE(bool, _switchValueOld, SwitchValueOld);
	// 今のフレームのスイッチの値
	CC_SYNTHESIZE(bool, _switchValue, SwitchValue);
	// エフェクトアニメーション
	CC_SYNTHESIZE_RETAIN(agtk::EffectAnimation*, _effectAnimation, EffectAnimation);
	// パーティクルグループ
	CC_SYNTHESIZE_RETAIN(agtk::ParticleGroup*, _particleGroup, ParticleGroup);
};

NS_AGTK_END

#endif	//__EFFECT_H__