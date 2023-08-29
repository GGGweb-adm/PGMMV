#ifndef __MACROS_H__
#define __MACROS_H__

#include "cocos2d.h"

#ifdef _WINDOWS
//Windowsでソースに埋め込んだ文字列を実行時にUTF-8と扱うよう指定
#pragma execution_character_set("utf-8")
#endif

#ifdef __cplusplus
	#define NS_AGTK_BEGIN                     namespace agtk {
	#define NS_AGTK_END                       }
	#define USING_NS_AGTK                     using namespace agtk
	#define NS_AGTK                           ::agtk
#else
	#define NS_AGTK_BEGIN 
	#define NS_AGTK_END 
	#define USING_NS_AGTK 
	#define NS_AGTK
#endif 

#ifdef __cplusplus
	#define NS_DATA_BEGIN                       namespace data {
	#define NS_DATA_END                         }
	#define USING_NS_DATA                       using namespace data
	#define NS_DATA                             ::data
#else
	#define NS_DATA_BEGIN 
	#define NS_DATA_END 
	#define USING_NS_DATA 
	#define NS_DATA
#endif 

#ifdef __cplusplus
	#define NS_AGTK_DATA_BEGIN                namespace agtk { namespace data {
	#define NS_AGTK_DATA_END                  }}
	#define USING_AGTK_DATA                   using namespace agtk::data
	#define NS_AGTK_DATA                      ::agtk::data
#else
	#define NS_AGTK_DATA_BEGIN 
	#define NS_AGTK_DATA_END 
	#define USING_AGTK_DATA 
	#define NS_AGTK_DATA
#endif 

#define AGTK_STRING(str)	#str
#define AGTK_LINEAR_INTERPOLATE(v1, v2, range, pos)		((v1) != (v2) ? (((v1) * ((range) - (pos)) + (v2) * (pos)) / (range)) : (v2))
#define AGTK_LINEAR_INTERPOLATE2(v1, v2, range, pos)	((v1) != (v2) ? (((v2) * ((range) - (pos)) + (v1) * (pos)) / (range)) : (v2))
#define AGTK_PARABOLA_INTERPOLATE(v1, v2, range, pos)	((v1) != (v2) ? ((v1) + ((v2) - (v1)) * ((pos) * (pos)) / ((range) * (range))) : (v2))
#define AGTK_PARABOLA_INTERPOLATE2(v1, v2, range, pos)	((v1) != (v2) ? ((v2) + ((v1) - (v2)) * (((range) - (pos)) * ((range) - (pos))) / ((range) * (range))) : (v2))

#define AGTK_RANDOM(min, max) ((min) + ((max) - (min)) * ((double)rand() / (1.0 + RAND_MAX)))

// #AGTK-NX
#if CC_TARGET_PLATFORM == CC_PLATFORM_NX
#else
#if defined(AGTK_RUNTIME)
#define USE_RUNTIME	//2018.02.14 agusa-k: スタンドアローン動作用
#elif defined(AGTK_PREVIEW) || !defined(AGTK_RUNTIME)
#define USE_PREVIEW	//2017.06.07 agusa-k: プレビュー機能用
#endif
#endif
#ifdef USE_PREVIEW
#define PreviewError(format, ...)      cocos2d::log(format, ##__VA_ARGS__)
#endif
//#define USE_COLLISION_MEASURE
#define USE_SCRIPT_PRECOMPILE	//動作確認が取れたらifdefを除去してすっきりさせる。
#define USE_MULTITHREAD_OBJECT_UPDATE	// Objectの壁判定更新をマルチスレッドで処理する
//#define USE_MULTITHREAD_MEASURE	//マルチスレッド処理の処理時間を計測
#define USE_MULTITHREAD_UPDOWN_THREADS	// マルチスレッド処理で使用するスレッド数を増減できるようにする
#define USE_DESIGNATED_RESOLUTION_RENDER
#define FIX_ACT2_4774
#define FIX_ACT2_4471
#define FIX_ACT2_4879
#define FIX_ACT2_5233
#define FIX_ACT2_5237

#define CC_SYNTHESIZE_INSTANCE(varType, varName, funName)    \
protected: varType varName; \
public: virtual varType get##funName(void) { return varName; } \
public: virtual void set##funName(varType var)   \
{ \
    if (varName != var) \
    { \
        CC_SAFE_DELETE(varName); \
        varName = var; \
    } \
} 

