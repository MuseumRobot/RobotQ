// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件

#pragma once

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string>
#include <QtGui/QApplication>
#include <QtGui/QDesktopWidget>
#include <QtGui/QMainWindow>
#include <QtGui/QMovie>
#include <QtTest/Qtest>
#include <QCheckBox>
#include <QDateTime>
#include <QDebug>
#include <QDialog>
#include <QFrame>
#include <QFileDialog>
#include <QMessageBox>
#include <QPainter>
#include <QPlainTextEdit>
#include <QString>
#include <QTime>
#include "targetver.h"
#include "common/GBK.h"
#include "common/AccountInfo.h"
#include "common/CommonTool.h"
#include "common/FileReader.h"
#include "include/hci_asr_recorder.h"
#include "include/hci_tts_player.h"
#include "include/Comm_data_motor3.h"
#include "include/Comm_data_star.h"
#include "include/UPURG.h"


#ifndef _SECURE_ATL
#define _SECURE_ATL 1
#endif

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // 从 Windows 头中排除极少使用的资料
#endif

#ifdef WIN64
#define CWinAppEx CWinApp
#endif

#ifdef _UNICODE
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif