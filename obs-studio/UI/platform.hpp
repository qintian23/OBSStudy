/******************************************************************************
    Copyright (C) 2013 by Hugh Bailey <obs.jim@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
******************************************************************************/

#pragma once

#include <util/c99defs.h>

#include <string>
#include <vector>

class QWidget;

/* Gets the path of obs-studio specific data files (such as locale) */
bool GetDataFilePath(const char *data, std::string &path);

std::string GetDefaultVideoSavePath();
/// <summary>
/// 获取电脑的区域语言并返回可支持的语言
/// </summary>
/// <returns></returns>
std::vector<std::string> GetPreferredLocales();

bool IsAlwaysOnTop(QWidget *window);
void SetAlwaysOnTop(QWidget *window, bool enable);

bool SetDisplayAffinitySupported(void);

#ifdef _WIN32
class RunOnceMutex;
/// <summary>
/// 检测程序是否已经运行
/// </summary>
/// <param name="already_running"></param>
RunOnceMutex
#else
void
#endif
CheckIfAlreadyRunning(bool &already_running);

#ifdef _WIN32
uint32_t GetWindowsVersion();
uint32_t GetWindowsBuild();
void SetAeroEnabled(bool enable);
void SetProcessPriority(const char *priority);
void SetWin32DropStyle(QWidget *window);
/// <summary>
/// 是否启用音频流衰减体验
/// </summary>
/// <param name="disable"></param>
/// <returns></returns>
bool DisableAudioDucking(bool disable);

struct RunOnceMutexData;

class RunOnceMutex {
	RunOnceMutexData *data = nullptr;

public:
	RunOnceMutex(RunOnceMutexData *data_) : data(data_) {}
	RunOnceMutex(const RunOnceMutex &rom) = delete;
	RunOnceMutex(RunOnceMutex &&rom);
	~RunOnceMutex();

	RunOnceMutex &operator=(const RunOnceMutex &rom) = delete;
	RunOnceMutex &operator=(RunOnceMutex &&rom);
};

QString GetMonitorName(const QString &id);
/// <summary>
/// 是否运行在Wine操作系统上
/// </summary>
/// <returns></returns>
bool IsRunningOnWine();
#endif

#ifdef __APPLE__
void EnableOSXVSync(bool enable);
void EnableOSXDockIcon(bool enable);
bool isInBundle();
void InstallNSApplicationSubclass();
void disableColorSpaceConversion(QWidget *window);
bool ProcessIsRosettaTranslated();
#endif
