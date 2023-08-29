#ifndef __AGTK_OBJECT_ACTION_LINK_CONDITION_DATA_H__
#define	__AGTK_OBJECT_ACTION_LINK_CONDITION_DATA_H__

#include "cocos2d.h"
#include "json/document.h"
#include "Lib/Macros.h"

USING_NS_CC;

NS_AGTK_BEGIN
NS_DATA_BEGIN

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API ObjectActionLinkConditionData : public cocos2d::Ref
{
public:
	enum EnumWallBit {
		kWallBitUp = 0x01,//0x0001,
		kWallBitLeft = 0x02,//0x0010,
		kWallBitRight = 0x04,//0x0100,
		kWallBitDown = 0x08,//0x1000,
	};
	enum EnumSlopeBit {
		kSlopeBitUp = 0x01,//0x0001,
		kSlopeBitDown = 0x08,//0x1000,
	};
	enum {
		kSelfObject = -2,
		kOtherThanSelfObject = -3,
		kChildObject = -4,
		kLockedObject = -5,
		kTouchedObject = -6,
		kParentObject = -7,
	};
	enum EnumQualifierType {
		kQualifierSingle = -1,
		kQualifierWhole = -2,
	};
	enum EnumTouchedObjectType {
		kObjectByGroup,
		kObjectById,
		kObjectNone,
		kObjectMax
	};
	enum EnumObjectTypeByType {
		kObjectTypeAll = -1,
		kObjectTypePlayer,
		kObjectTypeEnemy,
		kObjectTypeMax
	};
	enum EnumObjectGroup {
		kObjectGroupAll = -1,
		kObjectGroupPlayer,
		kObjectGroupEnemy,
		kObjectGroupAllOffset = 1,
	};
	enum EnumTileGroup {
		kTileGroupDefault = 0,
	};
	enum EnumAttackAttribute {
		kFire = 1,
		kWater,
		kEarth,
		kAir,
		kThunder,
		kIce,
		kLight,
		kDarkness
	};
	enum EnumCompareOperatorType {
		kOperatorLess,
		kOperatorLessEqual,
		kOperatorEqual,
		kOperatorGreaterEqual,
		kOperatorGreater,
		kOperatorNotEqual,
		kOperatorMax
	};
	enum EnumObjActionLinkConditionType {
		kConditionWallTouched,//壁判定に接触(0)
		kConditionNoWall,//壁判定が無い(1)
		kConditionWallAhead,//進んだ先で壁判定に接触(2)
		kConditionNoWallAhead,//進んだ先で判定が無い(3)
		kConditionObjectWallTouched,//他オブジェクトの壁判定に接触(4)
		kConditionAttackAreaTouched,//攻撃判定に接触(5)
		kConditionAttackAreaNear,//攻撃判定が近くにある(6)
		kConditionObjectNear,//他オブジェクトが近くにある(7)
		kConditionObjectFacingEachOther,//他のオブジェクトと向かいあっている(8)
		kConditionObjectFacing,//他のオブジェクトの方を向かっている(9)
		kConditionObjectFound,//他のオブジェクトを発見した(10)
		kConditionObjectFacingDirection,//他のオブジェクトが指定方向を向いている(11)
		kConditionHpZero,//体力が0(12)
		kConditionCameraOutOfRange,//カメラの範囲外にでた(13)
		kConditionLocked,//ロックした/された(14)
		kConditionProbability,//確率を使用(15)
		kConditionWaitTime,//一定時間が経過(16)
		kConditionSwitchVariableChanged,//スイッチ・変数が変化(17)
		kConditionAnimationFinished,//モーションの表示が全て終わった(18)
		kConditionJumpTop,//ジャンプが頂点になった(19)
		kConditionObjectActionChanged,//オブジェクトのアクションが変化(20)
		kConditionSlopeTouched,//坂に接触(21)
		kConditionBuriedInWall,//壁判定に埋まった(22)
		kConditionNoObjectWallTouched,//他オブジェクトの壁判定に接触していない(23)
		kConditionScript,//他オブジェクトの壁判定に接触していない(24)
		kConditionObjectHit,//他オブジェクトの当たり判定に接触(25)
		kConditionMax,
		kConditionCustomHead = 1000,//カスタム(1000～)
	};
	enum {
		kPluginConditionCustomMax = 100,
	};
	enum {
		kAllDirectionBit = 0x3DE,	//->1111011110
	};
protected:
	ObjectActionLinkConditionData();
	virtual ~ObjectActionLinkConditionData(){}
public:
	static ObjectActionLinkConditionData *create(const rapidjson::Value& json);
#if defined(AGTK_DEBUG)
	virtual void dump();
#endif
protected:
	virtual bool init(const rapidjson::Value& json);
private:
	CC_SYNTHESIZE(unsigned int, _id, Id);
	CC_SYNTHESIZE(EnumObjActionLinkConditionType, _type, Type);
	CC_SYNTHESIZE(bool, _ignored, Ignored);
	CC_SYNTHESIZE(bool, _condAnd, CondAnd);
	CC_SYNTHESIZE(bool, _notFlag, NotFlag);
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API ObjectActionLinkConditionWallTouchedData : public ObjectActionLinkConditionData
{
public:
	ObjectActionLinkConditionWallTouchedData() : ObjectActionLinkConditionData() {};
	virtual ~ObjectActionLinkConditionWallTouchedData() {};
public:
	CREATE_FUNC_PARAM(ObjectActionLinkConditionWallTouchedData, const rapidjson::Value&, json);
	CREATE_FUNC_PARAM2(ObjectActionLinkConditionWallTouchedData, void *, jsCx, void *, jsObj);
#if defined(AGTK_DEBUG)
	virtual void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
	bool init(void *jsCx, void *jsObj);
private:
	CC_SYNTHESIZE(int, _wallBit, WallBit);
	CC_SYNTHESIZE(bool, _useTileGroup, UseTileGroup);
	CC_SYNTHESIZE(int, _tileGroup, TileGroup);
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API ObjectActionLinkConditionNoWallData : public ObjectActionLinkConditionData
{
public:
	ObjectActionLinkConditionNoWallData() : ObjectActionLinkConditionData() {};
	virtual ~ObjectActionLinkConditionNoWallData() {};
public:
	CREATE_FUNC_PARAM(ObjectActionLinkConditionNoWallData, const rapidjson::Value&, json);
#if defined(AGTK_DEBUG)
	virtual void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
private:
	CC_SYNTHESIZE(int, _wallBit, WallBit);
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API ObjectActionLinkConditionWallAheadData : public ObjectActionLinkConditionData
{
public:
	ObjectActionLinkConditionWallAheadData() : ObjectActionLinkConditionData(){};
	virtual ~ObjectActionLinkConditionWallAheadData() {};
public:
	CREATE_FUNC_PARAM(ObjectActionLinkConditionWallAheadData, const rapidjson::Value&, json);
	CREATE_FUNC_PARAM2(ObjectActionLinkConditionWallAheadData, void *, jsCx, void *, jsObj);
#if defined(AGTK_DEBUG)
	virtual void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
	bool init(void *jsCx, void *jsObj);
private:
	CC_SYNTHESIZE(int, _wallBit, WallBit);
	CC_SYNTHESIZE(bool, _useTileGroup, UseTileGroup);
	CC_SYNTHESIZE(int, _tileGroup, TileGroup);
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API ObjectActionLinkConditionNoWallAheadData : public ObjectActionLinkConditionData
{
public:
	ObjectActionLinkConditionNoWallAheadData() : ObjectActionLinkConditionData() {};
	virtual ~ObjectActionLinkConditionNoWallAheadData() {};
public:
	CREATE_FUNC_PARAM(ObjectActionLinkConditionNoWallAheadData, const rapidjson::Value&, json);
#if defined(AGTK_DEBUG)
	virtual void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
private:
	CC_SYNTHESIZE(int, _wallBit, WallBit);
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API ObjectActionLinkConditionObjectWallTouchedData : public ObjectActionLinkConditionData
{
public:
	ObjectActionLinkConditionObjectWallTouchedData() : ObjectActionLinkConditionData() {};
	virtual ~ObjectActionLinkConditionObjectWallTouchedData() {};
public:
	CREATE_FUNC_PARAM(ObjectActionLinkConditionObjectWallTouchedData, const rapidjson::Value&, json);
	CREATE_FUNC_PARAM2(ObjectActionLinkConditionObjectWallTouchedData, void *, jsCx, void *, jsObj);
#if defined(AGTK_DEBUG)
	virtual void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
	bool init(void *jsCx, void *jsObj);
private:
	CC_SYNTHESIZE(int, _wallBit, WallBit);
	CC_SYNTHESIZE(int, _objectType, ObjectType);
	CC_SYNTHESIZE(int, _objectGroup, ObjectGroup);
	CC_SYNTHESIZE(int, _objectId, ObjectId);
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API ObjectActionLinkConditionNoObjectWallTouchedData : public ObjectActionLinkConditionData
{
public:
	ObjectActionLinkConditionNoObjectWallTouchedData() : ObjectActionLinkConditionData() {};
	virtual ~ObjectActionLinkConditionNoObjectWallTouchedData() {};
public:
	CREATE_FUNC_PARAM(ObjectActionLinkConditionNoObjectWallTouchedData, const rapidjson::Value&, json);
#if defined(AGTK_DEBUG)
	virtual void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
private:
	CC_SYNTHESIZE(int, _wallBit, WallBit);
	CC_SYNTHESIZE(int, _objectType, ObjectType);
	CC_SYNTHESIZE(int, _objectTypeByType, ObjectTypeByType);
	CC_SYNTHESIZE(int, _objectId, ObjectId);
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API ObjectActionLinkConditionAttackAreaTouchedData : public ObjectActionLinkConditionData
{
public:
	enum EnumAttackAttribute {
		kAttributeNone,
		kAttributePreset,
		kAttributeValue,
		kAttributeMax
	};
public:
	ObjectActionLinkConditionAttackAreaTouchedData() : ObjectActionLinkConditionData() {};
	virtual ~ObjectActionLinkConditionAttackAreaTouchedData() {};
public:
	CREATE_FUNC_PARAM(ObjectActionLinkConditionAttackAreaTouchedData, const rapidjson::Value&, json);
	CREATE_FUNC_PARAM2(ObjectActionLinkConditionAttackAreaTouchedData, void *, jsCx, void *, jsObj);
#if defined(AGTK_DEBUG)
	virtual void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
	bool init(void *jsCx, void *jsObj);
private:
	CC_SYNTHESIZE(int, _wallBit, WallBit);//攻撃判定と接触している方向を指定
	CC_SYNTHESIZE(int, _objectType, ObjectType);//攻撃しているオブジェクトを指定（0:オブジェクトの種類で指定,1:オブジェクトで指定,2:指定しない）
	CC_SYNTHESIZE(int, _objectGroup, ObjectGroup);//オブジェクトのグループの指定（-1:全てのオブジェクト
	CC_SYNTHESIZE(int, _objectId, ObjectId);//オブジェクト指定（-2:自身のオブジェクト,-3:自身以外のオブジェクト,それ以外はオブジェクトID）
	CC_SYNTHESIZE(EnumAttackAttribute, _attributeType, AttributeType);//攻撃属性を指定
	CC_SYNTHESIZE(int, _attributePresetId, AttributePresetId);//プリセット属性
	CC_SYNTHESIZE(int, _attributeValue, AttributeValue);//数値で指定
	CC_SYNTHESIZE(bool, _attributeEqual, AttributeEqual);//= or !=
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API ObjectActionLinkConditionAttackAreaNearData : public ObjectActionLinkConditionData
{
public:
	enum EnumDistanceType {
		kDistanceNone,//距離を指定しない
		kDistanceGreaterEqual,//指定距離以上
		kDistanceLessEqual,//指定距離以下
		kDistanceMax
	};
	enum EnumAttackAttribute {
		kAttributeNone,//属性を指定しない
		kAttributePreset,//プリセットの属性
		kAttributeValue,//数値で指定
		kAttributeMax
	};
public:
	ObjectActionLinkConditionAttackAreaNearData() : ObjectActionLinkConditionData() {};
	virtual ~ObjectActionLinkConditionAttackAreaNearData() {};
public:
	CREATE_FUNC_PARAM(ObjectActionLinkConditionAttackAreaNearData, const rapidjson::Value&, json);
	CREATE_FUNC_PARAM2(ObjectActionLinkConditionAttackAreaNearData, void *, jsCx, void *, jsObj);
#if defined(AGTK_DEBUG)
	virtual void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
	bool init(void *jsCx, void *jsObj);
private:
	//▼攻撃判定の方向と距離を指定
	CC_SYNTHESIZE(bool, _otherDirections, OtherDirections);//指定方向以外
	CC_SYNTHESIZE(bool, _objectDirection, ObjectDirection);//false:このオブジェクトの表示方向、true:８方向から指定
	CC_SYNTHESIZE(int, _directionBit, DirectionBit);//このオブジェクトを基準に8方向から指定。８方向をテンキーの1～4,6～9で表現したときの、方向の数値分だけ1を左シフトしたもののOR値(int)
	CC_SYNTHESIZE(EnumDistanceType, _distanceType, DistanceType);//距離をしていしない(kDistanceNone)、指定距離以上(kDistanceGreaterEqual)、指定距離以下(kDistanceLessEqual)
	CC_SYNTHESIZE(int, _distance, Distance);//指定距離（※ドット数）
	//▼攻撃しているオブジェクトを指定
	CC_SYNTHESIZE(int, _objectType, ObjectType);//攻撃しているオブジェクトを指定（オブジェクトの種類で指定:0, オブジェクトで指定:1, 指定しない:2）
	CC_SYNTHESIZE(int, _objectGroup, ObjectGroup);//オブジェクトのグループを指定（すべてのオブジェクト:-1
	CC_SYNTHESIZE(int, _objectId, ObjectId);//自身のオブジェクト:-2,自分以外のオブジェクト:-3,オブジェクトID>0
	//攻撃の属性を指定
	CC_SYNTHESIZE(EnumAttackAttribute, _attributeType, AttributeType);//攻撃の属性を指定（距離を指定しない:0,プリセットの属性:1,数値で指定:2）
	CC_SYNTHESIZE(int, _attributePresetId, AttributePresetId);//プリセットの属性
	CC_SYNTHESIZE(int, _attributeValue, AttributeValue);//数値で指定の数値
	CC_SYNTHESIZE(int, _attributeEqual, AttributeEqual);//数値で指定の（"=" or "!="）
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API ObjectActionLinkConditionObjectNearData : public ObjectActionLinkConditionData
{
public:
	enum EnumDistanceType {
		kDistanceNone,
		kDistanceGreaterEqual,
		kDistanceLessEqual,
		kDistanceMax
	};
public:
	ObjectActionLinkConditionObjectNearData() : ObjectActionLinkConditionData() {};
	virtual ~ObjectActionLinkConditionObjectNearData() {};
public:
	CREATE_FUNC_PARAM(ObjectActionLinkConditionObjectNearData, const rapidjson::Value&, json);
	CREATE_FUNC_PARAM2(ObjectActionLinkConditionObjectNearData, void *, jsCx, void *, jsObj);
#if defined(AGTK_DEBUG)
	virtual void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
	bool init(void *jsCx, void *jsObj);
private:
	CC_SYNTHESIZE(bool, _otherDirections, OtherDirections);
	CC_SYNTHESIZE(bool, _objectDirection, ObjectDirection);
	CC_SYNTHESIZE(int, _directionBit, DirectionBit);	//このオブジェクトを基準に8方向から指定。８方向をテンキーの1～4,6～9で表現したときの、方向の数値分だけ1を左シフトしたもののOR値(int)
	CC_SYNTHESIZE(int, _distanceType, DistanceType);
	CC_SYNTHESIZE(int, _distance, Distance);
	CC_SYNTHESIZE(int, _objectType, ObjectType);
	CC_SYNTHESIZE(int, _objectGroup, ObjectGroup);
	CC_SYNTHESIZE(int, _objectId, ObjectId);
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API ObjectActionLinkConditionObjectFacingEachOtherData : public ObjectActionLinkConditionData
{
public:
	ObjectActionLinkConditionObjectFacingEachOtherData() : ObjectActionLinkConditionData() {};
	virtual ~ObjectActionLinkConditionObjectFacingEachOtherData() {};
public:
	CREATE_FUNC_PARAM(ObjectActionLinkConditionObjectFacingEachOtherData, const rapidjson::Value&, json);
	CREATE_FUNC_PARAM2(ObjectActionLinkConditionObjectFacingEachOtherData, void *, jsCx, void *, jsObj);
#if defined(AGTK_DEBUG)
	virtual void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
	bool init(void *jsCx, void *jsObj);
private:
	CC_SYNTHESIZE(int, _objectType, ObjectType);
	CC_SYNTHESIZE(int, _objectGroup, ObjectGroup);
	CC_SYNTHESIZE(int, _objectId, ObjectId);
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API ObjectActionLinkConditionObjectFacingData : public ObjectActionLinkConditionData
{
public:
	ObjectActionLinkConditionObjectFacingData() : ObjectActionLinkConditionData() {};
	virtual ~ObjectActionLinkConditionObjectFacingData() {};
public:
	CREATE_FUNC_PARAM(ObjectActionLinkConditionObjectFacingData, const rapidjson::Value&, json);
	CREATE_FUNC_PARAM2(ObjectActionLinkConditionObjectFacingData, void *, jsCx, void *, jsObj);
#if defined(AGTK_DEBUG)
	virtual void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
	bool init(void *jsCx, void *jsObj);
private:
	CC_SYNTHESIZE(int, _objectType, ObjectType);
	CC_SYNTHESIZE(int, _objectGroup, ObjectGroup);
	CC_SYNTHESIZE(int, _objectId, ObjectId);
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API ObjectActionLinkConditionObjectFoundData : public ObjectActionLinkConditionData
{
public:
	ObjectActionLinkConditionObjectFoundData() : ObjectActionLinkConditionData() {};
	virtual ~ObjectActionLinkConditionObjectFoundData() {};
public:
	CREATE_FUNC_PARAM(ObjectActionLinkConditionObjectFoundData, const rapidjson::Value&, json);
	CREATE_FUNC_PARAM2(ObjectActionLinkConditionObjectFoundData, void *, jsCx, void *, jsObj);
#if defined(AGTK_DEBUG)
	virtual void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
	bool init(void *jsCx, void *jsObj);
private:
	CC_SYNTHESIZE(int, _viewportId, ViewportId);
	CC_SYNTHESIZE(bool, _discoveredAcrossLayersObject, DiscoveredAcrossLayersObject);
	CC_SYNTHESIZE(int, _objectType, ObjectType);
	CC_SYNTHESIZE(int, _objectGroup, ObjectGroup);
	CC_SYNTHESIZE(int, _objectId, ObjectId);
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API ObjectActionLinkConditionObjectFacingDirectionData : public ObjectActionLinkConditionData
{
public:
	ObjectActionLinkConditionObjectFacingDirectionData() : ObjectActionLinkConditionData() {};
	virtual ~ObjectActionLinkConditionObjectFacingDirectionData() {};
public:
	CREATE_FUNC_PARAM(ObjectActionLinkConditionObjectFacingDirectionData, const rapidjson::Value&, json);
	CREATE_FUNC_PARAM2(ObjectActionLinkConditionObjectFacingDirectionData, void *, jsCx, void *, jsObj);
#if defined(AGTK_DEBUG)
	virtual void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
	bool init(void *jsCx, void *jsObj);
private:
	CC_SYNTHESIZE(bool, _otherDirections, OtherDirections);
	CC_SYNTHESIZE(bool, _objectDirection, ObjectDirection);
	CC_SYNTHESIZE(int, _directionBit, DirectionBit);	//8方向から指定。８方向をテンキーの1～4,6～9で表現したときの、方向の数値分だけ1を左シフトしたもののOR値(int)
	CC_SYNTHESIZE(int, _objectType, ObjectType);
	CC_SYNTHESIZE(int, _objectGroup, ObjectGroup);
	CC_SYNTHESIZE(int, _objectId, ObjectId);
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API ObjectActionLinkConditionHpZeroData : public ObjectActionLinkConditionData
{
public:
	ObjectActionLinkConditionHpZeroData() : ObjectActionLinkConditionData() {};
	virtual ~ObjectActionLinkConditionHpZeroData() {};
public:
	CREATE_FUNC_PARAM(ObjectActionLinkConditionHpZeroData, const rapidjson::Value&, json);
	CREATE_FUNC_PARAM2(ObjectActionLinkConditionHpZeroData, void *, jsCx, void *, jsObj);
#if defined(AGTK_DEBUG)
	virtual void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
	bool init(void *jsCx, void *jsObj);
private:
	CC_SYNTHESIZE(int, _objectId, ObjectId);
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API ObjectActionLinkConditionCameraOutOfRangeData : public ObjectActionLinkConditionData
{
public:
	ObjectActionLinkConditionCameraOutOfRangeData() : ObjectActionLinkConditionData() {};
	virtual ~ObjectActionLinkConditionCameraOutOfRangeData() {};
public:
	CREATE_FUNC_PARAM(ObjectActionLinkConditionCameraOutOfRangeData, const rapidjson::Value&, json);
	CREATE_FUNC_PARAM2(ObjectActionLinkConditionCameraOutOfRangeData, void *, jsCx, void *, jsObj);
#if defined(AGTK_DEBUG)
	virtual void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
	bool init(void *jsCx, void *jsObj);
private:
	CC_SYNTHESIZE(int, _objectId, ObjectId);//対象とするオブジェクトを指定（-2:自身のオブジェクト,-3:自身以外のオブジェクト,n>0:オブジェクトID）
	CC_SYNTHESIZE(bool, _distanceFlag, DistanceFlag);//カメラ外と認識されるまでの距離を指定（false:距離を指定しない,true:距離を指定する）
	CC_SYNTHESIZE(int, _distance, Distance);//指定距離 ※ドット数
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API ObjectActionLinkConditionLockedData : public ObjectActionLinkConditionData
{
public:
	ObjectActionLinkConditionLockedData() : ObjectActionLinkConditionData() {};
	virtual ~ObjectActionLinkConditionLockedData() {};
public:
	CREATE_FUNC_PARAM(ObjectActionLinkConditionLockedData, const rapidjson::Value&, json);
	CREATE_FUNC_PARAM2(ObjectActionLinkConditionLockedData, void *, jsCx, void *, jsObj);
#if defined(AGTK_DEBUG)
	virtual void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
	bool init(void *jsCx, void *jsObj);
private:
	CC_SYNTHESIZE(int, _lockingObjectId, LockingObjectId);
	CC_SYNTHESIZE(int, _lockedObjectType, LockedObjectType);
	CC_SYNTHESIZE(int, _lockedObjectGroup, LockedObjectGroup);
	CC_SYNTHESIZE(int, _lockedObjectId, LockedObjectId);
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API ObjectActionLinkConditionProbabilityData : public ObjectActionLinkConditionData
{
public:
	ObjectActionLinkConditionProbabilityData() : ObjectActionLinkConditionData() {};
	virtual ~ObjectActionLinkConditionProbabilityData() {};
public:
	CREATE_FUNC_PARAM(ObjectActionLinkConditionProbabilityData, const rapidjson::Value&, json);
	CREATE_FUNC_PARAM2(ObjectActionLinkConditionProbabilityData, void *, jsCx, void *, jsObj);
#if defined(AGTK_DEBUG)
	virtual void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
	bool init(void *jsCx, void *jsObj);
private:
	CC_SYNTHESIZE(double, _probability, Probability);
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API ObjectActionLinkConditionWaitTimeData : public ObjectActionLinkConditionData
{
public:
	ObjectActionLinkConditionWaitTimeData() : ObjectActionLinkConditionData() {};
	virtual ~ObjectActionLinkConditionWaitTimeData() {};
public:
	CREATE_FUNC_PARAM(ObjectActionLinkConditionWaitTimeData, const rapidjson::Value&, json);
#if defined(AGTK_DEBUG)
	virtual void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
private:
	CC_SYNTHESIZE(double, _time, Time);
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API ObjectActionLinkConditionSwitchVariableChangedData : public ObjectActionLinkConditionData
{
public:
	enum EnumSwitchCondition {
		kSwitchConditionOn,
		kSwitchConditionOff,
		kSwitchConditionOnFromOff,
		kSwitchConditionOffFromOn,
		kSwitchConditionMax
	};
	enum EnumVariableCompareValueType {
		kVariableCompareValue,
		kVariableCompareVariable,
		kVariableCompareNaN,
		kVariableCompareValueMax
	};
public:
	ObjectActionLinkConditionSwitchVariableChangedData() : ObjectActionLinkConditionData() {};
	virtual ~ObjectActionLinkConditionSwitchVariableChangedData() {};
public:
	CREATE_FUNC_PARAM(ObjectActionLinkConditionSwitchVariableChangedData, const rapidjson::Value&, json);
	CREATE_FUNC_PARAM2(ObjectActionLinkConditionSwitchVariableChangedData, void *, jsCx, void *, jsObj);
#if defined(AGTK_DEBUG)
	virtual void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
	bool init(void *jsCx, void *jsObj);
private:
	CC_SYNTHESIZE(bool, _swtch, Swtch);
	CC_SYNTHESIZE(int, _switchObjectId, SwitchObjectId);
	CC_SYNTHESIZE(int, _switchId, SwitchId);
	CC_SYNTHESIZE(EnumQualifierType, _switchQualifierId, SwitchQualifierId);
	CC_SYNTHESIZE(EnumSwitchCondition, _switchCondition, SwitchCondition);
	CC_SYNTHESIZE(int, _variableObjectId, VariableObjectId);
	CC_SYNTHESIZE(int, _variableId, VariableId);
	CC_SYNTHESIZE(EnumQualifierType, _variableQualifierId, VariableQualifierId);
	CC_SYNTHESIZE(int, _compareVariableOperator, CompareVariableOperator);
	CC_SYNTHESIZE(EnumVariableCompareValueType, _compareValueType, CompareValueType);
	CC_SYNTHESIZE(double, _compareValue, CompareValue);
	CC_SYNTHESIZE(int, _compareVariableObjectId, CompareVariableObjectId);
	CC_SYNTHESIZE(int, _compareVariableId, CompareVariableId);
	CC_SYNTHESIZE(EnumQualifierType, _compareVariableQualifierId, CompareVariableQualifierId);
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API ObjectActionLinkConditionAnimationFinishedData : public ObjectActionLinkConditionData
{
public:
	ObjectActionLinkConditionAnimationFinishedData() : ObjectActionLinkConditionData() {};
	virtual ~ObjectActionLinkConditionAnimationFinishedData() {};
public:
	CREATE_FUNC_PARAM(ObjectActionLinkConditionAnimationFinishedData, const rapidjson::Value&, json);
	CREATE_FUNC_PARAM2(ObjectActionLinkConditionAnimationFinishedData, void *, jsCx, void *, jsObj);
#if defined(AGTK_DEBUG)
	virtual void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
	bool init(void *jsCx, void *jsObj);
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API ObjectActionLinkConditionJumpTopData : public ObjectActionLinkConditionData
{
public:
	ObjectActionLinkConditionJumpTopData() : ObjectActionLinkConditionData() {};
	virtual ~ObjectActionLinkConditionJumpTopData() {};
public:
	CREATE_FUNC_PARAM(ObjectActionLinkConditionJumpTopData, const rapidjson::Value&, json);
	CREATE_FUNC_PARAM2(ObjectActionLinkConditionJumpTopData, void *, jsCx, void *, jsObj);
#if defined(AGTK_DEBUG)
	virtual void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
	bool init(void *jsCx, void *jsObj);
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API ObjectActionLinkConditionObjectActionChangedData : public ObjectActionLinkConditionData
{
public:
	ObjectActionLinkConditionObjectActionChangedData() : ObjectActionLinkConditionData(), _actionObjectId(-1) {};
	virtual ~ObjectActionLinkConditionObjectActionChangedData() {};
public:
	CREATE_FUNC_PARAM(ObjectActionLinkConditionObjectActionChangedData, const rapidjson::Value&, json);
	CREATE_FUNC_PARAM2(ObjectActionLinkConditionObjectActionChangedData, void *, jsCx, void *, jsObj);
#if defined(AGTK_DEBUG)
	virtual void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
	bool init(void *jsCx, void *jsObj);
private:
	CC_SYNTHESIZE(int, _objectId, ObjectId);
	CC_SYNTHESIZE(int, _actionObjectId, ActionObjectId);
	CC_SYNTHESIZE(int, _actionId, ActionId);
	CC_SYNTHESIZE(bool, _otherActions, OtherActions);
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API ObjectActionLinkConditionSlopeTouchedData : public ObjectActionLinkConditionData
{
public:
	enum EnumDirectionType {
		kDirectionUpper,
		kDirectionLower,
		kDirectionNone,
		kDirectionMax
	};
	enum EnumDownwardType {
		kDownwardLeft,
		kDownwardRight,
		kDownwardNone,
		kDownwardMax
	};
public:
	ObjectActionLinkConditionSlopeTouchedData() : ObjectActionLinkConditionData() {};
	virtual ~ObjectActionLinkConditionSlopeTouchedData() {};
public:
	CREATE_FUNC_PARAM(ObjectActionLinkConditionSlopeTouchedData, const rapidjson::Value&, json);
	CREATE_FUNC_PARAM2(ObjectActionLinkConditionSlopeTouchedData, void *, jsCx, void *, jsObj);
#if defined(AGTK_DEBUG)
	virtual void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
	bool init(void *jsCx, void *jsObj);
private:
	CC_SYNTHESIZE(EnumDirectionType, _directionType, DirectionType);
	CC_SYNTHESIZE(EnumDownwardType, _downwardType, DownwardType);
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API ObjectActionLinkConditionBuriedInWallData : public ObjectActionLinkConditionData
{
public:
	ObjectActionLinkConditionBuriedInWallData() : ObjectActionLinkConditionData() {};
	virtual ~ObjectActionLinkConditionBuriedInWallData() {};
public:
	CREATE_FUNC_PARAM(ObjectActionLinkConditionBuriedInWallData, const rapidjson::Value&, json);
	CREATE_FUNC_PARAM2(ObjectActionLinkConditionBuriedInWallData, void *, jsCx, void *, jsObj);
#if defined(AGTK_DEBUG)
	virtual void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
	bool init(void *jsCx, void *jsObj);
private:
	CC_SYNTHESIZE(int, _objectId, ObjectId);
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API ObjectActionLinkConditionScriptData : public ObjectActionLinkConditionData
{
public:
	ObjectActionLinkConditionScriptData();
	virtual ~ObjectActionLinkConditionScriptData();
public:
	CREATE_FUNC_PARAM(ObjectActionLinkConditionScriptData, const rapidjson::Value&, json);
	const char *getScript();
#if defined(AGTK_DEBUG)
	virtual void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
private:
	CC_SYNTHESIZE_RETAIN(cocos2d::__String *, _script, Script);//スクリプト
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API ObjectActionLinkConditionObjectHitData : public ObjectActionLinkConditionData
{
public:
	ObjectActionLinkConditionObjectHitData() : ObjectActionLinkConditionData() {};
	virtual ~ObjectActionLinkConditionObjectHitData() {};
public:
	CREATE_FUNC_PARAM(ObjectActionLinkConditionObjectHitData, const rapidjson::Value&, json);
	CREATE_FUNC_PARAM2(ObjectActionLinkConditionObjectHitData, void *, jsCx, void *, jsObj);
#if defined(AGTK_DEBUG)
	virtual void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
	bool init(void *jsCx, void *jsObj);
private:
	CC_SYNTHESIZE(int, _wallBit, WallBit);
	CC_SYNTHESIZE(int, _objectType, ObjectType);
	CC_SYNTHESIZE(int, _objectGroup, ObjectGroup);
	CC_SYNTHESIZE(int, _objectId, ObjectId);
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API ObjectActionLinkConditionCustomData : public ObjectActionLinkConditionData
{
public:
	ObjectActionLinkConditionCustomData();
	virtual ~ObjectActionLinkConditionCustomData();
public:
	CREATE_FUNC_PARAM(ObjectActionLinkConditionCustomData, const rapidjson::Value&, json);
	const char *getValueJson();
#if defined(AGTK_DEBUG)
	virtual void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
private:
	CC_SYNTHESIZE(int, _customId, CustomId);
	CC_SYNTHESIZE_RETAIN(cocos2d::__String *, _valueJson, ValueJson);//設定値
};

NS_DATA_END
NS_AGTK_END

#endif	//__AGTK_OBJECT_ACTION_LINK_CONDITION_DATA_H__
