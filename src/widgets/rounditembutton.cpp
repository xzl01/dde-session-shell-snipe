
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

#include <QtCore/QObject>
#include <QSvgRenderer>
#include "rounditembutton.h"

#include <DFontSizeManager>

DWIDGET_USE_NAMESPACE

RoundItemButton::RoundItemButton(QWidget *parent)
    : RoundItemButton("", parent)
{

}

RoundItemButton::RoundItemButton(const QString &text, QWidget *parent)
    : QAbstractButton(parent),
      m_itemIcon(new QLabel(this)),
      m_itemText(new QLabel(this))
{
    TRACE_ME_IN;	//<<==--TracePoint!
    m_itemText->setText(text);
    DFontSizeManager::instance()->bind(m_itemText, DFontSizeManager::T6);
    m_opacityEffect = new QGraphicsOpacityEffect(this);
    m_opacityEffect->setOpacity(1.0);
    setGraphicsEffect(m_opacityEffect);

    initUI();
    initConnect();
    TRACE_ME_OUT;	//<<==--TracePoint!

}

RoundItemButton::~RoundItemButton()
{
}

void RoundItemButton::setDisabled(bool disabled)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    if (!disabled)
        updateState(Normal);
    else
        updateState(Disabled);

    QAbstractButton::setDisabled(disabled);

    // update qss
    setStyleSheet(styleSheet());

    // update opacity
    m_opacityEffect->setOpacity(disabled ? 0.5 : 1.0);
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void RoundItemButton::setChecked(bool checked)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    if (checked)
        updateState(Checked);
    else
        updateState(Normal);
        TRACE_ME_OUT;	//<<==--TracePoint!

}

void RoundItemButton::initConnect()
{
    TRACE_ME_IN;	//<<==--TracePoint!
    connect(this, &RoundItemButton::stateChanged, this, &RoundItemButton::setState, Qt::DirectConnection);
    connect(this, &RoundItemButton::stateChanged, this, &RoundItemButton::updateIcon);
    connect(this, &RoundItemButton::stateChanged, this, static_cast<void (RoundItemButton::*)()>(&RoundItemButton::update));
    connect(this, &RoundItemButton::iconChanged, this, &RoundItemButton::updateIcon);
    connect(this, &RoundItemButton::toggled, this, &RoundItemButton::setChecked);
    TRACE_ME_OUT;	//<<==--TracePoint!

//    connect(signalManager, &SignalManager::setButtonHover, [this] (const QString &text) {
//        if (m_itemText->text() != text && !isChecked() && !isDisabled()) {
//            updateState(Normal);
//        }
//    });
}

void RoundItemButton::initUI() {
    TRACE_ME_IN;	//<<==--TracePoint!
    m_itemIcon->setFocusPolicy(Qt::NoFocus);
    m_itemIcon->setFixedSize(75, 75);
    m_itemIcon->installEventFilter(this);

    m_itemText->setWordWrap(true);
    m_itemText->setForegroundRole(QPalette::WindowText);
    QPalette palette = m_itemText->palette();
    palette.setColor(QPalette::WindowText, Qt::white);
    m_itemText->setPalette(palette);
    m_itemText->setAlignment(Qt::AlignCenter | Qt::AlignTop);
    m_itemText->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_itemText->setContentsMargins(10, 5, 10, 5);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setMargin(0);
    mainLayout->addSpacing(10);
    mainLayout->addWidget(m_itemIcon);
    mainLayout->setAlignment(m_itemIcon, Qt::AlignHCenter);
    mainLayout->addWidget(m_itemText, 0, Qt::AlignCenter);

    setFocusPolicy(Qt::NoFocus);
    setLayout(mainLayout);
    setFixedSize(QSize(140, 140));
    setCheckable(true);

    QGraphicsDropShadowEffect *nameShadow = new QGraphicsDropShadowEffect(m_itemText);
    nameShadow->setBlurRadius(16);
    nameShadow->setColor(QColor(0, 0, 0, 85));
    nameShadow->setOffset(0, 4);
    TRACE_ME_OUT;	//<<==--TracePoint!

//    m_itemText->setGraphicsEffect(nameShadow);
}

void RoundItemButton::enterEvent(QEvent* event)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    Q_UNUSED(event)

    if (m_state == Disabled) {
        TRACE_ME_OUT;	//<<==--TracePoint!
        return;}

    if (m_state == Normal)
        updateState(Hover);
        TRACE_ME_OUT;	//<<==--TracePoint!


//    emit signalManager->setButtonHover(m_itemText->text());
}

