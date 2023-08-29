#include "GameScene.h"
#include "External/collision/CollisionComponent.hpp"

#include "AppMacros.h"
#include "Manager/AudioManager.h"
#include "Manager/FontManager.h"
#include "Manager/GameManager.h"
#include "Manager/JavascriptManager.h"
#include "Manager/DebugManager.h"
#include "Manager/GuiManager.h"
#include "Manager/EffectManager.h"
#ifdef USE_RUNTIME
#include "Lib/PhysicsObject.h"
#endif

#include "Lib/Portal.h"

#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32) || (CC_TARGET_PLATFORM == CC_PLATFORM_MAC)
#include "ImGUI/IMGUIGLViewImpl.h"
#include "ImGUI/ImGuiLayer.h"
#include "ImGUI/imgui.h"
#include "ImGUI/CCIMGUI.h"
// #AGTK-NX
#elif (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif
// #AGTK-NX #AGTK-WIN
#if 1
#include "Manager/MovieManager.h"
#endif

#include "Manager/DllPluginManager.h"
#include "scripting/js-bindings/manual/ScriptingCore.h"

#ifdef USE_COLLISION_MEASURE
extern int wallCollisionCount;
extern int hitCollisionCount;
extern int attackCollisionCount;
extern int connectionCollisionCount;
extern int woConnectionCollisionCount;
extern int noInfoCount;
extern int callCount;
extern int cachedCount;
extern int roughWallCollisionCount;
extern int roughHitCollisionCount;
#endif

// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif

USING_NS_CC;

std::string GameScene::mFontName = "fonts/ARIALUNI.TTF";

GameScene::GameScene()
{
	_counter = 0;
	_seconds = 0.0f;
	_timer = nullptr;
	_bDebugDisplay = true;//とりあえず。
	_bShowFrameRate = false;
	_debugFlag = false;
	_isRestartCanvas = false;
	_selectSceneId = 2;
	_logList = nullptr;

	_curSceneLinkDataList = nullptr;
	_fadeSpriteBefore = nullptr;
	_fadeSpriteAfter = nullptr;
#ifdef FIX_ACT2_5233
	_fadeFrontSpriteBefore = nullptr;
	_fadeFrontSpriteAfter = nullptr;
#endif
	_sceneChangeState = SceneChangeState::NONE;
	_layerColor = nullptr;
#ifdef USE_PREVIEW
	_paused = false;
#endif
	_startPointGroupIdx = 0;
	_moveObjectList = nullptr;
	_moveObjectDuration = 0;
	_moveObjectParent = nullptr;
#ifdef USE_DESIGNATED_RESOLUTION_RENDER
	_renderTexture = nullptr;
#endif
	_initialCalcMoveEffect = true;
// #AGTK-NX
#if CC_TARGET_PLATFORM == CC_PLATFORM_NX
#endif
}

GameScene::~GameScene()
{
	CCLOG("%d:%s", __LINE__, __FUNCTION__);
	CC_SAFE_RELEASE_NULL(_moveObjectParent);
	CC_SAFE_RELEASE_NULL(_moveObjectList);
	CC_SAFE_RELEASE_NULL(_layerColor);
	CC_SAFE_RELEASE_NULL(_fadeSpriteAfter);
	CC_SAFE_RELEASE_NULL(_fadeSpriteBefore);
#ifdef FIX_ACT2_5233
	CC_SAFE_RELEASE_NULL(_fadeFrontSpriteAfter);
	CC_SAFE_RELEASE_NULL(_fadeFrontSpriteBefore);
#endif
	CC_SAFE_RELEASE_NULL(_curSceneLinkDataList);

	CC_SAFE_RELEASE_NULL(_logList);
	CC_SAFE_RELEASE_NULL(_timer);
#ifdef USE_DESIGNATED_RESOLUTION_RENDER
	CC_SAFE_RELEASE_NULL(_renderTexture);
#endif
// #AGTK-NX
#if CC_TARGET_PLATFORM == CC_PLATFORM_NX
#endif
	CCLOG("%d:%s", __LINE__, __FUNCTION__);
}

void GameScene::onEnter()
{
	CCLOG("%d:%s", __LINE__, __FUNCTION__);
	Layer::onEnter();
	CCLOG("%d:%s", __LINE__, __FUNCTION__);
}

void GameScene::onEnterTranslationDidFinish()
{
	CCLOG("%d:%s", __LINE__, __FUNCTION__);
	Layer::onEnterTransitionDidFinish();
	CCLOG("%d:%s", __LINE__, __FUNCTION__);
}

void GameScene::onExit()
{
	CCLOG("%d:%s", __LINE__, __FUNCTION__);
	Layer::onExit();
	CCLOG("%d:%s", __LINE__, __FUNCTION__);
}

bool GameScene::init(int id)
{
	CCLOG("%d,%s", __LINE__, __FUNCTION__);
	//////////////////////////////
	// 1. super init first
	if (!Layer::init())
	{
		return false;
	}

	auto visibleSize = Director::getInstance()->getVisibleSize();
	Vec2 origin = Director::getInstance()->getVisibleOrigin();

	/////////////////////////////
	// 2. add a menu item with "X" image, which is clicked to quit the program
	//    you may modify it.

//    // add a "close" icon to exit the progress. it's an autorelease object
//    auto closeItem = MenuItemImage::create(
//                                           "CloseNormal.png",
//                                           "CloseSelected.png",
//                                           CC_CALLBACK_1(GameScene::menuCloseCallback, this));
//    
//    closeItem->setPosition(Vec2(origin.x + visibleSize.width - closeItem->getContentSize().width/2 ,
//                                origin.y + closeItem->getContentSize().height/2));
//
//    // create menu, it's an autorelease object
//    auto menu = Menu::create(closeItem, NULL);
//    menu->setPosition(Vec2::ZERO);
//    this->addChild(menu, 4);

//	FontManager *fm = FontManager::getInstance();
//	std::string title = GameManager::getInstance()->getProjectData()->getGameInformation()->getTitle();
//	auto text = fm->createWithArialFont(title, 24);
//	auto text = fm->createWithTTF(title, "Aharoni", 24);
//	text->setPosition(10, visibleSize.height - 60);
//	this->addChild(text, kZOrderMain+1, kTagMain);

//	text = fm->createWithArialFont("Bouldering", 16);
//	text->setPosition(10, visibleSize.height * 0.5f);
//	this->addChild(text, kZOrderMain, 100);

	auto pm = PrimitiveManager::getInstance();

	//--------------------------------------------------------------------------------------------------------------
	// scene
	//シーンフェード用のスプライトを作成
	Size size = Director::getInstance()->getVisibleSize();
	Rect rect = Rect(0, 0, size.width, size.height);

	// フェードイン・アウト時用のカラーレイヤー
	auto lc = cocos2d::LayerColor::create(Color4B::BLACK, size.width, size.height);
	lc->setVisible(false);
	lc->setIgnoreAnchorPointForPosition(false);
	lc->setPosition(Vec2::ZERO);
	lc->setAnchorPoint(Vec2::ZERO);
	this->setLayerColor(lc);
#ifdef FIX_ACT2_5233
	this->addChild(lc, ZOrder::Fade + 3, Tag::BG);
#else
	this->addChild(lc, ZOrder::Fade - 1, Tag::BG);
#endif

// #AGTK-NX
#ifdef USE_ANTIALIAS_MIN_FILTER
	// シーン縮小表示での品質劣化軽減処理
	auto setMinFilter = [](cocos2d::Sprite* sprite) {
		cocos2d::Texture2D::TexParams texparams = { GL_LINEAR, GL_NEAREST, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE };
		sprite->getTexture()->setTexParameters(texparams);
	};
#endif
	// 遷移前用スプライト生成
	auto rt = RenderTexture::create((int)size.width, (int)size.height, cocos2d::Texture2D::PixelFormat::RGBA8888);
	rt->setColor(Color3B::BLACK);//デフォルトは黒
	rt->setVisible(false);
	rt->getSprite()->setAnchorPoint(Vec2::ZERO);
// #AGTK-NX
#ifdef USE_ANTIALIAS_MIN_FILTER
	// シーン縮小表示での品質劣化軽減処理
	setMinFilter(rt->getSprite());
#else
	rt->getSprite()->getTexture()->setAliasTexParameters();
#endif
	rt->setPosition(Vec2::ZERO);
	rt->setCascadeOpacityEnabled(true);
	this->setFadeSpriteBefore(rt);
	this->addChild(rt, ZOrder::Fade, Tag::Fade);
#ifdef FIX_ACT2_5233
	auto frt = RenderTexture::create((int)size.width, (int)size.height, cocos2d::Texture2D::PixelFormat::RGBA8888);
	frt->setColor(Color3B::BLACK);//デフォルトは黒
	frt->setVisible(false);
	frt->getSprite()->setAnchorPoint(Vec2::ZERO);
// #AGTK-NX
#ifdef USE_ANTIALIAS_MIN_FILTER
	// シーン縮小表示での品質劣化軽減処理
	setMinFilter(frt->getSprite());
#else
	frt->getSprite()->getTexture()->setAliasTexParameters();
#endif
	frt->setPosition(Vec2::ZERO);
	frt->setCascadeOpacityEnabled(true);
	this->setFadeFrontSpriteBefore(frt);
	this->addChild(frt, ZOrder::Fade + 2, Tag::Fade);
#endif

	// 遷移後用スプライト生成
	auto rt2 = RenderTexture::create((int)size.width, (int)size.height, cocos2d::Texture2D::PixelFormat::RGBA8888);
	rt2->setColor(Color3B::BLACK);//デフォルトは黒
	rt2->setVisible(false);
	rt2->getSprite()->setAnchorPoint(Vec2::ZERO);
// #AGTK-NX
#ifdef USE_ANTIALIAS_MIN_FILTER
	// シーン縮小表示での品質劣化軽減処理
	setMinFilter(rt2->getSprite());
#else
	rt2->getSprite()->getTexture()->setAliasTexParameters();
#endif
	rt2->setPosition(Vec2::ZERO);
	rt2->setCascadeOpacityEnabled(true);
	this->setFadeSpriteAfter(rt2);
#ifdef FIX_ACT2_5233
	this->addChild(rt2, ZOrder::Fade, Tag::Fade);
#else
	this->addChild(rt2, ZOrder::Fade + 1, Tag::Fade);
#endif
#ifdef FIX_ACT2_5233
	auto frt2 = RenderTexture::create((int)size.width, (int)size.height, cocos2d::Texture2D::PixelFormat::RGBA8888);
	frt2->setColor(Color3B::BLACK);//デフォルトは黒
	frt2->setVisible(false);
	frt2->getSprite()->setAnchorPoint(Vec2::ZERO);
// #AGTK-NX
#ifdef USE_ANTIALIAS_MIN_FILTER
	// シーン縮小表示での品質劣化軽減処理
	setMinFilter(frt2->getSprite());
#else
	frt2->getSprite()->getTexture()->setAliasTexParameters();
#endif
	frt2->setPosition(Vec2::ZERO);
	frt2->setCascadeOpacityEnabled(true);
	this->setFadeFrontSpriteAfter(frt2);
	this->addChild(frt2, ZOrder::Fade + 2, Tag::Fade);
#endif

	// 移動対象オブジェクトリスト生成
	this->setMoveObjectList(cocos2d::__Array::create());

	// 移動対象オブジェクト用の親ノード生成
	auto node = Node::create();
#ifdef FIX_ACT2_5233
	this->addChild(node, ZOrder::Fade + 1, Tag::Fade);
#else
	this->addChild(node, ZOrder::Fade + 2, Tag::Fade);
#endif
	this->setMoveObjectParent(node);

	_selectSceneId = agtk::Scene::START_SCENE_ID;
	cocos2d::__Array *arr = cocos2d::__Array::create();
	this->setCurSceneLinkDataList(arr);

	// カレントシーンのリンクデータリスト更新
	this->updateCurSceneLinkDataList();

	std::string path = FileUtils::getInstance()->getWritablePath();

	registerInputListener(_eventDispatcher, this);

	//imgui
	this->debugGUI();
//	debugInit();
#if 0
	//movie
	auto videoLayer = CCVideoLayer::create("movies/001.mp4");
	videoLayer->setPosition(400, 100);
//	videoLayer->setContentSize(cocos2d::Size(100, 100));
	this->addChild(videoLayer, 10, 1001);
	videoLayer->playVideo();
	videoLayer->setScale(0.2f);
	videoLayer->setOpacity(128);
#endif

	this->setLogList(cocos2d::__Array::create());

	this->setTimer(agtk::Timer::create());

	//get game pad status in polling mode
	//scheduleUpdate();
	scheduleUpdateWithPriority(kSchedulePriorityScene);
	//	cocos2d::Director::getInstance()->getScheduler()->setTimeScale(2.0);
	CCLOG("init layer children: %d", this->getChildrenCount());

	GameManager::getInstance()->initPlugins();

#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX) // #AGTK-NX #TODO
#endif // (CC_TARGET_PLATFORM == CC_PLATFORM_NX)

// #AGTK-NX #AGTK-WIN
#ifdef USE_30FPS
	// ゲームシーンのフレームレート初期設定。デフォルトは60FPS
#if !defined(ENABLE_FORCE_30FPS)
	if(GameManager::getInstance()->getProjectData()->getMode30Fps())
#endif
	{
		GameManager::getInstance()->setFrameProgressScale(2.0f);
		GameManager::getInstance()->setFrameRate(FRAME30_RATE);
		Director::getInstance()->setAnimationInterval(1.0f / FRAME30_RATE);
	}
#endif

#ifdef USE_DESIGNATED_RESOLUTION_RENDER
	auto director = Director::getInstance();
	auto glview = Director::getInstance()->getOpenGLView();
	auto resolutionSize = glview->getDesignResolutionSize();
	auto renderTexture = cocos2d::RenderTexture::create(resolutionSize.width, resolutionSize.height, cocos2d::Texture2D::PixelFormat::RGBA8888);
	renderTexture->setContentSize(resolutionSize);
	this->setRenderTexture(renderTexture);
	auto sprite = renderTexture->getSprite();
	sprite->setAnchorPoint(cocos2d::Vec2(0, 0));
// #AGTK-NX
#ifdef USE_ANTIALIAS_MIN_FILTER
	// シーン縮小表示での品質劣化を軽減
	setMinFilter(sprite);
#else
	sprite->getTexture()->setAliasTexParameters();
#endif
	sprite->setCameraMask((unsigned short)cocos2d::CameraFlag::USER1);
#endif
	return true;
}

