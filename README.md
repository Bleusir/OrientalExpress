【注意事项】
-----------------------------------------------------------------------------------------------------------------------------------------------------
本项目已经迁移到至新仓库https://github.com/SSEDev/OrientalExpress。
获取东方快车项目的最新进展以及上海证券交易所更多开源项目，请访问https://github.com/SSEDev/

【项目简介】
-----------------------------------------------------------------------------------------------------------------------------------------------------
Oriental Express项目是由上海证券交易所技术人员主导的一个开源项目，提供接入所内各平台高速行情的客户端API。
本项目的初衷是提供封装所内高速行情发布底层通讯协议的开发包（C和JAVA两个版本），以帮助行情用户更快更好地开发客户端程序，更好地向市场提供行情服务。

【运作模式】
-----------------------------------------------------------------------------------------------------------------------------------------------------
本项目开源免费，由上海证券交易所和相关券商的技术团队负责维护，源代码将公布在Github代码托管网站上。
同时欢迎感兴趣的技术开发人员加入我们的团队，或提供技术支持。
需要使用本项目代码进行商业项目开发的单位或个人，需要接受上海证券交易所的授权。

【编译方法】
-----------------------------------------------------------------------------------------------------------------------------------------------------
本工程支持源代码方式编译或者直接通过lib下的库文件链接

1、根据编译机器的操作系统确定OS_TYPE=[Windows/Linux/Unix]，默认Linux；
2、make premake的使用，首次编译或者makeclean之后需要先做预处理；
3、选择编译类型 BUILD_TYPE=[Debug/Release]，默认Debug；
4、示例
   Windows下编译Debug版本
   make OS_TYPE=Windows premake [首次编译或者完成清理]
   make OS_TYPE=Windows BUILD_TYPE=Debug
   make OS_TYPE=Windows clean [清理时才会调用]

   Linux下编译Debug版本
   make OS_TYPE=Linux premake [首次编译或者完成清理]
   make OS_TYPE=Linux BUILD_TYPE=Debug
   make OS_TYPE=Linux clean [清理时才会调用]

   Unix下编译Debug版本
   make OS_TYPE=Unix premake [首次编译或者完成清理]
   make OS_TYPE=Unix BUILD_TYPE=Debug
   make OS_TYPE=Unix clean [清理时才会调用]

【接口使用说明】
-----------------------------------------------------------------------------------------------------------------------------------------------------
具体使用方法请参见接口头文件注释 src/eps/epsClient.h
客户端演示用例请参见 src/test/simpleEpsClient.c 和 src/test/complexEpsClient.c

simpleEpsClient是同步调用模式示例，仅适用于UDP，调用过程如下：
--> 初始化过程
1、调用EpsInitLib()初始化Express库
2、调用EpsCreateHandle()创建新句柄，
3、调用EpsRegisterSpi()注册回调通知函数
4、调用EpsConnect()启动连接服务器
5、调用EpsSubscribeMarketData()订阅指定市场行情

--> 回调过程
1、在注册的行情数据到达通知函数mktDataArrivedNotify中处理行情数据

--> 注销过程
1、调用EpsDisconnect()断开服务器连接
2、调用EpsDestroyHandle()销毁句柄
3、调用EpsUninitLib()注销Express库

complexEpsClient是一种异步调用模式，同时适用于TCP和UDP，调用过程如下：
--> 初始化过程
1、调用EpsInitLib()初始化Express库
2、调用EpsCreateHandle()创建新句柄，
3、调用EpsRegisterSpi()注册回调通知函数
4、调用EpsConnect()启动连接服务器

--> 回调过程
1、在注册的连接成功通知connectedNotify中调用EpsLogin()进行登录
2、在注册的登录应答通知loginRspNotify中调用EpsSubscribeMarketData()订阅指定市场行情
3、在注册的行情数据到达通知函数mktDataArrivedNotify中处理行情数据
4、在注册的连接断开通知disconnectedNotify、登出应答通知logoutRspNotify、事件发生通知eventOccurred中进行事件处理（可选）

--> 注销过程
1、调用EpsLogout()登出服务器（可选）
2、调用EpsDisconnect()断开服务器连接
3、调用EpsDestroyHandle()销毁句柄
4、调用EpsUninitLib()注销Express库

注意事项：
1、EpsInitLib()必须是第一个被调用的Express库函数
2、EpsUninitLib()必须是最后一个被调用的Express库函数
3、Express库支持最多同时创建32个句柄
4、Express库支持多线程调用
4、Express允许用户在注册回调通知时不实现所有的回调接口，原则是不注册即不通知
5、EpsInitLib()、EpsUninitLib()、EpsDestroyHandle()禁止在回调通知函数中调用

【授权使用】
-----------------------------------------------------------------------------------------------------------------------------------------------------
请填写《东方快车项目授权申请表.docx》后传真或扫描件发送到zhengwu@sse.com.cn，
我们授权的目的是了解下用户的使用情况，供我们参考，对用户的使用方法没有其他限制。

【联系方式】
-----------------------------------------------------------------------------------------------------------------------------------------------------
上海证券交易所 		吴征			zhengwu@sse.com.cn			
平安证券			金成勋			kimsh@163.com
