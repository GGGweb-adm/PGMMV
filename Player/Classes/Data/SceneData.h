#ifndef __AGKT_SCENE_DATA_H__
#define	__AGKT_SCENE_DATA_H__

#include "cocos2d.h"
#include "Lib/Macros.h"
#include "json/document.h"
#include "OthersData.h"

USING_NS_CC;

NS_AGTK_BEGIN
NS_DATA_BEGIN
//-------------------------------------------------------------------------------------------------------------------
//! 物理パーツ：基本データ
class AGTKPLAYER_API PhysicsBaseData : public cocos2d::Ref
{
public:
	/* 画像素材の配置設定 */
	enum EnumPlacement {
		kPlacementOriginalSize = 0,	// 元のサイズで表示
		kPlacementFit,				// オブジェクトに合わせて拡縮
		kPlacementTiling,			// タイル状に表示
		kPlacementScaling,			// 比率を指定して拡縮
	};

	static const int NO_CONNECT_SUB_ID = -1;//接続無しのサブID

protected:
	PhysicsBaseData();
	virtual ~PhysicsBaseData();

public:
	CREATE_FUNC_PARAM(PhysicsBaseData, const rapidjson::Value&, json);
#if defined(AGTK_DEBUG)
	void dump();
#endif

protected:
	virtual bool init(const rapidjson::Value& json);

	// ---------------------------------------------------------
	// ▼カラーリング・テクスチャの設定
	CC_SYNTHESIZE(bool, _coloring, Coloring);//カラーリング(true) or テクスチャ(false)

	CC_SYNTHESIZE(int, _colorA, ColorA);// 不透明度
	CC_SYNTHESIZE(int, _colorR, ColorR);// 色相：赤
	CC_SYNTHESIZE(int, _colorG, ColorG);// 色相：緑
	CC_SYNTHESIZE(int, _colorB, ColorB);// 色相：青

	CC_SYNTHESIZE(int, _imageId, ImageId);//画像ID
	CC_SYNTHESIZE(EnumPlacement, _placementType, PlacementType);//画像素材の配置設定
	CC_SYNTHESIZE(int, _placementX, PlacementX);//配置X座標オフセット(左上基準)
	CC_SYNTHESIZE(int, _placementY, PlacementY);//配置Y座標オフセット(左上基準)
	CC_SYNTHESIZE(double, _scaling, Scaling);//比率を指定し拡縮(%)

	// ---------------------------------------------------------
	// ▼非衝突グループ
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _nonCollisionGroup, NonCollisionGroup);//非衝突グループ(A:0, B:1, ...)でチェックされたのが入っている

	// ---------------------------------------------------------
	// ▼オプション
	CC_SYNTHESIZE(bool, _invisible, Invisible);//ゲーム実行時に非表示のON/OFF
};

//-------------------------------------------------------------------------------------------------------------------
//! 物理パーツ：円のデータ
class AGTKPLAYER_API PhysicsDiskData : public PhysicsBaseData
{
protected:
	PhysicsDiskData();
	virtual ~PhysicsDiskData();

public:
	CREATE_FUNC_PARAM(PhysicsDiskData, const rapidjson::Value&, json);
#if defined(AGTK_DEBUG)
	void dump();
#endif

protected:
	virtual bool init(const rapidjson::Value& json);

	// ---------------------------------------------------------
	// ▼表示の設定
	CC_SYNTHESIZE(double, _x, X);//初期位置: X座標
	CC_SYNTHESIZE(double, _y, Y);//初期位置: Y座標
	CC_SYNTHESIZE(double, _width, Width);//横幅
	CC_SYNTHESIZE(double, _height, Height);//縦幅
	CC_SYNTHESIZE(double, _rotation, Rotation);//角度

	// ---------------------------------------------------------
	// ▼物理演算用設定
	CC_SYNTHESIZE(double, _density, Density);//密度(kg/m^3)
	CC_SYNTHESIZE(double, _mass, Mass);//質量(kg)
	CC_SYNTHESIZE(double, _friction, Friction);//摩擦係数
	CC_SYNTHESIZE(double, _repulsion, Repulsion);//反発係数
};

//-------------------------------------------------------------------------------------------------------------------
//! 物理パーツ：四角形のデータ
class AGTKPLAYER_API PhysicsRectangleData : public PhysicsDiskData
{
protected:
	PhysicsRectangleData();
	virtual ~PhysicsRectangleData();

public:
	CREATE_FUNC_PARAM(PhysicsRectangleData, const rapidjson::Value&, json);
#if defined(AGTK_DEBUG)
	void dump();
#endif

protected:
	virtual bool init(const rapidjson::Value& json);

