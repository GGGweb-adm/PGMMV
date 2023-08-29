/****************************************************************************
 Copyright (c) 2010-2012 cocos2d-x.org
 Copyright (c) 2013-2016 Chukong Technologies Inc.
 Copyright (c) 2017-2018 Xiamen Yaji Software Co., Ltd.

 http://www.cocos2d-x.org

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.

"[WebSocket module] is based in part on the work of the libwebsockets  project
(http://libwebsockets.org)"

 ****************************************************************************/

#include "network/WebSocket.h"
#include "network/Uri.h"
#include "base/CCDirector.h"
#include "base/CCScheduler.h"
#include "base/CCEventDispatcher.h"
#include "base/CCEventListenerCustom.h"
#include "platform/CCFileUtils.h"

// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
#include <thread>
#include <mutex>
#endif
#include <queue>
#include <list>
#include <signal.h>
#include <errno.h>

// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
#include "libwebsockets.h"
#endif

#define NS_NETWORK_BEGIN namespace cocos2d { namespace network {
#define NS_NETWORK_END }}

#define WS_RX_BUFFER_SIZE (65536)
#define WS_RESERVE_RECEIVE_BUFFER_SIZE (4096)

#define  LOG_TAG    "WebSocket.cpp"

#ifdef USE_AGTK
#define USE_AGTK_CLOSE //明示的にcloseを外部から呼び出す想定の処理
#endif
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
// log, CCLOG aren't threadsafe, since we uses sub threads for parsing pcm data, threadsafe log output
// is needed. Define the following macros (ALOGV, ALOGD, ALOGI, ALOGW, ALOGE) for threadsafe log output.

//FIXME: Move _winLog, winLog to a separated file
static void _winLog(const char *format, va_list args)
{
    static const int MAX_LOG_LENGTH = 16 * 1024;
    int bufferSize = MAX_LOG_LENGTH;
    char* buf = nullptr;

    do
    {
        buf = new (std::nothrow) char[bufferSize];
        if (buf == nullptr)
            return; // not enough memory

        int ret = vsnprintf(buf, bufferSize - 3, format, args);
        if (ret < 0)
        {
            bufferSize *= 2;

            delete[] buf;
        }
        else
            break;

    } while (true);

    strcat(buf, "\n");

    int pos = 0;
    int len = strlen(buf);
    char tempBuf[MAX_LOG_LENGTH + 1] = { 0 };
    WCHAR wszBuf[MAX_LOG_LENGTH + 1] = { 0 };

    do
    {
        std::copy(buf + pos, buf + pos + MAX_LOG_LENGTH, tempBuf);

        tempBuf[MAX_LOG_LENGTH] = 0;

        MultiByteToWideChar(CP_UTF8, 0, tempBuf, -1, wszBuf, sizeof(wszBuf));
        OutputDebugStringW(wszBuf);

        pos += MAX_LOG_LENGTH;

    } while (pos < len);

    delete[] buf;
}

static void wsLog(const char * format, ...)
{
    va_list args;
    va_start(args, format);
    _winLog(format, args);
    va_end(args);
}

#else
#define wsLog printf
#endif

#define QUOTEME_(x) #x
#define QUOTEME(x) QUOTEME_(x)

// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif

// Since CCLOG isn't thread safe, we uses LOGD for multi-thread logging.
#ifdef ANDROID
    #if COCOS2D_DEBUG > 0
        #define LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG,__VA_ARGS__)
    #else
        #define LOGD(...)
    #endif

    #define LOGE(...)  __android_log_print(ANDROID_LOG_ERROR, LOG_TAG,__VA_ARGS__)
#else
    #if COCOS2D_DEBUG > 0
        #define LOGD(fmt, ...) wsLog("D/" LOG_TAG " (" QUOTEME(__LINE__) "): " fmt "", ##__VA_ARGS__)
    #else
        #define LOGD(fmt, ...)
    #endif

    #define LOGE(fmt, ...) wsLog("E/" LOG_TAG " (" QUOTEME(__LINE__) "): " fmt "", ##__VA_ARGS__)
#endif

// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
static void printWebSocketLog(int level, const char *line)
{
#if COCOS2D_DEBUG > 0
    static const char * const log_level_names[] = {
        "ERR",
        "WARN",
        "NOTICE",
        "INFO",
        "DEBUG",
        "PARSER",
        "HEADER",
        "EXTENSION",
        "CLIENT",
        "LATENCY",
    };

    char buf[30] = {0};
    int n;

    for (n = 0; n < LLL_COUNT; n++) {
        if (level != (1 << n))
            continue;
        sprintf(buf, "%s: ", log_level_names[n]);
        break;
    }

    LOGD("%s%s\n", buf, line);

#endif // #if COCOS2D_DEBUG > 0
}
#endif

NS_NETWORK_BEGIN

enum WS_MSG {
    WS_MSG_TO_SUBTRHEAD_SENDING_STRING = 0,
    WS_MSG_TO_SUBTRHEAD_SENDING_BINARY,
    WS_MSG_TO_SUBTHREAD_CREATE_CONNECTION
};

static std::vector<WebSocket*>* __websocketInstances = nullptr;
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
static std::mutex __instanceMutex;
#endif

// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
static struct lws_context* __wsContext = nullptr;
#endif
static WsThreadHelper* __wsHelper = nullptr;

#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
static std::string getFileNameForPath(const std::string& filePath)
{
    std::string fileName = filePath;
    const size_t lastSlashIdx = fileName.find_last_of("\\/");
    if (std::string::npos != lastSlashIdx)
    {
        fileName.erase(0, lastSlashIdx + 1);
    }
    return fileName;
}
#endif

// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
static struct lws_protocols __defaultProtocols[2];

static lws_context_creation_info convertToContextCreationInfo(const struct lws_protocols* protocols, bool peerServerCert)
{
    lws_context_creation_info info;
    memset(&info, 0, sizeof(info));
    /*
     * create the websocket context.  This tracks open connections and
     * knows how to route any traffic and which protocol version to use,
     * and if each connection is client or server side.
     *
     * For this client-only demo, we tell it to not listen on any port.
     */

    info.port = CONTEXT_PORT_NO_LISTEN;
    info.protocols = protocols;

    // FIXME: Disable 'permessage-deflate' extension temporarily because of issues:
    // https://github.com/cocos2d/cocos2d-x/issues/16045, https://github.com/cocos2d/cocos2d-x/issues/15767
    // libwebsockets issue: https://github.com/warmcat/libwebsockets/issues/593
    // Currently, we couldn't find out the exact reason.
    // libwebsockets official said it's probably an issue of user code
    // since 'libwebsockets' passed AutoBahn stressed Test.

    //    info.extensions = exts;

    info.gid = -1;
    info.uid = -1;
    if (peerServerCert)
    {
        info.options = LWS_SERVER_OPTION_EXPLICIT_VHOSTS | LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;
    }
    else
    {
        info.options = LWS_SERVER_OPTION_EXPLICIT_VHOSTS | LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT | LWS_SERVER_OPTION_PEER_CERT_NOT_REQUIRED;
    }
    info.user = nullptr;

    return info;
}
#endif

class WsMessage
{
public:
    WsMessage() : id(++__id), what(0), data(nullptr), user(nullptr){}
    unsigned int id;
    unsigned int what; // message type
    void* data;
    void* user;

private:
    static unsigned int __id;
};

unsigned int WsMessage::__id = 0;

/**
 *  @brief Websocket thread helper, it's used for sending message between UI thread and websocket thread.
 */
class WsThreadHelper
{
public:
    WsThreadHelper();
    ~WsThreadHelper();

    // Creates a new thread
    bool createWebSocketThread();
    // Quits websocket thread.
    void quitWebSocketThread();

    // Sends message to Cocos thread. It's needed to be invoked in Websocket thread.
    void sendMessageToCocosThread(const std::function<void()>& cb);

    // Sends message to Websocket thread. It's needs to be invoked in Cocos thread.
    void sendMessageToWebSocketThread(WsMessage *msg);

    // Waits the sub-thread (websocket thread) to exit,
    void joinWebSocketThread();

    void onSubThreadStarted();
    void onSubThreadLoop();
    void onSubThreadEnded();

protected:
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
    void wsThreadEntryFunc();
#endif
public:
    std::list<WsMessage*>* _subThreadWsMessageQueue;
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
    std::mutex   _subThreadWsMessageQueueMutex;
    std::thread* _subThreadInstance;
#endif
private:
    bool _needQuit;
};

// Wrapper for converting websocket callback from static function to member function of WebSocket class.
class WebSocketCallbackWrapper {
public:
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
    static int onSocketCallback(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len)
    {
        // Gets the user data from context. We know that it's a 'WebSocket' instance.
        if (wsi == nullptr) {
            return 0;
        }
        int ret = 0;
        WebSocket* ws = (WebSocket*)lws_wsi_user(wsi);
        if (ws != nullptr && __websocketInstances != nullptr)
        {
            if (std::find(__websocketInstances->begin(), __websocketInstances->end(), ws) != __websocketInstances->end())
            {
                ret = ws->onSocketCallback(wsi, reason, in, len);
            }
        }
        else
        {
//            LOGD("ws instance is nullptr.\n");
        }

        return ret;
    }
#endif
};

// Implementation of WsThreadHelper
WsThreadHelper::WsThreadHelper()
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
: _subThreadInstance(nullptr)
#endif
, _needQuit(false)
{
    _subThreadWsMessageQueue = new (std::nothrow) std::list<WsMessage*>();
}

WsThreadHelper::~WsThreadHelper()
{
    joinWebSocketThread();
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
    CC_SAFE_DELETE(_subThreadInstance);
#endif
    delete _subThreadWsMessageQueue;
}

bool WsThreadHelper::createWebSocketThread()
{
    // Creates websocket thread
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
    _subThreadInstance = new (std::nothrow) std::thread(&WsThreadHelper::wsThreadEntryFunc, this);
#endif
    return true;
}

void WsThreadHelper::quitWebSocketThread()
{
    _needQuit = true;
}