/**
* シーン遷移条件チェック
* @return				True:シーン遷移要求 / False:シーン遷移要求無し
*/
bool GameScene::checkSceneLinkCondition()
{
	//マネージャ類
	auto gameManager = GameManager::getInstance();
	auto inputManager = InputManager::getInstance();

	// ワーク変数
	auto projectPlayData = gameManager->getPlayData();
	auto scene = gameManager->getCurrentScene();
	int preSceneId = gameManager->getPrevSceneId();
	auto projectData = gameManager->getProjectData();
	auto transitionFlowData = projectData->getTransitionFlow();
	auto flowScreenList = transitionFlowData->getFlowScreenList();

	// シーン変更要求フラグ
	bool needSceneChange = false;

	// 開始するリンクデータから切替条件が満たされるデータを検索
	// ※ _curSceneLinkDataList は優先度が高い順に格納されている
	cocos2d::Ref *ref = nullptr;
	CCARRAY_FOREACH(this->getCurSceneLinkDataList(), ref) {

		auto linkData = dynamic_cast<agtk::data::FlowLinkData *>(ref);
		auto linkDataIdx = this->_curSceneLinkDataList->getIndexOfObject(ref);
		bool isAllConditionOk = (linkData->getChangeConditionType() == agtk::data::EnumChangeConditionType::kAllConditionsSatisfied);//全ての条件 or いずれかの条件
		bool isNoInputConditionOk = isAllConditionOk;//入力なし条件一致フラグ
		bool isInputConditionOk = false;// isAllConditionOk;//入力あり条件一致フラグ
		bool isPreSceneFinished = isAllConditionOk;//前のシーンが終了しているフラグ
		bool isTimeElapsed = isAllConditionOk;//一定時間が経過しているフラグ
		bool isSwitchVariableChanged = isAllConditionOk;//スイッチ、変数が変化しているフラグ

		// ==========================================================================
		// ▼入力に関する条件
		// ==========================================================================
		// 何も入力されなかったにチェックがある場合
		if (linkData->getNoInput()) {
			// 入力無し条件一致フラグ設定
			isNoInputConditionOk = inputManager->isNoneInput();

			// スタートの場合は入力なし条件一致フラグ(TRUE)
			if (preSceneId == agtk::data::FlowScreenData::SCENE_MENU_ID_START) {
				isNoInputConditionOk = true;
			}
		}
		
		// 以下の入力操作がなされたにチェックがある場合
		if (linkData->getUseInput()) {
			// 登録されている入力条件をチェックする
			auto inputConditionGroupList = linkData->getInputConditionGroupList();
			if (inputConditionGroupList->count() > 0) {
				auto isInputState = [&](agtk::data::ObjectInputConditionData::EnumTriggerType type, int keyCode) {
					switch (type) {
					case agtk::data::ObjectInputConditionData::kTriggerPressed: return inputManager->isPressed(keyCode); break;//押された
					case agtk::data::ObjectInputConditionData::kTriggerJustPressed: return inputManager->isTriggered(keyCode); break;//押された瞬間
					case agtk::data::ObjectInputConditionData::kTriggerJustReleased: return inputManager->isReleased(keyCode); break;//離された瞬間
					case agtk::data::ObjectInputConditionData::kTriggerReleased: return inputManager->isReleasing(keyCode); break;//離されている
					default:CC_ASSERT(0);
					}
					return false;
				};
				cocos2d::Ref *ref = nullptr;
				CCARRAY_FOREACH(inputConditionGroupList, ref) {
					auto inputConditionList = dynamic_cast<cocos2d::__Array *>(ref);
					cocos2d::Ref *ref2 = nullptr;
					bool ret = false;
					CCARRAY_FOREACH(inputConditionList, ref2) {
						auto data = dynamic_cast<agtk::data::FlowLinkInputConditionData *>(ref2);
						auto triggerType = data->getTriggerType();

						// 「入力に使う操作キー」の場合
						if (data->getUseKey()) {
							ret = isInputState(triggerType, data->getOperationKeyId());
						}
						// 「８方向入力」の場合
						else {
							switch (data->getDirectionInputType()) {
								// ------------------------------------------------------------------
								//方向キー
								// ------------------------------------------------------------------
							case agtk::data::ObjectInputConditionData::kDirectionInputCross:
								//左下
								if (data->getDirectionBit() & 1 << InputManager::kDirectionLeftDown) {
									ret |= isInputState(triggerType, InputController::Left) && isInputState(triggerType, InputController::Down);
								}
								//下
								if (data->getDirectionBit() & 1 << InputManager::kDirectionDown) {
									ret |= isInputState(triggerType, InputController::Down);
								}
								//右下
								if (data->getDirectionBit() & 1 << InputManager::kDirectionRightDown) {
									ret |= isInputState(triggerType, InputController::Right) && isInputState(triggerType, InputController::Down);
								}
								//左
								if (data->getDirectionBit() & 1 << InputManager::kDirectionLeft) {
									ret |= isInputState(triggerType, InputController::Left);
								}
								//右
								if (data->getDirectionBit() & 1 << InputManager::kDirectionRight) {
									ret |= isInputState(triggerType, InputController::Right);
								}
								//左上
								if (data->getDirectionBit() & 1 << InputManager::kDirectionLeftUp) {
									ret |= isInputState(triggerType, InputController::Left) && isInputState(triggerType, InputController::Up);
								}
								//上
								if (data->getDirectionBit() & 1 << InputManager::kDirectionUp) {
									ret |= isInputState(triggerType, InputController::Up);
								}
								//右上
								if (data->getDirectionBit() & 1 << InputManager::kDirectionRightUp) {
									ret |= isInputState(triggerType, InputController::Right) && isInputState(triggerType, InputController::Up);
								}
								break;
								// ------------------------------------------------------------------
								//左スティック
								// ------------------------------------------------------------------
							case agtk::data::ObjectInputConditionData::kDirectionInputLeftStick:
								//左下
								if (data->getDirectionBit() & 1 << InputManager::kDirectionLeftDown) {
									ret |= isInputState(triggerType, InputController::LeftStickLeft) && isInputState(triggerType, InputController::LeftStickDown);
								}
								//下
								if (data->getDirectionBit() & 1 << InputManager::kDirectionDown) {
									ret |= isInputState(triggerType, InputController::LeftStickDown);
								}
								//右下
								if (data->getDirectionBit() & 1 << InputManager::kDirectionRightDown) {
									ret |= isInputState(triggerType, InputController::LeftStickRight) && isInputState(triggerType, InputController::LeftStickDown);
								}
								//左
								if (data->getDirectionBit() & 1 << InputManager::kDirectionLeft) {
									ret |= isInputState(triggerType, InputController::LeftStickLeft);
								}
								//右
								if (data->getDirectionBit() & 1 << InputManager::kDirectionRight) {
									ret |= isInputState(triggerType, InputController::LeftStickRight);
								}
								//左上
								if (data->getDirectionBit() & 1 << InputManager::kDirectionLeftUp) {
									ret |= isInputState(triggerType, InputController::LeftStickLeft) && isInputState(triggerType, InputController::LeftStickUp);
								}
								//上
								if (data->getDirectionBit() & 1 << InputManager::kDirectionUp) {
									ret |= isInputState(triggerType, InputController::LeftStickUp);
								}
								//右上
								if (data->getDirectionBit() & 1 << InputManager::kDirectionRightUp) {
									ret |= isInputState(data->getTriggerType(), InputController::LeftStickRight) && isInputState(data->getTriggerType(), InputController::LeftStickUp);
								}
								break;
								// ------------------------------------------------------------------
								//右スティック
								// ------------------------------------------------------------------
							case agtk::data::ObjectInputConditionData::kDirectionInputRightStick:
								//左下
								if (data->getDirectionBit() & 1 << InputManager::kDirectionLeftDown) {
									ret |= isInputState(data->getTriggerType(), InputController::RightStickLeft) && isInputState(data->getTriggerType(), InputController::RightStickDown);
								}
								//下
								if (data->getDirectionBit() & 1 << InputManager::kDirectionDown) {
									ret |= isInputState(data->getTriggerType(), InputController::RightStickDown);
								}
								//右下
								if (data->getDirectionBit() & 1 << InputManager::kDirectionRightDown) {
									ret |= isInputState(data->getTriggerType(), InputController::RightStickRight) && isInputState(data->getTriggerType(), InputController::RightStickDown);
								}
								//左
								if (data->getDirectionBit() & 1 << InputManager::kDirectionLeft) {
									ret |= isInputState(data->getTriggerType(), InputController::RightStickLeft);
								}
								//右
								if (data->getDirectionBit() & 1 << InputManager::kDirectionRight) {
									ret |= isInputState(data->getTriggerType(), InputController::RightStickRight);
								}
								//左上
								if (data->getDirectionBit() & 1 << InputManager::kDirectionLeftUp) {
									ret |= isInputState(data->getTriggerType(), InputController::RightStickLeft) && isInputState(data->getTriggerType(), InputController::RightStickUp);
								}
								//上
								if (data->getDirectionBit() & 1 << InputManager::kDirectionUp) {
									ret |= isInputState(data->getTriggerType(), InputController::RightStickUp);
								}
								//右上
								if (data->getDirectionBit() & 1 << InputManager::kDirectionRightUp) {
									ret |= isInputState(data->getTriggerType(), InputController::RightStickRight) && isInputState(data->getTriggerType(), InputController::RightStickUp);
								}
								break;
							}
						}
						if (!ret) {
							break;
						}
					}
					if (ret) {
						isInputConditionOk = true;
					}
				}
			}
		}

		// 「すべての条件が満たされていたら切り替え」
		if (isAllConditionOk) {
			// かつ入力に関する条件設定が無い場合
			if (!linkData->getNoInput() && !linkData->getUseInput()) {
				isInputConditionOk = true;
				isNoInputConditionOk = true;
			}
			// かつ入力条件で「何も入力がない」にチェックがあり＆「入力操作」無しの場合
			else if(linkData->getNoInput() && !linkData->getUseInput()) {
				isInputConditionOk = true;
			}
		}

		// ==========================================================================
		// ▼その他条件設定
		// ==========================================================================
		// 前のシーンが終了しているにチェックが入っている場合
		if (linkData->getSceneTerminated()) {
			isPreSceneFinished = gameManager->getNeedTerminateScene();
		}

		// 一定時間が経過したにチェックが入っている場合
		if (linkData->getTimeElapsed()) {
			isTimeElapsed = (nullptr != scene && linkData->getTimeElapsed300() < scene->getElapsedTime() * 300);
		}

		// スイッチ、変数が変化にチェックが入っている場合
		if (linkData->getSwitchVariableChanged()) {
			cocos2d::Ref *ref2 = nullptr;
			CCARRAY_FOREACH(linkData->getSwitchVariableConditionList(), ref2) {
				auto conditionData = dynamic_cast<agtk::data::SwitchVariableConditionData *>(ref2);

				// スイッチが変化の場合
				if (conditionData->getSwtch()) {
					// スイッチデータリスト取得
					cocos2d::__Array *switchDataList = cocos2d::__Array::create();
					gameManager->getSwitchVariableDataList(conditionData->getSwitchQualifierId(), conditionData->getSwitchObjectId(), conditionData->getSwitchId(), true, switchDataList);

					// スイッチデータリストを回す
					cocos2d::Ref *ref4 = nullptr;
					bool ret = false;
					CCARRAY_FOREACH(switchDataList, ref4) {
						// スイッチのON/OFFの状態による条件チェック
						auto switchData = dynamic_cast<agtk::data::PlaySwitchData *>(ref4);
						ret |= (nullptr != switchData && gameManager->checkSwitchCondition(conditionData->getSwitchValue(), switchData->getValue(), switchData->isState()));
					}

					isSwitchVariableChanged = isAllConditionOk ? (isSwitchVariableChanged & ret) : (isSwitchVariableChanged | ret);
				}
				// 変数が変化の場合
				else {
					// 変数データリスト取得
					cocos2d::__Array *variableDataList = cocos2d::__Array::create();
					gameManager->getSwitchVariableDataList(conditionData->getVariableQualifierId(),conditionData->getVariableObjectId(),	conditionData->getVariableId(), false, variableDataList);

					// 変数データリストを回す
					cocos2d::Ref *ref4 = nullptr;
					bool ret = false;
					CCARRAY_FOREACH(variableDataList, ref4) {
						auto variableData = dynamic_cast<agtk::data::PlayVariableData *>(ref4);
						double srcValue = variableData->getValue();
						double compareValue = 0;
						ret |= gameManager->checkVariableCondition(
													conditionData->getCompareValueType(),
													srcValue,
													conditionData->getCompareOperator(),
													conditionData->getComparedValue(),
													conditionData->getComparedVariableQualifierId(),
													conditionData->getComparedVariableObjectId(),
													conditionData->getComparedVariableId());
					}

					isSwitchVariableChanged = isAllConditionOk ? (isSwitchVariableChanged & ret) : (isSwitchVariableChanged | ret);
				}
			}
		}

		// 条件が満たされているか算出
		bool conditionOk = isAllConditionOk ? (isNoInputConditionOk && isInputConditionOk && isPreSceneFinished && isTimeElapsed && isSwitchVariableChanged) : (isNoInputConditionOk || isInputConditionOk || isPreSceneFinished || isTimeElapsed || isSwitchVariableChanged);

		// 条件が満たされている場合
		if (conditionOk) {

			// フロースクリーンリストから preSceneId と一致する遷移元のデータを検索
			auto pair = linkData->getscreenIdPair();
			int nextScreenId = dynamic_cast<cocos2d::Integer *>(pair->getObjectAtIndex(1))->getValue();

			cocos2d::Ref *ref5 = nullptr;
			CCARRAY_FOREACH(flowScreenList, ref5) {
				auto screenData = dynamic_cast<agtk::data::FlowScreenData *>(ref5);
				if (screenData->getId() == nextScreenId) {
					// todo: メニュー画面の場合は現時点では未定

#ifdef USE_PREVIEW
					// スタートシーンの場合、自動テスト開始のフラグを立てる
					if (preSceneId == agtk::data::FlowScreenData::SCENE_MENU_ID_START && gameManager->getAutoTestReplayFilePath().compare("") != 0) {
						gameManager->setStartSceneFlg(true);
					}
#endif

					// 遷移先のシーンIDを設定
					gameManager->setNextSceneId(screenData->getSceneMenuId());

					// シーン変更要求フラグON
					needSceneChange = true;

					this->setTargetSceneLinkDataIdx(linkDataIdx);

					this->setStartPointGroupIdx(linkData->getStartPointGroup());

					break;
				}
			}

			break;
		}
	}

	return needSceneChange;
}

void GameScene::initCurSceneLinkDataList(int sceneId)
{
	_selectSceneId = sceneId;
	_sceneChangeState = SceneChangeState::NONE;
	this->updateCurSceneLinkDataList();
}

/**
 * カレントシーンのリンクデータリスト更新
 */
void GameScene::updateCurSceneLinkDataList()
{
	CCLOG("-- GameScene::updateCurSceneLinkDataList()");

	// ワーク変数
	auto projectData = GameManager::getInstance()->getProjectData();
	auto transitionFlowData = projectData->getTransitionFlow();
	auto flowLinkList = transitionFlowData->getFlowLinkList();
	auto flowScreenList = transitionFlowData->getFlowScreenList();

	this->getCurSceneLinkDataList()->removeAllObjects();

	cocos2d::Ref *ref = nullptr;
	CCARRAY_FOREACH(flowLinkList, ref) {
		// 遷移元のスクリーンIDを取得する
		auto linkData = dynamic_cast<agtk::data::FlowLinkData *>(ref);
		auto pair = linkData->getscreenIdPair();
		int fromScreenId = dynamic_cast<cocos2d::Integer *>(pair->getObjectAtIndex(0))->getValue();

		// フロースクリーンリストから preSceneId と一致する遷移元のデータを検索
		cocos2d::Ref *ref2 = nullptr;
		CCARRAY_FOREACH(flowScreenList, ref2) {
			auto screenData = dynamic_cast<agtk::data::FlowScreenData *>(ref2);
			// スクリーンIDが遷移元スクリーンIDの場合
			if (screenData->getId() == fromScreenId && screenData->getSceneMenuId() == _selectSceneId) {
				// 優先度を考慮してフローリンクデータを保持
				bool isInsert = false;
				for (int i = 0; i < this->getCurSceneLinkDataList()->count(); i++) {
					auto p = dynamic_cast<agtk::data::FlowLinkData *>(this->getCurSceneLinkDataList()->getObjectAtIndex(i));
					if (linkData->getPriority() > p->getPriority()) {
						this->getCurSceneLinkDataList()->insertObject(linkData, i);
						isInsert = true;
						break;
					}
				}
				if (!isInsert) {
					this->getCurSceneLinkDataList()->addObject(linkData);
				}
				break;
			}
		}
	}
}

/**
 * 画面切り替え演出処理
 * @param	isPreMove		True:切り替え前 / False:切り替え後
 * @param	isPortal		ポータルでの移動か？
 * @param	needSceneChange	シーン変更が必要か？
 */
