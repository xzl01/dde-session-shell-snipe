// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MODULE_INTERFACE_H
#define MODULE_INTERFACE_H

#include <QtCore>

const QString BASE_API_VERSION = "2.0.0";

namespace dss {
namespace module {

const QStringList ValidVersions = {"1.0.0", "1.0.1"};
class BaseModuleInterface
{
public:
    /**
     * @brief The ModuleType enum
     * 模块的类型
     */
    enum ModuleType {
        LoginType,   // 登陆插件
        TrayType     // 托盘插件
    };

    /**
    * @brief The LoadType enum
    * 模块加载的类型
    */
    enum LoadType {
        Load,       // 加载插件
        Notload     // 不加载插件
    };

    virtual ~BaseModuleInterface() = default;

    /**
     * @brief 界面相关的初始化
     * 插件在非主线程加载，故界面相关的初始化需要放在这个方法里，由主程序调用并初始化
     *
     * @since 1.0.0
     */
    virtual void init() = 0;

    /**
     * @brief 键值，用于与其它模块区分
     * @return QString 插件的唯一标识，由插件自己定义
     *
     * @since 1.0.0
     */
    virtual QString key() const = 0;

    /**
     * @brief 模块想显示的界面
     * content 对象由模块自己管理
     * @return QWidget*
     *
     * @since 1.0.0
     */
    virtual QWidget *content() = 0;

    /**
     * @brief 模块的类型 登陆器会根据不同
     * @return ModuleType 详见`ModuleType`说明
     *
     * @since 1.0.0
     */
    virtual ModuleType type() const = 0;

    /**
     * @brief isNeedInitPlugin 是否需要初始化插件，主要适应插件通过配置不显示等情况。
     * @return
     */
    virtual bool isNeedInitPlugin() const = 0;
};

} // namespace module
} // namespace dss

Q_DECLARE_INTERFACE(dss::module::BaseModuleInterface, "com.deepin.dde.shell.Modules")

#endif // MODULE_INTERFACE_H
