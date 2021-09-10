
#include <sys/time.h>
#define TRACE_ME_IN struct timeval tp ; gettimeofday ( &tp , nullptr ); printf("[%4ld.%4ld] In: %s\n",tp.tv_sec , tp.tv_usec,__PRETTY_FUNCTION__);
#define TRACE_ME_OUT gettimeofday (const_cast<timeval *>(&tp) , nullptr ); printf("[%4ld.%4ld] Out: %s\n",tp.tv_sec , tp.tv_usec,__PRETTY_FUNCTION__);

/*
 * Copyright (C) 2011 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     sbw <sbw@sbw.so>
 *             kirigaya <kirigaya@mkacg.com>
 *             Hualet <mr.asianwang@gmail.com>
 *
 * Maintainer: sbw <sbw@sbw.so>
 *             kirigaya <kirigaya@mkacg.com>
 *             Hualet <mr.asianwang@gmail.com>
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

#include "dbuslockfrontservice.h"

/*
 * Implementation of interface class DBusLockFront
 */

DBusLockFrontService::DBusLockFrontService(DBusLockAgent *parent)
    : QDBusAbstractAdaptor(parent)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    setAutoRelaySignals(true);
    TRACE_ME_OUT;	//<<==--TracePoint!

}

DBusLockFrontService::~DBusLockFrontService()
{
}

void DBusLockFrontService::Show()
{
    TRACE_ME_IN;	//<<==--TracePoint!
    parent()->Show();
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void DBusLockFrontService::ShowUserList()
{
    TRACE_ME_IN;	//<<==--TracePoint!
    parent()->ShowUserList();
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void DBusLockFrontService::ShowAuth(bool active)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    parent()->ShowAuth(active);
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void DBusLockFrontService::Suspend(bool enable)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    parent()->Suspend(enable);
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void DBusLockFrontService::Hibernate(bool enable)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    parent()->Hibernate(enable);
    TRACE_ME_OUT;	//<<==--TracePoint!

}