void WsThreadHelper::onSubThreadLoop()
{
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
    if (__wsContext)
#endif
    {
        //        _readyStateMutex.unlock();
        __wsHelper->_subThreadWsMessageQueueMutex.lock();
        bool isEmpty = __wsHelper->_subThreadWsMessageQueue->empty();

        if (!isEmpty)
        {
            auto iter = __wsHelper->_subThreadWsMessageQueue->begin();
            for (; iter != __wsHelper->_subThreadWsMessageQueue->end(); )
            {
                auto msg = (*iter);
                auto ws = (WebSocket*)msg->user;
                // TODO: ws may be a invalid pointer
                if (msg->what == WS_MSG_TO_SUBTHREAD_CREATE_CONNECTION)
                {
                    ws->onClientOpenConnectionRequest();
                    delete *iter;
                    iter = __wsHelper->_subThreadWsMessageQueue->erase(iter);
                }
                else
                {
                    ++iter;
                }


            }
        }
        __wsHelper->_subThreadWsMessageQueueMutex.unlock();

        // The second parameter passed to 'lws_service' means the timeout in milliseconds while polling websocket events.
        // The lower value the better, otherwise, it may trigger high CPU usage.
        // We set 2ms in 'lws_service' then sleep 3ms to make lower CPU cost.
        // Since messages are received in websocket thread and user code is in cocos thread, we need to post event to
        // cocos thread and trigger user callbacks by 'Scheduler::performFunctionInCocosThread'. If game's fps is set
        // to 60 (16.66ms), the latency will be (2ms + 3ms + 16.66ms + internet delay) > 21ms
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
        lws_service(__wsContext, 2);
        std::this_thread::sleep_for(std::chrono::milliseconds(3));
#endif
    }
}

void WsThreadHelper::onSubThreadStarted()
{
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
    int log_level = LLL_ERR | LLL_WARN | LLL_NOTICE | LLL_INFO/* | LLL_DEBUG | LLL_PARSER | LLL_HEADER*/ | LLL_EXT | LLL_CLIENT | LLL_LATENCY;
    lws_set_log_level(log_level, printWebSocketLog);

    memset(__defaultProtocols, 0, 2 * sizeof(struct lws_protocols));

    __defaultProtocols[0].name = "";
    __defaultProtocols[0].callback = WebSocketCallbackWrapper::onSocketCallback;
    __defaultProtocols[0].rx_buffer_size = WS_RX_BUFFER_SIZE;
    __defaultProtocols[0].id = std::numeric_limits<uint32_t>::max();

    lws_context_creation_info creationInfo = convertToContextCreationInfo(__defaultProtocols, true);
    __wsContext = lws_create_context(&creationInfo);
#endif
}

void WsThreadHelper::onSubThreadEnded()
{
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
    if (__wsContext != nullptr)
    {
        lws_context_destroy(__wsContext);
    }
#endif
}

// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
void WsThreadHelper::wsThreadEntryFunc()
{
    LOGD("WebSocket thread start, helper instance: %p\n", this);
    onSubThreadStarted();

    while (!_needQuit)
    {
        onSubThreadLoop();
    }

    onSubThreadEnded();

    LOGD("WebSocket thread exit, helper instance: %p\n", this);
}
#endif

void WsThreadHelper::sendMessageToCocosThread(const std::function<void()>& cb)
{
#ifdef USE_AGTK
	Director::getInstance()->getScheduler()->performFunctionInBackgroundInCocosThread(cb);
#else
    Director::getInstance()->getScheduler()->performFunctionInCocosThread(cb);
#endif
}

void WsThreadHelper::sendMessageToWebSocketThread(WsMessage *msg)
{
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
    std::lock_guard<std::mutex> lk(_subThreadWsMessageQueueMutex);
#endif
    _subThreadWsMessageQueue->push_back(msg);
}

void WsThreadHelper::joinWebSocketThread()
{
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
    if (_subThreadInstance->joinable())
    {
        _subThreadInstance->join();
    }
#endif
}

// Define a WebSocket frame
class WebSocketFrame
{
public:
    WebSocketFrame()
        : _payload(nullptr)
        , _payloadLength(0)
        , _frameLength(0)
    {
    }

    bool init(unsigned char* buf, ssize_t len)
    {
        if (buf == nullptr && len > 0)
            return false;

        if (!_data.empty())
        {
            LOGD("WebSocketFrame was initialized, should not init it again!\n");
            return false;
        }

        _data.reserve(LWS_PRE + len);
        _data.resize(LWS_PRE, 0x00);
        if (len > 0)
        {
            _data.insert(_data.end(), buf, buf + len);
        }

        _payload = _data.data() + LWS_PRE;
        _payloadLength = len;
        _frameLength = len;
        return true;
    }

    void update(ssize_t issued)
    {
        _payloadLength -= issued;
        _payload += issued;
    }

    unsigned char* getPayload() const { return _payload; }
    ssize_t getPayloadLength() const { return _payloadLength; }
    ssize_t getFrameLength() const { return _frameLength; }
private:
    unsigned char* _payload;
    ssize_t _payloadLength;

    ssize_t _frameLength;
    std::vector<unsigned char> _data;
};
//

void WebSocket::closeAllConnections()
{
    if (__websocketInstances != nullptr)
    {
        ssize_t count = __websocketInstances->size();
        for (ssize_t i = count-1; i >=0 ; i--)
        {
            WebSocket* instance = __websocketInstances->at(i);
            instance->close();
        }

// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
        std::lock_guard<std::mutex> lk(__instanceMutex);
#endif
        __websocketInstances->clear();
        delete __websocketInstances;
        __websocketInstances = nullptr;
    }
}

