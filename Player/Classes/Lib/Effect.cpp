#include "Effect.h"
#include "GameManager.h"

NS_AGTK_BEGIN
//-------------------------------------------------------------------------------------------------------------------
EffectAnimation::EffectAnimation()
{
	_player = nullptr;
	_targetObject = nullptr;
	_offset = Vec2::ZERO;
	_targetObjectConnectId = -1;
	_duration300 = 0;
	_targetObjectBackside = false;

	_isDeletable = false;
	_isCheckDuration = false;
	_isStop = false;
	_alpha = 1.0f;
	_mainAlpha = 1.0f;
#ifdef USE_REDUCE_RENDER_TEXTURE
	_forceBack = false;
#endif
}

EffectAnimation::~EffectAnimation()
{
	CC_SAFE_RELEASE_NULL(_player);
	_targetObject = nullptr;
}

bool EffectAnimation::init(int sceneLayerId, agtk::data::AnimationData * animationData, int zOrder)
{
	_sceneLayerId = sceneLayerId;

	auto scene = GameManager::getInstance()->getCurrentScene();
	CCASSERT(scene, "シーン取得失敗");

	auto sceneLayer = scene->getSceneLayer(sceneLayerId);
	CCASSERT(sceneLayer, "シーンレイヤー取得失敗");

	// プレイヤーを生成
	auto player = agtk::Player::create(animationData);

	CCASSERT(player, "プレイヤー生成失敗");

	// Zオーダーを設定
	this->setLocalZOrder(zOrder);

	// プレイヤーを設定
	this->setPlayer(player);

	// 親子関係を設定する
	this->addChild(player);
#ifdef USE_REDUCE_RENDER_TEXTURE
	sceneLayer->getObjectFrontNode()->addChild(this);
#else
	sceneLayer->addChild(this);
#endif

	// 再生開始（actionNoとactionDirectNoは1で固定）
	_player->play(1, 1);

	return true;
}

#ifdef USE_REDUCE_RENDER_TEXTURE
bool EffectAnimation::init(agtk::Object *object, agtk::data::AnimationData * animationData, int zOrder, bool bForceBack)
{
	_forceBack = bForceBack;

	// ACT2-5033 実行アクションの「レイヤーを停止」で使用しているので登録しておく
	_sceneLayerId = object->getLayerId();

	// プレイヤーを生成
	auto player = agtk::Player::create(animationData);

	CCASSERT(player, "プレイヤー生成失敗");

	// Zオーダーを設定
	this->setLocalZOrder(zOrder);

	// プレイヤーを設定
	this->setPlayer(player);

	// 親子関係を設定する
	this->addChild(player);
	if (_forceBack) {
		this->setTargetObjctBackside(true);
		object->addChild(this, agtk::Object::kPartPriorityBackEffect);
	}
	else {
		object->addChild(this, agtk::Object::kPartPriorityFrontEffect);
	}

	// 再生開始（actionNoとactionDirectNoは1で固定）
	_player->play(1, 1);

	return true;
}
#endif

void EffectAnimation::update(float delta)
{
	// プレイヤーを生成できていない場合
	if (!_player) {
		// 処理しない
		_isDeletable = true;
		return;
	}

	// 座標の更新を行う
	updatePosition();

	// 生存時間をチェックする場合
	if (_isCheckDuration) {
		// 生存時間を減らす
		_duration300 -= delta * 300;

		// 生存時間が0になった場合
		if (_duration300 <= 0) {
			// 削除させる
			deleteEffect();
			return;
		}
	}
	// 生存時間をチェックしない場合
	else
	{
		// アニメーションの再生が完了した場合
		if (_player->getBasePlayer()->getCurrentAnimationMotion()->isAllAnimationFinished())
		{
			// 削除させる
			deleteEffect();
			return;
		}
	}

	// 停止設定時、アニメーションの再生が完了している場合
	if (_isStop && _player->getBasePlayer()->getCurrentAnimationMotion()->getReachedLastFrame()) {
		// 削除させる
		deleteEffect();
		return;
	}

	// プレイヤーの更新を行う
	_player->update(delta);
#ifdef USE_REDUCE_RENDER_TEXTURE
	//updateBackside();	//旧プレイヤーに動作を合わせるため、動的な表示優先順位の更新を行わない。
#endif
}

