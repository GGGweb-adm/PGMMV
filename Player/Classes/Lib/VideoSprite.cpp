#include "VideoSprite.h"
#include "renderer/ccGLStateCache.h"
#include "AudioEngine.h"
#include "Manager/AudioManager.h"

USING_NS_CC;
using namespace experimental;

NS_AGTK_BEGIN

/**
 * @class VideoSprite
 */
VideoSprite::VideoSprite() : cocos2d::Sprite()
{
	_vlc = nullptr;
	_vlcPlayer = nullptr;
	_filename = nullptr;
	_buffer = nullptr;
	_texWidth = 0;
	_texHeight = 0;
	_endReached = false;
	_state = kStateIdle;
	_volume = nullptr;
	_pause = false;
}

VideoSprite::~VideoSprite()
{
	CC_SAFE_RELEASE_NULL(_filename);
	if (_vlcPlayer) {
		libvlc_media_player_stop(_vlcPlayer);
		libvlc_media_player_release(_vlcPlayer);
	}
	if (_vlc) {
		libvlc_release(_vlc);
	}
	if (_buffer) {
		free(_buffer);
	}
	CC_SAFE_RELEASE_NULL(_volume);
}

VideoSprite *VideoSprite::createWithFilename(std::string filename, cocos2d::Size size, bool bLoop)
{
	auto p = new (std::nothrow) VideoSprite();
	if (p && p->initWithFilename(filename, size, bLoop)) {
		p->autorelease();
		return p;
	}
	CC_SAFE_DELETE(p);
	return nullptr;
}

bool VideoSprite::initWithFilename(std::string filename, cocos2d::Size size, bool bLoop)
{
	_vlc = libvlc_new(0, NULL);
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
	{
		std::string from = "/";
		std::string to = "\\";
		std::string::size_type pos = filename.find(from);
		while (pos != std::string::npos) {
			filename.replace(pos, from.size(), to);
			pos = filename.find(from, pos + to.size());
		}
	}
#endif
	//
	libvlc_media_t *media = libvlc_media_new_path(_vlc, filename.c_str());
	if (media == nullptr) {
		CC_ASSERT(0);
		return false;
	}
	_vlcPlayer = libvlc_media_player_new_from_media(media);
	if (bLoop) {//ループ有り。
		libvlc_media_add_option(media, "input-repeat=-1");
	}
	libvlc_media_release(media);

	libvlc_video_set_callbacks(_vlcPlayer, VideoSprite::lock, VideoSprite::unlock, VideoSprite::display, this);
	libvlc_event_attach(
		libvlc_media_player_event_manager(_vlcPlayer),
		libvlc_MediaPlayerEndReached,
		endReached,
		(void *)this);

	if (size.width == 0 && size.height == 0) {
		size = cocos2d::Director::getInstance()->getWinSize();
	}
	unsigned int width = size.width;
	unsigned int height = size.height;
	unsigned int length = width * height * 4;
	_buffer = (unsigned char *)malloc(length);
	CC_ASSERT(_buffer);
	memset(_buffer, 0, length);
	cocos2d::Texture2D *texture = new cocos2d::Texture2D();
	texture->initWithData(_buffer, length, cocos2d::Texture2D::PixelFormat::RGBA8888, width, height, size);
	if (!initWithTexture(texture)) {
		return false;
	}
	libvlc_video_set_format(_vlcPlayer, "RGBA", width, height, width << 2);

	this->setTexWidth(width);
	this->setTexHeight(height);
	this->setFilename(cocos2d::__String::create(filename));

	_volume = agtk::ValueTimer<float>::create(0.0f);
	CC_SAFE_RETAIN(_volume);

	//scheduleUpdate();
	return true;
}

void *VideoSprite::lock(void *data, void **p_pixels)
{
	auto p = static_cast<VideoSprite *>(data);
	*p_pixels = p->_buffer;
	return NULL;
}

void VideoSprite::unlock(void *data, void *id, void *const *p_pixels)
{
	CC_ASSERT(id == NULL);
}

void VideoSprite::display(void *data, void *id)
{
}

void VideoSprite::endReached(const struct libvlc_event_t *event, void *data)
{
	if (libvlc_MediaPlayerEndReached == event->type) {
		auto p = static_cast<VideoSprite *>(data);
		p->_endReached = true;
		p->setState(kStateStop);
		CCLOG("END!");
	}
}

unsigned VideoSprite::videoSetup(void **opaque, char *chroma, unsigned *width, unsigned *height, unsigned *pitches, unsigned *lines)
{
	CC_ASSERT(0);
	auto p = static_cast<VideoSprite *>(*opaque);
	return 0;
}

const char *VideoSprite::getFilename()
{
	CC_ASSERT(_filename);
	return _filename->getCString();
}

void VideoSprite::play(float volume)
{
	CC_ASSERT(_vlc && _vlcPlayer);
	libvlc_media_player_play(_vlcPlayer);
	_endReached = false;
	this->setState(kStatePlay);
	this->setVolume(volume);
	_pause = false;
}

void VideoSprite::stop()
{
	CC_ASSERT(_vlcPlayer);
	libvlc_media_player_stop(_vlcPlayer);
}

void VideoSprite::pause()
{
	CC_ASSERT(_vlcPlayer);
	libvlc_media_player_set_pause(_vlcPlayer, true);
	_pause = true;
}

void VideoSprite::resume()
{
	CC_ASSERT(_vlcPlayer);
	libvlc_media_player_set_pause(_vlcPlayer, false);
	_pause = false;
}

libvlc_state_t VideoSprite::getVlcState()
{
	auto state = libvlc_media_player_get_state(_vlcPlayer);
	CCLOG("%d", state);
	return state;
}

void VideoSprite::draw(cocos2d::Renderer *renderer, const cocos2d::Mat4 &transform, uint32_t flags)
{
	_insideBounds = (flags & FLAGS_TRANSFORM_DIRTY) ? renderer->checkVisibility(transform, _contentSize) : _insideBounds;
	if (_insideBounds) {
		if (_buffer) {
			cocos2d::GL::bindTexture2D(_texture->getName());
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _texWidth, _texHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, (uint8_t *)_buffer);
		}
		else {
			cocos2d::GL::bindTexture2D((GLuint)0);
		}
		_trianglesCommand.init(_globalZOrder,
			_texture,
			getGLProgramState(),
			_blendFunc,
			_polyInfo.triangles,
			transform,
			flags);

		renderer->addCommand(&_trianglesCommand);
	}
}

void VideoSprite::update(float dt)
{
	_volume->update(dt);
	auto bgmVolume = AudioManager::getInstance()->getBgmVolume();
	libvlc_audio_set_volume(_vlcPlayer, _volume->getValue() * bgmVolume);
}

void VideoSprite::setVolume(float volume, float seconds)
{
	_volume->start(volume, seconds);
}

float VideoSprite::getVolume()
{
	return _volume->getValue();
}

// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif
NS_AGTK_END