WebSocket::WebSocket()
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
: _readyState(State::CONNECTING)
, _wsInstance(nullptr)
, _lwsProtocols(nullptr)
#endif
, _isDestroyed(std::make_shared<std::atomic<bool>>(false))
, _delegate(nullptr)
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif
, _closeState(CloseState::NONE)
{
    // reserve data buffer to avoid allocate memory frequently
    _receivedData.reserve(WS_RESERVE_RECEIVE_BUFFER_SIZE);
    if (__websocketInstances == nullptr)
    {
        __websocketInstances = new (std::nothrow) std::vector<WebSocket*>();
    }

    __websocketInstances->push_back(this);
    
    std::shared_ptr<std::atomic<bool>> isDestroyed = _isDestroyed;
#ifdef USE_AGTK_CLOSE
	_resetDirectorListener = nullptr;
#else
    _resetDirectorListener = Director::getInstance()->getEventDispatcher()->addCustomEventListener(Director::EVENT_RESET, [this, isDestroyed](EventCustom*){
        if (*isDestroyed)
            return;
        close();
    });
#endif // USE_AGTK_CLOSE
}

WebSocket::~WebSocket()
{
    LOGD("In the destructor of WebSocket (%p)\n", this);

// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
    std::lock_guard<std::mutex> lk(__instanceMutex);
#endif
    if (__websocketInstances != nullptr)
    {
        auto iter = std::find(__websocketInstances->begin(), __websocketInstances->end(), this);
        if (iter != __websocketInstances->end())
        {
            __websocketInstances->erase(iter);
        }
        else
        {
            LOGD("ERROR: WebSocket instance (%p) wasn't added to the container which saves websocket instances!\n", this);
        }
    }

    if (__websocketInstances == nullptr || __websocketInstances->empty())
    {
        __wsHelper->quitWebSocketThread();
        LOGD("before join ws thread\n");
        __wsHelper->joinWebSocketThread();
        LOGD("after join ws thread\n");

        CC_SAFE_DELETE(__wsHelper);
    }

// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif

    for(auto name:_protocolNames){
        free(name);
    }
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
    free(_lwsProtocols);
#endif
    
    Director::getInstance()->getEventDispatcher()->removeEventListener(_resetDirectorListener);
    
    *_isDestroyed = true;
}


bool WebSocket::init(const Delegate& delegate,
                     const std::string& url,
                     const std::vector<std::string>* protocols/* = nullptr*/,
                     const std::string& caFilePath/* = ""*/)
{
    _delegate = const_cast<Delegate*>(&delegate);
    _url = url;
    _caFilePath = caFilePath;

    if (_url.empty())
        return false;

    if (protocols != nullptr && !protocols->empty())
    {
        size_t size = protocols->size();
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
        _lwsProtocols = (struct lws_protocols*)malloc((size + 1) * sizeof(struct lws_protocols));
        memset(_lwsProtocols, 0, (size + 1) * sizeof(struct lws_protocols));

        static uint32_t __wsId = 0;
#endif

        for (size_t i = 0; i < size; ++i)
        {
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
            _lwsProtocols[i].callback = WebSocketCallbackWrapper::onSocketCallback;
#endif
            size_t nameLen = protocols->at(i).length();
            char* name = (char*)malloc(nameLen + 1);
            _protocolNames.push_back(name);
            name[nameLen] = '\0';
            strcpy(name, protocols->at(i).c_str());
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
            _lwsProtocols[i].name = name;
            _lwsProtocols[i].id = ++__wsId;
            _lwsProtocols[i].rx_buffer_size = WS_RX_BUFFER_SIZE;
            _lwsProtocols[i].per_session_data_size = 0;
            _lwsProtocols[i].user = nullptr;
#endif

            _clientSupportedProtocols += name;
            if (i < (size - 1))
            {
                _clientSupportedProtocols += ",";
            }
        }
    }

    bool isWebSocketThreadCreated = true;
    if (__wsHelper == nullptr)
    {
        __wsHelper = new (std::nothrow) WsThreadHelper();
        isWebSocketThreadCreated = false;
    }

// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif

    WsMessage* msg = new (std::nothrow) WsMessage();
    msg->what = WS_MSG_TO_SUBTHREAD_CREATE_CONNECTION;
    msg->user = this;
    __wsHelper->sendMessageToWebSocketThread(msg);

    // fixed https://github.com/cocos2d/cocos2d-x/issues/17433
    // createWebSocketThread has to be after message WS_MSG_TO_SUBTHREAD_CREATE_CONNECTION was sent.
    // And websocket thread should only be created once.
    if (!isWebSocketThreadCreated)
    {
        __wsHelper->createWebSocketThread();
    }

    return true;
}

void WebSocket::send(const std::string& message)
{
    if (_readyState == State::OPEN)
    {
        // In main thread
        Data* data = new (std::nothrow) Data();
        data->bytes = (char*)malloc(message.length() + 1);
        // Make sure the last byte is '\0'
        data->bytes[message.length()] = '\0';
        strcpy(data->bytes, message.c_str());
        data->len = static_cast<ssize_t>(message.length());

        WsMessage* msg = new (std::nothrow) WsMessage();
        msg->what = WS_MSG_TO_SUBTRHEAD_SENDING_STRING;
        msg->data = data;
        msg->user = this;
        __wsHelper->sendMessageToWebSocketThread(msg);
    }
    else
    {
        LOGD("Couldn't send message since websocket wasn't opened!\n");
    }
}

