#include "Particle.h"
//#include "Data/ParticleData.h"
#include "collision/CollisionComponent.hpp"
#include "collision/CollisionUtils.hpp"

#include "GameManager.h"
#include "AudioManager.h"
#include "Data/AnimationData.h"
#ifdef USE_PREVIEW
//#include "base/firePngData.h"
#endif

//#define USE_RADIUS_REFLECTION_FLEE		// 回転モードの反射で回転の影響を受けないバージョン
#define HIGH_DEFINITION_COLLISIION_CHECK	// 高精細衝突判定モード

//! パーティクルの範囲を表示するためのバウンディングボックス画像
static unsigned char boundingBoxPng[] = {
	0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a,
	0x00, 0x00, 0x00, 0x0d, 0x49, 0x48, 0x44, 0x52,
	0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x10,
	0x08, 0x03, 0x00, 0x00, 0x00, 0x28, 0x2d, 0x0f,
	0x53, 0x00, 0x00, 0x03, 0x00, 0x50, 0x4c, 0x54,
	0x45, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x80,
	0x80, 0x80, 0x00, 0x00, 0xff, 0x3f, 0x00, 0x00,
	0x3f, 0x00, 0x7f, 0x3f, 0x00, 0xbf, 0x3f, 0x00,
	0xff, 0x5f, 0x00, 0x00, 0x5f, 0x00, 0x7f, 0x5f,
	0x00, 0xbf, 0x5f, 0x00, 0xff, 0x7f, 0x00, 0x00,
	0x7f, 0x00, 0x7f, 0x7f, 0x00, 0xbf, 0x7f, 0x00,
	0xff, 0x9f, 0x00, 0x00, 0x9f, 0x00, 0x7f, 0x9f,
	0x00, 0xbf, 0x9f, 0x00, 0xff, 0xbf, 0x00, 0x00,
	0xbf, 0x00, 0x7f, 0xbf, 0x00, 0xbf, 0xbf, 0x00,
	0xff, 0xdf, 0x00, 0x00, 0xdf, 0x00, 0x7f, 0xdf,
	0x00, 0xbf, 0xdf, 0x00, 0xff, 0xff, 0x00, 0x00,
	0xff, 0x00, 0x7f, 0xff, 0x00, 0xbf, 0xff, 0x00,
	0xff, 0x00, 0x3f, 0x00, 0x00, 0x3f, 0x7f, 0x00,
	0x3f, 0xbf, 0x00, 0x3f, 0xff, 0x3f, 0x3f, 0x00,
	0x3f, 0x3f, 0x7f, 0x3f, 0x3f, 0xbf, 0x3f, 0x3f,
	0xff, 0x5f, 0x3f, 0x00, 0x5f, 0x3f, 0x7f, 0x5f,
	0x3f, 0xbf, 0x5f, 0x3f, 0xff, 0x7f, 0x3f, 0x00,
	0x7f, 0x3f, 0x7f, 0x7f, 0x3f, 0xbf, 0x7f, 0x3f,
	0xff, 0x9f, 0x3f, 0x00, 0x9f, 0x3f, 0x7f, 0x9f,
	0x3f, 0xbf, 0x9f, 0x3f, 0xff, 0xbf, 0x3f, 0x00,
	0xbf, 0x3f, 0x7f, 0xbf, 0x3f, 0xbf, 0xbf, 0x3f,
	0xff, 0xdf, 0x3f, 0x00, 0xdf, 0x3f, 0x7f, 0xdf,
	0x3f, 0xbf, 0xdf, 0x3f, 0xff, 0xff, 0x3f, 0x00,
	0xff, 0x3f, 0x7f, 0xff, 0x3f, 0xbf, 0xff, 0x3f,
	0xff, 0x00, 0x5f, 0x00, 0x00, 0x5f, 0x7f, 0x00,
	0x5f, 0xbf, 0x00, 0x5f, 0xff, 0x3f, 0x5f, 0x00,
	0x3f, 0x5f, 0x7f, 0x3f, 0x5f, 0xbf, 0x3f, 0x5f,
	0xff, 0x5f, 0x5f, 0x00, 0x5f, 0x5f, 0x7f, 0x5f,
	0x5f, 0xbf, 0x5f, 0x5f, 0xff, 0x7f, 0x5f, 0x00,
	0x7f, 0x5f, 0x7f, 0x7f, 0x5f, 0xbf, 0x7f, 0x5f,
	0xff, 0x9f, 0x5f, 0x00, 0x9f, 0x5f, 0x7f, 0x9f,
	0x5f, 0xbf, 0x9f, 0x5f, 0xff, 0xbf, 0x5f, 0x00,
	0xbf, 0x5f, 0x7f, 0xbf, 0x5f, 0xbf, 0xbf, 0x5f,
	0xff, 0xdf, 0x5f, 0x00, 0xdf, 0x5f, 0x7f, 0xdf,
	0x5f, 0xbf, 0xdf, 0x5f, 0xff, 0xff, 0x5f, 0x00,
	0xff, 0x5f, 0x7f, 0xff, 0x5f, 0xbf, 0xff, 0x5f,
	0xff, 0x00, 0x7f, 0x00, 0x00, 0x7f, 0x7f, 0x00,
	0x7f, 0xbf, 0x00, 0x7f, 0xff, 0x3f, 0x7f, 0x00,
	0x3f, 0x7f, 0x7f, 0x3f, 0x7f, 0xbf, 0x3f, 0x7f,
	0xff, 0x5f, 0x7f, 0x00, 0x5f, 0x7f, 0x7f, 0x5f,
	0x7f, 0xbf, 0x5f, 0x7f, 0xff, 0x7f, 0x7f, 0x00,
	0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0xbf, 0x7f, 0x7f,
	0xff, 0x9f, 0x7f, 0x00, 0x9f, 0x7f, 0x7f, 0x9f,
	0x7f, 0xbf, 0x9f, 0x7f, 0xff, 0xbf, 0x7f, 0x00,
	0xbf, 0x7f, 0x7f, 0xbf, 0x7f, 0xbf, 0xbf, 0x7f,
	0xff, 0xdf, 0x7f, 0x00, 0xdf, 0x7f, 0x7f, 0xdf,
	0x7f, 0xbf, 0xdf, 0x7f, 0xff, 0xff, 0x7f, 0x00,
	0xff, 0x7f, 0x7f, 0xff, 0x7f, 0xbf, 0xff, 0x7f,
	0xff, 0x00, 0x9f, 0x00, 0x00, 0x9f, 0x7f, 0x00,
	0x9f, 0xbf, 0x00, 0x9f, 0xff, 0x3f, 0x9f, 0x00,
	0x3f, 0x9f, 0x7f, 0x3f, 0x9f, 0xbf, 0x3f, 0x9f,
	0xff, 0x5f, 0x9f, 0x00, 0x5f, 0x9f, 0x7f, 0x5f,
	0x9f, 0xbf, 0x5f, 0x9f, 0xff, 0x7f, 0x9f, 0x00,
	0x7f, 0x9f, 0x7f, 0x7f, 0x9f, 0xbf, 0x7f, 0x9f,
	0xff, 0x9f, 0x9f, 0x00, 0x9f, 0x9f, 0x7f, 0x9f,
	0x9f, 0xbf, 0x9f, 0x9f, 0xff, 0xbf, 0x9f, 0x00,
	0xbf, 0x9f, 0x7f, 0xbf, 0x9f, 0xbf, 0xbf, 0x9f,
	0xff, 0xdf, 0x9f, 0x00, 0xdf, 0x9f, 0x7f, 0xdf,
	0x9f, 0xbf, 0xdf, 0x9f, 0xff, 0xff, 0x9f, 0x00,
	0xff, 0x9f, 0x7f, 0xff, 0x9f, 0xbf, 0xff, 0x9f,
	0xff, 0x00, 0xbf, 0x00, 0x00, 0xbf, 0x7f, 0x00,
	0xbf, 0xbf, 0x00, 0xbf, 0xff, 0x3f, 0xbf, 0x00,
	0x3f, 0xbf, 0x7f, 0x3f, 0xbf, 0xbf, 0x3f, 0xbf,
	0xff, 0x5f, 0xbf, 0x00, 0x5f, 0xbf, 0x7f, 0x5f,
	0xbf, 0xbf, 0x5f, 0xbf, 0xff, 0x7f, 0xbf, 0x00,
	0x7f, 0xbf, 0x7f, 0x7f, 0xbf, 0xbf, 0x7f, 0xbf,
	0xff, 0x9f, 0xbf, 0x00, 0x9f, 0xbf, 0x7f, 0x9f,
	0xbf, 0xbf, 0x9f, 0xbf, 0xff, 0xbf, 0xbf, 0x00,
	0xbf, 0xbf, 0x7f, 0xbf, 0xbf, 0xbf, 0xbf, 0xbf,
	0xff, 0xdf, 0xbf, 0x00, 0xdf, 0xbf, 0x7f, 0xdf,
	0xbf, 0xbf, 0xdf, 0xbf, 0xff, 0xff, 0xbf, 0x00,
	0xff, 0xbf, 0x7f, 0xff, 0xbf, 0xbf, 0xff, 0xbf,
	0xff, 0x00, 0xdf, 0x00, 0x00, 0xdf, 0x7f, 0x00,
	0xdf, 0xbf, 0x00, 0xdf, 0xff, 0x3f, 0xdf, 0x00,
	0x3f, 0xdf, 0x7f, 0x3f, 0xdf, 0xbf, 0x3f, 0xdf,
	0xff, 0x5f, 0xdf, 0x00, 0x5f, 0xdf, 0x7f, 0x5f,
	0xdf, 0xbf, 0x5f, 0xdf, 0xff, 0x7f, 0xdf, 0x00,
	0x7f, 0xdf, 0x7f, 0x7f, 0xdf, 0xbf, 0x7f, 0xdf,
	0xff, 0x9f, 0xdf, 0x00, 0x9f, 0xdf, 0x7f, 0x9f,
	0xdf, 0xbf, 0x9f, 0xdf, 0xff, 0xbf, 0xdf, 0x00,
	0xbf, 0xdf, 0x7f, 0xbf, 0xdf, 0xbf, 0xbf, 0xdf,
	0xff, 0xdf, 0xdf, 0x00, 0xdf, 0xdf, 0x7f, 0xdf,
	0xdf, 0xbf, 0xdf, 0xdf, 0xff, 0xff, 0xdf, 0x00,
	0xff, 0xdf, 0x7f, 0xff, 0xdf, 0xbf, 0xff, 0xdf,
	0xff, 0x00, 0xff, 0x00, 0x00, 0xff, 0x7f, 0x00,
	0xff, 0xbf, 0x00, 0xff, 0xff, 0x3f, 0xff, 0x00,
	0x3f, 0xff, 0x7f, 0x3f, 0xff, 0xbf, 0x3f, 0xff,
	0xff, 0x5f, 0xff, 0x00, 0x5f, 0xff, 0x7f, 0x5f,
	0xff, 0xbf, 0x5f, 0xff, 0xff, 0x7f, 0xff, 0x00,
	0x7f, 0xff, 0x7f, 0x7f, 0xff, 0xbf, 0x7f, 0xff,
	0xff, 0x9f, 0xff, 0x00, 0x9f, 0xff, 0x7f, 0x9f,
	0xff, 0xbf, 0x9f, 0xff, 0xff, 0xbf, 0xff, 0x00,
	0xbf, 0xff, 0x7f, 0xbf, 0xff, 0xbf, 0xbf, 0xff,
	0xff, 0xdf, 0xff, 0x00, 0xdf, 0xff, 0x7f, 0xdf,
	0xff, 0xbf, 0xdf, 0xff, 0xff, 0xff, 0xff, 0x00,
	0xff, 0xff, 0x7f, 0xff, 0xff, 0xbf, 0xff, 0xff,
	0xff, 0x24, 0x27, 0x72, 0xfa, 0x00, 0x00, 0x01,
	0x00, 0x74, 0x52, 0x4e, 0x53, 0xff, 0x00, 0x80,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xdf, 0x9d, 0xfa,
	0xc1, 0x00, 0x00, 0x00, 0x20, 0x74, 0x45, 0x58,
	0x74, 0x53, 0x6f, 0x66, 0x74, 0x77, 0x61, 0x72,
	0x65, 0x00, 0x70, 0x6e, 0x67, 0x69, 0x6f, 0x2e,
	0x63, 0x28, 0x62, 0x79, 0x20, 0x4b, 0x65, 0x69,
	0x6a, 0x69, 0x20, 0x41, 0x67, 0x75, 0x73, 0x61,
	0x29, 0x97, 0xb7, 0x42, 0xf6, 0x00, 0x00, 0x00,
	0x14, 0x49, 0x44, 0x41, 0x54, 0x78, 0x9c, 0x63,
	0xf8, 0x8f, 0x06, 0x18, 0xfe, 0x33, 0xa1, 0x80,
	0xe1, 0x2e, 0x80, 0x06, 0x00, 0x93, 0x50, 0x3d,
	0x4d, 0x63, 0xd2, 0xb4, 0xfc, 0x00, 0x00, 0x00,
	0x00, 0x49, 0x45, 0x4e, 0x44, 0xae, 0x42, 0x60,
	0x82
};

