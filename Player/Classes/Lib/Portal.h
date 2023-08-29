#ifndef __PORTAL_H__
#define	__PORTAL_H__

#include "Lib/Macros.h"
#include "Lib/Common.h"
#include "Lib/Object.h"
#include "Data/ProjectData.h"

NS_AGTK_BEGIN

// -----------------------------------------------------------------------------------------
// !ポータルタッチクラス
// -----------------------------------------------------------------------------------------
class AGTKPLAYER_API PortalTouchedData : public cocos2d::Ref
{
private:
	PortalTouchedData();
	virtual ~PortalTouchedData();

public:
	CREATE_FUNC_PARAM4(PortalTouchedData, agtk::Object *, object, cocos2d::Vec2, appearPosition, int, sceneLayerId, int, visibleDlay300);
	cocos2d::__Array *getParticleGroupBackupList();
	cocos2d::__Array *getEffectBackupList();

private:
	virtual bool init(agtk::Object * object, cocos2d::Vec2 appearPosition, int sceneLayerId, int visibleDlay300);

private:
	CC_SYNTHESIZE_RETAIN(agtk::Object*, _object, Object);//オブジェクト
	CC_SYNTHESIZE(cocos2d::Vec2, _appearPosition, AppearPosition);//出現位置
	CC_SYNTHESIZE(int, _sceneLayerId, sceneLayerId);//移動先シーンレイヤーID
	CC_SYNTHESIZE(int, _visibleDelay300, VisibleDelay300);//表示されるまでのディレイ時間
	CC_SYNTHESIZE(cocos2d::Vec2, _srcPortalCenterPos, SrcPortalCenterPos);//飛び元ポータルの中心座標
	CC_SYNTHESIZE(cocos2d::Vec2, _dstportalCenterPos, DstPortalCenterPos);//飛び先ポータルの中心座標
	CC_SYNTHESIZE(cocos2d::Vec2, _srcObjectPos, srcObjectPos);//遷移前のオブジェクトの座標
	CC_SYNTHESIZE(bool, _needObjectMoveEffect, NeedObjectMoveEffect);//オブジェクトの移動演出の要不要フラグ
#define USE_ACT2_5389
#ifdef USE_ACT2_5389
#else
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _childrenObjectList, ChildrenObjectList);//子オブジェクトリスト
#endif
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _particleGroupBackupList, ParticleGroupBackupList);
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _effectBackupList, EffectBackupList);
};

// -----------------------------------------------------------------------------------------
// !ポータルクラス
// -----------------------------------------------------------------------------------------
class AGTKPLAYER_API Portal : public cocos2d::Node
{
private:
	Portal();
	virtual ~Portal();

public:
	static Portal *create(int id, int idx, int type, cocos2d::__Array * areaSettingDataList, cocos2d::__Array * movableList, agtk::data::MoveSettingData * moveSettingData, agtk::data::SceneData * sceneData)
	{
		Portal *ret = new (std::nothrow) Portal();
		if (ret && ret->init(id, idx, type, areaSettingDataList, movableList, moveSettingData, sceneData)) {
			ret->autorelease();
		}
		else {
			CC_SAFE_DELETE(ret);
		}
		return ret;
	}
	virtual void update(float dt) override;

	//ポータルに触れたオブジェクトを追加
	bool addTouchObject(agtk::Object * object);
private:
	virtual bool init(int id, int idx, int type, cocos2d::__Array * areaSettingDataList, cocos2d::__Array * movableList, agtk::data::MoveSettingData * moveSettingData, agtk::data::SceneData * sceneData);

	// リセット
	void reset();

	// ポータルに触れたデータの生成
	void createTouchedData(agtk::Object *object, cocos2d::Vec2 objectPosition);

	// 移動開始条件の「スイッチ、変数が変化」チェック
	bool checkSwichAndVariablesCondision(cocos2d::__Array *condisionList);

private:
	CC_SYNTHESIZE_READONLY(int, _id, Id);	// ポータルID
	CC_SYNTHESIZE(int, _portalType, PortalType);//ポータルタイプ(0:A / 1:B)
	CC_SYNTHESIZE(bool, _isMovable, IsMovable);//移動可能フラグ
	CC_SYNTHESIZE(bool, _keepHorzPosition, KeepHorzPosition);//移動先でX方向の位置を合わせる
	CC_SYNTHESIZE(bool, _keepVertPosition, KeepVertPosition);//移動先でY方向の位置を合わせる
	CC_SYNTHESIZE(int, _sceneLayerId, SceneLayerId);//シーンレイヤーID

	CC_SYNTHESIZE(bool, _isSameScene, IsSameScene);//移動先シーンが移動元シーンと同一か？
	CC_SYNTHESIZE(int, _moveToSceneId, MoveToSceneId);//移動先のシーンID
	CC_SYNTHESIZE(int, _moveToSceneLayerId, MoveToSceneLayerId);//移動先のシーンレイヤーID
	CC_SYNTHESIZE(Vec2, _moveToPortalPosition, MoveToPortalPosition);//移動先ポータルの位置
	CC_SYNTHESIZE(Size, _moveToPortalSize, MoveToPortalSize);//移動先ポータルのサイズ

	CC_SYNTHESIZE_RETAIN(agtk::data::MoveSettingData *, _moveSettingData, MoveSettingData);//移動設定データ(GameManagerへ渡す用)

	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _touchedObjectIdList, TouchedObjectIdList);//ポータルに触れたオブジェクトIDリスト
	
	CC_SYNTHESIZE(int, _rePortalDirectionBit, RePortalDirectionBit);//再移動判定用の入力方向ビット値
	std::vector<int> _lastTouchObjectInstanceIdList;	// 直前にポータルに触れていたオブジェクトインスタンスIDリスト。
	std::vector<int> _touchObjectInstanceIdList;	// ポータルに触れたオブジェクトインスタンスIDを記録。

	// 以下は動作用メンバ
	float _elapsedTime;//経過時間
	bool _startCountElapsedTime;//経過時間計測開始フラグ
	bool _needMovePortal;//ポータル移動要求フラグ

	// デバッグ用表示
	void showDebugVisible(bool isShow);
};

NS_AGTK_END

#endif __PORTAL_H__