void GameScene::calcMoveEffect(bool isPreMove, bool isPortal, bool needSceneChange)
{
	CCLOG("-- GameScene::calcMoveEffect(%s), isPortal:%s", DUMP_BOOLTEXT(isPreMove), DUMP_BOOLTEXT(isPortal));
	AGTK_DEBUG_ACTION_LOG("%d,%s", __LINE__, __FUNCTION__);

	// ワーク変数
	auto scene = this->getScene();
#ifdef USE_DESIGNATED_RESOLUTION_RENDER
	if (needSceneChange || _initialCalcMoveEffect) {
		if (scene) {
			scene->getCamera()->update(0.1f);
		}
	}
#endif
	auto visibleSize = Director::getInstance()->getVisibleSize();
	auto screenCenterPos = visibleSize / 2;
	auto gm = GameManager::getInstance();
	auto camera = Director::getInstance()->getRunningScene()->getDefaultCamera();
	auto camRotQuat = camera->getRotationQuat();
	auto camAnchor = camera->getAnchorPoint();
	auto camScale = Vec2(camera->getScaleX(), camera->getScaleY());
	auto camSize = camera->getContentSize();
	auto camAnchorDiff = Vec2((0 - camAnchor.x) * camSize.width * camScale.x, (0 - camAnchor.y) * camSize.height * camScale.y);

	agtk::data::BaseSceneChangeEffectData *effectData = nullptr;

	// ポータル移動による演出の場合
	if (isPortal) {
		// ゲームマネージャに登録された演出データを設定
		effectData = gm->getPortalData();
	}
	// シーン遷移による演出の場合
	else {
		// シーンリンクデータから演出データを取得
		effectData = this->_curSceneLinkDataList->count() > 0 ? dynamic_cast<agtk::data::BaseSceneChangeEffectData *>(this->_curSceneLinkDataList->getObjectAtIndex(this->_targetSceneLinkDataIdx)) : nullptr;
	}

	// シーン切り替え中ステートを設定
	_sceneChangeState = isPreMove ? SceneChangeState::PREMOVE : SceneChangeState::POSTMOVE;

	if (nullptr != effectData) {
		CCLOG("-- START calcMoveEffect: %s", (isPreMove ? "PRE_MOVE" : "POST_MOVE"));
		AGTK_DEBUG_ACTION_LOG("%d,%s", __LINE__, __FUNCTION__);

		// 移動前演出タイプ取得
		auto preEffectType = effectData->getPreMoveEffect();
		// 移動前演出が連結スライド系か？
		bool isSlideLinkEffect = preEffectType > agtk::data::EnumMoveEffect::kMoveEffectSlideRight;
		// 演出タイプと時間取得(連結スライド系は移動前の演出タイプと時間を使用する)
		auto effectType = isPreMove || isSlideLinkEffect ? preEffectType : effectData->getPostMoveEffect();
		auto effectDuration = (isPreMove || isSlideLinkEffect ? effectData->getPreMoveDuration300() : effectData->getPostMoveDuration300()) / 300.0f;

		//「ACT2-1648」の対応。スタート時は、移動前演出のエフェクト時間を0にする。
		bool bStartScreen = false;
		if (!isPortal) {
			std::function<bool(agtk::data::BaseSceneChangeEffectData *)> isStartScreen = [&](agtk::data::BaseSceneChangeEffectData *data) {
				auto projectData = GameManager::getInstance()->getProjectData();
				auto transitionFlow = projectData->getTransitionFlow();
				cocos2d::Ref *ref = nullptr;
				agtk::data::FlowScreenData *startFlowScreen = nullptr;
				CCARRAY_FOREACH(transitionFlow->getFlowScreenList(), ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
					auto flowScreen = static_cast<agtk::data::FlowScreenData *>(ref);
#else
					auto flowScreen = dynamic_cast<agtk::data::FlowScreenData *>(ref);
#endif
					if (flowScreen->getSceneMenuId() == agtk::data::FlowScreenData::SCENE_MENU_ID_START) {
						startFlowScreen = flowScreen;
						break;
					}
				}
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto flowLinkData = static_cast<agtk::data::FlowLinkData *>(effectData);
#else
				auto flowLinkData = dynamic_cast<agtk::data::FlowLinkData *>(effectData);
#endif
				if (flowLinkData) {
					auto screenIdPair = flowLinkData->getscreenIdPair();
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
					auto flowScreenId = static_cast<cocos2d::Integer *>(screenIdPair->getObjectAtIndex(0));
#else
					auto flowScreenId = dynamic_cast<cocos2d::Integer *>(screenIdPair->getObjectAtIndex(0));
#endif
					if (flowScreenId->getValue() == startFlowScreen->getId()) {
						return true;
					}
				}
				return false;
			};
			bStartScreen = isStartScreen(effectData);
			if (bStartScreen) {
				effectDuration = 0.0f;
			}
		}

		// BGM系データ取得
		auto bgmChangeType = isPreMove ? effectData->getPreMoveBgmChangeType() : effectData->getPostMoveBgmChangeType();
		auto moveBgmChangeTiming = isPreMove ? effectData->getPreMoveBgmChangeTiming() : effectData->getPostMoveBgmChangeTiming();
		auto bgmFadeOut = isPreMove ? effectData->getPreMoveBgmFadeout() : effectData->getPostMoveBgmFadeout();
		auto bgmId = isPreMove ? effectData->getPreMoveBgmId() : effectData->getPostMoveBgmId();
		auto bgmLoop = isPreMove ? effectData->getPreMoveBgmLoop() : effectData->getPostMoveBgmLoop();

		// SE再生フラグ取得
		auto moveSePlay = isPreMove ? effectData->getPreMovePlaySe() : effectData->getPostMovePlaySe();

#ifdef FIX_ACT2_5233
		int objectLayerId = -1;
		if (isPortal) {
			if (isPreMove) {
				cocos2d::Ref *ref = nullptr;
				CCARRAY_FOREACH(gm->getPortalTouchedPlayerList(), ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
					auto data = static_cast<agtk::PortalTouchedData *>(ref);
#else
					auto data = dynamic_cast<agtk::PortalTouchedData *>(ref);
#endif
					auto object = data->getObject();
					objectLayerId = object->getLayerId();
					gm->setMoveToLayerId(data->getsceneLayerId());
					break;
				}
			}
			else {
				objectLayerId = gm->getMoveToLayerId();
			}
		}
#endif

		// 遷移前かつスライド連結演出の場合
		if (isPreMove && isSlideLinkEffect) {

			auto objectList = cocos2d::Array::create();

			std::function<void(agtk::Object *, agtk::Object::ForceVisibleState)> setForceVisibleState = [&](agtk::Object *obj, agtk::Object::ForceVisibleState state) {
				cocos2d::Ref *ref;
				if (obj->getConnectObjectList()->count() > 0) {
					CCARRAY_FOREACH(obj->getConnectObjectList(), ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
						auto connectObject = static_cast<agtk::Object *>(ref);
#else
						auto connectObject = dynamic_cast<agtk::Object *>(ref);
#endif
						connectObject->setForceVisibleState(state);
						if (objectList->containsObject(connectObject) == false) {
							objectList->addObject(connectObject);
							setForceVisibleState(connectObject, state);
						}
					}
				}
				if (obj->getChildrenObjectList()->count() > 0) {
					CCARRAY_FOREACH(obj->getConnectObjectList(), ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
						auto childObject = static_cast<agtk::Object *>(ref);
#else
						auto childObject = dynamic_cast<agtk::Object *>(ref);
#endif
						if (objectList->containsObject(childObject) == false) {
							objectList->addObject(childObject);
							setForceVisibleState(childObject, state);
						}
					}
				}
			};

			//オブジェクト表示状態でレンダリング（※ただし遷移中プレイヤーは非表示）
			auto state = agtk::Object::ForceVisibleState::kNotVisible;
			cocos2d::Ref *ref;
			CCARRAY_FOREACH(gm->getPortalTouchedPlayerList(), ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto data = static_cast<agtk::PortalTouchedData *>(ref);
#else
				auto data = dynamic_cast<agtk::PortalTouchedData *>(ref);
#endif
				auto object = data->getObject();
				objectList->addObject(object);
				object->setForceVisibleState(state);
				setForceVisibleState(object, state);
			}
			//オブジェクトに紐づいているGUIを非表示に。
			CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto object = static_cast<agtk::Object *>(ref);
#else
				auto object = dynamic_cast<agtk::Object *>(ref);
#endif
				GuiManager::getInstance()->setVisibleGui(false, object);//非表示。
			}
			scene->updateSceneRenderTextureList(0.1f, false, objectLayerId);
		}

		// =========================================================
		// SE演出処理
		// =========================================================
		// SEを鳴らす場合
		if (moveSePlay) {
			// SE再生
			auto seId = isPreMove ? effectData->getPreMoveSeId() : effectData->getPostMoveSeId();
			AudioManager::getInstance()->playSe(seId);
		}

		// =========================================================
		// BGM演出処理
		// =========================================================
		// 画面演出開始時にBGM演出
		if (moveBgmChangeTiming == agtk::data::EnumBgmChangeTiming::kBgmChangeTimingBeforeMoveEffect ) {
			// BGMを再生 ※連結スライド演出時はイン・アウトが同時の為動作時間を半分にする
			GameManager::BgmInfo bgm;
			bgm._bgmId = bgmId;
			bgm._loop = bgmLoop;
			gm->calcMoveBgmEffect(isSlideLinkEffect ? effectDuration * 0.5f : effectDuration, bgmChangeType, bgmFadeOut, { bgm }, isPreMove);
		}

		// =========================================================
		// 画面演出処理
		// =========================================================
		// 画面切り替え演出後のコールバック生成
		auto afterEffectCallback = CCCallFunc::create([=](void) {

			CCLOG("-- PROC afterEffectCallback");
			AGTK_DEBUG_ACTION_LOG("%d,%s", __LINE__, __FUNCTION__);

			this->getLayerColor()->setVisible(false);
			this->getFadeSpriteBefore()->setVisible(false);
			this->getFadeSpriteAfter()->setVisible(false);
			this->getFadeSpriteBefore()->setPosition(cocos2d::Vec2::ZERO);
			this->getFadeSpriteAfter()->setPosition(cocos2d::Vec2::ZERO);
#ifdef FIX_ACT2_5233
			this->getFadeFrontSpriteBefore()->setVisible(false);
			this->getFadeFrontSpriteAfter()->setVisible(false);
			this->getFadeFrontSpriteBefore()->setPosition(cocos2d::Vec2::ZERO);
			this->getFadeFrontSpriteAfter()->setPosition(cocos2d::Vec2::ZERO);
#endif

			// 画面演出終了時にBGM演出
			if (moveBgmChangeTiming == agtk::data::EnumBgmChangeTiming::kBgmChangeTimingAfterMoveEffect ) {
				// BGMを再生 ※連結スライド演出時はイン・アウトが同時の為動作時間を半分にする
				GameManager::BgmInfo bgm;
				bgm._bgmId = bgmId;
				bgm._loop = bgmLoop;
				gm->calcMoveBgmEffect(isSlideLinkEffect ? effectDuration * 0.5f : effectDuration, bgmChangeType, bgmFadeOut, { bgm }, isPreMove);
			}

			// --------------------------------------------------------------------
			// シーン切り替え前演出後の場合
			// --------------------------------------------------------------------
			if (this->_sceneChangeState == SceneChangeState::PREMOVE) {
				CCLOG("-- from PRE_MOVE");
				AGTK_DEBUG_ACTION_LOG("%d,%s", __LINE__, __FUNCTION__);
				cocos2d::Vec2 srcPortalCenterPos = Vec2::ZERO;		// 遷移前のポータルの中心座標
				cocos2d::Vec2 dstPortalCenterPos = Vec2::ZERO;		// 遷移後のポータルの中心座標
				cocos2d::Vec2 srcObjOffsetForCamera = Vec2::ZERO;	// 遷移前のオブジェクトのカメラ中央からの座標オフセット

				bool isNewScene = false;
				auto nextSceneId = gm->getNextSceneId();
				this->_selectSceneId = nextSceneId;

				// 操作をキャンセル
				InputManager::getInstance()->reset();

				// 次のシーンが「START」でない かつ シーン変更要求がある場合
				if (nextSceneId > agtk::Scene::START_SCENE_ID && needSceneChange) {
					// 前のシーンが存在する場合
					cocos2d::__Array *menuObjectList = nullptr;
					if (scene) {

						// 前のシーンのBGM情報を保持
						auto sceneData = scene->getSceneData();
						gm->setPreMoveBgmId(sceneData->getBgmId());
						gm->setPreMoveBgmLoopFlag(sceneData->getLoopBgmFlag());

						// ※getObjectAllReference()との置き換え不可
						menuObjectList = scene->getObjectAll(agtk::SceneLayer::kTypeMenu);
						menuObjectList->retain();

						// シーンを破棄
						scene->end();
					}

					// ポータル移動の場合
					if (isPortal) {
						// ポータルに触れたプレイヤーリストから
						// スタートポイントによって生成されたオブジェクトを除外用リストに登録する
						gm->getIgnoreCreatePlayerList()->removeAllObjects();
						cocos2d::Ref *ref = nullptr;
						CCARRAY_FOREACH(gm->getPortalTouchedPlayerList(), ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
							auto data = static_cast<agtk::PortalTouchedData *>(ref);
#else
							auto data = dynamic_cast<agtk::PortalTouchedData *>(ref);
#endif
							auto object = data->getObject();
							auto objectId = object->getObjectData()->getId();
							auto playerId = object->getPlayObjectData()->getPlayerId();

							// スタートポイントによって生成されたオブジェクトの場合
							if (playerId > 0 && objectId > 0) {
								// 除外用データを生成して除外用リストに登録
								auto ignoreData = GameManager::IgnoreCreateObjectData::create(playerId, objectId);
								gm->getIgnoreCreatePlayerList()->addObject(ignoreData);
							}
						}
					}

					// 次のシーンを生成
					// スタート位置グループもここで指定する
					auto project = gm->getProjectData();
					auto nextScene = agtk::Scene::create(project->getSceneData(nextSceneId), this->getStartPointGroupIdx(), isPortal ? agtk::Scene::kCreateTypePortal : agtk::Scene::kCreateTypeScreenFlow);
					this->addChild(nextScene, ZOrder::Scene, Tag::Scene);
					nextScene->start(this, bStartScreen);
					if (menuObjectList != nullptr) {
						//メニューシーンのスイッチ・変数を引き継ぐ。
						nextScene->setTakeOverMenuObject(menuObjectList);
						menuObjectList->release();
					}
					isNewScene = true;
				}

				// 次のシーンを取得
				auto nextScene = gm->getCurrentScene();

				//「アクションで復活」条件のオブジェクトで、シーン配置以外のオブジェクトは破棄する。
				gm->removeCommandReappearObjectList(true);

				auto newObjectList = cocos2d::__Array::create();
				auto oldObjectList = cocos2d::__Array::create();

				// ポータル移動の場合
				if (isPortal) {
					// 出現位置を設定する
					int apperNumper = 1;			// 出現順序(データがポータルに触れた順)
					cocos2d::Ref *ref = nullptr;
					CCARRAY_FOREACH(gm->getPortalTouchedPlayerList(), ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
						auto data = static_cast<agtk::PortalTouchedData *>(ref);
#else
						auto data = dynamic_cast<agtk::PortalTouchedData *>(ref);
#endif
						auto object = data->getObject();
						auto moveToSceneLayerId = data->getsceneLayerId();

						// 出現位置取得
						auto appearPos = data->getAppearPosition();

						// 新しいシーンへ移動した場合
						if (isNewScene) {
							// 新しいシーンへオブジェクトを登録する
							auto sceneLayer = nextScene->getSceneLayer(moveToSceneLayerId);
							if (sceneLayer == nullptr) continue;
							auto connectObjectPortalLoadList = data->getObject()->getConnectObjectPortalLoadList();// ポータル移動時の接続オブジェクトリストを生成オブジェクトに設定する。
							object = nextScene->getSceneLayer(moveToSceneLayerId)->addOtherSceneObject(data->getObject(), appearPos);
							object->setConnectObjectPortalLoadList(connectObjectPortalLoadList);
							newObjectList->addObject(object);
							oldObjectList->addObject(data->getObject());
							cocos2d::Ref * ref3 = nullptr;
							{
								auto particleManager = ParticleManager::getInstance();
								auto list = data->getParticleGroupBackupList();
								if (list != nullptr) {
									CCARRAY_FOREACH(list, ref3) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
										auto backup = static_cast<ParticleManager::ParticleGroupBackup *>(ref3);
#else
										auto backup = dynamic_cast<ParticleManager::ParticleGroupBackup *>(ref3);
#endif
										particleManager->addParticle(object, nextScene->getSceneData()->getId(), moveToSceneLayerId, backup);
									}
								}
							}
							{
								auto effectManager = EffectManager::getInstance();
								auto list = data->getEffectBackupList();
								if (list != nullptr) {
									CCARRAY_FOREACH(list, ref3) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
										auto backup = static_cast<EffectManager::EffectBackup *>(ref3);
#else
										auto backup = dynamic_cast<EffectManager::EffectBackup *>(ref3);
#endif
										effectManager->addEffectAnimation(object, backup);
									}
								}
							}
#ifdef USE_ACT2_5389
							std::function<void(agtk::Object*, int, const cocos2d::Vec2 &, agtk::Object *)> createChildObjectInfoListRecur = [&](agtk::Object *object, int moveToSceneLayerId, const cocos2d::Vec2 &appearPos, agtk::Object *newObject) {
								if (object->getChildrenObjectList()->count() > 0) {
									cocos2d::Ref *ref2;
									auto childObjectList = object->getChildrenObjectList();
									CCARRAY_FOREACH(childObjectList, ref2) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
										auto child = static_cast<agtk::Object*>(ref2);
#else
										auto child = dynamic_cast<agtk::Object*>(ref2);
#endif
										auto newChild = nextScene->getSceneLayer(moveToSceneLayerId)->addOtherSceneObject(child, appearPos, object);
										if (newChild == nullptr) continue;
										newObjectList->addObject(newChild);
										oldObjectList->addObject(child);
										newObject->addChildObject(newChild, child->getParentFollowPosOffset(), child->getParentFollowConnectId());
										createChildObjectInfoListRecur(child, moveToSceneLayerId, appearPos, newChild);
									}
								}
							};
							createChildObjectInfoListRecur(data->getObject(), moveToSceneLayerId, appearPos, object);
#else
							if (data->getObject()->getChildrenObjectList()->count() > 0) {
								cocos2d::Ref *ref2;
								auto childObjectList = data->getObject()->getChildrenObjectList();
								CCARRAY_FOREACH(childObjectList, ref2) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
									auto child = static_cast<agtk::Object*>(ref2);
#else
									auto child = dynamic_cast<agtk::Object*>(ref2);
#endif
									auto newChild = nextScene->getSceneLayer(moveToSceneLayerId)->addOtherSceneObject(child, appearPos);
									if (newChild == nullptr) continue;
									newObjectList->addObject(newChild);
									oldObjectList->addObject(child);
									object->addChildObject(newChild, child->getParentFollowPosOffset(), child->getParentFollowConnectId());
								}
							}
#endif
							if (object == nullptr) continue;
						}
						// 同一シーンでレイヤーが異なる場合
						else if (object->getLayerId() != moveToSceneLayerId) {
							object->setConnectObjectPortalLoadList(std::vector<agtk::Object::ConnectObjectLoadList>());	//同一シーン移動なので、子オブジェクトは破棄されない。そのため、新たに生成されないようにする。 

							// レイヤーを移動
							// ※レイヤー移動でコリジョンコンポーネントがおかしくなるので入れ替える必要がある
							std::function<void(agtk::Object *)> changeSceneLayer = [&](agtk::Object *obj) {

								//detach now layer
								obj->setRemoveLayerMoveFlag(true);
								obj->removeSelf(false);
								obj->setRemoveLayerMoveFlag(false);

								//attach next layer
								auto nextSceneLayer = nextScene->getSceneLayer(moveToSceneLayerId);
								nextSceneLayer->getObjectList()->addObject(obj);
								obj->setLayerId(moveToSceneLayerId);
								obj->setSceneLayer(nextSceneLayer);

								nextSceneLayer->addCollisionDetaction(obj);
								obj->setupPhysicsBody(true);
								obj->setPhysicsBitMask(moveToSceneLayerId, nextScene->getSceneData()->getId());
								// オブジェクトに紐付いた物理オブジェクトの生成
								nextSceneLayer->createPhysicsObjectWithObject(obj);
// #AGTK-NX
#ifndef USE_SAR_OPTIMIZE_1
								nextSceneLayer->addObject(obj);
#endif

								// インスタンスIDを再設定。
								auto playObjectData = obj->getPlayObjectData();
								playObjectData->setInstanceId(scene->getObjectInstanceId(obj));
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_1
								// InstanceId設定後にSceneLayerに追加
								nextSceneLayer->addObject(obj);
#endif
							};
							changeSceneLayer(object);
							newObjectList->addObject(object);
#ifdef USE_ACT2_5389
							std::function<void(agtk::Object*)> createChildObjectInfoListRecur2 = [&](agtk::Object *object) {
								if (object->getChildrenObjectList()->count() > 0) {
									cocos2d::Ref *ref2;
									auto childObjectList = object->getChildrenObjectList();
									CCARRAY_FOREACH(childObjectList, ref2) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
										auto child = static_cast<agtk::Object *>(ref2);
#else
										auto child = dynamic_cast<agtk::Object *>(ref2);
#endif
										changeSceneLayer(child);
										newObjectList->addObject(child);
										createChildObjectInfoListRecur2(child);
									}
								}
							};
							createChildObjectInfoListRecur2(data->getObject());
#else
							if (data->getObject()->getChildrenObjectList()->count() > 0) {
								cocos2d::Ref *ref2;
								auto childObjectList = data->getObject()->getChildrenObjectList();
								CCARRAY_FOREACH(childObjectList, ref2) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
									auto child = static_cast<agtk::Object *>(ref2);
#else
									auto child = dynamic_cast<agtk::Object *>(ref2);
#endif
									changeSceneLayer(child);
									newObjectList->addObject(child);
								}
							}
#endif
						}
						// 同一シーンで同一レイヤーの場合。
						else if(object->getLayerId() == moveToSceneLayerId) {
							object->setConnectObjectPortalLoadList(std::vector<agtk::Object::ConnectObjectLoadList>());	//同一シーンで同一レイヤー移動なので、子オブジェクトは破棄されない。そのため、新たに生成されないようにする。
							newObjectList->addObject(object);
#ifdef USE_ACT2_5389
							std::function<void(agtk::Object*)> createChildObjectInfoListRecur3 = [&](agtk::Object *object) {
								if (object->getChildrenObjectList()->count() > 0) {
									cocos2d::Ref *ref2;
									auto childObjectList = object->getChildrenObjectList();
									CCARRAY_FOREACH(childObjectList, ref2) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
										auto child = static_cast<agtk::Object *>(ref2);
#else
										auto child = dynamic_cast<agtk::Object *>(ref2);
#endif
										newObjectList->addObject(child);
									}
								}
							};
							createChildObjectInfoListRecur3(data->getObject());
