
#include <sys/time.h>
#define TRACE_ME_IN struct timeval tp ; gettimeofday ( &tp , nullptr ); printf("[%4ld.%4ld] In: %s\n",tp.tv_sec , tp.tv_usec,__PRETTY_FUNCTION__);
#define TRACE_ME_OUT gettimeofday (const_cast<timeval *>(&tp) , nullptr ); printf("[%4ld.%4ld] Out: %s\n",tp.tv_sec , tp.tv_usec,__PRETTY_FUNCTION__);

/*
 * Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     lixin <lixin_cm@deepin.com>
 *
 * Maintainer: lixin <lixin_cm@deepin.com>
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

#include "userloginwidget.h"
#include "lockpasswordwidget.h"
#include "framedatabind.h"
#include "userinfo.h"
#include "src/widgets/useravatar.h"
#include "src/widgets/loginbutton.h"
#include "src/global_util/constants.h"
#include "src/widgets/kblayoutwidget.h"
#include "src/session-widgets/framedatabind.h"
#include "src/global_util/keyboardmonitor.h"
#include "src/widgets/dpasswordeditex.h"

#include <DFontSizeManager>
#include <DPalette>
#include "dhidpihelper.h"

#include <QVBoxLayout>
#include <QAction>
#include <QImage>
#include <QPropertyAnimation>

static const int BlurRectRadius = 15;
static const int WidgetsSpacing = 10;
static const int Margins = 16;
static const QColor shutdownColor(QColor(247, 68, 68));

UserLoginWidget::UserLoginWidget(QWidget *parent)
    : QWidget(parent)
    , m_blurEffectWidget(new DBlurEffectWidget(this))
    , m_userAvatar(new UserAvatar(this))
    , m_nameLbl(new QLabel(this))
    , m_passwordEdit(new DPasswordEditEx(this))
    , m_lockPasswordWidget(new LockPasswordWidget)
    , m_accountEdit(new DLineEditEx(this))
    , m_lockButton(new DFloatingButton(DStyle::SP_LockElement))
    , m_kbLayoutBorder(new DArrowRectangle(DArrowRectangle::ArrowTop))
    , m_kbLayoutWidget(new KbLayoutWidget(QStringList()))
    , m_showType(NormalType)
    , m_isLock(false)
    , m_isLogin(false)
    , m_isSelected(false)
    , m_isLockNoPassword(false)
    , m_capslockMonitor(KeyboardMonitor::instance())
    , m_isAlertMessageShow(false)
    , m_aniTimer(new QTimer(this))
{
    TRACE_ME_IN;	//<<==--TracePoint!
    initUI();
    initConnect();
    TRACE_ME_OUT;	//<<==--TracePoint!

}

UserLoginWidget::~UserLoginWidget()
{
    TRACE_ME_IN;	//<<==--TracePoint!
    m_kbLayoutBorder->deleteLater();
    TRACE_ME_OUT;	//<<==--TracePoint!

}

//重置控件的状态
void UserLoginWidget::resetAllState()
{
    TRACE_ME_IN;	//<<==--TracePoint!
    m_passwordEdit->hideLoadSlider();
    m_passwordEdit->lineEdit()->clear();
    m_passwordEdit->lineEdit()->setPlaceholderText(QString());
    m_accountEdit->lineEdit()->clear();
    m_accountEdit->lineEdit()->setEnabled(true);
    if (m_authType == SessionBaseModel::LightdmType) {
        m_lockButton->setIcon(DStyle::SP_ArrowNext);
    } else {
        m_lockButton->setIcon(DStyle::SP_LockElement);
    }
    updateUI();
    TRACE_ME_OUT;	//<<==--TracePoint!

}

//密码连续输入错误5次，设置提示信息
void UserLoginWidget::setFaildMessage(const QString &message, SessionBaseModel::AuthFaildType type)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    if (m_isLock && !message.isEmpty()) {
        m_lockPasswordWidget->setMessage(message);
        m_accountEdit->lineEdit()->setEnabled(false);
        m_passwordEdit->hideAlertMessage();
        TRACE_ME_OUT;	//<<==--TracePoint!
        return;
    }

    if (type == SessionBaseModel::KEYBOARD) {
        m_passwordEdit->hideLoadSlider();
    } else {
        m_passwordEdit->hideAlertMessage();
    }

    m_passwordEdit->lineEdit()->clear();
    m_passwordEdit->lineEdit()->setPlaceholderText(message);
    m_passwordEdit->lineEdit()->update();
    TRACE_ME_OUT;	//<<==--TracePoint!

}

//密码输入错误,设置错误信息
void UserLoginWidget::setFaildTipMessage(const QString &message, SessionBaseModel::AuthFaildType type)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    Q_UNUSED(type);

    m_accountEdit->lineEdit()->setEnabled(true);
    if (m_isLock && !message.isEmpty()) {
        m_passwordEdit->hideAlertMessage();
        TRACE_ME_OUT;	//<<==--TracePoint!
        return;
    }
    m_passwordEdit->lineEdit()->clear();
    m_passwordEdit->hideLoadSlider();
    m_passwordEdit->showAlertMessage(message, 3000);
    m_passwordEdit->raise();
    TRACE_ME_OUT;	//<<==--TracePoint!

}

//设置窗体显示模式
void UserLoginWidget::setWidgetShowType(UserLoginWidget::WidgetShowType showType)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    m_showType = showType;
    updateUI();
    if (m_showType == NormalType || m_showType == IDAndPasswordType) {
        QMap<QString, int> registerFunctionIndexs;
        std::function<void (QVariant)> accountChanged = std::bind(&UserLoginWidget::onOtherPageAccountChanged, this, std::placeholders::_1);
        registerFunctionIndexs["UserLoginAccount"] = FrameDataBind::Instance()->registerFunction("UserLoginAccount", accountChanged);
        std::function<void (QVariant)> passwordChanged = std::bind(&UserLoginWidget::onOtherPagePasswordChanged, this, std::placeholders::_1);
        registerFunctionIndexs["UserLoginPassword"] = FrameDataBind::Instance()->registerFunction("UserLoginPassword", passwordChanged);
        std::function<void (QVariant)> kblayoutChanged = std::bind(&UserLoginWidget::onOtherPageKBLayoutChanged, this, std::placeholders::_1);
        registerFunctionIndexs["UserLoginKBLayout"] = FrameDataBind::Instance()->registerFunction("UserLoginKBLayout", kblayoutChanged);
        connect(this, &UserLoginWidget::destroyed, this, [ = ] {
            for (auto it = registerFunctionIndexs.constBegin(); it != registerFunctionIndexs.constEnd(); ++it)
            {
                FrameDataBind::Instance()->unRegisterFunction(it.key(), it.value());
            }
        });

        QTimer::singleShot(0, this, [ = ] {
            FrameDataBind::Instance()->refreshData("UserLoginAccount");
            FrameDataBind::Instance()->refreshData("UserLoginPassword");
            FrameDataBind::Instance()->refreshData("UserLoginKBLayout");
        });
    }
    TRACE_ME_OUT;	//<<==--TracePoint!

}

//更新窗体控件显示
void UserLoginWidget::updateUI()
{
    TRACE_ME_IN;	//<<==--TracePoint!
    m_lockPasswordWidget->hide();
    m_accountEdit->hide();
    m_nameLbl->hide();
    switch (m_showType) {
    case NoPasswordType: {
        bool isNopassword = true;
        if (m_authType == SessionBaseModel::LockType && !m_isLockNoPassword) {
            isNopassword = false;
            m_passwordEdit->lineEdit()->setFocus();
        } else {
            m_lockButton->setFocus();
        }
        m_passwordEdit->setVisible(!isNopassword && !m_isLock&& !m_isLockNoPassword);
        m_lockPasswordWidget->setVisible(m_isLock);

        m_lockButton->show();
        m_nameLbl->show();
        break;
    }
    case NormalType: {
        m_passwordEdit->setVisible(!m_isLock);
        m_lockButton->show();
        m_nameLbl->show();
        m_lockPasswordWidget->setVisible(m_isLock);
        m_passwordEdit->lineEdit()->setFocus();
        break;
    }
    case IDAndPasswordType: {
        // 解决右键菜单弹出问题
        m_passwordEdit->setContextMenuPolicy(Qt::NoContextMenu);
        m_passwordEdit->show();
        m_passwordEdit->setShowKB(false);
        m_passwordEdit->lineEdit()->setPlaceholderText(tr("Password"));
        // 解决右键菜单弹出问题
        m_accountEdit->setContextMenuPolicy(Qt::NoContextMenu);
        m_accountEdit->show();
        m_accountEdit->lineEdit()->setPlaceholderText(tr("Account"));
        m_accountEdit->lineEdit()->setFocus();
        m_nameLbl->hide();
        m_lockButton->show();

        setTabOrder(m_accountEdit->lineEdit(), m_passwordEdit->lineEdit());
        setTabOrder(m_passwordEdit->lineEdit(), m_lockButton);
        break;
    }
    case UserFrameType: {
        m_nameLbl->show();
        m_passwordEdit->hide();
        m_lockButton->hide();
        break;
    }
    case UserFrameLoginType: {
        m_nameLbl->show();
        m_passwordEdit->hide();
        m_lockButton->hide();
        break;
    }
    default:
        break;
    }

    if (m_accountEdit->isVisible()) {
        setFocusProxy(m_accountEdit->lineEdit());
    } else if (m_passwordEdit->isVisible())
        setFocusProxy(m_passwordEdit->lineEdit());
    TRACE_ME_OUT;	//<<==--TracePoint!


}

void UserLoginWidget::ShutdownPrompt(SessionBaseModel::PowerAction action)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    m_action = action;

    resetPowerIcon();
    TRACE_ME_OUT;	//<<==--TracePoint!

}

bool UserLoginWidget::inputInfoCheck(bool is_server)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    if (is_server && m_accountEdit->isVisible() && m_accountEdit->text().isEmpty()) {
        setFaildTipMessage(tr("Please enter the account"));
        m_accountEdit->lineEdit()->setFocus();
        TRACE_ME_OUT;	//<<==--TracePoint!
        return false;
    }

    if (m_passwordEdit->isVisible() && m_passwordEdit->lineEdit()->text().isEmpty()) {
        m_passwordEdit->hideLoadSlider();
        if (is_server) setFaildTipMessage(tr("Please enter the password"));
        TRACE_ME_OUT;	//<<==--TracePoint!
        return false;
    }

    if (m_lockPasswordWidget->isVisible()) {
        m_passwordEdit->hideLoadSlider();
        TRACE_ME_OUT;	//<<==--TracePoint!
        return false;
    }

    TRACE_ME_OUT;	//<<==--TracePoint!
    return true;
}

void UserLoginWidget::onOtherPageAccountChanged(const QVariant &value)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    int cursorIndex =  m_accountEdit->lineEdit()->cursorPosition();
    m_accountEdit->setText(value.toString());
    m_accountEdit->lineEdit()->setCursorPosition(cursorIndex);
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void UserLoginWidget::onOtherPagePasswordChanged(const QVariant &value)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    int cursorIndex =  m_passwordEdit->lineEdit()->cursorPosition();
    m_passwordEdit->setText(value.toString());
    m_passwordEdit->lineEdit()->setCursorPosition(cursorIndex);
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void UserLoginWidget::onOtherPageKBLayoutChanged(const QVariant &value)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    if (value.toBool()) {
        m_kbLayoutBorder->setParent(window());
    }

    m_kbLayoutBorder->setVisible(value.toBool());

    if (m_kbLayoutBorder->isVisible()) {
        m_kbLayoutBorder->raise();
    }

    refreshKBLayoutWidgetPosition();
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void UserLoginWidget::toggleKBLayoutWidget()
{
    TRACE_ME_IN;	//<<==--TracePoint!
    if (m_kbLayoutBorder->isVisible()) {
        m_kbLayoutBorder->hide();
    } else {
        // 保证布局选择控件不会被其它控件遮挡
        // 必须要将它作为主窗口的子控件
        m_kbLayoutBorder->setParent(window());
        m_kbLayoutBorder->setVisible(true);
        refreshKBLayoutWidgetPosition();
        m_kbLayoutBorder->raise();
    }
    FrameDataBind::Instance()->updateValue("UserLoginKBLayout", m_kbLayoutBorder->isVisible());
    updateClipPath();
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void UserLoginWidget::refreshKBLayoutWidgetPosition()
{
    TRACE_ME_IN;	//<<==--TracePoint!
    const QPoint &point = mapTo(m_kbLayoutBorder->parentWidget(), QPoint(m_passwordEdit->geometry().x() + (m_passwordEdit->width() / 2),
                                                                         m_passwordEdit->geometry().bottomLeft().y()));
    m_kbLayoutBorder->move(point.x(), point.y());
    m_kbLayoutBorder->setArrowX(15);
    updateClipPath();
    TRACE_ME_OUT;	//<<==--TracePoint!

}

//设置密码输入框不可用
void UserLoginWidget::disablePassword(bool disable, uint lockTime)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    m_isLock = disable;
    m_passwordEdit->setDisabled(disable);
    m_passwordEdit->setVisible(!disable);
    m_lockPasswordWidget->setVisible(disable);

    if (!m_passwordEdit->lineEdit()->text().isEmpty()) {
        m_passwordEdit->lineEdit()->clear();
    }
    m_passwordEdit->lineEdit()->setFocus();

    if (disable) {
        setFaildMessage(tr("Please try again %n minute(s) later", "", int(lockTime)));
    }

    if ( false == disable && true == m_isServerMode){
        m_accountEdit->lineEdit()->setEnabled(true);
    }
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void UserLoginWidget::updateAuthType(SessionBaseModel::AuthType type)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    m_authType = type;
    if (m_authType == SessionBaseModel::LightdmType) {
        m_lockButton->setIcon(DStyle::SP_ArrowNext);
    }
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void UserLoginWidget::updateIsLockNoPassword(const bool lockNoPassword)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    m_isLockNoPassword = lockNoPassword;
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void UserLoginWidget::receiveUserKBLayoutChanged(const QString &layout)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    m_passwordEdit->receiveUserKBLayoutChanged(layout);
    m_passwordEdit->lineEdit()->setFocus();
    emit requestUserKBLayoutChanged(layout);
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void UserLoginWidget::refreshBlurEffectPosition()
{
    TRACE_ME_IN;	//<<==--TracePoint!
    QRect rect = m_userLayout->geometry();
    rect.setTop(rect.top() + m_userAvatar->height() / 2 + m_userLayout->margin());

    m_blurEffectWidget->setGeometry(rect);
    TRACE_ME_OUT;	//<<==--TracePoint!

}

//窗体resize事件,更新阴影窗体的位置
void UserLoginWidget::resizeEvent(QResizeEvent *event)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    refreshBlurEffectPosition();
    QTimer::singleShot(0, this, &UserLoginWidget::refreshKBLayoutWidgetPosition);

    TRACE_ME_OUT;	//<<==--TracePoint!
    return QWidget::resizeEvent(event);
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void UserLoginWidget::showEvent(QShowEvent *event)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    updateUI();

    m_lockPasswordWidget->setFixedSize(QSize(m_passwordEdit->width(), m_passwordEdit->height()));

    TRACE_ME_OUT;	//<<==--TracePoint!
    return QWidget::showEvent(event);
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void UserLoginWidget::mousePressEvent(QMouseEvent *event)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    Q_UNUSED(event);
    emit clicked();
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void UserLoginWidget::paintEvent(QPaintEvent *event)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    Q_UNUSED(event);
    if (!m_isSelected) {
        TRACE_ME_OUT;	//<<==--TracePoint!
        return;}
    QPainter painter(this);
    //选中时，在窗体底端居中，绘制92*4尺寸的圆角矩形，样式数据来源于设计图
    painter.setPen(QColor(255, 255, 255, 76));
    painter.setBrush(QColor(255, 255, 255, 76));
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.drawRoundedRect(QRect(width() / 2 - 46, rect().bottom() - 4, 92, 4), 2, 2);
    TRACE_ME_OUT;	//<<==--TracePoint!

}

//fixed BUG 3518
void UserLoginWidget::hideEvent(QHideEvent *event)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    Q_UNUSED(event);

    m_passwordEdit->hideAlertMessage();
    m_kbLayoutBorder->hide();
    TRACE_ME_OUT;	//<<==--TracePoint!

}

bool UserLoginWidget::eventFilter(QObject *watched, QEvent *event)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *key_event = static_cast<QKeyEvent *>(event);
        if (key_event->modifiers() & (Qt::ControlModifier | Qt::AltModifier | Qt::MetaModifier)) {
            if ((key_event->modifiers() & Qt::ControlModifier) && key_event->key() == Qt::Key_A)
            {
                TRACE_ME_OUT;	//<<==--TracePoint!
                return false;
            }
            TRACE_ME_OUT;	//<<==--TracePoint!
            return true;
        }
    }

    TRACE_ME_OUT;	//<<==--TracePoint!
    return QObject::eventFilter(watched, event);
}

//初始化窗体控件
void UserLoginWidget::initUI()
{
    TRACE_ME_IN;	//<<==--TracePoint!
    m_userAvatar->setAvatarSize(UserAvatar::AvatarLargeSize);
    m_userAvatar->setFixedSize(100, 100);
    m_userAvatar->setFocusPolicy(Qt::NoFocus);

    m_capslockMonitor->start(QThread::LowestPriority);

    QPalette palette = m_nameLbl->palette();
    palette.setColor(QPalette::WindowText, Qt::white);
    m_nameLbl->setPalette(palette);
    m_nameLbl->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    DFontSizeManager::instance()->bind(m_nameLbl, DFontSizeManager::T2);
    m_nameLbl->setAlignment(Qt::AlignCenter);
    m_nameLbl->setTextFormat(Qt::TextFormat::PlainText);

    m_passwordEdit->lineEdit()->setContextMenuPolicy(Qt::NoContextMenu);
    m_passwordEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_passwordEdit->setFixedHeight(DDESESSIONCC::PASSWDLINEEDIT_HEIGHT);
    m_passwordEdit->lineEdit()->setAlignment(Qt::AlignCenter);
    m_passwordEdit->capslockStatusChanged(m_capslockMonitor->isCapslockOn());
    m_passwordEdit->lineEdit()->setFocusPolicy(Qt::StrongFocus);
    m_passwordEdit->lineEdit()->installEventFilter(this);

    m_kbLayoutBorder->hide();
    m_kbLayoutBorder->setBackgroundColor(QColor(102, 102, 102));    //255*0.2
    m_kbLayoutBorder->setBorderColor(QColor(0, 0, 0, 0));
    m_kbLayoutBorder->setBorderWidth(0);
    m_kbLayoutBorder->setMargin(0);
    m_kbLayoutBorder->setContent(m_kbLayoutWidget);
    m_kbLayoutBorder->setFixedWidth(DDESESSIONCC::PASSWDLINEEIDT_WIDTH);
    m_kbLayoutWidget->setFixedWidth(DDESESSIONCC::PASSWDLINEEIDT_WIDTH);

    m_kbLayoutClip=new Dtk::Widget::DClipEffectWidget(m_kbLayoutBorder);
    updateClipPath();

    m_lockPasswordWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_lockPasswordWidget->setLockIconVisible(false);
    m_accountEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_accountEdit->setClearButtonEnabled(false);
    m_accountEdit->setFixedHeight(DDESESSIONCC::PASSWDLINEEDIT_HEIGHT);
    m_accountEdit->lineEdit()->setAlignment(Qt::AlignCenter);
    m_passwordEdit->lineEdit()->setFocusPolicy(Qt::StrongFocus);
    m_accountEdit->lineEdit()->installEventFilter(this);

    m_passwordEdit->setVisible(true);

    m_userLayout = new QVBoxLayout;
    m_userLayout->setMargin(WidgetsSpacing);
    m_userLayout->setSpacing(WidgetsSpacing);

    m_userLayout->addWidget(m_userAvatar, 0, Qt::AlignHCenter);

    m_nameLayout = new QHBoxLayout;
    m_nameLayout->setMargin(0);
    m_nameLayout->setSpacing(5);
    //在用户名前，插入一个图标(m_loginLabel)用来表示多用户切换时已登录用户的标记
    m_loginLabel = new QLabel();
    QPixmap pixmap = DHiDPIHelper::loadNxPixmap(":/icons/dedpin/builtin/select.svg");
    pixmap.setDevicePixelRatio(devicePixelRatioF());
    m_loginLabel->setPixmap(pixmap);
    m_loginLabel->hide();
    m_nameLayout->addWidget(m_loginLabel);
    m_nameLayout->addWidget(m_nameLbl);
    m_nameFrame = new QFrame;
    m_nameFrame->setLayout(m_nameLayout);
    m_userLayout->addWidget(m_nameFrame, 0, Qt::AlignHCenter);

    m_userLayout->addWidget(m_accountEdit);
    m_userLayout->addWidget(m_passwordEdit);
    m_userLayout->addWidget(m_lockPasswordWidget);
    m_lockPasswordWidget->hide();

    m_blurEffectWidget->setMaskColor(DBlurEffectWidget::LightColor);
    // fix BUG 3400 设置模糊窗体的不透明度为30%
    m_blurEffectWidget->setMaskAlpha(76);
    m_blurEffectWidget->setBlurRectXRadius(BlurRectRadius);
    m_blurEffectWidget->setBlurRectYRadius(BlurRectRadius);

    m_lockButton->setFocusPolicy(Qt::StrongFocus);

    m_lockLayout = new QVBoxLayout;
    m_lockLayout->setMargin(0);
    m_lockLayout->setSpacing(0);
    m_lockLayout->addWidget(m_lockButton, 0, Qt::AlignHCenter);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setMargin(0);
    mainLayout->addStretch();
    mainLayout->addLayout(m_userLayout);
    mainLayout->addLayout(m_lockLayout);
    //此处插入18的间隔，是为了在登录和锁屏多用户切换时，绘制选中的样式（一个92*4的圆角矩形，距离阴影下边间隔14像素）
    mainLayout->addSpacing(18);
    mainLayout->addStretch();

    setLayout(mainLayout);
    TRACE_ME_OUT;	//<<==--TracePoint!

}

//初始化槽函数连接
void UserLoginWidget::initConnect()
{
    TRACE_ME_IN;	//<<==--TracePoint!
    connect(m_passwordEdit->lineEdit(), &QLineEdit::textChanged, this, [ = ](const QString & value) {
        FrameDataBind::Instance()->updateValue("UserLoginPassword", value);
    });
    connect(m_passwordEdit, &DPasswordEditEx::returnPressed, this, [ = ] {
        const QString account = m_accountEdit->text();
        const QString passwd = m_passwordEdit->text();

        m_accountEdit->lineEdit()->setEnabled(false);
        emit requestAuthUser(account, passwd);
    });

    connect(m_lockButton, &QPushButton::clicked, this, [ = ] {
        const QString account = m_accountEdit->text();
        const QString password = m_passwordEdit->text();

        if (m_passwordEdit->isVisible())
        {
            m_passwordEdit->lineEdit()->setFocus();
        }

        m_passwordEdit->showLoadSlider();
        m_accountEdit->lineEdit()->setEnabled(false);
        emit requestAuthUser(m_accountEdit->text(), password);
    });
    connect(m_userAvatar, &UserAvatar::clicked, this, &UserLoginWidget::clicked);

    connect(m_kbLayoutWidget, &KbLayoutWidget::setButtonClicked, this, &UserLoginWidget::receiveUserKBLayoutChanged);
    //鼠标点击切换键盘布局，就将DArrowRectangle隐藏掉
    connect(m_kbLayoutWidget, &KbLayoutWidget::setButtonClicked, m_kbLayoutBorder, &DArrowRectangle::hide);
    //大小写锁定状态改变
    connect(m_capslockMonitor, &KeyboardMonitor::capslockStatusChanged, m_passwordEdit, &DPasswordEditEx::capslockStatusChanged);
    connect(m_passwordEdit, &DPasswordEditEx::toggleKBLayoutWidget, this, &UserLoginWidget::toggleKBLayoutWidget);
    connect(m_passwordEdit, &DPasswordEditEx::selectionChanged, this, &UserLoginWidget::hidePasswordEditMessage);
    //字体大小改变需要更新用户名显示
    connect(qGuiApp, &QGuiApplication::fontChanged, this, &UserLoginWidget::updateNameLabel);
    TRACE_ME_OUT;	//<<==--TracePoint!

}

//设置用户名
void UserLoginWidget::setName(const QString &name)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    if (m_showType != IDAndPasswordType) {
        m_name = name;
    }
    updateNameLabel();
    TRACE_ME_OUT;	//<<==--TracePoint!

}

//设置用户头像
void UserLoginWidget::setAvatar(const QString &avatar)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    m_userAvatar->setIcon(avatar);
    TRACE_ME_OUT;	//<<==--TracePoint!

}

//设置用户头像尺寸
void UserLoginWidget::setUserAvatarSize(const AvatarSize &avatarSize)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    if (avatarSize == AvatarSmallSize) {
        m_userAvatar->setAvatarSize(m_userAvatar->AvatarSmallSize);
    } else if (avatarSize == AvatarNormalSize) {
        m_userAvatar->setAvatarSize(m_userAvatar->AvatarNormalSize);
    } else {
        m_userAvatar->setAvatarSize(m_userAvatar->AvatarLargeSize);
    }
    m_userAvatar->setFixedSize(avatarSize, avatarSize);
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void UserLoginWidget::setWidgetWidth(int width)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    this->setFixedWidth(width);
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void UserLoginWidget::setIsLogin(bool isLogin)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    m_isLogin = isLogin;
    m_loginLabel->setVisible(isLogin);
    updateNameLabel();
    if (m_isLogin) {
        m_nameFrame->setContentsMargins(0, 0, Margins, 0);
    } else {
        m_nameFrame->setContentsMargins(0, 0, 0, 0);
    }
    TRACE_ME_OUT;	//<<==--TracePoint!

}

bool UserLoginWidget::getIsLogin()
{
    TRACE_ME_IN;	//<<==--TracePoint!
    TRACE_ME_OUT;	//<<==--TracePoint!
    return  m_isLogin;
}

bool UserLoginWidget::getSelected()
{
    TRACE_ME_IN;	//<<==--TracePoint!
    TRACE_ME_OUT;	//<<==--TracePoint!
    return  m_isSelected;
}

void UserLoginWidget::setIsServer(bool isServer)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    m_isServerUser = isServer;
    TRACE_ME_OUT;	//<<==--TracePoint!

}

bool UserLoginWidget::getIsServer()
{
    TRACE_ME_IN;	//<<==--TracePoint!
    TRACE_ME_OUT;	//<<==--TracePoint!
    return m_isServerUser;
}

void UserLoginWidget::setIsServerMode(bool isServer)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    m_isServerMode = isServer;
    TRACE_ME_OUT;	//<<==--TracePoint!

}

bool UserLoginWidget::getIsServerMode()
{
    TRACE_ME_IN;	//<<==--TracePoint!
    TRACE_ME_OUT;	//<<==--TracePoint!
    return m_isServerMode;
}

void UserLoginWidget::updateKBLayout(const QStringList &list)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    QTimer::singleShot(0, this, [this, list] {
        m_kbLayoutWidget->updateButtonList(list);
        m_kbLayoutBorder->setContent(m_kbLayoutWidget);
        m_passwordEdit->setKBLayoutList(list);
        updateClipPath();
    });
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void UserLoginWidget::hideKBLayout()
{
    TRACE_ME_IN;	//<<==--TracePoint!
    m_kbLayoutBorder->hide();
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void UserLoginWidget::setKBLayoutList(QStringList kbLayoutList)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    m_KBLayoutList = kbLayoutList;
    updateKBLayout(m_KBLayoutList);
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void UserLoginWidget::setDefaultKBLayout(const QString &layout)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    m_kbLayoutWidget->setDefault(layout);
    updateClipPath();
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void UserLoginWidget::clearPassWord()
{
    TRACE_ME_IN;	//<<==--TracePoint!
    m_passwordEdit->lineEdit()->clear();
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void UserLoginWidget::setPassWordEditFocus()
{
    TRACE_ME_IN;	//<<==--TracePoint!
    m_passwordEdit->lineEdit()->setFocus();
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void UserLoginWidget::setUid(uint uid)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    m_uid = uid;
    TRACE_ME_OUT;	//<<==--TracePoint!

}

uint UserLoginWidget::uid()
{
    TRACE_ME_IN;	//<<==--TracePoint!
    TRACE_ME_OUT;	//<<==--TracePoint!
    return m_uid;
}

void UserLoginWidget::setSelected(bool isSelected)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    m_isSelected = isSelected;
    update();
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void UserLoginWidget::setFastSelected(bool isSelected)
{
    TRACE_ME_IN;	//<<==--TracePoint!
    m_isSelected = isSelected;
    repaint();
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void UserLoginWidget::updateClipPath() 
{
    TRACE_ME_IN;	//<<==--TracePoint!
    if (!m_kbLayoutClip) {
        TRACE_ME_OUT;	//<<==--TracePoint!
        return;}
    QRectF rc (0, 0, DDESESSIONCC::PASSWDLINEEIDT_WIDTH, m_kbLayoutBorder->height());
    qInfo() << "m_kbLayoutBorder->arrowHeight()-->" << rc.height();
    int iRadius = 20;
    QPainterPath path;
    path.lineTo (0, 0);
    path.lineTo (rc.width(), 0);
    path.lineTo (rc.width(), rc.height() - iRadius);
    path.arcTo (rc.width() - iRadius, rc.height() - iRadius, iRadius, iRadius, 0, -90);
    path.lineTo (rc.width() - iRadius, rc.height());
    path.lineTo (iRadius, rc.height());
    path.arcTo (0, rc.height() - iRadius, iRadius, iRadius, -90, -90);
    path.lineTo (0, rc.height() - iRadius);
    path.lineTo (0, 0);
    m_kbLayoutClip->setClipPath (path);
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void UserLoginWidget::hidePasswordEditMessage()
{
    TRACE_ME_IN;	//<<==--TracePoint!
    if (m_isAlertMessageShow) {
        m_passwordEdit->hideAlertMessage();
        m_isAlertMessageShow = false;
    }
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void UserLoginWidget::updateNameLabel()
{
    TRACE_ME_IN;	//<<==--TracePoint!
    int width = m_nameLbl->fontMetrics().width(m_name);
    int labelMaxWidth = this->width() - 3 * m_nameLayout->spacing();
    if (m_isLogin)
        labelMaxWidth -= (m_loginLabel->pixmap()->width() + m_nameLbl->height());

    if (width > labelMaxWidth) {
        QString str = m_nameLbl->fontMetrics().elidedText(m_name, Qt::ElideRight, labelMaxWidth);
        m_nameLbl->setText(str);
    }else{
        m_nameLbl->setText(m_name);
    }
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void UserLoginWidget::resetPowerIcon()
{
    TRACE_ME_IN;	//<<==--TracePoint!
    QPalette lockPalatte;
    if (m_action == SessionBaseModel::PowerAction::RequireRestart) {
        m_lockButton->setIcon(QIcon(":/img/bottom_actions/reboot.svg"));
        lockPalatte.setColor(QPalette::Highlight, shutdownColor);
    } else if (m_action == SessionBaseModel::PowerAction::RequireShutdown) {
        m_lockButton->setIcon(QIcon(":/img/bottom_actions/shutdown.svg"));
        lockPalatte.setColor(QPalette::Highlight, shutdownColor);
    } else {
        if (m_authType == SessionBaseModel::LightdmType) {
            m_lockButton->setIcon(DStyle::SP_ArrowNext);
            TRACE_ME_OUT;	//<<==--TracePoint!
            return;
        } else {
            m_lockButton->setIcon(DStyle::SP_LockElement);
        }
    }
    m_lockButton->setPalette(lockPalatte);
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void UserLoginWidget::unlockSuccessAni()
{
    TRACE_ME_IN;	//<<==--TracePoint!
    m_timerIndex = 0;
    m_lockButton->setIcon(DStyle::SP_LockElement);

    disconnect(m_connection);
    m_connection = connect(m_aniTimer, &QTimer::timeout, [ & ]() {
        if (m_timerIndex <= 11) {
            m_lockButton->setIcon(QIcon(QString(":/img/unlockTrue/unlock_%1.svg").arg(m_timerIndex)));
        } else {
            m_aniTimer->stop();
            emit unlockActionFinish();
            m_lockButton->setIcon(DStyle::SP_LockElement);
        }

        m_timerIndex++;
    });

    m_aniTimer->start(15);
    TRACE_ME_OUT;	//<<==--TracePoint!

}

void UserLoginWidget::unlockFailedAni()
{
    TRACE_ME_IN;	//<<==--TracePoint!
    m_passwordEdit->lineEdit()->clear();
    m_passwordEdit->hideLoadSlider();

    m_timerIndex = 0;
    m_lockButton->setIcon(DStyle::SP_LockElement);

    disconnect(m_connection);
    m_connection = connect(m_aniTimer, &QTimer::timeout, [ & ](){
        if(m_timerIndex <= 15){
            m_lockButton->setIcon(QIcon(QString(":/img/unlockFalse/unlock_error_%1.svg").arg(m_timerIndex)));
        }  else {
            m_aniTimer->stop();
            resetPowerIcon();
        }

        m_timerIndex++;
    });

    m_aniTimer->start(15);
    TRACE_ME_OUT;	//<<==--TracePoint!

}
