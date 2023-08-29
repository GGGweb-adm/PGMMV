#ifndef __AGTK_TILE_DATA_H__
#define	__AGTK_TILE_DATA_H__

#include "cocos2d.h"
#include "json/document.h"
#include "Lib/Macros.h"

USING_NS_CC;

NS_AGTK_BEGIN
NS_DATA_BEGIN

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API TileAnimationData : public cocos2d::Ref
{
private:
	TileAnimationData();
	virtual ~TileAnimationData();
public:
	CREATE_FUNC_PARAM(TileAnimationData, const rapidjson::Value&, json);
#if defined(AGTK_DEBUG)
	void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
private:
	CC_SYNTHESIZE(int, _x, X);
	CC_SYNTHESIZE(int, _y, Y);
	CC_SYNTHESIZE(int, _frame300, Frame300);
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API TileSwitchVariableConditionData : public cocos2d::Ref
{
public:
	enum EnumCompareValueType {
		kCompareValue,//定数
		kCompareVariable,//変数
		kCompareNaN,//数値以外（非数）
		kCompareMax
	};
	enum {
		kSelfObject = -2,
		kOtherThanSelfObject = -3,
		kChildObject = -4,
		kLockedObject = -5,
		kTouchedObject = -6,
	};
	enum EnumQualifierType {
		kQualifierSingle = -1,
		kQualifierWhole = -2,
	};
private:
	TileSwitchVariableConditionData();
	virtual ~TileSwitchVariableConditionData();
public:
	CREATE_FUNC_PARAM(TileSwitchVariableConditionData, const rapidjson::Value&, json);
#if defined(AGTK_DEBUG)
	void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
private:
	CC_SYNTHESIZE(bool, _swtch, Swtch);//スイッチ or 変数
	//スイッチを条件に設定
	CC_SYNTHESIZE(int, _switchObjectId, SwitchObjectId);//スイッチオブジェクトID
	CC_SYNTHESIZE(int, _switchId, SwitchId);//スイッチID
	CC_SYNTHESIZE(EnumQualifierType, _switchQualifierId, SwitchQualifierId);
	CC_SYNTHESIZE(int, _switchValue, SwitchValue);//スイッチ値
	//変数を条件に設定
	CC_SYNTHESIZE(int, _variableObjectId, VariableObjectId);
	CC_SYNTHESIZE(int, _variableId, VariableId);
	CC_SYNTHESIZE(EnumQualifierType, _variableQualifierId, VariableQualifierId);
	CC_SYNTHESIZE(int, _compareOperator, CompareOperator);
	CC_SYNTHESIZE(EnumCompareValueType, _compareValueType, CompareValueType);
	CC_SYNTHESIZE(double, _comparedValue, ComparedValue);
	CC_SYNTHESIZE(int, _comparedVariableObjectId, ComparedVariableObjectId);
	CC_SYNTHESIZE(int, _comparedVariableId, ComparedVariableId);
	CC_SYNTHESIZE(EnumQualifierType, _comparedVariableQualifierId, ComparedVariableQualifierId);
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API TileSwitchVariableAssignData : public cocos2d::Ref
{
public:
	enum EnumSwitchAssignValue {
		kSwitchAssignOn,
		kSwitchAssignOff,
		kSwitchAssignToggle,
		kSwitchAssignMax
	};
	enum {
		kSelfObject = -2,
		kOtherThanSelfObject = -3,
		kChildObject = -4,
		kLockedObject = -5,
		kTouchedObject = -6,
	};
	enum EnumVariableAssignOperatorType {
		kVariableAssignOperatorSet,
		kVariableAssignOperatorAdd,
		kVariableAssignOperatorSub,
		kVariableAssignOperatorMul,
		kVariableAssignOperatorDiv,
		kVariableAssignOperatorMod,
		kVariableAssignOperatorMax
	};
	enum EnumQualifierType {
		kQualifierSingle = -1,
		kQualifierWhole = -2,
	};
private:
	TileSwitchVariableAssignData();
	virtual ~TileSwitchVariableAssignData();
public:
	CREATE_FUNC_PARAM(TileSwitchVariableAssignData, const rapidjson::Value&, json);
#if defined(AGTK_DEBUG)
	void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
private:
	CC_SYNTHESIZE(bool, _swtch, Swtch);
	CC_SYNTHESIZE(int, _switchObjectId, SwitchObjectId);
	CC_SYNTHESIZE(EnumQualifierType, _switchQualifierId, SwitchQualifierId);
	CC_SYNTHESIZE(int, _switchId, SwitchId);
	CC_SYNTHESIZE(int, _switchValue, SwitchValue);
	CC_SYNTHESIZE(int, _variableObjectId, VariableObjectId);
	CC_SYNTHESIZE(int, _variableId, VariableId);
	CC_SYNTHESIZE(EnumQualifierType, _variableQualifierId, VariableQualifierId);
	CC_SYNTHESIZE(int, _variableAssignValueType, VariableAssignValueType);
	CC_SYNTHESIZE(EnumVariableAssignOperatorType, _variableAssignOperator, VariableAssignOperator);
	CC_SYNTHESIZE(double, _assignValue, AssignValue);
	CC_SYNTHESIZE(int, _assignVariableObjectId, AssignVariableObjectId);
	CC_SYNTHESIZE(int, _assignVariableId, AssignVariableId);
	CC_SYNTHESIZE(EnumQualifierType, _assignVariableQualifierId, AssignVariableQualifierId);
	CC_SYNTHESIZE(int, _randomMin, RandomMin);
	CC_SYNTHESIZE(int, _randomMax, RandomMax);
	CC_SYNTHESIZE_RETAIN(cocos2d::__String *, _assignScript, AssignScript);
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API TileData : public cocos2d::Ref
{
public:
	enum EnumConditionType {
		kConditionTouched,
		kConditionOverlapped,
		kConditionMax
	};
	enum EnumPlayerMoveType {
		kPlayerMoveNone,
		kPlayerMoveTouched,
		kPlayerMoveReleased,
		kPlayerMoveOverlapped,
		kPlayerMoveMax
	};
	enum EnumChangeTileType {
		kChangeTileDisappear,
		kChangeTileNone,
		kChangeTile,
		kChangeTileMax
	};
private:
	TileData();
	virtual ~TileData();
public:
	CREATE_FUNC_PARAM(TileData, const rapidjson::Value&, json);
	const char *getMemo();
	const char *getGimmickDescription();
	const char *getGimmickMemo();
	int getTileAnimationMaxFrame300();
	int getGroupBit()const;

#if defined(AGTK_DEBUG)
	void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
private:
	//-----------------------------------------------------------------------------------------------------------------------
	//基本タイル
	//※プレイヤーキャラがタイルに触れている間のみ効果を発動
	CC_SYNTHESIZE(EnumConditionType, _conditionType, ConditionType);//効果発動条件（0:プレイヤーがタイルの壁判定に触れたら、1:プレイヤーがタイルに重なったら）
	CC_SYNTHESIZE(int, _group, Group);// タイルグループ
	CC_SYNTHESIZE(int, _targetObjectGroupBit, TargetObjectGroupBit);// 対象のオブジェクトグループ
	CC_SYNTHESIZE(bool, _areaAttributeFlag, AreaAttributeFlag);//オブジェクトのエリア判定変数に数字代入（フラグ）
	CC_SYNTHESIZE(int, _areaAttribute, AreaAttribute);//オブジェクトのエリア判定変数に数字代入
	CC_SYNTHESIZE(bool, _moveSpeedChanged, MoveSpeedChanged);//プレイヤーキャラの移動速度を増減（フラグ）
	CC_SYNTHESIZE(double, _moveSpeedChange, MoveSpeedChange);//プレイヤーキャラの移動速度増減（±）
	CC_SYNTHESIZE(bool, _jumpChanged, JumpChanged);//プレイヤーキャラのジャンプ力を増減（フラグ）
	CC_SYNTHESIZE(double, _jumpChange, JumpChange);//プレイヤーキャラのジャンプ力を増減（±）
	CC_SYNTHESIZE(bool, _moveXFlag, MoveXFlag);//オブジェクトをX方向に移動(±)（フラグ）
	CC_SYNTHESIZE(double, _moveX, MoveX);//オブジェクトをX方向に移動(±)
	CC_SYNTHESIZE(bool, _moveYFlag, MoveYFlag);//オブジェクトをY方向に移動(±)（フラグ）
	CC_SYNTHESIZE(double, _moveY, MoveY);//オブジェクトをY方向に移動(±)
	CC_SYNTHESIZE(bool, _slipChanged, SlipChanged);//プレイヤーキャラの移動が滑るようになる（フラグ）
	CC_SYNTHESIZE(double, _slipChange, SlipChange);//プレイヤーキャラの移動が滑るようになる
	CC_SYNTHESIZE(bool, _getDead, GetDead);//プレイヤーキャラが死亡する
	CC_SYNTHESIZE(bool, _hpChanged, HpChanged);//プレイヤーキャラのHPを増減（フラグ）
	CC_SYNTHESIZE(double, _hpChange, HpChange);//プレイヤーキャラのHPを増減（±）
	CC_SYNTHESIZE(bool, _triggerPeriodically, TriggerPeriodically);		// プレイヤーキャラの体力を増減する間隔（フラグ）
	CC_SYNTHESIZE(double, _triggerPeriod, TriggerPeriod);				// プレイヤーキャラの体力を増減する間隔（値）
	CC_SYNTHESIZE(bool, _gravityEffectChanged, GravityEffectChanged);//重力効果を増減（フラグ）
	CC_SYNTHESIZE(double, _gravityEffectChange, GravityEffectChange);//重力効果を増減（％）
	CC_SYNTHESIZE(int, _a, A);//アルファ値
	CC_SYNTHESIZE(int, _r, R);//R値
	CC_SYNTHESIZE(int, _g, G);//G値
	CC_SYNTHESIZE(int, _b, B);//B値
	CC_SYNTHESIZE(double, _physicsRepulsion, PhysicsRepulsion);//反発係数
	CC_SYNTHESIZE(double, _physicsFriction, PhysicsFriction);//摩擦係数
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _tileAnimationData, TileAnimationData);//->TileAnimationData
	CC_SYNTHESIZE_RETAIN(cocos2d::__String *, _memo, Memo);//メモ
	//-----------------------------------------------------------------------------------------------------------------------
	//ギミック設定
	//変更の条件
	CC_SYNTHESIZE_RETAIN(cocos2d::__String *, _gimmickDescription, GimmickDescription);//ギミック内容
	CC_SYNTHESIZE(EnumPlayerMoveType, _playerMoveType, PlayerMoveType);//プレイヤー触れたらorプレイヤーが離れたら
	CC_SYNTHESIZE(bool, _tileAttackAreaTouched, TileAttackAreaTouched);//タイルに当たる攻撃判定が触れたら
	CC_SYNTHESIZE(bool, _timePassed, TimePassed);//時間経過が状態変更（フラグ）
	CC_SYNTHESIZE(int, _passedTime, PassedTime);//時間経過で状態変更
	CC_SYNTHESIZE(bool, _switchVariableCondition, SwitchVariableCondition);//スイッチ、変数の条件で変更（フラグ）
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _switchVariableConditionList, SwitchVariableConditionList);//スイッチ、変数の条件で変更
	CC_SYNTHESIZE(int, _gimmickTargetObjectGroupBit, GimmickTargetObjectGroupBit);// ギミックの対象のオブジェクトグループ
	//変更後の状態
	CC_SYNTHESIZE(EnumChangeTileType, _changeTileType, ChangeTileType);//タイル変更する（タイルを消滅させる、タイルを変更しない、タイルを変更する）
	CC_SYNTHESIZE(int, _changeTileX, ChangeTileX);//変更タイルX
	CC_SYNTHESIZE(int, _changeTileY, ChangeTileY);//変更タイルY
	CC_SYNTHESIZE(bool, _playSe, PlaySe);//SEを鳴らす（フラグ）
	CC_SYNTHESIZE(int, _playSeId, PlaySeId);//SEを鳴らす（ID）
	CC_SYNTHESIZE(bool, _appearObject, AppearObject);//オブジェクトが出現（フラグ）
	CC_SYNTHESIZE(int, _appearObjectId, AppearObjectId);//オブジェクトが出現（オブジェクトID）
	CC_SYNTHESIZE(bool, _showEffect, ShowEffect);//エフェクトを表示
	CC_SYNTHESIZE(int, _showEffectId, ShowEffectId);//エフェクトID
	CC_SYNTHESIZE(bool, _showParticle, ShowParticle);//パーティクルを表示
	CC_SYNTHESIZE(int, _showParticleId, ShowParticleId);//パーティクルID
	CC_SYNTHESIZE(int, _switchVariableAssign, SwitchVariableAssign);//スイッチ、変数を変更（フラグ）
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _switchVariableAssignList, SwitchVariableAssignList);//スイッチ、変数を変更
	CC_SYNTHESIZE_RETAIN(cocos2d::__String *, _gimmickMemo, GimmickMemo);//ギミックメモ
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API TilesetData : public cocos2d::Ref
{
public:
	enum WallSet {
		Up,
		Left,
		Right,
		Down,
	};
	enum EnumTilesetType {
		kNormal,
		kGimmick,
		kAuto,
		kMax
	};
private:
	TilesetData();
	virtual ~TilesetData();
public:
	CREATE_FUNC_PARAM(TilesetData, const rapidjson::Value&, json);
	const char *getName();
	const char *getMemo();
	int getWallSetting(int id);
	bool getWallSetting(int id, WallSet set);
	TileData *getTileData(int id);
	TileData *getTileData(int x, int y);
#if defined(AGTK_DEBUG)
	void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
private:
	CC_SYNTHESIZE(int, _id, Id);
	CC_SYNTHESIZE_RETAIN(cocos2d::__String *, _name, Name);
	CC_SYNTHESIZE(EnumTilesetType, _tilesetType, TilesetType);
	CC_SYNTHESIZE(int, _imageId, ImageId);
	CC_SYNTHESIZE_RETAIN(cocos2d::__String *, _memo, Memo);
	CC_SYNTHESIZE(int, _horzTileCount, HorzTileCount);
	CC_SYNTHESIZE(int, _vertTileCount, VertTileCount);
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _wallSetting, WallSetting);//->cocos2d::Integer
	CC_SYNTHESIZE_RETAIN(cocos2d::__Dictionary *, _tileDataList, TileDataList);//->TileData
	CC_SYNTHESIZE(bool, _folder, Folder);
	CC_SYNTHESIZE_RETAIN(cocos2d::__Dictionary *, _children, Children);
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API Tile : public cocos2d::Ref
{
private:
	Tile();
	virtual ~Tile();
public:
	enum {
		HorzSubtileCount = 2,
		VertSubtileCount = 2,
	};
	CREATE_FUNC_PARAM2(Tile, const rapidjson::Value&, json, const char *, id);
	const char *getId();
#if defined(AGTK_DEBUG)
	void dump();
#endif
	int getSubtileX(int sx, int sy);
	int getSubtileY(int sx, int sy);
private:
	virtual bool init(const rapidjson::Value& json, const char* id);
private:
	CC_SYNTHESIZE(int, _tilesetId, TilesetId);
	CC_SYNTHESIZE(int, _x, X);
	CC_SYNTHESIZE(int, _y, Y);
	CC_SYNTHESIZE(cocos2d::Vec2, _position, Position);
	CC_SYNTHESIZE_RETAIN(cocos2d::__String *, _id, Id);
	int subtileX[HorzSubtileCount][VertSubtileCount];
	int subtileY[HorzSubtileCount][VertSubtileCount];
};

NS_DATA_END
NS_AGTK_END

#endif	//__AGTK_TILE_DATA_H__
