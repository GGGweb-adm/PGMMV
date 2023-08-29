#include "ViewerScene.h"
#include "PrimitiveManager.h"
#include "FontManager.h"
#include "GameManager.h"
#include "AudioManager.h"
#include "JavascriptManager.h"
#include "DebugManager.h"
#include "Lib/Common.h"
#include "collision/CollisionComponent.hpp"
#include "collision/CollisionUtils.hpp"
#include "Lib/Animation.h"
#include "Lib/Object.h"
#include "Lib/Player/ImagePlayer.h"
#include "Lib/Player/GifPlayer.h"
#include "Lib/Player/SSPlayer.h"
#include "Lib/Player/SpinePlayer.h"
#include "Lib/VideoSprite.h"

USING_NS_CC;

NS_AGTK_BEGIN
NS_AGTK_END

//-------------------------------------------------------------------------------------------------------------------
ViewerScene::ViewerScene()
{
	_sceneId = 0;
	_counter = 0.0f;
}

ViewerScene::~ViewerScene()
{
}

void ViewerScene::onEnter()
{
	Layer::onEnter();
}

void ViewerScene::onEnterTranslationDidFinish()
{
	Layer::onEnterTransitionDidFinish();
}

void ViewerScene::onExit()
{
	Layer::onExit();
	PrimitiveManager::getInstance()->removeAll();
}

