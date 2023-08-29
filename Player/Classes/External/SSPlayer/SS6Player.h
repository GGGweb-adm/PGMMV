//-----------------------------------------------------------
// SS6Player for Cocos2d-x v1.6.0
//
// Copyright(C) Web Technology Corp.
// http://www.webtech.co.jp/
//-----------------------------------------------------------
//
// SS6Player.h
//


/************************************************************
対応するssbpフォーマットはバージョン11です。
Ss6ConverterのフォーマットバージョンはSpriteStudio6-SDKを参照してください。
https://github.com/SpriteStudio/SpriteStudio6-SDK

- Quick start
 
  #include "./SSPlayer/SS6Player.h"

  
  // SS6プレイヤーの宣言
  ss::SSPlayerControl *ssplayer;
  ss::ResourceManager *resman;


  //プレイヤーを使用する前の初期化処理
  //この処理はアプリケーションの初期化で１度だけ行ってください。
  ss::SSPlatformInit();



  //リソースマネージャの作成
  resman = ss::ResourceManager::getInstance();

  //SS6Player for Cocos2d-xではSSPlayerControlを作成し、getSSPInstance()を経由してプレイヤーを操作します
  //プレイヤーの作成
  ssplayer = ss::SSPlayerControl::create();

  //アニメデータをリソースに追加
  //それぞれのプラットフォームに合わせたパスへ変更してください。
  resman->addData("character_template_comipo/character_template1.ssbp");
  //プレイヤーにリソースを割り当て
  ssplayer->getSSPInstance()->setData("character_template1");				// ssbpファイル名（拡張子不要）
  //再生するモーションを設定
  ssplayer->getSSPInstance()->play("character_template_3head/stance");		// アニメーション名を指定(ssae名/アニメーション名)


  //表示位置を設定
  Size size = cocos2d::Director::getInstance()->getWinSize();
  ssplayer->setPosition(size.width / 2, size.height / 2);	//位置の設定
  ssplayer->setScale(1.0f, 1.0f);							//スケール設定
  ssplayer->setRotation(0);									//角度設定
  ssplayer->setOpacity(255);								//透明度設定
  ssplayer->getSSPInstance()->setColor(255, 255, 255);		//カラー値設定
  ssplayer->getSSPInstance()->setFlip(false, false);		//反転設定

  //プレイヤーをゲームシーンに追加
  this->addChild(ssplayer, 10);




  使用するアニメーションに合わせて Playerクラス定義部分にある設定用定数を変更してください。

  プレイヤーの制限についてはこちらのページを参照してください。
  https://github.com/SpriteStudio/SS6PlayerForCocos2d-x/wiki

  使用方法についてはPlayerクラスのコメントを参照してください。

*************************************************************/

#ifndef SSPlayer_h
#define SSPlayer_h

#include "cocos2d.h"
#include "SS6PlayerData.h"
#include "SS6PlayerTypes.h"
#include "SS6PlayerPlatform.h"

//エフェクト関連
#include "./Common/Loader/ssloader.h"
#include "./Common/Animator/ssplayer_macro.h"
#include "./Common/Animator/ssplayer_matrix.h"
#include "./Common/Animator/ssplayer_effectfunction.h"
#include "./Common/Animator/ssplayer_cellmap.h"
#include "./Common/Animator/ssplayer_PartState.h"
//#include "./Common/Animator/MersenneTwister.h"

#pragma warning(disable : 4996)

namespace ss
{
class ResourceManager;
class CustomSprite;
class CellCache;
class CellRef;
class AnimeCache;
#ifdef USE_AGTK//sakihama-h, 2018.05.16
struct AnimeRef;
#else
class AnimeRef;
#endif
class ResourceSet;
struct ProjectData;
class SSSize;
class Player;

//関数定義
extern void get_uv_rotation(float *u, float *v, float cu, float cv, float deg);

/**
* 定数
*/

#define __SSPI__	(3.14159265358979323846f)
#define __SS2PI__	(__SSPI__ * 2)
#define SSRadianToDegree(Radian) ((float)( Radian * __SS2PI__ )/ 360.0f )
#define SSDegreeToRadian(Degree) ((float)( Degree * 360.0f) / __SS2PI__)


#define SS_SAFE_DELETE(p)            do { if(p) { delete (p); (p) = 0; } } while(0)
#define SS_SAFE_DELETE_ARRAY(p)     do { if(p) { delete[] (p); (p) = 0; } } while(0)
#define SS_SAFE_FREE(p)                do { if(p) { free(p); (p) = 0; } } while(0)
#define SS_SAFE_RELEASE(p)            do { if(p) { (p)->release(); } } while(0)
#define SS_SAFE_RELEASE_NULL(p)        do { if(p) { (p)->release(); (p) = 0; } } while(0)
#define SS_SAFE_RETAIN(p)            do { if(p) { (p)->retain(); } } while(0)
#define SS_BREAK_IF(cond)            if(cond) break

#ifdef COCOS2D_DEBUG
	#define SSLOG(...)       do {} while (0)
	#define SS_ASSERT(cond)    assert(cond)
	#define SS_ASSERT2(cond, msg) SS_ASSERT(cond)
	#define SSLOGERROR(format,...)  do {} while (0)
#else
	#define SSLOG(...)       do {} while (0)
	#define SS_ASSERT(cond)
	#define SS_ASSERT2(cond, msg) ((void)(cond))
	#define SSLOGERROR(format,...)  do {} while (0)
#endif

// attributeのindex  
enum {
	ATTRIBUTE_POS,
	ATTRIBUTE_COL,
	ATTRIBUTE_UV,
};

// uniformのindex  
enum {
	WVP,
	SAMPLER,
	RATE,
};

/**
* SSPlayerControl 
  Cocos2d-xからSSPlayerを使用するためのラッパークラス
  アプリケーション側はSSPlayerControlを作成し、getSSPInstance()を経由してプレイヤーを操作します
*/
class SSPlayerControl : public cocos2d::Sprite
{
public:
	/**
	* SSPlayerControlインスタンスを構築します.
	*
	* @param  resman  使用するResourceManagerインスタンス. 省略時はデフォルトインスタンスが使用されます.
	* @return Playerインスタンス
	*/
	static SSPlayerControl* create(ResourceManager* resman = nullptr);

	/**
	* Playerのポインタを取得します.
	* SS6Player for Cocos2d-xでは　ssplayer->getSSPInstance()->play("") のように
	* getSSPInstance()を経由してプレイヤーにアクセスします
	* プレイヤーの使用方法については Player クラスのコメントを参照してください。
	*
	* @return Playerインスタンス
	*/
	Player *getSSPInstance();

	/**
	* RenderTextureに描画する場合にレンダリング用のブレンドファンクションを使用します.
	* レンダリングターゲットとアルファ値がブレンドされてしまうためカラー値のみのレンダリングファンクションを使用します。
	*
	* //使用例
	* auto rt = RenderTexture::create(1280, 720);
	* rt->begin();
	* ssplayer->renderingBlendFuncEnable(true);	//レンダリング用ブレンドファンクションを使用する
	* ssplayer->visit();
	* ssplayer->renderingBlendFuncEnable(false);
	* rt->end();
	*
	* @param  flg	      通常描画:false、レンダリング描画:true
	*/
	void renderingBlendFuncEnable(int flg) { _enableRenderingBlendFunc = flg; };

public:
	SSPlayerControl();
	~SSPlayerControl();

