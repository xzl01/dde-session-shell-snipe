/*
 * Copyright (C) 2019 ~ 2022 Uniontech Software Technology Co.,Ltd
 *
 * Author:     mayijian <mayijian@uniontech.com>
 *
 * Maintainer: mayijian <mayijian@uniontech.com>
 *
 */

#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include <QThread>
#include <QList>

#define DEFAULT_CHECK_INTERVAL (3 * 1000) // 默认循环检测间隔时间为3s
#define DEFAULT_CHECK_COUNT 60 // 默认循环检测60次

#define PACKS_COUNT 1 // ping发包数
#define DEAD_LINE 2 // ping最多等待超时秒数

/**
 * @name Connectivity
 * @brief 连接器工作线程，用于检测服务端网络能否访问，持续循环连接访问，直到连接成功就退出
 */
class Connectivity : public QThread
{
    Q_OBJECT
public:
    Connectivity(const QString &url, QObject *parent = nullptr);
    ~Connectivity();

    inline bool connectedState() const { return m_connected; }

    void stop();
    void startCheck(unsigned long interval = DEFAULT_CHECK_INTERVAL, int count = DEFAULT_CHECK_COUNT); // 开启网络检测
    void setConnectedState(bool connected);

protected:
    void run();

private:
    bool ping(unsigned int count = PACKS_COUNT, unsigned int deadline = DEAD_LINE);

Q_SIGNALS:
    void connectedStateChanged(bool connected);

private:
    const QString m_url;
    bool m_connected;
    bool m_loop; // 循环状态标识
    unsigned long m_interval; // 每次间隔时间为interval,单位ms
    int m_count; // 循环检测调用count次，若count小于等于0,则永久循环检测
};

class __Network;
/**
 * @name NetworkManager
 * @brief 服务端网络管理模块
 */
class NetworkManager : public QObject
{
    Q_OBJECT
public:
    enum NMState {
        NM_STATE_UNKNOWN = 0,
        NM_STATE_ASLEEP = 10,
        NM_STATE_DISCONNECTED = 20,
        NM_STATE_DISCONNECTING = 30,
        NM_STATE_CONNECTING = 40,
        NM_STATE_CONNECTED_LOCAL = 50,
        NM_STATE_CONNECTED_SITE = 60,
        NM_STATE_CONNECTED_GLOBAL = 70,
    };

    enum DeviceStatus {
        Unknown = 0,
        Unmanaged = 10,
        Unavailable = 20,
        Disconnected = 30,
        Prepare = 40,
        Config = 50,
        NeedAuth = 60,
        IpConfig = 70,
        IpCheck = 80,
        Secondaries = 90,
        Activated = 100,
        Deactivation = 110,
        Failed = 120,
    };

    enum DeviceType {
        None,
        Wired,
        Wireless,
    };

public:
    static NetworkManager *instance();

public:
    bool connected(); // 网络连接状态

private:
    explicit NetworkManager(QObject *parent = nullptr);
    bool getDeviceNetworkConnectedState(); // 获取设备网络连接状态
    void updateDeviceInfoList(const QJsonArray &array, DeviceType type);

private Q_SLOTS:
    void onActiveConnectionsChanged(const QString &value);

Q_SIGNALS:
    void serverConnectedStateChanged(bool connected);

private:
    __Network *m_networkInterface;
    Connectivity *m_connectivity;
    bool m_wiredConnected; // 有线网络连接状态
    bool m_wirelessConnected; // 无线网络连接状态
};

#endif // NETWORK_MANAGER_H