#else
							if (data->getObject()->getChildrenObjectList()->count() > 0) {
								cocos2d::Ref *ref2;
								auto childObjectList = data->getObject()->getChildrenObjectList();
								CCARRAY_FOREACH(childObjectList, ref2) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
									auto child = static_cast<agtk::Object *>(ref2);
#else
									auto child = dynamic_cast<agtk::Object *>(ref2);
#endif
									newObjectList->addObject(child);
								}
							}
#endif
						}

						if (newObjectList->count() > 0) {
							std::function<void(agtk::Object *)> objectAfterSetting = [&](agtk::Object *obj) {
								// --------------------------------------------------------------
								// 移動先の位置を設定
								auto physicsNode = obj->getphysicsNode();
								// 物理ノードがある場合
								if (physicsNode) {
									// 現在地から移動先へのベクトルを取得
									auto cocosAppearPos = agtk::Scene::getPositionCocos2dFromScene(appearPos, nextScene);
									auto moveVec = cocosAppearPos - physicsNode->getPosition();
									// 物理ノードの位置を設定
									physicsNode->setPosition(cocosAppearPos);
									// 物理付きオブジェクトに紐付いた物理オブジェクトを移動
									cocos2d::Ref *ref = nullptr;
									auto physicsBaseList = obj->getPinAxisConnectedPhysicsBaseList();
									CCARRAY_FOREACH(physicsBaseList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
										auto parts = static_cast<agtk::PhysicsBase *>(ref);
#else
										auto parts = dynamic_cast<agtk::PhysicsBase *>(ref);
#endif
										parts->setPosition(parts->getPosition() + moveVec);
									}
								}
								obj->setPosition(appearPos);
								obj->setOldPosition(appearPos);
								obj->setPremoveObjectPosition(appearPos);

								// 位置が強制的に変更されるので保持する値を上書き
								// 速度は移動前を維持するため上書きしない
								auto playObjectData = obj->getPlayObjectData();
								playObjectData->getVariableData(agtk::data::kObjectSystemVariableX)->setExternalValue(appearPos.x);
								playObjectData->getVariableData(agtk::data::kObjectSystemVariableY)->setExternalValue(appearPos.y);

								// ワープ直後フラグON
								obj->setIsPortalWarped(true);
#ifdef FIX_ACT2_4774
								obj->setWarpedTransitionPortalId(gm->getTransitionPortalId());
#endif
								//強制的に非表示状態にする。
								obj->setForceVisibleStateAll(agtk::Object::ForceVisibleState::kNotVisible);

								// 表示までのディレイを設定
								float objectDelayTime = (data->getVisibleDelay300() * apperNumper) / 300.0f + effectData->getPostMoveDuration300() / 300.0f;
								obj->runAction(Sequence::create(
									CCCallFunc::create([obj]() {
										// ACT2-4880 DisabledからWaitDuration300で対応
										obj->setWaitDuration300All(-1);
									}),
									DelayTime::create(objectDelayTime),
									CCCallFunc::create([obj, isNewScene, nextScene]() {
										// ACT2-4880 DisabledからWaitDuration300で対応
										obj->setWaitDuration300All(0);

										//強制表示状態を無効にする。
										//※接続オブジェクトがある場合は、親オブジェクトと同様に強制表示状態を無効にする。
										obj->setForceVisibleStateAll(agtk::Object::ForceVisibleState::kIgnore);

										// 別のシーンへ遷移した場合
										if (isNewScene) {
											// オブジェクトに紐付いた物理オブジェクトを生成
											nextScene->getSceneLayer(obj->getLayerId())->createPhysicsObjectWithObject(obj);
										}
									}),
									nullptr
								));

							};
							cocos2d::Ref *ref2;
							CCARRAY_FOREACH(newObjectList, ref2) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
								auto obj = static_cast<agtk::Object *>(ref2);
#else
								auto obj = dynamic_cast<agtk::Object *>(ref2);
#endif
								objectAfterSetting(obj);
							}
						}
						// 連結スライドの場合
						if (isSlideLinkEffect) {
							// 移動元と移動先のポータルの中心座標を保持
							srcPortalCenterPos = data->getSrcPortalCenterPos();
							dstPortalCenterPos = agtk::Scene::getPositionCocos2dFromScene(data->getDstPortalCenterPos(), nextScene);
						}
						// 次の出現番号へ
						apperNumper++;
					}

					// ロック情報を再登録する。
					if (isNewScene && oldObjectList->count() > 0) {
						for (int i = 0; i < newObjectList->count(); i++) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
							auto newObject = static_cast<agtk::Object *>(newObjectList->getObjectAtIndex(i));
							auto oldObject = static_cast<agtk::Object *>(oldObjectList->getObjectAtIndex(i));
#else
							auto newObject = dynamic_cast<agtk::Object *>(newObjectList->getObjectAtIndex(i));
							auto oldObject = dynamic_cast<agtk::Object *>(oldObjectList->getObjectAtIndex(i));
#endif
							auto newPlayObjectData = newObject->getPlayObjectData();
							auto oldPlayObjectData = oldObject->getPlayObjectData();
							for (int j = 0; j < oldObjectList->count(); j++) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
								auto oldObject2 = static_cast<agtk::Object *>(oldObjectList->getObjectAtIndex(j));
								auto newObject2 = static_cast<agtk::Object *>(newObjectList->getObjectAtIndex(j));
#else
								auto oldObject2 = dynamic_cast<agtk::Object *>(oldObjectList->getObjectAtIndex(j));
								auto newObject2 = dynamic_cast<agtk::Object *>(newObjectList->getObjectAtIndex(j));
#endif
								if (oldObject == oldObject2) continue;
								auto oldPlayObjectData2 = oldObject2->getPlayObjectData();
								auto newPlayObjectData2 = newObject2->getPlayObjectData();
								if (oldPlayObjectData2->isLocked(oldObject->getInstanceId())) {
									newPlayObjectData2->addLocking(newObject->getInstanceId());
									newPlayObjectData2->setLockTarget(true);
								}
							}
						}
					}

					// カメラを更新
					nextScene->getCamera()->update(0.1f);
				}

				if (nextScene) {
					// シーンを更新
					if (isNewScene) {
						this->onEnter();
					}

					bool isPortalAndSlideLinkEffect = (isPortal && isSlideLinkEffect);
					auto camera = nextScene->getCamera();
					auto cameraPos = camera->getCameraCenterPos();

					if (agtk::DebugManager::getInstance()->getSkipOneFrameWhenSceneCreatedIgnored() == false) {
						
						// ポータル移動 ＆ 連結スライド演出の場合
						if (isPortalAndSlideLinkEffect) {

							// 前シーンのカメラの回転・考慮したオフセット座標を算出
							auto offset = camRotQuat.getInversed() * Vec3((this->getPreSceneCameraPos().x - srcPortalCenterPos.x) / this->getPreSceneCameraScale().x * camera->getCamera()->getScaleX(),
								(this->getPreSceneCameraPos().y - srcPortalCenterPos.y) / this->getPreSceneCameraScale().y * camera->getCamera()->getScaleY(),
								0);

							// 飛び先ポータルの中心と飛び元ポータルの中心をスライド方向に合わせて軸合わせする
							switch (effectType) {
								// 左右連結
							case agtk::data::EnumMoveEffect::kMoveEffectSlideLeftLink:
							case agtk::data::EnumMoveEffect::kMoveEffectSlideRightLink:
							{
								cameraPos.y = dstPortalCenterPos.y + offset.y;
							} break;
							// 上下連結
							case agtk::data::EnumMoveEffect::kMoveEffectSlideUpLink:
							case agtk::data::EnumMoveEffect::kMoveEffectSlideDownLink:
							{
								cameraPos.x = dstPortalCenterPos.x + offset.x;
							} break;
							}
						}

						auto updateCameraForce = [&](agtk::Camera *camera, cocos2d::Vec2 cameraPos) {
							if (isPortalAndSlideLinkEffect && cameraPos != camera->getPosition()) {
								camera->setIgnoreCorrectionPos(true);
								camera->setPosition(cameraPos);
								camera->update(0.0f);
								camera->setIgnoreCorrectionPos(false);
							}
						};

						//生成時に１フレームスキップする。
						auto director = Director::getInstance();
						auto duration = camera->getMoveDuration();
						camera->setMoveDuration(0.0f);//※カメラの移動補完の値を0にして、即指定カメラ位置へ移動。
						auto framePerSeconds = 0.0f;// director->getAnimationInterval();
						//※１フレームスキップするために、ここで２回更新する。
						//※更新中にオブジェクトが消滅すると該当のレンダリングコマンドが死亡するのでレンダリングも挟む
						auto alivePhysicsObj = gm->getAlivePhysicsObj();
						nextScene->setSceneCreateSkipFrameFlag(true);
						{
							updateCameraForce(camera, cameraPos);//一時的にカメラ位置を強制設定。
							nextScene->update(framePerSeconds);
							director->getRenderer()->render();
							updateCameraForce(camera, cameraPos);//一時的にカメラ位置を強制設定。
							nextScene->update(framePerSeconds);
							director->getRenderer()->render();
						}
						nextScene->setSceneCreateSkipFrameFlag(false);
						camera->setMoveDuration(duration);
						gm->setAlivePhysicsObj(alivePhysicsObj);
					}

					// ポータル移動 ＆ 連結スライド演出の場合
					if (isPortalAndSlideLinkEffect) {
						// 新しいシーンのカメラ位置を補正。
						// 飛び先画面の連結部分を連結方向によって合わせるための調整。
						auto newCameraPos = camera->getCameraCenterPos();

						switch (effectType) {
						// 左右連結
						case agtk::data::EnumMoveEffect::kMoveEffectSlideLeftLink:
						case agtk::data::EnumMoveEffect::kMoveEffectSlideRightLink:
							cameraPos.x = newCameraPos.x;
							break;
						// 上下連結
						case agtk::data::EnumMoveEffect::kMoveEffectSlideUpLink:
						case agtk::data::EnumMoveEffect::kMoveEffectSlideDownLink:
							cameraPos.y = newCameraPos.y;
							break;
						}

						camera->setIgnoreCorrectionPos(true);
						camera->setPosition(cameraPos);
						camera->update(0.1f);
						nextScene->updateSceneRenderTextureList(0.1f, false);
						camera->setIgnoreCorrectionPos(false);
					}
				}

				// ポータル移動の場合
				if (isPortal) {

					// 接触したプレイヤーリストをクリア
					if (gm->getPortalTouchedPlayerList()) {
						gm->getPortalTouchedPlayerList()->removeAllObjects();
						gm->setPortalTouchedPlayerList(nullptr);
					}
				}

				// スイッチ、変数を変更する場合
				if (effectData->getPostMoveChangeSwitchVariable()) {
					// スイッチ、変数を変更
					GameManager::EnumPlaceType placeType = GameManager::kPlaceMax;
					int id2 = -1;
					int id3 = -1;
					if (isPortal) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
						auto moveSettingData = static_cast<agtk::data::MoveSettingData *>(effectData);
#else
						auto moveSettingData = dynamic_cast<agtk::data::MoveSettingData *>(effectData);
#endif
						placeType = GameManager::kPlacePortal;
						id2 = moveSettingData->getAbId();
						id3 = 1;
					}
					else {
						placeType = GameManager::kPlaceTransitionLink;
					}
					GameManager::getInstance()->calcSwichVariableChange(effectData->getPostMoveSwitchVariableAssignList(), placeType, effectData->getId(), id2, id3);
					// 変数・スイッチ変更時のオブジェクトに対して変更処理。
					GameManager::getInstance()->updateObjectVariableAndSwitch();
				}
				
				// シーン切り替え後演出開始
				this->calcMoveEffect(false, isPortal, false);
				_initialCalcMoveEffect = false;
			}
			// --------------------------------------------------------------------
			// シーン切り替え後演出後の場合
			// --------------------------------------------------------------------
			else if (this->_sceneChangeState == SceneChangeState::POSTMOVE) {
				CCLOG("-- from POST_MOVE");
				AGTK_DEBUG_ACTION_LOG("%d,%s", __LINE__, __FUNCTION__);

				// 移動用オブジェクトを削除
				cocos2d::Ref *ref = nullptr;
				CCARRAY_FOREACH(this->getMoveObjectList(), ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
					auto slideLinkData = static_cast<SlideLinkData *>(ref);
#else
					auto slideLinkData = dynamic_cast<SlideLinkData *>(ref);
#endif
					auto moveTarget = slideLinkData->getMoveTarget();
					this->getMoveObjectParent()->removeChild(moveTarget);
				}
				this->getMoveObjectList()->removeAllObjects();

				// シーン演出ステートを設定
				this->_sceneChangeState = SceneChangeState::NONE;

				// オブジェクトのアクションプログラムを実行する場合
				if (effectData->getPostMoveExecuteObjectAction()) {
					// オブジェクトのアクションプログラムを実行
					GameManager::getInstance()->calcObjectActionProgram(effectData->getPostMoveObjectActionList());
				}

				// カレントシーンリンクデータリスト更新
				this->updateCurSceneLinkDataList();

				// ポータル遷移中フラグをOFF
				gm->setIsPortalMoving(false);
				// シーン遷移中フラグをOFF
				gm->setIsSceneMoving(false);

#ifdef USE_PREVIEW
				// 自動テスト開始
				if (gm->getStartSceneFlg()) {
					gm->setStartSceneFlg(false);
					// リプレイファイル読み込み
					std::string path = gm->getAutoTestReplayFilePath();
					gm->dropFileHandler(path.c_str());
				}
#endif
			}
		});

		//リソース読み込み（非同期）
		auto loadResourcesCallback = CallFunc::create([&](void) {
			auto gameManager = GameManager::getInstance();
			if (this->_sceneChangeState == SceneChangeState::PREMOVE) {
				gameManager->loadResources(gameManager->getNextSceneId());
// #AGTK-NX #AGTK-WIN
#ifdef USE_CLEAR_UNUSED_TEX
				// 前シーンの不要なテクスチャキャッシュをクリア
				gameManager->markLoadResources();
				CCDirector::getInstance()->getTextureCache()->removeUnusedVolatileTextures();
				gameManager->unmarkLoadResources();
				//CCLOG("# texInfo %s", CCDirector::getInstance()->getTextureCache()->getCachedTextureInfo().c_str());
#endif
			}
		});
		//リソース読み込み終了チェック。
		auto checkEndLoadResourcesCallback = agtk::IfCallFunc::create([&]() {
			if (this->_sceneChangeState == SceneChangeState::POSTMOVE) {
				return true;
			}
			auto debugManager = agtk::DebugManager::getInstance();
			auto gameManager = GameManager::getInstance();
			auto loadingScene = gameManager->getLoadingScene();
			if (loadingScene == nullptr) {
				return true;
			}
			auto state = gameManager->getStateLoading();
			switch (state) {
			case GameManager::kStateLoadingNone: {
				gameManager->setStateLoading(GameManager::kStateLoadingTimerStart);
				break; }
			case GameManager::kStateLoadingTimerStart: {
				gameManager->getLoadingTimer()->start();
				gameManager->setStateLoading(GameManager::kStateLoadingTimerUpdate);
				break; }
			case GameManager::kStateLoadingTimerUpdate: {
				if (debugManager->getShowLoadingScene()) {
					if (gameManager->getLoadingTimer()->getSeconds() > 1.0f) {//一秒
						gameManager->setStateLoading(GameManager::kStateLoadingShowStart);
					}
				}
				else {
					if (gameManager->getStateLoadResouces() != GameManager::EnumStateLoadResources::LOAD) {
						gameManager->setStateLoading(GameManager::kStateLoadingTimerEnd);
					} else
					if (gameManager->getLoadingTimer()->getSeconds() > 1.0f) {//一秒
						gameManager->setStateLoading(GameManager::kStateLoadingShowStart);
					}
				}
				break; }
			case GameManager::kStateLoadingTimerEnd: {
				break; }
			case GameManager::kStateLoadingShowStart: {
				auto currentScene = this->getScene();
				if (currentScene) {
					currentScene->stop();
				}
				this->addChild(loadingScene, ZOrder::Fade, Tag::Scene);
				loadingScene->start(this);
				this->onEnter();
				gameManager->setStateLoading(GameManager::kStateLoadingShowUpdate);
				break; }
			case GameManager::kStateLoadingShowUpdate: {
				if (debugManager->getShowLoadingScene()) {
					if (gameManager->getLoadingTimer()->getSeconds() > 2.0f) {
						loadingScene->stop();
						this->removeChild(loadingScene);
						gameManager->setStateLoading(GameManager::kStateLoadingShowEnd);
					}
				}
				else {
					if (gameManager->getStateLoadResouces() != GameManager::EnumStateLoadResources::LOAD) {
						loadingScene->stop();
						this->removeChild(loadingScene);
						gameManager->setStateLoading(GameManager::kStateLoadingShowEnd);
					}
				}
				break; }
			case GameManager::kStateLoadingShowEnd: {
				break; }
			}
			if (gameManager->getStateLoading() == GameManager::kStateLoadingTimerEnd || gameManager->getStateLoading() == GameManager::kStateLoadingShowEnd) {
				gameManager->setStateLoading(GameManager::kStateLoadingNone);
				gameManager->getLoadingTimer()->reset();
				return true;
			}
			return false;
		});

		// ------------------------------------------------------
		// 演出対象の画面をキャプチャ
		// ------------------------------------------------------
		auto director = Director::getInstance();
		auto renderer = director->getRenderer();
		auto targetFadeSprite = isPreMove ? _fadeSpriteBefore : _fadeSpriteAfter;
