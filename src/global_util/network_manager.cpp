/*
 * Copyright (C) 2019 ~ 2022 Uniontech Software Technology Co.,Ltd
 *
 * Author:     mayijian <mayijian@uniontech.com>
 *
 * Maintainer: mayijian <mayijian@uniontech.com>
 *
 */

#include "network_manager.h"
#include <com_deepin_daemon_network.h>

#include <QDBusConnection>

using DBusNetwork = com::deepin::daemon::Network;

#define MAX_BUFFER_LENGTH (1024)

static const QString DefaultPingUrl = "platform-udcp.uos.icbc";

Connectivity::Connectivity(const QString &url, QObject *parent)
    : QThread(parent)
    , m_url(url)
    , m_connected(ping())
    , m_loop(false)
    , m_interval(DEFAULT_CHECK_INTERVAL)
    , m_count(DEFAULT_CHECK_COUNT)
{
}

Connectivity::~Connectivity()
{
}

void Connectivity::stop()
{
    if (isRunning()) {
        m_loop = false;
        m_count = 0;
        wait();
    }
}

void Connectivity::startCheck(unsigned long interval, int count)
{
    stop();
    m_interval = interval;
    m_count = count;
    start();
}

void Connectivity::setConnectedState(bool connected)
{
    if (m_connected == connected) {
        return;
    }

    m_connected = connected;
    Q_EMIT connectedStateChanged(m_connected);
}

void Connectivity::run()
{
    // 若时间间隔参数等于0,则默认只触发一次状态检测
    if (m_interval == 0) {
        setConnectedState(ping());
        return;
    }

    if (m_count <= 0) {
        m_loop = true;
        while (m_loop) {
            bool connected = ping();
            setConnectedState(connected);
            if (connected) {
                break;
            }

            QThread::msleep(m_interval);
        }

        return;
    }

    for (int i = 0; i < m_count; i++) {
        bool connected = ping();
        setConnectedState(connected);
        // 检测到连接则退出
        if (connected) {
            break;
        }

        QThread::msleep(m_interval);
    }
}

bool Connectivity::ping(unsigned int count, unsigned int deadline)
{
    bool ret = false;
    do {
        const QString &cmd = QString("ping -c %1 -w %2 %3 | grep loss | awk '{print $6}' | awk -F '%' '{print $1}'").arg(count).arg(deadline).arg(m_url);
        FILE *fp = popen(cmd.toStdString().c_str(), "r");
        if (!fp) {
            break;
        }

        char buffer[MAX_BUFFER_LENGTH] = {0};
        fgets(buffer, sizeof(buffer) - 1, fp);
        pclose(fp);
        bool success = false;
        int loss = QString(buffer).trimmed().toInt(&success);
        // 非法ping命令语句的执行结果为空,需异常处理
        if (!success) {
            qWarning() << "ERROR, execute ping command exception，illegal cmd: " << cmd;
            break;
        }

        if (loss != 0) {
            qWarning() << "disconnected, PING URL: " << m_url << ", " << loss << "% packet loss";
            break;
        }

        ret = true;
    } while (false);

    return ret;
}

NetworkManager *NetworkManager::instance()
{
    static NetworkManager manager;
    return &manager;
}

bool NetworkManager::connected()
{
    if (!getDeviceNetworkConnectedState()) {
        return false;
    }

    return m_connectivity->connectedState();
}

NetworkManager::NetworkManager(QObject *parent)
    : QObject(parent)
    , m_networkInterface(new DBusNetwork("com.deepin.daemon.Network", "/com/deepin/daemon/Network", QDBusConnection::sessionBus(), this))
    , m_connectivity(new Connectivity(DefaultPingUrl, this))
    , m_wiredConnected(false)
    , m_wirelessConnected(false)
{
    onActiveConnectionsChanged(QString());
    connect(m_networkInterface, &DBusNetwork::ActiveConnectionsChanged, this, &NetworkManager::onActiveConnectionsChanged);
    connect(m_connectivity, &Connectivity::connectedStateChanged, this, &NetworkManager::serverConnectedStateChanged);
}

bool NetworkManager::getDeviceNetworkConnectedState()
{
    QDBusInterface netIface("org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager", "org.freedesktop.NetworkManager", QDBusConnection::systemBus(), this);
    const QDBusObjectPath &primaryConnection = qvariant_cast<QDBusObjectPath>(netIface.property("PrimaryConnection"));
    const quint32 state = qvariant_cast<quint32>(netIface.property("State"));
    return !((primaryConnection.path() == "/") || (state <= NM_STATE_DISCONNECTED));
}

void NetworkManager::updateDeviceInfoList(const QJsonArray &array, DeviceType type)
{
    bool laseWireConnected = m_wiredConnected;
    bool laseWirelessConnected = m_wirelessConnected;
    for (const auto arr : array) {
        if (!arr.isObject()) {
            continue;
        }

        const QJsonObject &obj = arr.toObject();
        const int state = obj["State"].toInt();
        const QString &interface = obj["Interface"].toString();
        // 网卡设备断开连接下线通知
        if (state <= Disconnected) {
            if (type == Wired) {
                m_wiredConnected = false;
            }

            if (type == Wireless) {
                m_wirelessConnected = false;
            }

            // 前后状态发送变更时，才会发送网络连接状态变更信号
            if ((!m_wiredConnected && laseWireConnected) || (!m_wirelessConnected && laseWirelessConnected)) {
                qDebug() << "network [DOWN], interface: " << interface << ", type:" << type;
                m_connectivity->startCheck();
            }
        }

        // 网卡设备重新连接上线通知
        if (state == Activated) {
            if (type == Wired) {
                m_wiredConnected = true;
            }

            if (type == Wireless) {
                m_wirelessConnected = true;
            }

            if ((m_wiredConnected && !laseWireConnected) || (m_wirelessConnected && !laseWirelessConnected)) {
                qDebug() << "network [UP], interface: " << interface << ", type:" << type;
                m_connectivity->startCheck();
            }
        }
    }
}

void NetworkManager::onActiveConnectionsChanged(const QString &value)
{
    Q_UNUSED(value);
    QJsonParseError err;
    const QJsonDocument &doc = QJsonDocument::fromJson(m_networkInterface->devices().toLocal8Bit(), &err);
    if (!doc.isObject() || (err.error != QJsonParseError::NoError)) {
        return;
    }

    const QJsonObject &obj = doc.object();
    updateDeviceInfoList(obj["wired"].toArray(), Wired);
    updateDeviceInfoList(obj["wireless"].toArray(), Wireless);
}
