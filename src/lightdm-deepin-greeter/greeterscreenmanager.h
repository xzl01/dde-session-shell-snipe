#ifndef GREETERSCREENMANAGER_H
#define GREETERSCREENMANAGER_H


#include <QObject>
#include <QWidget>
#include <QScreen>
#include <QMap>
#include <functional>
#include <QTimer>

class SessionBaseModel;

class GreeterScreenManager :public QObject
{
    Q_OBJECT
public:
    explicit GreeterScreenManager(QObject *parent = nullptr);
    void setSessionBaseModel(SessionBaseModel *model) {m_model = model;}
    void register_for_mutil_screen(std::function<QWidget* (QScreen *)> function);
    void startRaiseContentFrame();

private:
    void onScreenAdded(QScreen *screen);
    void onScreenRemoved(QScreen *screen);
    void raiseContentFrame();

private:
    std::function<QWidget* (QScreen *)> m_registerFunction;
    QMap<QScreen*, QWidget*> m_frames;
    QTimer *m_raiseContentFrameTimer;
    SessionBaseModel *m_model = nullptr;
};

#endif // GREETERSCREENMANAGER_H
