#ifndef __OTHERS_DATA_H__
#define __OTHERS_DATA_H__

#include "Lib/Macros.h"
#include "json/document.h"

NS_AGTK_BEGIN
NS_DATA_BEGIN


//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API OthersData : public cocos2d::Ref
{
public:
	class Point : public cocos2d::Ref
	{
	private:
		Point();
		virtual ~Point();
		virtual bool init(const rapidjson::Value& json);

	public:
		CREATE_FUNC_PARAM(Point, const rapidjson::Value&, json);

		CC_SYNTHESIZE(int, _id, Id);			// ID
		CC_SYNTHESIZE(double, _move, Move);		// 次のポイントまでの移動量
		CC_SYNTHESIZE(int, _offsetX, OffsetX);	// オフセットX
		CC_SYNTHESIZE(int, _offsetY, OffsetY);	// オフセットY
		CC_SYNTHESIZE(int, _switchId, SwitchId);// 対象スイッチID
		CC_SYNTHESIZE(int, _switchObjectId, SwitchObjectId);// 対象オブジェクトID
		CC_SYNTHESIZE(int, _switchQualifierId, SwitchQualifierId);// 対象オブジェクト制限

		CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _switchVariableAssignList, SwitchVariableAssignList);	// スイッチ、変数を変更
	};