	virtual bool init();
	virtual void update(float dt);
	virtual void draw(cocos2d::Renderer *renderer, const cocos2d::Mat4 &transform, uint32_t flags);
	virtual void setPosition(const cocos2d::Vec2& position);
	virtual void setPosition(float x, float y);

	void onDraw(cocos2d::Renderer *renderer, const cocos2d::Mat4 &transform, uint32_t flags);
	void onRenderingDraw(cocos2d::Renderer *renderer, const cocos2d::Mat4 &transform, uint32_t flags);
	void initCustomShaderProgram( );

	static cocos2d::GLProgram*	_defaultShaderProgram;
	static cocos2d::GLProgram*	_MASKShaderProgram;
	static cocos2d::GLProgram*	_partColorMIXONEShaderProgram;
	static cocos2d::GLProgram*	_partColorMIXVERTShaderProgram;
	static cocos2d::GLProgram*	_partColorMULShaderProgram;
	static cocos2d::GLProgram*	_partColorADDShaderProgram;
	static cocos2d::GLProgram*	_partColorSUBShaderProgram;

	static std::map<int, int> _MASK_uniform_map;
	static std::map<int, int> _MIXONE_uniform_map;
	static std::map<int, int> _MIXVERT_uniform_map;
	static std::map<int, int> _MUL_uniform_map;
	static std::map<int, int> _ADD_uniform_map;
	static std::map<int, int> _SUB_uniform_map;
#ifdef USE_AGTK//sakiham-h, 2018.07.02
protected:
#else
private:
#endif
	Player *_ssp;
	cocos2d::CustomCommand _customCommand;
	cocos2d::CustomCommand _customCommandRendering;

	cocos2d::Vec2 _position;		//プレイヤーのポジション
	bool _enableRenderingBlendFunc;	//レンダリング用のブレンドステートを使用する
};

/**
* State
パーツの情報を格納します。Stateの内容をもとに描画処理を作成してください。
*/
struct State
{
	std::string name;				/// パーツ名
	int flags;						/// このフレームで更新が行われるステータスのフラグ
	int flags2;						/// このフレームで更新が行われるステータスのフラグ2
	int cellIndex;					/// パーツに割り当てられたセルの番号
	float x;						/// SSアトリビュート：X座標
	float y;						/// SSアトリビュート：Y座標
	float z;						/// SSアトリビュート：Z座標
	float pivotX;					/// 原点Xオフセット＋セルに設定された原点オフセットX
	float pivotY;					/// 原点Yオフセット＋セルに設定された原点オフセットY
	float rotationX;				/// X回転
	float rotationY;				/// Y回転
	float rotationZ;				/// Z回転
	float scaleX;					/// Xスケール
	float scaleY;					/// Yスケール
	float localscaleX;				/// Xローカルスケール
	float localscaleY;				/// Yローカルスケール
	int opacity;					/// 不透明度（0～255）
	int localopacity;				/// ローカル不透明度（0～255）
	float size_X;					/// SSアトリビュート：Xサイズ
	float size_Y;					/// SSアトリビュート：Xサイズ
	float uv_move_X;				/// SSアトリビュート：UV X移動
	float uv_move_Y;				/// SSアトリビュート：UV Y移動
	float uv_rotation;				/// SSアトリビュート：UV 回転
	float uv_scale_X;				/// SSアトリビュート：UV Xスケール
	float uv_scale_Y;				/// SSアトリビュート：UV Yスケール
	float boundingRadius;			/// SSアトリビュート：当たり半径
	int partsColorFunc;				/// SSアトリビュート：パーツカラーのブレンド方法
	int partsColorType;				/// SSアトリビュート：パーツカラーの単色か頂点カラーか。
	int masklimen;					/// マスク強度
	int priority;					/// 優先度
	bool flipX;						/// 横反転（親子関係計算済）
	bool flipY;						/// 縦反転（親子関係計算済）
	bool isVisibled;				/// 非表示（親子関係計算済）
	SSV3F_C4B_T2F_Quad quad;		/// 頂点データ、座標、カラー値、UVが含まれる（頂点変形、サイズXY、UV移動XY、UVスケール、UV回転、反転が反映済）
	SSPARTCOLOR_RATE rate;			/// パーツカラーに含まれるレート
	TextuerData texture;			/// セルに対応したテクスチャ番号（ゲーム側で管理している番号を設定する）
	SSRect rect;					/// セルに対応したテクスチャ内の表示領域（開始座標、幅高さ）
	int blendfunc;					/// パーツに設定されたブレンド方法
	float mat[16];					/// パーツの位置を算出するためのマトリクス（親子関係計算済）
	//再生用パラメータ
	float Calc_rotationX;			/// X回転（親子関係計算済）
	float Calc_rotationY;			/// Y回転（親子関係計算済）
	float Calc_rotationZ;			/// Z回転（親子関係計算済）
	float Calc_scaleX;				/// Xスケール（親子関係計算済）
	float Calc_scaleY;				/// Yスケール（親子関係計算済）
	int Calc_opacity;				/// 不透明度（0～255）（親子関係計算済）
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
	//メッシュデータ
	std::vector<SsVector3> meshVertexPoint;

	void init()
	{
		flags = 0;
		cellIndex = 0;
		x = 0.0f;
		y = 0.0f;
		z = 0.0f;
		pivotX = 0.0f;
		pivotY = 0.0f;
		rotationX = 0.0f;
		rotationY = 0.0f;
		rotationZ = 0.0f;
		scaleX = 1.0f;
		scaleY = 1.0f;
		localscaleX = 1.0f;
		localscaleY = 1.0f;
		opacity = 255;
		localopacity = 255;
		size_X = 1.0f;
		size_Y = 1.0f;
		uv_move_X = 0.0f;
		uv_move_Y = 0.0f;
		uv_rotation = 0.0f;
		uv_scale_X = 1.0f;
		uv_scale_Y = 1.0f;
		boundingRadius = 0.0f;
		masklimen = 0;
		priority = 0;
		partsColorFunc = 0;
		partsColorType = 0;
		rate.oneRate = 1.0f;
		rate.vartTLRate = 1.0f;
		rate.vartTRRate = 1.0f;
		rate.vartBLRate = 1.0f;
		rate.vartBRRate = 1.0f;
		flipX = false;
		flipY = false;
		isVisibled = false;
		memset(&quad, 0, sizeof(quad));
		texture.handle = 0;
		texture.size_w = 0;
		texture.size_h = 0;
		rect.size.height = 0;
		rect.size.width = 0;
		rect.origin.x = 0;
		rect.origin.y = 0;
		blendfunc = 0;
		memset(&mat, 0, sizeof(mat));
		instanceValue_curKeyframe = 0;
		instanceValue_startFrame = 0;
		instanceValue_endFrame = 0;
		instanceValue_loopNum = 0;
		instanceValue_speed = 0;
		instanceValue_loopflag = 0;
		effectValue_curKeyframe = 0;
		effectValue_startTime = 0;
		effectValue_speed = 0;
		effectValue_loopflag = 0;

		Calc_rotationX = 0.0f;
		Calc_rotationY = 0.0f;
		Calc_rotationZ = 0.0f;
		Calc_scaleX = 1.0f;
		Calc_scaleY = 1.0f;
		Calc_opacity = 255;

		meshVertexPoint.clear();
	}