#ifdef FIX_ACT2_5233
		auto targetFadeFrontSprite = isPreMove ? _fadeFrontSpriteBefore : _fadeFrontSpriteAfter;
#endif

		if (scene) {
			// ---------------------------------------------- //
			// シーンを演出用レンダーテクスチャに描画         //
			// ---------------------------------------------- //
			cocos2d::Node *invisibleNode = nullptr;
			if(!isPreMove &&
				(effectType == agtk::data::EnumMoveEffect::kMoveEffectSlideDownLink ||
				effectType == agtk::data::EnumMoveEffect::kMoveEffectSlideUpLink ||
				effectType == agtk::data::EnumMoveEffect::kMoveEffectSlideLeftLink ||
				effectType == agtk::data::EnumMoveEffect::kMoveEffectSlideRightLink)) {
				invisibleNode = this->getMoveObjectParent();
			}
			if (invisibleNode) {
				invisibleNode->setVisible(false);
			}
#ifdef FIX_ACT2_5233
			RenderSceneToFadeSprite(camera, targetFadeSprite, targetFadeFrontSprite, objectLayerId, scene, renderer, effectType);
#else
			RenderSceneToFadeSprite(camera, targetFadeSprite, scene, renderer, effectType);
#endif
			if (invisibleNode) {
				invisibleNode->setVisible(true);
			}
		}
		else {
			targetFadeSprite->clear(0, 0, 0, 1);
#ifdef FIX_ACT2_5233
			targetFadeFrontSprite->clear(0, 0, 0, 0);
#endif
		}

		targetFadeSprite->getSprite()->setAnchorPoint(camAnchor);
		targetFadeSprite->setScale(camScale.x, camScale.y);
#ifdef FIX_ACT2_5233
		targetFadeFrontSprite->getSprite()->setAnchorPoint(camAnchor);
		targetFadeFrontSprite->setScale(camScale.x, camScale.y);
#endif

		// 強制レンダリング
		renderer->render();

		_layerColor->setOpacity(255);
		_layerColor->setAnchorPoint(camAnchor);
		_layerColor->setScale(camScale.x, camScale.y);
		_layerColor->setVisible(true);
		targetFadeSprite->setVisible(true);
#ifdef FIX_ACT2_5233
		targetFadeFrontSprite->setVisible(true);