NS_AGTK_BEGIN
//-------------------------------------------------------------------------------------------------------------------
// パーティクルクラス
//-------------------------------------------------------------------------------------------------------------------
/**
 * コンストラクタ
 * @param	seed	ランダム用シード
 */
Particle::Particle(int seed)
{
	_textureId = TEXTURE_ID_NONE;
	_id = -1;
	_paused = false;
	_stopped = false;
	_duration300 = 300;
	_loop = false;
	_gravityDirection = 0;
	_gravity = 9.8f;
#ifdef USE_AGTK_PARTICLE_LOCAL_RANDOM
	_mt.seed(seed);
#endif
	_playSeId = SOUND_ID_NONE;
	_playVoiceId = VOICE_ID_NONE;
	_sceneId = 0;
	_sceneLayerId = 0;
	_animeParticleData = nullptr;
	_alpha = 1.0f;
	_isDisableAntiAlias = false;
	_objectNode = nullptr;
	_base64Cache = "";
#ifdef USE_PREVIEW
	_lastDirection = 0;
	_lastEndRadius = 0;
#endif
	_parentBeforeRemoveChild = nullptr;
}

/**
 * デストラクタ
 */
Particle::~Particle()
{
#ifdef USE_PREVIEW
	if (getTextureId() != TEXTURE_ID_NONE){
		removeTexture(getTextureId());
	}
#endif
	CC_SAFE_RELEASE_NULL(_animeParticleData);
}

/**
 * 生成
 * @param	seed	ランダム用シード
 * @return			インスタンス or nullptr
 */
Particle* Particle::create(int seed)
{
	Particle* ret = new (std::nothrow) Particle(seed);
	if (ret && ret->initWithTotalParticles(128))
	{
		ret->autorelease();
	}
	else
	{
		CC_SAFE_DELETE(ret);
	}
	return ret;
}

/**
 * 生成
 * @param	seed					ランダム用シード
 * @param	animeParticleData		パーティクル型アニメーションデータ
 * @param	animParticleImageList	パーティクル型アニメーション用画像リスト
 * @return							インスタンス or nullptr
 */
Particle* Particle::create(int seed, agtk::data::AnimeParticleData *animeParticleData, cocos2d::Dictionary *animParticleImageList, cocos2d::Node* objectNode)
{
	Particle* ret = new (std::nothrow) Particle(seed);
	if (ret && ret->initWithAnimeParticleData(animeParticleData, animParticleImageList, objectNode))
	{
		ret->autorelease();
	}
	else
	{
		CC_SAFE_DELETE(ret);
	}
	return ret;
}

/**
 * テクスチャーをキャッシュするキーを作成
 * @param	id	テクスチャID
 * @return		パーティクル画像のキー名
 * @note		(負数はシステムが用意した画像 / 正数はユーザーが用意した画像)
 */
std::string Particle::getTextureKey(int id)
{
	auto projectData = GameManager::getInstance()->getProjectData();
	if (projectData != nullptr) {
		auto particleData = projectData->getParticleImageData(id);
		if (nullptr != particleData) {
			this->setTextureId(id);
			std::string key = particleData->getFilename()->_string;
			// アンチエイリアス無効の場合、keyに判別できる文字列を追加
			if (_isDisableAntiAlias) {
				key.append("_DAA");
			}
			return key;
		}
	}
	return std::string("boundingbox");
}

/**
 * idのテクスチャーを作成＆キャッシュし、返す。
 * @param	id			パーティクル画像ID
 * @param	base64		base64エンコードされた画像データ
 * @param	isPreview	プレビューモードフラグ
 * @return				Texture2D のポインタ
 * @note				base64には画像をbase64エンコードしたバイナリーデータを設定する。nullptrの場合はバウンディングボックス画像が使われる。
 * 						エディターのプレビューでは同じIDで画像が変わっているかもしれないため、毎回テクスチャーを作り直す必要がある。
 */
Texture2D *Particle::getTexture(int id, const char *base64, bool isPreview)
{
	Texture2D* texture = nullptr;
	Image* image = nullptr;
	bool ret = false;

	const std::string key = getTextureKey(id);

// #AGTK-NX #AGTK-WIN
#ifdef 	USE_SAR_PROVISIONAL_3
	if (!isPreview) {
		// 通常時はテクスチャキャッシュを優先的に参照
		texture = Director::getInstance()->getTextureCache()->getTextureForKey(key);
		if (texture) {
			return texture;
		}
	}
#endif

	image = new (std::nothrow) Image();
	if (nullptr == image) {
		return nullptr;
	}

	// プレビュー用の場合
	if (isPreview) {
		// 毎回テクスチャーを作り直すため、一度キャッシュを削除
		Director::getInstance()->getTextureCache()->removeTextureForKey(key);
		// 画像データがある場合
		if (nullptr != base64) {
			unsigned char *buffer;
			auto len = base64Decode((unsigned char *)base64, strlen(base64), &buffer);
			if (nullptr == buffer) {
				return nullptr;
			}
			ret = image->initWithImageData(buffer, len);
			free(buffer);
		}
		// boundingBox表示の場合
		else {
			ret = image->initWithImageData(boundingBoxPng, sizeof(boundingBoxPng));
		}
	}
	// 通常時の場合
	else {
		std::string path;
		if (_isDisableAntiAlias) {
			path = key.substr(0, key.size() - 4);	// "_DAA"分削除
		}
		else {
			path = key;
		}
		// キー名が boundingbox の場合(画像が存在しない場合これになる)
		if (strcmp(path.c_str(), "boundingbox") == 0) {
			ret = image->initWithImageData(boundingBoxPng, sizeof(boundingBoxPng));
		}
		else {
			ret = image->initWithImageFile(path);
		}
	}

	if (ret) {
		// テクスチャ生成
		texture = Director::getInstance()->getTextureCache()->addImage(image, key);
		if (_isDisableAntiAlias) {
			// アンチエイリアスを無効
			texture->setAliasTexParameters();
		}
		else {
			texture->setAntiAliasTexParameters();
		}
	}

	CC_SAFE_RELEASE(image);

	return texture;
}

/**
 * キャッシュしていた画像を削除する
 * @param	id	テクスチャID
 */
void Particle::removeTexture(int id)
{
	//const std::string key = getTextureKey(id);
	//Director::getInstance()->getTextureCache()->removeTextureForKey(key);

	(void)id;
	auto texture = ParticleSystem::getTexture();
	if (texture != nullptr) {
		Director::getInstance()->getTextureCache()->removeTexture(texture);
	}
}

/**
 * アニメパティクルデータを元に初期化
 * @param	animeParticleData		アニメパーティクルデータ
 * @param	animParticleImageList	アニメパーティクル画像リスト
 */