	State() { init(); }
};

/**
* CustomSprite
*/
class CustomSprite
{
private:

private:
	float				_opacity;
	int					_hasPremultipliedAlpha;
	bool				_flipX;
	bool				_flipY;

public:
	float				_mat[16];		//継承マトリクス
	float				_localmat[16];	//ローカルマトリクス
	State				_state;
	bool				_isStateChanged;
	CustomSprite*		_parent;
	Player*				_ssplayer;
	Player*				_parentPlayer;
	float				_liveFrame;
	SSV3F_C4B_T2F_Quad	_sQuad;

	PartData			_partData;
	bool				_maskInfluence;		//親パーツのマスク対象を加味したマスク対象

	//エフェクト用パラメータ
	SsEffectRenderV2*	refEffect;
	SsPartState			partState;

	//モーションブレンド用ステータス
	State				_orgState;

	//エフェクト制御用ワーク
	bool effectAttrInitialized;
	float effectTimeTotal;

	//メッシュ情報
	bool					_meshIsBind;		//バインドされたメッシュか？
	int						_meshVertexSize;	//メッシュの頂点サイズ
	std::vector<SsVector2>	_meshVertexUV;		//メッシュのUV
	std::vector<SsVector3>	_meshIndices;		//メッシュの頂点順
	float*					_mesh_uvs;			// UVバッファ
//	float*					_mesh_colors;		// カラーバッファ
	unsigned char*			_mesh_colors;		// カラーバッファ
	float*					_mesh_vertices;		// 座標バッファ
	int						_meshTriangleSize;	//トライアングルのサイズ
	unsigned short*			_mesh_indices;		// 頂点順


	SSPlayerControl*	_playercontrol;

public:
	CustomSprite();
	virtual ~CustomSprite();

	static CustomSprite* create();

	void initState()
	{
		_state.init();
		_isStateChanged = true;
	}

	void setStateValue(float& ref, float value)
	{
		if (ref != value)
		{
			ref = value;
			_isStateChanged = true;
		}
	}

	void setStateValue(int& ref, int value)
	{
		if (ref != value)
		{
			ref = value;
			_isStateChanged = true;
		}
	}

	void setStateValue(ss_s64& ref, ss_s64 value)
	{
		if (ref != value)
		{
			ref = value;
			_isStateChanged = true;
		}
	}

	void setStateValue(bool& ref, bool value)
	{
		if (ref != value)
		{
			ref = value;
			_isStateChanged = true;
		}
	}

	void setStateValue(SSV3F_C4B_T2F_Quad& ref, SSV3F_C4B_T2F_Quad value)
	{
		//		if (ref != value)
		{
			ref = value;
			_isStateChanged = true;
		}
	}

	void setStateValue(SSPARTCOLOR_RATE& ref, SSPARTCOLOR_RATE value)
	{
		//		if (ref != value)
		{
			ref = value;
			_isStateChanged = true;
		}
	}

	void setState(const State& state)
	{
		_state.name = state.name;
		setStateValue(_state.flags, state.flags);
		setStateValue(_state.flags2, state.flags2);
		setStateValue(_state.cellIndex, state.cellIndex);
		setStateValue(_state.x, state.x);
		setStateValue(_state.y, state.y);
		setStateValue(_state.z, state.z);
		setStateValue(_state.pivotX, state.pivotX);
		setStateValue(_state.pivotY, state.pivotY);
		setStateValue(_state.rotationX, state.rotationX);
		setStateValue(_state.rotationY, state.rotationY);
		setStateValue(_state.rotationZ, state.rotationZ);
		setStateValue(_state.scaleX, state.scaleX);
		setStateValue(_state.scaleY, state.scaleY);
		setStateValue(_state.localscaleX, state.localscaleX);
		setStateValue(_state.localscaleY, state.localscaleY);
		setStateValue(_state.opacity, state.opacity);
		setStateValue(_state.localopacity, state.localopacity);
		setStateValue(_state.size_X, state.size_X);
		setStateValue(_state.size_Y, state.size_Y);
		setStateValue(_state.uv_move_X, state.uv_move_X);
		setStateValue(_state.uv_move_Y, state.uv_move_Y);
		setStateValue(_state.uv_rotation, state.uv_rotation);
		setStateValue(_state.uv_scale_X, state.uv_scale_X);
		setStateValue(_state.uv_scale_Y, state.uv_scale_Y);
		setStateValue(_state.boundingRadius, state.boundingRadius);
		setStateValue(_state.masklimen, state.masklimen);
		setStateValue(_state.priority, state.priority);
		setStateValue(_state.isVisibled, state.isVisibled);
		setStateValue(_state.flipX, state.flipX);
		setStateValue(_state.flipY, state.flipY);
		setStateValue(_state.blendfunc, state.blendfunc);
		setStateValue(_state.partsColorFunc, state.partsColorFunc);
		setStateValue(_state.partsColorType, state.partsColorType);
		setStateValue(_state.quad, state.quad);
		setStateValue(_state.rate, state.rate);

		_state.texture = state.texture;
		_state.rect = state.rect;
		memcpy(&_state.mat, &state.mat, sizeof(_state.mat));


		setStateValue(_state.instanceValue_curKeyframe, state.instanceValue_curKeyframe);
		setStateValue(_state.instanceValue_startFrame, state.instanceValue_startFrame);
		setStateValue(_state.instanceValue_endFrame, state.instanceValue_endFrame);
		setStateValue(_state.instanceValue_loopNum, state.instanceValue_loopNum);
		setStateValue(_state.instanceValue_speed, state.instanceValue_speed);
		setStateValue(_state.instanceValue_loopflag, state.instanceValue_loopflag);
		setStateValue(_state.effectValue_curKeyframe, state.effectValue_curKeyframe);
		setStateValue(_state.effectValue_startTime, state.effectValue_startTime);
		setStateValue(_state.effectValue_speed, state.effectValue_speed);
		setStateValue(_state.effectValue_loopflag, state.effectValue_loopflag);

		_state.Calc_rotationX = state.Calc_rotationX;
		_state.Calc_rotationY = state.Calc_rotationY;
		_state.Calc_rotationZ = state.Calc_rotationZ;
		_state.Calc_scaleX = state.Calc_scaleX;
		_state.Calc_scaleY = state.Calc_scaleY;
		_state.Calc_opacity = state.Calc_opacity;

		_state.meshVertexPoint.clear();
		_state.meshVertexPoint = state.meshVertexPoint;
	}


	// override
	virtual void setOpacity(unsigned char opacity);

	// original functions
	SSV3F_C4B_T2F_Quad& getAttributeRef();

	void setFlippedX(bool flip);
	void setFlippedY(bool flip);
	bool isFlippedX();
	bool isFlippedY();
	void sethasPremultipliedAlpha(int PremultipliedAlpha);

public:
};


/**
 * ResourceManager
 */
//正方向の指定
//SSSetPlusDirectionの初期化に使用します。
//プレイヤー内部ではSpriteStudioに準拠し上を正方向として処理します。
//使用するプラットフォームのY座標系が下が正方向の場合にPLUS_DOWNを指定して初期化してください。
enum {
	PLUS_UP,	//上が正方向
	PLUS_DOWN	//下が正方向
};

class ResourceManager
{
public:
	static const std::string s_null;

	/**
	 * デフォルトインスタンスを取得します.
	 *
	 * @return デフォルトのResourceManagerインスタンス
	 */
	static ResourceManager* getInstance();