void WebSocket::send(const unsigned char* binaryMsg, unsigned int len)
{
    if (_readyState == State::OPEN)
    {
        // In main thread
        Data* data = new (std::nothrow) Data();
        if (len == 0)
        {
            // If data length is zero, allocate 1 byte for safe.
            data->bytes = (char*)malloc(1);
            data->bytes[0] = '\0';
        }
        else
        {
            data->bytes = (char*)malloc(len);
            memcpy((void*)data->bytes, (void*)binaryMsg, len);
        }
        data->len = len;

        WsMessage* msg = new (std::nothrow) WsMessage();
        msg->what = WS_MSG_TO_SUBTRHEAD_SENDING_BINARY;
        msg->data = data;
        msg->user = this;
        __wsHelper->sendMessageToWebSocketThread(msg);
    }
    else
    {
        LOGD("Couldn't send message since websocket wasn't opened!\n");
    }
}

void WebSocket::close()
{
    if (_closeState != CloseState::NONE)
    {
        LOGD("close was invoked, don't invoke it again!\n");
        return;
    }

    _closeState = CloseState::SYNC_CLOSING;
    LOGD("close: WebSocket (%p) is closing...\n", this);
    {
        _readyStateMutex.lock();
        if (_readyState == State::CLOSED)
        {
            // If readState is closed, it means that onConnectionClosed was invoked in websocket thread,
            // but the callback of performInCocosThread has not been triggered. We need to invoke
            // onClose to release the websocket instance.
            _readyStateMutex.unlock();
            _delegate->onClose(this);
            return;
        }

        _readyState = State::CLOSING;
        _readyStateMutex.unlock();
    }

    {
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
        std::unique_lock<std::mutex> lkClose(_closeMutex);
        _closeCondition.wait(lkClose);
#endif
        _closeState = CloseState::SYNC_CLOSED;
    }

    // Wait 5 milliseconds for onConnectionClosed to exit!
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    _delegate->onClose(this);
}

void WebSocket::closeAsync()
{
    if (_closeState != CloseState::NONE)
    {
        LOGD("close was invoked, don't invoke it again!\n");
        return;
    }

    _closeState = CloseState::ASYNC_CLOSING;

    LOGD("closeAsync: WebSocket (%p) is closing...\n", this);
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
    std::lock_guard<std::mutex> lk(_readyStateMutex);
#endif
    if (_readyState == State::CLOSED || _readyState == State::CLOSING)
    {
        LOGD("closeAsync: WebSocket (%p) was closed, no need to close it again!\n", this);
        return;
    }

    _readyState = State::CLOSING;
}

WebSocket::State WebSocket::getReadyState()
{
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
    std::lock_guard<std::mutex> lk(_readyStateMutex);
#endif
    return _readyState;
}

// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
struct lws_vhost* WebSocket::createVhost(struct lws_protocols* protocols, int& sslConnection)
{
    auto fileUtils = FileUtils::getInstance();
    bool isCAFileExist = fileUtils->isFileExist(_caFilePath);
    if (isCAFileExist)
    {
        _caFilePath = fileUtils->fullPathForFilename(_caFilePath);
    }

    lws_context_creation_info info = convertToContextCreationInfo(protocols, isCAFileExist);

    if (sslConnection != 0)
    {
        if (isCAFileExist)
        {
#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
            // if ca file is in the apk, try to extract it to writable path
            std::string writablePath = fileUtils->getWritablePath();
            std::string caFileName = getFileNameForPath(_caFilePath);
            std::string newCaFilePath = writablePath + caFileName;

            if (fileUtils->isFileExist(newCaFilePath))
            {
                LOGD("CA file (%s) in writable path exists!", newCaFilePath.c_str());
                _caFilePath = newCaFilePath;
                info.ssl_ca_filepath = _caFilePath.c_str();
            }
            else
            {
                if (fileUtils->isFileExist(_caFilePath))
                {
                    std::string fullPath = fileUtils->fullPathForFilename(_caFilePath);
                    LOGD("Found CA file: %s", fullPath.c_str());
                    if (fullPath[0] != '/')
                    {
                        LOGD("CA file is in APK");
                        auto caData = fileUtils->getDataFromFile(fullPath);
                        if (!caData.isNull())
                        {
                            FILE* fp = fopen(newCaFilePath.c_str(), "wb");
                            if (fp != nullptr)
                            {
                                LOGD("New CA file path: %s", newCaFilePath.c_str());
                                fwrite(caData.getBytes(), caData.getSize(), 1, fp);
                                fclose(fp);
                                _caFilePath = newCaFilePath;
                                info.ssl_ca_filepath = _caFilePath.c_str();
                            }
                            else
                            {
                                CCASSERT(false, "Open new CA file failed");
                            }
                        }
                        else
                        {
                            CCASSERT(false, "CA file is empty!");
                        }
                    }
                    else
                    {
                        LOGD("CA file isn't in APK!");
                        _caFilePath = fullPath;
                        info.ssl_ca_filepath = _caFilePath.c_str();
                    }
                }
                else
                {
                    CCASSERT(false, "CA file doesn't exist!");
                }
            }
#else
            info.ssl_ca_filepath = _caFilePath.c_str();
#endif
        }
        else
        {
            LOGD("WARNING: CA Root file isn't set. SSL connection will not peer server certificate\n");
            sslConnection = sslConnection | LCCSCF_ALLOW_SELFSIGNED | LCCSCF_SKIP_SERVER_CERT_HOSTNAME_CHECK;
        }
    }