bool Particle::initWithAnimeParticleData(agtk::data::AnimeParticleData *animeParticleData, cocos2d::Dictionary *animParticleImageList, cocos2d::Node* objectNode)
{
	// パーティクルシステム初期化成功時
	if( ParticleSystemQuad::initWithTotalParticles(animeParticleData->getEmitVolume()) )
	{
		// パーティクルアニメーションデータ
		this->setAnimeParticleData(animeParticleData);

		// ----------------------------------------------------
		// サウンド設定
		// ----------------------------------------------------
		// サウンド再生の有無
		this->setIsPlaySe(animeParticleData->getPlaySe());

		// サウンドID
		this->setPlaySeId(animeParticleData->getPlaySeId());

		// 音声再生の有無
		this->setIsPlayVoice(animeParticleData->getPlayVoice());

		// 音声ID
		this->setPlayVoiceId(animeParticleData->getPlayVoiceId());

		// ----------------------------------------------------
		// パーティクル設定
		// ----------------------------------------------------
		// ループのON/OFF
		this->setLoop(animeParticleData->getLoop());

		// パーティクルはカメラに追従してほしいので位置タイプはRELATIVEに設定
		this->setPositionType(cocos2d::ParticleSystem::PositionType::RELATIVE);

		// 継続時間
		this->setDuration(getLoop() ? DURATION_INFINITY : animeParticleData->getDuration300() / 300.0f);

		// ライフタイム
		this->setLife(animeParticleData->getLifetime300() / 300.0f);

		// ライフタイムの分散度
		this->setLifeVar(animeParticleData->getLifetimeDispersion() / 300.0f);

		// 発生量
		this->setTotalParticles(animeParticleData->getEmitVolume());
		this->setEmissionRate(getTotalParticles() / (getLife() == 0 ? 1 : getLife()));

		// 同時発生のON/OFF
		this->setInstantEmit(animeParticleData->getInstantEmit());

		// 事前発生のON/OFF
		this->setPreviousEmit(animeParticleData->getPreviousEmit());

		// パーティクルモード(重力 or 回転)
		auto emitterMode = animeParticleData->getEmitterMode();
		this->_emitterMode = (emitterMode == data::AnimeParticleData::kEmitterModeGravity) ? Mode::GRAVITY : (emitterMode == data::AnimeParticleData::kEmitterModeRotation) ? Mode::RADIUS : Mode::GRAVITY;
		
		// 発生方向
		this->setAngle((this->_emitterMode == Mode::GRAVITY ? 90.0f : -90.0f) - animeParticleData->getDirection());

		// 発生方向の分散具合
		this->setAngleVar(animeParticleData->getDirectionDispersion());

		// 開始サイズ
		this->setStartSize(animeParticleData->getStartSize());

		// 開始サイズの分散度
		this->setStartSizeVar(animeParticleData->getStartSizeDispersion());

		// 最終サイズ
		this->setEndSize(animeParticleData->getEndSize());

		// 最終サイズの分散度
		this->setEndSizeVar(animeParticleData->getEndSizeDispersion());

		// 開始角度
		this->setStartSpin(animeParticleData->getStartRotation());

		// 開始角度の分散度
		this->setStartSpinVar(animeParticleData->getStartRotationDispersion());

		// 最終角度
		this->setEndSpin(animeParticleData->getEndRotation());

		// 最終角度の分散度
		this->setEndSpinVar(animeParticleData->getEndRotationDispersion());

		// 最終角度を相対値で設定
		this->setEndRotationIsRelative(animeParticleData->getEndRotationIsRelative());

		// 加算モード
		this->setBlendAdditive(animeParticleData->getAddMode());

		// ----------------------------------------------------
		// 「動作パターン：重力」の場合
		if(this->_emitterMode == Mode::GRAVITY){

			// 重力の方向
			this->setGravityDirection(animeParticleData->getGravityDirection());
			float rad = (90 - getGravityDirection()) * M_PI / 180.0f;
			float gravity = animeParticleData->getGravity();
			this->modeA.gravity.set(cosf(rad) * gravity, sinf(rad) * gravity);

			// 重力の強さ
			this->setGravity(animeParticleData->getGravity());

			// 速度と速度の分散度
			this->modeA.speed = animeParticleData->getSpeed();
			this->modeA.speedVar = animeParticleData->getSpeedDispersion();

			// 接線加速度
			this->modeA.tangentialAccel = animeParticleData->getTangentAccel();

			// 接線加速度の分散度
			this->modeA.tangentialAccelVar = animeParticleData->getTangentAccelDispersion();

			// 法線加速度
			this->modeA.radialAccel = animeParticleData->getNormalAccel();

			// 法線加速度の分散度
			this->modeA.radialAccelVar = animeParticleData->getNormalAccelDispersion();

			// TODO: 各パーティクル毎の回転方向
			// ※エディタ側に該当する設定項目が無いためコメントアウト中
			//this->modeA.rotationIsDir;
		}
		// ----------------------------------------------------
		// 「動作パターン：回転」の場合
		else {
			// 開始回転幅
			this->modeB.startRadius = animeParticleData->getStartRotationWidth();
			
			// 開始回転幅の分散度
			this->modeB.startRadiusVar = animeParticleData->getStartRotationWidthDispersion();
			
			// 最終回点幅
			//! 最終回転幅が負数の場合は発生方向を逆転させる
			auto endRadius = animeParticleData->getEndRotationWidth();
			if (endRadius < 0) {
				endRadius *= -1;
				this->setAngle(fmod(this->getAngle() + 180.0f, 360.0f));
			}
			this->modeB.endRadius = endRadius;
			
			// 最終回転幅の分散度
			this->modeB.endRadiusVar = animeParticleData->getEndRotationWidthDispersion();
			
			// 回転数(1秒間に何回転するか)
			this->modeB.rotatePerSecond = animeParticleData->getRotations() * 360.0f;

			// 回転数の分散度
			this->modeB.rotatePerSecondVar = animeParticleData->getRotationsDispersion();
		}

		// パーティクル生成位置
		this->setSourcePosition(Vec2(animeParticleData->getX(), -animeParticleData->getY()));
		this->_posVar.set(animeParticleData->getXDispersion(), animeParticleData->getYDispersion());

		// 1フレームのパーティクル生成数
		this->setEmissionRate(_totalParticles / _life);

		// ----------------------------------------------------
		// パーティクルカラー
		// ----------------------------------------------------
		this->setAlpha(1.0f);

		this->setMiddleColorPercent(animeParticleData->getMiddlePercent());

		// ----------------------------------------------------
		// アンチエイリアスの無効
		// ----------------------------------------------------
		this->setIsDisableAntiAlias(animeParticleData->getDisableAntiAlias());

		// ----------------------------------------------------
		// パーティクルテクスチャ
		// ----------------------------------------------------
		Texture2D* texture = getTexture(animeParticleData->getTemplateId(), nullptr);
		if (texture != nullptr)
		{
			setTexture(texture);
		}

		// ----------------------------------------------------
		// その他
		// ----------------------------------------------------
		// 判定の有無
		this->setIsCheck(animeParticleData->getCheck());

		// 壁や当たり判定で跳ねるフラグ
		this->setTouchBound(animeParticleData->getTouchBound());

		// 壁や当たり判定で跳ねる際の反発係数(※要望により反発力を強化しています)
		this->setRepulsion(animeParticleData->getRepulsion() * INCREASE_REPULTION);

		// 壁や当たり判定で消えるフラグ
		this->setTouchDisappear(animeParticleData->getTouchDisappear());

		// 壁や当たり判定で消えるまでのヒット回数
		this->setDisappearCount(animeParticleData->getDisappearCount());
		_collisionCount = this->getDisappearCount();

		// ----------------------------------------------------
		// ParticleSystem への独自設定
		// ----------------------------------------------------
		// ループしない場合
		if (!this->getLoop()) {
			// 自動的に削除されるよう設定
			this->setAutoRemoveOnFinish(true);
		}

		// 生成直後は待機
		this->pauseEmissions();

		// オブジェクトのノード設定がされていたら保存
		if (objectNode != nullptr) {
			this->_objectNode = objectNode;
		}

		return true;
	}

	return false;
}