void RoundItemButton::leaveEvent(QEvent* event)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    Q_UNUSED(event)

    if (m_state == Disabled) {
        TRACE_ME_OUT;	//<<==--TracePoint!
        return;}

    if (m_state == Checked) {
        TRACE_ME_OUT;	//<<==--TracePoint!
        return;}

    updateState(Normal);
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void RoundItemButton::mousePressEvent(QMouseEvent* event)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    Q_UNUSED(event);

    updateState(Pressed);
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void RoundItemButton::mouseReleaseEvent(QMouseEvent* e)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    Q_UNUSED(e);

    if (m_state == Checked)
        updateState(Hover);
    else
        updateState(Pressed);

    if (m_state != Disabled)
        emit clicked();
        TRACE_ME_OUT;	//<<==--TracePoint!

}

bool RoundItemButton::eventFilter(QObject *watched, QEvent *event)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    if (watched == m_itemIcon && event->type() == QEvent::Paint) {
        QSvgRenderer renderer(m_currentIcon, m_itemIcon);
        QPainter painter(m_itemIcon);
        renderer.render(&painter, m_itemIcon->rect());
    }

    TRACE_ME_OUT;	//<<==--TracePoint!
    return false;
}

void RoundItemButton::paintEvent(QPaintEvent* event)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    QWidget::paintEvent(event);
    QPainter painter(this);

    if (m_state == Checked) {
        QPen pen;
        QColor penColor(151, 151, 151, 127);
        pen.setColor(penColor);
        pen.setWidth(m_penWidth);
        painter.setPen(pen);
        painter.setRenderHint(QPainter::Antialiasing, true);

        //只绘制轮廓，无中间填充色
        //底下的矩形框轮廓是外描绘，所以需要减去画笔的宽度绘制轮廓
        QRect itemTextRect = m_itemText->geometry().marginsRemoved(QMargins(m_penWidth / 2, m_penWidth / 2, m_penWidth / 2, m_penWidth / 2));
        painter.drawRoundedRect(itemTextRect, m_rectRadius, m_rectRadius);
        painter.drawEllipse(m_itemIcon->geometry());

        //填充中间背景
        painter.setBrush(QColor(0, 15, 39, 178));
        painter.setPen(Qt::NoPen);
        //drawEllipse是内描绘，中间填充色需要减去画笔宽度/2
        QRect m_itemIconRtct(m_itemIcon->geometry().marginsRemoved((QMargins(m_penWidth / 2, m_penWidth / 2, m_penWidth / 2, m_penWidth / 2))));
        painter.drawEllipse(m_itemIconRtct);

        //drawRoundedRect外描绘，填充色区域需要再减掉画笔宽度
        QRect textBackgroundRect(m_itemText->geometry().marginsRemoved(QMargins(m_penWidth, m_penWidth, m_penWidth, m_penWidth)));
        painter.drawRoundedRect(textBackgroundRect, m_rectRadius, m_rectRadius);

    } else if (m_state == Hover) {
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(255, 255, 255, 127));
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.drawRoundedRect(m_itemText->geometry(), m_rectRadius, m_rectRadius);
        painter.drawEllipse(m_itemIcon->geometry());
    }
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void RoundItemButton::updateIcon()
{
    TRACE_ME_IN;	//<<==--TracePoint!
    switch (m_state)
    {
    case Disabled:  /* show normal pic */
    case Normal:    m_currentIcon = m_normalIcon;  break;
    case Default:
    case Hover:     m_currentIcon = m_hoverIcon;   break;
    case Checked:   m_currentIcon = m_normalIcon;  break;
    case Pressed:   m_currentIcon = m_pressedIcon; break;
    default:;
    }

    m_itemIcon->update();
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void RoundItemButton::updateState(const RoundItemButton::State state)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    if (m_state != state) {
        m_state = state;
        emit stateChanged(state);
    }

    //Hover状态下，字体颜色设置为黑色
    if (state == Hover) {
        QPalette palette = m_itemText->palette();
        palette.setColor(QPalette::WindowText, Qt::black);
        m_itemText->setPalette(palette);
    } else {
        QPalette palette = m_itemText->palette();
        palette.setColor(QPalette::WindowText, Qt::white);
        m_itemText->setPalette(palette);
    }

    QAbstractButton::setChecked(m_state == Checked);
    TRACE_ME_OUT;	//<<==--TracePoint!
    return updateIcon();
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void RoundItemButton::setNormalPic(const QString &path)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    m_normalIcon = path;

    updateIcon();
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void RoundItemButton::setHoverPic(const QString &path)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    m_hoverIcon = path;

    updateIcon();
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void RoundItemButton::setPressPic(const QString &path)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    m_pressedIcon = path;

    updateIcon();
    TRACE_ME_OUT;	//<<==--TracePoint!

}
