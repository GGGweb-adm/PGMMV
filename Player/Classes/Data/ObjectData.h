#ifndef __AGTK_OBJECT_DATA_H__
#define	__AGTK_OBJECT_DATA_H__

#include "cocos2d.h"
#include "json/document.h"
#include "Lib/Macros.h"
#include "Data/ObjectCommandData.h"
#include "Data/ObjectActionLinkConditionData.h"
#include "Data/AssetData.h"

USING_NS_CC;

NS_AGTK_BEGIN
NS_DATA_BEGIN
class TileData;

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API ObjectActionData : public cocos2d::Ref
{
private:
	ObjectActionData();
	virtual ~ObjectActionData();
public:
	CREATE_FUNC_PARAM(ObjectActionData, const rapidjson::Value&, json);
	const char *getName();
	const char *getMemo();
	agtk::data::ObjectCommandData *getObjectCommandData(int id);
#if defined(AGTK_DEBUG)
	void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
private:
	CC_SYNTHESIZE(unsigned int, _id, Id);//ID
	CC_SYNTHESIZE_RETAIN(cocos2d::__String *, _name, Name);//名前
	CC_SYNTHESIZE(int, _a, A);//アルファ値
	CC_SYNTHESIZE(int, _r, R);//R値
	CC_SYNTHESIZE(int, _g, G);//G値
	CC_SYNTHESIZE(int, _b, B);//B値
	CC_SYNTHESIZE(int, _animMotionId, AnimMotionId);//モーション
	CC_SYNTHESIZE(int, _animDirectionId, AnimDirectionId);//表示方向
	CC_SYNTHESIZE(bool, _takeOverMotion, TakeOverMotion);//遷移前のモーションを引き継ぐ
	CC_SYNTHESIZE(bool, _jumpable, Jumpable);//ジャンプ動作を行う
	CC_SYNTHESIZE(bool, _enableCustomizedJump, EnableCustomizedJump);//調整ジャンプを有効化
	CC_SYNTHESIZE(bool, _ignoreMoveInput, IgnoreMoveInput);//動作中に移動の入力を受け付けない
	CC_SYNTHESIZE(bool, _keepDirection, KeepDirection);//動作中の表示方向の変更を受け付けない
	CC_SYNTHESIZE(bool, _ignoreGravity, IgnoreGravity);//重力の影響を受けない
	CC_SYNTHESIZE(double, _gravity, Gravity);//重力(%)
	CC_SYNTHESIZE(bool, _reflectGravityToDisplayDirection, ReflectGravityToDisplayDirection);// 重力の影響を表示方向へ反映
	CC_SYNTHESIZE(double, _moveSpeed, MoveSpeed);//左右の移動速度を変更(%)
	CC_SYNTHESIZE(double, _upDownMoveSpeed, UpDownMoveSpeed);//上下（前後）の移動速度を変更(%)
	CC_SYNTHESIZE(double, _jumpSpeed, JumpSpeed);//ジャンプ速度を変更(%)
	CC_SYNTHESIZE(double, _turnSpeed, TurnSpeed);//旋回速度を変更(%)
	CC_SYNTHESIZE_RETAIN(cocos2d::__Dictionary *, _objCommandList, ObjCommandList);//その他の実行アクション
	CC_SYNTHESIZE_RETAIN(cocos2d::__String *, _memo, Memo);//メモ
	CC_SYNTHESIZE(float, _x, X);
	CC_SYNTHESIZE(float, _y, Y);
	CC_SYNTHESIZE(float, _width, Width);
	CC_SYNTHESIZE(float, _height, Height);
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API ObjectInputConditionData : public cocos2d::Ref
{
public:
	enum EnumTriggerType {
		kTriggerPressed,//押された
		kTriggerJustPressed,//押された瞬間
		kTriggerJustReleased,//離された瞬間
		kTriggerReleased,//離されている
		kTriggerMax
	};

	enum EnumDirectionInputType {
		kDirectionInputNone = -1,
		kDirectionInputCross,// 「↑↓←→」
		kDirectionInputLeftStick,//「左スティック」
		kDirectionInputRightStick,//「右スティック」
		kDirectionInputMax
	};

protected:
	ObjectInputConditionData();
	virtual ~ObjectInputConditionData();
public:
	CREATE_FUNC_PARAM(ObjectInputConditionData, const rapidjson::Value&, json);
#if defined(AGTK_DEBUG)
	void dump();
#endif
protected:
	virtual bool init(const rapidjson::Value& json);
private:
	CC_SYNTHESIZE(unsigned int, _id, Id);//ID
	CC_SYNTHESIZE(int, _operationKeyId, OperationKeyId);//入力に使う操作キー
	CC_SYNTHESIZE(EnumTriggerType, _triggerType, TriggerType);//入力方法
	CC_SYNTHESIZE(bool, _condAnd, CondAnd);//論理演算(AND:true, OR:false)
	// 以下追加分
	CC_SYNTHESIZE(unsigned int, _directionBit, DirectionBit);//「8方向で指定」の2つ目の設定値。８方向をテンキーの1～4,6～9で表現したときの、方向の数値分だけ1を左シフトしたもののOR値
	CC_SYNTHESIZE(EnumDirectionInputType, _directionInputType, DirectionInputType);//「8方向で指定」の1つ目の設定値。0から順に「↑↓←→」「左スティック」「右スティック」
	CC_SYNTHESIZE(bool, _useKey, UseKey);//「入力に使う操作キー」がチェックされているかフラグ
	CC_SYNTHESIZE(unsigned int, _acceptFrameCount, AcceptFrameCount);//受付フレーム
};

//-------------------------------------------------------------------------------------------------------------------
class ObjectCommonActionSettingData;
class AGTKPLAYER_API ObjectActionLinkData : public cocos2d::Ref
{
public:
	enum EnumChangeConditionType {
		kAllConditionsSatisfied,//すべての条件が満たされていたら切り替え
		kAnyConditionSatisfied,//いずれかの条件が満たされていたら切り替え
		kActionFinished,//アクションが最後まで実行されたら
		kAlways,//強制的に自動切り替え
		kMax
	};
private:
	ObjectActionLinkData();
	virtual ~ObjectActionLinkData();
public:
	CREATE_FUNC_PARAM(ObjectActionLinkData, const rapidjson::Value&, json);
	const char *getMemo();
	int getActionIdPair(int id);
	int getPrevActionId();
	int getNextActionId();
#if defined(AGTK_DEBUG)
	void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
	bool initConditionGroupList();
private:
	CC_SYNTHESIZE(unsigned int, _id, Id);
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _actionIdPair, ActionIdPair);//アクションID（0:アクション元,1:アクション先）
	CC_SYNTHESIZE(int, _priority, Priority);//判定優先度
	CC_SYNTHESIZE(int, _a, A);//アルファ値
	CC_SYNTHESIZE(int, _r, R);//R値
	CC_SYNTHESIZE(int, _g, G);//B値
	CC_SYNTHESIZE(int, _b, B);//G値
	CC_SYNTHESIZE(EnumChangeConditionType, _changeConditionType, ChangeConditionType);//アクションを切り替える条件
	CC_SYNTHESIZE(bool, _noInput, NoInput);//何も入力されなかった
	CC_SYNTHESIZE(bool, _useInput, UseInput);//以下の入力操作がされた
	CC_SYNTHESIZE_RETAIN(cocos2d::__Dictionary *, _inputConditionList, InputConditionList);//->ObjectInputConditionList
	CC_SYNTHESIZE_RETAIN(cocos2d::__Dictionary *, _linkConditionList, LinkConditionList);//その他の条件設定
	CC_SYNTHESIZE_RETAIN(cocos2d::__String *, _memo, Memo);//メモ
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _coordList, CoordList);
	//Additional Data
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _inputConditionGroupList, InputConditionGroupList);//->ObjectInputConditionList
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _linkConditionGroupList, LinkConditionGroupList);//->ObjectActionLinkConditionData
	CC_SYNTHESIZE(agtk::data::ObjectCommonActionSettingData *, _commonActionSettingData, CommonActionSettingData);//weak link
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API ObjectActionGroupData : public cocos2d::Ref
{
private:
	ObjectActionGroupData();
	virtual ~ObjectActionGroupData();
public:
	CREATE_FUNC_PARAM(ObjectActionGroupData, const rapidjson::Value&, json);
	const char *getName();
#if defined(AGTK_DEBUG)
	void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
private:
	CC_SYNTHESIZE(unsigned int, _id, Id);
	CC_SYNTHESIZE_RETAIN(cocos2d::__String *, _name, Name);
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _actionIdList, ActionIdList);
	CC_SYNTHESIZE(float, _x, X);
	CC_SYNTHESIZE(float, _y, Y);
	CC_SYNTHESIZE(float, _width, Width);
	CC_SYNTHESIZE(float, _height, Height);
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API ObjectDisappearSettingData : public cocos2d::Ref
{
public:
	enum EnumTargetObjectType {
		kTargetObjectByType,//オブジェクトの種類指定
		kTargetObjectById,//オブジェクトの指定
		kTargetObjectMax
	};
	enum EnumDamageRange {
		kRangeInsideCamera,//カメラ範囲内
		kRangeWholeScene,//シーン全体
		kRangeMax
	};
private:
	ObjectDisappearSettingData();
	virtual ~ObjectDisappearSettingData();
public:
	CREATE_FUNC_PARAM(ObjectDisappearSettingData, const rapidjson::Value&, json);
#if defined(AGTK_DEBUG)
	void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
private:
	//------------------------------------------------------------------------------------------------------------
	//他オブジェクトの体力を減らす
	CC_SYNTHESIZE(bool, _otherObjectDecrementHp, OtherObjectDecrementHp);//他オブジェクトの体力を減らす（設定有無）
	CC_SYNTHESIZE(int, _decrement, Decrement);//他オブジェクトの体力を減らす（減少量）
	CC_SYNTHESIZE(EnumTargetObjectType, _targetObjectType, TargetObjectType);//他オブジェクトの指定（EnumTargetObjectType）
	CC_SYNTHESIZE(int, _targetObjectGroup, TargetObjectGroup);//オブジェクトのグループを指定 -1:すべてのオブジェクトグループ
	CC_SYNTHESIZE(int, _targetObjectId, TargetObjectId);//オブジェクトで指定（-3:自身以外のオブジェクト,-2:自身のオブジェクト,>0:オブジェクトID）
	//------------------------------------------------------------------------------------------------------------
	//ダメージ範囲
	CC_SYNTHESIZE(EnumDamageRange, _damageRange, DamageRange);//ダメージ範囲（0:カメラ範囲内, 1:シーン全体）
	CC_SYNTHESIZE(int, _cameraRangeAdjust, CameraRangeAdjust);//カメラ範囲調整（単位ドット）
	//------------------------------------------------------------------------------------------------------------
	//その他消滅後の設定
	CC_SYNTHESIZE_RETAIN(cocos2d::__Dictionary *, _objCommandList, ObjCommandList);//実行アクション
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API ObjectFireBulletSettingData : public cocos2d::Ref
{
public:
	//弾の飛び方(初期動作)
	enum EnumInitialBulletLocus {
		kInitialBulletLocusFree,
		kInitialBulletLocusFireObjectDirection,
		kInitialBulletLocusTowardObject,
		kInitialBulletLocusOneDirection,
		kInitialBulletLocusMax
	};
	//弾の飛び方(変化)
	enum EnumNextBulletLocus {
		kNextBulletLocusFree,
		kNextBulletLocusFollowLockedObject,
		kNextBulletLocusFollowObjectInsideCamera,
		kNextBulletLocusBoomerang,
		kNextBulletLocusMax
	};
	//動きを指定
	enum EnumSpreadType {
		kSpreadFixed,//固定
		kSpreadWiper,//ワイパー
		kSpreadRandom,//ランダム
		kSpreadMax
	};
private:
	ObjectFireBulletSettingData();
	virtual ~ObjectFireBulletSettingData();
public:
	CREATE_FUNC_PARAM(ObjectFireBulletSettingData, const rapidjson::Value&, json);
	const char *getName();
#if defined(AGTK_DEBUG)
	void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
private:
	CC_SYNTHESIZE(unsigned int, _id, Id);
	CC_SYNTHESIZE_RETAIN(cocos2d::__String *, _name, Name);
	CC_SYNTHESIZE(int, _bulletObjectId, BulletObjectId);
	CC_SYNTHESIZE(bool, _bulletChild, BulletChild);//弾を子オブジェクトにする
	CC_SYNTHESIZE(int, _fireBulletCount, FireBulletCount);//１回の発射で出る弾数
	CC_SYNTHESIZE(int, _bulletInterval300, BulletInterval300);//弾と弾の間隔
	CC_SYNTHESIZE(int, _dispBulletCount, DispBulletCount);//画面内に表示される弾数
	CC_SYNTHESIZE(bool, _dispBulletUnlimited, DispBulletUnlimited);//弾数制限なし
	CC_SYNTHESIZE(EnumInitialBulletLocus, _initialBulletLocus, InitialBulletLocus);//弾の飛び方タイプ(初期動作)
	CC_SYNTHESIZE(EnumNextBulletLocus, _nextBulletLocus, NextBulletLocus);//弾の飛び方タイプ(変化)
	//飛び方を指定しない
	CC_SYNTHESIZE(bool, _setActionDirectionToFireObjectDirection, SetActionDirectionToFireObjectDirection);//発射元のオブジェクトに表示方向を合わせる
	CC_SYNTHESIZE(int ,_towardObjectGroupBit, TowardObjectGroupBit)// 画面内の指定されたオブジェクトグループ方向に飛ぶ
	//指定方向に飛ぶ
	CC_SYNTHESIZE(int, _oneDirectionAngle, OneDirectionAngle);//一定方向に飛ぶ（方向指定）
	CC_SYNTHESIZE(int, _oneDirectionSpreadRange, OneDirectionSpreadRange);//発射範囲を広げる
	CC_SYNTHESIZE(EnumSpreadType, _oneDirectionSpreadType, OneDirectionSpreadType);//動きを指定（固定、ワイパー、ランダム）
	//発射元がロックしているオブジェクトを追尾
	CC_SYNTHESIZE(int, _followLockedObjectPerformance, FollowLockedObjectPerformance);//追尾性能
	CC_SYNTHESIZE(int, _followLockedObjectStartDelayStart300, FollowLockedObjectStartDelayStart300);//追尾開始までの時間範囲（開始値）
	CC_SYNTHESIZE(int, _followLockedObjectStartDelayEnd300, FollowLockedObjectStartDelayEnd300);//追尾開始までの時間範囲（終了値）
	//画面内のオブジェクトを自動追尾
	CC_SYNTHESIZE(int, _followObjectInsideCameraPerformance, FollowObjectInsideCameraPerformance);//追尾性能
	CC_SYNTHESIZE(int, _followObjectInsideCameraStartDelayStart300, FollowObjectInsideCameraStartDelayStart300);//追尾開始までの時間範囲（開始値）
	CC_SYNTHESIZE(int, _followObjectInsideCameraStartDelayEnd300, FollowObjectInsideCameraStartDelayEnd300);//追尾開始までの時間範囲（終了値）
	CC_SYNTHESIZE(int, _followObjectInsideCameraTargetObjectGroupBit, FollowObjectInsideCameraTargetObjectGroupBit);//追尾対象
	//ブーメラン軌道
	CC_SYNTHESIZE(int, _boomerangTurnDuration300, BoomerangTurnDuration300);//折り返しまでの時間
	CC_SYNTHESIZE(bool, _boomerangDecelBeforeTurn, BoomerangDecelBeforeTurn);//折り返すまでに減速(フラグ)
	CC_SYNTHESIZE(int, _boomerangComebackPerformance, BoomerangComebackPerformance);//追尾性能
	CC_SYNTHESIZE(bool, _boomerangTurnWhenTouchingWall, BoomerangTurnWhenTouchingWall);//壁判定に接触で折り返す(フラグ)
	//飛び方を指定しない場合のオプション
	CC_SYNTHESIZE(bool, _freeBulletGravityFlag, FreeBulletGravityFlag);//重力の影響を受けないの有無
	CC_SYNTHESIZE(double, _freeBulletGravity, FreeBulletGravity);//重力の影響をうけない
	CC_SYNTHESIZE(bool, _freeBulletMoveSpeedFlag, FreeBulletMoveSpeedFlag);//移動速度を変更の有無
	CC_SYNTHESIZE(double, _freeBulletMoveSpeed, FreeBulletMoveSpeed);//移動速度を変更
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API ObjectEffectSettingData : public cocos2d::Ref
{
public:
	// アニメーションの種類
	enum EnumAnimationType {
		kAnimationTypeEffect,	// エフェクト
		kAnimationTypeParticle,	// パーティクル
	};

	//位置の種類
	enum EnumPositionType {
		kPositionCenter,		// このオブジェクトの中心
		kPositionFoot,			// このオブジェクトの足元
		kPositionUseConnection,	// 接続点を使用
		kPositionMax
	};

	static const int NO_SETTING = -1;	// 表示するエフェクトやパーティクルの「設定無し」の値

private:
	ObjectEffectSettingData();
	virtual ~ObjectEffectSettingData();
public:
	CREATE_FUNC_PARAM(ObjectEffectSettingData, const rapidjson::Value&, json);
	const char *getName();
#if defined(AGTK_DEBUG)
	void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
private:
	CC_SYNTHESIZE(int, _id, Id);
	CC_SYNTHESIZE_RETAIN(cocos2d::__String *, _name, Name);	
	CC_SYNTHESIZE(EnumAnimationType, _animationType, AnimationType);	// アニメーションの種類
	CC_SYNTHESIZE(int, _effectId, EffectId);							// 表示するエフェクトID
	CC_SYNTHESIZE(int, _particleId, ParticleId);						// 表示するパーティクルID
	CC_SYNTHESIZE(bool, _objectSwitch, ObjectSwitch);					// このオブジェクトのスイッチを使用フラグ
	CC_SYNTHESIZE(int, _objectSwitchId, ObjectSwitchId);				// このオブジェクトのスイッチID
	CC_SYNTHESIZE(int, _systemSwitchId, SystemSwitchId);				// player共通スイッチID
	CC_SYNTHESIZE(EnumPositionType, _positionType, PositionType);		// 位置の種類
	CC_SYNTHESIZE(int, _connectionId, ConnectionId);					// 接続点を使用
	CC_SYNTHESIZE(int, _adjustX, AdjustX);								// 位置を調整X
	CC_SYNTHESIZE(int, _adjustY, AdjustY);								// 位置を調整Y
	CC_SYNTHESIZE(int, _dispDuration300, DispDuration300);				// 表示から消えるまでの時間
	CC_SYNTHESIZE(bool, _dispDurationUnlimited, DispDurationUnlimited);	// 時間制限なしフラグ
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API ObjectConnectSettingData : public cocos2d::Ref
{
public:
	//位置の種類
	enum EnumPositionType {
		kPositionCenter,//このオブジェクトの中心
		kPositionFoot,//このオブジェクトの足元
		kPositionUseConnection,//接続点を仕様
		kPositionMax
	};

private:
	ObjectConnectSettingData();
	virtual ~ObjectConnectSettingData();
public:
	CREATE_FUNC_PARAM(ObjectConnectSettingData, const rapidjson::Value&, json);
	const char *getName();
#if defined(AGTK_DEBUG)
	void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
private:
	CC_SYNTHESIZE(unsigned int, _id, Id);
	CC_SYNTHESIZE_RETAIN(cocos2d::__String *, _name, Name);
	CC_SYNTHESIZE(int, _objectId, ObjectId);
	CC_SYNTHESIZE(bool, _objectSwitch, ObjectSwitch);
	CC_SYNTHESIZE(int, _objectSwitchId, ObjectSwitchId);
	CC_SYNTHESIZE(int, _systemSwitchId, SystemSwitchId);
	CC_SYNTHESIZE(EnumPositionType, _positionType, PositionType);
	CC_SYNTHESIZE(int, _connectionId, ConnectionId);
	CC_SYNTHESIZE(int, _adjustX, AdjustX);
	CC_SYNTHESIZE(int, _adjustY, AdjustY);
	CC_SYNTHESIZE(bool, _childObject, ChildObject);
	CC_SYNTHESIZE(bool, _setDirectionToConnectObjectDirection, SetDirectionToConnectObjectDirection);
	CC_SYNTHESIZE(bool, _lowerPriority, LowerPriority);
};

class AGTKPLAYER_API ObjectAdditionalDisplayData : public cocos2d::Ref
{
public:
	// 表示タイプ
	enum EnumParamDisplayType {
		kParamDisplayText,	// テキストを表示
		kParamDisplayGauge,	// ゲージ形式で表示
		kParamDisplayMax
	};
private:
	ObjectAdditionalDisplayData();
	virtual ~ObjectAdditionalDisplayData();
public:
	CREATE_FUNC_PARAM(ObjectAdditionalDisplayData, const rapidjson::Value&, json);
#if defined(AGTK_DEBUG)
	void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
private:
	CC_SYNTHESIZE(unsigned int, _id, Id);
	CC_SYNTHESIZE(bool, _showText, ShowText);//テキストを表示（フラグ）
	//テキストを表示
	CC_SYNTHESIZE(int, _textId, TextId);//テキスト素材ID
	CC_SYNTHESIZE(unsigned char, _textColorR, TextColorR);//テキストの色(R)
	CC_SYNTHESIZE(unsigned char, _textColorG, TextColorG);//テキストの色(G)
	CC_SYNTHESIZE(unsigned char, _textColorB, TextColorB);//テキストの色(B)
	CC_SYNTHESIZE(unsigned char, _textColorA, TextColorA);//テキストの色(A) 
	CC_SYNTHESIZE(EnumParamDisplayType, _paramDisplayType, ParamDisplayType);//表示タイプ（テキストを表示、ゲージ形式で表示）
	//パラメーターを表示
	//変数を選択
	CC_SYNTHESIZE(int, _variableObjectId, VariableObjectId);
	CC_SYNTHESIZE(int, _variableId, VariableId);
	CC_SYNTHESIZE(bool, _useParentVariable, UseParentVariable);//オプション（親オブジェクトの同名の変数を選択）
	CC_SYNTHESIZE(bool, _variableMaxEnabled, VariableMaxEnabled);// 上限を設定
	CC_SYNTHESIZE(int, _variableMaxObjectId, VariableMaxObjectId);
	CC_SYNTHESIZE(int, _variableMaxVariableId, VariableMaxVariableId);
	CC_SYNTHESIZE(bool, _variableMaxAutoScalingEnabled, VariableMaxAutoScalingEnabled);// 上限に合わせてスケールを自動調整
	CC_SYNTHESIZE(bool, _variableMaxUseParent, VariableMaxUseParent);//オプション（親オブジェクトの同名の変数を選択）上限値用
	CC_SYNTHESIZE(unsigned char, _paramTextColorR, ParamTextColorR);//テキストの色(R)
	CC_SYNTHESIZE(unsigned char, _paramTextColorG, ParamTextColorG);//テキストの色(G)
	CC_SYNTHESIZE(unsigned char, _paramTextColorB, ParamTextColorB);//テキストの色(B)
	CC_SYNTHESIZE(unsigned char, _paramTextColorA, ParamTextColorA);//テキストの色(A)
	CC_SYNTHESIZE(int, _paramTextFontId, ParamTextFontId);//フォントを選択
	CC_SYNTHESIZE(unsigned char, _paramGaugeColorR, ParamGaugeColorR);//ゲージの色(R)
	CC_SYNTHESIZE(unsigned char, _paramGaugeColorG, ParamGaugeColorG);//ゲージの色(G)
	CC_SYNTHESIZE(unsigned char, _paramGaugeColorB, ParamGaugeColorB);//ゲージの色(B)
	CC_SYNTHESIZE(unsigned char, _paramGaugeColorA, ParamGaugeColorA);//ゲージの色(A)
	CC_SYNTHESIZE(unsigned char, _paramGaugeBgColorR, ParamGaugeBgColorR);//ゲージ背景の色(R)
	CC_SYNTHESIZE(unsigned char, _paramGaugeBgColorG, ParamGaugeBgColorG);//ゲージ背景の色(G)
	CC_SYNTHESIZE(unsigned char, _paramGaugeBgColorB, ParamGaugeBgColorB);//ゲージ背景の色(B)
	CC_SYNTHESIZE(unsigned char, _paramGaugeBgColorA, ParamGaugeBgColorA);//ゲージ背景の色(A)
	//レイアウトを調整
	CC_SYNTHESIZE(double, _adjustX, AdjustX);//オフセットX値
	CC_SYNTHESIZE(double, _adjustY, AdjustY);//オフセットY値
	CC_SYNTHESIZE(double, _scaleX, ScaleX);//スケールX値(%)
	CC_SYNTHESIZE(double, _scaleY, ScaleY);//スケールY値(%)
	CC_SYNTHESIZE(double, _rotation, Rotation);//回転
	CC_SYNTHESIZE(bool, _hide, Hide);//???
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API ObjectMoveDispDirectionSettingData : public cocos2d::Ref
{
private:
	ObjectMoveDispDirectionSettingData();
	virtual ~ObjectMoveDispDirectionSettingData();
public:
	CREATE_FUNC_PARAM(ObjectMoveDispDirectionSettingData, const rapidjson::Value&, json);
	bool checkExistsActionInfo(int actionId);
#if defined(AGTK_DEBUG)
	void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
private:
	CC_SYNTHESIZE(int, _upOperationKeyId, UpOperationKeyId);
	CC_SYNTHESIZE(int, _downOperationKeyId, DownOperationKeyId);
	CC_SYNTHESIZE(int, _leftOperationKeyId, LeftOperationKeyId);
	CC_SYNTHESIZE(int, _rightOperationKeyId, RightOperationKeyId);
	CC_SYNTHESIZE_RETAIN(cocos2d::__Dictionary *, _actionInfoList, ActionInfoList);//->ObjectActionInfoList
};

//-------------------------------------------------------------------------------------------------------------------
/**
 * 移動方向を別操作で指定するデータの対応するアクションボックスデータクラス
 */
class AGTKPLAYER_API ObjectActionInfoList : public cocos2d::Ref
{
private:
	ObjectActionInfoList();
	virtual ~ObjectActionInfoList();
public:
	CREATE_FUNC_PARAM(ObjectActionInfoList, const rapidjson::Value&, json);
#if defined(AGTK_DEBUG)
	void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
private:
	CC_SYNTHESIZE(int, _actionId, ActionId);
	CC_SYNTHESIZE(bool, _disabled, Disabled);
	CC_SYNTHESIZE(int, _id, Id);
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API ObjectPhysicsSettingData : public cocos2d::Ref
{
private:
	ObjectPhysicsSettingData();
	virtual ~ObjectPhysicsSettingData();
public:
	CREATE_FUNC_PARAM(ObjectPhysicsSettingData, const rapidjson::Value&, json);
#if defined(AGTK_DEBUG)
	void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
private:
	CC_SYNTHESIZE(bool, _physicsAffected, PhysicsAffected);//bool
	CC_SYNTHESIZE(bool, _affectPhysics, AffectPhysics);//bool
	CC_SYNTHESIZE(bool, _followConnectedPhysics, FollowConnectedPhysics);//bool
	CC_SYNTHESIZE(int, _templateId, TemplateId);//int
	CC_SYNTHESIZE(double, _density, Density);//double
	CC_SYNTHESIZE(double, _mass, Mass);//double
	CC_SYNTHESIZE(double, _friction, Friction);//double
	CC_SYNTHESIZE(double, _repulsion, Repulsion);//double
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _nonCollisionGroup, NonCollisionGroup);//->cocos2d::Integer
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API ObjectViewportLightSettingData : public cocos2d::Ref
{
public:
	//位置の種類
	enum EnumPositionType {
		kPositionCenter,//このオブジェクトの中心
		kPositionFoot,//このオブジェクトの足元
		kPositionUseConnection,//接続点を使用
		kPositionMax
	};
private:
	ObjectViewportLightSettingData();
	virtual ~ObjectViewportLightSettingData();
public:
	CREATE_FUNC_PARAM(ObjectViewportLightSettingData, const rapidjson::Value&, json);
	const char *getName();
#if defined(AGTK_DEBUG)
	void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
private:
	CC_SYNTHESIZE(unsigned int, _id, Id);
	CC_SYNTHESIZE_RETAIN(cocos2d::__String *, _name, Name);
	//------------------------------------------------------------------------------------------------------------
	//▼基本設定
	CC_SYNTHESIZE(bool, _viewport, Viewport);//視界・照明を設定する(視界:true, 照明:false)
	CC_SYNTHESIZE(bool, _objectSwitch, ObjectSwitch);//このオブジェクトのスイッチ（true:このオブジェクトのスイッチ、false:Resources 共通）
	CC_SYNTHESIZE(int, _objectSwitchId, ObjectSwitchId);//このオブジェクトのスイッチID
	CC_SYNTHESIZE(int, _systemSwitchId, SystemSwitchId);//Resources 共通のスイッチID
	//------------------------------------------------------------------------------------------------------------
	//▼視界・照明の形状
	CC_SYNTHESIZE(int, _angleRange, AngleRange);//幅（中心角）※0～360°
	CC_SYNTHESIZE(int, _radius, Radius);//距離（半径）※ドット数
	CC_SYNTHESIZE(int, _scaleX, ScaleX);//スケールX(%)
	CC_SYNTHESIZE(int, _scaleY, ScaleY);//スケールY(%)
	CC_SYNTHESIZE(double, _rotation, Rotation);//回転
	CC_SYNTHESIZE(bool, _coloring, Coloring);//指定色で塗るフラグ
	CC_SYNTHESIZE(int, _a, A);//不透明度
	CC_SYNTHESIZE(int, _r, R);//R値
	CC_SYNTHESIZE(int, _g, G);//G値
	CC_SYNTHESIZE(int, _b, B);//B値
	CC_SYNTHESIZE(bool, _intensityFlag, IntensityFlag);//明るさを調整フラグ
	CC_SYNTHESIZE(int, _intensityOffset, IntensityOffset);//明るさを調整（0～100）
	CC_SYNTHESIZE(bool, _defocusCircumferenceFlag, DefocusCircumferenceFlag);//円周をぼかすフラグ
	CC_SYNTHESIZE(int, _defocusCircumference, DefocusCircumference);//円周をぼかす（0～100）
	CC_SYNTHESIZE(bool, _defocusSideFlag, DefocusSideFlag);//側面をぼかすフラグ
	CC_SYNTHESIZE(int, _defocusSide, DefocusSide);//側面をぼかす（0～100）
	//------------------------------------------------------------------------------------------------------------
	//▼視界・照明の接続
	CC_SYNTHESIZE(EnumPositionType, _positionType, PositionType);//位置タイプ（kPositionCenter:このオブジェクトの中心、kPositionFoot:このオブジェクトの足元、kPositionUseConnection:接続点を使用）
	CC_SYNTHESIZE(int, _connectionId, ConnectionId);//接続点を使用の場合の接続点ID
	CC_SYNTHESIZE(int, _adjustX, AdjustX);//位置を調整X
	CC_SYNTHESIZE(int, _adjustY, AdjustY);//位置を調整Y
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API ObjectCommonActionSettingData : public cocos2d::Ref
{
private:
	ObjectCommonActionSettingData();
	virtual ~ObjectCommonActionSettingData();
public:
	CREATE_FUNC_PARAM(ObjectCommonActionSettingData, const rapidjson::Value&, json);
#if defined(AGTK_DEBUG)
	void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
private:
	CC_SYNTHESIZE(unsigned int, _id, Id);
	CC_SYNTHESIZE_RETAIN(ObjectActionLinkData *, _objActionLink, ObjActionLink);
	CC_SYNTHESIZE_RETAIN(ObjectActionData *, _objAction, ObjAction);
	CC_SYNTHESIZE_RETAIN(ObjectActionLinkData *, _objActionLink2, ObjActionLink2);
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API ObjectCommonActionGroupData : public cocos2d::Ref
{
private:
	ObjectCommonActionGroupData();
	virtual ~ObjectCommonActionGroupData();
public:
	CREATE_FUNC_PARAM(ObjectCommonActionGroupData, const rapidjson::Value&, json);
	const char *getName();
#if defined(AGTK_DEBUG)
	void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
private:
	CC_SYNTHESIZE(unsigned int, _id, Id);
	CC_SYNTHESIZE_RETAIN(cocos2d::__String *, _name, Name);
	CC_SYNTHESIZE(int, _a, A);
	CC_SYNTHESIZE(int, _r, R);
	CC_SYNTHESIZE(int, _g, G);
	CC_SYNTHESIZE(int, _b, B);
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _actionIdList, ActionIdList);//->cocos2d::Integer
	CC_SYNTHESIZE(int, _x, X);
	CC_SYNTHESIZE(int, _y, Y);
	CC_SYNTHESIZE(int, _width, Width);
	CC_SYNTHESIZE(int, _height, Height);
	CC_SYNTHESIZE(bool, _collapsed, Collapsed);
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API ObjectAroundCharacterViewSettingData : public cocos2d::Ref
{
public:
	enum EnumViewType
	{
		kDrawSOLID,					//ベタ塗り
		kArroundTransparentCircle,	//周辺透過：円
		kArroundTransparentRect		//周辺透過：矩形
	};
private:
	ObjectAroundCharacterViewSettingData();
	virtual ~ObjectAroundCharacterViewSettingData();
public:
	CREATE_FUNC_PARAM(ObjectAroundCharacterViewSettingData, const rapidjson::Value&, json);
#if defined(AGTK_DEBUG)
	void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
private:
	CC_SYNTHESIZE(bool, _tileOn, TileOn);//bool
	CC_SYNTHESIZE(bool, _objectOn, ObjectOn);//bool
	CC_SYNTHESIZE(EnumViewType, _viewType, ViewType);//int
	CC_SYNTHESIZE(bool, _multiplyFill, MultiplyFill);//bool
	CC_SYNTHESIZE(int, _fillA, FillA);//int
	CC_SYNTHESIZE(int, _fillR, FillR);//int
	CC_SYNTHESIZE(int, _fillG, FillG);//int
	CC_SYNTHESIZE(int, _fillB, FillB);//int
	CC_SYNTHESIZE(int, _transparency, Transparency);//int
	CC_SYNTHESIZE(int, _width, Width);//int
	CC_SYNTHESIZE(int, _height, Height);//int
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API ObjectInvincibleSettingData : public cocos2d::Ref
{
private:
	ObjectInvincibleSettingData();
	virtual ~ObjectInvincibleSettingData();
public:
	CREATE_FUNC_PARAM(ObjectInvincibleSettingData, const rapidjson::Value&, json);
	const char *getName();
#if defined(AGTK_DEBUG)
	void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
private:
	//------------------------------------------------------------------------------------------------------------
	//▼基本設定
	CC_SYNTHESIZE(unsigned int, _id, Id);//ID
	CC_SYNTHESIZE_RETAIN(cocos2d::__String *, _name, Name);//名前
	CC_SYNTHESIZE(int, _duration300, Duration300);//無敵時間を設定（秒）
	CC_SYNTHESIZE(bool, _infinite, Infinite);//無限
	//------------------------------------------------------------------------------------------------------------
	//▼無敵の表示設定
	//オブジェクトのフィルター効果
	CC_SYNTHESIZE(bool, _filterEffectFlag, FilterEffectFlag);//フィルター効果フラグ
	CC_SYNTHESIZE_RETAIN(FilterEffect *, _filterEffect, FilterEffect);//オブジェクトのフィルター効果
	//------------------------------------------------------------------------------------------------------------
	//オブジェクトの点滅
	CC_SYNTHESIZE(bool, _wink, Wink);//無敵中オブジェクトが点滅フラグ
	CC_SYNTHESIZE(int, _winkInterval300, WinkInterval300);//点滅間隔（秒）
	//終了時にオブジェクトが点滅
	CC_SYNTHESIZE(bool, _finishWink, FinishWink);//終了時にオブジェクトが点滅フラグ
	CC_SYNTHESIZE(int, _finishWinkInterval300, FinishWinkInterval300);//点滅間隔（秒）
	CC_SYNTHESIZE(int, _finishWinkDuration300, FinishWinkDuration300);//点滅期間（秒）
	//------------------------------------------------------------------------------------------------------------
	//▼その他設定
	CC_SYNTHESIZE(bool, _wallAreaAttack, WallAreaAttack);//当たり判定を攻撃判定にする
	CC_SYNTHESIZE(bool, _playBgm, PlayBgm);//無敵中BGMを変更するフラグ
	CC_SYNTHESIZE(int, _bgmId, BgmId);//無敵中BGMのID
	//有効・無効用のスイッチを指定
	CC_SYNTHESIZE(bool, _objectSwitch, ObjectSwitch);//オブジェクトスイッチフラグ（false:Resource共通, true:このオブジェクトのスイッチ）
	CC_SYNTHESIZE(int, _objectSwitchId, ObjectSwitchId);//このオブジェクトのスイッチ
	CC_SYNTHESIZE(int, _systemSwitchId, SystemSwitchId);//Resources共通
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API ObjectDamagedSettingData : public cocos2d::Ref
{
private:
	ObjectDamagedSettingData();
	virtual ~ObjectDamagedSettingData();
public:
	CREATE_FUNC_PARAM(ObjectDamagedSettingData, const rapidjson::Value&, json);
	const char *getName();
	const char *getDamagedScript();
#if defined(AGTK_DEBUG)
	void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
private:
	CC_SYNTHESIZE(unsigned int, _id, Id);
	CC_SYNTHESIZE_RETAIN(cocos2d::__String *, _name, Name);
	//------------------------------------------------------------------------------------------------------------
	//▼基本設定
	CC_SYNTHESIZE(bool, _damagedRateFlag, DamagedRateFlag);//被ダメージ率の係数フラグ（true:被ダメージ率の係数（％）、false:ダメージをスクリプトで記述）
	CC_SYNTHESIZE(int, _damagedRate, DamagedRate);//被ダメージ率の係数（％）
	CC_SYNTHESIZE_RETAIN(cocos2d::String *, _damagedScript, DamagedScript);//ダメージをスクリプトで記述
	CC_SYNTHESIZE(bool, _critical, Critical);//クリティカル発生率を変更フラグ
	CC_SYNTHESIZE(int, _criticalRate, CriticalRate);//クリティカル発生率（％）
	//------------------------------------------------------------------------------------------------------------
	//▼対象とする攻撃の属性設定を指定
	CC_SYNTHESIZE(int, _attributeType, AttributeType);//属性タイプ指定（0:属性指定無し、1:プリセットの属性、2:属性値で指定）
	CC_SYNTHESIZE(int, _attributePresetId, AttributePresetId);//プリセットの属性
	CC_SYNTHESIZE(int, _attributeValue, AttributeValue);//属性値を指定
	CC_SYNTHESIZE(bool, _attributeEqual, AttributeEqual);//属性値の演算（=,!=）
	//------------------------------------------------------------------------------------------------------------
	//▼被ダメージ時の表示設定
	CC_SYNTHESIZE(double, _duration300, Duration300);//表示時間を設定（秒）
	CC_SYNTHESIZE(bool, _filterEffectFlag, FilterEffectFlag);//オブジェクトのフィルター効果フラグ
	CC_SYNTHESIZE_RETAIN(FilterEffect *, _filterEffect, FilterEffect);//オブジェクトのフィルター効果
	CC_SYNTHESIZE(bool, _wink, Wink);//演出中オブジェクトが点滅フラグ
	CC_SYNTHESIZE(int, _winkInterval300, WinkInterval300);//点滅間隔（秒）
	//------------------------------------------------------------------------------------------------------------
	//▼ヒットストップの設定
	CC_SYNTHESIZE(double, _dioGameSpeed, DioGameSpeed);				//ゲームスピード
	CC_SYNTHESIZE(double, _dioEffectDuration, DioEffectDuration);	//効果時間
	CC_SYNTHESIZE(bool, _dioReceiving, DioReceiving);				//ダメージを受けたオブジェクト
	CC_SYNTHESIZE(bool, _dioRecvParent, DioRecvParent);				//ダメージを受けたオブジェクト：親オブジェクトを含む
	CC_SYNTHESIZE(bool, _dioRecvChild, DioRecvChild);				//ダメージを受けたオブジェクト：子オブジェクトを含む
	CC_SYNTHESIZE(bool, _dioDealing, DioDealing);					//ダメージを与えたオブジェクト
	CC_SYNTHESIZE(bool, _dioDealParent, DioDealParent);				//ダメージを与えたオブジェクト：親オブジェクトを含む
	CC_SYNTHESIZE(bool, _dioDealChild, DioDealChild);				//ダメージを与えたオブジェクト：子オブジェクトを含む
	//------------------------------------------------------------------------------------------------------------
	//▼その他
	CC_SYNTHESIZE(bool, _playeSe, PlaySe);//SEを再生フラグ
	CC_SYNTHESIZE(int, _seId, SeId);//SEID
	CC_SYNTHESIZE(bool, _objectSwitch, ObjectSwitch);//被ダメージ設定の有効・無効用のスイッチを指定（true:このオブジェクト,false:Resources共通）
	CC_SYNTHESIZE(int, _objectSwitchId, ObjectSwitchId);//このオブジェクトのスイッチ
	CC_SYNTHESIZE(int, _systemSwitchId, SystemSwitchId);//Resources共通のスイッチ
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API ObjectMoveRestrictionSettingData : public cocos2d::Ref
{
public:

private:
	ObjectMoveRestrictionSettingData();
	virtual ~ObjectMoveRestrictionSettingData();
public:
	CREATE_FUNC_PARAM(ObjectMoveRestrictionSettingData, const rapidjson::Value&, json);
	const char *getName();
#if defined(AGTK_DEBUG)
	void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
private:
	CC_SYNTHESIZE(int, _id, Id);
	CC_SYNTHESIZE_RETAIN(cocos2d::__String *, _name, Name);
	CC_SYNTHESIZE(int, _left, Left);									// 左
	CC_SYNTHESIZE(int, _right, Right);									// 右
	CC_SYNTHESIZE(int, _top, Top);										// 上
	CC_SYNTHESIZE(int, _bottom, Bottom);								// 下
	CC_SYNTHESIZE(bool, _objectSwitch, ObjectSwitch);					// このオブジェクトのスイッチを使用フラグ
	CC_SYNTHESIZE(int, _objectSwitchId, ObjectSwitchId);				// このオブジェクトのスイッチID
	CC_SYNTHESIZE(int, _systemSwitchId, SystemSwitchId);				// player共通スイッチID
};

//-------------------------------------------------------------------------------------------------------------------
/**
* オブジェクト消滅後、復活するのに必要なデータを保持するためのクラス
*/
class AGTKPLAYER_API ObjectReappearData : public cocos2d::Ref
{
private:
	ObjectReappearData();
	virtual ~ObjectReappearData();
public:
	CREATE_FUNC(ObjectReappearData);
#if defined(AGTK_DEBUG)
	void dump();
#endif

private:
	virtual bool init();
private:
	CC_SYNTHESIZE(int, _sceneId, SceneId);
	CC_SYNTHESIZE(unsigned int, _scenePartsId, ScenePartsId);
	CC_SYNTHESIZE(int, _sceneLayerId, SceneLayerId);
	CC_SYNTHESIZE(int, _objectId, ObjectId);
	CC_SYNTHESIZE(int, _initialActionId, InitialActionId);
	CC_SYNTHESIZE(cocos2d::Vec2, _initialPosition, InitialPosition);
	CC_SYNTHESIZE(cocos2d::Vec2, _initialScale, InitialScale);
	CC_SYNTHESIZE(float, _initialRotation, InitialRotation);
	CC_SYNTHESIZE(int, _initialMoveDirectionId, InitialMoveDirectionId);
	CC_SYNTHESIZE(bool, _reappearFlag, ReappearFlag);
	CC_SYNTHESIZE(int, _initialCourseId, InitialCourseId);
	CC_SYNTHESIZE(int, _initialCoursePointId, InitialCoursePointId);
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API ObjectData : public cocos2d::Ref
{
public:
	enum EnumObjGroup {
		kObjGroupAll=-1,
		kObjGroupPlayer,
		kObjGroupEnemy,
		kObjGroupStageGimmick,
		kObjGroupMax
	};
	enum EnumMoveType {
		kMoveNormal,
		kMoveTank,
		kMoveCar,
		kMoveMax
	};
	enum EnumFollowType {
		kFollowNone,
		kFollowClose,
		kFollowNearTime,
		kFollowNearDot,
		kFollowMax
	};

	// 子オブジェクト時の被ダメージ
	enum EnumChildDamageType {
		kChildDamageOwn,	// 自身の体力が変化
		kChildDamageParent, //対象を親オブジェクトに変更
		kChildDamageMax
	};

	enum EnumAppearCondition {
		kAppearConditionCameraNear,
		kAppearConditionAlways,
		kAppearConditionMax
	};

	enum EnumReappearCondition {
		kReappearConditionCameraFar,
		kReappearConditionSceneChange,
		kReappearConditionByCommand,	//アクションで復活
		kReappearConditionNone,
		kReappearConditionMax
	};
private:
	ObjectData();
	virtual ~ObjectData();
public:
	CREATE_FUNC_PARAM(ObjectData, const rapidjson::Value&, json);
	const char *getName();
	ObjectActionData *getActionData(int id);
	ObjectActionLinkData *getActionLinkData(int id);
	ObjectActionGroupData *getActionGroupData(int id);
	ObjectFireBulletSettingData *getFireBulletSettingData(int id);
	ObjectViewportLightSettingData *getViewportLightSettingData(int id);
	cocos2d::__Array *getVariableArray();
	VariableData *getVariableData(int id);
	VariableData *getVariableDataByName(const char *name);
	cocos2d::__Array *getSwitchArray();
	SwitchData *getSwitchData(int id);
	SwitchData *getSwitchDataByName(const char *name);
	bool isGroupPlayer()const;
	bool isGroupEnemy()const;
	int getGroupBit()const;
	//! @brief 壁判定を行なってよい相手か？
	bool isCollideWith(ObjectData* data)const;
	bool isCollideWithObjectGroup(int group)const;

#if defined(AGTK_DEBUG)
	void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
private:
	CC_SYNTHESIZE(unsigned int, _id, Id);//int
	CC_SYNTHESIZE_RETAIN(cocos2d::__String *, _name, Name);//string
	CC_SYNTHESIZE(EnumObjGroup, _group, Group);//オブジェクトグループ
	CC_SYNTHESIZE(int, _animationId, AnimationId);//アニメーションを選択
	CC_SYNTHESIZE(bool, _operatable, Operatable);//入力デバイスで操作するオブジェクト
	CC_SYNTHESIZE(int, _priority, Priority);//処理の優先度
	CC_SYNTHESIZE(int, _dispPriority, DispPriority);//値が大きいほどレイヤー内で手前に表示されます。
	CC_SYNTHESIZE(int, _hitObjectGroupBit, HitObjectGroupBit);//このオブジェクトの攻撃判定が当たるオブジェクトグループ
	CC_SYNTHESIZE(int, _collideWithObjectGroupBit, CollideWithObjectGroupBit);//このオブジェクトの壁判定が当たるオブジェクトグループ
	CC_SYNTHESIZE(int, _collideWithTileGroupBit, CollideWithTileGroupBit);//このオブジェクトの壁判定が当たるタイルグループ
	//-----------------------------------------------------------------------------------------------------------------------
	// アクションプログラム
	CC_SYNTHESIZE_RETAIN(cocos2d::__Dictionary *, _actionList, ActionList);//->ObjectActionData
	CC_SYNTHESIZE_RETAIN(cocos2d::__Dictionary *, _actionLinkList, ActionLinkList);//->OjbectActionLinkData
	CC_SYNTHESIZE_RETAIN(cocos2d::__Dictionary *, _actionGroupList, ActionGroupList);//->ObjectActionGroupData
	CC_SYNTHESIZE_RETAIN(cocos2d::__String *, _memo, Memo);//メモ
	//-----------------------------------------------------------------------------------------------------------------------
	// 基本設定
	CC_SYNTHESIZE(int, _initialActionId, InitialActionId);//配置時の初期アクション
	CC_SYNTHESIZE(bool, _initialDisplayDirectionFlag, InitialDisplayDirectionFlag);//初期表示方向を設定（フラグ）
	CC_SYNTHESIZE(double, _initialDisplayDirection, InitialDisplayDirection);//初期表示方向を設定
	CC_SYNTHESIZE(bool, _testplayOnly, TestplayOnly);//テストプレイでのみ有効なオブジェクトとする　※チェックを入れると、ビルドしたゲームでこのオブジェクトは無効化されます。
	CC_SYNTHESIZE(EnumAppearCondition, _appearCondition, AppearCondition);//オブジェクトの出現条件
	CC_SYNTHESIZE(int, _appearConditionTileCount, AppearConditionTileCount);//カメラまでのタイル数（条件：カメラが近づいたら kAppearConditionCameraNear）
	CC_SYNTHESIZE(EnumReappearCondition, _reappearCondition, ReappearCondition);//オブジェクト消滅後の復活条件
	CC_SYNTHESIZE(int, _reappearConditionTileCount, ReappearConditionTileCount);//カメラまでのタイル数（条件：カメラ外にでたら）
	CC_SYNTHESIZE(int, _hp, Hp);//体力
	CC_SYNTHESIZE(int, _maxHp, MaxHp);//最大体力
	CC_SYNTHESIZE(int, _minAttack, MinAttack);//最小攻撃力
	CC_SYNTHESIZE(int, _maxAttack, MaxAttack);//最大攻撃力
	CC_SYNTHESIZE(int, _initialDamageRate, InitialDamageRate);//初期被ダメージ率(%)
	CC_SYNTHESIZE(int, _damageVariationValue, DamageVariationValue);//変動値
	CC_SYNTHESIZE(bool, _critical, Critical);//攻撃にクリティカルを設定
	CC_SYNTHESIZE(double, _criticalRatio, CriticalRatio);//クリティカル倍率（％）
	CC_SYNTHESIZE(double, _criticalIncidence, CriticalIncidence);//クリティカル発生率（％）
	CC_SYNTHESIZE(bool, _invincibleOnDamaged, InvincibleOnDamaged);//ダメージを受けた時一定時間無敵にする
	CC_SYNTHESIZE(int, _invincibleDuration300, InvincibleDuration300);//無敵時間を設定（秒）
	CC_SYNTHESIZE(bool, _winkWhenInvincible, WinkWhenInvincible);//無敵中オブジェクトが点滅
	CC_SYNTHESIZE(int, _winkInterval300, WinkInterval300);//点滅間隔(秒)
	CC_SYNTHESIZE(bool, _disappearWhenHp0, DisappearWhenHp0);//体力がなくなったら強制的に消滅
	CC_SYNTHESIZE(bool, _disappearWhenOutOfCamera, DisappearWhenOutOfCamera); // カメラ外にでたら強制的に消滅
	CC_SYNTHESIZE(int, _disappearCameraMarginTileCount, DisappearCameraMarginTileCount); // カメラ外にでたら強制的に消滅 ※カメラまでのタイル数
	CC_SYNTHESIZE(bool, _fixedInCamera, FixedInCamera);//カメラとの位置関係を固定する
	CC_SYNTHESIZE(bool, _pushedbackByObject, PushedbackByObject);//他オブジェクトから押し戻されない
	CC_SYNTHESIZE(bool, _takeoverStatesAtSceneEnd, TakeoverStatesAtSceneEnd);//シーン終了時の状態を維持
	//-----------------------------------------------------------------------------------------------------------------------
	// 移動とジャンプ
	CC_SYNTHESIZE(EnumMoveType, _moveType, MoveType);//移動の種類(基本移動,戦車タイプ移動,車タイプ移動）
	CC_SYNTHESIZE(bool, _normalAccelMove, NormalAccelMove);//基本移動の加速移動
	CC_SYNTHESIZE(bool, _tankAccelMove, TankAccelMove);//戦車タイプ移動の加速移動
	CC_SYNTHESIZE(bool, _carAccelMove, CarAccelMove);//車タイプ移動の加速移動
	CC_SYNTHESIZE(bool, _slipOnSlope, SlipOnSlope);//坂ですべる
	CC_SYNTHESIZE(bool, _diagonalMoveWithPolar, DiagonalMoveWithPolar);//斜め移動を極座標で計算
	CC_SYNTHESIZE(bool, _moveAroundWithinDots, MoveAroundWithinDots);//指定ドット以内の引掛りを自動調整(有効フラグ)
	CC_SYNTHESIZE(int, _dotsToMoveAround, DotsToMoveAround);//指定ドット以内の引掛りを自動調整（ドット数）
	// 操作キー設定
	CC_SYNTHESIZE(int, _upMoveKeyId, UpMoveKeyId);//上移動（操作入力キーID）
	CC_SYNTHESIZE(int, _downMoveKeyId, DownMoveKeyId);//下移動（操作入力キーID）
	CC_SYNTHESIZE(int, _leftMoveKeyId, LeftMoveKeyId);//左移動（操作入力キーID）
	CC_SYNTHESIZE(int, _rightMoveKeyId, RightMoveKeyId);//右移動（操作入力キーID）
	CC_SYNTHESIZE(int, _forwardMoveKeyId, ForwardMoveKeyId);//前方向への移動（操作入力キーID）
	CC_SYNTHESIZE(int, _backwardMoveKeyId, BackwardMoveKeyId);//後方向への移動（操作入力キーID）
	CC_SYNTHESIZE(int, _leftTurnKeyId, LeftTurnKeyId);//左旋回（操作入力キーID）
	CC_SYNTHESIZE(int, _rightTurnKeyId, RightTurnKeyId);//右旋回（操作入力キーID）
	//▼通常移動に関するパラメータの設定
	//基本移動のパラメータ：[移動量]を大きくすると、オブジェクトの移動が速くなります
	CC_SYNTHESIZE(double, _horizontalMove, HorizontalMove);//左右の移動量
	CC_SYNTHESIZE(double, _verticalMove, VerticalMove);//上下の移動量
	//加速移動のパラメータ：[加速量]を大きくすると、オブジェクトが移動を開始してから「最大移動量」へ達するまでが速くなります。
	//　　　　　　　　　　　[減速量]を大きくすると、オブジェクトが移動の入力をやめてから「完全に停止」するまでが速くなります。
	CC_SYNTHESIZE(double, _horizontalAccel, HorizontalAccel);//左右の加速量
	CC_SYNTHESIZE(double, _horizontalMaxMove, HorizontalMaxMove);//左右の最大移動量
	CC_SYNTHESIZE(double, _horizontalDecel, HorizontalDecel);//左右の減速量
	CC_SYNTHESIZE(double, _verticalAccel, VerticalAccel);//上下の加速量
	CC_SYNTHESIZE(double, _verticalMaxMove, VerticalMaxMove);//上下の最大移動量
	CC_SYNTHESIZE(double, _verticalDecel, VerticalDecel);//上下の減速量
	//▼前後移動、旋回に関するパラメーター
	//基本移動パラメータ：[移動量]を大きくすると、オブジェクトの移動が速くなります。
	CC_SYNTHESIZE(double, _forwardMove, ForwardMove);//前方への移動量
	CC_SYNTHESIZE(double, _backwardMove, BackwardMove);//後方への移動量
	//加速移動のパラメータ：[加速量]を大きくすると、オブジェクトが移動を開始してから[最大移動量]へ達するまでが速くなります。
	//　　　　　　　　　　：[減速量]を大きくすると、オブジェクトが移動の入力をやめてから[完全に停止]するまでが速くなります。
	CC_SYNTHESIZE(double, _forwardAccel, ForwardAccel);//前方への加速量
	CC_SYNTHESIZE(double, _forwardMaxMove, ForwardMaxMove);//前方への最大移動量
	CC_SYNTHESIZE(double, _forwardDecel, ForwardDecel);//前方への減速量
	CC_SYNTHESIZE(double, _backwardAccel, BackwardAccel);//後方への加速量
	CC_SYNTHESIZE(double, _backwardMaxMove, BackwardMaxMove);//後方への最大移動量
	CC_SYNTHESIZE(double, _backwardDecel, BackwardDecel);//後方への減速量
	//旋回のパラメータ：[回転量]を大きくすると、オブジェクトの旋回が速くなります。
	CC_SYNTHESIZE(double, _leftTurn, LeftTurn);//左旋回の回転量
	CC_SYNTHESIZE(double, _rightTurn, RightTurn);//右旋回の回転量
	//ジャンプと落下に関するパラメーター
	//[ジャンプの初速]を大きくすると、オブジェクトのジャンプが速くなります。
	//[重力]を大きくすると、オブジェクトの滞空時間が短くなります。0を設定すると無重力になります。
	CC_SYNTHESIZE(double, _jumpInitialSpeed, JumpInitialSpeed);//ジャンプの初速
	CC_SYNTHESIZE(double, _gravity, Gravity);//重力の影響
	CC_SYNTHESIZE(bool, _movableWhenJumping, MovableWhenJumping);//ジャンプ中の軌道修正を不可
	CC_SYNTHESIZE(bool, _movableWhenFalling, MovableWhenFalling);//落下中の軌道修正を不可
	CC_SYNTHESIZE(bool, _fallOnCollideWithWall, FallOnCollideWithWall);//Begin falling when a walli hit
	CC_SYNTHESIZE(bool, _fallOnCollideWithHitbox, FallOnCollideWithHitbox)//begin falling when another object is hit
	CC_SYNTHESIZE(int, _jumpInputOperationKeyId, JumpInputOperationKeyId);//入力用の操作キー 
	CC_SYNTHESIZE(int, _affectInputDuration300, AffectInputDuration300);//入力の受付時間を設定（※０秒で入力が終了するまで受け付ける）
	//落下最大移動量
	CC_SYNTHESIZE(bool, _limitFalldownAmount, LimitFalldownAmount);//落下の最大移動量フラグ
	CC_SYNTHESIZE(double, _maxFalldownAmount, MaxFalldownAmount);//落下の最大移動量
	//-----------------------------------------------------------------------------------------------------------------------
	// 表示と親子関係
	//▼表示関連
	CC_SYNTHESIZE(bool, _afterimage, Afterimage);//残像を表示
	CC_SYNTHESIZE(int, _afterimageCount, AfterimageCount);//残像数
	CC_SYNTHESIZE(double, _afterimageInterval, AfterimageInterval);//残像の間隔
	CC_SYNTHESIZE(int, _afterimageDuration300, AfterimageDuration300);//残像の表示時間
	CC_SYNTHESIZE(bool, _fillAfterimage, FillAfterimage);//指定色で塗る（フラグ）
	CC_SYNTHESIZE(unsigned char, _fillAfterimageA, FillAfterimageA);//指定色で塗る（α）
	CC_SYNTHESIZE(unsigned char, _fillAfterimageR, FillAfterimageR);//指定色で塗る（赤）
	CC_SYNTHESIZE(unsigned char, _fillAfterimageG, FillAfterimageG);//指定色で塗る（緑）
	CC_SYNTHESIZE(unsigned char, _fillAfterimageB, FillAfterimageB);//指定色で塗る（青）
	CC_SYNTHESIZE(bool, _fadeoutAfterimage, FadeoutAfterimage);//残像をフェードアウト（フラグ）
	CC_SYNTHESIZE(double, _fadeoutAfterimageStart300, FadeoutAfterimageStart300);//残像をフェードアウト（開始時間）
	CC_SYNTHESIZE(int, _useAnimationAfterimage, UseAnimationAfterimage);//アニメーションを残像として使用
	CC_SYNTHESIZE(int, _afterimageAnimationId, AfterimageAnimationId);//アニメーションID
	//▼オブジェクトの親子関係に関するパラメータ
	CC_SYNTHESIZE(EnumFollowType, _followType, FollowType);//子オブジェクト時の移動（親オブジェクトを追従しない、親オブジェクトから離れず追従する、親オブジェクトと一定間隔開けて追従する）
	CC_SYNTHESIZE(int, _followPrecision, FollowPrecision);//追従精度
	CC_SYNTHESIZE(double, _followIntervalByTime300, FollowIntervalByTime300);//親オブジェクトと一定間隔開けて追従する（間隔を設定：時間で設定）
	CC_SYNTHESIZE(double, _followIntervalByLocusLength, FollowIntervalByLocusLength);//親オブジェクトと一定間隔開けて追従する（間隔を設定：移動量で設定（ドット））
	CC_SYNTHESIZE(EnumChildDamageType, _childDamageType, ChildDamageType);//子オブジェクト時の被ダメージ
	CC_SYNTHESIZE(bool, _takeOverDamageRateToParent, TakeOverDamageRateToParent);//自身の被ダメージ率を親に継承
	CC_SYNTHESIZE(bool, _unattackableToParent, UnattackableToParent);//子オブジェクト時の攻撃判定
	CC_SYNTHESIZE(bool, _takeoverDispDirection, TakeoverDispDirection);//親オブジェクトから引き継ぐ要素（表示方向）
	CC_SYNTHESIZE(bool, _takeoverAngle, TakeoverAngle);//親オブジェクトから引き継ぐ要素（角度）
	CC_SYNTHESIZE(bool, _takeoverScaling, TakeoverScaling);//親オブジェクトから引き継ぐ要素（スケール）
	CC_SYNTHESIZE(bool, _takeoverIntensity, TakeoverIntensity);//親オブジェクトから引き継ぐ要素（輝度）
	CC_SYNTHESIZE(bool, _disappearSettingFlag, DisappearSettingFlag);//bool
	CC_SYNTHESIZE_RETAIN(ObjectDisappearSettingData *, _disappearSetting, DisappearSetting);//->ObjectDisappearSettingData
	CC_SYNTHESIZE(bool, _fireBulletSettingFlag, FireBulletSettingFlag);//bool
	CC_SYNTHESIZE_RETAIN(cocos2d::__Dictionary *, _fireBulletSettingList, FireBulletSettingList);//->ObjectFireBulletSettingData
	CC_SYNTHESIZE(bool, _effectSettingFlag, EffectSettingFlag);//bool
	CC_SYNTHESIZE_RETAIN(cocos2d::__Dictionary *, _effectSettingList, EffectSettingList);//->ObjectEffectSettingData
	CC_SYNTHESIZE(bool, _connectSettingFlag, ConnectSettingFlag);//bool
	CC_SYNTHESIZE_RETAIN(cocos2d::__Dictionary *, _connectSettingList, ConnectSettingList);//->ObjectConnectSettingData
	CC_SYNTHESIZE_RETAIN(cocos2d::__Dictionary *, _additionalDisplayList, AdditionalDisplayList);//->ObjectAdditionalDisplayData
	CC_SYNTHESIZE(bool, _moveDispDirectionSettingFlag, MoveDispDirectionSettingFlag);//bool
	CC_SYNTHESIZE_RETAIN(ObjectMoveDispDirectionSettingData *, _moveDispDirectionSetting, MoveDispDirectionSetting);//->ObjectMoveDispDirectionSettingData
	CC_SYNTHESIZE(bool, _physicsSettingFlag, PhysicsSettingFlag);//bool
	CC_SYNTHESIZE_RETAIN(ObjectPhysicsSettingData *, _physicsSetting, PhysicsSetting);//->PhysicsSettingData
	CC_SYNTHESIZE(bool, _viewportLightSettingFlag, ViewportLightSettingFlag);//bool
	CC_SYNTHESIZE_RETAIN(cocos2d::__Dictionary *, _viewportLightSettingList, ViewportLightSettingList);//->ObjectViewportLightSettingData
	CC_SYNTHESIZE(bool, _commonActionSettingFlag, CommonActionSettingFlag);//bool
	CC_SYNTHESIZE_RETAIN(cocos2d::__Dictionary *, _commonActionSettingList, CommonActionSettingList);//->ObjectCommonActionSettingData
	CC_SYNTHESIZE_RETAIN(ObjectCommonActionGroupData *, _commonActionGroup, CommonActionGroup);//->ObjectCommonActionGroupData
	CC_SYNTHESIZE(bool, _aroundCharacterViewSettingFlag, AroundCharacterViewSettingFlag);//bool
	CC_SYNTHESIZE_RETAIN(ObjectAroundCharacterViewSettingData *, _aroundCharacterViewSetting, AroundCharacterViewSetting);//->ObjectAroundCharacterViewSettingData
	CC_SYNTHESIZE(bool, _invincibleSettingFlag, InvincibleSettingFlag);//bool
	CC_SYNTHESIZE_RETAIN(cocos2d::__Dictionary *, _invincibleSettingList, InvincibleSettingList);//->ObjectInvincibleSettingData
	CC_SYNTHESIZE(bool, _damagedSettingFlag, DamagedSettingFlag);//bool
	CC_SYNTHESIZE_RETAIN(cocos2d::__Dictionary *, _damagedSettingList, DamagedSettingList);//->ObjectDamagedSettingData
	CC_SYNTHESIZE(bool, _moveRestrictionSettingFlag, MoveRestrictionSettingFlag);//bool
	CC_SYNTHESIZE_RETAIN(cocos2d::__Dictionary *, _moveRestrictionSettingList, MoveRestrictionSettingList);//->ObjectMoveRestrictionSettingData
	CC_SYNTHESIZE_RETAIN(cocos2d::__Dictionary *, _variableList, VariableList);//->VariableData
	CC_SYNTHESIZE_RETAIN(cocos2d::__Dictionary *, _switchList, SwitchList);//->SwitchData
	CC_SYNTHESIZE(bool, _folder, Folder);//bool
	CC_SYNTHESIZE_RETAIN(cocos2d::__Dictionary *, _children, Children);
	CC_SYNTHESIZE_RETAIN(cocos2d::__Dictionary *, _physicsPartList, PhysicsPartList);//->PhysicsPartData
	CC_SYNTHESIZE(int, _priorityInPhysics, PriorityInPhysics);//int

	CC_SYNTHESIZE(int, _databaseId, DatabaseId);

};

NS_DATA_END
NS_AGTK_END

#endif	//__AGTK_OBJECT_DATA_H__