	/**
	 * ssbpファイルを読み込み管理対象とします.
	 * dataKeyはssbpのファイル名（拡張子なし）になります.
	 *
	 * @param  ssbpFilepath  ssbp ファイルのパス
	 * @param  imageBaseDir  画像ファイルを読み込むルートになるパス。省略時は ssbp ファイルのある場所をルートとします。
	 *                       指定しない場合は引数を "" としてください。
	 * @param  zipFilepath   ZIP ファイルから読み込む場合のパス。省略時は ssbpFilepath で指定された ssbp ファイルを直接読み込みます。
	 *                       ZIP から ssbp ファイルを読み込む場合は、ssbpFilepath には zip ファイル内の ssbp ファイルへのパスを指定してください。
	 *                       例：temp フォルダに test.ssbp があり、temp フォルダをZIPにした temp.ZIP から読みこむ場合
	 *                       zipFilepath は Resourceフォルダから temp.zip までのパス。
	 *                       ssbpFilepath は ZIP 内の ssbp ファイルのパスとなり temp/test.ssbp になり、
	 *                       引数は以下のようになります。
	 *                       resman->addData("temp/test.ssbp", "", "temp.zip");
	 *
	 * @param  imageZipLoad  画像を ZIP ファイルから読み込む場合は true 、ファイルを読む場合は false にします。
	 *                       false を指定し、直接画像を読み込む場合は Resource フォルダに ZIP と同じ構成のフォルダを作成し画像を置くか、
	 *                       または imageBaseDir で画像のあるフォルダを指定してください。
	 * @return dataKey
	 */
	std::string addData(const std::string& ssbpFilepath, const std::string& imageBaseDir = s_null, const std::string& zipFilepath = s_null, bool imageZipLoad = true);

	/**
	 * ssbpファイルを読み込み管理対象とします.
	 *
	 * @param  dataKey       dataKeyの指定
	 * @param  ssbpFilepath  ssbpファイルのパス
	 * @param  imageBaseDir  画像ファイルの読み込み元ルートパス. 省略時はssbpのある場所をルートとします.
	 * @param  zipFilepath   上記 addData のコメントを参照してください。
	 * @param  imageZipLoad  上記 addData のコメントを参照してください。
	 * @return dataKey
	 */
	std::string addDataWithKey(const std::string& dataKey, const std::string& ssbpFilepath, const std::string& imageBaseDir = s_null, const std::string& zipFilepath = s_null, bool imageZipLoad = true);

	/**
	 * 指定されたssbpデータを管理対象とします.
	 *
	 * @param  dataKey       dataKeyの指定
	 * @param  data          ssbpデータ
	 * @param  imageBaseDir  画像ファイルの読み込み元ルートパス. 省略時はssbpのある場所をルートとします.
	 * @param  zipFilepath   上記 addData のコメントを参照してください。
	 * @param  imageZipLoad  上記 addData のコメントを参照してください。
	 * @return dataKey
	 */
	std::string addData(const std::string& dataKey, const ProjectData* data, const std::string& imageBaseDir = s_null, const std::string& zipFilepath = s_null, bool imageZipLoad = true);
	
	/**
	 * 指定データを解放します.
	 * パス、拡張子を除いたssbp名を指定してください。
	 *
	 * @param  dataKey
	 */
	void removeData(const std::string& dataKey);

	/**
	 * 全てのデータを解放します.
	 */
	void removeAllData();

	/**
	* 名前に対応するデータ取得します.
	*/
	ResourceSet* getData(const std::string& dataKey);

	/**
	* ssbpに含まれるアニメーション名を取得します.
	*/
	std::vector<std::string> getAnimeName(const std::string& dataKey);
		
	/**
	* 指定したセルのテクスチャを変更します.
	* SSTextureLoadで差し替える画像を読み込み、取得したインデックスを設定してください。
	*
	* @param  ssbpName       ssbp名（拡張子を除くファイル名）
	* @param  ssceName       ssce名（拡張子を除くファイル名）
	* @param  texture        変更後のテクスチャハンドル
	* @return 変更を行ったか
	*
	*/
	bool changeTexture(char* ssbpName, char* ssceName, long texture);

	/**
	* 指定したデータのテクスチャを破棄します。
	* @param  dataName       ssbp名（拡張子を除くファイル名）
	* @return 成功失敗
	*/
	bool releseTexture(char* ssbpName);

	/**
	* 読み込んでいるssbpからアニメーションの開始フレーム数を取得します。
	* @param  ssbpName       ssbp名（拡張子を除くファイル名）
	* @param  animeName      ssae/モーション名
	* @return アニメーションの開始フレーム（存在しない場合はアサート）
	*/
	int getStartFrame(std::string ssbpName, std::string animeName);

	/**
	* 読み込んでいるssbpからアニメーションの終了フレーム数を取得します。
	* @param  ssbpName       ssbp名（拡張子を除くファイル名）
	* @param  animeName      ssae/モーション名
	* @return アニメーションの終了フレーム（存在しない場合はアサート）
	*/
	int getEndFrame(std::string ssbpName, std::string animeName);

	/**
	* 読み込んでいるssbpからアニメーションの総フレーム数を取得します。
	* @param  ssbpName       ssbp名（拡張子を除くファイル名）
	* @param  animeName      ssae/モーション名
	* @return アニメーションの総フレーム（存在しない場合はアサート）
	*/
	int getTotalFrame(std::string ssbpName, std::string animeName);

	/**
	* 名前が登録されていればtrueを返します
	*
	* @param dataKey
	* @return
	*/
	bool isDataKeyExists(const std::string& dataKey);

	/**
	 * 新たなResourceManagerインスタンスを構築します.
	 *
	 * @return ResourceManagerインスタンス
	 */
	static ResourceManager* create();

public:
	ResourceManager(void);
	virtual ~ResourceManager();

protected:
	std::map<std::string, ResourceSet*>	_dataDic;
};



/**
 * UserData
 */
struct UserData
{
	enum {
		FLAG_INTEGER	= 1 << 0,
		FLAG_RECT		= 1 << 1,
		FLAG_POINT		= 1 << 2,
		FLAG_STRING		= 1 << 3
	};

	const char*	partName;		/// Part name
	int			frameNo;		/// Frame no