bool ViewerScene::init(int id)
{
	//////////////////////////////
    // 1. super init first
    if ( !Layer::init() )
    {
        return false;
    }
	this->setSceneId(id);

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();
	auto pm = PrimitiveManager::getInstance();

#if 1
	//Av 
	{
		std::string path = "movies/001.mp4";
#if 1
		auto videoSprite = agtk::VideoSprite::createWithFilename(path, cocos2d::Size(240, 401));
		videoSprite->setPosition(
			videoSprite->getContentSize().width * 0.5f, 
			videoSprite->getContentSize().height *0.5f
		);
		videoSprite->play();
		this->addChild(videoSprite, 0, "videoPlayer");
#else
		//init
		libvlc_instance_t *vlc;
		libvlc_media_player_t *vlc_player;

		vlc = libvlc_new(0, NULL);
		vlc_player = libvlc_media_player_new(vlc);

		CCSize size = CCDirector::sharedDirector()->getWinSize();
		unsigned int width = size.width;
		unsigned int height = size.height;
		char *videobuf = (char *)malloc((width * height) << 2);
		memset(videobuf, 0, (width * height) << 2);
//		libvlc_video_set_callbacks(vlc_player, lock, unlock, display, NULL);
		libvlc_video_set_callbacks(vlc_player, NULL, NULL, NULL, NULL);
		libvlc_video_set_format(vlc_player, "RGBA", width, height, width << 2);
		
		//set texture
//		CCTexture2D *texture = new CCTexture2D();
//		texture->initWithData(videobuf, kCCTexture2DPixelFormat_RGBA8888, width, height, size);
//		return initWithTexture(texture);

		//play
		libvlc_media_t *media = libvlc_media_new_path(vlc, path.c_str());
		libvlc_media_player_set_media(vlc_player, media);
		libvlc_media_release(media);
		libvlc_media_player_play(vlc_player);

//		libvlc_media_track_t **tracks;
//		libvlc_media_tracks_get(media, &tracks);

		//stop
		libvlc_media_player_stop(vlc_player);

		//pause
		libvlc_media_player_pause(vlc_player);

		//update

		//final
		free(videobuf);
		libvlc_media_player_stop(vlc_player);
		libvlc_media_player_release(vlc_player);
		libvlc_release(vlc);
#endif
	}

#endif
#if 1
#if 0
	{
		auto ss5man = ss::SS5Manager::getInstance();
		ss5man->createEffectBuffer(1024);			//エフェクト用バッファの作成
													//リソースマネージャの作成
		auto resman = ss::ResourceManager::getInstance();

		//プレイヤーの作成
		auto ssplayer = ss::Player::create();

		std::string fname = "animations/002/0831_mouse.ssae";
//		if (!resman->isDataKeyExists(fname)) {
//			resman->addData()
//			resman->addData(fname);
//		}
		ssplayer->play(fname, "move_1");

		//アニメデータをリソースに追加
		//それぞれのプラットフォームに合わせたパスへ変更してください。
		//resman->addData("character_template_comipo\\character_template1.ssbp");
		//プレイヤーにリソースを割り当て
		//ssplayer->setData("character_template1");        // ssbpファイル名（拡張子不要）
		//再生するモーションを設定
		//ssplayer->play("character_template_3head/stance");				 // アニメーション名を指定(ssae名/アニメーション名)

		//・当たり判定
		//・ユーザーデータ
		//	std::string fname = "ss/mouse/0831_mouse.ssbp";
		//ssplayer->setData("0831_mouse");        // ssbpファイル名（拡張子不要）
		ssplayer->play("0831_mouse/move_1");//Bouldering_loop");				 // アニメーション名を指定(ssae名/アニメーション名)
		this->addChild(ssplayer);
	}
#endif
#if 0
	//Sprite Studio
	{
		//プレイヤーの作成
		auto animationData = GameManager::getInstance()->getProjectData()->getAnimationData(3);
		auto ssplayer = agtk::SSPlayer::createWithAnimationData(animationData, animationData->getResourceInfoData(0));
		ssplayer->play(0, 0);
		this->addChild(ssplayer);
		ssplayer->setPosition(visibleSize.width * 0.5f, visibleSize.height * 0.5f);
	}
#endif
	//spine
#if 0
	{
		auto animationData = GameManager::getInstance()->getProject()->getAnimationData(4);//agtk::data::AnimationData *animationData
//		auto spineplayer = agtk::SpinePlayer::createWithFilename("spine/spineboy.json", "spine/spineboy.atlas");
		auto spineplayer = agtk::SpinePlayer::createWithAnimationData(animationData);
		spineplayer->play(0, 0);
		spineplayer->setPosition(visibleSize.width * 0.5f, visibleSize.height * 0.5f);
		this->addChild(spineplayer);
	}
#else
	{
//		spine::SkeletonAnimation *skeletonNode;
//		skeletonNode = spine::SkeletonAnimation::createWithFile("spine/spineboy.json", "spine/spineboy.atlas", 0.6f);
//		skeletonNode->setScale();
//		skeletonNode->setStartListener([this](int trackIndex) {
//			spTrackEntry *entry = spAnimationState_getCurrent(skeletonNode->getState(), trackIndex);
//			const char *animationName = (entry && entry->animation) ? entry->animation->name : 0;
//			CCLOG("%d start: %s", trackIndex, animationName);
//		});
//		skeletonNode->setEndListener([](int trackIndex) {
//			CCLOG("%d end", trackIndex);
//		});
//		skeletonNode->setCompleteListener([](int trackIndex, int loopCount) {
//			CCLOG("%d complete:%d", trackIndex, loopCount);
//		});
//		skeletonNode->setEventListener([](int trackIndex, spEvent *event) {
//			CCLOG("%d event: %s, %d, %f, %s", trackIndex, event->data->name, event->intValue, event->floatValue, event->stringValue);
//		});
//		skeletonNode->setMix("walk", "jump", 0.2f);
//		skeletonNode->setMix("jump", "run", 0.2f);
//		skeletonNode->setAnimation(0, "walk", true);
//		skeletonNode->setAnimation(0, "jump", true);
//		spTrackEntry *jumpEntry = skeletonNode->addAnimation(0, "jump", false, 3);
//		skeletonNode->addAnimation(0, "run", true);
//		skeletonNode->setTrackStartListener(jumpEntry, [](int trackIndex) {
//			CCLOG("jumped!");
//		});
//		skeletonNode->setPosition(cocos2d::Vec2(visibleSize.width * 0.5f, visibleSize.height * 0.5f));
//		this->addChild(skeletonNode);
//		CCLOG("%f,%f", skeletonNode->getContentSize().width, skeletonNode->getContentSize().height);
//		CCLOG("%f,%f", skeletonNode->getPositionX(), skeletonNode->getPositionY());
//
//		auto pm = PrimitiveManager::getInstance();
//		auto tile = pm->createRectangle(0, 0,
//			skeletonNode->getState()->data->skeletonData->width,
//			skeletonNode->getState()->data->skeletonData->height,
//			cocos2d::Color4F::RED);
//		skeletonNode->addChild(tile);
	}
#endif
	//GIF
#if 0
	{
		auto animationData = GameManager::getInstance()->getProjectData()->getAnimationData(4);//agtk::data::AnimationData *animationData
		auto gifplayer = agtk::GifPlayer::createWithAnimationData(animationData, animationData->getResourceInfoData());
		gifplayer->setPosition(visibleSize.width * 0.5f, visibleSize.height * 0.5f);
		gifplayer->play(0, 0);
		this->addChild(gifplayer);
	}
#endif
#if 0
	{
		auto animationData = GameManager::getInstance()->getProject()->getAnimationData(2);//agtk::data::AnimationData *animationData
		auto imgplayer = agtk::ImagePlayer::createWithAnimationData(animationData);
		imgplayer->setPosition(cocos2d::Vec2(visibleSize.width * 0.5f, visibleSize.height * 0.5f));
		imgplayer->play(0, 0);
		this->addChild(imgplayer, 0, "imgPlayer");
	}
#endif
#else
	auto ss5man = ss::SS5Manager::getInstance();
	ss5man->createEffectBuffer(1024);			//エフェクト用バッファの作成
	//リソースマネージャの作成
	auto resman = ss::ResourceManager::getInstance();

	//プレイヤーの作成
	auto ssplayer = ss::Player::create();

	std::string fname = "ss/mouse/0831_mouse.ssbp";
	if (!resman->isDataKeyExists(fname)) {
		resman->addData(fname);
	}

	//アニメデータをリソースに追加
	//それぞれのプラットフォームに合わせたパスへ変更してください。
	//resman->addData("character_template_comipo\\character_template1.ssbp");
	//プレイヤーにリソースを割り当て
	//ssplayer->setData("character_template1");        // ssbpファイル名（拡張子不要）
	//再生するモーションを設定
	//ssplayer->play("character_template_3head/stance");				 // アニメーション名を指定(ssae名/アニメーション名)

	//・当たり判定
	//・ユーザーデータ
//	std::string fname = "ss/mouse/0831_mouse.ssbp";
	ssplayer->setData("0831_mouse");        // ssbpファイル名（拡張子不要）
	ssplayer->play("0831_mouse/move_1");//Bouldering_loop");				 // アニメーション名を指定(ssae名/アニメーション名)
	ssplayer->setPosition(visibleSize.width, 150);//visibleSize.width / 2, visibleSize.height / 2);
	ssplayer->setScale(0.50f, 0.50f);
	CCLOG("%d,%d", ssplayer->getFrameNo(), ssplayer->getMaxFrame());

//	Scheduler *s = Director::getInstance()->getScheduler();
//	s->scheduleUpdateForTarget(ssplayer, 0, false);

#if 0
	//ユーザーデータコールバックを設定
	ssplayer->setUserDataCallback([&](ss::Player *player, const ss::UserData *userData) {
		CCLOG("setUserDataCallback");

	});

	//アニメーション終了コールバックを設定
	ssplayer->setPlayEndCallback([&](ss::Player *player) {
		CCLOG("PlayerEnd");
	});
#endif
//	this->addChild(ssplayer, 0, "ss");
#endif

	//	gadgetObject->runAnimation(0, 0);

    /////////////////////////////
    // 3. add your codes below...

    // add a label shows "Hello World"
    // create and initialize a label
    
    //auto label = Label::createWithTTF("Hello World", "fonts/Marker Felt.ttf", 24);
    //
    //// position the label on the center of the screen
    //label->setPosition(Vec2(origin.x + visibleSize.width/2,
    //                        origin.y + visibleSize.height - label->getContentSize().height));

    //// add the label as a child to this layer
    //this->addChild(label, 1);

#if 0
	// add "HelloWorld" splash screen"
	auto sprite = Sprite::create("HelloWorld.png");

	//// position the sprite on the center of the screen
	sprite->setPosition(Vec2(visibleSize.width/2 + origin.x, visibleSize.height/2 + origin.y));
	this->addChild(sprite, 0);

	auto shader = agtk::Shader::getInstance();
//	auto program = shader->getShaderProgram(agtk::Shader::kShaderGray);
	//program->initWithFilenames("shaders/glayscale.vsh", "shaders/glayscale.fsh");
	//program->bindAttribLocation(cocos2d::GLProgram::ATTRIBUTE_NAME_POSITION,cocos2d::GLProgram::VERTEX_ATTRIB_POSITION);  
	//program->bindAttribLocation(cocos2d::GLProgram::ATTRIBUTE_NAME_COLOR,cocos2d::GLProgram::VERTEX_ATTRIB_COLOR);  
	//program->bindAttribLocation(cocos2d::GLProgram::ATTRIBUTE_NAME_TEX_COORD,cocos2d::GLProgram::VERTEX_ATTRIB_TEX_COORD);  

//	auto programState = GLProgramState::getOrCreateWithGLProgram(program);

//	program->link();
//	program->updateUniforms();  
//	program->setUniformLocationWith1f(program->getUniformLocationForName("u_Rate"), 0.0f);
//auto programState = shader->getProgramState(agtk::Shader::kShaderCRTMonitor);
//sprite->setGLProgramState(programState);
	shader->setProgramState(sprite, agtk::Shader::kShaderCRTMonitor);

	auto sprite2 = Sprite::create("HelloWorld.png");

	//// position the sprite on the center of the screen
	sprite2->setPosition(Vec2(0, 0));
	sprite2->setScale(0.5);

//	sprite2->setGLProgramState(programState);
	shader->setProgramState(sprite2, agtk::Shader::kShaderMosaic);

	this->addChild(sprite2, 0);
#endif
	// add the sprite as a child to this layer
	// ss end
	//-----------------------------------------------------------------------------------------------------------------

#if 0
	agtk::Player *player = agtk::Player::createWithFilename("animation/nayntech/gadget02.json");
	player->runAnimation("wait", "right");
	player->setPosition(visibleSize.width * 0.5f, visibleSize.height * 0.5f);
	player->addComponent(CollisionComponent::create(2));
	this->addChild(player);
	#endif
#if 1
	cocos2d::Color4F white = cocos2d::Color4F::WHITE;
	white.a = 0.5f;
	auto tile = pm->createRectangle(
		visibleSize.width * 0.5f, visibleSize.height * 0.5f, visibleSize.width - 2.0f, visibleSize.height - 2.0f,
		white
	);
	this->addChild(tile, 10);
	auto line1 = pm->createLine(0, visibleSize.height * 0.5f, visibleSize.width, visibleSize.height * 0.5f, white);
	auto line2 = pm->createLine(visibleSize.width * 0.5f, 0, visibleSize.width * 0.5f, visibleSize.height, white);
	this->addChild(line1, 10);
	this->addChild(line2, 10);
#endif
	//JavascriptManager::getInstance()->init();
//	JavascriptManager::getInstance()->run("playBgm(1);playSe(1);");
	//JavascriptManager::getInstance()->term();

	std::string path = FileUtils::getInstance()->getWritablePath();

	registerInputListener(_eventDispatcher, this);

	//get game pad status in polling mode
    scheduleUpdate();

	//	cocos2d::Director::getInstance()->getScheduler()->setTimeScale(2.0);
	return true;
}

