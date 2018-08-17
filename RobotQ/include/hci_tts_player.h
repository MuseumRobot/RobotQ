/** 
 * @file    hci_tts_player.h 
 * @brief   HCI_TTS_PLAYER SDK 头文件  
 */  

#ifndef __HCI_TTS_PLAYER_HEADER__
#define __HCI_TTS_PLAYER_HEADER__

#include "hci_sys.h"

#ifdef __cplusplus
extern "C"
{
#endif


/** @defgroup HCI_TTS_PLAYER_API 灵云TTS播放器API */
/* @{ */

    /** @defgroup HCI_PLAYER_STRUCT 结构体 */
    /* @{ */
    
    /**
	*@brief	模块名称
	*/
#ifndef MODULE_NAME
#define MODULE_NAME    "HCI_TTS_PLAYER"
#endif
    
/**
 * @brief	播放器回调时的通知事件
 */
typedef enum 
{
	PLAYER_EVENT_BEGIN,				///< 开始播放
	PLAYER_EVENT_PAUSE,				///< 暂停播放
	PLAYER_EVENT_RESUME,			///< 恢复播放
	PLAYER_EVENT_PROGRESS,			///< 播放进度
	PLAYER_EVENT_BUFFERING,			///< 播放缓冲,等待合成数据
	PLAYER_EVENT_END,				///< 播放完毕
	PLAYER_EVENT_ENGINE_ERROR,		///< 引擎出错
	PLAYER_EVENT_DEVICE_ERROR,		///< 设备出错
	PLAYER_EVENT_MALLOC_ERROR,		///< 分配空间失败
	PLAYER_EVENT_BUFFERING_END      ///< 缓冲结束，继续播放
} 
PLAYER_EVENT;

/**
 * @brief	播放机状态
 */
typedef enum
{
	PLAYER_STATE_NOT_INIT,			///< 没有初始化
	PLAYER_STATE_IDLE,				///< 空闲状态，可以启动播放
	PLAYER_STATE_PLAYING,			///< 正在播放状态
	PLAYER_STATE_PAUSE,				///< 暂停播放状态
	PLAYER_STATE_ERROR,				///< 错误状态
} 
PLAYER_STATE;

/**
* @brief	播放器错误码
*/
typedef enum
{
	PLAYER_ERR_UNKNOWN = -1,					///< -1: 未知错误，不会出现

	PLAYER_ERR_NONE = 0,						///< 0: 成功

	PLAYER_ERR_NOT_INIT,						///< 1: 没有初始化
	PLAYER_ERR_ALREADY_INIT,					///< 2: 已经初始化
	PLAYER_ERR_OPEN_DEVICE,						///< 3: 打开设备出错
	PLAYER_ERR_ALREADY_BEGIN,					///< 4: 已经开始
	PLAYER_ERR_NOT_BEGIN,						///< 5: 没有开始
	PLAYER_ERR_OUT_OF_MEMORY,					///< 6: 分配空间失败
	PLAYER_ERR_ENGINE_ERROR,					///< 7: 初始化 TTS SDK 出错
}
PLAYER_ERR_CODE;

/**  
* @brief	播放器事件变化回调
* @note
* PLAYER_EVENT_PROGRESS 事件不会在该回调中触发，它只会在 Callback_PlayerEventProgressChange 中触发
* PLAYER_EVENT_ENGINE_ERROR、PLAYER_EVENT_DEVICE_ERROR 事件不会在该回调中触发，只会在 Callback_PlayerEventPlayerError 中触发
* @param	ePlayerEvent		回调时的通知事件
* @param	pUsrParam			用户自定义参数
*/
typedef void (HCIAPI * Callback_PlayerEventStateChange)(
	_MUST_ _IN_ PLAYER_EVENT ePlayerEvent,
	_OPT_ _IN_ void * pUsrParam );

/**  
* @brief	播放进度变化通知
* @note
* 只会触发 PLAYER_EVENT_PROGRESS 事件
* @param	ePlayerEvent		回调时的通知事件
* @param	nStart				本次进度变化时开始的文件位置(单位：字节)
* @param	nStop				本次进度变化时结束的文件位置(单位：字节)
* @param	pUsrParam			用户自定义参数
*/ 
typedef void (HCIAPI * Callback_PlayerEventProgressChange)(
	_MUST_ _IN_ PLAYER_EVENT ePlayerEvent,
	_MUST_ _IN_ int nStart,
	_MUST_ _IN_ int nStop,
	_OPT_ _IN_ void * pUsrParam );

/**  
* @brief	出错状态的回调
* 只会触发 PLAYER_EVENT_ENGINE_ERROR 事件
* @param	ePlayerEvent		回调时的通知事件
* @param	eErrorCode			错误码
* @param	pUsrParam			用户自定义参数
*/ 
typedef void (HCIAPI * Callback_PlayerEventPlayerError)(
	_MUST_ _IN_ PLAYER_EVENT ePlayerEvent,
	_MUST_ _IN_ HCI_ERR_CODE eErrorCode,
	_OPT_ _IN_ void * pUsrParam );

/**  
 * @brief	audiosession回调
 * @note
 * 只在iOS平台使用
 * @param	pExtendParam	扩展参数，当前无效
 * @param	pUsrParam		用户数据
 */ 
typedef void (HCIAPI * Callback_PlayerSetAudioSession)(
    _MUST_ _IN_ void * pExtendParam,
    _OPT_ _IN_ void * pUsrParam);

/**  
* @ingroup HCI_PLAYER_STRUCT
* @brief	播放器回调函数的结构体
*/
typedef struct _PLAYER_CALLBACK_PARAM {
	Callback_PlayerEventStateChange pfnStateChange;         ///< 播放器状态回调
	void * pvStateChangeUsrParam;                           ///< 播放器状态回调自定义参数
	Callback_PlayerEventProgressChange pfnProgressChange;   ///< 播放器进度回调
	void * pvProgressChangeUsrParam;                        ///< 播放器进度回调自定义参数
	Callback_PlayerEventPlayerError pfnPlayerError;         ///< 播放器错误回调
	void * pvPlayerErrorUsrParam;                           ///< 播放器错误回调自定义参数
} PLAYER_CALLBACK_PARAM;

/* @} */

/** @defgroup HCI_PLAYER_FUNC 函数 */
/* @{ */

/**  
 * @brief	播放器SDK初始化
 * @param	pszSdkConfig		初始化播放器的配置串 详细说明 参见 hci_tts_init
 * @param	pCallbackParam		回调函数的集合
 * @return	
 * @n
 *	<table>
 *		<tr><td>@ref PLAYER_ERR_NONE</td><td>操作成功</td></tr>
 *		<tr><td>@ref PLAYER_ERR_ALREADY_INIT</td><td>已经初始化</td></tr>
 *		<tr><td>@ref PLAYER_ERR_ENGINE_ERROR</td><td>TTS SDK 初始化失败，失败的原因会通过 Callback_PlayerEventPlayerError 回调返回</td></tr>
 *		<tr><td>@ref PLAYER_ERR_OUT_OF_MEMORY</td><td>分配空间失败</td></tr>
 *	</table>
 *
 *  <b>IOS特殊配置</b>
 *  @n
 *  配置项"volumeChannel"，可选值"media"或"system"，默认值为"system"。当该配置项的值为"media"时，TTS播放器初始化后，
 *  无论是否处于播放状态，按下音量键调节的是媒体音量；当值为"system"时，非播放状态，按下音量键调节的是铃声音量，
 *  TTS播放器播放时，调节的是媒体音量。备注：启动此模式需要在设置中将铃声和提醒中的用按钮调整开关打开。
 */ 
PLAYER_ERR_CODE HCIAPI hci_tts_player_init(
		_MUST_ _IN_ const char * pszSdkConfig,
		_MUST_ _IN_ PLAYER_CALLBACK_PARAM *pCallbackParam);

/**  
 * @brief	设置audiosession回调
 * @param	pfnCallBack		audiosession回调
 * @param	pUsrParam		用户数据
 * @return	
 * @n
 *	<table>
 *		<tr><td>@ref PLAYER_ERR_NONE</td><td>操作成功</td></tr>
 *		<tr><td>@ref PLAYER_ERR_NOT_INIT</td><td>player尚未初始化</td></tr>
 *	</table>
 * @n
 * @note
 * 此接口只在iOS平台使用
 * player默认是与其他声音（如音乐播放器）混音的。如果你的应用需要在其他声音播放的时候响应中断进行相应的处理，
 * 可以调用此函数，设置Callback_PlayerSetAudioSession回调，在回调中进行audiosession的设置。如何设置audiosession，请参
 * 考苹果AudioToolbox的官方文档。下面是一个设置audiosession的示例
 @code
 *  void playerSetAudioSessionCallback(void * pExtendParam,void * pUsrParam)
 *	{
 *		//初始化audiosession
 *		AudioSessionInitialize(NULL, NULL, MyAudioSessionInterruptionListener, pUsrParam);
 *		//设置kAudioSessionProperty_AudioCategory
 *		UInt32 sessionCategory = kAudioSessionCategory_MediaPlayback;
 *		AudioSessionSetProperty ( kAudioSessionProperty_AudioCategory, sizeof (sessionCategory), &sessionCategory);
 *		//设置使用扬声器播放
 *		UInt32 audioRouteOverride = kAudioSessionOverrideAudioRoute_Speaker;
 *		AudioSessionSetProperty(kAudioSessionOverrideAudioRoute_None, sizeof(audioRouteOverride), &audioRouteOverride);
 *	}
 @endcode
 */ 
PLAYER_ERR_CODE hci_tts_player_set_audio_session_callback(
    _MUST_ _IN_ Callback_PlayerSetAudioSession pfnCallBack ,
    _OPT_ _IN_ void * pUsrParam);

    
/**
 * @brief	得到播放器的当前状态
 * @return	播放器的当前状态
 */ 
PLAYER_STATE HCIAPI hci_tts_player_get_state();

/**  
 * @brief	播放器当前是否可以停止
 * @details	当播放器处于 PLAYER_STATE_PLAYING 或者 PLAYER_STATE_PAUSE时，才能够被停止
 *          此函数只是为了方便检测是否可以停止的一个小工具函数
 * @return	
 * @n
 *	<table>
 *		<tr><td>true</td><td>可以停止播放器</td></tr>
 *		<tr><td>false</td><td>不可以停止播放器</td></tr>
 *	</table>
 */ 
bool HCIAPI hci_tts_player_can_stop();

/**  
 * @brief	启动播放器，播放一串文本
 * @details
 * 配置串中包括识别参数
 * @note
 * 播放器仅提供对 pcm8k8bit pcm8k16bit pcm16k8bit pcm16k16bit pcm11k8bit pcm11k16bit 格式的支持 
 * 如果不是播放器支持的格式，会在 Callback_PlayerEventStateChange 中触发打开设备失败的事件
 * @param	pszText				要播放的文本，UTF-8编码，以'\0'结束
 * @param	pszConfig			播放相关的配置串，可通过deviceId指定播放设备，其余参数详细说明参见 hci_tts_session_start 需要添加合成的配置项
 * @return	
 * @n
 *	<table>
 *		<tr><td>@ref PLAYER_ERR_NONE</td><td>操作成功</td></tr>
 *		<tr><td>@ref PLAYER_ERR_NOT_INIT</td><td>没有初始化</td></tr>
 *		<tr><td>@ref PLAYER_ERR_ALREADY_BEGIN</td><td>已经开始播放</td></tr>
 *		<tr><td>@ref PLAYER_ERR_OUT_OF_MEMORY</td><td>分配空间失败</td></tr>
 *	</table>
 */ 
PLAYER_ERR_CODE HCIAPI hci_tts_player_start(
	_MUST_ _IN_ const char * pszText,
	_MUST_ _IN_ const char * pszConfig);

/**  
 * @brief	暂停播放器
 * @return	
 * @n
 *	<table>
 *		<tr><td>@ref PLAYER_ERR_NONE</td><td>操作成功</td></tr>
 *		<tr><td>@ref PLAYER_ERR_NOT_INIT</td><td>没有初始化</td></tr>
 *		<tr><td>@ref PLAYER_ERR_NOT_BEGIN</td><td>没有开始播放</td></tr>
 *	</table>
 */ 
PLAYER_ERR_CODE HCIAPI hci_tts_player_pause();

/**  
 * @brief	恢复播放器播放
 * @return	
 * @n
 *	<table>
 *		<tr><td>@ref PLAYER_ERR_NONE</td><td>操作成功</td></tr>
 *		<tr><td>@ref PLAYER_ERR_NOT_INIT</td><td>没有初始化</td></tr>
 *		<tr><td>@ref PLAYER_ERR_NOT_BEGIN</td><td>没有开始播放</td></tr>
 *	</table>
 */ 
PLAYER_ERR_CODE HCIAPI hci_tts_player_resume();

/**  
 * @brief	停止播放器
 * @return	
 * @n
 *	<table>
 *		<tr><td>@ref PLAYER_ERR_NONE</td><td>操作成功</td></tr>
 *		<tr><td>@ref PLAYER_ERR_NOT_INIT</td><td>没有初始化</td></tr>
 *		<tr><td>@ref PLAYER_ERR_NOT_BEGIN</td><td>没有开始播放</td></tr>
 *	</table>
 */ 
PLAYER_ERR_CODE HCIAPI hci_tts_player_stop();

/**  
 * @brief	播放器SDK反初始化
 * @return	
 * @n
 *	<table>
 *		<tr><td>@ref PLAYER_ERR_NONE</td><td>操作成功</td></tr>
 *		<tr><td>@ref PLAYER_ERR_NOT_INIT</td><td>没有初始化</td></tr>
 *	</table>
 */ 
PLAYER_ERR_CODE HCIAPI hci_tts_player_release();

/* @} */
/* @} */
//////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
}
#endif
//////////////////////////////////////////////////////////////////////////
#endif // _hci_cloud_tts_player_api_header_