	int			flags;			/// Flags of valid data
	int			integer;		/// Integer
	int			rect[4];		/// Rectangle Left, Top, Right, Bottom
	int			point[2];		/// Position X, Y
	const char*	str;			/// String (zero terminated)
	int			strSize;		/// String size (byte count)
};


/**
* LabelData
*/
struct LabelData
{
	std::string	str;			/// String (zero terminated)
	int			strSize;		/// String size (byte count)
	int			frameNo;		/// Frame no
};

//インスタンスデータ
struct Instance
{
	int			refStartframe;		//開始フレーム
	int			refEndframe;		//終了フレーム
	float		refSpeed;			//再生速度
	int			refloopNum;			//ループ回数
	bool		infinity;			//無限ループ
	bool		reverse;			//逆再選
	bool		pingpong;			//往復
	bool		independent;		//独立動作
	void clear(void)
	{
		refStartframe = 0;			//開始フレーム
		refEndframe = 1;			//終了フレーム
		refSpeed = 1;				//再生速度
		refloopNum = 1;				//ループ回数
		infinity = false;			//無限ループ
		reverse = false;			//逆再選
		pingpong = false;			//往復
		independent = false;		//独立動作
	}
};


/**
* ResluteState
* ゲーム側に返すパーツステータス。
* 必要に応じてカスタマイズしてください。
*/
struct ResluteState
{
	int flags;						/// このフレームで更新が行われるステータスのフラグ
	int cellIndex;					/// パーツに割り当てられたセルの番号
	float x;						/// SSアトリビュート：X座標
	float y;						/// SSアトリビュート：Y座標
	float z;						/// SSアトリビュート：Z座標
	float pivotX;					/// 原点Xオフセット＋セルに設定された原点オフセットX
	float pivotY;					/// 原点Yオフセット＋セルに設定された原点オフセットY
	float rotationX;				/// X回転（親子関係計算済）
	float rotationY;				/// Y回転（親子関係計算済）
	float rotationZ;				/// Z回転（親子関係計算済）
	float scaleX;					/// Xスケール（親子関係計算済）
	float scaleY;					/// Yスケール（親子関係計算済）
	float localscaleX;				/// Xローカルスケール
	float localscaleY;				/// Yローカルスケール
	int opacity;					/// 不透明度（0～255）（親子関係計算済）
	int localopacity;				/// ローカル不透明度（0～255）
	float size_X;					/// SS6アトリビュート：Xサイズ
	float size_Y;					/// SS6アトリビュート：Xサイズ
	float uv_move_X;				/// SS6アトリビュート：UV X移動
	float uv_move_Y;				/// SS6アトリビュート：UV Y移動
	float uv_rotation;				/// SS6アトリビュート：UV 回転
	float uv_scale_X;				/// SS6アトリビュート：UV Xスケール
	float uv_scale_Y;				/// SS6アトリビュート：UV Yスケール
	float boundingRadius;			/// SS6アトリビュート：当たり半径
	int	priority;					/// SS6アトリビュート：優先度
	int partsColorFunc;				/// SS6アトリビュート：カラーブレンドのブレンド方法
	int partsColorType;				/// SS6アトリビュート：カラーブレンドの単色か頂点カラーか。
	bool flipX;						/// 横反転（親子関係計算済）
	bool flipY;						/// 縦反転（親子関係計算済）
	bool isVisibled;				/// 非表示（親子関係計算済）

	int	part_type;					/// パーツ種別
	int	part_boundsType;			/// 当たり判定種類
	int	part_alphaBlendType;		/// BlendType
	int	part_labelcolor;			/// ラベルカラー
};

/**
* 再生するフレームに含まれるパーツデータのフラグ
*/
enum {
	PART_FLAG_INVISIBLE			= 1 << 0,		/// 非表示
	PART_FLAG_FLIP_H			= 1 << 1,		/// 横反転
	PART_FLAG_FLIP_V			= 1 << 2,		/// 縦反転

	// optional parameter flags
	PART_FLAG_CELL_INDEX		= 1 << 3,		/// セル番号
	PART_FLAG_POSITION_X		= 1 << 4,		/// X座標
	PART_FLAG_POSITION_Y		= 1 << 5,		/// Y座標
	PART_FLAG_POSITION_Z		= 1 << 6,		/// Z座標
	PART_FLAG_PIVOT_X			= 1 << 7,		/// 原点オフセットX
	PART_FLAG_PIVOT_Y           = 1 << 8,		/// 原点オフセットY
	PART_FLAG_ROTATIONX			= 1 << 9,		/// X回転
	PART_FLAG_ROTATIONY			= 1 << 10,		/// Y回転
	PART_FLAG_ROTATIONZ			= 1 << 11,		/// Z回転
	PART_FLAG_SCALE_X			= 1 << 12,		/// スケールX
	PART_FLAG_SCALE_Y			= 1 << 13,		/// スケールY
	PART_FLAG_LOCALSCALE_X		= 1 << 14,		/// ローカルスケールX
	PART_FLAG_LOCALSCALE_Y		= 1 << 15,		/// ローカルスケールY
	PART_FLAG_OPACITY			= 1 << 16,		/// 不透明度
	PART_FLAG_LOCALOPACITY		= 1 << 17,		/// ローカル不透明度
	PART_FLAG_PARTS_COLOR		= 1 << 18,		/// パーツカラー
	PART_FLAG_VERTEX_TRANSFORM	= 1 << 19,		/// 頂点変形

	PART_FLAG_SIZE_X			= 1 << 20,		/// サイズX
	PART_FLAG_SIZE_Y			= 1 << 21,		/// サイズY

	PART_FLAG_U_MOVE			= 1 << 22,		/// UV移動X
	PART_FLAG_V_MOVE			= 1 << 23,		/// UV移動Y
	PART_FLAG_UV_ROTATION		= 1 << 24,		/// UV回転
	PART_FLAG_U_SCALE			= 1 << 25,		/// UVスケールX
	PART_FLAG_V_SCALE			= 1 << 26,		/// UVスケールY
	PART_FLAG_BOUNDINGRADIUS	= 1 << 27,		/// 当たり半径

	PART_FLAG_MASK				= 1 << 28,		/// マスク強度
	PART_FLAG_PRIORITY			= 1 << 29,		/// 優先度

	PART_FLAG_INSTANCE_KEYFRAME	= 1 << 30,		/// インスタンス
	PART_FLAG_EFFECT_KEYFRAME   = 1 << 31,		/// エフェクト

	NUM_PART_FLAGS
};

enum {
	PART_FLAG_MESHDATA = 1 << 0,		/// メッシュデータ

