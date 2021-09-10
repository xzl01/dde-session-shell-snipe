
#include <sys/time.h>
#define TRACE_ME_IN struct timeval tp ; gettimeofday ( &tp , nullptr ); printf("[%4ld.%4ld] In: %s\n",tp.tv_sec , tp.tv_usec,__PRETTY_FUNCTION__);
#define TRACE_ME_OUT gettimeofday (const_cast<timeval *>(&tp) , nullptr ); printf("[%4ld.%4ld] Out: %s\n",tp.tv_sec , tp.tv_usec,__PRETTY_FUNCTION__);

/*
 * Copyright (C) 2011 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     sbw <sbw@sbw.so>
 *             kirigaya <kirigaya@mkacg.com>
 *             Hualet <mr.asianwang@gmail.com>
 *             zorowk <near.kingzero@gmail.com>
 *
 * Maintainer: sbw <sbw@sbw.so>
 *             kirigaya <kirigaya@mkacg.com>
 *             Hualet <mr.asianwang@gmail.com>
 *             zorowk <near.kingzero@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "keyboardmonitor.h"
#include <DGuiApplicationHelper>

DGUI_USE_NAMESPACE

KeyboardMonitor::KeyboardMonitor() : QThread()
{
//    if(DGuiApplicationHelper::isXWindowPlatform()) {
//        keyBoardPlatform = new KeyboardPlantformX11();
//    } else {
//        keyBoardPlatform = new KeyboardPlantformWayland();
//    }
    TRACE_ME_IN;	//<<==--TracePoint!
    keyBoardPlatform = new KeyboardPlantformX11();

    connect(keyBoardPlatform, &KeyBoardPlatform::capslockStatusChanged, this, &KeyboardMonitor::capslockStatusChanged);
    connect(keyBoardPlatform, &KeyBoardPlatform::numlockStatusChanged, this, &KeyboardMonitor::numlockStatusChanged);
    TRACE_ME_OUT;	//<<==--TracePoint!

}

KeyboardMonitor *KeyboardMonitor::instance()
{
    TRACE_ME_IN;	//<<==--TracePoint!
    static KeyboardMonitor *KeyboardMonitorInstance = nullptr;

    if (!KeyboardMonitorInstance) {
        KeyboardMonitorInstance = new KeyboardMonitor;
    }

    TRACE_ME_OUT;	//<<==--TracePoint!
    return KeyboardMonitorInstance;
}

bool KeyboardMonitor::isCapslockOn()
{
    TRACE_ME_IN;	//<<==--TracePoint!
    TRACE_ME_OUT;	//<<==--TracePoint!
    return keyBoardPlatform->isCapslockOn();
}

bool KeyboardMonitor::isNumlockOn()
{
    TRACE_ME_IN;	//<<==--TracePoint!
    TRACE_ME_OUT;	//<<==--TracePoint!
    return keyBoardPlatform->isNumlockOn();
}

bool KeyboardMonitor::setNumlockStatus(const bool &on)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    TRACE_ME_OUT;	//<<==--TracePoint!
    return keyBoardPlatform->setNumlockStatus(on);
}

void KeyboardMonitor::run()
{
    TRACE_ME_IN;	//<<==--TracePoint!
    keyBoardPlatform->run();
    TRACE_ME_OUT;	//<<==--TracePoint!

}