	/* 現状四角形は円と全く同じメンバを持ちます */
};
//-------------------------------------------------------------------------------------------------------------------
//! 物理パーツ：接着のデータ
class AGTKPLAYER_API PhysicsPinData : public PhysicsBaseData
{
protected:
	PhysicsPinData();
	virtual ~PhysicsPinData();

public:
	CREATE_FUNC_PARAM(PhysicsPinData, const rapidjson::Value&, json);
#if defined(AGTK_DEBUG)
	void dump();
#endif

protected:
	virtual bool init(const rapidjson::Value& json);

	// ---------------------------------------------------------
	// ▼表示の設定
	CC_SYNTHESIZE(double, _width, Width);//横幅
	CC_SYNTHESIZE(double, _height, Height);//縦幅

	// ---------------------------------------------------------
	// ▼連結の情報(上位パーツのどの部分を下位パーツのどの部分へ固定するかという情報)
	CC_SYNTHESIZE(int, _upperId, UpperId);//上位のシーンパーツID
	CC_SYNTHESIZE(int, _upperSubId, UpperSubId);//0以上であれば上位のシーンパーツ内のシーンパーツID
	CC_SYNTHESIZE(int, _upperX, UpperX);//上位のシーンパーツのX座標(ローカル)
	CC_SYNTHESIZE(int, _upperY, UpperY);//上位のシーンパーツのY座標(ローカル)
	CC_SYNTHESIZE(int, _lowerId, LowerId);//下位のシーンパーツID
	CC_SYNTHESIZE(int, _lowerSubId, LowerSubId);//0以上であれば下位のシーンパーツ内のシーンパーツID
	CC_SYNTHESIZE(int, _lowerX, LowerX);//下位のシーンパーツのX座標(ローカル)
	CC_SYNTHESIZE(int, _lowerY, LowerY);//下位のシーンパーツのY座標(ローカル)
};
//-------------------------------------------------------------------------------------------------------------------
//! 物理パーツ：ロープのデータ
class AGTKPLAYER_API PhysicsRopeData : public PhysicsBaseData
{
public:
	static const int POINT_X = 0;	// 中継点配列データのX座標インデックス
	static const int POINT_Y = 1;	// 中継点配列データのY座標インデックス
protected:
	PhysicsRopeData();
	virtual ~PhysicsRopeData();

public:
	CREATE_FUNC_PARAM(PhysicsRopeData, const rapidjson::Value&, json);
#if defined(AGTK_DEBUG)
	void dump();
#endif

protected:
	virtual bool init(const rapidjson::Value& json);

	// ---------------------------------------------------------
	// ▼物理演算用パラメータ
	CC_SYNTHESIZE(double, _length, Length);//長さ
	CC_SYNTHESIZE(double, _width, Width);//太さ
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _pointList, PointList);//中継点配列(Vec2の配列)

	// ---------------------------------------------------------
	// ▼連結の情報
	CC_SYNTHESIZE(int, _connectId1, ConnectId1);//接続点1のシーンパーツID
	CC_SYNTHESIZE(int, _connectSubId1, ConnectSubId1);//0以上であれば接続点1のシーンパーツ内のシーンパーツID
	CC_SYNTHESIZE(int, _connectX1, ConnectX1);//接続点1のシーンパーツのX座標(ローカル)
	CC_SYNTHESIZE(int, _connectY1, ConnectY1);//接続点1のシーンパーツのY座標(ローカル)
	CC_SYNTHESIZE(int, _connectId2, ConnectId2);//接続点2のシーンパーツID(接続点2がシーンの場合は垂れ下がる)
	CC_SYNTHESIZE(int, _connectSubId2, ConnectSubId2);//0以上であれば接続点2のシーンパーツ内のシーンパーツID
	CC_SYNTHESIZE(int, _connectX2, ConnectX2);//接続点2のシーンパーツのX座標(ローカル)
	CC_SYNTHESIZE(int, _connectY2, ConnectY2);//接続点2のシーンパーツのY座標(ローカル)
};
//-------------------------------------------------------------------------------------------------------------------
//! 物理パーツ：バネのデータ
class AGTKPLAYER_API PhysicsSpringData : public PhysicsBaseData
{
protected:
	PhysicsSpringData();
	virtual ~PhysicsSpringData();

public:
	CREATE_FUNC_PARAM(PhysicsSpringData, const rapidjson::Value&, json);
#if defined(AGTK_DEBUG)
	void dump();
#endif

protected:
	virtual bool init(const rapidjson::Value& json);

	// ---------------------------------------------------------
	// ▼物理演算用パラメータ
	CC_SYNTHESIZE(bool, _fixConnectAngle1, FixConnectAngle1);//連結点１とバネの角度を固定するフラグ
	CC_SYNTHESIZE(bool, _fixConnectAngle2, FixConnectAngle2);//連結点２とバネの角度を固定するフラグ
	CC_SYNTHESIZE(double, _springConstant, SpringConstant);//バネ定数(N/m)
	CC_SYNTHESIZE(double, _dampingCoefficient, DampingCoefficient);//減衰係数
	CC_SYNTHESIZE(double, _naturalLength, NaturalLength);//自然長
	CC_SYNTHESIZE(double, _width, Width);//太さ(表示用)


