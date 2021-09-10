
#include <sys/time.h>
#define TRACE_ME_IN struct timeval tp ; gettimeofday ( &tp , nullptr ); printf("[%4ld.%4ld] In: %s\n",tp.tv_sec , tp.tv_usec,__PRETTY_FUNCTION__);
#define TRACE_ME_OUT gettimeofday ( &tp , nullptr ); printf("[%4ld.%4ld] Out: %s\n",tp.tv_sec , tp.tv_usec,__PRETTY_FUNCTION__);

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

#include <QtGui/QPainter>
#include <QDebug>
#include "imagebutton.h"

ImageButton::ImageButton(QString url, QString name, QWidget *parent)
    : QPushButton(parent)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    m_url = url;
    m_name = name;
    this->setObjectName(name);
    setFixedSize(120, 120);
    TRACE_ME_OUT;	//<<==--TracePoint!


   // connect(this, SIGNAL(clicked()), this, SLOT(sendClicked()));
}
ImageButton::~ImageButton()
{
}
void ImageButton::sendClicked() {
    TRACE_ME_IN;	//<<==--TracePoint!
    emit clicked(m_name);
    TRACE_ME_OUT;	//<<==--TracePoint!

}
void ImageButton::hideIn(QString name) {
    TRACE_ME_IN;	//<<==--TracePoint!
    if (name!=this->objectName()) {
        this->hide();
    }
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void ImageButton::showOut() {
    TRACE_ME_IN;	//<<==--TracePoint!
    this->show();
    TRACE_ME_OUT;	//<<==--TracePoint!

}
void ImageButton::setIconSize(const AvatarSize &avatarsize) {
    TRACE_ME_IN;	//<<==--TracePoint!
    int tmpAvatarSize = SMALL_ICON_SIZE;

    if (avatarsize==AvatarBigSize) {
        tmpAvatarSize = LARGE_ICON_SIZE;
    } else {
        tmpAvatarSize = SMALL_ICON_SIZE;
    }

    m_avatarsize = avatarsize;
    this->setFixedSize(tmpAvatarSize, tmpAvatarSize);
    TRACE_ME_OUT;	//<<==--TracePoint!

}
void ImageButton::paintEvent(QPaintEvent *) {
    TRACE_ME_IN;	//<<==--TracePoint!
    int iconSize = SMALL_ICON_SIZE;
    if (m_avatarsize == AvatarBigSize) {
        iconSize = LARGE_ICON_SIZE;
    }

    QPainter painter(this);
    QRect ellipseRec((width() -iconSize) / 2.0, (height() - iconSize) / 2.0, iconSize, iconSize);
    QPainterPath path;
    path.addEllipse(ellipseRec);
    painter.setRenderHints(QPainter::Antialiasing
                           | QPainter::SmoothPixmapTransform);

    painter.setClipPath(path);

    QPixmap pixmap(m_url);
    painter.drawPixmap(ellipseRec, pixmap);
    QColor penColor =  "yellow";

    QPen pen;
    pen.setColor(penColor);
    pen.setWidth(m_borderWidth);
    painter.setPen(pen);
    painter.drawPath(path);
    painter.end();
    TRACE_ME_OUT;	//<<==--TracePoint!

}
