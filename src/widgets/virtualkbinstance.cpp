
#include <sys/time.h>
#define TRACE_ME_IN struct timeval tp ; gettimeofday ( &tp , nullptr ); printf("[%4ld.%4ld] In: %s\n",tp.tv_sec , tp.tv_usec,__PRETTY_FUNCTION__);
#define TRACE_ME_OUT gettimeofday (const_cast<timeval *>(&tp) , nullptr ); printf("[%4ld.%4ld] Out: %s\n",tp.tv_sec , tp.tv_usec,__PRETTY_FUNCTION__);

#include "virtualkbinstance.h"

#include <QProcess>
#include <QWindow>
#include <QWidget>
#include <QTimer>
#include <QDebug>
#include <QResizeEvent>

VirtualKBInstance &VirtualKBInstance::Instance()
{
    TRACE_ME_IN;	//<<==--TracePoint!
    static VirtualKBInstance virtualKB;
    TRACE_ME_OUT;	//<<==--TracePoint!
    return virtualKB;
}

QWidget *VirtualKBInstance::virtualKBWidget() {
    TRACE_ME_IN;	//<<==--TracePoint!
    TRACE_ME_OUT;	//<<==--TracePoint!
    return m_virtualKBWidget;
}

VirtualKBInstance::~VirtualKBInstance()
{
    TRACE_ME_IN;	//<<==--TracePoint!
    stopVirtualKBProcess();
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void VirtualKBInstance::init()
{
    TRACE_ME_IN;	//<<==--TracePoint!
    if (m_virtualKBWidget) {
        emit initFinished();
        TRACE_ME_OUT;	//<<==--TracePoint!
        return;
    }

    //初始化启动onborad进程
    if (!m_virtualKBProcess) {
        m_virtualKBProcess = new QProcess(this);

        connect(m_virtualKBProcess, &QProcess::readyReadStandardOutput, [ = ]{
            //启动完成onborad进程后，获取onborad主界面，将主界面显示在锁屏界面上
            QByteArray output = m_virtualKBProcess->readAllStandardOutput();

            if (output.isEmpty())
{
    TRACE_ME_OUT;	//<<==--TracePoint!
    return;
}

            int xid = atoi(output.trimmed().toStdString().c_str());
            QWindow * w = QWindow::fromWinId(xid);
            m_virtualKBWidget = QWidget::createWindowContainer(w);
            m_virtualKBWidget->setFixedSize(600, 200);
            m_virtualKBWidget->hide();

            QTimer::singleShot(300, [=] {
                emit initFinished();
            });
        });
        connect(m_virtualKBProcess, SIGNAL(finished(int)), this, SLOT(onVirtualKBProcessFinished(int)));

        m_virtualKBProcess->start("onboard", QStringList() << "-e" << "--layout" << "Small" << "--size" << "60x5" << "-a");
    }
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void VirtualKBInstance::stopVirtualKBProcess()
{
    //结束onborad进程
    TRACE_ME_IN;	//<<==--TracePoint!
    if (m_virtualKBProcess) {
        m_virtualKBProcess->close();
        m_virtualKBWidget = nullptr;
    }
    TRACE_ME_OUT;	//<<==--TracePoint!

}

bool VirtualKBInstance::eventFilter(QObject *watched, QEvent *event)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    if (watched == m_virtualKBWidget && event->type() == QEvent::Resize) {
        QResizeEvent *e = static_cast<QResizeEvent*>(event);
        TRACE_ME_OUT;	//<<==--TracePoint!
        return e->size() != QSize(600, 200);
    }

    TRACE_ME_OUT;	//<<==--TracePoint!
    return QObject::eventFilter(watched, event);
}

VirtualKBInstance::VirtualKBInstance(QObject *parent)
    : QObject(parent)
    , m_virtualKBWidget(nullptr)
{
}

void VirtualKBInstance::onVirtualKBProcessFinished(int exitCode)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    Q_UNUSED(exitCode);
    m_virtualKBProcess = nullptr;
    TRACE_ME_OUT;	//<<==--TracePoint!

}