	// ---------------------------------------------------------
	// ▼連結の情報
	CC_SYNTHESIZE(int, _connectId1, ConnectId1);//接続点1のシーンパーツID
	CC_SYNTHESIZE(int, _connectSubId1, ConnectSubId1);//0以上であれば接続点1のシーンパーツ内のシーンパーツID
	CC_SYNTHESIZE(int, _connectX1, ConnectX1);//接続点1のシーンパーツのX座標(ローカル)
	CC_SYNTHESIZE(int, _connectY1, ConnectY1);//接続点1のシーンパーツのY座標(ローカル)
	CC_SYNTHESIZE(int, _connectId2, ConnectId2);//接続点2のシーンパーツID(接続点2がシーンの場合は垂れ下がる)
	CC_SYNTHESIZE(int, _connectSubId2, ConnectSubId2);//0以上であれば接続点2のシーンパーツ内のシーンパーツID
	CC_SYNTHESIZE(int, _connectX2, ConnectX2);//接続点2のシーンパーツのX座標(ローカル)
	CC_SYNTHESIZE(int, _connectY2, ConnectY2);//接続点2のシーンパーツのY座標(ローカル)
};
//-------------------------------------------------------------------------------------------------------------------
//! 物理パーツ：回転軸のデータ
class AGTKPLAYER_API PhysicsAxisData : public PhysicsBaseData
{
protected:
	PhysicsAxisData();
	virtual ~PhysicsAxisData();

public:
	CREATE_FUNC_PARAM(PhysicsAxisData, const rapidjson::Value&, json);
#if defined(AGTK_DEBUG)
	void dump();
#endif

protected:
	virtual bool init(const rapidjson::Value& json);
	// ---------------------------------------------------------
	// ▼表示の設定
	CC_SYNTHESIZE(double, _width, Width);//横幅
	CC_SYNTHESIZE(double, _height, Height);//縦幅

	// ---------------------------------------------------------
	// ▼物理演算用パラメータ
	CC_SYNTHESIZE(bool, _addRightRotation, AddRightRotation);//回転機能を追加(右回り)のフラグ
	CC_SYNTHESIZE(bool, _reverseDirection, ReverseDirection);//回転方向を切り替えるのフラグ
	CC_SYNTHESIZE(bool, _addBrakeFunction, AddBrakeFunction);//ブレーキ機能を追加のフラグ
	CC_SYNTHESIZE(bool, _enableBySwitch, EnableBySwitch);//動作をスイッチで切替えのフラグ
	CC_SYNTHESIZE(double, _rpm, Rpm);//回転の速さ(rpm)
	CC_SYNTHESIZE(double, _torque, Torque);//回転のトルク(N・m)
	CC_SYNTHESIZE(double, _dampingRatio, DampingRatio);//回転の減衰率(ブレーキ時)

	// ---------------------------------------------------------
	// ▼動作切り替え用スイッチ
	CC_SYNTHESIZE(int, _rightRotationSwitchId, RightRotationSwitchId);//右回転：スイッチの設定値
	CC_SYNTHESIZE(int, _rightRotationSwitchObjectId, RightRotationSwitchObjectId);//右回転：スイッチオブジェクトID
	CC_SYNTHESIZE(int, _rightRotationSwitchQualifierId, RightRotationSwitchQualifierId);//右回転：スイッチ制限ID

	CC_SYNTHESIZE(int, _leftRotationSwitchId, LeftRotationSwitchId);//左回転：スイッチの設定値
	CC_SYNTHESIZE(int, _leftRotationSwitchObjectId, LeftRotationSwitchObjectId);//左回転：スイッチオブジェクトID
	CC_SYNTHESIZE(int, _leftRotationSwitchQualifierId, LeftRotationSwitchQualifierId);//左回転：スイッチ制限ID

	CC_SYNTHESIZE(int, _brakeSwitchId, BrakeSwitchId);//ブレーキ：スイッチの設定値
	CC_SYNTHESIZE(int, _brakeSwitchObjectId, BrakeSwitchObjectId);//ブレーキ：スイッチオブジェクトID
	CC_SYNTHESIZE(int, _brakeSwitchQualifierId, BrakeSwitchQualifierId);//ブレーキ：スイッチ制限ID