#endif

		// ------------------------------------------------------
		// 演出タイプ別処理
		// ------------------------------------------------------
		switch (effectType)
		{
			// -------------------------------------------------
			// 無し
			// -------------------------------------------------
		default:
		case agtk::data::EnumMoveEffect::kMoveEffectNone:
			_layerColor->setColor(Color3B::BLACK);
#ifdef FIX_ACT2_5233
			_layerColor->setOpacity(0);
#else
			targetFadeSprite->setOpacity(255);
#endif
			targetFadeSprite->setPositionY(0);
#ifdef FIX_ACT2_5233
			targetFadeFrontSprite->setPositionY(0);
#endif
			this->runAction(agtk::Sequence::create(
				loadResourcesCallback,
				DelayTime::create(effectDuration),
				checkEndLoadResourcesCallback,
				afterEffectCallback, nullptr)
			);
			break;
			// -------------------------------------------------
			// 黒フェード
			// -------------------------------------------------
		case agtk::data::EnumMoveEffect::kMoveEffectBlack:
			_layerColor->setColor(Color3B::BLACK);
			targetFadeSprite->setPositionY(0);
#ifdef FIX_ACT2_5233
			targetFadeFrontSprite->setPositionY(0);
#endif
			if (isPreMove) {
#ifdef FIX_ACT2_5233
				_layerColor->setOpacity(0);
				_layerColor->runAction(agtk::Sequence::create(
					loadResourcesCallback,
					FadeIn::create(effectDuration),
					checkEndLoadResourcesCallback,
					afterEffectCallback,
					nullptr
				));
#else
				targetFadeSprite->setOpacity(255);
				targetFadeSprite->runAction(agtk::Sequence::create(
					loadResourcesCallback,
					FadeOut::create(effectDuration),
					checkEndLoadResourcesCallback,
					afterEffectCallback,
					nullptr
				));
#endif
			}
			else {
#ifdef FIX_ACT2_5233
				_layerColor->setOpacity(255);
				_layerColor->runAction(agtk::Sequence::create(
					loadResourcesCallback,
					FadeOut::create(effectDuration),
					checkEndLoadResourcesCallback,
					afterEffectCallback,
					nullptr
				));
#else
				targetFadeSprite->setOpacity(0);
				targetFadeSprite->runAction(agtk::Sequence::create(
					loadResourcesCallback,
					FadeIn::create(effectDuration),
					checkEndLoadResourcesCallback,
					afterEffectCallback,
					nullptr
				));
#endif
			}
			break;
			// -------------------------------------------------
			// 白フェード
			// -------------------------------------------------
		case agtk::data::EnumMoveEffect::kMoveEffectWhite:
			_layerColor->setColor(Color3B::WHITE);
			targetFadeSprite->setPositionY(0);
#ifdef FIX_ACT2_5233
			targetFadeFrontSprite->setPositionY(0);
#endif
			if (isPreMove) {
#ifdef FIX_ACT2_5233
				_layerColor->setOpacity(0);
				_layerColor->runAction(agtk::Sequence::create(
					loadResourcesCallback,
					FadeIn::create(effectDuration),
					checkEndLoadResourcesCallback,
					afterEffectCallback,
					nullptr
				));
#else
				targetFadeSprite->setOpacity(255);
				targetFadeSprite->runAction(agtk::Sequence::create(
					loadResourcesCallback,
					FadeOut::create(effectDuration),
					checkEndLoadResourcesCallback,
					afterEffectCallback,
					nullptr
				));
#endif
			}
			else {
#ifdef FIX_ACT2_5233
				_layerColor->setOpacity(255);
				_layerColor->runAction(agtk::Sequence::create(
					loadResourcesCallback,
					FadeOut::create(effectDuration),
					checkEndLoadResourcesCallback,
					afterEffectCallback,
					nullptr
				));
#else
				targetFadeSprite->setOpacity(0);
				targetFadeSprite->runAction(agtk::Sequence::create(
					loadResourcesCallback,
					FadeIn::create(effectDuration),
					checkEndLoadResourcesCallback,
					afterEffectCallback,
					nullptr
				));
#endif
			}
			break;
			// -------------------------------------------------
			// スライドアップ
			// -------------------------------------------------
		case agtk::data::EnumMoveEffect::kMoveEffectSlideUp:
		{
			auto moveDiff = visibleSize.height * targetFadeSprite->getScaleY();
			_layerColor->setColor(Color3B::BLACK);
#ifdef FIX_ACT2_5233
			_layerColor->setOpacity(0);
#else
			targetFadeSprite->setOpacity(255);
#endif
			if (isPreMove) {
				targetFadeSprite->setPositionY(0);
#ifdef FIX_ACT2_5233
				targetFadeFrontSprite->setPositionY(0);
#endif
				targetFadeSprite->runAction(agtk::Sequence::create(
					loadResourcesCallback,
#ifdef FIX_ACT2_5233
					Spawn::create(
						TargetedAction::create(targetFadeSprite, MoveBy::create(effectDuration, cocos2d::Vec2(0, moveDiff))),
						TargetedAction::create(targetFadeFrontSprite, MoveBy::create(effectDuration, cocos2d::Vec2(0, moveDiff))),
						nullptr),
#else
					MoveBy::create(effectDuration, cocos2d::Vec2(0, moveDiff)),
#endif
					checkEndLoadResourcesCallback,
					afterEffectCallback,
					nullptr
				));
			}
			else {
				targetFadeSprite->setPositionY(targetFadeSprite->getPositionY() - moveDiff);
#ifdef FIX_ACT2_5233
				targetFadeFrontSprite->setPositionY(targetFadeFrontSprite->getPositionY() - moveDiff);
#endif
				targetFadeSprite->runAction(agtk::Sequence::create(
					loadResourcesCallback,
#ifdef FIX_ACT2_5233
					Spawn::create(
						TargetedAction::create(targetFadeSprite, MoveBy::create(effectDuration, cocos2d::Vec2(0, moveDiff))),
						TargetedAction::create(targetFadeFrontSprite, MoveBy::create(effectDuration, cocos2d::Vec2(0, moveDiff))),
						nullptr),
#else
					MoveBy::create(effectDuration, cocos2d::Vec2(0, moveDiff)),
#endif
					checkEndLoadResourcesCallback,
					afterEffectCallback,
					nullptr
				));
			}
		}	break;
			// -------------------------------------------------
			// スライドダウン
			// -------------------------------------------------
		case agtk::data::EnumMoveEffect::kMoveEffectSlideDown:
		{
			auto moveDiff = visibleSize.height * targetFadeSprite->getScaleY();
			_layerColor->setColor(Color3B::BLACK);
#ifdef FIX_ACT2_5233
			_layerColor->setOpacity(0);
#else
			targetFadeSprite->setOpacity(255);
#endif
			if (isPreMove) {
				targetFadeSprite->setPositionY(0);
#ifdef FIX_ACT2_5233
				targetFadeFrontSprite->setPositionY(0);
#endif
				targetFadeSprite->runAction(agtk::Sequence::create(
					loadResourcesCallback,
#ifdef FIX_ACT2_5233
					Spawn::create(
						TargetedAction::create(targetFadeSprite, MoveBy::create(effectDuration, cocos2d::Vec2(0, -moveDiff))),
						TargetedAction::create(targetFadeFrontSprite, MoveBy::create(effectDuration, cocos2d::Vec2(0, -moveDiff))),
						nullptr),
#else
					MoveBy::create(effectDuration, cocos2d::Vec2(0, -moveDiff)),
#endif
					checkEndLoadResourcesCallback,
					afterEffectCallback,
					nullptr
				));
			}
			else {
				targetFadeSprite->setPositionY(targetFadeSprite->getPositionY() + moveDiff);
#ifdef FIX_ACT2_5233
				targetFadeFrontSprite->setPositionY(targetFadeFrontSprite->getPositionY() + moveDiff);
#endif
				targetFadeSprite->runAction(agtk::Sequence::create(
					loadResourcesCallback,
#ifdef FIX_ACT2_5233
					Spawn::create(
						TargetedAction::create(targetFadeSprite, MoveBy::create(effectDuration, cocos2d::Vec2(0, -moveDiff))),
						TargetedAction::create(targetFadeFrontSprite, MoveBy::create(effectDuration, cocos2d::Vec2(0, -moveDiff))),
						nullptr),
#else
					MoveBy::create(effectDuration, cocos2d::Vec2(0, -moveDiff)),
#endif
					checkEndLoadResourcesCallback,
					afterEffectCallback,
					nullptr
				));
			}
		}	break;
			// -------------------------------------------------
			// スライドレフト
			// -------------------------------------------------
		case agtk::data::EnumMoveEffect::kMoveEffectSlideLeft:
		{
			auto moveDiff = visibleSize.width * targetFadeSprite->getScaleX();
			_layerColor->setColor(Color3B::BLACK);
#ifdef FIX_ACT2_5233
			_layerColor->setOpacity(0);
#else
			targetFadeSprite->setOpacity(255);
#endif
			if (isPreMove) {
				targetFadeSprite->setPositionX(0);
#ifdef FIX_ACT2_5233
				targetFadeFrontSprite->setPositionX(0);
#endif
				targetFadeSprite->runAction(agtk::Sequence::create(
					loadResourcesCallback,
#ifdef FIX_ACT2_5233
					Spawn::create(
						TargetedAction::create(targetFadeSprite, MoveBy::create(effectDuration, cocos2d::Vec2(-moveDiff, 0))),
						TargetedAction::create(targetFadeFrontSprite, MoveBy::create(effectDuration, cocos2d::Vec2(-moveDiff, 0))),
						nullptr),
#else
					MoveBy::create(effectDuration, cocos2d::Vec2(-moveDiff, 0)),
#endif
					checkEndLoadResourcesCallback,
					afterEffectCallback,
					nullptr
				));
			}
			else {
				targetFadeSprite->setPositionX(targetFadeSprite->getPositionX() + moveDiff);
#ifdef FIX_ACT2_5233
				targetFadeFrontSprite->setPositionX(targetFadeFrontSprite->getPositionX() + moveDiff);
#endif
				targetFadeSprite->runAction(agtk::Sequence::create(
					loadResourcesCallback,
#ifdef FIX_ACT2_5233
					Spawn::create(
						TargetedAction::create(targetFadeSprite, MoveBy::create(effectDuration, cocos2d::Vec2(-moveDiff, 0))),
						TargetedAction::create(targetFadeFrontSprite, MoveBy::create(effectDuration, cocos2d::Vec2(-moveDiff, 0))),
						nullptr),
#else
					MoveBy::create(effectDuration, cocos2d::Vec2(-moveDiff, 0)),
#endif
					checkEndLoadResourcesCallback,
					afterEffectCallback,
					nullptr
				));
			}
		}	break;
			// -------------------------------------------------
			// スライドライト
			// -------------------------------------------------
		case agtk::data::EnumMoveEffect::kMoveEffectSlideRight:
		{
			auto moveDiff = visibleSize.width * targetFadeSprite->getScaleX();
			_layerColor->setColor(Color3B::BLACK);
#ifdef FIX_ACT2_5233
			_layerColor->setOpacity(0);
#else
			targetFadeSprite->setOpacity(255);
#endif
			if (isPreMove) {
				targetFadeSprite->setPositionX(0);
#ifdef FIX_ACT2_5233
				targetFadeFrontSprite->setPositionX(0);
#endif
				targetFadeSprite->runAction(agtk::Sequence::create(
					loadResourcesCallback,
#ifdef FIX_ACT2_5233
					Spawn::create(
						TargetedAction::create(targetFadeSprite, MoveBy::create(effectDuration, cocos2d::Vec2(moveDiff, 0))),
						TargetedAction::create(targetFadeFrontSprite, MoveBy::create(effectDuration, cocos2d::Vec2(moveDiff, 0))),
						nullptr),
#else
					MoveBy::create(effectDuration, cocos2d::Vec2(moveDiff, 0)),
#endif
					checkEndLoadResourcesCallback,
					afterEffectCallback,
					nullptr
				));
			}
			else {
				targetFadeSprite->setPositionX(targetFadeSprite->getPositionX() - moveDiff);
#ifdef FIX_ACT2_5233
				targetFadeFrontSprite->setPositionX(targetFadeFrontSprite->getPositionX() - moveDiff);
#endif
				targetFadeSprite->runAction(agtk::Sequence::create(
					loadResourcesCallback,
#ifdef FIX_ACT2_5233
					Spawn::create(
						TargetedAction::create(targetFadeSprite, MoveBy::create(effectDuration, cocos2d::Vec2(moveDiff, 0))),
						TargetedAction::create(targetFadeFrontSprite, MoveBy::create(effectDuration, cocos2d::Vec2(moveDiff, 0))),
						nullptr),
#else
					MoveBy::create(effectDuration, cocos2d::Vec2(moveDiff, 0)),
#endif
					checkEndLoadResourcesCallback,
					afterEffectCallback,
					nullptr
				));
			}
		}	break;
			// -------------------------------------------------
			// スライドアップ(連結)
			// -------------------------------------------------
		case agtk::data::EnumMoveEffect::kMoveEffectSlideUpLink:
		{
#ifdef FIX_ACT2_5233
			_layerColor->setOpacity(0);
#endif
			if (isPreMove) {
				targetFadeSprite->setPositionY(0);
				targetFadeSprite->setOpacity(255);
#ifdef FIX_ACT2_5233
				targetFadeFrontSprite->setPositionY(0);
#endif
				targetFadeSprite->runAction(agtk::Sequence::create(
					loadResourcesCallback,
					checkEndLoadResourcesCallback,
					afterEffectCallback,
					nullptr
				));
			}
			else {
				auto scaleX = targetFadeSprite->getScaleX();
				auto scaleY = targetFadeSprite->getScaleY();
				_fadeSpriteBefore->setScale(scaleX, scaleY);
				_fadeSpriteBefore->getSprite()->setAnchorPoint(targetFadeSprite->getSprite()->getAnchorPoint());
				_fadeSpriteBefore->setPosition(targetFadeSprite->getPosition());
				_fadeSpriteBefore->setVisible(true);
				targetFadeSprite->setPositionY(targetFadeSprite->getPositionY() - visibleSize.height * scaleY);

				_fadeSpriteBefore->runAction(Sequence::create(
					MoveBy::create(effectDuration, cocos2d::Vec2(0, visibleSize.height * scaleY)),
					nullptr
				));

				targetFadeSprite->runAction(Sequence::create(
					MoveBy::create(effectDuration, cocos2d::Vec2(0, visibleSize.height * scaleY)),
					afterEffectCallback,
					nullptr
				));
#ifdef FIX_ACT2_5233
				_fadeFrontSpriteBefore->setScale(scaleX, scaleY);
				_fadeFrontSpriteBefore->getSprite()->setAnchorPoint(targetFadeFrontSprite->getSprite()->getAnchorPoint());
				_fadeFrontSpriteBefore->setPosition(targetFadeFrontSprite->getPosition());
				_fadeFrontSpriteBefore->setVisible(true);
				targetFadeFrontSprite->setPositionY(targetFadeFrontSprite->getPositionY() - visibleSize.height * scaleY);

				_fadeFrontSpriteBefore->runAction(Sequence::create(
					MoveBy::create(effectDuration, cocos2d::Vec2(0, visibleSize.height * scaleY)),
					nullptr
					));

				targetFadeFrontSprite->runAction(Sequence::create(
					MoveBy::create(effectDuration, cocos2d::Vec2(0, visibleSize.height * scaleY)),
					nullptr
					));
#endif
			}
		}	break;
			// -------------------------------------------------
			// スライドダウン(連結)
			// -------------------------------------------------
		case agtk::data::EnumMoveEffect::kMoveEffectSlideDownLink:
		{
			_layerColor->setColor(Color3B::BLACK);
#ifdef FIX_ACT2_5233
			_layerColor->setOpacity(0);
#endif
			if (isPreMove) {
				targetFadeSprite->setPositionY(0);
				targetFadeSprite->setOpacity(255);
#ifdef FIX_ACT2_5233
				targetFadeFrontSprite->setPositionY(0);
#endif
				targetFadeSprite->runAction(agtk::Sequence::create(
					loadResourcesCallback,
					checkEndLoadResourcesCallback,
					afterEffectCallback,
					nullptr
				));
			}
			else {
				auto scaleX = targetFadeSprite->getScaleX();
				auto scaleY = targetFadeSprite->getScaleY();
				_fadeSpriteBefore->setScale(scaleX, scaleY);
				_fadeSpriteBefore->getSprite()->setAnchorPoint(targetFadeSprite->getSprite()->getAnchorPoint());
				_fadeSpriteBefore->setPosition(targetFadeSprite->getPosition());
				_fadeSpriteBefore->setVisible(true);
				targetFadeSprite->setPositionY(targetFadeSprite->getPositionY() + visibleSize.height * scaleY);

				_fadeSpriteBefore->runAction(Sequence::create(
					MoveBy::create(effectDuration, cocos2d::Vec2(0, -visibleSize.height * scaleY)),
					nullptr
				));

				targetFadeSprite->runAction(Sequence::create(
					MoveBy::create(effectDuration, cocos2d::Vec2(0, -visibleSize.height * scaleY)),
					afterEffectCallback,
					nullptr
				));
#ifdef FIX_ACT2_5233
				_fadeFrontSpriteBefore->setScale(scaleX, scaleY);
				_fadeFrontSpriteBefore->getSprite()->setAnchorPoint(targetFadeFrontSprite->getSprite()->getAnchorPoint());
				_fadeFrontSpriteBefore->setPosition(targetFadeFrontSprite->getPosition());
				_fadeFrontSpriteBefore->setVisible(true);
				targetFadeFrontSprite->setPositionY(targetFadeFrontSprite->getPositionY() + visibleSize.height * scaleY);

				_fadeFrontSpriteBefore->runAction(Sequence::create(
					MoveBy::create(effectDuration, cocos2d::Vec2(0, -visibleSize.height * scaleY)),
					nullptr
					));

				targetFadeFrontSprite->runAction(Sequence::create(
					MoveBy::create(effectDuration, cocos2d::Vec2(0, -visibleSize.height * scaleY)),
					nullptr
					));
#endif
			}
		}	break;
			// -------------------------------------------------
			// スライドレフト(連結)
			// -------------------------------------------------
		case agtk::data::EnumMoveEffect::kMoveEffectSlideLeftLink:
		{
			_layerColor->setColor(Color3B::BLACK);
#ifdef FIX_ACT2_5233
			_layerColor->setOpacity(0);
#endif

			if (isPreMove) {
				targetFadeSprite->setPositionX(0);
				targetFadeSprite->setOpacity(255);
#ifdef FIX_ACT2_5233
				targetFadeFrontSprite->setPositionX(0);
#endif
				targetFadeSprite->runAction(agtk::Sequence::create(
					loadResourcesCallback,
					checkEndLoadResourcesCallback,
					afterEffectCallback,
					nullptr
				));
			}
			else {
				auto scaleX = targetFadeSprite->getScaleX();
				auto scaleY = targetFadeSprite->getScaleY();
				_fadeSpriteBefore->setScale(scaleX, scaleY);
				_fadeSpriteBefore->getSprite()->setAnchorPoint(targetFadeSprite->getSprite()->getAnchorPoint());
				_fadeSpriteBefore->setPosition(targetFadeSprite->getPosition());
				_fadeSpriteBefore->setVisible(true);
				targetFadeSprite->setPositionX(targetFadeSprite->getPositionX() + visibleSize.width * scaleX);

				_fadeSpriteBefore->runAction(Sequence::create(
					MoveBy::create(effectDuration, cocos2d::Vec2(-visibleSize.width * scaleX, 0)),
					nullptr
				));

				targetFadeSprite->runAction(Sequence::create(
					MoveBy::create(effectDuration, cocos2d::Vec2(-visibleSize.width * scaleX, 0)),
					afterEffectCallback,
					nullptr
				));
#ifdef FIX_ACT2_5233
				_fadeFrontSpriteBefore->setScale(scaleX, scaleY);
				_fadeFrontSpriteBefore->getSprite()->setAnchorPoint(targetFadeFrontSprite->getSprite()->getAnchorPoint());
				_fadeFrontSpriteBefore->setPosition(targetFadeFrontSprite->getPosition());
				_fadeFrontSpriteBefore->setVisible(true);
				targetFadeFrontSprite->setPositionX(targetFadeFrontSprite->getPositionX() + visibleSize.width * scaleX);

				_fadeFrontSpriteBefore->runAction(Sequence::create(
					MoveBy::create(effectDuration, cocos2d::Vec2(-visibleSize.width * scaleX, 0)),
					nullptr
					));

				targetFadeFrontSprite->runAction(Sequence::create(
					MoveBy::create(effectDuration, cocos2d::Vec2(-visibleSize.width * scaleX, 0)),
					nullptr
					));
#endif
			}
		}	break;
			// -------------------------------------------------
			// スライドライト(連結)
			// -------------------------------------------------
		case agtk::data::EnumMoveEffect::kMoveEffectSlideRightLink:
		{
			_layerColor->setColor(Color3B::BLACK);
#ifdef FIX_ACT2_5233
			_layerColor->setOpacity(0);
#endif

			if (isPreMove) {
				targetFadeSprite->setPositionX(0);
				targetFadeSprite->setOpacity(255);
#ifdef FIX_ACT2_5233
				targetFadeFrontSprite->setPositionX(0);
#endif
				targetFadeSprite->runAction(agtk::Sequence::create(
					loadResourcesCallback,
					checkEndLoadResourcesCallback,
					afterEffectCallback,
					nullptr
				));
			}
			else {
				auto scaleX = targetFadeSprite->getScaleX();
				auto scaleY = targetFadeSprite->getScaleY();
				_fadeSpriteBefore->setScale(scaleX, scaleY);
				_fadeSpriteBefore->getSprite()->setAnchorPoint(targetFadeSprite->getSprite()->getAnchorPoint());
				_fadeSpriteBefore->setPosition(targetFadeSprite->getPosition());
				_fadeSpriteBefore->setVisible(true);
				targetFadeSprite->setPositionX(targetFadeSprite->getPositionX() - visibleSize.width * scaleX);

				_fadeSpriteBefore->runAction(Sequence::create(
					MoveBy::create(effectDuration, cocos2d::Vec2(visibleSize.width * scaleX, 0)),
					nullptr
				));

				targetFadeSprite->runAction(Sequence::create(
					MoveBy::create(effectDuration, cocos2d::Vec2(visibleSize.width * scaleX, 0)),
					afterEffectCallback,
					nullptr
				));
#ifdef FIX_ACT2_5233
				_fadeFrontSpriteBefore->setScale(scaleX, scaleY);
				_fadeFrontSpriteBefore->getSprite()->setAnchorPoint(targetFadeFrontSprite->getSprite()->getAnchorPoint());
				_fadeFrontSpriteBefore->setPosition(targetFadeFrontSprite->getPosition());
				_fadeFrontSpriteBefore->setVisible(true);
				targetFadeFrontSprite->setPositionX(targetFadeFrontSprite->getPositionX() - visibleSize.width * scaleX);

				_fadeFrontSpriteBefore->runAction(Sequence::create(
					MoveBy::create(effectDuration, cocos2d::Vec2(visibleSize.width * scaleX, 0)),
					nullptr
					));

				targetFadeFrontSprite->runAction(Sequence::create(
					MoveBy::create(effectDuration, cocos2d::Vec2(visibleSize.width * scaleX, 0)),
					nullptr
					));
#endif
			}
		}	break;
		}
	}

	// シーンが存在する場合
	if (nullptr != scene) {

		auto camera = scene->getCamera();
		auto camPos = camera->getCameraCenterPos();

		// 遷移前の場合
		if (isPreMove) {
			// カメラの位置・スケール値・角度を記憶
			this->setPreSceneCameraPos(camPos);
			this->setPreSceneCameraScale(camScale);
			this->setPreSceneCameraRotationQuat(camRotQuat);

			// ポータル遷移 かつ スライド連結演出の場合
			if (isPortal && effectData->getPreMoveEffect() > agtk::data::EnumMoveEffect::kMoveEffectSlideRight) {

				// オブジェクトから移動用プレイヤーを生成するメソッド
				auto createPlayerFromObj = [](agtk::Object *object) -> agtk::Player* {

					auto objectAction = object->getCurrentObjectAction();
					int directionId = objectAction->getObjectActionData()->getAnimMotionId();

					// モーションが「設定しない」の場合
					if (directionId < 0) {
						auto player = object->getPlayer();
						if (player != nullptr) {
							directionId = player->getBasePlayer()->getCurrentAnimationMotion()->getMotionData()->getId();
						}
					}

					// オブジェクトを新規生成
					auto newObject = agtk::Object::create(
						object->getSceneLayer(),
						object->getObjectData()->getId(),
						object->getCurrentObjectAction()->getId(),
						object->getPosition(),
						Vec2(object->getScaleX(), object->getScaleY()),
						object->getRotation(),
						object->getDispDirection()
						, -1, -1, directionId
					);
					//※objectを生成してplayerのみ扱うのは特別な処理のため、ここでGuiManagerに登録したオブジェクト情報を破棄する。
					GuiManager::getInstance()->removeGui(newObject);
					newObject->setDisabled(object->getDisabled());

					auto player = newObject->getPlayer();
					if (player != nullptr) {
						player->setVisible(object->getPlayer()->isVisible());
						player->setParent(nullptr);
					}

					// ACT2-6250 「ポータル遷移時に素材セットが適応されない」の対応
					if (player != nullptr) {
						player->setResourceSetId(object->getResourceSetId());
					}
					newObject->setResourceSetId(object->getResourceSetId());

					return player;
				};

				// オブジェクトから移動用オブジェクトを生成
				cocos2d::Ref *ref = nullptr;
				CCARRAY_FOREACH(gm->getPortalTouchedPlayerList(), ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
					auto data = static_cast<agtk::PortalTouchedData *>(ref);
#else
					auto data = dynamic_cast<agtk::PortalTouchedData *>(ref);
#endif
					auto object = data->getObject();

					//オブジェクトのルート親を取得する。
					std::function<agtk::Object *(agtk::Object *)> getOwnParent = [&](agtk::Object *obj) -> agtk::Object* {
						auto parent = obj->getOwnParentObject();
						if (parent != nullptr) {
							auto obj = getOwnParent(parent);
							if (obj == nullptr) {
								return parent;
							}
							parent = obj;
							return parent;
						}
						return obj;
					};

					std::function<void(agtk::Object *, bool)> createPlayer = [&](agtk::Object *obj, bool bChildren) {
						//GuiList
						auto guiList = cocos2d::__Array::create();
						GuiManager::getInstance()->getGuiList(guiList, obj);

						auto player = obj->getPlayer();
						// 移動用プレイヤーを生成
						auto movePlayer = createPlayerFromObj(obj);
						if (movePlayer) {

							// カメラの回転と反転状態の Quaternion を生成
							Quaternion qt = camRotQuat.getInversed();

							// 演出用オブジェクトの回転状態を設定
							movePlayer->setRotationQuat(qt);

							// 移動元の座標を算出
							auto diff = agtk::Scene::getPositionCocos2dFromScene(obj->getPosition(), scene) - camPos;
							auto newPos = diff + (Vec2(visibleSize.width * camScale.x, visibleSize.height * camScale.y) * 0.5f) + camAnchorDiff;

							auto appearPos = data->getAppearPosition();
							if (bChildren) {
								auto parent = getOwnParent(obj);
								auto parentPos = parent->getPosition();
								auto childPos = obj->getPosition();
								appearPos += (childPos - parentPos);
							}

							// 移動元の座標を回転
							Vec3 tmpPos = qt * Vec3(newPos.x, newPos.y, 0);

							newPos.x = tmpPos.x;
							newPos.y = tmpPos.y;

							// 移動用オブジェクトを移動用ノードにアタッチ
							this->getMoveObjectParent()->addChild(movePlayer, obj->getObjectData()->getDispPriority());

							// 座標を設定
							movePlayer->setPosition(newPos);

							// スケールを設定
							movePlayer->setScale(player->getScaleX(), player->getScaleY());

							// オブジェクト移動演出が必要な場合
							if (data->getNeedObjectMoveEffect()) {
								movePlayer->setVisible(true);
							}

							// スライド連結演出用データを生成
							auto slideLinkData = GameScene::SlideLinkData::create(movePlayer, diff, appearPos);

							if (guiList->count() > 0) {
								cocos2d::Ref *ref;
								CCARRAY_FOREACH(guiList, ref) {
									auto gui = dynamic_cast<agtk::ObjectParameterGaugeUi *>(ref);
									if (gui != nullptr) {
										auto dummyGui = agtk::DummyParameterGaugeUi::create(movePlayer, gui);
										dummyGui->update(0.0f);
										slideLinkData->getGuiList()->addObject(dummyGui);
									}
								}
							}

							this->getMoveObjectList()->addObject(slideLinkData);
						}
					};

					createPlayer(object, false);
#ifdef USE_ACT2_5389
					std::function<void(agtk::Object*)> createPlayerRecur = [&](agtk::Object *object) {
						if (object->getChildrenObjectList()->count() > 0) {
							cocos2d::Ref *ref2;
							auto childrenObjectList = object->getChildrenObjectList();
							CCARRAY_FOREACH(childrenObjectList, ref2) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
								auto child = static_cast<agtk::Object *>(ref2);
#else
								auto child = dynamic_cast<agtk::Object *>(ref2);
#endif
								createPlayer(child, true);
								createPlayerRecur(child);
							}
						}
					};
					createPlayerRecur(object);
#else
					if (data->getObject()->getChildrenObjectList()->count() > 0) {
						cocos2d::Ref *ref2;
						auto childrenObjectList = data->getObject()->getChildrenObjectList();
						CCARRAY_FOREACH(childrenObjectList, ref2) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
							auto child = static_cast<agtk::Object *>(ref2);
#else
							auto child = dynamic_cast<agtk::Object *>(ref2);
#endif
							createPlayer(child, true);
						}
					}
#endif
				}
			}
		}
		// 遷移後 かつ ポータル遷移 かつ スライド連結演出の場合
		else if (isPortal && effectData->getPreMoveEffect() > agtk::data::EnumMoveEffect::kMoveEffectSlideRight) {

			auto effectDuration = effectData->getPreMoveDuration300() / 300.0f;
			auto screenCenter = (Vec2(visibleSize.width * camScale.x, visibleSize.height * camScale.y) * 0.5f);

			// 移動用オブジェクトの位置と移動先を設定
			cocos2d::Ref * ref = nullptr;
			CCARRAY_FOREACH(this->getMoveObjectList(), ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto slideLinkData = static_cast<SlideLinkData *>(ref);
#else
				auto slideLinkData = dynamic_cast<SlideLinkData *>(ref);
#endif
				auto moveObjPlayer = slideLinkData->getMoveTarget();

				// スケール再計算
				auto startScale = Vec2(camScale.x / _preSceneCameraScale.x * moveObjPlayer->getScaleX(), camScale.y / _preSceneCameraScale.y * moveObjPlayer->getScaleY());
				moveObjPlayer->setScale(startScale.x, startScale.y);

				// 前シーンのカメラの回転状態取得
				auto startRotQt = _preSceneCameraRotationQuat.getInversed();

				// 移動元座標再計算
				auto diff = slideLinkData->getMoveStartPos();
				auto startPos = camAnchorDiff + Vec2(diff.x * startScale.x, diff.y * startScale.y);

				Vec3 tmpPos = startRotQt * Vec3(startPos.x, startPos.y, 0);
				
				startPos.x = tmpPos.x + screenCenter.x;
				startPos.y = tmpPos.y + screenCenter.y;

				// 移動先座標算出
				auto endPos = agtk::Scene::getPositionCocos2dFromScene(slideLinkData->getMoveEndPos(), scene) - camPos + camAnchorDiff + screenCenter;
				auto endScale = startScale;

				// 移動元座標へ移動用オブジェクトを再配置
				moveObjPlayer->setPosition(startPos);

				// 移動用の差分座標と拡縮差分と移動時間を保持
				slideLinkData->setMoveDurationMax(effectDuration);
				slideLinkData->setDeltaPos(endPos - startPos);
				slideLinkData->setDeltaScale(endScale - startScale);

				// 移動開始時のクオータニオンを保持
				slideLinkData->setStartRotQuat(startRotQt);
			}

			// オブジェクト移動時間設定
			this->setMoveObjectDuration(effectDuration);
		}
	}
	AGTK_DEBUG_ACTION_LOG("%d,%s", __LINE__, __FUNCTION__);
}

