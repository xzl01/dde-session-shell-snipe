
#include <sys/time.h>
#define TRACE_ME_IN struct timeval tp ; gettimeofday ( &tp , nullptr ); printf("[%4ld.%4ld] In: %s\n",tp.tv_sec , tp.tv_usec,__PRETTY_FUNCTION__);
#define TRACE_ME_OUT gettimeofday (const_cast<timeval *>(&tp) , nullptr ); printf("[%4ld.%4ld] Out: %s\n",tp.tv_sec , tp.tv_usec,__PRETTY_FUNCTION__);

#include "lockpasswordwidget.h"

#include <QPainter>
#include <QHBoxLayout>
#include <DHiDPIHelper>

DWIDGET_USE_NAMESPACE

LockPasswordWidget::LockPasswordWidget(QWidget *parent) : QWidget(parent)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    setAttribute(Qt::WA_TranslucentBackground);

    QHBoxLayout *layout = new QHBoxLayout;
    layout->setMargin(0);
    layout->setSpacing(0);
    lockLbl = new QLabel(this);
    lockLbl->setPixmap(DHiDPIHelper::loadNxPixmap(":/img/action_icons/unlock_normal.svg"));
    layout->addSpacing(5);
    layout->addWidget(lockLbl);
    layout->addStretch();
    setLayout(layout);
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void LockPasswordWidget::setMessage(const QString &message)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    m_message = message;

    update();
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void LockPasswordWidget::paintEvent(QPaintEvent *event)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    QWidget::paintEvent(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::HighQualityAntialiasing);

    QFont font;
    font.setWordSpacing(0);
    painter.setFont(font);
    painter.setBrush(Qt::white);
    painter.setPen(Qt::NoPen);
    painter.setOpacity(0.1);
    painter.drawRoundedRect(rect(), 5, 5);

    QTextOption option;
    option.setAlignment(Qt::AlignCenter);
    painter.setOpacity(1);
    painter.setPen(Qt::white);
    painter.drawText(rect(), m_message, option);
    TRACE_ME_OUT;	//<<==--TracePoint!

}
