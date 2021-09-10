
#include <sys/time.h>
#define TRACE_ME_IN struct timeval tp ; gettimeofday ( &tp , nullptr ); printf("[%4ld.%4ld] In: %s\n",tp.tv_sec , tp.tv_usec,__PRETTY_FUNCTION__);
#define TRACE_ME_OUT gettimeofday (const_cast<timeval *>(&tp) , nullptr ); printf("[%4ld.%4ld] Out: %s\n",tp.tv_sec , tp.tv_usec,__PRETTY_FUNCTION__);

#include "framedatabind.h"

FrameDataBind::FrameDataBind()
    : QObject()
{

}

FrameDataBind::~FrameDataBind()
{

}

FrameDataBind *FrameDataBind::Instance()
{
    TRACE_ME_IN;	//<<==--TracePoint!
    static FrameDataBind frameDataBind;
    TRACE_ME_OUT;	//<<==--TracePoint!
    return &frameDataBind;
}

int FrameDataBind::registerFunction(const QString &flag, std::function<void (QVariant)> function)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    for (auto it = m_registerList.begin(); it != m_registerList.end(); ++it) {
        if (it.key() == flag) {
            QList<int> keys = it.value().keys();
            int index = 0;
            while (true) {
                if (!keys.contains(index)) {
                    it.value()[index] = function;
                    TRACE_ME_OUT;	//<<==--TracePoint!
                    return index;
                }

                ++index;
            }
        }
    }

    m_registerList[flag][0] = function;
    TRACE_ME_OUT;	//<<==--TracePoint!
    return 0;
}

void FrameDataBind::unRegisterFunction(const QString &flag, int index)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    for (auto it = m_registerList.begin(); it != m_registerList.end(); ++it) {
        if (it.key() == flag) {
            it.value().remove(index);
        }
    }
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void FrameDataBind::updateValue(const QString &flag, const QVariant &value)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    m_datas[flag] = value;

    refreshData(flag);
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void FrameDataBind::refreshData(const QString &flag)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    QList<std::function<void (QVariant)>> functions = m_registerList[flag].values();

    for (auto it = functions.constBegin(); it != functions.constEnd(); ++it) {
        (*it)(m_datas[flag]);
    }
    TRACE_ME_OUT;	//<<==--TracePoint!

}
