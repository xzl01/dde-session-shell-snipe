
#include <sys/time.h>
#define TRACE_ME_IN struct timeval tp ; gettimeofday ( &tp , nullptr ); printf("[%4ld.%4ld] In: %s\n",tp.tv_sec , tp.tv_usec,__PRETTY_FUNCTION__);
#define TRACE_ME_OUT gettimeofday (const_cast<timeval *>(&tp) , nullptr ); printf("[%4ld.%4ld] Out: %s\n",tp.tv_sec , tp.tv_usec,__PRETTY_FUNCTION__);

#include "loginbutton.h"

#include <QHBoxLayout>
#include <QLabel>
#include <DHiDPIHelper>
#include <QPainter>
#include <QKeyEvent>

DWIDGET_USE_NAMESPACE

LoginButton::LoginButton(QWidget *parent)
    : QWidget(parent)
    , m_text(new QLabel)
    , m_icon(new QLabel)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    m_text->setAlignment(Qt::AlignCenter);

    setObjectName("LoginButton");

    QHBoxLayout *layout = new QHBoxLayout;
    layout->setMargin(0);
    layout->setSpacing(0);

    layout->addStretch();
    layout->addWidget(m_text, 0, Qt::AlignVCenter);
    layout->addSpacing(20);
    layout->addWidget(m_icon);
    layout->addStretch();

    m_icon->setFixedSize(18, 12);

    setLayout(layout);
    setIcon(":/img/nopassword_login_normal.svg");
    m_text->setStyleSheet("font-size: 12px; color: white;");
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void LoginButton::setText(const QString &text)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    m_text->setText(text);
    m_text->adjustSize();
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void LoginButton::setIcon(const QString &icon)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    m_iconPath = icon;
    m_icon->setPixmap(DHiDPIHelper::loadNxPixmap(icon));
    emit iconChanged(icon);
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void LoginButton::enterEvent(QEvent *event)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    setIcon(":/img/nopassword_login_hover.svg");
    TRACE_ME_OUT;	//<<==--TracePoint!
    return QWidget::enterEvent(event);
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void LoginButton::leaveEvent(QEvent *event)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    setIcon(":/img/nopassword_login_normal.svg");
    TRACE_ME_OUT;	//<<==--TracePoint!
    return QWidget::leaveEvent(event);
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void LoginButton::mousePressEvent(QMouseEvent *event)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    setIcon(":/img/nopassword_login_press.svg");
    TRACE_ME_OUT;	//<<==--TracePoint!
    return QWidget::mousePressEvent(event);
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void LoginButton::mouseReleaseEvent(QMouseEvent *event)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    emit clicked();
    setIcon(":/img/nopassword_login_normal.svg");
    TRACE_ME_OUT;	//<<==--TracePoint!
    return QWidget::mouseReleaseEvent(event);
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void LoginButton::paintEvent(QPaintEvent *event)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    QWidget::paintEvent(event);

    QPainter painter(this);

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(255, 255, 255, 0.1 * 255));
    painter.drawRoundedRect(rect(), 5, 5);
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void LoginButton::keyPressEvent(QKeyEvent *event)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) {
        emit clicked();
    }

    TRACE_ME_OUT;	//<<==--TracePoint!
    return QWidget::keyPressEvent(event);
    TRACE_ME_OUT;	//<<==--TracePoint!

}
