﻿/**
*  SS6PlayerData.h
*/

#ifndef SS6PlayerData_h
#define SS6PlayerData_h

namespace ss
{

/*
* ssbpに含まれるデータの定義
*/

/**
* 型定義
*/
typedef short			ss_s16;
typedef unsigned short	ss_u16;
typedef unsigned int	ss_u32;
typedef int				ss_s32;
typedef int				ss_offset;
typedef long long		ss_s64;

struct EffectParticleInfiniteEmitEnabled
{
	ss_s32	flag;					// ダミーデータ
};
struct EffectParticleTurnToDirectionEnabled
{
	float	Rotation;				// 方向オフセット
};
struct EffectParticlePointGravity
{
	float	Position_x;				// 重力点X
	float	Position_y;				// 重力点Y
	float	Power;					// パワー
};

struct EffectParticleElementTransSize
{
	float	SizeXMinValue;			// 幅倍率最小
	float	SizeXMaxValue;			// 幅倍率最大
	float	SizeYMinValue;			// 高さ倍率最小
	float	SizeYMaxValue;			// 高さ倍率最大
	float	ScaleFactorMinValue;	// 倍率最小
	float	ScaleFactorMaxValue;	// 倍率最大
};

struct EffectParticleElementSize
{
	float	SizeXMinValue;			// 幅倍率最小
	float	SizeXMaxValue;			// 幅倍率最大
	float	SizeYMinValue;			// 高さ倍率最小
	float	SizeYMaxValue;			// 高さ倍率最大
	float	ScaleFactorMinValue;	// 倍率最小
	float	ScaleFactorMaxValue;	// 倍率最大
};

struct EffectParticleElementAlphaFade
{
	float	disprangeMinValue;		// 表示区間開始
	float	disprangeMaxValue;		// 表示区間終了
};

struct EffectParticleElementTransColor
{
	ss_u32	ColorMinValue;			// 設定カラー最小
	ss_u32	ColorMaxValue;			// 設定カラー最大
};

struct EffectParticleElementInitColor
{
	ss_u32	ColorMinValue;			// 設定カラー最小
	ss_u32	ColorMaxValue;			// 設定カラー最大
};

struct EffectParticleElementTangentialAcceleration
{
	float	AccelerationMinValue;	// 設定加速度最小
	float	AccelerationMaxValue;	// 設定加速度最大
};

struct EffectParticleElementTransSpeed
{
	float	SpeedMinValue;			// 速度目標値最小
	float	SpeedMaxValue;			// 速度目標値最大
};

struct EffectParticleElementRotationTrans
{
	float	RotationFactor;			// 角度目標加算値
	float	EndLifeTimePer;			// 到達時間
};

struct EffectParticleElementRotation
{
	float	RotationMinValue;		// 角度初期値最小
	float	RotationMaxValue;		// 角度初期値最大
	float	RotationAddMinValue;	// 角度初期加算値最小
	float	RotationAddMaxValue;	// 角度初期加算値最大
};

struct EffectParticleElementPosition
{
	float	OffsetXMinValue;		// X座標に加算最小
	float	OffsetXMaxValue;		// X座標に加算最大
	float	OffsetYMinValue;		// Y座標に加算最小
	float	OffsetYMaxValue;		// Y座標に加算最大
};

struct EffectParticleElementGravity
{
	float	Gravity_x;				// X方向の重力
	float	Gravity_y;				// Y方向の重力
};


struct EffectParticleElementDelay
{
	ss_s32	DelayTime;				// 遅延時間
};

struct EffectParticleElementRndSeedChange
{
	ss_s32	Seed;					// 上書きするシード値
};

struct EffectParticleElementBasic
{
	ss_s32		priority;			// 表示優先度
	ss_s32		maximumParticle;	// 最大パーティクル数
	ss_s32		attimeCreate;		// 一度に作成するパーティクル数
	ss_s32		interval;			// 生成間隔
	ss_s32		lifetime;			// エミッター生存時間
	float		speedMinValue;		// 初速最小
	float		speedMaxValue;		// 初速最大
	ss_s32		lifespanMinValue;	// パーティクル生存時間最小
	ss_s32		lifespanMaxValue;	// パーティクル生存時間最大
	float		angle;				// 射出方向
	float		angleVariance;		// 射出方向範囲
};

struct EffectNode
{
	ss_s16		arrayIndex;			// 通し番号
	ss_s16		parentIndex;		// 親の番号
	ss_s16		type;				// ノードの種類
	ss_s16		cellIndex;			// セルの番号
	ss_s16		blendType;			// 描画方法
	ss_s16		numBehavior;		// コマンドパラメータ数
	ss_offset	Behavior;			// コマンド詳細
};

struct EffectFile
{
	ss_offset	name;				// const char* エフェクトファイル名
	ss_s16		fps;				// FPS
	ss_s16		isLockRandSeed;		// 乱数を固定するかどうか
	ss_s16		lockRandSeed;		// 固定する場合の乱数の種
	ss_s16		layoutScaleX;		// レイアウトスケールX
	ss_s16		layoutScaleY;		// レイアウトスケールY
	ss_s16		numNodeList;		// 含まれるノード数
	ss_offset	effectNode;			// const EffectNode*
};

/**
* セルマップ
*/
struct CellMap
{
	ss_offset	name;				// const char*
	ss_offset	imagePath;			// const char*
	ss_s16		index;				// セルマップのインデックス
	ss_s16		wrapmode;			// ラップモード
	ss_s16		filtermode;			// フィルタモード
	ss_s16		reserved;
};


/**
* セル
*/
struct Cell
{
	ss_offset	name;				// const char*
	ss_offset	cellMap;			// const CellMap*
	ss_s16		indexInCellMap;		// セルマップ内のインデックス
	ss_s16		x;					// セルの開始座標
	ss_s16		y;					// セルの開始座標
	ss_u16		width;				// セルの幅
	ss_u16		height;				// セルの高さ
	ss_s16		reserved;			// ダミーデータ
	float		pivot_X;			// 原点補正
	float		pivot_Y;			// 原点補正
	float		u1;					// セルのUV
	float		v1;					// セルのUV
	float		u2;					// セルのUV
	float		v2;					// セルのUV
};


/**
* アニメ情報
*/
struct AnimationData
{
	ss_offset	name;				// const char*
	ss_offset	defaultData;		// const AnimationInitialData*
	ss_offset	frameData;			// const ss_s16*
	ss_offset	userData;			// const ss_s16*
	ss_offset	labelData;			// const ss_s16*
	ss_offset	meshsDataUV;		// const ss_s16*
	ss_offset	meshsDataIndices;	// const ss_s16*
	ss_s16		startFrames;		// 開始フレーム
	ss_s16		endFrames;			// 終了フレーム
	ss_s16		totalFrames;		// 総フレーム
	ss_s16		fps;				// FPS
	ss_s16		labelNum;			// ラベル数
	ss_s16		canvasSizeW;		// 基準枠幅
	ss_s16		canvasSizeH;		// 基準枠高さ
	ss_s16		dummy;				// ダミーデータ
	float		canvasPvotX;		// 基準枠位置
	float		canvasPvotY;		// 基準枠位置
};


/**
* 各パーツの初期値
*/
struct AnimationInitialData
{
	ss_s16		index;
	ss_s16		dummy;
	ss_u32		lowflag;
	ss_u32		highflag;
	ss_s16		priority;
	ss_s16		cellIndex;
	ss_s16		opacity;
	ss_s16		localopacity;
	ss_s16		masklimen;
	ss_s16		dummy2;
	float		positionX;
	float		positionY;
	float		positionZ;
	float		pivotX;
	float		pivotY;
	float		rotationX;
	float		rotationY;
	float		rotationZ;
	float		scaleX;
	float		scaleY;
	float		localscaleX;
	float		localscaleY;
	float		size_X;
	float		size_Y;
	float		uv_move_X;
	float		uv_move_Y;
	float		uv_rotation;
	float		uv_scale_X;
	float		uv_scale_Y;
	float		boundingRadius;
	//インスタンスアトリビュート
	int			instanceValue_curKeyframe;
	int			instanceValue_startFrame;
	int			instanceValue_endFrame;
	int			instanceValue_loopNum;
	float		instanceValue_speed;
	int			instanceValue_loopflag;
	//エフェクトアトリビュート
	int			effectValue_curKeyframe;
	int			effectValue_startTime;
	float		effectValue_speed;
	int			effectValue_loopflag;
};

/**
* パーツ
*/
struct PartData
{
	ss_offset	name;				// const char*
	ss_s16		index;				// SS内のパーツインデックス
	ss_s16		parentIndex;		// 親のパーツインデックス
	ss_s16		type;				// パーツ種別
	ss_s16		boundsType;			// 当たり判定種類
	ss_s16		alphaBlendType;		// BlendType
	ss_s16		dummy1;				//ダミーデータ
	ss_offset	refname;			// const char*　インスタンスとして配置されるアニメーション名
	ss_offset	effectfilename;		// const char*　参照するエフェクトファイル名
	ss_offset	colorLabel;			// const char*	カラーラベル
	ss_s16		maskInfluence;		// マスク対象
};

/**
* アニメーションパック情報
*/
struct AnimePackData
{
	ss_offset	name;				// const char*
	ss_offset	parts;				// const PartData*
	ss_offset	animations;			// const AnimationData*
	ss_s16		numParts;
	ss_s16		numAnimations;
};

/**
* プロジェクト情報
*/
struct ProjectData
{
	ss_u32		dataId;
	ss_u32		version;
	ss_u32		flags;				// プロジェクト設定
	ss_offset	imageBaseDir;		// const char*
	ss_offset	cells;				// const Cell*
	ss_offset	animePacks;			// const AnimePackData*
	ss_offset	effectFileList;		// const EffectFileList*
	ss_s16		numCells;
	ss_s16		numAnimePacks;
	ss_s16		numEffectFileList;	// プロジェクトに登録されているエフェクトファイル数
};

};	// namespace ss

#endif