#ifdef USE_PREVIEW
//パーティクルにパラメータを設定する。
//boundingBoxが真のときは、バウンディングボックス用の設定に差し替える。
//todo バウンディングボックスの表示が画像を使っているため、チラチラする問題あり。直線プリミティブを使った表示の方がきれいになるが、パーティクルの輪郭の算出が面倒そう。
void Particle::setParameters(const rapidjson::Value &json, bool boundingBox)
{
	//「パーティクルモード(emitterMode)」が「発生方向(direction)」より先に設定されるように。
	//「最終回転幅(endRotationWidth)」が「発生方向(direction)」より後に設定されるように。
	std::list<const char *> keyList;
	std::list<const char *> lastKeyList;
	for (auto it = json.MemberBegin(); it != json.MemberEnd(); it++) {
		auto key = it->name.GetString();
		if (strcmp(key, "direction") == 0 || strcmp(key, "endRotationWidth") == 0) {
			lastKeyList.push_back(key);
			continue;
		}
		keyList.push_back(key);
	}
	
	if (lastKeyList.size() > 0) {
		for (auto key : lastKeyList) {
			keyList.push_back(key);
		}
	}
	for(auto paramName: keyList){
		auto &value = json[paramName];
		if(strcmp(paramName, "id") == 0) {
			this->setId(value.GetInt());
		} else if(strcmp(paramName, "playSe") == 0) {
			// サウンド再生の有無
			this->setIsPlaySe(value.GetBool());
		} else if(strcmp(paramName, "playSeId") == 0){
			// サウンドID
			this->setPlaySeId(value.GetInt());
		} else if(strcmp(paramName, "playVoice") == 0){
			// 音声再生の有無
			this->setIsPlayVoice(value.GetBool());
		} else if(strcmp(paramName, "playVoiceId") == 0){
			// 音声ID
			this->setPlayVoiceId(value.GetInt());
		} else if(strcmp(paramName, "duration300") == 0){
			// 継続時間
			this->setDuration300(value.GetInt());
			this->setDuration(getLoop() ? DURATION_INFINITY : getDuration300() / 300.0f);
		} else if (strcmp(paramName, "loop") == 0){
			// ループのON/OFF
			this->setLoop(value.GetBool());
			this->setDuration(getLoop() ? DURATION_INFINITY : getDuration300() / 300.0f);
		} else if (strcmp(paramName, "emitVolume") == 0){
			// 発生量
			this->setTotalParticles(16383);	//パーティクルの最大数を動的に変えると問題を起こすため、あらかじめ最大数分の領域を確保させる。
			this->setTotalParticles(value.GetInt());
			this->setEmissionRate(getTotalParticles() / (getLife() == 0 ? 1 : getLife()));
		} else if(strcmp(paramName, "instantEmit") == 0){
			// 同時発生のON/OFF
			this->setInstantEmit(value.GetBool());
		} else if(strcmp(paramName, "previousEmit") == 0){
			// 事前発生のON/OFF
			this->setPreviousEmit(value.GetBool());
		} else if(strcmp(paramName, "lifetime300") == 0){
			// 生存時間
			this->setLife(value.GetInt() / 300.0f);
			this->setEmissionRate(getTotalParticles() / (getLife() == 0 ? 1 : getLife()));
		} else if(strcmp(paramName, "lifetimeDispersion") == 0){
			// ライフタイムの分散度
			this->setLifeVar(value.GetInt() / 300.0f);
		} else if(strcmp(paramName, "direction") == 0){
			// 発生方向
			this->setAngle((this->_emitterMode == Mode::GRAVITY ? 90.0f : -90.0f) - value.GetDouble());
			this->setLastDirection(value.GetDouble());
		} else if(strcmp(paramName, "directionDispersion") == 0){
			// 発生方向の分散具合
			this->setAngleVar(value.GetDouble());
		} else if(strcmp(paramName, "speed") == 0){
			// 重力：速度と速度の分散度
			this->modeA.speed = value.GetDouble();
		} else if(strcmp(paramName, "speedDispersion") == 0){
			// 速度の分散度
			this->modeA.speedVar = value.GetDouble();
		} else if(strcmp(paramName, "startSize") == 0){
			// 開始サイズ
			this->setStartSize(value.GetDouble());
		} else if(strcmp(paramName, "startSizeDispersion") == 0){
			// 開始サイズの分散度
			this->setStartSizeVar(value.GetDouble());
		} else if(strcmp(paramName, "endSize") == 0){
			// 最終サイズ
			this->setEndSize(value.GetDouble());
		} else if(strcmp(paramName, "endSizeDispersion") == 0){
			// 最終サイズの分散度
			this->setEndSizeVar(value.GetDouble());
		} else if(strcmp(paramName, "startRotation") == 0){
			// 開始角度
			this->setStartSpin(value.GetDouble());
		} else if(strcmp(paramName, "startRotationDispersion") == 0){
			// 開始角度の分散度
			this->setStartSpinVar(value.GetDouble());
		} else if(strcmp(paramName, "endRotation") == 0){
			// 最終角度
			this->setEndSpin(value.GetDouble());
		} else if(strcmp(paramName, "endRotationDispersion") == 0){
			// 最終角度の分散度
			this->setEndSpinVar(value.GetDouble());
		} else if(strcmp(paramName, "endRotationIsRelative") == 0){
			// 最終角度を相対値で設定
			this->setEndRotationIsRelative(value.GetBool());
		} else if(strcmp(paramName, "addMode") == 0){
			// 加算モード
			this->setBlendAdditive(value.GetBool());
		} else if(strcmp(paramName, "emitterMode") == 0){
			// パーティクルモード(重力 or 回転)
			auto emitterMode = value.GetInt();
			this->_emitterMode = (emitterMode == data::AnimeParticleData::kEmitterModeGravity) ? Mode::GRAVITY : (emitterMode == data::AnimeParticleData::kEmitterModeRotation) ? Mode::RADIUS : Mode::GRAVITY;
			if (emitterMode == data::AnimeParticleData::kEmitterModeGravity) {
				this->setAngle(90.0f - this->getLastDirection());
			}
			else if (emitterMode == data::AnimeParticleData::kEmitterModeRotation) {
				auto angle = -90.0f - this->getLastDirection();
				auto endRadius = this->getLastEndRadius();
				if (endRadius < 0) {
					//endRadius *= -1;
					angle = (fmod(angle + 180.0f, 360.0f));
				}
				this->setAngle(angle);
			}
		} else if(strcmp(paramName, "rotations") == 0){
			// 回転数(1秒間に何回転するか)
			this->modeB.rotatePerSecond = value.GetDouble() * 360.0f;
		} else if(strcmp(paramName, "rotationsDispersion") == 0){
			// 回転数の分散度
			this->modeB.rotatePerSecondVar = value.GetDouble();
		} else if(strcmp(paramName, "startRotationWidth") == 0){
			// 開始回転幅
			this->modeB.startRadius = value.GetDouble();
		} else if(strcmp(paramName, "startRotationWidthDispersion") == 0){
			// 開始回転幅の分散度
			this->modeB.startRadiusVar = value.GetDouble();
		} else if(strcmp(paramName, "endRotationWidth") == 0){
			// 最終回点幅
			//! 最終回転幅が負数の場合は発生方向を逆転させる
			auto endRadius = value.GetDouble();
			this->setLastEndRadius(endRadius);
			if (endRadius < 0) {
				endRadius *= -1;
				if (this->_emitterMode == Mode::RADIUS) {
					this->setAngle(fmod(this->getAngle() + 180.0f, 360.0f));
				}
			}
			this->modeB.endRadius = endRadius;
		} else if(strcmp(paramName, "endRotationWidthDispersion") == 0){
			// 最終回転幅の分散度
			this->modeB.endRadiusVar = value.GetDouble();
		} else if(strcmp(paramName, "normalAccel") == 0){
			// 法線加速度
			this->modeA.radialAccel = value.GetDouble();
		} else if(strcmp(paramName, "normalAccelDispersion") == 0){
			// 法線加速度の分散度
			this->modeA.radialAccelVar = value.GetDouble();
		} else if(strcmp(paramName, "tangentAccel") == 0){
			// 接線加速度
			this->modeA.tangentialAccel = value.GetDouble();
		} else if(strcmp(paramName, "tangentAccelDispersion") == 0){
			// 接線加速度の分散度
			this->modeA.tangentialAccelVar = value.GetDouble();
		} else if(strcmp(paramName, "gravityDirection") == 0){
			// 重力の方向
			this->setGravityDirection(value.GetDouble());
			float rad = (90 - getGravityDirection()) * M_PI / 180.0f;
			float gravity = getGravity();
			this->modeA.gravity.set(cosf(rad) * gravity, sinf(rad) * gravity);
		} else if(strcmp(paramName, "gravity") == 0){
			// 重力の強さ
			this->setGravity(value.GetDouble());
			float rad = (90 - getGravityDirection()) * M_PI / 180.0f;
			float gravity = getGravity();
			this->modeA.gravity.set(cosf(rad) * gravity, sinf(rad) * gravity);
		} else if(strcmp(paramName, "x") == 0){
			// パーティクル生成X座標
			//記録のみ。プレビューの座標更新はParticleManager::update(float delta, GameManager *gameManager)で実行。
			float y = this->getEmitPos().y;
			this->setEmitPos(Vec2(value.GetDouble(), y));
		} else if(strcmp(paramName, "xDispersion") == 0){
			// パーティクル生成X座標の分散度
			this->_posVar.x = value.GetDouble();
		} else if(strcmp(paramName, "y") == 0){
			// パーティクル生成Y座標
			//記録のみ。プレビューの座標更新はParticleManager::update(float delta, GameManager *gameManager)で実行。
			float x = this->getEmitPos().x;
			this->setEmitPos(Vec2(x, -value.GetDouble()));
		} else if(strcmp(paramName, "yDispersion") == 0){
			// パーティクル生成Y座標の分散度
			this->_posVar.y = value.GetDouble();
		} else if (strcmp(paramName, "startAlpha") == 0){
			if (boundingBox){
				_startColor.a = 1.0f;
			} else {
				_startColor.a = value.GetInt() / 255.0f;
			}
		} else if(strcmp(paramName, "startAlphaDispersion") == 0){
			if (boundingBox){
				_startColorVar.a = 0.0f;
			} else {
				_startColorVar.a = value.GetInt() / 255.0f;
			}
		} else if(strcmp(paramName, "startR") == 0){
			if (boundingBox){
				_startColor.r = 1.0f;
			} else {
				_startColor.r = value.GetInt() / 255.0f;
			}
		} else if(strcmp(paramName, "startRDispersion") == 0){
			if (boundingBox){
				_startColorVar.r = 0.0f;
			} else {
				_startColorVar.r = value.GetInt() / 255.0f;
			}
		} else if(strcmp(paramName, "startG") == 0){
			if (boundingBox){
				_startColor.g = 1.0f;
			} else {
				_startColor.g = value.GetInt() / 255.0f;
			}
		} else if(strcmp(paramName, "startGDispersion") == 0){
			if (boundingBox){
				_startColorVar.g = 0.0f;
			} else {
				_startColorVar.g = value.GetInt() / 255.0f;
			}
		} else if(strcmp(paramName, "startB") == 0){
			if (boundingBox){
				_startColor.b = 1.0f;
			} else {
				_startColor.b = value.GetInt() / 255.0f;
			}
		} else if(strcmp(paramName, "startBDispersion") == 0){
			if (boundingBox){
				_startColorVar.b = 0.0f;
			} else {
				_startColorVar.b = value.GetInt() / 255.0f;
			}
		} else if (strcmp(paramName, "middleAlpha") == 0) {
			if (boundingBox) {
				_middleColor.a = 1.0f;
			}
			else {
				_middleColor.a = value.GetInt() / 255.0f;
			}
		}
		else if (strcmp(paramName, "middleAlphaDispersion") == 0) {
			if (boundingBox) {
				_middleColorVar.a = 0.0f;
			}
			else {
				_middleColorVar.a = value.GetInt() / 255.0f;
			}
		}
		else if (strcmp(paramName, "middleR") == 0) {
			if (boundingBox) {
				_middleColor.r = 1.0f;
			}
			else {
				_middleColor.r = value.GetInt() / 255.0f;
			}
		}
		else if (strcmp(paramName, "middleRDispersion") == 0) {
			if (boundingBox) {
				_middleColorVar.r = 0.0f;
			}
			else {
				_middleColorVar.r = value.GetInt() / 255.0f;
			}
		}
		else if (strcmp(paramName, "middleG") == 0) {
			if (boundingBox) {
				_middleColor.g = 1.0f;
			}
			else {
				_middleColor.g = value.GetInt() / 255.0f;
			}
		}
		else if (strcmp(paramName, "middleGDispersion") == 0) {
			if (boundingBox) {
				_middleColorVar.g = 0.0f;
			}
			else {
				_middleColorVar.g = value.GetInt() / 255.0f;
			}
		}
		else if (strcmp(paramName, "middleB") == 0) {
			if (boundingBox) {
				_middleColor.b = 1.0f;
			}
			else {
				_middleColor.b = value.GetInt() / 255.0f;
			}
		}
		else if (strcmp(paramName, "middleBDispersion") == 0) {
			if (boundingBox) {
				_middleColorVar.b = 0.0f;
			}
			else {
				_middleColorVar.b = value.GetInt() / 255.0f;
			}
		}
		else if (strcmp(paramName, "middlePercent") == 0) {
			if (boundingBox) {
				_middleColorPer = 50.f;
			}
			else {
				_middleColorPer = value.GetInt();
			}
		} else if(strcmp(paramName, "endAlpha") == 0){
			if (boundingBox){
				_endColor.a = 1.0f;
			} else {
				_endColor.a = value.GetInt() / 255.0f;
			}
		} else if(strcmp(paramName, "endAlphaDispersion") == 0){
			if (boundingBox){
				_endColorVar.a = 0.0f;
			} else {
				_endColorVar.a = value.GetInt() / 255.0f;
			}
		} else if(strcmp(paramName, "endR") == 0){
			if (boundingBox){
				_endColor.r = 1.0f;
			} else {
				_endColor.r = value.GetInt() / 255.0f;
			}
		} else if(strcmp(paramName, "endRDispersion") == 0){
			if (boundingBox){
				_endColorVar.r = 0.0f;
			} else {
				_endColorVar.r = value.GetInt() / 255.0f;
			}
		} else if(strcmp(paramName, "endG") == 0){
			if (boundingBox){
				_endColor.g = 1.0f;
			} else {
				_endColor.g = value.GetInt() / 255.0f;
			}
		} else if(strcmp(paramName, "endGDispersion") == 0){
			if (boundingBox){
				_endColorVar.g = 0.0f;
			} else {
				_endColorVar.g = value.GetInt() / 255.0f;
			}
		} else if(strcmp(paramName, "endB") == 0){
			if (boundingBox){
				_endColor.b = 1.0f;
			} else {
				_endColor.b = value.GetInt() / 255.0f;
			}
		} else if(strcmp(paramName, "endBDispersion") == 0){
			if (boundingBox){
				_endColorVar.b = 0.0f;
			} else {
				_endColorVar.b = value.GetInt() / 255.0f;
			}
		} else if(strcmp(paramName, "templateId") == 0){
			// パーティクルテクスチャ
			if (boundingBox){
				Texture2D* texture = getTexture(-getId(), nullptr, true);
				if (texture != nullptr)
				{
					setTexture(texture);
				}
			} else if (value.IsString()){
				_base64Cache = value.GetString();
				Texture2D* texture = getTexture(getId(), value.GetString(), true);
				if (texture != nullptr)
				{
					setTexture(texture);
				}
			}
		} else if (strcmp(paramName, "disableAntiAlias") == 0) {
			// アンチエイリアスを無効
			this->setIsDisableAntiAlias(value.GetBool());

			if (keyList.size() == 1) {
				// プレビュー時はキャッシュを毎度消しているので再生中のアンチエイリアス有無を変更した際にも再生成する
				if (boundingBox) {
					Texture2D* texture = getTexture(-getId(), nullptr, true);
					if (texture != nullptr)
					{
						setTexture(texture);
					}
				}
				else if (_base64Cache.compare("") != 0) {
					Texture2D* texture = getTexture(getId(), _base64Cache.c_str(), true);
					if (texture != nullptr)
					{
						setTexture(texture);
					}
				}
			}
		} else if(strcmp(paramName, "check") == 0){
			// 判定の有無
			this->setIsCheck(value.GetBool());
		} else if(strcmp(paramName, "touchDisappear") == 0){
			// 壁や当たり判定で消えるフラグ
			this->setTouchDisappear(value.GetBool());
		} else if(strcmp(paramName, "disappearCount") == 0){
			// 壁や当たり判定で消えるまでのヒット回数
			this->setDisappearCount(value.GetInt());
			this->_collisionCount = this->getDisappearCount();
		} else if(strcmp(paramName, "touchBound") == 0){
			// 壁や当たり判定で跳ねるフラグ
			this->setTouchBound(value.GetBool());
		} else if(strcmp(paramName, "repulsion") == 0){
			// 壁や当たり判定で跳ねる際の反発係数
			this->setRepulsion(value.GetDouble());
		} else if(strcmp(paramName, "reset") == 0){
			reset();
		}
	}
}
#endif