    lws_vhost* vhost = lws_create_vhost(__wsContext, &info);

    return vhost;
}
#endif

void WebSocket::onClientOpenConnectionRequest()
{
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
    if (nullptr != __wsContext)
#endif
    {
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
        static const struct lws_extension exts[] = {
            {
                "permessage-deflate",
                lws_extension_callback_pm_deflate,
                // client_no_context_takeover extension is not supported in the current version, it will cause connection fail
                // It may be a bug of lib websocket build
                //            "permessage-deflate; client_no_context_takeover; client_max_window_bits"
                "permessage-deflate; client_max_window_bits"
            },
            {
                "deflate-frame",
                lws_extension_callback_pm_deflate,
                "deflate_frame"
            },
            { nullptr, nullptr, nullptr /* terminator */ }
        };
#endif

        _readyStateMutex.lock();
        _readyState = State::CONNECTING;
        _readyStateMutex.unlock();

// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
        Uri uri = Uri::parse(_url);
        LOGD("scheme: %s, host: %s, port: %d, path: %s\n", uri.getScheme().c_str(), uri.getHostName().c_str(), static_cast<int>(uri.getPort()), uri.getPathEtc().c_str());

        int sslConnection = 0;
        if (uri.isSecure())
            sslConnection = LCCSCF_USE_SSL;

        struct lws_vhost* vhost = nullptr;
        if (_lwsProtocols != nullptr)
        {
            vhost = createVhost(_lwsProtocols, sslConnection);
        }
        else
        {
            vhost = createVhost(__defaultProtocols, sslConnection);
        }

        int port = static_cast<int>(uri.getPort());
        if (port == 0)
            port = uri.isSecure() ? 443 : 80;

        const std::string& hostName = uri.getHostName();
        std::string path = uri.getPathEtc();
        const std::string& authority = uri.getAuthority();
        if (path.empty())
            path = "/";

        struct lws_client_connect_info connectInfo;
        memset(&connectInfo, 0, sizeof(connectInfo));
        connectInfo.context = __wsContext;
        connectInfo.address = hostName.c_str();
        connectInfo.port = port;
        connectInfo.ssl_connection = sslConnection;
        connectInfo.path = path.c_str();
        connectInfo.host = hostName.c_str();
        connectInfo.origin = authority.c_str();
        connectInfo.protocol = _clientSupportedProtocols.empty() ? nullptr : _clientSupportedProtocols.c_str();
        connectInfo.ietf_version_or_minus_one = -1;
        connectInfo.userdata = this;
        connectInfo.client_exts = exts;
        connectInfo.vhost = vhost;

        _wsInstance = lws_client_connect_via_info(&connectInfo);

        if (nullptr == _wsInstance)
        {
            onConnectionError();
            return;
        }
#endif
	}
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
    else
    {
        LOGE("Create websocket context failed!");
    }
#endif
}