/**
* フェード用スプライトへシーンをレンダリング(メニュー以外)
* @param	camera				カメラ
* @param	targetFadeSprite	レンダリング先のスプライト
* @param	scene				描画するシーン
* @param	renderer			レンダラー
* @param	effectType			エフェクトタイプ
*/
#ifdef FIX_ACT2_5233
void GameScene::RenderSceneToFadeSprite(cocos2d::Camera *camera, cocos2d::RenderTexture *targetFadeSprite, cocos2d::RenderTexture *targetFadeFrontSprite, int objectLayerId, agtk::Scene *scene, cocos2d::Renderer *renderer, agtk::data::EnumMoveEffect effectType)
#else
void GameScene::RenderSceneToFadeSprite(cocos2d::Camera *camera, cocos2d::RenderTexture *targetFadeSprite, agtk::Scene *scene, cocos2d::Renderer *renderer, agtk::data::EnumMoveEffect effectType)
#endif
{
	// 「なし」「黒フェード」「白フェード」の場合はメニューを含む
	bool isAddMenuScene = (effectType == agtk::data::EnumMoveEffect::kMoveEffectNone ||
							effectType == agtk::data::EnumMoveEffect::kMoveEffectBlack ||
							effectType == agtk::data::EnumMoveEffect::kMoveEffectWhite);

	// スライド系以外の場合
	// ※スライド連結系はカメラの位置をポータルの中心軸で合わすので再補完を行わない
	if (effectType != agtk::data::EnumMoveEffect::kMoveEffectSlideDownLink &&
		effectType != agtk::data::EnumMoveEffect::kMoveEffectSlideUpLink &&
		effectType != agtk::data::EnumMoveEffect::kMoveEffectSlideLeftLink &&
		effectType != agtk::data::EnumMoveEffect::kMoveEffectSlideRightLink) {
		//カメラの移動補完の値を0にして、即指定カメラ位置へ移動。
		auto sceneCamera = scene->getCamera();
		auto duration = sceneCamera->getMoveDuration();
		sceneCamera->setMoveDuration(0.0f);
		sceneCamera->update(Director::getInstance()->getAnimationInterval());
		sceneCamera->setMoveDuration(duration);
	}

	bool isIgnoreMenuScene = (scene->getSceneTopMost()->getWithMenuRenderTexture() && !isAddMenuScene);
	if (isIgnoreMenuScene) {
		// SceneTopMost(メニュー含む)をメニュー除いた状態にする
		scene->updateSceneRenderTextureListIgnoreMenu(0);
	}

	cocos2d::Mat4 vm = camera->getViewMatrix();
	auto menuCamera = scene->getCamera()->getMenuCamera();
	auto vm_menu = menuCamera->getViewMatrix();

	targetFadeSprite->setKeepMatrix(true);
	targetFadeSprite->beginWithClear(0, 0, 0, 1);
#ifdef FIX_ACT2_5233
	bool targetChanged = false;
	bool bVisitedSceneTopMost = false;
#endif
	{
		auto withMenuRenderTextureCtrl = scene->getSceneTopMost()->getWithMenuRenderTexture();
#ifdef FIX_ACT2_5233
		if (objectLayerId < 0 && withMenuRenderTextureCtrl) {
#else
		if (withMenuRenderTextureCtrl) {
#endif
			auto topMostSprite = withMenuRenderTextureCtrl->getLastRenderTexture()->getSprite();
			topMostSprite->setVisible(true);
			topMostSprite->visit(renderer, vm, false);
			topMostSprite->setVisible(false);
			bVisitedSceneTopMost = true;//ON
		}
		else {
			auto renderTextureCtrl = scene->getSceneTopMost()->getRenderTexture();
#ifdef FIX_ACT2_5233
			if (objectLayerId < 0 && renderTextureCtrl) {
#else
			if (renderTextureCtrl) {
#endif
				auto topMostSprite = renderTextureCtrl->getLastRenderTexture()->getSprite();
				topMostSprite->setVisible(true);
				topMostSprite->visit(renderer, vm, false);
				topMostSprite->setVisible(false);
				bVisitedSceneTopMost = true;//ON
			}
			else {
				if (renderTextureCtrl) {

					auto topMostSprite = renderTextureCtrl->getLastRenderTexture()->getSprite();
					topMostSprite->setVisible(true);
					topMostSprite->visit(renderer, vm, false);
					topMostSprite->setVisible(false);

					targetChanged = true;
					targetFadeSprite->end();
					targetFadeSprite->setKeepMatrix(false);
					targetFadeFrontSprite->setKeepMatrix(true);
					targetFadeFrontSprite->beginWithClear(0, 0, 0, 0);

					auto renderTextureFrontCtrl = scene->getSceneTopMost()->getRenderTextureFront();
					if (renderTextureFrontCtrl != nullptr) {
						auto topMostSpriteFront = renderTextureFrontCtrl->getLastRenderTexture()->getSprite();
						topMostSpriteFront->setVisible(true);
						topMostSpriteFront->visit(renderer, vm, false);
						topMostSpriteFront->setVisible(false);
					}

					bVisitedSceneTopMost = true;//ON
				}
				else {
					// 背景
					{
						auto sceneBackground = scene->getSceneBackground();
						auto renderTextureCtrl = sceneBackground->getRenderTexture();
						if (renderTextureCtrl) {
							auto bgSprite = renderTextureCtrl->getLastRenderTexture()->getSprite();
							bgSprite->setVisible(true);
							bgSprite->visit(renderer, vm, false);
							bgSprite->setVisible(false);
						}
						else {
							sceneBackground->setVisible(true);
							sceneBackground->visit(renderer, vm, true);
							sceneBackground->setVisible(false);
						}
					}

					// シーンレイヤー
					{
						auto dic = scene->getSceneLayerList();
						cocos2d::DictElement *el = nullptr;
						CCDICT_FOREACH(dic, el) {
							bool isRender = false;

							auto sceneLayer = dynamic_cast<agtk::SceneLayer *>(el->getObject());
							auto renderTextureCtrl = sceneLayer->getRenderTexture();
							if (renderTextureCtrl) {
								if (renderTextureCtrl->isUseShader()) {
									isRender = true;
								}
							}

							if (isRender) {
								auto sprite = renderTextureCtrl->getLastRenderTexture()->getSprite();
								sprite->setVisible(true);
								sprite->visit(renderer, vm, false);
								sprite->setVisible(false);
							}
							else {
								sceneLayer->setVisible(true);
								sceneLayer->visit(renderer, vm, true);
								sceneLayer->setVisible(false);
							}
#ifdef FIX_ACT2_5233
							if (objectLayerId >= 0 && objectLayerId == sceneLayer->getLayerId()) {
								targetChanged = true;
								targetFadeSprite->end();
								targetFadeSprite->setKeepMatrix(false);
								targetFadeFrontSprite->setKeepMatrix(true);
								targetFadeFrontSprite->beginWithClear(0, 0, 0, 0);
							}
#endif
						}
					}
				}
			}

			// メニューシーンを非表示
			// gameManager->getCurrentLayerをvisitすると濃くなるので非表示にしておく
			std::deque<bool> menuVisibleList;
			{
				auto menuLayerList = scene->getMenuLayerList();
				cocos2d::DictElement *el;
				CCDICT_FOREACH(menuLayerList, el) {
					auto menuLayer = dynamic_cast<agtk::SceneLayer *>(el->getObject());
					menuVisibleList.emplace_back(menuLayer->isVisible());
					menuLayer->setVisible(false);
				}
			}

			// デバッグ表示
			auto gameManager = GameManager::getInstance();
			auto layer = gameManager->getCurrentLayer();
			layer->setVisible(true);
			auto topMost = scene->getSceneTopMost();
			auto lVisible = topMost->isVisible();
			topMost->setVisible(false);
			layer->visit(renderer, vm, true);
			topMost->setVisible(lVisible);
			layer->setVisible(false);

			// メニューシーン表示(非表示からの戻し)
			{
				int menuVisibleIndex = 0;

				auto menuLayerList = scene->getMenuLayerList();
				cocos2d::DictElement *el;
				CCDICT_FOREACH(menuLayerList, el) {
					auto menuLayer = dynamic_cast<agtk::SceneLayer *>(el->getObject());

					menuLayer->setVisible(menuVisibleList[menuVisibleIndex]);

					++menuVisibleIndex;
				}
			}

			// メニューシーン
			{
				if (isAddMenuScene) {
					auto menuLayerList = scene->getMenuLayerList();
					cocos2d::DictElement *el;
					CCDICT_FOREACH(menuLayerList, el) {
						auto menuLayer = dynamic_cast<agtk::SceneLayer *>(el->getObject());

						menuLayer->setVisible(true);
						if (menuLayer->getLayerId() != agtk::data::SceneData::kHudTopMostLayerId) {
							menuLayer->visit(renderer, vm_menu, true);
						}
						else {
							menuLayer->visit(renderer, vm, true);
						}
						menuLayer->setVisible(false);
					}
				}
			}
		}
	}
#ifdef FIX_ACT2_5233
	if (!targetChanged) {
		targetFadeSprite->end();
		targetFadeSprite->setKeepMatrix(false);
		if (targetFadeFrontSprite) {
			targetFadeFrontSprite->setKeepMatrix(true);
			targetFadeFrontSprite->beginWithClear(0, 0, 0, 0);
		}
	}
	if (targetFadeFrontSprite) {
		targetFadeFrontSprite->end();
		targetFadeFrontSprite->setKeepMatrix(false);
	}
#else
	targetFadeSprite->end();
	targetFadeSprite->setKeepMatrix(false);
#endif

#ifdef USE_DESIGNATED_RESOLUTION_RENDER
	auto renderTexture = this->getRenderTexture();
	if (renderTexture && renderTexture->getParent()) {
		renderTexture->setVisible(false);

		if (!isAddMenuScene) {
			// メニューだけをレンダーしたものを作成
			// 通常描画だと画面サイズ次第で拡縮がおこるため
			scene->setVisible(true);
			GameManager::visitSceneOnlyMenu(renderer, scene, renderTexture);
			renderTexture->setVisible(true);
			scene->setVisible(false);

			// メニューシーン
			{
				auto menuLayerList = scene->getMenuLayerList();
				cocos2d::DictElement *el;
				CCDICT_FOREACH(menuLayerList, el) {
					auto menuLayer = dynamic_cast<agtk::SceneLayer *>(el->getObject());

					menuLayer->setVisible(true);
				}
			}
		}
	}
#endif

	auto gameManager = GameManager::getInstance();
	auto layer = gameManager->getCurrentLayer();
	layer->setVisible(true);

	//SceneTopMostがvisitされてない場合、SceneTopMostのレンダーテクスチャを非表示にする。
	if (!bVisitedSceneTopMost) {
		scene->getSceneTopMost()->setVisibleRenderTexture(false);
	}
}

/**
* データロード演出処理
*/
void GameScene::calcLoadDataEffect()
{
	auto renderer = Director::getInstance()->getRenderer();
	auto gm = GameManager::getInstance();
	float effectDuration = gm->getLoadEffectDuration300() / 300.0f / 2;	//前演出と後演出で処理時間を分割。

	auto loadBit = gm->getLoadBit();
	if ((loadBit & GameManager::kLoadBit_Scene) && gm->getLoadEffectType() != agtk::data::ObjectCommandFileLoadData::kEffectTypeNone)
	{
		// 画面演出がある場合。
		// ロード中ステートを設定
		_sceneChangeState = SceneChangeState::LOADING;

		// フェードアウト準備
		auto scene = GameManager::getInstance()->getCurrentScene();
		auto camera = scene->getCamera()->getCamera();
		auto cameraPos = camera->getPosition();
		auto cameraAnchor = camera->getAnchorPoint();
		auto cameraScale = Vec2(camera->getScaleX(), camera->getScaleY());

		// 現在のシーンをレンダリング
#ifdef FIX_ACT2_5233
		RenderSceneToFadeSprite(camera, _fadeSpriteBefore, nullptr, -1, scene, renderer);
#else
		RenderSceneToFadeSprite(camera, _fadeSpriteBefore, scene, renderer);
#endif
		renderer->render();

		// フェード用の設定
		auto layerColor = this->getLayerColor();
		layerColor->setAnchorPoint(cameraAnchor);
		layerColor->setPosition(cameraPos);
		layerColor->setScale(cameraScale.x, cameraScale.y);
		//layerColor->setVisible(true);
		layerColor->setVisible(false);

		// レンダリングしたシーンを配置
		_fadeSpriteBefore->setVisible(true);
		_fadeSpriteBefore->setPositionY(0);
		_fadeSpriteBefore->getSprite()->setAnchorPoint(cameraAnchor);
		_fadeSpriteBefore->setScale(cameraScale.x, cameraScale.y);
	}
	// 退出エフェクト準備
	auto create_setup_for_out = [this]() {
		return CCCallFunc::create([this]() {
			auto gm = GameManager::getInstance();
			int effectType = gm->getLoadEffectType();
			// フェード用の設定
			auto layerColor = this->getLayerColor();
			if (effectType == agtk::data::ObjectCommandFileLoadData::kEffectTypeBlack) {
				layerColor->setColor(Color3B::BLACK);
				layerColor->setOpacity(0);
			}
			else if (effectType == agtk::data::ObjectCommandFileLoadData::kEffectTypeWhite) {
				layerColor->setColor(Color3B::WHITE);
				layerColor->setOpacity(0);
			}
			else {
				layerColor->setOpacity(0);
			}
		});
	};
	// 退出エフェクト
	auto create_effectout = [this, effectDuration]() {
		auto gm = GameManager::getInstance();
		int effectType = gm->getLoadEffectType();
		if (effectType == agtk::data::ObjectCommandFileLoadData::kEffectTypeBlack || effectType == agtk::data::ObjectCommandFileLoadData::kEffectTypeWhite) {
			auto layerColor = this->getLayerColor();
			return TargetedAction::create(layerColor, FadeIn::create(effectDuration));
		}
		else {
			auto scene = GameManager::getInstance()->getCurrentScene();
			auto camera = scene->getCamera()->getCamera();
			auto cameraScale = Vec2(camera->getScaleX(), camera->getScaleY());
			auto width = _fadeSpriteBefore->getSprite()->getContentSize().width * cameraScale.x;
			auto height = _fadeSpriteBefore->getSprite()->getContentSize().height * cameraScale.y;
			switch (effectType) {
			case agtk::data::ObjectCommandFileLoadData::kEffectTypeSlideUp:
				return TargetedAction::create(_fadeSpriteBefore, MoveBy::create(effectDuration, cocos2d::Vec2(0, height)));
			case agtk::data::ObjectCommandFileLoadData::kEffectTypeSlideDown:
				return TargetedAction::create(_fadeSpriteBefore, MoveBy::create(effectDuration, cocos2d::Vec2(0, -height)));
			case agtk::data::ObjectCommandFileLoadData::kEffectTypeSlideLeft:
				return TargetedAction::create(_fadeSpriteBefore, MoveBy::create(effectDuration, cocos2d::Vec2(-width, 0)));
			case agtk::data::ObjectCommandFileLoadData::kEffectTypeSlideRight:
				return TargetedAction::create(_fadeSpriteBefore, MoveBy::create(effectDuration, cocos2d::Vec2(width, 0)));
			}
		}
	};

	// 操作をキャンセルして現在のシーンを破棄
	auto create_delete_scene = [this]() {
		return CCCallFunc::create([this]() {
			InputManager::getInstance()->reset(true); // ロード時は全ての操作をキャンセルさせる
			GameManager::getInstance()->getCurrentScene()->end();
			auto layerColor = this->getLayerColor();
			layerColor->setVisible(false);
		});
	};
	// シーン遷移する場合のデータをロード -> ロードするシーンを生成 -> オブジェクトの各種パラメータを復帰
	auto create_data_load_scene_change = [this, renderer]() {
		return CCCallFunc::create([this, renderer]() {
			auto gm = GameManager::getInstance();

			// ロード後は新規シーン扱いなのでシーン変更による復活待ちオブジェクトリストを初期化
			gm->getSceneChangeReappearObjectList()->removeAllObjects();

			// データロード
			gm->loadData();

			// ロード対象のシーンを生成
			
			auto loadSceneData = gm->getProjectData()->getSceneData(gm->getLoadSceneId());
			auto nextScene = agtk::Scene::create(loadSceneData, this->getStartPointGroupIdx(), agtk::Scene::kCreateTypeLoad);
			this->addChild(nextScene, ZOrder::Scene, Tag::Scene);
			nextScene->start(this, false);
			nextScene->onEnter();

			//次シーンのカメラ情報から遷移用レイヤーへ情報を設定。
			{
				auto camera = nextScene->getCamera()->getCamera();
				auto cameraPos = camera->getPosition();
				auto cameraAnchor = camera->getAnchorPoint();
				auto cameraScale = Vec2(camera->getScaleX(), camera->getScaleY());

				// フェード用の設定
				auto layerColor = this->getLayerColor();
				layerColor->setPosition(cameraPos);
				layerColor->setAnchorPoint(cameraAnchor);
				layerColor->setScale(cameraScale.x, cameraScale.y);

				// レンダリングしたシーンを配置
				_fadeSpriteBefore->getSprite()->setAnchorPoint(cameraAnchor);
				_fadeSpriteBefore->setScale(cameraScale.x, cameraScale.y);
			}

			// ロードデータをシーンに反映
			gm->attachLoadData();

			// ロード先のシーンIDを設定
			if (nextScene) {
				auto nextSceneId = nextScene->getSceneData()->getId();
				gm->setNextSceneId(nextSceneId);

				auto sceneCamera = nextScene->getCamera();
				auto camera = sceneCamera->getCamera();
				auto duration = sceneCamera->getMoveDuration();
				sceneCamera->setMoveDuration(0.0f);

				if (agtk::DebugManager::getInstance()->getSkipOneFrameWhenSceneCreatedIgnored() == false) {
					//生成時に１フレームスキップする。
					auto framePerSeconds = 0.0f;// Director::getInstance()->getAnimationInterval();
					//※１フレームスキップするために、ここで２回更新する。
					// ※スキップ中にオブジェクトが死亡するとレンダラーも死ぬので更新とともにレンダリングを実行する
					nextScene->setSceneCreateSkipFrameFlag(true);
					{
						nextScene->update(framePerSeconds);
						renderer->render();
						nextScene->update(framePerSeconds);
						renderer->render();
					}
					nextScene->setSceneCreateSkipFrameFlag(false);
				}
				sceneCamera->update(0.1f);
				sceneCamera->setMoveDuration(duration);

				// 新しいシーンをレンダリング
#ifdef FIX_ACT2_5233
				RenderSceneToFadeSprite(camera, _fadeSpriteBefore, nullptr, -1, nextScene, renderer);
#else
				RenderSceneToFadeSprite(camera, _fadeSpriteBefore, nextScene, renderer);
#endif
				renderer->render();

				this->getLayerColor()->setPosition(camera->getPosition());
				_fadeSpriteBefore->getSprite()->setAnchorPoint(camera->getAnchorPoint());
				_fadeSpriteBefore->setScale(camera->getScaleX(), camera->getScaleY());
				_fadeSpriteBefore->setPosition(camera->getPosition());

				// カレントシーンリンクデータリストを次のシーンIDで初期化
				this->initCurSceneLinkDataList(nextSceneId);

				//_sceneChangeStateが書き換わってしまうので戻す。
				_sceneChangeState = SceneChangeState::LOADING;
			}
		});
	};
	// データをロード->ロードするシーンを生成->オブジェクトの各種パラメータを復帰
	auto create_data_load = []() {
		return CCCallFunc::create([]() {
			auto gm = GameManager::getInstance();
			gm->loadData();
			gm->attachLoadData();
		});
	};
	// 進入エフェクト準備
	auto create_setup_for_in = [this]() {
		return CCCallFunc::create([this]() {
			auto gm = GameManager::getInstance();
			int effectType = gm->getLoadEffectType();
			auto layerColor = this->getLayerColor();
			layerColor->setVisible(true);
			if (effectType == agtk::data::ObjectCommandFileLoadData::kEffectTypeBlack || effectType == agtk::data::ObjectCommandFileLoadData::kEffectTypeWhite) {
			}
			else {
				auto scene = GameManager::getInstance()->getCurrentScene();
				auto camera = scene->getCamera()->getCamera();
				auto cameraScale = Vec2(camera->getScaleX(), camera->getScaleY());
				auto width = _fadeSpriteBefore->getSprite()->getContentSize().width * cameraScale.x;
				auto height = _fadeSpriteBefore->getSprite()->getContentSize().height * cameraScale.y;
				_fadeSpriteBefore->setVisible(true);
				_fadeSpriteBefore->getSprite()->setVisible(true);
				switch (effectType) {
				case agtk::data::ObjectCommandFileLoadData::kEffectTypeSlideUp:
					_fadeSpriteBefore->setPositionY(-height);
					break;
				case agtk::data::ObjectCommandFileLoadData::kEffectTypeSlideDown:
					_fadeSpriteBefore->setPositionY(height);
					break;
				case agtk::data::ObjectCommandFileLoadData::kEffectTypeSlideLeft:
					_fadeSpriteBefore->setPositionX(width);
					break;
				case agtk::data::ObjectCommandFileLoadData::kEffectTypeSlideRight:
					_fadeSpriteBefore->setPositionX(-width);
					break;
				}
			}
		});
	};
	// 進入エフェクト
	auto create_effectin = [this, effectDuration]() {
		auto gm = GameManager::getInstance();
		int effectType = gm->getLoadEffectType();
		if (effectType == agtk::data::ObjectCommandFileLoadData::kEffectTypeBlack || effectType == agtk::data::ObjectCommandFileLoadData::kEffectTypeWhite) {
			auto layerColor = this->getLayerColor();
			return TargetedAction::create(layerColor, FadeOut::create(effectDuration));
		}
		else {
			auto scene = GameManager::getInstance()->getCurrentScene();
			auto camera = scene->getCamera()->getCamera();
			auto cameraScale = Vec2(camera->getScaleX(), camera->getScaleY());
			auto width = _fadeSpriteBefore->getSprite()->getContentSize().width * cameraScale.x;
			auto height = _fadeSpriteBefore->getSprite()->getContentSize().height * cameraScale.y;
			switch (effectType) {
			case agtk::data::ObjectCommandFileLoadData::kEffectTypeSlideUp:
				return TargetedAction::create(_fadeSpriteBefore, MoveBy::create(effectDuration, cocos2d::Vec2(0, height)));
			case agtk::data::ObjectCommandFileLoadData::kEffectTypeSlideDown:
				return TargetedAction::create(_fadeSpriteBefore, MoveBy::create(effectDuration, cocos2d::Vec2(0, -height)));
			case agtk::data::ObjectCommandFileLoadData::kEffectTypeSlideLeft:
				return TargetedAction::create(_fadeSpriteBefore, MoveBy::create(effectDuration, cocos2d::Vec2(-width, 0)));
			case agtk::data::ObjectCommandFileLoadData::kEffectTypeSlideRight:
				return TargetedAction::create(_fadeSpriteBefore, MoveBy::create(effectDuration, cocos2d::Vec2(width, 0)));
			}
		}
	};
	// 後処理
	auto create_exit = [this, effectDuration]() {
		return CCCallFunc::create([this, effectDuration]() {
			this->getLayerColor()->setVisible(false);
			this->getFadeSpriteBefore()->setVisible(false);
			this->_sceneChangeState = SceneChangeState::NONE;

			// シーンに登録されているBGMを再生
			AudioManager::getInstance()->stopAllBgm();
			auto gm = GameManager::getInstance();
			if (gm->getLoadedBgm().size() > 0 ) {
				gm->calcMoveBgmEffect(effectDuration, agtk::data::kBgmChangeTypeChange, false, gm->getLoadedBgm(), false);
			}
		});
	};

	if ((loadBit & GameManager::kLoadBit_Scene)) 
	{// シーン遷移する場合
		if (gm->getLoadEffectType() != agtk::data::ObjectCommandFileLoadData::kEffectTypeNone) {
			effectDuration = gm->getLoadEffectDuration300() / 300.0f;
			this->runAction(Sequence::create(
				create_setup_for_out(),
				create_effectout(),
				create_delete_scene(),
				create_data_load_scene_change(),
				create_setup_for_in(),
				create_effectin(),
				create_exit(),
				nullptr
			));
		}
		else {
			// 演出なし
			this->runAction(Sequence::create(
				create_delete_scene(),
				create_data_load_scene_change(),
				create_exit(),
				nullptr
				));
		}
	}
	else 
	{// シーン遷移がない場合
		this->runAction(Sequence::create(
			create_data_load(),
			nullptr
			));
	}
}

void GameScene::debugGUI()
{
#if 0
	//movie
	auto videoLayer = CCVideoLayer::create("movies/001.mp4");
	videoLayer->setPosition(400, 100);
	//	videoLayer->setContentSize(cocos2d::Size(100, 100));
	this->addChild(videoLayer, 10, 1001);
	videoLayer->playVideo();
	videoLayer->setScale(0.2f);
	videoLayer->setOpacity(128);
#endif
	agtk::DebugManager::getInstance()->start(this);
#if defined(AGTK_DEBUG)
	agtk::DebugManager::getInstance()->onMovieState = [&](agtk::DebugManager::MovieState state) {
		switch (state) {
		case agtk::DebugManager::MovieState::Play: {
#if 0
			CCVideoLayer *videoLayer = dynamic_cast<CCVideoLayer *>(this->getChildByTag(GameScene::Tag::Movie));
			if (videoLayer == nullptr) {
//				videoLayer = CCVideoLayer::create("movies/001.mp4");
				videoLayer = CCVideoLayer::create("animation/g1.gif");
				if (videoLayer) {
					videoLayer->setPosition(400, 100);
					this->addChild(videoLayer, 10, GameScene::Tag::Movie);
					videoLayer->playVideo();
					videoLayer->setScale(0.2f);
				}
			}
			else {
				videoLayer->playVideo();
			}
#endif
			break; }
		case agtk::DebugManager::MovieState::Stop: {
#if 0
			auto videoLayer = dynamic_cast<CCVideoLayer *>(this->getChildByTag(GameScene::Tag::Movie));
			if (videoLayer) {
				this->removeChild(videoLayer);
			}
#endif
//			CCVideoTextureCache::sharedTextureCache()->clear();
			break; }
		case agtk::DebugManager::MovieState::Pause: {
#if 0
			auto videoLayer = dynamic_cast<CCVideoLayer *>(this->getChildByTag(GameScene::Tag::Movie));
			if (videoLayer) {
				videoLayer->stopVideo();
			}
#endif
			break; }
		}
	};
#endif
}

void GameScene::update(float delta)
{
	if (_isRestartCanvas) {
		//restartCanvas()中はupdateを行わない
		return;
	}
#ifdef USE_COLLISION_MEASURE
	wallCollisionCount = 0;
	hitCollisionCount = 0;
	attackCollisionCount = 0;
	connectionCollisionCount = 0;
	woConnectionCollisionCount = 0;
	noInfoCount = 0;
	callCount = 0;
	cachedCount = 0;
	roughWallCollisionCount = 0;
	roughHitCollisionCount = 0;
#endif
	//ゲーム終了。
	auto gameManager = GameManager::getInstance();
	auto playData = gameManager->getPlayData();
	if (playData->getCommonSwitchData(agtk::data::kProjectSystemSwitchQuitTheGame)->getValue()) {
		close();
		return;
	}

	//一時停止。
	auto debugWindow = agtk::DebugManager::getInstance()->getDebugPerformanceAndSpeedSettingsWindow();
	if (debugWindow->getState() == agtk::DebugPerformanceAndSpeedSettingsWindow::kStateScenePause) {
		auto scene = this->getScene();
		if (scene) {
			scene->updateVisit(delta);
#ifdef USE_DESIGNATED_RESOLUTION_RENDER
			auto renderTexture = this->getRenderTexture();
			if (renderTexture && renderTexture->getParent()) {
				scene->setVisible(true);
				auto renderer = Director::getInstance()->getRenderer();
				GameManager::visitScene(renderer, scene, renderTexture);
				renderTexture->setVisible(true);
				scene->setVisible(false);
			}
#endif
		}
		return;
	}
	//コマ送り。
	if (debugWindow->getState() == agtk::DebugPerformanceAndSpeedSettingsWindow::kStateSceneFrameByFrame) {
		debugWindow->setState(agtk::DebugPerformanceAndSpeedSettingsWindow::kStateScenePause);
	}

//#AGTK-NX
#if defined(USE_AUTO_FRAMERATE) && (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif

// #AGTK-NX #AGTK-WIN
#if 1
	auto realDelta = delta;
#endif
	//※アニメーションのフレームを飛ばさないように秒間を1/60に固定。
#if defined(USE_RUNTIME)
	delta = Director::getInstance()->getAnimationInterval();
#else
	auto debugManager = agtk::DebugManager::getInstance();
	if (debugManager->getFixFramePerSecondFlag()) {
		delta = FRAME_PER_SECONDS;
	}
#endif
// #AGTK-NX #AGTK-WIN
#if 1
	if (MovieManager::getInstance()->getMovieList()->count() > 0) {
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif
		// Movie使用中は実時間でゲームを進行
		delta = realDelta;
	}
#endif
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif

	auto timer = this->getTimer();
	timer->start();
	_counter++;
	_seconds += delta;
#ifdef USE_PREVIEW
	if (getPaused()) {
		return;
	}
#endif
	BaseLayer::update(delta);

	// シーン切り替え中でない場合
	if (_sceneChangeState == GameScene::NONE) {

		auto scene = this->getScene();

		auto im = InputManager::getInstance();
		auto gm = GameManager::getInstance();

		if (scene) {
			// F11 キーが押された場合
#ifdef USE_PREVIEW
			if (im->isTriggeredKeyboard((int)EventKeyboard::KeyCode::KEY_F11)) {
				static bool bDisp = false;
				scene->setVisibleObjectDebugDisplayArea(bDisp);
#ifdef AGTK_DEBUG
				scene->setVisibleObjectDebugDisplayPlayer(bDisp);
#endif
				bDisp = !bDisp;
			}
#endif
			// シーン更新
			scene->update(delta);
#ifdef USE_DESIGNATED_RESOLUTION_RENDER
			auto renderTexture = this->getRenderTexture();
			if (renderTexture && renderTexture->getParent()) {
				scene->setVisible(true);
				auto renderer = Director::getInstance()->getRenderer();
				GameManager::visitScene(renderer, scene, renderTexture);
				renderTexture->setVisible(true);
				scene->setVisible(false);
			}
#endif

			// 変数・スイッチの要求チェック
			gm->checkRequestVariableAndSwitch();

			// セーブ要求チェック
			gm->checkRequestSave();

			// 削除要求チェック
			gm->checkRequestDelete();

			// コピー要求チェック
			gm->checkRequestCopy();

			//共通変数・スイッチの更新
			gm->getPlayData()->update(delta);

			// ロード要求が承認された場合
			if ( gm->checkRequestLoad() ) {
				// ロード演出実行
				calcLoadDataEffect();
				return;
			}
		}

		// シーン遷移チェック
		bool needSceneChange = this->checkSceneLinkCondition();

		// シーン変更要求がある場合
		if (needSceneChange || gm->getNeedTerminateScene()) {
			// シーン破棄要求をOFF
			bool bNeedTerminateScene = gm->getNeedTerminateScene();
			gm->setNeedTerminateScene(false);
			// シーン遷移中フラグON
			if (bNeedTerminateScene == false) {
				gm->setIsSceneMoving(true);
			}
			// シーン遷移での画面演出開始
			calcMoveEffect(true, false, true);
		}
		// ポータル移動要求がある場合
		else if (gm->getNeedPortalMove() && gm->getNextSceneId() >= 0) {
			// ポータル移動要求をOFF
			gm->setNeedPortalMove(false);
			// ポータル遷移中をON
			gm->setIsPortalMoving(true);

			// ポータル移動での画面演出開始
			calcMoveEffect(true, true, gm->getNeedSceneChange());
		}
	}
	// 移動用オブジェクト更新時間がある場合
	else if (this->getMoveObjectDuration() > 0) {

		auto newDuration = this->getMoveObjectDuration() - delta;
		if (newDuration < 0) {
			newDuration = 0;
		}
		this->setMoveObjectDuration(newDuration);

		// 移動用オブジェクトを移動させる
		// ※MoveTo を使用すると何故か最終位置がずれてしまうためこの方法を採る事にした
		cocos2d::Ref * ref = nullptr;
		CCARRAY_FOREACH(this->getMoveObjectList(), ref) {
			auto slideLinkData = dynamic_cast<SlideLinkData *>(ref);
			auto durationMax = slideLinkData->getMoveDurationMax();
			auto player = slideLinkData->getMoveTarget();

			// 現在地取得
			auto nowPos = player->getPosition();
			auto nowScale = Vec2(player->getScaleX(), player->getScaleY());

			// 経過時間分の差分計算
			auto diffPos = delta * slideLinkData->getDeltaPos() / durationMax;
			auto diffScale = delta * slideLinkData->getDeltaScale() / durationMax;

			Quaternion diffQuat = Quaternion::identity();
			Quaternion::lerp(slideLinkData->getStartRotQuat(), Quaternion::identity(), 1.0f - newDuration / durationMax, &diffQuat);

			// 差分を各パラメータに追加
			player->setPosition(nowPos + diffPos);
			player->setScale(nowScale.x + diffScale.x, nowScale.y + diffScale.y);
			player->setRotationQuat(diffQuat);
			player->update(delta);

			if(slideLinkData->getGuiList()->count() > 0) {
				cocos2d::Ref *ref;
				CCARRAY_FOREACH(slideLinkData->getGuiList(), ref) {
					auto gui = dynamic_cast<agtk::DummyParameterGaugeUi *>(ref);
					gui->update(delta);
				}
			}
		}
	}
	{
		auto gameManager = GameManager::getInstance();
		auto loadingScene = gameManager->getLoadingScene();
		if (loadingScene && gameManager->getCurrentScene() == loadingScene) {
			loadingScene->update(delta);
		}
	}

	timer->stop();
}

/**
 * カレントシーンの取得
 * @return	カレントシーン
 */
agtk::Scene *GameScene::getScene()
{
	return GameManager::getInstance()->getCurrentScene();
}

void GameScene::debugInit()
{
	ImFontConfig config;
	config.OversampleH = 1;
	config.OversampleV = 1;
	config.PixelSnapH = 1;
	ImGuiIO& io = ImGui::GetIO();
#ifdef IMGUI_NAV_SUPPORT
	io.Fonts->AddFontFromFileTTF(mFontName.c_str(), 16.0f, &config, io.Fonts->GetGlyphRangesChineseFull());
#else
	io.Fonts->AddFontFromFileTTF(mFontName.c_str(), 16.0f, &config, io.Fonts->GetGlyphRangesChinese());
#endif
	io.Fonts->Build();

	CCIMGUI::getInstance()->addImGUI([&]() {
		this->debugDraw();
	}, "DebugWindowId");

	cocos2d::Director::getInstance()->getScheduler()->schedule([&](float delta)
	{
		if (!Director::getInstance()->getRunningScene()->getChildByName("debugLayer"))
		{
			Director::getInstance()->getRunningScene()->addChild(ImGuiLayer::create(), INT_MAX, "debugLayer");
		}
		//this->debugDraw();
	}, this, 0, false, "checkImGUI");

	//FrameRate
	_bShowFrameRate = true;
	auto director = Director::getInstance();
	director->setDisplayStats(_bShowFrameRate);
}

void GameScene::debugDraw()
{
	if (this->getDebugFlag() == false) {
		return;
	}
	debugDrawMainMenuBar();
}

void GameScene::debugDrawMainMenuBar()
{
	bool bDebugDisplay = _bDebugDisplay;
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("設定"))
		{
			ImGui::MenuItem("判定エリア表示", nullptr, &_bDebugDisplay);
			bool bShowFrameRate = _bShowFrameRate;
			ImGui::MenuItem("フレームレート表示", nullptr, &_bShowFrameRate);
			if (bShowFrameRate != _bShowFrameRate) {
				auto director = Director::getInstance();
				director->setDisplayStats(_bShowFrameRate);
			}
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}
#ifdef USE_PREVIEW
	if (bDebugDisplay != _bDebugDisplay) {
		auto scene = this->getScene();
		scene->setVisibleObjectDebugDisplayArea(_bDebugDisplay);
#ifdef AGTK_DEBUG
		scene->setVisibleObjectDebugDisplayPlayer(_bDebugDisplay);
#endif
	}
#endif
}

void GameScene::close()
{
	Director::getInstance()->end();

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
	exit(0);
#endif
}

void GameScene::setFontName(const std::string &fontName)
{
	mFontName = std::string("fonts/") + fontName + ".ttf";
}