	// ---------------------------------------------------------
	// ▼連結の情報(上位パーツのどの部分を下位パーツのどの部分へ固定するかという情報)
	CC_SYNTHESIZE(int, _upperId, UpperId);//上位のシーンパーツID
	CC_SYNTHESIZE(int, _upperSubId, UpperSubId);//0以上であれば上位のシーンパーツ内のシーンパーツID
	CC_SYNTHESIZE(int, _upperX, UpperX);//上位のシーンパーツのX座標(ローカル)
	CC_SYNTHESIZE(int, _upperY, UpperY);//上位のシーンパーツのY座標(ローカル)
	CC_SYNTHESIZE(int, _lowerId, LowerId);//下位のシーンパーツID
	CC_SYNTHESIZE(int, _lowerSubId, LowerSubId);//0以上であれば下位のシーンパーツ内のシーンパーツID
	CC_SYNTHESIZE(int, _lowerX, LowerX);//下位のシーンパーツのX座標(ローカル)
	CC_SYNTHESIZE(int, _lowerY, LowerY);//下位のシーンパーツのY座標(ローカル)

	// ---------------------------------------------------------
	// ▼オプション
	CC_SYNTHESIZE(bool, _useVerificationKey, UseVerificationKey);//動作検証用に動作切替えキーを追加のフラグ
	CC_SYNTHESIZE(int, _rightRotationKeyId, RightRotationKeyId);//右回転に設定された入力キーID
	CC_SYNTHESIZE(int, _leftRotationKeyId, LeftRotationKeyId);//左回転に設定された入力キーID
	CC_SYNTHESIZE(int, _brakeKeyId, BrakeKeyId);//ブレーキ機能に設定された入力キーID
};
//-------------------------------------------------------------------------------------------------------------------
//! 物理パーツ：力学系基本データ
class AGTKPLAYER_API PhysicsForceBaseData : public cocos2d::Ref
{
public:
	/* 範囲タイプ */
	enum EnumRangeType {
		kRangeTypeFan = 0,	// 円形
		kRangeTypeBand,		// 矩形
	};

protected:
	PhysicsForceBaseData();
	virtual ~PhysicsForceBaseData();

public:
	CREATE_FUNC_PARAM(PhysicsForceBaseData, const rapidjson::Value&, json);
#if defined(AGTK_DEBUG)
	void dump();
#endif

protected:
	virtual bool init(const rapidjson::Value& json);

	// ---------------------------------------------------------
	// ▼物理演算用パラメータ
	CC_SYNTHESIZE(bool, _rotateAccordingToConnectedObject, RotateAccordingToConnectedObject);//連結したオブジェクトに合わせて回転するのフラグ
	CC_SYNTHESIZE(bool, _limitDirectionRange, LimitDirectionRange);//方向と範囲を設定のフラグ
	CC_SYNTHESIZE(bool, _enableBySwitch, EnableBySwitch);//動作をスイッチで切替えのフラグ

	CC_SYNTHESIZE(double, _strength, Strength);//力の強さ
	CC_SYNTHESIZE(double, _direction, Direction);//力の作用方向(上が0度)

	CC_SYNTHESIZE(EnumRangeType, _rangeType, rangeType);//力の作用範囲：矩形：横幅
	CC_SYNTHESIZE(double, _fanAngle, FanAngle);//力の作用範囲：円：角度(上が0度)
	CC_SYNTHESIZE(double, _bandWidth, BandWidth);//力の作用範囲：矩形：横幅

	// ---------------------------------------------------------
	// ▼連結の情報
	CC_SYNTHESIZE(int, _connectId, ConnectId);//接続点のシーンパーツID
	CC_SYNTHESIZE(int, _connectSubId, ConnectSubId);//0以上であれば接続点のシーンパーツ内のシーンパーツID
	CC_SYNTHESIZE(int, _connectX, ConnectX);//接続点のシーンパーツのX座標(ローカル)
	CC_SYNTHESIZE(int, _connectY, ConnectY);//接続点のシーンパーツのY座標(ローカル)

	// ---------------------------------------------------------
	// ▼オプション
	CC_SYNTHESIZE(bool, _useVerificationKey, UseVerificationKey);//動作検証用に動作切替えキーを追加のフラグ
};

//-------------------------------------------------------------------------------------------------------------------
//! 物理パーツ：爆発のデータ
class AGTKPLAYER_API PhysicsExplosionData : public PhysicsForceBaseData
{
protected:
	PhysicsExplosionData();
	virtual ~PhysicsExplosionData();

public:
	CREATE_FUNC_PARAM(PhysicsExplosionData, const rapidjson::Value&, json);
#if defined(AGTK_DEBUG)
	void dump();
#endif

protected:
	virtual bool init(const rapidjson::Value& json);

