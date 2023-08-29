#ifndef __AGKT_ANIMATION_DATA_H__
#define	__AGKT_ANIMATION_DATA_H__

#include "cocos2d.h"
#include "Lib/Macros.h"
#include "json/document.h"

NS_AGTK_BEGIN
NS_DATA_BEGIN

class AGTKPLAYER_API FrameData : public cocos2d::Ref
{
public:
	enum Interpolate {
		None,//無し
		Linear,//直線
		Curve,//曲線
	};
private:
	FrameData();
	virtual ~FrameData();
public:
	CREATE_FUNC_PARAM(FrameData, const rapidjson::Value&, json);
	cocos2d::Color3B getColor();
	cocos2d::Vec2 getScale();
	cocos2d::Vec2 getOffset();
	cocos2d::Vec2 getCenter();
#if defined(AGTK_DEBUG)
	void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
private:
	CC_SYNTHESIZE(unsigned int, _id, Id);//int
	CC_SYNTHESIZE(int, _frameCount300, FrameCount300);//int
	CC_SYNTHESIZE(Interpolate, _interpType, InterpType);//int
	CC_SYNTHESIZE(bool, _playSe, PlaySe);//bool
	CC_SYNTHESIZE(int, _playSeId, PlaySeId);//int
	CC_SYNTHESIZE(bool, _playVoice, PlayVoice);//bool
	CC_SYNTHESIZE(int, _playVoiceId, PlayVoiceId);//int
	CC_SYNTHESIZE(bool, _centerOrigin, CenterOrigin);//bool
	CC_SYNTHESIZE(double, _originX, OriginX);//double
	CC_SYNTHESIZE(double, _originY, OriginY);//double
	CC_SYNTHESIZE(double, _offsetX, OffsetX);//double
	CC_SYNTHESIZE(double, _offsetY, OffsetY);//double
	CC_SYNTHESIZE(double, _centerX, CenterX);//double
	CC_SYNTHESIZE(double, _centerY, CenterY);//double
	CC_SYNTHESIZE(double, _scalingX, ScalingX);//double
	CC_SYNTHESIZE(double, _scalingY, ScalingY);//double
	CC_SYNTHESIZE(bool, _flipX, FlipX);//bool
	CC_SYNTHESIZE(bool, _flipY, FlipY);//bool
	CC_SYNTHESIZE(double, _rotation, Rotation);
	CC_SYNTHESIZE(unsigned char, _alpha, Alpha);
	CC_SYNTHESIZE(unsigned char, _r, R);
	CC_SYNTHESIZE(unsigned char, _g, G);
	CC_SYNTHESIZE(unsigned char, _b, B);
	CC_SYNTHESIZE(int, _imageTileX, ImageTileX);
	CC_SYNTHESIZE(int, _imageTileY, ImageTileY);
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API TimelineInfoAreaData : public cocos2d::Ref
{
public:
	enum EnumInterpType {
		kInterpNone,
		kInterpLinear,
		kInterpCurve,
		kInterpMax
	};
private:
	TimelineInfoAreaData();
	virtual ~TimelineInfoAreaData();
public:
	CREATE_FUNC_PARAM(TimelineInfoAreaData, const rapidjson::Value&, json);
#if defined(AGTK_DEBUG)
	void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
private:
	CC_SYNTHESIZE(int, _frame, Frame);//フレーム
	CC_SYNTHESIZE(double, _x, X);//位置X(%)
	CC_SYNTHESIZE(double, _y, Y);//位置Y(%)
	CC_SYNTHESIZE(double, _width, Width);//サイズ幅(%)
	CC_SYNTHESIZE(double, _height, Height);//サイズ高さ(%)
	CC_SYNTHESIZE(bool, _backside, Backside);//[接続点]画像の裏側に設定
	CC_SYNTHESIZE(bool, _valid, Valid);
	CC_SYNTHESIZE(EnumInterpType, _interpType, InterpType);//補間タイプ（無し、直線、曲線）
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API TimelineInfoData : public cocos2d::Ref
{
public:
	enum EnumTimelineType {
		kTimelineWall,					// 壁判定
		kTimelineHit,					// 当たり判定
		kTimelineAttack,				// 攻撃判定
		kTimelineConnection,			// 接続点
		kTimelineMax,					// 
		kTimelineMaxWithoutConnection,	// 
	};
private:
	TimelineInfoData();
	virtual ~TimelineInfoData();
public:
	CREATE_FUNC_PARAM(TimelineInfoData, const rapidjson::Value&, json);
	const char *getName();
	agtk::data::TimelineInfoAreaData *getAreaData(int id);
#if defined(AGTK_DEBUG)
	void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
private:
	CC_SYNTHESIZE(unsigned int, _id, Id);
	CC_SYNTHESIZE_RETAIN(cocos2d::__String *, _name, Name);
	CC_SYNTHESIZE(EnumTimelineType, _timelineType, TimelineType);
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _areaList, AreaList);//->TimelineInfoAreaData
};

//-------------------------------------------------------------------------------------------------------------------
class AnimationData;
class AGTKPLAYER_API DirectionData : public cocos2d::Ref
{
private:
	DirectionData();
	virtual ~DirectionData();
public:
	CREATE_FUNC_PARAM(DirectionData, const rapidjson::Value&, json);
	CREATE_FUNC_PARAM(DirectionData, AnimationData *, animationData);
	const char *getName();
	const char *getAnimationName();
	TimelineInfoData *getTimelineInfoData(int id);
#if defined(AGTK_DEBUG)
	void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
	virtual bool init(AnimationData * animationData);
private:
	CC_SYNTHESIZE(unsigned int, _id, Id);
	CC_SYNTHESIZE_RETAIN(cocos2d::__String *, _name, Name);
	CC_SYNTHESIZE(int, _directionBit, DirectionBit);
	CC_SYNTHESIZE(bool, _autoGeneration, AutoGeneration);
	CC_SYNTHESIZE(bool, _yFlip, YFlip);
	CC_SYNTHESIZE(int, _resourceInfoId, ResourceInfoId);//※TODO:必要なくなるかも。
	CC_SYNTHESIZE_RETAIN(cocos2d::__String *, _animationName, AnimationName);
	CC_SYNTHESIZE_RETAIN(cocos2d::__Dictionary *, _timelineInfoList, TimelineInfoList);
	CC_SYNTHESIZE_RETAIN(cocos2d::__Dictionary *, _frameList, FrameList);
	CC_SYNTHESIZE(int, _imageWidth, ImageWidth);
	CC_SYNTHESIZE(int, _imageHeight, ImageHeight);
};

//-------------------------------------------------------------------------------------------------------------------
class ResourceInfoData;
class AGTKPLAYER_API MotionData : public cocos2d::Ref
{
private:
	MotionData();
	virtual ~MotionData();
public:
	CREATE_FUNC_PARAM(MotionData, const rapidjson::Value&, json);
	CREATE_FUNC_PARAM(MotionData, AnimationData *, animationData);
	const char *getName();
	agtk::data::DirectionData *getDirectionData(int id);
	agtk::data::DirectionData *getDirectionDataByDirectionBit(int directionBit, int directionBitOld = 0);
#if defined(AGTK_DEBUG)
	void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
	virtual bool init(AnimationData * animationData);
private:
	CC_SYNTHESIZE(unsigned int, _id, Id);
	CC_SYNTHESIZE_RETAIN(cocos2d::__String *, _name, Name);
	CC_SYNTHESIZE(int, _dispFrameCount300, DispFrameCount300);
	CC_SYNTHESIZE(unsigned int, _loopCount, LoopCount);
	CC_SYNTHESIZE(bool, _infiniteLoop, InfiniteLoop);
	CC_SYNTHESIZE(bool, _reversePlay, ReversePlay);
	CC_SYNTHESIZE_RETAIN(cocos2d::__Dictionary *, _directionList, DirectionList);//->DrectionData
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API ResourceInfoData : public cocos2d::Ref
{
private:
	ResourceInfoData(){}
	virtual ~ResourceInfoData(){}
public:
	enum {
		kInvalidImageId = -2
	};
	CREATE_FUNC_PARAM(ResourceInfoData, const rapidjson::Value&, json);
	bool compareImage(ResourceInfoData *data);
#if defined(AGTK_DEBUG)
	void dump();
#endif
	int getImageIdByResourceSetId(int resourceSetId);
	const std::vector<int> &getImageIdList() const { return _imageIdList; }
	const std::vector<int> &getResourceSetIdList() const { return _resourceSetIdList; }
private:
	virtual bool init(const rapidjson::Value& json);
private:
	CC_SYNTHESIZE(unsigned int, _id, Id);
	CC_SYNTHESIZE(bool, _image, Image);
	CC_SYNTHESIZE(int, _imageId, ImageId);
	std::vector<int> _imageIdList;
	std::vector<int> _resourceSetIdList;
	CC_SYNTHESIZE(unsigned int, _hDivCount, HDivCount);
	CC_SYNTHESIZE(unsigned int, _vDivCount, VDivCount);
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API AnimeParticleData : public cocos2d::Ref
{
private:
	AnimeParticleData();
	virtual ~AnimeParticleData();
public:
	CREATE_FUNC_PARAM(AnimeParticleData, const rapidjson::Value&, json);
	const char *getDescription();
#if defined(AGTK_DEBUG)
	void dump();
#endif
	enum EnumEmitterMode {
		kEmitterModeGravity,
		kEmitterModeRotation,
		kEmitterModeMax
	};
private:
	virtual bool init(const rapidjson::Value& json);
private:
	CC_SYNTHESIZE(unsigned int, _id, Id);
	CC_SYNTHESIZE_RETAIN(cocos2d::__String *, _name, Name);
	CC_SYNTHESIZE_RETAIN(cocos2d::__String *, _description, Description);
	CC_SYNTHESIZE(bool, _playSe, PlaySe);
	CC_SYNTHESIZE(int, _playSeId, PlaySeId);
	CC_SYNTHESIZE(bool, _playVoice, PlayVoice);
	CC_SYNTHESIZE(int, _playVoiceId, PlayVoiceId);
	CC_SYNTHESIZE(int, _duration300, Duration300);
	CC_SYNTHESIZE(bool, _loop, Loop);
	CC_SYNTHESIZE(double, _emitVolume, EmitVolume);
	CC_SYNTHESIZE(bool, _instantEmit, InstantEmit);
	CC_SYNTHESIZE(bool, _previousEmit, PreviousEmit);
	CC_SYNTHESIZE(int, _lifetime300, Lifetime300);
	CC_SYNTHESIZE(double, _lifetimeDispersion, LifetimeDispersion);
	CC_SYNTHESIZE(double, _direction, Direction);
	CC_SYNTHESIZE(double, _directionDispersion, DirectionDispersion);
	CC_SYNTHESIZE(bool, _disableAntiAlias, DisableAntiAlias);
	CC_SYNTHESIZE(double, _speed, Speed);
	CC_SYNTHESIZE(double, _speedDispersion, SpeedDispersion);
	CC_SYNTHESIZE(double, _startSize, StartSize);
	CC_SYNTHESIZE(double, _startSizeDispersion, StartSizeDispersion);
	CC_SYNTHESIZE(double, _endSize, EndSize);
	CC_SYNTHESIZE(double, _endSizeDispersion, EndSizeDispersion);
	CC_SYNTHESIZE(double, _startRotation, StartRotation);
	CC_SYNTHESIZE(double, _startRotationDispersion, StartRotationDispersion);
	CC_SYNTHESIZE(double, _endRotation, EndRotation);
	CC_SYNTHESIZE(double, _endRotationDispersion, EndRotationDispersion);
	CC_SYNTHESIZE(bool, _endRotationIsRelative, EndRotationIsRelative);
	CC_SYNTHESIZE(bool, _addMode, AddMode);
	CC_SYNTHESIZE(int, _emitterMode, EmitterMode);
	CC_SYNTHESIZE(double, _rotations, Rotations);
	CC_SYNTHESIZE(double, _rotationsDispersion, RotationsDispersion);
	CC_SYNTHESIZE(double, _startRotationWidth, StartRotationWidth);
	CC_SYNTHESIZE(double, _startRotationWidthDispersion, StartRotationWidthDispersion);
	CC_SYNTHESIZE(double, _endRotationWidth, EndRotationWidth);
	CC_SYNTHESIZE(double, _endRotationWidthDispersion, EndRotationWidthDispersion);
	CC_SYNTHESIZE(double, _normalAccel, NormalAccel);
	CC_SYNTHESIZE(double, _normalAccelDispersion, NormalAccelDispersion);
	CC_SYNTHESIZE(double, _tangentAccel, TangentAccel);
	CC_SYNTHESIZE(double, _tangentAccelDispersion, TangentAccelDispersion);
	CC_SYNTHESIZE(double, _gravityDirection, GravityDirection);
	CC_SYNTHESIZE(double, _gravity, Gravity);
	CC_SYNTHESIZE(double, _x, X);
	CC_SYNTHESIZE(double, _xDispersion, XDispersion);
	CC_SYNTHESIZE(double, _y, Y);
	CC_SYNTHESIZE(double, _yDispersion, YDispersion);
	CC_SYNTHESIZE(int, _startAlpha, StartAlpha);
	CC_SYNTHESIZE(int, _startAlphaDispersion, StartAlphaDispersion);
	CC_SYNTHESIZE(int, _startR, StartR);
	CC_SYNTHESIZE(int, _startRDispersion, StartRDispersion);
	CC_SYNTHESIZE(int, _startG, StartG);
	CC_SYNTHESIZE(int, _startGDispersion, StartGDispersion);
	CC_SYNTHESIZE(int, _startB, StartB);
	CC_SYNTHESIZE(int, _startBDispersion, StartBDispersion);
	CC_SYNTHESIZE(int, _middleAlpha, MiddleAlpha);
	CC_SYNTHESIZE(int, _middleAlphaDispersion, MiddleAlphaDispersion);
	CC_SYNTHESIZE(int, _middleR, MiddleR);
	CC_SYNTHESIZE(int, _middleRDispersion, MiddleRDispersion);
	CC_SYNTHESIZE(int, _middleG, MiddleG);
	CC_SYNTHESIZE(int, _middleGDispersion, MiddleGDispersion);
	CC_SYNTHESIZE(int, _middleB, MiddleB);
	CC_SYNTHESIZE(int, _middleBDispersion, MiddleBDispersion);
	CC_SYNTHESIZE(int, _middlePercent, MiddlePercent);
	CC_SYNTHESIZE(int, _endAlpha, EndAlpha);
	CC_SYNTHESIZE(int, _endAlphaDispersion, EndAlphaDispersion);
	CC_SYNTHESIZE(int, _endR, EndR);
	CC_SYNTHESIZE(int, _endRDispersion, EndRDispersion);
	CC_SYNTHESIZE(int, _endG, EndG);
	CC_SYNTHESIZE(int, _endGDispersion, EndGDispersion);
	CC_SYNTHESIZE(int, _endB, EndB);
	CC_SYNTHESIZE(int, _endBDispersion, EndBDispersion);
	CC_SYNTHESIZE(int, _templateId, TemplateId);
	CC_SYNTHESIZE(bool, _check, Check);
	CC_SYNTHESIZE(bool, _touchDisappear, TouchDisappear);
	CC_SYNTHESIZE(int, _disappearCount, DisappearCount);
	CC_SYNTHESIZE(bool, _touchBound, TouchBound);
	CC_SYNTHESIZE(double, _repulsion, Repulsion);
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API AnimationData : public cocos2d::Ref
{
public:
	enum EnumAnimType {
		kMotion,
		kEffect,
		kParticle,
		kMax
	};
	enum EnumOriginType {
		kOriginLeftUp,
		kOriginCenter,
		kOriginFoot,
		kOriginXy,
		kOriginMax
	};
private:
	AnimationData();
	virtual ~AnimationData();
public:
	CREATE_FUNC_PARAM(AnimationData, const rapidjson::Value&, json);
	const char *getName();
	const char *getMemo();
	const char *getAnimationName();
	ResourceInfoData *getResourceInfoData(int id = -1);
	agtk::data::MotionData *getMotionData(int id);
	agtk::data::DirectionData *getDirectionData(int motionId, int directionId);
public:
	cocos2d::Vec2 calcOriginPosition(cocos2d::Size& size);
#if defined(AGTK_DEBUG)
	void dump();
#endif
	const std::vector<int> &getResourceSetIdList() const { return _resourceSetIdList; }
	const std::vector<string> &getResourceSetNameList() const { return _resourceSetNameList; }
private:
	virtual bool init(const rapidjson::Value& json);
private:
	CC_SYNTHESIZE(unsigned int, _id, Id);
	CC_SYNTHESIZE_RETAIN(cocos2d::__String *, _name, Name);
	CC_SYNTHESIZE(EnumAnimType, _type, Type);
	CC_SYNTHESIZE(EnumOriginType, _originType, OriginType);
	CC_SYNTHESIZE(int, _originX, OriginX);
	CC_SYNTHESIZE(int, _originY, OriginY);

	CC_SYNTHESIZE(int, _frameCount300, FrameCount300);
	CC_SYNTHESIZE(bool, _infiniteLoop, InfiniteLoop);
	CC_SYNTHESIZE(int, _loopCount, LoopCount);
	CC_SYNTHESIZE(bool, _reversePlay, ReversePlay);

	CC_SYNTHESIZE_RETAIN(cocos2d::__Dictionary *, _resourceInfoList, ResourceInfoList);//->ResourceInfoData
	std::vector<int> _resourceSetIdList;
	std::vector<string> _resourceSetNameList;
	CC_SYNTHESIZE_RETAIN(cocos2d::__Dictionary *, _motionList, MotionList);//->MotionData
	CC_SYNTHESIZE_RETAIN(cocos2d::__Dictionary *, _frameList, FrameList);//->FrameData
	CC_SYNTHESIZE_RETAIN(cocos2d::__Dictionary *, _particleList, ParticleList);//->ParticleData
	CC_SYNTHESIZE_RETAIN(cocos2d::__String *, _memo, Memo);
	CC_SYNTHESIZE(bool, _folder, Folder);
	CC_SYNTHESIZE_RETAIN(cocos2d::__Dictionary *, _children, Children);
	CC_SYNTHESIZE_RETAIN(cocos2d::__String *, _animationName, AnimationName);
};

NS_DATA_END
NS_AGTK_END

#endif	//__AGKT_ANIMATION_DATA_H__