int WebSocket::onClientWritable()
{
//    LOGD("onClientWritable ... \n");
    {
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
        std::lock_guard<std::mutex> readMutex(_readyStateMutex);
#endif
        if (_readyState == State::CLOSING)
        {
            LOGD("Closing websocket (%p) connection.\n", this);
            return -1;
        }
    }

    do
    {
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
        std::lock_guard<std::mutex> lk(__wsHelper->_subThreadWsMessageQueueMutex);
#endif

        if (__wsHelper->_subThreadWsMessageQueue->empty())
        {
            break;
        }

        std::list<WsMessage*>::iterator iter = __wsHelper->_subThreadWsMessageQueue->begin();

        while (iter != __wsHelper->_subThreadWsMessageQueue->end())
        {
            WsMessage* msg = *iter;
            if (msg->user == this)
            {
                break;
            }
            else
            {
                ++iter;
            }
        }

        ssize_t bytesWrite = 0;
        if (iter != __wsHelper->_subThreadWsMessageQueue->end())
        {
            WsMessage* subThreadMsg = *iter;

            Data* data = (Data*)subThreadMsg->data;

            const ssize_t c_bufferSize = WS_RX_BUFFER_SIZE;

            const ssize_t remaining = data->len - data->issued;
            const ssize_t n = std::min(remaining, c_bufferSize);

            WebSocketFrame* frame = nullptr;

            if (data->ext)
            {
                frame = (WebSocketFrame*)data->ext;
            }
            else
            {
                frame = new (std::nothrow) WebSocketFrame();
                bool success = frame && frame->init((unsigned char*)(data->bytes + data->issued), n);
                if (success)
                {
                    data->ext = frame;
                }
                else
                { // If frame initialization failed, delete the frame and drop the sending data
                  // These codes should never be called.
                    LOGD("WebSocketFrame initialization failed, drop the sending data, msg(%d)\n", (int)subThreadMsg->id);
                    delete frame;
                    CC_SAFE_FREE(data->bytes);
                    CC_SAFE_DELETE(data);
                    __wsHelper->_subThreadWsMessageQueue->erase(iter);
                    CC_SAFE_DELETE(subThreadMsg);
                    break;
                }
            }

// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
            int writeProtocol;

            if (data->issued == 0)
            {
                if (WS_MSG_TO_SUBTRHEAD_SENDING_STRING == subThreadMsg->what)
                {
                    writeProtocol = LWS_WRITE_TEXT;
                }
                else
                {
                    writeProtocol = LWS_WRITE_BINARY;
                }

                // If we have more than 1 fragment
                if (data->len > c_bufferSize)
                    writeProtocol |= LWS_WRITE_NO_FIN;
            } else {
                // we are in the middle of fragments
                writeProtocol = LWS_WRITE_CONTINUATION;
                // and if not in the last fragment
                if (remaining != n)
                    writeProtocol |= LWS_WRITE_NO_FIN;
            }
#endif

// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
            bytesWrite = lws_write(_wsInstance, frame->getPayload(), frame->getPayloadLength(), (lws_write_protocol)writeProtocol);
#endif
            // Handle the result of lws_write
            // Buffer overrun?
            if (bytesWrite < 0)
            {
                LOGD("ERROR: msg(%u), lws_write return: %d, but it should be %d, drop this message.\n", subThreadMsg->id, (int)bytesWrite, (int)n);
                // socket error, we need to close the socket connection
                CC_SAFE_FREE(data->bytes);
                delete ((WebSocketFrame*)data->ext);
                data->ext = nullptr;
                CC_SAFE_DELETE(data);
                __wsHelper->_subThreadWsMessageQueue->erase(iter);
                CC_SAFE_DELETE(subThreadMsg);

                closeAsync();
            }
            else if (bytesWrite < frame->getPayloadLength())
            {
                frame->update(bytesWrite);
                LOGD("frame wasn't sent completely, bytesWrite: %d, remain: %d\n", (int)bytesWrite, (int)frame->getPayloadLength());
            }
            // Do we have another fragments to send?
            else if (remaining > frame->getFrameLength() && bytesWrite == frame->getPayloadLength())
            {
                // A frame was totally sent, plus data->issued to send next frame
                LOGD("msg(%u) append: %d + %d = %d\n", subThreadMsg->id, (int)data->issued, (int)frame->getFrameLength(), (int)(data->issued + frame->getFrameLength()));
                data->issued += frame->getFrameLength();
                delete ((WebSocketFrame*)data->ext);
                data->ext = nullptr;
            }
            // Safely done!
            else
            {
                LOGD("Safely done, msg(%d)!\n", subThreadMsg->id);
                if (remaining == frame->getFrameLength())
                {
                    LOGD("msg(%u) append: %d + %d = %d\n", subThreadMsg->id, (int)data->issued, (int)frame->getFrameLength(), (int)(data->issued + frame->getFrameLength()));
                    LOGD("msg(%u) was totally sent!\n", subThreadMsg->id);
                }
                else
                {
                    LOGD("ERROR: msg(%u), remaining(%d) < bytesWrite(%d)\n", subThreadMsg->id, (int)remaining, (int)frame->getFrameLength());
                    LOGD("Drop the msg(%u)\n", subThreadMsg->id);
                    closeAsync();
                }

                CC_SAFE_FREE(data->bytes);
                delete ((WebSocketFrame*)data->ext);
                data->ext = nullptr;
                CC_SAFE_DELETE(data);
                __wsHelper->_subThreadWsMessageQueue->erase(iter);
                CC_SAFE_DELETE(subThreadMsg);

                LOGD("-----------------------------------------------------------\n");
            }
        }

    } while(false);

// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
    if (_wsInstance != nullptr)
    {
        lws_callback_on_writable(_wsInstance);
    }
#endif

    return 0;
}

// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
int WebSocket::onClientReceivedData(void* in, ssize_t len)
#endif
{
    // In websocket thread
    static int packageIndex = 0;
    packageIndex++;
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
    if (in != nullptr && len > 0)
    {
        LOGD("Receiving data:index:%d, len=%d\n", packageIndex, (int)len);

        unsigned char* inData = (unsigned char*)in;
        _receivedData.insert(_receivedData.end(), inData, inData + len);
    }
#endif
    else
    {
        LOGD("Empty message received, index=%d!\n", packageIndex);
    }

// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
    // If no more data pending, send it to the client thread
    size_t remainingSize = lws_remaining_packet_payload(_wsInstance);
    int isFinalFragment = lws_is_final_fragment(_wsInstance);
//    LOGD("remainingSize: %d, isFinalFragment: %d\n", (int)remainingSize, isFinalFragment);

    if (remainingSize == 0 && isFinalFragment)
    {
        std::vector<char>* frameData = new (std::nothrow) std::vector<char>(std::move(_receivedData));

        // reset capacity of received data buffer
        _receivedData.reserve(WS_RESERVE_RECEIVE_BUFFER_SIZE);

        ssize_t frameSize = frameData->size();

        bool isBinary = (lws_frame_is_binary(_wsInstance) != 0);

        if (!isBinary)
        {
            frameData->push_back('\0');
        }

        std::shared_ptr<std::atomic<bool>> isDestroyed = _isDestroyed;
        __wsHelper->sendMessageToCocosThread([this, frameData, frameSize, isBinary, isDestroyed](){
            // In UI thread
            LOGD("Notify data len %d to Cocos thread.\n", (int)frameSize);

            Data data;
            data.isBinary = isBinary;
            data.bytes = (char*)frameData->data();
            data.len = frameSize;

            if (*isDestroyed)
            {
                LOGD("WebSocket instance was destroyed!\n");
            }
            else
            {
                _delegate->onMessage(this, data);
            }

            delete frameData;
        });
    }

    return 0;
#endif
}

// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
int WebSocket::onConnectionOpened()
#endif
{
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else	
    const lws_protocols* lwsSelectedProtocol = lws_get_protocol(_wsInstance);
    _selectedProtocol = lwsSelectedProtocol->name;
    LOGD("onConnectionOpened...: %p, client protocols: %s, server selected protocol: %s\n", this, _clientSupportedProtocols.c_str(), _selectedProtocol.c_str());
    /*
     * start the ball rolling,
     * LWS_CALLBACK_CLIENT_WRITEABLE will come next service
     */
    lws_callback_on_writable(_wsInstance);
#endif

    {
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
        std::lock_guard<std::mutex> lk(_readyStateMutex);
        if (_readyState == State::CLOSING || _readyState == State::CLOSED)
        {
            return 0;
        }
        _readyState = State::OPEN;
#endif
    }

// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
    std::shared_ptr<std::atomic<bool>> isDestroyed = _isDestroyed;
    __wsHelper->sendMessageToCocosThread([this, isDestroyed](){
        if (*isDestroyed)
        {
            LOGD("WebSocket instance was destroyed!\n");
        }
        else
        {
            _delegate->onOpen(this);
        }
    });
    return 0;
#endif
}

// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
int WebSocket::onConnectionError()
{
    {
        std::lock_guard<std::mutex> lk(_readyStateMutex);
        LOGD("WebSocket (%p) onConnectionError, state: %d ...\n", this, (int)_readyState);
        if (_readyState == State::CLOSED)
        {
            return 0;
        }
        _readyState = State::CLOSING;
    }

    std::shared_ptr<std::atomic<bool>> isDestroyed = _isDestroyed;
    __wsHelper->sendMessageToCocosThread([this, isDestroyed](){
        if (*isDestroyed)
        {
            LOGD("WebSocket instance was destroyed!\n");
        }
        else
        {
            _delegate->onError(this, ErrorCode::CONNECTION_FAILURE);
        }
    });

    onConnectionClosed();

    return 0;
#endif
}

// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
int WebSocket::onConnectionClosed()
{
    {
        std::lock_guard<std::mutex> lk(_readyStateMutex);
        LOGD("WebSocket (%p) onConnectionClosed, state: %d ...\n", this, (int)_readyState);
        if (_readyState == State::CLOSED)
        {
            return 0;
        }

        if (_readyState == State::CLOSING)
        {
            if (_closeState == CloseState::SYNC_CLOSING)
            {
                LOGD("onConnectionClosed, WebSocket (%p) is closing by client synchronously.\n", this);
                for(;;)
                {
                    std::lock_guard<std::mutex> lkClose(_closeMutex);
                    _closeCondition.notify_one();
                    if (_closeState == CloseState::SYNC_CLOSED)
                    {
                        break;
                    }
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                }

                return 0;
            }
            else if (_closeState == CloseState::ASYNC_CLOSING)
            {
                LOGD("onConnectionClosed, WebSocket (%p) is closing by client asynchronously.\n", this);
            }
            else
            {
                LOGD("onConnectionClosed, WebSocket (%p) is closing by server.\n", this);
            }
        }
        else
        {
            LOGD("onConnectionClosed, WebSocket (%p) is closing by server.\n", this);
        }

        _readyState = State::CLOSED;
    }

    std::shared_ptr<std::atomic<bool>> isDestroyed = _isDestroyed;
    __wsHelper->sendMessageToCocosThread([this, isDestroyed](){
        if (*isDestroyed)
        {
            LOGD("WebSocket instance (%p) was destroyed!\n", this);
        }
        else
        {
            _delegate->onClose(this);
        }
    });

    LOGD("WebSocket (%p) onConnectionClosed DONE!\n", this);
    return 0;
#endif
}

// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
int WebSocket::onSocketCallback(struct lws *wsi,
                     int reason,
                     void *in, ssize_t len)
{
    //LOGD("socket callback for %d reason\n", reason);

    int ret = 0;
    switch (reason)
    {
        case LWS_CALLBACK_CLIENT_ESTABLISHED:
            ret = onConnectionOpened();
            break;

        case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
            ret = onConnectionError();
            break;

        case LWS_CALLBACK_WSI_DESTROY:
            ret = onConnectionClosed();
            break;

        case LWS_CALLBACK_CLIENT_RECEIVE:
            ret = onClientReceivedData(in, len);
            break;

        case LWS_CALLBACK_CLIENT_WRITEABLE:
            ret = onClientWritable();
            break;
        case LWS_CALLBACK_CHANGE_MODE_POLL_FD:
        case LWS_CALLBACK_LOCK_POLL:
        case LWS_CALLBACK_UNLOCK_POLL:
            break;
        case LWS_CALLBACK_PROTOCOL_INIT:
            LOGD("protocol init...");
            break;
        case LWS_CALLBACK_PROTOCOL_DESTROY:
            LOGD("protocol destroy...");
            break;
        default:
            LOGD("WebSocket (%p) Unhandled websocket event: %d\n", this, reason);
            break;
    }

    return ret;
}
#endif

NS_NETWORK_END