void ViewerScene::update(float delta)
{
	_counter += 1.0f;// delta;

//	for (int i = 0; i < 2; i++) {
//		auto p = dynamic_cast<agtk::RenderTexture *>(this->getRenderList()->getObjectAtIndex(i));
//		p->update(delta);
//	}
//	auto render = dynamic_cast<RenderTexture *>(this->getChildByName("render"));
//	if (render) {
//		render->update(delta);
//	}
#if 1
#else
	auto renderTexture = dynamic_cast<agtk::RenderTexture *>(this->getChildByTag(111));
	if (renderTexture) {
		renderTexture->update(delta);
	}
#endif
	InputManager *im = InputManager::getInstance();
#if 0
	InputDataRaw::CGamepad *gp = im->getCGamepad(0);
	if(gp->trigger[InputDataRaw::BUTTON_CANCEL]){
		auto p = dynamic_cast<agtk::ImagePlayer *>(this->getChildByName("imgPlayer"));
		if (p) {
			p->play(0, 0);
		}
	}
	if(gp->trigger[InputDataRaw::BUTTON_PAGE_DOWN]){
		static bool bPause = false;
		if (!bPause) {
			this->pause();
		}
		else {
			this->resume();
		}
		bPause = !bPause;
	}
	if(gp->trigger[InputDataRaw::BUTTON_PAGE_UP]){
		GameManager *gm = GameManager::getInstance();
//		auto director = Director::getInstance();
//		cocos2d::Size designResolutionSize = cocos2d::Size(480, 320);
//		cocos2d::Size smallResolutionSize = cocos2d::Size(480, 320);
//		director->setContentScaleFactor(MIN(smallResolutionSize.height/designResolutionSize.height, smallResolutionSize.width/designResolutionSize.width));
//		auto frameSize = director->getOpenGLView()->getFrameSize();
//		director->getOpenGLView()->setFrameSize(frameSize.width, frameSize.height);
	}
#endif
	auto videoPlayer = dynamic_cast<agtk::VideoSprite *>(this->getChildByName("videoPlayer"));
	if (videoPlayer) {
#if 0
		if (gp->trigger[InputDataRaw::BUTTON_OK]) {
			videoPlayer->pause();
		}
		else if (gp->trigger[InputDataRaw::BUTTON_MENU]) {
			videoPlayer->resume();
		}
#endif
	}

}