/**
 * パーティクルの動作を開始（再開）
 */
void Particle::play()
{
	// ポーズ中だった場合
	if(getPaused()){
		setPaused(false);
		//this->_isActive = true;
	}

	// 停止中だった場合
	if(getStopped()){
		setStopped(false);
		reset();
#if 0	//reset()内で既に処理しているので二重処理を回避。
		// 事前発生か同時発生がONの場合
		if(isActuallyPreviousEmit() || getInstantEmit()){
			procInitialEmit();
		}

		// サウンド再生がON かつ 不正なサウンドIDでない場合
		if (getIsPlaySe() && getPlaySeId() != SOUND_ID_NONE) {
			AudioManager::getInstance()->playSe(getPlaySeId());
		}

		// 音声再生がON かつ 不正な音声IDでない場合
		if (getIsPlayVoice() && getPlayVoiceId() != VOICE_ID_NONE) {
			AudioManager::getInstance()->playVoice(getPlayVoiceId());
		}
#endif
	}
}

/**
 * パーティクルの動作を一時停止
 */
void Particle::pause()
{
	if(!getStopped() && !getPaused()){
		setPaused(true);
		//this->_isActive = false;
	}
}

/**
 * パーティクルの動作を停止
 * @param	isReset	パーティクルの状態を初期化するか？
 * @note isReset がON の場合パーティクルの状態を初期状態に戻す。
 */
void Particle::stop(bool isReset)
{
	if (getPaused()){
		setPaused(false);
	}
	if (!getStopped()){

		if (isReset) {
			setStopped(true);
			resetSystem();
			stopSystem();
		}
		else {
			this->_isActive = false;
		}
	}
}

/**
 * パーティクルの状態を初期状態にリセット
 */
void Particle::reset()
{
	if (!getStopped() && !getPaused()){
		this->setEmissionRate(getTotalParticles() / (getLife() == 0 ? 1 : getLife()));	//事前発生で書き換えるため戻す。
		resetSystem();

		// 事前発生か同時発生がONの場合
		if (isActuallyPreviousEmit() || getInstantEmit()){
			procInitialEmit();
		}

		// サウンド再生がON かつ 不正なサウンドIDでない場合
		if (getIsPlaySe() && getPlaySeId() != SOUND_ID_NONE) {
			if (_objectNode != nullptr) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto _object = static_cast<agtk::Object *>(_objectNode);
#else
				auto _object = dynamic_cast<agtk::Object *>(_objectNode);
#endif
				_object->playSeObject(getPlaySeId());
			}
			else {
				AudioManager::getInstance()->playSe(getPlaySeId());
			}
		}

		// 音声再生がON かつ 不正な音声IDでない場合
		if (getIsPlayVoice() && getPlayVoiceId() != VOICE_ID_NONE) {
			if (_objectNode != nullptr) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto _object = static_cast<agtk::Object *>(_objectNode);
#else
				auto _object = dynamic_cast<agtk::Object *>(_objectNode);
#endif
				_object->playVoiceObject(getPlayVoiceId());
			}
			else {
				AudioManager::getInstance()->playVoice(getPlayVoiceId());
			}
		}
	}
}

/**
 * 更新
 * @param	dt	前フレームからの経過時間
 */
void Particle::update(float dt)
{
	if (getPaused()){
		return;
	}

	// パーティクルを更新
	ParticleSystemQuad::update(dt);
	
	// 衝突判定を行う かつ パーティクルがある場合
	if (getIsCheck() && _particleCount > 0) {

#ifdef HIGH_DEFINITION_COLLISIION_CHECK
		// 衝突判定
		if (checkCollision(dt)) {
			// 衝突して座標が更新されたらParticleQuadsを再更新する
			updateParticleQuads();
		}
#else
		// ==================================================================
		// ここからパーティクル毎の衝突判定
		// ==================================================================
		auto projectData = GameManager::getInstance()->getProjectData();
		auto scene = GameManager::getInstance()->getCurrentScene();
		auto sceneLayer = scene->getSceneLayer(this->getSceneLayerId());
		if (sceneLayer == nullptr) {
			CC_ASSERT(0);
			return;
		}
		auto objectList = sceneLayer->getObjectList();
		cocos2d::__Array *tileList = cocos2d::__Array::create();

		// パーティクルのバウンディング取得
		Point boundMin, boundMax;
		getParticleBound(scene->getSceneSize(), &boundMin, &boundMax);

		// 衝突する可能性のあるタイルリストを取得
		auto tiles = sceneLayer->getCollisionTileList(boundMin, boundMax);
		if (nullptr != tiles && tiles->count() > 0) {
			for (int i = 0, max = tiles->count(); i < max; i++) {
				tileList->addObject(tiles->getObjectAtIndex(i));
			}
		}

		// オブジェクトに対して衝突判定と処理
		cocos2d::Ref * el = nullptr;
		CCARRAY_FOREACH(objectList, el) {
			auto obj = dynamic_cast<Object *>(el);

			if (obj) {
				// オブジェクトの壁判定を取得
				auto node = obj->getAreaNode(agtk::data::TimelineInfoData::kTimelineWall);

				// 壁判定を持っている場合
				if (node) {
					// 衝突処理
					this->calcCollision(node, obj->getObjectMovement(), 0x0f, dt);
				}
			}
		}

		// タイルに対して衝突判定と処理
		CCARRAY_FOREACH(tileList, el) {
			auto tile = dynamic_cast<Tile *>(el);

			if (tile) {

				auto tileset = projectData->getTilesetData(tile->getTilesetId());
				int tileId = tile->getY() * tileset->getHorzTileCount() + tile->getX();
				int wallBit = tileset->getWallSetting(tileId);

				// 壁判定がある場合
				if (wallBit) {

					// 衝突処理
					this->calcCollision(tile, nullptr, wallBit, dt);
				}
			}
		}

		updateParticleQuads();
#endif
	}
}

/**
 * パーティクルの初期発生処理（事前発生、同時発生）
 */
void Particle::procInitialEmit()
{
	auto lDuration = _duration;
	_duration = DURATION_INFINITY;
	this->update(0);	//リセット前のパーティクルを一掃する。
	auto lPaused = ParticleSystem::isPaused();
	if (lPaused) {
		resumeEmissions();
	}
	if (_instantEmit) {
		//一気に最大数を出現させる。
		auto dt = 0.00001f;
		auto lEmissionRate = _emissionRate;
		_emissionRate = _totalParticles / dt;
		this->update(dt);
		_emissionRate = getLoop() ? lEmissionRate : 0.00001f;
	} else if (isActuallyPreviousEmit()) {
		//最大数出現させることで安定した状態のところまで進める。
		auto dt = 1.0f / _emissionRate;
		for (int i = 0; i < _totalParticles; i++) {
			this->update(dt);
		}
	}
	_duration = lDuration;
	_elapsed = 0;
	if (lPaused) {
		pauseEmissions();
	}
}

/**
 * エンター
 * @note ParticleSystem の onEnter 内で強制的にスケジューリングされるのを解除するためオーバーライド
 */
void Particle::onEnter()
{
	ParticleSystemQuad::onEnter();

	this->unscheduleUpdate();
}

/**
 * パーティクルのバウンディング取得
 * @param	sceneSize	シーンサイズ
 * @param	boundMin	バウンディング最小値(戻り値)
 * @param	boundMax	バウンディング最大値(戻り値)
 */
void Particle::getParticleBound(cocos2d::Vec2 sceneSize, cocos2d::Point *boundMin, cocos2d::Point *boundMax)
{
	boundMin->x = FLT_MAX;
	boundMin->y = FLT_MAX;
	boundMax->x = FLT_MIN;
	boundMax->y = FLT_MIN;

	// パーティクルの中で一番左下に存在するものと一番右上に存在するものを算出
	// ※パーティクルのサイズ(円)を考慮
	for (int i = 0; i < _particleCount; i++) {

		Point pos = Point(_particleData.startPosX[i] + _particleData.posx[i], _particleData.startPosY[i] + _particleData.posy[i]);
		auto particleSize = _particleData.size[i];

		// 画面外のパーティクルは除外
		if (pos.x < 0 || pos.y < 0 || pos.x > sceneSize.x || pos.y > sceneSize.y) {
			continue;
		}

		boundMin->x = min(pos.x - particleSize, boundMin->x);
		boundMin->y = min(pos.y - particleSize, boundMin->y);
		boundMax->x = max(pos.x + particleSize, boundMax->x);
		boundMax->y = max(pos.y + particleSize, boundMax->y);
	}
}

