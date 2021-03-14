#include "greeterscreenmanager.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QDebug>
#include <QDesktopWidget>
#include <QHash>
#include <QRect>

#include "src/session-widgets/sessionbasemodel.h"

GreeterScreenManager::GreeterScreenManager(QObject *parent)
    : QObject(parent)
    , m_registerFunction(nullptr)
    , m_raiseContentFrameTimer(new QTimer(this))
{
    connect(qApp, &QGuiApplication::screenAdded, this, &GreeterScreenManager::onScreenAdded);
    connect(qApp, &QGuiApplication::screenRemoved, this, &GreeterScreenManager::onScreenRemoved);

    // 在sw平台存在复制模式显示问题，使用延迟来置顶一个Frame
    m_raiseContentFrameTimer->setInterval(80);
    m_raiseContentFrameTimer->setSingleShot(true);

    connect(m_raiseContentFrameTimer, &QTimer::timeout, this, &GreeterScreenManager::raiseContentFrame);
}

void GreeterScreenManager::register_for_mutil_screen(std::function<QWidget *(QScreen *)> function)
{
    m_registerFunction = function;

    // update all screen
    for (QScreen *screen : qApp->screens()) {
        qDebug() << __FILE__ << __LINE__ << __func__ << ": screen info ---- " <<  screen << screen->geometry();
        if (qGuiApp->primaryScreen() == screen)
            continue;
        onScreenAdded(screen);
    }
    onScreenAdded(qGuiApp->primaryScreen());
}

void GreeterScreenManager::startRaiseContentFrame()
{
    m_raiseContentFrameTimer->start();
}

void GreeterScreenManager::onScreenAdded(QScreen *screen)
{
    qDebug() << __FILE__ << __LINE__ << __func__ << ": screen add ---- " <<  screen << screen->geometry();
    if (!m_registerFunction) {
        return;
    }

    if (m_frames.contains(screen)) {
        return;
    }

    m_frames[screen] = m_registerFunction(screen);
    startRaiseContentFrame();
}

inline uint qHash(const QRect& rect)
{
    QString s = QString("%1:%2:%3:%4").arg(rect.x()).arg(rect.y()).arg(rect.right()).arg(rect.bottom());
    return qHash(s);
}

inline bool screenGeometryValid(const QRect &rect)
{
    //防止坐标为4个0,此时QRect.isValid返回true
    return rect.width() > 1 && rect.height() > 1;
}

void GreeterScreenManager::raiseContentFrame()
{
    //统计screen的信息
    QHash<QRect, QScreen*> rect2Screen;
    QHash<QRect, int> rect2Count;
    QScreen *contentVisibleScreen = nullptr;
    for (auto it = m_frames.constBegin(); it != m_frames.constEnd(); ++it) {
        const auto& geometry = it.key()->geometry();
        rect2Screen.insertMulti(geometry, it.key());
        rect2Count[geometry]++; //这里key不存在时，会静默插入key且value为0
        if (it.value()->property("contentVisible").toBool()) {
            if (screenGeometryValid(geometry)) {
                contentVisibleScreen = it.key();
            }
        }
    }

    //挑选一个内容可见的screen，优先主屏幕
    //优先cursor所在的frame的逻辑更好，但是wl中取不到cursor位置
    while (contentVisibleScreen == nullptr) {
        const QRect& primaryRect = QGuiApplication::primaryScreen()->geometry();
        if (screenGeometryValid(primaryRect)) {
            contentVisibleScreen = QGuiApplication::primaryScreen();
            break;
        }
        for (auto it = m_frames.constBegin(); it != m_frames.constEnd(); ++it) {
            const auto& geometry = it.key()->geometry();
            if (screenGeometryValid(geometry)) {
                contentVisibleScreen = it.key();
                break;
            }
        }
        break;
    }

    qDebug() << "raiseContentFrame: contentVisibleScreen=" << contentVisibleScreen;
    if (contentVisibleScreen == nullptr) {
        qWarning() << "GreeterScreenManager::raiseContentFrame, contentVisibleScreen is null";
        return ;
    }

    //坐标不重复的显示，重复的只留一个显示，在所有显示的screen中必须有一个content可见
    for (auto it = m_frames.constBegin(); it != m_frames.constEnd(); ++it) {
        const auto& geometry = it.key()->geometry();
        if (!screenGeometryValid(geometry)) {
            it.value()->hide();
            qDebug() << "raiseContentFrame, hide1:" << geometry << it.key();
            continue;
        }

        if (rect2Count[geometry] == 1) {
            it.value()->show();
            continue;
        }

        //如果conttent visible命中这里，就留visible这个显示，否则就留第一个显示。
        if (contentVisibleScreen->geometry() == geometry) {
            if (it.key() == contentVisibleScreen) {
                it.value()->show();
            } else {
                it.value()->hide();
                qDebug() << "raiseContentFrame, hide2:" << geometry << it.key();
            }
        } else {
            if (it.key() == rect2Screen[geometry]) {
                it.value()->show();
            } else {
                it.value()->hide();
                qDebug() << "raiseContentFrame, hide2:" << geometry << it.key();
            }
        }
    }

    m_frames[contentVisibleScreen]->setProperty("contentVisible", QVariant(true));
}

void GreeterScreenManager::onScreenRemoved(QScreen *screen)
{
    if (!m_frames.contains(screen)) {
        return;
    }

    m_frames[screen]->deleteLater();
    m_frames.remove(screen);
    startRaiseContentFrame();
}

