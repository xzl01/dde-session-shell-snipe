
#include <sys/time.h>
#define TRACE_ME_IN struct timeval tp ; gettimeofday ( &tp , nullptr ); printf("[%4ld.%4ld] In: %s\n",tp.tv_sec , tp.tv_usec,__PRETTY_FUNCTION__);
#define TRACE_ME_OUT gettimeofday (const_cast<timeval *>(&tp) , nullptr ); printf("[%4ld.%4ld] Out: %s\n",tp.tv_sec , tp.tv_usec,__PRETTY_FUNCTION__);

/*
 * Copyright (C) 2015 ~ 2018 Deepin Technology Co., Ltd.
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

#include "dbusvariant.h"

Inhibit::Inhibit() {}
Inhibit::~Inhibit() {}

void Inhibit::registerMetaType()
{
    TRACE_ME_IN;	//<<==--TracePoint!
    qRegisterMetaType<Inhibit>("Inhibit");
    qDBusRegisterMetaType<Inhibit>();
    qRegisterMetaType<InhibitorsList>("InhibitorsList");
    qDBusRegisterMetaType<InhibitorsList>();
    TRACE_ME_OUT;	//<<==--TracePoint!

}


QDBusArgument &operator<<(QDBusArgument &argument, const Inhibit &obj)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    argument.beginStructure();
    argument << obj.what << obj.who << obj.why << obj.mode<< obj.uid << obj.pid;
    argument.endStructure();
    TRACE_ME_OUT;	//<<==--TracePoint!
    return argument;
}


const QDBusArgument &operator>>(const QDBusArgument &argument, Inhibit &obj)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    argument.beginStructure();
    argument >> obj.what >> obj.who >> obj.why >> obj.mode >> obj.uid >> obj.pid;
    argument.endStructure();
    TRACE_ME_OUT;	//<<==--TracePoint!
    return argument;
}

UserInfo::UserInfo() {}
UserInfo::~UserInfo() {}

void UserInfo::registerMetaType()
{
    TRACE_ME_IN;	//<<==--TracePoint!
    qRegisterMetaType<UserInfo>("UserInfo");
    qDBusRegisterMetaType<UserInfo>();
    qRegisterMetaType<UserList>("UserList");
    qDBusRegisterMetaType<UserList>();
    TRACE_ME_OUT;	//<<==--TracePoint!

}

QDBusArgument &operator<<(QDBusArgument &argument, const UserInfo &obj)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    argument.beginStructure();
    argument << obj.pid << obj.id << obj.userName;
    argument.endStructure();
    TRACE_ME_OUT;	//<<==--TracePoint!
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, UserInfo &obj)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    argument.beginStructure();
    argument >> obj.pid >> obj.id >> obj.userName;
    argument.endStructure();
    TRACE_ME_OUT;	//<<==--TracePoint!
    return argument;
}

SeatInfo::SeatInfo() {}
SeatInfo::~SeatInfo() {}

void SeatInfo::registerMetaType() {
    TRACE_ME_IN;	//<<==--TracePoint!
    qRegisterMetaType<SeatInfo>("SeatInfo");
    qDBusRegisterMetaType<SeatInfo>();
    qRegisterMetaType<SeatList>("SeatList");
    qDBusRegisterMetaType<SeatList>();
    TRACE_ME_OUT;	//<<==--TracePoint!

}

QDBusArgument &operator<<(QDBusArgument &argument, const SeatInfo &obj)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    argument.beginStructure();
    argument << obj.id << obj.seat;
    argument.endStructure();
    TRACE_ME_OUT;	//<<==--TracePoint!
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, SeatInfo &obj)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    argument.beginStructure();
    argument >> obj.id >> obj.seat;
    argument.endStructure();
    TRACE_ME_OUT;	//<<==--TracePoint!
    return argument;
}

SessionInfo::SessionInfo() {}
SessionInfo::~SessionInfo() {}

void SessionInfo::registerMetaType() {
    TRACE_ME_IN;	//<<==--TracePoint!
    qRegisterMetaType<SessionInfo>("SessionInfo");
    qDBusRegisterMetaType<SessionInfo>();
    qRegisterMetaType<SessionList>("SessionList");
    qDBusRegisterMetaType<SessionList>();
    TRACE_ME_OUT;	//<<==--TracePoint!

}

QDBusArgument &operator<<(QDBusArgument &argument, const SessionInfo &obj)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    argument.beginStructure();
    argument << obj.session << obj.pid << obj.user << obj.id << obj.seat;
    argument.endStructure();
    TRACE_ME_OUT;	//<<==--TracePoint!
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, SessionInfo &obj)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    argument.beginStructure();
    argument >> obj.session >> obj.pid >> obj.user >> obj.id >> obj.seat;
    argument.endStructure();
    TRACE_ME_OUT;	//<<==--TracePoint!
    return argument;
}