/**
* パーティクルの衝突判定
* @param	dt	前フレームからのデルタタイム
* @return		True:パーティクルの座標変更がある / False:パーティクルの座標変更が無い
*/
bool Particle::checkCollision(float dt)
{
	// 戻り値
	bool isModifiedPos = false;

	// ワーク変数
	auto scene = GameManager::getInstance()->getCurrentScene();
	auto sceneSize = scene->getSceneSize();
	auto sceneLayerId = this->getSceneLayerId();
	auto sceneLayer = (this->getSceneId() == agtk::data::SceneData::kMenuSceneId) ? scene->getMenuLayer(sceneLayerId) : scene->getSceneLayer(sceneLayerId);
	if (sceneLayer == nullptr) {
		CC_ASSERT(0);
		return false;
	}
	auto sceneData = sceneLayer->getSceneData();
	auto objectList = sceneLayer->getObjectList();

	// パーティクルの数分回す
	for (int i = 0; i < _particleCount; i++) {

		// パーティクルのベースポジション
		auto basePos = Point(_particleData.startPosX[i], _particleData.startPosY[i]);

		// パーティクルの現在位置と前フレームの位置とそのベクトルを算出
		auto particlePos = Point(basePos.x + _particleData.posx[i], basePos.y + _particleData.posy[i]);

		// シーン外に飛んでいったパーティクルの場合
		if (particlePos.x < 0 || particlePos.y < 0 || particlePos.x > sceneSize.x || particlePos.y > sceneSize.y) {
			// 何もしない
			continue;
		}

		auto particlePrePos = Point(basePos.x + _particleData.prePosX[i], basePos.y + _particleData.prePosY[i]);

		// パーティクルの前フレームの位置と現在位置とのバウンディングボックスを生成
		auto particleRadius = _particleData.size[i] * 0.5f;
		auto boundMin = Point(min(particlePos.x - particleRadius, particlePrePos.x - particleRadius),
								min(particlePos.y - particleRadius, particlePrePos.y - particleRadius));
		auto boundMax = Point(max(particlePos.x + particleRadius, particlePrePos.x + particleRadius),
								max(particlePos.y + particleRadius, particlePrePos.y + particleRadius));

		// バウンディングボックスが画面外のパーティクルの場合
		if (boundMax.x < 0 || boundMax.y < 0 || boundMin.x > sceneSize.x || boundMin.y > sceneSize.y) {
			// スキップ
			continue;
		}

		// パーティクルの移動線分生成
		auto particleVec = Vec2(particlePos.x - particlePrePos.x, particlePos.y - particlePrePos.y); //移動分のベクトル
		auto particleLength = particleVec.getLength();//移動分のベクトル長
		auto particleNormalizedVec = particleVec.getNormalized();//移動分の単位ベクトル

		// パーティクルの速度ベクトル取得
		cocos2d::Vec2 particleSpeed = (_particleData.isModeA[i] ? Vec2(_particleData.modeA.dirX[i], _particleData.modeA.dirY[i]) : (particlePos - particlePrePos) * (1.0f / dt));

		// パーティクルを前フレームの位置から現在の位置へ少しずつ進めていく
		bool isHit = false;
		for (float v = 0; v < particleLength && !isHit; v += 1.0f) {
			auto pos = particlePrePos + (particleNormalizedVec * v);
			CollisionLine particleSeqment = { &particlePrePos, &pos };

			// -------------------------------------------------------
			// オブジェクトとの衝突チェック
			// -------------------------------------------------------
			cocos2d::Ref * el = nullptr;
			CCARRAY_FOREACH(objectList, el) {
				auto obj = dynamic_cast<Object *>(el);

				// オブジェクトが存在 かつ Disabled でない場合
				if (obj && !obj->getDisabled()) {
					// オブジェクトの壁判定を取得
					auto node = obj->getAreaNode(agtk::data::TimelineInfoData::kTimelineWall);

					// 壁判定を持っている場合
					if (node) {
						// 衝突チェック
						cocos2d::Vec2 crossPos = Vec2::ZERO;
						cocos2d::Vec2 reflection = Vec2::ZERO;
						CollisionCorner objCorner = CollisionUtils::getCorner(node);
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
						if (CollisionUtils::checkCircleHitSegmentAndGetIntersectionAndReflection(objCorner.lines(), &particleSeqment, 0x0f, _particleData.size[i], _repulsion, &particleSpeed, &crossPos, &reflection)) {
#else
#endif
							// パーティクルデータを衝突後の状態へ更新
							updateParticleData(i, crossPos, reflection);
							isHit = true;
							isModifiedPos = true;
						}
					}
				}
			}

			// -------------------------------------------------------
			// 坂との衝突チェック
			// -------------------------------------------------------
			auto slopes = sceneLayer->getCollisionSlopeList(boundMin, boundMax, particleVec);
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
			AutoDeleter<MtVector<Slope *>> deleter(slopes);
#endif
			if (nullptr != slopes && slopes->count() > 0) {
				cocos2d::Ref * ref = nullptr;
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
				for(int i = 0; i < (int)slopes->size(); i++){
					auto slope = (*slopes)[i];
#else
				CCARRAY_FOREACH(slopes, ref) {
					auto slope = dynamic_cast<Slope *>(ref);
#endif
					if (slope) {
						// 上から通過可能か
						auto isPassFromUpper = slope->getSlopeData()->getPassableFromUpper();
						// 下から通過可能か
						auto isPassFromLower = slope->getSlopeData()->getPassableFromLower();
						// 坂の線分を生成
						cocos2d::Point slopeStartPoint = cocos2d::Point(agtk::Scene::getPositionSceneFromCocos2d(slope->start, sceneData));
						cocos2d::Point slopeEndPoint = cocos2d::Point(agtk::Scene::getPositionSceneFromCocos2d(slope->end, sceneData));
						CollisionLine slopeLine = { &slopeStartPoint, &slopeEndPoint, 0x0F };
						Vec2 slopeNormalizedVec = (slopeEndPoint - slopeStartPoint).getNormalized();
						Vec2 slopeNormalVec = Vec2(-slopeNormalizedVec.y, slopeNormalizedVec.x);
						
						std::vector<CollisionLine> slopeSeqments;
						slopeSeqments.push_back(slopeLine);

						// 衝突チェック
						cocos2d::Vec2 crossPos = Vec2::ZERO;
						cocos2d::Vec2 reflection = Vec2::ZERO;
						if (CollisionUtils::checkCircleHitSegmentAndGetIntersectionAndReflection(slopeSeqments, &particleSeqment, 0x0F, _particleData.size[i], _repulsion, &particleSpeed, &crossPos, &reflection, false)) {

							// パーティクルの移動線分ベクトルと坂の法線との内積
							auto dot = Vec2::dot(particleVec, slopeNormalVec);

							// 下から通過可能で下から衝突した or 上から通過可能で上から衝突した場合
							if (dot > 0 && isPassFromLower || dot < 0 && isPassFromUpper) {
								// スキップ
								continue;
							}

							// パーティクルデータを衝突後の状態へ更新
							updateParticleData(i, crossPos, reflection);
							isHit = true;
							isModifiedPos = true;
							break;
						}
					}
				}
			}

			// -------------------------------------------------------
			// タイルとの衝突チェック
			// -------------------------------------------------------
			// 衝突する可能性のあるタイルリストを取得
			auto tiles = sceneLayer->getCollisionTileList(boundMin, boundMax);
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
			if(tiles.size() > 0){
#else
			if (nullptr != tiles && tiles->count() > 0) {
#endif
				auto projectData = GameManager::getInstance()->getProjectData();

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
				for(auto tile: tiles){
#else
				cocos2d::Ref * ref = nullptr;
				CCARRAY_FOREACH(tiles, ref) {
					auto tile = dynamic_cast<Tile *>(ref);
#endif

					if (tile && tile->getType() == agtk::Tile::EnumType::kTypeTile) {
						auto tileset = projectData->getTilesetData(tile->getTilesetId());
						int tileId = tile->getY() * tileset->getHorzTileCount() + tile->getX();
						int wallBit = tileset->getWallSetting(tileId);

						// 壁判定がある場合
						if (wallBit) {
							// 衝突チェック
							cocos2d::Vec2 crossPos = Vec2::ZERO;
							cocos2d::Vec2 reflection = Vec2::ZERO;
							CollisionCorner objCorner = CollisionUtils::getCorner(tile);
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
							if (CollisionUtils::checkCircleHitSegmentAndGetIntersectionAndReflection(objCorner.lines(), &particleSeqment, wallBit, _particleData.size[i], _repulsion, &particleSpeed, &crossPos, &reflection)) {
#else
#endif
								// パーティクルデータを衝突後の状態へ更新
								updateParticleData(i, crossPos, reflection);
								isHit = true;
								isModifiedPos = true;
							}
						}
					}
				}
			}
		}
	}

	return isModifiedPos;
}


/**
 * パーティクルの衝突処理
 * @param	target			対象ノード
 * @param	objectMovement	対象ノードがオブジェクトの場合のオブジェクト移動管理クラス
 * @param	wallBit			壁ビット値
 * @param	dt				前フレームからの経過時間
 * @note	より良い方法が無いか検討の必要あり
 */
void Particle::calcCollision(cocos2d::Node *target, ObjectMovement *objectMovement, int wallBit, float dt)
{
	// ワーク変数
	auto scene = GameManager::getInstance()->getCurrentScene();
	auto sceneSize = scene->getSceneSize();
	CollisionCorner objCorner = CollisionUtils::getCorner(target);
	cocos2d::Point particlePos = Point::ZERO;
	cocos2d::Point particlePrePos = Point::ZERO;

	// パーティクルの数分回す
	for (int i = 0; i < _particleCount; i++) {

		cocos2d::Vec2 basePos = Vec2(_particleData.startPosX[i], _particleData.startPosY[i]);

		// 個々のパーティクルの速度ベクトルと前フレームの位置から現在位置への線分を算出
		cocos2d::Point particlePos = cocos2d::Point(basePos.x + _particleData.posx[i], basePos.y + _particleData.posy[i]);

		// 画面外のパーティクルは除外
		if (particlePos.x < 0 || particlePos.y < 0 || particlePos.x > sceneSize.x || particlePos.y > sceneSize.y) {
			continue;
		}

		cocos2d::Point particlePrePos = cocos2d::Point(basePos.x + _particleData.prePosX[i], basePos.y + _particleData.prePosY[i]);
		CollisionLine particleLine = { &particlePrePos, &particlePos };
		cocos2d::Vec2 particleSpeed = (_particleData.isModeA[i] ? Vec2(_particleData.modeA.dirX[i], _particleData.modeA.dirY[i]) : (particlePos - particlePrePos) * (1.0f / dt));

		cocos2d::Vec2 crossPos = Vec2::ZERO;
		cocos2d::Vec2 reflection = Vec2::ZERO;

		// 壁とパーティクルが衝突している場合
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
		if (CollisionUtils::checkCircleHitSegmentAndGetIntersectionAndReflection(objCorner.lines(), &particleLine, wallBit, _particleData.size[i], _repulsion, &particleSpeed, &crossPos, &reflection)) {
#else
#endif
			// パーティクルのデータを更新
			updateParticleData(i, crossPos, reflection);
		}
	}
}

/**
 * パーティクルデータ更新
 * @param	idx				パーティクルデータのIDX
 * @param	crossPos		交点
 */
void Particle::updateParticleData(int idx, const cocos2d::Vec2 &crossPos, const cocos2d::Vec2 &reflection)
{
	// 座標を更新
	_particleData.posx[idx] = crossPos.x - _particleData.startPosX[idx];
	_particleData.posy[idx] = crossPos.y - _particleData.startPosY[idx];

	// 衝突可能回数をデクリメント
	_particleData.collisionCount[idx]--;

	// 重力モードの場合
	if (_particleData.isModeA[idx]) {
		// 反射ベクトルをパーティクルの方向ベクトルへ代入
		_particleData.modeA.dirX[idx] = _touchBound ? reflection.x : 0.0f;
		_particleData.modeA.dirY[idx] = _touchBound ? reflection.y : 0.0f;
	}
	// 円モードの場合
	else {
		// 角度と半径も更新
		auto particlePosVec = cocos2d::Vec2(_particleData.posx[idx], _particleData.posy[idx]);
		auto deg = CC_RADIANS_TO_DEGREES(atan2f(particlePosVec.y, particlePosVec.x));
		_particleData.modeB.angle[idx] = CC_DEGREES_TO_RADIANS((deg < 0 ? deg - 180.0f : deg + 180.0f));
		_particleData.modeB.radius[idx] = particlePosVec.length();

#ifdef USE_RADIUS_REFLECTION_FLEE
		// 回転モードから重力モードへ変更する
		_particleData.isModeA[idx] = true;
		_particleData.modeA.radialAccel[idx] = 0;
		_particleData.modeA.tangentialAccel[idx] = 0;

		// 反射ベクトルをパーティクルの方向ベクトルへ代入
		_particleData.modeA.dirX[idx] = (_touchBound ? reflection.x : 0.0f);
		_particleData.modeA.dirY[idx] = (_touchBound ? reflection.y : 0.0f);
#else
		// 反射するので角速度を反転させる
		_particleData.modeB.degreesPerSecond[idx] *= _touchBound ? -_repulsion : 0.0f;
#endif
	}

	// 消える場合
	if (getTouchDisappear() && _particleData.collisionCount[idx] <= 0) {
		// ライフタイムを0に設定
		_particleData.timeToLive[idx] = 0.0f;
	}
}

/**
* パーティクルの削除
*/
void Particle::deleteParticle()
{
	this->setVisible(false);

	// パーティクルの数分回す
	for (int i = 0; i < _particleCount; i++) {
		// ライフタイムを0に設定
		_particleData.timeToLive[i] = 0.0f;
	}
}

void Particle::setAlpha(float alpha)
{
	_alpha = alpha;
	auto animeParticleData = this->getAnimeParticleData();
	if (animeParticleData) {
		// ----------------------------------------------------
		// パーティクルカラー
		// ----------------------------------------------------
		// 開始カラー
		_startColor.r = alpha * animeParticleData->getStartR() / 255.0f;
		_startColor.g = alpha * animeParticleData->getStartG() / 255.0f;
		_startColor.b = alpha * animeParticleData->getStartB() / 255.0f;
		_startColor.a = alpha * animeParticleData->getStartAlpha() / 255.0f;
		_startColorVar.r = alpha * animeParticleData->getStartRDispersion() / 255.0f;
		_startColorVar.g = alpha * animeParticleData->getStartGDispersion() / 255.0f;
		_startColorVar.b = alpha * animeParticleData->getStartBDispersion() / 255.0f;
		_startColorVar.a = alpha * animeParticleData->getStartAlphaDispersion() / 255.0f;
		// 中間カラー
		_middleColor.r = alpha * animeParticleData->getMiddleR() / 255.0f;
		_middleColor.g = alpha * animeParticleData->getMiddleG() / 255.0f;
		_middleColor.b = alpha * animeParticleData->getMiddleB() / 255.0f;
		_middleColor.a = alpha * animeParticleData->getMiddleAlpha() / 255.0f;
		_middleColorVar.r = alpha * animeParticleData->getMiddleRDispersion() / 255.0f;
		_middleColorVar.g = alpha * animeParticleData->getMiddleGDispersion() / 255.0f;
		_middleColorVar.b = alpha * animeParticleData->getMiddleBDispersion() / 255.0f;
		_middleColorVar.a = alpha * animeParticleData->getMiddleAlphaDispersion() / 255.0f;
		// 最終カラー
		_endColor.r = alpha * animeParticleData->getEndR() / 255.0f;
		_endColor.g = alpha * animeParticleData->getEndG() / 255.0f;
		_endColor.b = alpha * animeParticleData->getEndB() / 255.0f;
		_endColor.a = alpha * animeParticleData->getEndAlpha() / 255.0f;
		_endColorVar.r = alpha * animeParticleData->getEndRDispersion() / 255.0f;
		_endColorVar.g = alpha * animeParticleData->getEndGDispersion() / 255.0f;
		_endColorVar.b = alpha * animeParticleData->getEndBDispersion() / 255.0f;
		_endColorVar.a = alpha * animeParticleData->getEndAlphaDispersion() / 255.0f;
	}
}

#ifdef USE_REDUCE_RENDER_TEXTURE
void Particle::visit(Renderer *renderer, const Mat4& parentTransform, uint32_t parentFlags)
{
	Node::visit(renderer, parentTransform, parentFlags);
}
#endif

//実際に事前発生が有効の場合に真を返す。
//ループでない or 同時発生 の場合、事前発生は有効でなくなる。
bool Particle::isActuallyPreviousEmit()
{
	if (!_loop || _instantEmit) {
		return false;
	}
	return _previousEmit;
}

//-------------------------------------------------------------------------------------------------------------------
//! パーティクルグループクラス
//-------------------------------------------------------------------------------------------------------------------
/**
 * コンストラクタ
 */
ParticleGroup::ParticleGroup()
{
	_id = -1;
	_layerList = nullptr;
	_duration300 = 0;
	_isDurationUnlimited = false;
	_sceneLayerId = -1;
	_followTarget = nullptr;
	_followAdjustPos = cocos2d::Vec2::ZERO;
	_shaderStateAfterimage = nullptr;
	_targetObjectBackside = false;
	_pauseEmissionFlag = false;
	_sceneData = nullptr;
#ifdef USE_REDUCE_RENDER_TEXTURE
	_forceBack = false;
#endif
}

/**
 * デストラクタ
 */
ParticleGroup::~ParticleGroup()
{
	cocos2d::Ref *ref = nullptr;
	CCARRAY_FOREACH(_layerList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto particle = static_cast<agtk::Particle *>(ref);
#else
		auto particle = dynamic_cast<agtk::Particle *>(ref);
#endif
		particle->removeFromParent();
	}
	_layerList->removeAllObjects();

	CC_SAFE_RELEASE_NULL(_layerList);
	CC_SAFE_RELEASE_NULL(_followTarget);
	CC_SAFE_RELEASE_NULL(_shaderStateAfterimage);
	CC_SAFE_RELEASE_NULL(_sceneData);
}

/**
 * 生成
 * @param	followTarget		追従対象
 * @param	sceneId				表示するシーンID
 * @param	sceneLayerId		表示するシーンレイヤーID
 * @param	particleId			パーティクルID
 * @param	pos					生成位置
 * @param	connectionId		接続点ID
 * @param	duration300			再生継続時間
 * @param	durationUnlimited	再生継続の無制限フラグ
 * @param	particleDataList	パーティクルデータリスト
 * @return						パーティクルグループのインスタンス
 */
#ifdef USE_REDUCE_RENDER_TEXTURE
ParticleGroup* ParticleGroup::create(cocos2d::Node *followTarget, int sceneId, int sceneLayerId, int particleId, const cocos2d::Vec2 pos, int connectionId, float duration300, bool durationUnlimited, const cocos2d::__Dictionary * particleDataList, bool bForceBack, cocos2d::Node* objectNode)
#else
ParticleGroup* ParticleGroup::create(cocos2d::Node *followTarget, int sceneId, int sceneLayerId, int particleId, const cocos2d::Vec2 pos, int connectionId, float duration300, bool durationUnlimited, const cocos2d::__Dictionary * particleDataList, cocos2d::Node* objectNode)
#endif
{
	ParticleGroup* ret = new (std::nothrow) ParticleGroup();
#ifdef USE_REDUCE_RENDER_TEXTURE
	if (ret && ret->init(followTarget, sceneId, sceneLayerId, particleId, pos, connectionId, duration300, durationUnlimited, particleDataList, bForceBack, objectNode))
#else
	if (ret && ret->init(followTarget, sceneId, sceneLayerId, particleId, pos, connectionId, duration300, durationUnlimited, particleDataList, objectNode))
#endif
	{
		ret->autorelease();
	}
	else
	{
		CC_SAFE_DELETE(ret);
	}
	return ret;
}


/**
 * 初期化
 * @param	followTarget		追従対象
 * @param	sceneId				表示するシーンID
 * @param	sceneLayerId		表示するシーンレイヤーID
 * @param	particleId			パーティクルID
 * @param	pos					生成補正位置
 * @param	connectionId		接続点ID
 * @param	duration300			再生継続時間
 * @param	durationUnlimited	再生継続の無制限フラグ
 * @param	particleDataList	パーティクルデータリスト
 * @return						初期化の成否
 */
#ifdef USE_REDUCE_RENDER_TEXTURE
bool ParticleGroup::init(cocos2d::Node *followTarget, int sceneId, int sceneLayerId, int particleId, const cocos2d::Vec2 pos, int connectionId, float duration300, bool durationUnlimited, const cocos2d::__Dictionary * particleDataList, bool bForceBack, cocos2d::Node* objectNode)
#else
bool ParticleGroup::init(cocos2d::Node *followTarget, int sceneId, int sceneLayerId, int particleId, const cocos2d::Vec2 pos, int connectionId, float duration300, bool durationUnlimited, const cocos2d::__Dictionary * particleDataList, cocos2d::Node* objectNode)
#endif
{
	auto scene = GameManager::getInstance()->getCurrentScene();
	// 配置対象のシーンレイヤー取得
	auto sceneLayer = sceneId == agtk::data::SceneData::kMenuSceneId ? scene->getMenuLayer(sceneLayerId) : scene->getSceneLayer(sceneLayerId);

#ifdef USE_REDUCE_RENDER_TEXTURE
	this->setForceBack(bForceBack);
#endif
	// シーンデータ設定
	auto sceneData = sceneLayer->getSceneData();
	this->setSceneData(sceneData);

	// パーティクルID設定
	this->setId(particleId);

	// 再生継続時間設定
	this->setDuration300(duration300);

	// 再生継続時間の無制限フラグ設定
	this->setIsDurationUnlimited(durationUnlimited);

	// 補正位置設定
	this->setFollowAdjustPos(pos);

	// 接続点タイプフラグ保持
	this->setConnectionId(connectionId);

	// 座標オフセット設定
	this->setPositionOffset(Vec2::ZERO);

	// 初期パーティクル座標設定
	auto firstParticlePos = cocos2d::Vec2::ZERO;

	// 補正位置を計算用に取得
	auto adjustPos = _followAdjustPos;

	// 追従対象がある場合
#ifdef USE_REDUCE_RENDER_TEXTURE
	if (followTarget && !_forceBack) {
#else
	if (followTarget) {
#endif
		// 追従対象を保持
		this->setFollowTarget(followTarget);

		// 追従対象の座標(Scene座標系)を取得
		firstParticlePos = _followTarget->getPosition();

		// 接続点がある場合
		if (_connectionId > 0) {
			// 接続点を取得
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto obj = static_cast<agtk::Object *>(_followTarget);
#else
			auto obj = dynamic_cast<agtk::Object *>(_followTarget);
#endif
			agtk::Vertex4 vertex4;
			auto existsConnect = obj->getTimeline(agtk::data::TimelineInfoData::kTimelineConnection, _connectionId, vertex4);
			if (existsConnect) {
				adjustPos = Scene::getPositionSceneFromCocos2d(vertex4.addr()[0]);
			}
			adjustPos -= firstParticlePos;
		}
	}
	
	firstParticlePos = scene->getPositionCocos2dFromScene(firstParticlePos + adjustPos, sceneData);

	// 所属するシーンID保持
	this->setSceneId(sceneId);

	// 所属するシーンレイヤーID保持
	this->setSceneLayerId(sceneLayerId);

	// レイヤーリスト生成
	this->setLayerList(cocos2d::__Array::create());

	// パーティクル画像リスト取得
	auto animParticleImageList = GameManager::getInstance()->getProjectData()->getAnimParticleImageList();
	
	// レイヤー毎にパーティクルを生成
	//若いレイヤーを手前に表示させるため逆順に生成させる。
	std::vector<agtk::data::AnimeParticleData *> list;
	cocos2d::DictElement *el = nullptr;
	CCDICT_FOREACH(particleDataList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto particleData = static_cast<agtk::data::AnimeParticleData *>(el->getObject());
#else
		auto particleData = dynamic_cast<agtk::data::AnimeParticleData *>(el->getObject());
#endif
		list.insert(list.begin(), particleData);
	}
	for(auto particleData: list){
		auto seed = rand();
		auto particle = agtk::Particle::create(seed, particleData, animParticleImageList, objectNode);
		particle->setId(this->getLayerList()->count());
		particle->setSceneId(sceneId);
		particle->setSceneLayerId(sceneLayerId);
		particle->setPosition(firstParticlePos);
#ifdef USE_REDUCE_RENDER_TEXTURE
#else
		if (followTarget != nullptr) {
			particle->setZOrder(followTarget->getLocalZOrder());
		}
#endif
		particle->stop(true);
		this->getLayerList()->addObject(particle);
#ifdef USE_REDUCE_RENDER_TEXTURE
		auto isBlendAdditive = particle->isBlendAdditive();
		if (_forceBack) {
			//裏側を強制の場合は、オブジェクトの奥側に追加。
			followTarget->addChild(particle, Object::kPartPriorityBackParticle);
		} else
		if (followTarget && !isBlendAdditive) {
			//オブジェクト指定ありで、加算モードでない場合は、オブジェクトの手前側に追加。
			followTarget->addChild(particle, Object::kPartPriorityFrontParticle);
		}
		else if (isBlendAdditive) {

			auto isBackside = [&](cocos2d::Node *node)->bool {
				bool flag = false;
				auto object = dynamic_cast<agtk::Object *>(_followTarget);
				if (object) {
					auto player = object->getPlayer();
					if (player && _connectionId >= 0) {
						//オブジェクトの接続点に紐付けられている場合に、表側か裏側かに合わせて、表示優先度を変える。
						flag = player->getTimelineBackside(_connectionId);
					}
				}
				return flag;
			} (followTarget);

			//加算モードならシーンレイヤー（の加算パーティクル用Node）に追加。
			if (isBackside) {
				sceneLayer->getAdditiveParticleBacksideNode()->addChild(particle);
			} else {
				sceneLayer->getAdditiveParticleNode()->addChild(particle);
			}
			//加算モードならシーンレイヤー（の加算パーティクル用Node）に追加。
			//sceneLayer->getAdditiveParticleNode()->addChild(particle);
		} else {
			//いずれでもなければ、どのオブジェクトより手前に追加。
			sceneLayer->getObjectFrontNode()->addChild(particle);
		}
#else
		sceneLayer->addChild(particle);
#endif
		particle->play();
	}
#ifdef USE_REDUCE_RENDER_TEXTURE
	updateBackside();
#endif

	return true;
}

/**
 * 更新
 * @param	dt	前フレームからの経過時間
 */
void ParticleGroup::update(float dt)
{
	// 座標更新
	auto scene = GameManager::getInstance()->getCurrentScene();
	cocos2d::Vec2 newPos = _followAdjustPos;
	cocos2d::Vec2 followPos = Vec2::ZERO;
	bool isPauseEmission = false;
	auto sceneData = this->getSceneData();

#ifdef USE_REDUCE_RENDER_TEXTURE
	// 追従対象があり、背面強制でない場合
	if (_followTarget && !_forceBack) {
#else
	// 追従対象がある場合
	if (_followTarget) {
#endif
		followPos = _followTarget->getPosition();

		// 接続点がある場合
		if (_connectionId > 0) {
			// 接続点を取得
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto obj = static_cast<agtk::Object *>(_followTarget);
#else
			auto obj = dynamic_cast<agtk::Object *>(_followTarget);
#endif

			auto player = obj->getPlayer();
			bool valid = (player) ? player->getTimelineValid(_connectionId) : true;

			agtk::Vertex4 vertex4;
			auto existsConnect = obj->getTimeline(agtk::data::TimelineInfoData::kTimelineConnection, _connectionId, vertex4);
			if(existsConnect) {
				newPos = Scene::getPositionSceneFromCocos2d(vertex4.addr()[0], sceneData);
			}
			newPos -= followPos;

			// 接続点が無い場合、または接続点が存在しない場合。
			if (existsConnect == false || valid == false) {
				// 一時的にパーティクル生成を停止にする
				isPauseEmission = true;
			}
		}
	}
	else {
		// 一時的にパーティクル生成を停止にする
		isPauseEmission = this->getPauseEmissionFlag();
	}

	newPos = scene->getPositionCocos2dFromScene(followPos + newPos, sceneData);

	// 再生継続時間無制限でない場合
	if (!_isDurationUnlimited) {

		// 再生継続時間をデクリメント
		_duration300 -= (dt * 300);
		if (_duration300 < 0) _duration300 = 0;
	}

	// レイヤーリストが存在する場合
	if (nullptr != _layerList && _layerList->count() > 0) {
		cocos2d::Ref * ref = nullptr;
		CCARRAY_FOREACH(_layerList, ref) {
			auto particle = dynamic_cast<Particle *>(ref);
			bool particleStop = false;

			if (nullptr != particle) {
				// 再生中のパーティクルが再生継続出来なくなった場合
				if (!_isDurationUnlimited && _duration300 <= 0) {
					particle->stop(false);
					particleStop = true;
				}

				if (isPauseEmission && !particle->isPaused()) {
					particle->pauseEmissions();
				}
				else if (!isPauseEmission && particle->isPaused()) {
					particle->resumeEmissions();
				}

				particle->setPosition(newPos);
				particle->update(dt);

				if (particleStop || (!particle->isActive() && !particle->getStopped() && particle->isFinish())) {
					particle->setStopped(true);
				}
			}
		}
	}
#ifdef USE_REDUCE_RENDER_TEXTURE
	//updateBackside();	//旧プレイヤーに動作を合わせるため、動的な表示優先順位の更新を行わない。
#endif
}

#ifdef USE_REDUCE_RENDER_TEXTURE
void ParticleGroup::updateBackside()
{
	if (_forceBack) {
		//奥側強制なら何もしなくてOK。
		return;
	}
	//「画像の裏側に設定」
	auto object = dynamic_cast<agtk::Object *>(_followTarget);
	if (object) {
		auto player = object->getPlayer();
		if (player && _connectionId >= 0) {
			//オブジェクトの接続点に紐付けられている場合に、表側か裏側かに合わせて、表示優先度を変える。
			auto newBackside = player->getTimelineBackside(_connectionId);
			if (newBackside != _targetObjectBackside) {
				this->setTargetObjctBackside(newBackside);
				if (nullptr != _layerList && _layerList->count() > 0) {
					cocos2d::Ref * ref = nullptr;
					CCARRAY_FOREACH(_layerList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
						auto particle = static_cast<Particle *>(ref);
#else
						auto particle = dynamic_cast<Particle *>(ref);
#endif
						if (!particle->isBlendAdditive()) {
							particle->setLocalZOrder(newBackside ? agtk::Object::kPartPriorityBackParticle : agtk::Object::kPartPriorityFrontParticle);
						}
					}
				}
			}
			this->setTargetObjctBackside(player->getTimelineBackside(_connectionId));
		}
	}
}
#endif

/**
 * パーティクル処理の変更
 * @param	procType	処理タイプ
 */
void ParticleGroup::changeProccess(PARTICLE_PROC_TYPE procType, bool isReset)
{
	if (nullptr != _layerList && _layerList->count() > 0) {
		cocos2d::Ref * ref = nullptr;
		CCARRAY_FOREACH(_layerList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto particle = static_cast<Particle *>(ref);
#else
			auto particle = dynamic_cast<Particle *>(ref);
#endif

			// 処理タイプ別
			switch (procType)
			{
			case agtk::ParticleGroup::PLAY:
				particle->play();
				break;
			case agtk::ParticleGroup::STOP:
				particle->stop(isReset);
				break;
			case agtk::ParticleGroup::PAUSE:
				particle->pause();
				break;
			case agtk::ParticleGroup::RESET:
				particle->reset();
				break;
			default:
				break;
			}
		}
	}
}

/**
 * 停止しているか？
 * @return	True: 停止中 / False: 動作中
 */
bool ParticleGroup::isStopped()
{
	bool result = true;

	// レイヤーリストが存在する場合
	if (nullptr != _layerList && _layerList->count() > 0) {
		cocos2d::Ref * ref = nullptr;
		CCARRAY_FOREACH(_layerList, ref) {
			auto particle = dynamic_cast<Particle *>(ref);

			if (nullptr != particle) {
				result &= particle->getStopped();
			}
		}
	}

	return result;
}

/**
 * Active(Emitterから発生している)か？
 * @return	True: 発生中 / False: 停止中
 */
bool ParticleGroup::isActive()
{
	bool result = false;

	// レイヤーリストが存在する場合
	if (nullptr != _layerList && _layerList->count() > 0) {
		cocos2d::Ref * ref = nullptr;
		CCARRAY_FOREACH(_layerList, ref) {
			auto particle = dynamic_cast<Particle *>(ref);

			if (nullptr != particle) {
				result |= particle->isActive();
			}
		}
	}

	return result;
}

/**
 * パーティクルが無限ループか？
 * @return	True:無限ループ / False:有限
 */
bool ParticleGroup::isUnlimited()
{
	bool isUnlimited = this->getIsDurationUnlimited();

	if (nullptr != _layerList && _layerList->count() > 0) {
		cocos2d::Ref * ref = nullptr;
		bool ret = false;
		CCARRAY_FOREACH(_layerList, ref) {
			auto particle = dynamic_cast<Particle *>(ref);

			if (nullptr != particle) {
				ret |= particle->getLoop();
			}
		}

		isUnlimited &= ret;
	}

	return isUnlimited;
}

/**
 * 削除
 */
void ParticleGroup::remove()
{
	if (nullptr != _layerList && _layerList->count() > 0) {
		cocos2d::Ref * ref = nullptr;
		CCARRAY_FOREACH(_layerList, ref) {
			auto particle = dynamic_cast<Particle *>(ref);
			if (nullptr != particle) {
				particle->removeFromParent();
			}
		}

		_layerList->removeAllObjects();
	}
}

/**
* 残像用シェーダの設定を行う
*/
void ParticleGroup::setShaderAfterimage()
{
	if (_shaderStateAfterimage == nullptr) {
		if (_layerList != nullptr && _layerList->count() > 0) {

			_shaderStateAfterimage = Shader::createShaderParticleAfterimage();

			cocos2d::Ref * ref = nullptr;
			CCARRAY_FOREACH(_layerList, ref) {
				auto particle = dynamic_cast<Particle *>(ref);

				if (nullptr != particle) {
					particle->setGLProgram(_shaderStateAfterimage->getGLProgram());
				}
			}
		}
	}
}


/**
* 残像用シェーダのカラーを設定
*/
void ParticleGroup::setAfterimageColor(cocos2d::Color4B color)
{
	if (_shaderStateAfterimage != nullptr) {
		auto program = _shaderStateAfterimage->getGLProgram();

		program->updateUniforms();
		program->setUniformLocationWith4f(program->getUniformLocationForName("RgbaColor"), (float)color.r / 255.0f, (float)color.g / 255.0f, (float)color.b / 255.0f, (float)color.a / 255.0f);
	}
}

/**
* 残像用シェーダのαを設定
*/
void ParticleGroup::setAfterimageAlpha(float alpha)
{
	if (_shaderStateAfterimage != nullptr) {
		auto program = _shaderStateAfterimage->getGLProgram();

		program->updateUniforms();
		program->setUniformLocationWith1f(program->getUniformLocationForName("Alpha"), alpha);
	}
}

/**
* パーティクルの削除
*/
void ParticleGroup::deleteParticle()
{
	if (_layerList != nullptr && _layerList->count() > 0) {

		if (nullptr != _shaderStateAfterimage) {
			CC_SAFE_RELEASE_NULL(_shaderStateAfterimage);
		}

		cocos2d::Ref * ref = nullptr;
		CCARRAY_FOREACH(_layerList, ref) {
			auto particle = dynamic_cast<Particle *>(ref);

			if (nullptr != particle) {
				particle->deleteParticle();
			}
		}
	}
}

#ifdef USE_REDUCE_RENDER_TEXTURE
void ParticleGroup::visit(Renderer *renderer, const Mat4& parentTransform, uint32_t parentFlags)
{
	//Node::visit(renderer, _parent->getNodeToParentTransform(), parentFlags);
	Node::visit(renderer, parentTransform, parentFlags);
}
#endif


NS_AGTK_END