	NUM_PART_FLAGS2
};

/**
* 頂点変形フラグ
*/
enum VertexFlag 
{
	VERTEX_FLAG_LT = 1 << 0,
	VERTEX_FLAG_RT = 1 << 1,
	VERTEX_FLAG_LB = 1 << 2,
	VERTEX_FLAG_RB = 1 << 3,
	VERTEX_FLAG_ONE = 1 << 4	// color blend only
};

/**
* インスタンスループ設定フラグ
*/
enum InstanceLoopFlag  
{
	INSTANCE_LOOP_FLAG_INFINITY = 1 << 0,		//
	INSTANCE_LOOP_FLAG_REVERSE = 1 << 1,
	INSTANCE_LOOP_FLAG_PINGPONG = 1 << 2,
	INSTANCE_LOOP_FLAG_INDEPENDENT = 1 << 3,
};

//エフェクトアトリビュートのループフラグ
enum EffectLoopFlag  
{
	EFFECT_LOOP_FLAG_INDEPENDENT = 1 << 0,
};

/**
* Animation Part Type
*/
enum PartsType
{
	PARTTYPE_INVALID = -1,
	PARTTYPE_NULL,			/// null。領域を持たずSRT情報のみ。ただし円形の当たり判定は設定可能。
	PARTTYPE_NORMAL,		/// 通常パーツ。領域を持つ。画像は無くてもいい。
	PARTTYPE_TEXT,			/// テキスト(予約　未実装）
	PARTTYPE_INSTANCE,		/// インスタンス。他アニメ、パーツへの参照。シーン編集モードの代替になるもの
	PARTTYPE_ARMATURE,		///< ボーンパーツ
	PARTTYPE_EFFECT,		// ss5.5対応エフェクトパーツ
	PARTTYPE_MESH,			///< メッシュパーツ
	PARTTYPE_MOVENODE,		///< 動作起点
	PARTTYPE_CONSTRAINT,		///<コンストレイント
	PARTTYPE_MASK,			///< マスク
	PARTTYPE_JOINT,			///< メッシュとボーンの関連付けパーツ
	PARTTYPE_BONEPOINT,		///< ボーンポイント
	PARTTYPE_NUM
};

/*
* 当たり判定の種類
*/
enum CollisionType
{
	INVALID = -1,
	NONE,			///< 当たり判定として使わない。
	QUAD,			///< 自在に変形する四辺形。頂点変形など適用後の４角を結んだ領域。最も重い。
	AABB,			///< 回転しない全体を囲む矩形で交差判定
	CIRCLE,			///< 真円の半径で距離により判定する
	CIRCLE_SMIN,	///< 真円の半径で距離により判定する (スケールはx,yの最小値をとる）
	CIRCLE_SMAX,	///< 真円の半径で距離により判定する (スケールはx,yの最大値をとる）
	num
};

/**
* αブレンド方法
*/
enum BlendType
{
	BLEND_MIX,		///< 0 ブレンド（ミックス）
	BLEND_MUL,		///< 1 乗算
	BLEND_ADD,		///< 2 加算
	BLEND_SUB,		///< 3 減算
	BLEND_MULALPHA, ///< 4 α乗算
	BLEND_SCREEN, 	///< 5 スクリーン
	BLEND_EXCLUSION,///< 6 除外
	BLEND_INVERT, 	///< 7 反転
	BLEND_MASK, 	///< マスク　プレイヤーではマスクを描画モード扱いにしておく
	BLEND_NUM,
};

/*
Common\Loader\sstypes.hに実際の定義があります。
/// テクスチャラップモード
namespace SsTexWrapMode
{
	enum _enum
	{
		invalid = -1,	/// なし
		clamp,			/// クランプする
		repeat,			/// リピート
		mirror,			/// ミラー
		num
	};
};

/// テクスチャフィルターモード 画素補間方法
namespace SsTexFilterMode
{
	enum _enum
	{
		invalid = -1,
		nearlest,	///< ニアレストネイバー
		linear,		///< リニア、バイリニア
		num
	};
};
*/

//カラーラベル定数
#define COLORLABELSTR_NONE		""
#define COLORLABELSTR_RED		"Red"
#define COLORLABELSTR_ORANGE	"Orange"
#define COLORLABELSTR_YELLOW	"Yellow"
#define COLORLABELSTR_GREEN		"Green"
#define COLORLABELSTR_BLUE		"Blue"
#define COLORLABELSTR_VIOLET	"Violet"
#define COLORLABELSTR_GRAY		"Gray"
enum
{
	COLORLABEL_NONE,		///< 0 なし
	COLORLABEL_RED,			///< 1 赤
	COLORLABEL_ORANGE,		///< 2 オレンジ
	COLORLABEL_YELLOW,		///< 3 黄色
	COLORLABEL_GREEN,		///< 4 緑
	COLORLABEL_BLUE,		///< 5 青
	COLORLABEL_VIOLET,		///< 6 紫
	COLORLABEL_GRAY,		///< 7 灰色
};

//------------------------------------------------------------------------------
//プレイヤーの設定定義
//使用するアニメーションに合わせて設定してください。


//プレイヤーで扱えるアニメに含まれるパーツの最大数
#define PART_VISIBLE_MAX (512)

//このサンプルでは3D機能を使用して描画します。
//それぞれのプラットフォームに合わせた座標系で使用してください。
//座標系を反転させる場合はsetPositionで画面サイズから引いた座標を設定して運用するといいと思います。

//------------------------------------------------------------------------------


/**
 * Player
 */
class Player
{
public:
	/**
	 * Playerインスタンスを構築します.
	 *
	 * @param  resman  使用するResourceManagerインスタンス. 省略時はデフォルトインスタンスが使用されます.
	 * @return Playerインスタンス
	 */
	static Player* create(ResourceManager* resman = NULL);

	/**
	 * 使用するResourceManagerインスタンスを設定します.
	 *
	 * @param  resman  使用するResourceManagerインスタンス. 省略時はデフォルトインスタンスが使用されます.
	 */
	void setResourceManager(ResourceManager* resman = NULL);

	/**
	 * 使用中のResourceManagerインスタンスを解放します.
	 * 再度ResourceManagerインスタンスを設定するまでは再生できなくなります.
	 */
	void releaseResourceManager();

	/**
	 * 再生するssbpデータのdataKeyを設定します.
	 *
	 * @param  dataKey  再生するデータのdataKey
	 */
	void setData(const std::string& dataKey);

	/**
	* 再生しているssbpデータのdataKeyを取得します.
	*
	* @return 再生しているssbp名
	*/
	std::string getPlayDataName(void);

	/**
	 * 設定されているssbpデータを解放します.
	 */
	void releaseData();

	/**
	 * 設定されているアニメーションを解放します.
	 */
	void releaseAnime();

	/**
	* アニメーションの再生を開始します.
	*
	* @param  ssaeName      パック名(ssae名）
	* @param  motionName    再生するモーション名
	* @param  loop          再生ループ数の指定. 省略時は0
	* @param  startFrameNo  再生を開始するフレームNoの指定. 省略時は0
	*/
	void play(const std::string& ssaeName, const std::string& motionName, int loop = 0, int startFrameNo = 0);

	/**
	* アニメーションの再生を開始します.
	* アニメーション名から再生するデータを選択します.
	* "ssae名/モーション名で指定してください.
	* sample.ssaeのanime_1を指定する場合、sample/anime_1となります.
	* ※ver1.1からモーション名のみで指定する事はできなくなりました。
	*
	* @param  animeName     再生するアニメーション名
	* @param  loop          再生ループ数の指定. 省略時は0
	* @param  startFrameNo  再生を開始するフレームNoの指定. 省略時は0
	*/
	void play(const std::string& animeName, int loop = 0, int startFrameNo = 0);

	/**
	* 現在再生しているモーションとブレンドしながら再生します。
	* アニメーション名から再生するデータを選択します.
	* "ssae名/モーション名で指定してください.
	* sample.ssaeのanime_1を指定する場合、sample/anime_1となります.
	* ※ver1.1からモーション名のみで指定する事はできなくなりました。
	*
	* ブレンドするアニメーションの条件は以下になります。
	* ・同じssbp内に含まれている事
	* ・同じパーツ構成（パーツ順、パーツ数）である事
	* SpriteStudioのフレームコントロールに並ぶパーツを上から順にブレンドしていきます。
	* パーツ名等のチェックは行なっていませんので遷移元と遷移先アニメのパーツの順番を同じにする必要があります。
	* 遷移元と遷移先のパーツ構成があっていない場合、正しくブレンドされませんのでご注意ください。
	*
	* 合成されるアトリビュートは
	* 座標X、座標Y、X回転、Y回転、Z回転、スケールX、スケールYのみです。
	* それ以外のアトリビュートは遷移先アニメの値が適用されます。
	* インスタンスパーツが参照しているソースアニメはブレンドされません。
	* エフェクトパーツから発生したパーティクルはブレンドされません。
	* 
	*
	* @param  animeName     再生するアニメーション名
	* @param  loop          再生ループ数の指定. 省略時は0
	* @param  startFrameNo  再生を開始するフレームNoの指定. 省略時は0
	* @param  blendTime		モーションブレンドを行う時間、単位は秒　省略時は1秒
	*/
	void motionBlendPlay(const std::string& animeName, int loop = 0, int startFrameNo = 0, float blendTime = 0.1f);

	/**
	 * 再生を中断します.
	 */
	void animePause();

	/**
	 * 再生を再開します.
	 */
	void animeResume();

	/**
	 * 再生を停止します.
	 * ゲーム側でアニメーションの表示フレームを制御する場合はstop()を呼び出した後
	 * ゲーム側の更新処理でsetFrameNo()を呼び出し指定のフレームを表示してください。
	 */
	void stop();

	/**
	 * 再生しているアニメーションのパック名(ssae)を返します.
	 *
	 * @return パック名(ssae)
	 */
	const std::string& getPlayPackName() const;