// 3つの引数を持つcreateマクロ
#define CREATE_FUNC_PARAM3(__TYPE__,__PARAMTYPE__,__PARAM__,__PARAMTYPE2__,__PARAM2__,__PARAMTYPE3__,__PARAM3__) \
static __TYPE__* create(__PARAMTYPE__ __PARAM__, __PARAMTYPE2__ __PARAM2__, __PARAMTYPE3__ __PARAM3__) \
{ \
    __TYPE__ *pRet = new(std::nothrow) __TYPE__(); \
    if (pRet && pRet->init(__PARAM__, __PARAM2__, __PARAM3__)) \
    { \
        pRet->autorelease(); \
        return pRet; \
    } \
    else \
    { \
        delete pRet; \
        pRet = nullptr; \
        return nullptr; \
    } \
}
// 4つの引数を持つcreateマクロ
#define CREATE_FUNC_PARAM4(__TYPE__,__PARAMTYPE__,__PARAM__,__PARAMTYPE2__,__PARAM2__,__PARAMTYPE3__,__PARAM3__,__PARAMTYPE4__,__PARAM4__) \
static __TYPE__* create(__PARAMTYPE__ __PARAM__, __PARAMTYPE2__ __PARAM2__, __PARAMTYPE3__ __PARAM3__, __PARAMTYPE4__ __PARAM4__) \
{ \
    __TYPE__ *pRet = new(std::nothrow) __TYPE__(); \
    if (pRet && pRet->init(__PARAM__, __PARAM2__, __PARAM3__, __PARAM4__)) \
    { \
        pRet->autorelease(); \
        return pRet; \
    } \
    else \
    { \
        delete pRet; \
        pRet = nullptr; \
        return nullptr; \
    } \
}
// 5つの引数を持つcreateマクロ
#define CREATE_FUNC_PARAM5(__TYPE__,__PARAMTYPE__,__PARAM__,__PARAMTYPE2__,__PARAM2__,__PARAMTYPE3__,__PARAM3__,__PARAMTYPE4__,__PARAM4__,__PARAMTYPE5__,__PARAM5__) \
static __TYPE__* create(__PARAMTYPE__ __PARAM__, __PARAMTYPE2__ __PARAM2__, __PARAMTYPE3__ __PARAM3__, __PARAMTYPE4__ __PARAM4__, __PARAMTYPE5__ __PARAM5__) \
{ \
    __TYPE__ *pRet = new(std::nothrow) __TYPE__(); \
    if (pRet && pRet->init(__PARAM__, __PARAM2__, __PARAM3__, __PARAM4__, __PARAM5__)) \
    { \
        pRet->autorelease(); \
        return pRet; \
    } \
    else \
    { \
        delete pRet; \
        pRet = nullptr; \
        return nullptr; \
    } \
}
// 6つの引数を持つcreateマクロ
#define CREATE_FUNC_PARAM6(__TYPE__,__PARAMTYPE__,__PARAM__,__PARAMTYPE2__,__PARAM2__,__PARAMTYPE3__,__PARAM3__,__PARAMTYPE4__,__PARAM4__,__PARAMTYPE5__,__PARAM5__,__PARAMTYPE6__,__PARAM6__) \
static __TYPE__* create(__PARAMTYPE__ __PARAM__, __PARAMTYPE2__ __PARAM2__, __PARAMTYPE3__ __PARAM3__, __PARAMTYPE4__ __PARAM4__, __PARAMTYPE5__ __PARAM5__, __PARAMTYPE6__ __PARAM6__) \
{ \
    __TYPE__ *pRet = new(std::nothrow) __TYPE__(); \
    if (pRet && pRet->init(__PARAM__, __PARAM2__, __PARAM3__, __PARAM4__, __PARAM5__, __PARAM6__)) \
    { \
        pRet->autorelease(); \
        return pRet; \
    } \
    else \
    { \
        delete pRet; \
        pRet = nullptr; \
        return nullptr; \
    } \
}
// 7つの引数を持つcreateマクロ
#define CREATE_FUNC_PARAM7(__TYPE__,__PARAMTYPE__,__PARAM__,__PARAMTYPE2__,__PARAM2__,__PARAMTYPE3__,__PARAM3__,__PARAMTYPE4__,__PARAM4__,__PARAMTYPE5__,__PARAM5__,__PARAMTYPE6__,__PARAM6__,__PARAMTYPE7__,__PARAM7__) \
static __TYPE__* create(__PARAMTYPE__ __PARAM__, __PARAMTYPE2__ __PARAM2__, __PARAMTYPE3__ __PARAM3__, __PARAMTYPE4__ __PARAM4__, __PARAMTYPE5__ __PARAM5__, __PARAMTYPE6__ __PARAM6__, __PARAMTYPE7__ __PARAM7__) \
{ \
    __TYPE__ *pRet = new(std::nothrow) __TYPE__(); \
    if (pRet && pRet->init(__PARAM__, __PARAM2__, __PARAM3__, __PARAM4__, __PARAM5__, __PARAM6__, __PARAM7__)) \
    { \
        pRet->autorelease(); \
        return pRet; \
    } \
    else \
    { \
        delete pRet; \
        pRet = nullptr; \
        return nullptr; \
    } \
}
#define PROBABILITY_MAX 10000
#define DOT_PER_METER 48.0f

// #AGTK-NX
#if CC_TARGET_PLATFORM == CC_PLATFORM_NX
#else
// Windows,その他プラットフォーム向けプリプロセッサ定義
#define USE_PRELOAD_TEX			// ※Windows,その他プラットフォーム向け Effect, Particle テクスチャのプリロードとキャッシュ有効化
#endif // CC_TARGET_PLATFORM == CC_PLATFORM_NX