#ifdef USE_REDUCE_RENDER_TEXTURE
void EffectAnimation::updateBackside()
{
	if (!_forceBack && _targetObject && _targetObjectConnectId >= 0) {
		//オブジェクトの接続点に紐付けられている場合に、表側か裏側かに合わせて、表示優先度を変える。
		//todo ベンチマークの動作が遅くなるかも。
		auto _targetObjPlayer = _targetObject->getPlayer();
		if (_targetObjPlayer) {
			auto newBackside = _targetObjPlayer->getTimelineBackside(_targetObjectConnectId);
			if (newBackside != _targetObjectBackside) {
				this->setTargetObjctBackside(newBackside);
				this->setLocalZOrder(newBackside ? agtk::Object::kPartPriorityBackEffect : agtk::Object::kPartPriorityFrontEffect);
			}
		}
	}
}
#endif

void EffectAnimation::updatePosition()
{
#ifdef USE_REDUCE_RENDER_TEXTURE
	if (!_forceBack && _targetObject) {
#else
	if (_targetObject) {
#endif
		cocos2d::Vec2 pos = cocos2d::Vec2::ZERO;

		// 接続点を使用する場合
		if (_targetObjectConnectId >= 0) {
			agtk::Vertex4 v4;
			_targetObject->getTimeline(agtk::data::TimelineInfoData::kTimelineConnection, _targetObjectConnectId, v4);
			pos = agtk::Scene::getPositionSceneFromCocos2d(v4.addr()[0]);

			// 接続点のキーフレームがない場合は非表示化を行う
			_player->setVisible(_targetObject->existsArea(_targetObjectConnectId));
		}
		// 接続点を使用しない場合
		else {
			// 対象オブジェクトの中心位置を取得する
			pos = _targetObject->getCenterPosition();
		}

		// 位置調整を反映
		pos.x += _offset.x;
		pos.y += _offset.y;

		pos = agtk::Scene::getPositionCocos2dFromScene(pos);
		_player->setPosition(pos);
	}
}

void EffectAnimation::remove()
{
#ifdef USE_REDUCE_RENDER_TEXTURE
	this->removeFromParent();
#else
	auto scene = GameManager::getInstance()->getCurrentScene();
	CCASSERT(scene, "シーン取得失敗");

	auto sceneLayer = scene->getSceneLayer(_sceneLayerId);
	//CCASSERT(sceneLayer, "シーンレイヤー取得失敗");
	if (sceneLayer) {
		sceneLayer->removeChild(this);
	}
#endif
	this->removeChild(_player);
}

void EffectAnimation::deleteEffect()
{
	_isDeletable = true;
	if (_player != nullptr) {
		_player->setVisible(false);
	}
}

void EffectAnimation::stopEffect()
{
	_isStop = true;
}

bool EffectAnimation::isDeletable()
{
	return _isDeletable;
}

void EffectAnimation::setTargetObject(agtk::Object *object)
{
	_targetObject = object;
}

agtk::Object* EffectAnimation::getTargetObject()
{
	return _targetObject;
}

void EffectAnimation::setFillColor(cocos2d::Color4B color)
{
	if (_player != nullptr) {
		auto shader = _player->getShader(agtk::Shader::kShaderColorAfterimageRbga);
		if (shader == nullptr) {
			shader = _player->setShader(agtk::Shader::kShaderColorAfterimageRbga, 1);
		}
		if (shader) shader->setShaderRgbaColor(color);
	}
}

void EffectAnimation::setInnerAlpha(float alpha)
{
	if (_player != nullptr) {
		auto shader = _player->getShader(agtk::Shader::kShaderColorAfterimageRbga);
		if (shader == nullptr) {
			shader = _player->setShader(agtk::Shader::kShaderColorAfterimageRbga, 1);
		}
		if (shader) shader->setShaderAlpha(alpha);
	}
}

void EffectAnimation::setAlpha(float alpha)
{
	_alpha = alpha;
	this->setInnerAlpha(_alpha * _mainAlpha);
}

void EffectAnimation::setMainAlpha(float alpha)
{
	_mainAlpha = alpha;
	this->setInnerAlpha(_alpha *_mainAlpha);
}

//-------------------------------------------------------------------------------------------------------------------
ObjectEffect::ObjectEffect()
{
	_switchValue = false;
	_switchValueOld = false;

	_effectSettingData = nullptr;
	_effectAnimation = nullptr;
	_particleGroup = nullptr;
}

ObjectEffect::~ObjectEffect()
{
	CC_SAFE_RELEASE_NULL(_effectSettingData);
	CC_SAFE_RELEASE_NULL(_effectAnimation);
	CC_SAFE_RELEASE_NULL(_particleGroup);
}

bool ObjectEffect::init(agtk::data::ObjectEffectSettingData* effectData)
{
	// エフェクトの設定データを保持
	setEffectSettingData(effectData);

	return true;
}

NS_AGTK_END