	/**
	 * 再生しているアニメーション名を返します.
	 *
	 * @return アニメーション名
	 */
	const std::string& getPlayAnimeName() const;
	
	/**
	* アニメーションの開始フレームを取得します.
	*
	* @return 開始フレーム
	*/
	int getStartFrame() const;

	/**
	* アニメーションの終了フレームを取得します.
	*
	* @return 終了フレーム
	*/
	int getEndFrame() const;

	/**
	* アニメーションの総フレームを取得します.
	*
	* @return 総フレーム
	*/
	int getTotalFrame() const;

	/**
	* FPSを取得します.
	*
	* @return アニメーションに設定されたFPS
	*/
	int getFPS() const;

	/**
	 * 再生フレームNoを取得します.
	 * Get frame no of playing.
	 *
	 * @return 再生フレームNo. frame no.
	 */
	int getFrameNo() const;

	/**
	 * 再生フレームNoを設定します.
	 * Set frame no of playing.
	 *
	 * @param 再生フレームNo. frame no.
	 */
	void setFrameNo(int frameNo);

	/**
	 * 再生スピードを取得します. (1.0f:標準)
	 * Set speed to play. (1.0f:normal speed)
	 */
	float getStep() const;

	/**
	 * 再生スピードを設定します. (1.0f:標準)
	 * Get speed to play. (1.0f:normal speed)
	 */
	void setStep(float step);
	
	/** 
	 * 指定されている再生ループ回数を取得します. (0:指定なし)
	 * Get a playback loop count of specified. (0:not specified)
	 */
	int getLoop() const;

	/** 
	 * 再生ループ回数を設定します. (0:指定なし)
	 * Set a playback loop count.  (0:not specified)
	 */
	void setLoop(int loop);

	/** 
	 * 現在までのループ再生回数を取得します.
	 * Get repeat count a playback.
	 */
	int getLoopCount() const;

	/** 
	 * 現在までのループ再生回数をクリアします.
	 * Clear repeat count a playback.
	 */
	void clearLoopCount();

	/**
	 * フレームスキップ（フレームレートに合わせ再生フレームをスキップする）の設定をします. (default: true)
	 * Set frame-skip(to skip the playback frame according to the frame rate). (default: true)
	 */
	void setFrameSkipEnabled(bool enabled);
	
	/** 
	 * フレームスキップの設定状態を返します.
	 * Get frame-skip setting.
	 */
	bool isFrameSkipEnabled() const;

	/**
	* ラベル名からフレーム位置を取得します.
	*/
	int getLabelToFrame(char* findLabelName);

	/**
	* 再生しているアニメーションに含まれるパーツ数を取得します.
	*/
	int getPartsCount(void);

	/**
	* indexからパーツ名を取得します.
	*
	* @param  result        パーツ情報を受け取るバッファ
	* @param  name          取得するパーツ名
	* @param  frameNo       取得するフレーム番号 -1の場合は現在再生しているフレームが適用される
	*/
	const char* getPartName(int partId) const;

	/**
	* パーツ名からindexを取得します.
	*/
	int indexOfPart(const char* partName) const;

	/**
	* パーツの名から、パーツ情報を取得します.
	*
	* @param  result        パーツ情報を受け取るバッファ
	* @param  name          取得するパーツ名
	* @param  frameNo       取得するフレーム番号 -1の場合は現在再生しているフレームが適用される
	*/
	bool getPartState(ResluteState& result, const char* name, int frameNo = -1);

	/**
	* パーツ名からパーツの表示、非表示を設定します.
	* コリジョン用のパーツや差し替えグラフィック等、SS上で表示を行うがゲーム中では非表示にする場合に使用します。
	* SSの非表示アトリビュート設定するわけではないので注意してください。
	*/
	void setPartVisible(std::string partsname, bool flg);

	/**
	* パーツ名からパーツに割り当たるセルを変更します.
	* この関数で設定したパーツは参照セルアトリビュートの影響をうけません。
	* アニメに設定されたセルに戻す場合は、セル名に""を指定してください。
	*
	* @param  partsname         パーツ名
	* @param  sscename          セルマップ名
	* @param  cellname          表示させたいセル名
	*/
	void setPartCell(std::string partsname, std::string sscename, std::string cellname);

	/*
	* プレイヤー本体の位置を設定します。
	*/
	void  setPosition(float x, float y);

	/*
	* プレイヤー本体の回転角度を設定します。2Dの回転はZに値を設定してください。
	*/

	void  setRotation(float x, float y, float z);
	/*
	* プレイヤー本体のスケールを設定します。
	*/
	void  setScale(float x, float y);

	/*
	* プレイヤー本体の透明度を設定します。
	*/
	void  setAlpha(int a);

	/*
	* アニメの輝度を設定します.
	* setColor(Color3B)ではなくこちらを使用してください。
	* 制限としてカラーブレンドが適用されたパーツの色は変更できませんので注意してください。
	*
	* @param  r          赤成分(0～255)
	* @param  g          緑成分(0～255)
	* @param  b          青成分(0～255)
	*/
	void setColor(int r, int g, int b);

	/*
	* 名前を指定してパーツの再生するインスタンスアニメを変更します。
	* 指定したパーツがインスタンスパーツでない場合、falseを返します.
	* インスタンスパーツ名はディフォルトでは「ssae名:モーション名」とつけられています。
	* 再生するアニメの名前は"ssae名/アニメーション名"として再生してください。
	* 現在再生しているアニメを指定することは入れ子となり無限ループとなるためできません。
	* 変更するアニメーションは同じssbpに含まれる必要があります。
	*
	* インスタンスキーを手動で設定する事が出来ます。
	* アニメーションに合わせて開始フレーム、終了フレーム等のインスタンスアトリビュート情報を設定してください。
	* 終了フレーム最大値は総フレーム-1になります。
	* 上書きフラグがfalseの場合、SS上に設定されたインスタンスアトリビュートの設定を使用します。
	* 使用例：
	* ss::Instance param;
	* param.clear();
	* param.refEndframe = resman->getMaxFrame("ssbp名","ssae名/モーション名") - 1;	//アニメーションの長さを取得
	* param.infinity = true;														//無限ループを設定
	* ssplayer->changeInstanceAnime("再生しているアニメーションに含まれるインスタンスパーツ名", "ssae名/モーション名", true, param);
	*
	* @param  partsname			SS上のパーツ名
	* @param  animeName			参照するアニメ名
	* @param  overWrite			インスタンスキーの上書きフラグ
	* @param  keyParam			インスタンスキーのパラメータ
	*/
	bool changeInstanceAnime(std::string partsname, std::string animeName, bool overWrite, Instance keyParam);

	/*
	* プレイヤーにインスタンスパラメータを設定します。
	*
	* @param  overWrite			インスタンスキーの上書きフラグ
	* @param  keyParam			インスタンスキーのパラメータ
	*/
	void setInstanceParam(bool overWrite, Instance keyParam);

	/*
	* プレイヤーからインスタンスパラメータを取得します。
	*
	* @param  overWrite			インスタンスキーの上書きフラグ
	* @param  keyParam			インスタンスキーのパラメータ
	*/
	void getInstanceParam(bool *overWrite, Instance *keyParam);

	/*
	* アニメーションのループ範囲（再生位置）を上書きします。
	*
	* @param  frame			開始フレーム（-1で上書き解除）
	*/
	void setStartFrame(int frame);