	// ---------------------------------------------------------
	// ▼物理演算用パラメータ
	CC_SYNTHESIZE(int, _duration300, Duration300);//爆発の力が最大になるまでの時間
	CC_SYNTHESIZE(double, _effectiveDistance, EffectiveDistance);//爆発の範囲
	CC_SYNTHESIZE(bool, _effectiveInfinite, EffectiveInfinite);//力の作用する距離は無限大のフラグ

	// ---------------------------------------------------------
	// ▼動作切替え用スイッチ
	CC_SYNTHESIZE(int, _switchId, SwitchId);//スイッチID
	CC_SYNTHESIZE(int, _switchObjectId, SwitchObjectId);//スイッチオブジェクトID
	CC_SYNTHESIZE(int, _switchQualifierId, SwitchQualifierId);//スイッチ制限ID

	// ---------------------------------------------------------
	// ▼オプション
	CC_SYNTHESIZE(int, _keyId, KeyId);//動作検証用の入力キーID
};
//-------------------------------------------------------------------------------------------------------------------
//! 物理パーツ：引力・斥力のデータ
class AGTKPLAYER_API PhysicsForceData : public PhysicsForceBaseData
{
protected:
	PhysicsForceData();
	virtual ~PhysicsForceData();

public:
	CREATE_FUNC_PARAM(PhysicsForceData, const rapidjson::Value&, json);
#if defined(AGTK_DEBUG)
	void dump();
#endif

protected:
	virtual bool init(const rapidjson::Value& json);

	// ---------------------------------------------------------
	// ▼物理演算用パラメータ
	CC_SYNTHESIZE(bool, _attractiveForce, AttractiveForce);//引力を発生(true) or 斥力を発生(false)
	CC_SYNTHESIZE(bool, _constantForce, ConstantForce);//力の作用は範囲内で一定のフラグ
	CC_SYNTHESIZE(double, _distance, Distance);//力の作用する距離
	CC_SYNTHESIZE(bool, _infinite, Infinite);//力の作用する距離は無限大のフラグ

	// ---------------------------------------------------------
	// ▼動作切替え用スイッチ
	CC_SYNTHESIZE(int, _attractiveForceSwitchId, AttractiveForceSwitchId);//引力：スイッチID
	CC_SYNTHESIZE(int, _attractiveForceSwitchObjectId, AttractiveForceSwitchObjectId);//引力：スイッチオブジェクトID
	CC_SYNTHESIZE(int, _attractiveForceSwitchQualifierId, AttractiveForceSwitchQualifierId);//引力：スイッチ制限ID

	CC_SYNTHESIZE(int, _repulsiveForceSwitchId, RepulsiveForceSwitchId);//斥力：スイッチID
	CC_SYNTHESIZE(int, _repulsiveForceSwitchObjectId, RepulsiveForceSwitchObjectId);//斥力：スイッチオブジェクトID
	CC_SYNTHESIZE(int, _repulsiveForceSwitchQualifierId, RepulsiveForceSwitchQualifierId);//斥力：スイッチ制限ID
															  
	// ---------------------------------------------------------
	// ▼オプション
	CC_SYNTHESIZE(int, _attractiveForceKeyId, AttractiveForceKeyId);//引力の動作検証用の入力キーID
	CC_SYNTHESIZE(int, _repulsiveForceKeyId, RepulsiveForceKeyId);//斥力の動作検証用の入力キーID
};

//-------------------------------------------------------------------------------------------------------------------
//! 物理パーツデータ
class AGTKPLAYER_API PhysicsPartData : public cocos2d::Ref
{
public:
	/* 物理パーツタイプ */
	enum EnumPhysicsType {
		kDisk,			// 円
		kRectangle,		// 四角形
		kPin,			// 接着
		kRope,			// ロープ
		kSpring,		// ばね
		kAxis,			// 回転軸
		kExplosion,		// 爆発
		kForce,			// 引力・斥力
	};

private:
	PhysicsPartData();
	virtual ~PhysicsPartData();

public:
	CREATE_FUNC_PARAM(PhysicsPartData, const rapidjson::Value&, json);
#if defined(AGTK_DEBUG)
	void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);

private:
	// ----------------------------------------------------------
	// !基本データ
	CC_SYNTHESIZE(int, _id, Id);//シーンパーツID(オブジェクト内の物理パーツの場合は 99999999)
	CC_SYNTHESIZE(bool, _partType, PartType);//パーツタイプ
	CC_SYNTHESIZE(int, _layerIndex, LayerIndex);//配置レイヤーインデックス
	CC_SYNTHESIZE_RETAIN(cocos2d::__String *, _name, Name);//名前
	CC_SYNTHESIZE(int, _priority, Priority);//表示優先度(数字が大きいほど手前、同一の場合は追加順)
	CC_SYNTHESIZE(int, _dispPriority, DispPriority);//表示優先度(値が大きいほどレイヤー内で手前に表示されます。)
	CC_SYNTHESIZE(EnumPhysicsType, _type, Type);//物理演算パーツタイプ

	CC_SYNTHESIZE_RETAIN(cocos2d::Ref *, _physicsData, PhysicsData);//物理パーツデータ
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API LayerData : public cocos2d::Ref
{
private:
	LayerData();
	virtual ~LayerData();
public:
	CREATE_FUNC_PARAM2(LayerData, const rapidjson::Value&, json, int, layerId);
	const char *getName();
#if defined(AGTK_DEBUG)
	void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json, int layerId);
