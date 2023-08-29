#include "WebSocket.h"
#ifdef USE_PREVIEW
#include "Manager/GameManager.h"
#endif

NS_AGTK_BEGIN //---------------------------------------------------------------------------------//

WebSocket::WebSocket()
{
	_websocket = nullptr;
	onConnectionOpened = nullptr;
	onMessageReceived = nullptr;
	onBinaryMessageReceived = nullptr;
	onConnectionClosed = nullptr;
	onErrorOccurred = nullptr;
}

WebSocket::~WebSocket()
{
#ifdef USE_PREVIEW
	AGTK_ACTION_LOG(1, "normal termination.");
#endif
	CC_SAFE_DELETE(_websocket);
}

bool WebSocket::init()
{
	//TODO: 
	//auto data = new cocos2d::Data();
	if (_websocket) {
		return true;
	}
	_websocket = new (std::nothrow) cocos2d::network::WebSocket();
	return true;
}


void WebSocket::onOpen(cocos2d::network::WebSocket *ws)
{
	if (onConnectionOpened) {
		onConnectionOpened();
	}
}

void WebSocket::onMessage(cocos2d::network::WebSocket *ws, const cocos2d::network::WebSocket::Data& data)
{
	bool bBinary = (strlen(data.bytes) != data.len);
	if (onMessageReceived && (!data.isBinary && !bBinary) && data.len > 0) {
		onMessageReceived(data.bytes);
	}
	else if(onBinaryMessageReceived && (data.isBinary || bBinary) && data.len > 0) {
		onBinaryMessageReceived(data.bytes, data.len);
	}
	else {
//		CC_ASSERT(0);
	}
}

void WebSocket::onClose(cocos2d::network::WebSocket *ws)
{
	if (onConnectionClosed) {
		onConnectionClosed();
	}
}

void WebSocket::onError(cocos2d::network::WebSocket *ws, const cocos2d::network::WebSocket::ErrorCode& error)
{
	if (onErrorOccurred) {
		onErrorOccurred(error);
	}
}

void WebSocket::connect(const std::string hostname, int port)
{
	CC_ASSERT(_websocket);
	bool ret = _websocket->init(*this, "ws://" + hostname + ":" + std::to_string(port));
	CC_ASSERT(ret);
}

void WebSocket::disconnect()
{
	CC_ASSERT(_websocket);
	//CC_ASSERT(_websocket->getReadyState() == cocos2d::network::WebSocket::State::OPEN);
	_websocket->close();
}

bool WebSocket::send(const std::string msg)
{
	if (!_websocket) {
		return false;
	}
	if (_websocket->getReadyState() == cocos2d::network::WebSocket::State::OPEN) {
		_websocket->send(msg);
		return true;
	}
	return false;
}

bool WebSocket::send(const unsigned char *binaryMsg, unsigned int len)
{
	if (!_websocket) {
		return false;
	}
	if (_websocket->getReadyState() == cocos2d::network::WebSocket::State::OPEN) {
		_websocket->send(binaryMsg, len);
		return true;
	}
	return false;
}

int WebSocket::getState()
{
	if (!_websocket) {
		return (int)cocos2d::network::WebSocket::State::CLOSED;
	}
	return (int)_websocket->getReadyState();
}

NS_AGTK_END //-----------------------------------------------------------------------------------//