	/*
	* アニメーションのループ範囲（終了位置）を上書きします。
	* SpriteStudioのフレーム数+1を設定してください。
	*
	* @param  frame			終了フレーム（-1で上書き解除）
	*/
	void setEndFrame(int frame);

	/*
	* アニメーションのループ範囲（再生位置）を上書きします。
	*
	* @param  labelname			開始フレームとなるラベル名（""で上書き解除）
	*/
	void setStartFrameToLabelName(char *findLabelName);

	/*
	* アニメーションのループ範囲（終了位置）を上書きします。
	*
	* @param  labelname			終了フレームとなるラベル名（""で上書き解除）
	*/
	void setEndFrameToLabelName(char *findLabelName);

	/*
	* プレイヤー本体の反転を設定します。
	*/
	void setFlip(bool flipX, bool flipY);

	/*
	* プレイヤーのマスク機能の有効、無効を設定します。
	* SpriteStudioのマスクはステンシルバッファを使用して実現しています。
	* そのためパーツの描画でステンシルマスクを設定しており、プレイヤー以外でステンシルマスクを使用する際に
	* ステンシルマスクの設定が上書きされプレイヤーの描画が意図しない結果になる場合があります。
	* 無効（false）にする事でプレイヤーのマスクに関する処理を行いようにし、ほかのマスク処理に影響されるようになります。
	* 無効場合はアニメーションに含まれるマスクパーツは作用しませんのでご注意ください。
	*/
	void setMaskFunctionUse(bool flg);

	/*
	* パーツ番号に対応したスプライト情報を取得します。
	* 
	* @param  partIndex			パーツ番号
	*/
	CustomSprite* getSpriteData(int partIndex);

	/*
	* 表示を行うパーツ数を取得します
	*/
	int getDrawSpriteCount(void);

	/*
	* rootパーツの状態を決めるマトリクスを設定します。
	*
	* @param  mat			与えるマトリクス
	* @param  use			マトリクスを適用するか？
	*
	*/
	void setParentMatrix(float* mat, bool use);

	typedef std::function<void(Player*, const UserData*)> UserDataCallback;
	typedef std::function<void(Player*)> PlayEndCallback;
	/**
	* ユーザーデータを受け取るコールバックを設定します.
	* 再生したフレームにユーザーデータが設定されている場合呼び出されます。
	* プレイヤーを判定する場合、ゲーム側で管理しているss::Playerのアドレスと比較して判定してください。
	*
	* コールバック内でパーツのステータスを取得したい場合は、この時点ではアニメが更新されていないため、
	* getPartStateに data->frameNo でフレーム数を指定して取得してください。
	* //再生しているモーションに含まれるパーツ名「collision」のステータスを取得します。
	* ss::ResluteState result;
	* ssplayer->getPartState(result, "collision", data->frameNo);
	*
	* コールバック内でアニメーションの再生フレーム変更したい場合は
	* 次に行われるゲームのアップデート内でプレイヤーに対してアニメーションの操作をしてください。
	*
	* @param  callback  ユーザーデータ受け取りコールバック
	*
	* @code
	* player->setUserDataCallback(CC_CALLBACK_2(MyScene::userDataCallback, this));
	* --
	* void MyScene::userDataCallback(ss::Player* player, const ss::UserData* data)
	* {
	*   ...
	* }
	* @endcode
	*/
	void setUserDataCallback(const UserDataCallback& callback);

	/**
	* 再生終了時に呼び出されるコールバックを設定します.
	* 再生したアニメーションが終了した段階で呼び出されます。
	* プレイヤーを判定する場合、ゲーム側で管理しているss::Playerのアドレスと比較して判定してください。
	* player->getPlayAnimeName();
	* を使用する事で再生しているアニメーション名を取得する事もできます。
	*
	* ループ回数分再生した後に呼び出される点に注意してください。
	* 無限ループで再生している場合はコールバックが発生しません。
	*
	* コールバック内でアニメーションの再生フレーム変更したい場合は
	* 次に行われるゲームのアップデート内でプレイヤーに対してアニメーションの操作をしてください。
	*
	* @param  callback  再生終了受け取りコールバック
	*
	* @code
	* player->setPlayEndCallback(CC_CALLBACK_1(MyScene::playEndCallback, this));
	* --
	* void MyScene::playEndCallback(ss::Player* player)
	* {
	*   ...
	* }
	* @endcode
	*/
	void setPlayEndCallback(const PlayEndCallback& callback);


public:
	Player(void);
	~Player();
	bool init();
	void update(float dt);
	void draw();

	State getState(void);
	bool getMaskFunctionUse(void) { return _maskEnable; };

	SSPlayerControl*	_playercontrol;

#ifdef USE_AGTK//sakihama-h, 2018.05.10
	AnimeRef *getCurrentAnimeRef() { return _currentAnimeRef; };
	AnimeRef *getAnimeRef(const std::string& animeName);
	int getFrameMax();
	int getFrameMax(const std::string& animeName);
#endif

protected:
	void allocParts(int numParts, bool useCustomShaderProgram);
	void releaseParts();
	void setPartsParentage();

	void play(AnimeRef* animeRef, int loop, int startFrameNo);
	void updateFrame(float dt);
	void setFrame(int frameNo, float dt = 0.0f);
	void checkUserData(int frameNo);
	float parcentVal(float val1, float val2, float parcent);
	float parcentValRot(float val1, float val2, float parcent);
	void setMaskFuncFlag(bool flg);
	void setMaskParentSetting(bool flg);

protected:
	ResourceManager*	_resman;
	ResourceSet*		_currentRs;
	std::string			_currentdataKey;
	std::string			_currentAnimename;
	AnimeRef*			_currentAnimeRef;
	std::vector<CustomSprite *>	_parts;

	Player*				_motionBlendPlayer;
	float				_blendTime;
	float				_blendTimeMax;

	bool				_frameSkipEnabled;
	float				_playingFrame;
	float				_step;
	int					_loop;
	int					_loopCount;
	bool				_isPlaying;
	bool				_isPausing;
	bool				_isPlayFirstUserdataChack;
	int					_prevDrawFrameNo;
	bool				_partVisible[PART_VISIBLE_MAX];
	int					_cellChange[PART_VISIBLE_MAX];
	int					_partIndex[PART_VISIBLE_MAX];
	int					_animefps;
	int					_col_r;
	int					_col_g;
	int					_col_b;
	bool				_instanceOverWrite;				//インスタンス情報を上書きするか？
	Instance			_instanseParam;					//インスタンスパラメータ
	int					_startFrameOverWrite;			//開始フレームの上書き設定
	int					_endFrameOverWrite;				//終了フレームの上書き設定
	int					_seedOffset;					//エフェクトシードオフセット
	int					_draw_count;					//表示スプライト数

	UserData			_userData;

	State				_state;

	float				_parentMat[16];					//プレイヤーが持つ継承されたマトリクス
	bool				_parentMatUse;					//プレイヤーが持つ継承されたマトリクスがあるか？
	bool				_maskParentSetting;				//親パーツのマスク対象（インスタンスのみ使用する）
	bool				_maskFuncFlag;					//マスク機能を有効にするか？（インスタンスのソースアニメはマスクが無効になる）
	bool				_maskEnable;					//マスク機能を無効にするか？

	std::vector<CustomSprite *> _maskIndexList;			//マスク対象となるパーツ

	int _direction;										//プレイヤーの座標系設定
	int _window_w;
	int _window_h;

	UserDataCallback	_userDataCallback;
	PlayEndCallback		_playEndCallback;
};


};	// namespace ss

#endif