#define USE_CLEAR_UNUSED_TEX	// シーン切り替え時の未使用テクスチャキャッシュ破棄

//#define USE_SAR_TEST_0
//#define USE_SAR_OHNISHI_DEBUG
//#define USE_SAR_MENU_FOR_DEBUG	// DEBUG向けメニュー拡張
//#define USE_SAR_PROVISIONAL		// 暫定的な不具合修正。修正すると挙動が変わるため無効化
#define USE_SAR_PROVISIONAL_1	// 暫定的な不具合修正。コントローラー割り当て処理
#define USE_SAR_PROVISIONAL_2	// 暫定的な不具合修正。破棄データを参照する問題の対応
#define USE_SAR_PROVISIONAL_3	// 暫定的な不具合修正。Particleのテクスチャキャッシュが利用されていない問題の対応。
#define USE_SAR_FIX_AND_OPTIMIZE_1		// 壁の分割判定数が増加しすぎる問題の対応と高速化
#define USE_SAR_PROVISIONAL_4	// 暫定的な不具合修正。未登録の壁判定を参照して本来接触しないタイルと判定を取ってしまう問題の対応。


#define FRAME60_RATE (60)

// #AGTK-NX #AGTK-WIN
//#define ENABLE_FORCE_30FPS		// 強制的な30FPS固定モード テスト用
#define USE_30FPS				// 30FPS対応
#define USE_30FPS_1				// 30FPS対応
#define USE_30FPS_2				// 30FPS対応
#define USE_30FPS_3				// 30FPS対応 60fps毎の壁判定による位置確定
#define USE_30FPS_3_2			// 30FPS対応 壁判定結果の2パス分のマージを無効化
#define USE_30FPS_4				// 30FPS対応 中間フレーム壁判定情報の適用

#ifdef USE_30FPS
#define FRAME30_RATE (30)
#define FRAME20_RATE (20)
#define FRAME_PER_SECONDS (1.0f / GameManager::getInstance()->getFrameRate())
#else
#define FRAME_PER_SECONDS (1.0f / FRAME60_RATE)
#endif

#define FRAME_ALLOWABLE_ERROR 0.001f // フレームカウント計算で許容する誤差 1/1000sec程度の誤差を無視

#define USE_LOCKING_MULTI_TARGET_FIX // オブジェクトのロックの複数対応修正版
#define USE_REDUCE_RENDER_TEXTURE
#define USE_PHYSICS_STATIC_MASS_MOMENT	//静的な物理パーツを動かないようにする。

// dump 時にboolをTrue/Falseで表示する用
#define DUMP_BOOLTEXT(x) (x ? "True" : "False")

// JSON メンバチェック
#define CHECK_JSON_MENBER(x) if (!json.HasMember(x)) { CC_ASSERT(0); return false; }

// JSONデータ設定
#define ASSIGN_JSON_MEMBER(keyName, setterName, jsonGetter) CHECK_JSON_MENBER(keyName) this->set##setterName(json[keyName].Get##jsonGetter())
#define ASSIGN_JSON_MEMBER_STR(keyName, setterName, jsonGetter) CHECK_JSON_MENBER(keyName) this->set##setterName(cocos2d::__String::create(json[keyName].Get##jsonGetter()))
#define ASSIGN_JSON_MEMBER_WITH_CAST(keyName, setterName, jsonGetter, valueType) CHECK_JSON_MENBER(keyName) this->set##setterName((valueType)json[keyName].Get##jsonGetter())

// デバッグログ
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
#define DBGMGR_ADDLOG(s, ...)				DebugManager::getInstance()->getDebugExecuteLogWindow()->addLog(s, __VA_ARGS__)
#define AGTK_ACTION_LOG(flag, s, ...)		GameManager::actionLog(flag, s, __VA_ARGS__);//action_log.txtへ出力
#define AGTK_LOG(s, ...)					GameManager::debugLog(s, __VA_ARGS__)//agtk_log.txtへ出力 
#if defined(USE_AGTK_DEBUG_LOG)
#define AGTK_DEBUG_LOG(s, ...)				GameManager::debugLog(s, __VA_ARGS__)//agtk_log.txtへ出力 
#define AGTK_DEBUG_ACTION_LOG(s, ...)		GameManager::actionLog(1, s, __VA_ARGS__);//action_log.txtへ出力
#else
#define AGTK_DEBUG_LOG(s, ...)				do {} while (0)
#define AGTK_DEBUG_ACTION_LOG(s, ...)		do {} while (0)
#endif
#else
#endif

#ifdef AGTKPLAYER_EXPORTS
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
#define AGTKPLAYER_API __declspec(dllexport)
#else
#endif
#else
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
#define AGTKPLAYER_API __declspec(dllimport)
#else
#endif
#endif

#ifdef __cplusplus
template<typename T>
class AutoDeleter {
public:
	AutoDeleter(T *obj) : _obj(obj) {}

	~AutoDeleter() { delete _obj; }
	T *_obj;
};
#endif

#endif /* __MACROS_H__ */