protected:
	OthersData();
	virtual ~OthersData();

	virtual bool init(const rapidjson::Value& json);
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API OthersCameraData : public OthersData
{
public:
	// カメラの追従対象
	enum EnumFollowTargetType
	{
		kFollowTargetPlayer,	// プレイヤー
		kFollowTargetObject,	// オブジェクト
		kFollowTargetCourse,	// コース
		kFollowTargetNone,		// 無し
	};
	static const int CAMERA_SCROLL_AND_SCALE_THREASHOLD = 16; // カメラスクロールと拡縮の判定開始の閾値(pxel)
private:
	OthersCameraData();
	virtual ~OthersCameraData();
	virtual bool init(const rapidjson::Value& json);

public:
	CREATE_FUNC_PARAM(OthersCameraData, const rapidjson::Value&, json);

	// 初期設定
	CC_SYNTHESIZE(bool, _initialPlayerPosition, InitialPlayerPosition);	// カメラの初期位置：プレイヤー
	CC_SYNTHESIZE(int, _x, X);											// カメラの初期位置：座標で指定：X方向
	CC_SYNTHESIZE(int, _y, Y);											// カメラの初期位置：座標で指定：Y方向
	CC_SYNTHESIZE(int, _width, Width);									// カメラの表示サイズ：X方向
	CC_SYNTHESIZE(int, _height, Height);								// カメラの表示サイズ：Y方向

	// 動作設定
	CC_SYNTHESIZE(EnumFollowTargetType, _followTargetType, FollowTargetType);	// カメラの追従対象
	CC_SYNTHESIZE(int, _objectId, ObjectId);									// カメラの追従対象：オブジェクト
	CC_SYNTHESIZE(int, _courseScenePartId, CourseScenePartId);					// カメラの追従対象：コース
	CC_SYNTHESIZE(int, _startPointId, StartPointId);							// カメラの追従対象：コース：開始ポイント
	CC_SYNTHESIZE(bool, _scrollToShowAllPlayers, ScrollToShowAllPlayers);		// すべてのプレイヤーを収めるようスクロールする
	CC_SYNTHESIZE(bool, _scaleToShowAllPlayers, ScaleToShowAllPlayers);			// すべてのプレイヤーを収めるよう表示を拡縮する
	CC_SYNTHESIZE(double, _maxScaling, MaxScaling);								// 最大倍率を設定（％）


	// カメラを有効にするスイッチを設定
	CC_SYNTHESIZE(int, _switchObjectId, SwitchObjectId);		// 対象オブジェクトID
	CC_SYNTHESIZE(int, _switchQualifierId, SwitchQualifierId);	// 対象オブジェクト制限
	CC_SYNTHESIZE(int, _switchId, SwitchId);					// 対象のスイッチID
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API OthersLineCourseData : public OthersData
{
private:
	OthersLineCourseData();
	virtual ~OthersLineCourseData();
	virtual bool init(const rapidjson::Value& json);

public:
	CREATE_FUNC_PARAM(OthersLineCourseData, const rapidjson::Value&, json);

	// 
	CC_SYNTHESIZE(unsigned char, _r, R);	// カラー（赤）
	CC_SYNTHESIZE(unsigned char, _g, G);	// カラー（緑）
	CC_SYNTHESIZE(unsigned char, _b, B);	// カラー（青）
	CC_SYNTHESIZE(unsigned char, _a, A);	// カラー（α）

	CC_SYNTHESIZE(double, _x, X);			// 座標X
	CC_SYNTHESIZE(double, _y, Y);			// 座標Y

	// ループの設定
	CC_SYNTHESIZE(int, _loopCount, LoopCount);						// ループ回数
	CC_SYNTHESIZE(bool, _loopUnlimited, LoopUnlimited);				// ループ無限
	CC_SYNTHESIZE(bool, _connectEndAndStart, ConnectEndAndStart);	// 終点と始点をつなぐ
	CC_SYNTHESIZE(bool, _endReverseMove, EndReverseMove);			// 終点から反転して移動

	// 配置されているポイント
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _pointList, PointList);	// 配置されているポイント
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API OthersCurveCourseData : public OthersData
{
private:
	OthersCurveCourseData();
	virtual ~OthersCurveCourseData();
	virtual bool init(const rapidjson::Value& json);

public:
	CREATE_FUNC_PARAM(OthersCurveCourseData, const rapidjson::Value&, json);

	// 
	CC_SYNTHESIZE(unsigned char, _r, R);	// カラー（赤）
	CC_SYNTHESIZE(unsigned char, _g, G);	// カラー（緑）
	CC_SYNTHESIZE(unsigned char, _b, B);	// カラー（青）
	CC_SYNTHESIZE(unsigned char, _a, A);	// カラー（α）

	CC_SYNTHESIZE(double, _x, X);			// 座標X
	CC_SYNTHESIZE(double, _y, Y);			// 座標Y

	// ループの設定
	CC_SYNTHESIZE(int, _loopCount, LoopCount);						// ループ回数
	CC_SYNTHESIZE(bool, _loopUnlimited, LoopUnlimited);				// ループ無限
	CC_SYNTHESIZE(bool, _connectEndAndStart, ConnectEndAndStart);	// 終点から始点をつなぐ
	CC_SYNTHESIZE(bool, _endReverseMove, EndReverseMove);			// 終点から反転して移動


	// 配置されているポイント
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _pointList, PointList);	// 配置されているポイント
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API OthersCircleCourseData : public OthersData
{
private:
	OthersCircleCourseData();
	virtual ~OthersCircleCourseData();
	virtual bool init(const rapidjson::Value& json);

public:
	CREATE_FUNC_PARAM(OthersCircleCourseData, const rapidjson::Value&, json);

	// 
	CC_SYNTHESIZE(unsigned char, _r, R);		// カラー（赤）
	CC_SYNTHESIZE(unsigned char, _g, G);		// カラー（緑）
	CC_SYNTHESIZE(unsigned char, _b, B);		// カラー（青）
	CC_SYNTHESIZE(unsigned char, _a, A);		// カラー（α）

	CC_SYNTHESIZE(double, _x, X);				// 座標X
	CC_SYNTHESIZE(double, _y, Y);				// 座標Y
	CC_SYNTHESIZE(double, _radiusX, RadiusX);	// スケールX
	CC_SYNTHESIZE(double, _radiusY, RadiusY);	// スケールY

	// ループの設定
	CC_SYNTHESIZE(int, _loopCount, LoopCount);				// ループ回数
	CC_SYNTHESIZE(bool, _loopUnlimited, LoopUnlimited);		// ループ無限
	CC_SYNTHESIZE(bool, _endReverseMove, EndReverseMove);	// 始点まで戻ったら逆回転
	CC_SYNTHESIZE(bool, _reverseCourse, ReverseCourse);		// 逆回転

	// 配置されているポイント
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, pointList, PointList);	// 配置されているポイント
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API OthersSlopeData : public OthersData
{
private:
	OthersSlopeData();
	virtual ~OthersSlopeData();
	virtual bool init(const rapidjson::Value& json);

public:
	CREATE_FUNC_PARAM(OthersSlopeData, const rapidjson::Value&, json);

	// 表示の設定
	CC_SYNTHESIZE(double, _startX, StartX);	// 始点X
	CC_SYNTHESIZE(double, _startY, StartY);	// 始点Y
	CC_SYNTHESIZE(double, _endX, EndX);		// 終点X
	CC_SYNTHESIZE(double, _endY, EndY);		// 終点Y
	CC_SYNTHESIZE(int, _dispPriority, DispPriority);//値が大きいほどレイヤー内で手前に表示されます。

	// 坂専用パラメータ
	CC_SYNTHESIZE(bool, _passableFromUpper, PassableFromUpper);	// 上から通過可能
	CC_SYNTHESIZE(bool, _passableFromLower, PassableFromLower);	// 下から通過可能

	// 坂に簡易効果を設定
	CC_SYNTHESIZE(int, _objectGroupBit, ObjectGroupBit);				// オブジェクトのグループ
	CC_SYNTHESIZE(bool, _moveSpeedChanged, MoveSpeedChanged);			// プレイヤーキャラの移動速度を増減（フラグ）
	CC_SYNTHESIZE(double, _moveSpeedChange, MoveSpeedChange);			// プレイヤーキャラの移動速度を増減（値）
	CC_SYNTHESIZE(bool, _jumpChanged, JumpChanged);						// プレイヤーキャラのジャンプ力を増減（フラグ）
	CC_SYNTHESIZE(double, _jumpChange, JumpChange);						// プレイヤーキャラのジャンプ力を増減（値）
	CC_SYNTHESIZE(bool, _moveXFlag, MoveXFlag);							// オブジェクトをX方向に移動(±)（フラグ）
	CC_SYNTHESIZE(double, _moveX, MoveX);								// オブジェクトをX方向に移動(±)（値）
	CC_SYNTHESIZE(bool, _moveYFlag, MoveYFlag);							// オブジェクトをY方向に移動(±)（フラグ）
	CC_SYNTHESIZE(double, _moveY, MoveY);								// オブジェクトをY方向に移動(±)（値）
	CC_SYNTHESIZE(bool, _slipChanged, SlipChanged);						// プレイヤーの移動が滑るようになる（フラグ）
	CC_SYNTHESIZE(double, _slipChange, SlipChange);						// プレイヤーの移動が滑るようになる（値）
	CC_SYNTHESIZE(bool, _getDead, GetDead);								// プレイヤーキャラが死亡する
	CC_SYNTHESIZE(bool, _hpChanged, HpChanged);							// プレイヤーキャラの体力を増減（フラグ）
	CC_SYNTHESIZE(double, _hpChange, HpChange);							// プレイヤーキャラの体力を増減（値）
	CC_SYNTHESIZE(bool, _triggerPeriodically, TriggerPeriodically);		// プレイヤーキャラの体力を増減する間隔（フラグ）
	CC_SYNTHESIZE(double, _triggerPeriod, TriggerPeriod);				// プレイヤーキャラの体力を増減する間隔（値）
	CC_SYNTHESIZE(bool, _gravityEffectChanged, GravityEffectChanged);	// 重力効果を増減（フラグ）
	CC_SYNTHESIZE(int, _gravityEffectChange, GravityEffectChange);		// 重力効果を増減（％）
	CC_SYNTHESIZE(bool, _moveObjectAlongSlope, MoveObjectAlongSlope);	// オブジェクトを吸着（フラグ）

	// 物理演算用の設定
	CC_SYNTHESIZE(double, _physicsFriction, PhysicsFriction);	// 摩擦係数
	CC_SYNTHESIZE(double, _physicsRepulsion, PhysicsRepulsion);	// 反発係数
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API OthersLoopData : public OthersData
{
private:
	OthersLoopData();
	virtual ~OthersLoopData();
	virtual bool init(const rapidjson::Value& json);

public:
	CREATE_FUNC_PARAM(OthersLoopData, const rapidjson::Value&, json);

	// 表示の設定
	CC_SYNTHESIZE(double, _x, X);					// 座標X
	CC_SYNTHESIZE(double, _y, Y);					// 座標Y
	CC_SYNTHESIZE(int, _rotation, Rotation);	// 回転
	CC_SYNTHESIZE(double, _radius, Radius);		// 円のサイズ：半径
	CC_SYNTHESIZE(double, _startX, StartX);		// 始点X
	CC_SYNTHESIZE(double, _startY, StartY);		// 始点Y
	CC_SYNTHESIZE(double, _endX, EndX);			// 終点X
	CC_SYNTHESIZE(double, _endY, EndY);			// 終点Y
	CC_SYNTHESIZE(int, _dispPriority, DispPriority);//値が大きいほどレイヤー内で手前に表示されます。

	// 360度ループ用のパラメータ
	CC_SYNTHESIZE(bool, _needMoveFlag, NeedMoveFlag);			// ループに必要な移動量を設定（フラグ）
	CC_SYNTHESIZE(int, _needMove, NeedMove);					// ループに必要な移動量を設定（値）
	CC_SYNTHESIZE(bool, _moveToEnd, MoveToEnd);					// ループ終了まで自動移動
	CC_SYNTHESIZE(bool, _disabledFromRight, DisabledFromRight);	// 右から通行不可
	CC_SYNTHESIZE(bool, _disabledFromLeft, DisabledFromLeft);	// 左から通行不可
};

NS_DATA_END
NS_AGTK_END

#endif	//__OTHERS_DATA_H__