private:
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _tileList, TileList);//->Tile
	CC_SYNTHESIZE_RETAIN(cocos2d::__String *, _name, Name);
	//Additional Data
	CC_SYNTHESIZE(int, _layerId, LayerId);
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API ScenePartData : public cocos2d::Ref
{
protected:
	ScenePartData();
	virtual ~ScenePartData();
public:
	enum EnumPartType {
		kPartObject,
		kPartPhysics,
		kPartOthers,
		kPartPortal,
		kPartMax
	};
	const char *getName();
#if defined(AGTK_DEBUG)
	virtual void dump();
#endif
protected:
	virtual bool init(const rapidjson::Value& json);
private:
	CC_SYNTHESIZE(unsigned int, _id, Id);
	CC_SYNTHESIZE_RETAIN(cocos2d::__String *, _name, Name);//名
	CC_SYNTHESIZE(EnumPartType, _partType, PartType);
	CC_SYNTHESIZE(int, _layerIndex, LayerIndex);//配置レイヤー
	CC_SYNTHESIZE(bool, _visible, Visible);
	CC_SYNTHESIZE(bool, _locked, Locked);
	CC_SYNTHESIZE(int, _priority, Priority);
	CC_SYNTHESIZE(int, _dispPriority, DispPriority);
	CC_SYNTHESIZE(bool, _folder, Folder);
	CC_SYNTHESIZE_RETAIN(cocos2d::__Dictionary *, _objectChildren, ObjectChildren);
	CC_SYNTHESIZE_RETAIN(cocos2d::__Dictionary *, _physicsChildren, PhysicsChildren);
	CC_SYNTHESIZE_RETAIN(cocos2d::__Dictionary *, _othersChildren, OthersChildren);
	CC_SYNTHESIZE_RETAIN(cocos2d::__Dictionary *, _portalChildren, PortalChildren);

};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API ScenePartObjectData : public ScenePartData
{
public:
	static const int START_POINT_OBJECT_ID = 99999999;//スタートポイントID
private:
	ScenePartObjectData();
	virtual ~ScenePartObjectData();

public:
	CREATE_FUNC_PARAM(ScenePartObjectData, const rapidjson::Value&, json);
	bool isStartPointObjectData();
#if defined(AGTK_DEBUG)
	virtual void dump();
#endif
	bool isStartPointObject() { return (_objectId == START_POINT_OBJECT_ID); }
private:
	virtual bool init(const rapidjson::Value& json);

	CC_SYNTHESIZE(int, _objectId, ObjectId);//オブジェクトID
	CC_SYNTHESIZE(int, _initialActionId, InitialActionId);//出現時のアクション
	CC_SYNTHESIZE(bool, _initialDisplayDirectionFlag, InitialDisplayDirectionFlag);//初期表示方向を設定（フラグ）
	CC_SYNTHESIZE(double, _initialDisplayDirection, InitialDisplayDirection);//初期表示方向を設定
	CC_SYNTHESIZE(double, _x, X);//初期位置X
	CC_SYNTHESIZE(double, _y, Y);//初期位置Y
	CC_SYNTHESIZE(double, _scalingX, ScalingX);//スケールX
	CC_SYNTHESIZE(double, _scalingY, ScalingY);//スケールY
	CC_SYNTHESIZE(double, _rotation, Rotation);//回転
	CC_SYNTHESIZE_RETAIN(cocos2d::__Dictionary *, _actionCommandListObject, ActionCommandListObject);//その他の実行アクション
	CC_SYNTHESIZE_RETAIN(cocos2d::__Dictionary *, _commonActionCommandListObject, CommonActionCommandListObject);//コモン実行アクション
	CC_SYNTHESIZE_RETAIN(cocos2d::__Dictionary *, _disappearActionCommandList, DisappearActionCommandList);//消滅実行アクション
	CC_SYNTHESIZE_RETAIN(cocos2d::__Dictionary *, _physicsPartList, PhysicsPartList);//シーンに配置後の物理パーツ
	CC_SYNTHESIZE(int, _courseScenePartId, CourseScenePartId);//コース
	CC_SYNTHESIZE(int, _courseStartPointId, CourseStartPointId);//コース開始ポイント
	//スタートポイント用
	CC_SYNTHESIZE(int, _startPointGroupIndex, StartPointGroupIndex);
	CC_SYNTHESIZE(int, _startPointPlayerBit, StartPointPlayerBit);
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API ScenePartOthersData : public ScenePartData
{
public:
	enum OthersType
	{
		kOthersCamera,			// カメラ
		kOthersLineCourse,		// コース（直線）
		kOthersCurveCourse,		// コース（曲線）
		kOthersCircleCourse,	// コース（円）
		kOthersSlope,			// 坂を設置
		kOthersLoop,			// 360度ループを設置
		kOthersMax
	};

private:
	ScenePartOthersData();
	virtual ~ScenePartOthersData();

public:
	CREATE_FUNC_PARAM(ScenePartOthersData, const rapidjson::Value&, json);
#if defined(AGTK_DEBUG)
	virtual void dump();
#endif

private:
	virtual bool init(const rapidjson::Value& json);

	CC_SYNTHESIZE(OthersType, _othersType, OthersType);
	CC_SYNTHESIZE_RETAIN(OthersData *, _part, Part);
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API ScenePartPortalData : public ScenePartData
{

};

//-------------------------------------------------------------------------------------------------------------------
class FilterEffect;
class AGTKPLAYER_API SceneFilterEffectData : public cocos2d::Ref
{
public:
	enum {
		kLayerIndexAllSceneLayers = -1,		// 全てのシーンレイヤー
		kLayerIndexBackground = -2,			// 背景
		kLayerIndexTopMost = -3,			// 最前面
		kLayerIndexTopMostWithMenu = -4,	// 最前面(メニュー含む)
	};
private:
	SceneFilterEffectData();
	virtual ~SceneFilterEffectData();
public:
	CREATE_FUNC_PARAM(SceneFilterEffectData, const rapidjson::Value&, json);
	FilterEffect *getFilterEffect() { return _filterEffect; };
	void setFilterEffect(FilterEffect *filterEffect);
#if defined(AGTK_DEBUG)
	void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
private:
	CC_SYNTHESIZE(int, _id, Id);
	CC_SYNTHESIZE(int, _layerIndex, LayerIndex);
	CC_SYNTHESIZE(bool, _disabled, Disabled);
	FilterEffect *_filterEffect;
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API SceneData : public cocos2d::Ref
{
public:
	enum EnumBgImagePlacement {
		kCenter = 0, kMagnify, kTiling, kMagnifyKeepRatio,
	};
	enum EnumPlayBgmType {
		kPlayBgmNone,
		kPlayBgmPlay,
		kPlayBgmContinue,
		kPlayBgmMax
	};
	enum {
		kMenuSceneId			= 99999999,	//メニュー画面ID
		kTopMostLayerId			= 999998,	//最前面レイヤーID
		kTopMostWithMenuLayerId = 999999,	//最前面レイヤーID(メニュー含む)
		kHudTopMostLayerId		= 99999,	//暗黙の最前面レイヤーID (cocos2d::CameraFlag::USER2)
		kHudMenuLayerId			= 9999,		//暗黙の最前面メニューレイヤーID
		kBackgroundLayerId		= 0,		//背景レイヤーID
	};
private:
	SceneData();
	virtual ~SceneData();
public:
	CREATE_FUNC_PARAM2(SceneData, const rapidjson::Value&, json, cocos2d::Size, tileSize);
	const char *getName();
	const char *getMemo();
	LayerData *getLayer(int id);
	cocos2d::__Array *getScenePartObjectList(int layerId);
	cocos2d::__Array *getScenePartOthersList(int layerId);
	int getScenePartObjectDataMaxId();
	int getObjectSingleId(int objectId);
	double getLayerMoveSpeedX(int layerId);
	double getLayerMoveSpeedY(int layerId);
	ScenePartObjectData *getScenePartObjectData(int id);
	bool isScenePartObject(ScenePartObjectData *data);
	bool isMenuScene();
#if defined(AGTK_DEBUG)
	void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json, cocos2d::Size tileSize);
private:
	CC_SYNTHESIZE(int, _id, Id);
	CC_SYNTHESIZE_RETAIN(cocos2d::__String *, _name, Name);
	CC_SYNTHESIZE(bool, _folder, Folder);
	CC_SYNTHESIZE_RETAIN(cocos2d::__Dictionary *, _children, Children);
	CC_SYNTHESIZE(int, _horzScreenCount, HorzScreenCount);
	CC_SYNTHESIZE(int, _vertScreenCount, VertScreenCount);
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _tileset, Tileset);//->cocos2d::Integer
	CC_SYNTHESIZE_RETAIN(cocos2d::__String *, _memo, Memo);
	CC_SYNTHESIZE(EnumPlayBgmType, _playBgmType, PlayBgmType);
	CC_SYNTHESIZE(int, _bgmId, BgmId);
	CC_SYNTHESIZE(bool, _loopBgmFlag, LoopBgmFlag);
	CC_SYNTHESIZE(cocos2d::Color3B, _bgColor, BgColor);//bgColorR,bgColorG,bgColorB
	CC_SYNTHESIZE(bool, _setBgImageFlag, SetBgImageFlag);//背景画像を設定フラグ
	CC_SYNTHESIZE(int, _bgImageId, BgImageId);//背景画像ID
	CC_SYNTHESIZE(EnumBgImagePlacement, _bgImagePlacement, BgImagePlacement);
	CC_SYNTHESIZE(double, _bgImageMoveSpeed, BgImageMoveSpeed);
	CC_SYNTHESIZE(double, _bgImageMoveDirection, BgImageMoveDirection);
	CC_SYNTHESIZE(double, _bgMoveSpeedX, BgMoveSpeedX);
	CC_SYNTHESIZE(double, _bgMoveSpeedY, BgMoveSpeedY);
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _layerMoveSpeedX, LayerMoveSpeedX);//->cocos2d::Integer
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _layerMoveSpeedY, LayerMoveSpeedY);//->cocos2d::Integer
	CC_SYNTHESIZE(double, _gravityAccel, GravityAccel);
	CC_SYNTHESIZE(double, _airResistance, AirResistance);//空気抵抗
	CC_SYNTHESIZE(double, _gravityDirection, GravityDirection);//重力方向
	CC_SYNTHESIZE(bool, _screenAutoScroll, ScreenAutoScroll);
	CC_SYNTHESIZE(double, _screenAutoScrollSpeed, ScreenAutoScrollSpeed);
	CC_SYNTHESIZE(double, _screenAutoScrollDirection, ScreenAutoScrollDirection);
	CC_SYNTHESIZE(int, _screenAutoScrollSwitchObjectId, ScreenAutoScrollSwitchObjectId);
	CC_SYNTHESIZE(int, _screenAutoScrollSwitchQualifierId, ScreenAutoScrollSwitchQualifierId);
	CC_SYNTHESIZE(int, _screenAutoScrollSwitchId, ScreenAutoScrollSwitchId);
	CC_SYNTHESIZE(bool, _verticalLoop, VerticalLoop);//シーンの上下を繋げる
	CC_SYNTHESIZE(bool, _horizontalLoop, HorizontalLoop);//シーンの左右を繋げる
	CC_SYNTHESIZE(float, _limitAreaX, LimitAreaX);
	CC_SYNTHESIZE(float, _limitAreaY, LimitAreaY);
	CC_SYNTHESIZE(float, _limitAreaWidth, LimitAreaWidth);
	CC_SYNTHESIZE(float, _limitAreaHeight, LimitAreaHeight);
	CC_SYNTHESIZE(bool, _disableLimitArea, DisableLimitArea);
	CC_SYNTHESIZE(cocos2d::Rect, _limitCameraRect, LimitCameraRect);//limitCameraX,limitCameraY,limitCameraWidth,limitCameraHeight
	CC_SYNTHESIZE(bool, _disableLimitCamera, DisableLimitCamera);
	CC_SYNTHESIZE(int, _initialMenuLayerId, InitialMenuLayerId);//初期表示メニュー画面（メニュー画面レイヤーID）
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _preloadMenuLayerIdList, PreloadMenuLayerIdList);//事前に読み込むメニュー画面を登録（メニュー画面レイヤーID配列）
	CC_SYNTHESIZE_RETAIN(cocos2d::__Dictionary *, _layerList, LayerList);//->LayerData(layer1,layer2,...,layerN)
	CC_SYNTHESIZE_RETAIN(cocos2d::__Dictionary *, _scenePartObjectList, ScenePartObjectList);//->ScenePartObjectData
	CC_SYNTHESIZE_RETAIN(cocos2d::__Dictionary *, _scenePartPhysicsList, ScenePartPhysicsList);//->ScenePartPhysicsData
	CC_SYNTHESIZE_RETAIN(cocos2d::__Dictionary *, _scenePartOthersList, ScenePartOthersList);//->ScenePartOthersData
	CC_SYNTHESIZE_RETAIN(cocos2d::__Dictionary *, _scenePartPortalList, ScenePartPortalList);//->ScenePartPortalData
	CC_SYNTHESIZE_RETAIN(cocos2d::__Dictionary *, _sceneFilterEffectList, SceneFilterEffectList);//->FilterEffect
	CC_SYNTHESIZE_RETAIN(cocos2d::__Dictionary *, _objectSingleIdHash, ObjectSingleIdHash);//->cocos2d::Integer
};

NS_DATA_END
NS_AGTK_END

#endif	//__AGKT_SCENE_DATA_H__
