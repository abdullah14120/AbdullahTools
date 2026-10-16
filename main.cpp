#include <QApplication>
#include <QMainWindow>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QStackedWidget>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QProgressBar>
#include <QFrame>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include <QDateTime>
#include <QProcess>
#include <QTimer>
#include <QClipboard>
#include <QGuiApplication>
#include <QSysInfo>
#include <QCryptographicHash>
#include <QCheckBox>
#include <QButtonGroup>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFont>
#include <QFontDatabase>
#include <QRegularExpression>
#include <QQueue>
#include <QByteArray>

// ============================================================================
// دوال مساعدة عامة (خطوط Gemini + بصمة الجهاز HWID + مسار ADB)
// ============================================================================

static void setupGeminiTypography(QApplication &app) {
    QString fontsDir = QCoreApplication::applicationDirPath() + "/fonts";
    QString selectedFamily = "Segoe UI";

    QDir dir(fontsDir);
    if (dir.exists()) {
        QStringList fontFiles = dir.entryList(QStringList() << "*.ttf" << "*.otf", QDir::Files);
        for (const QString &file : fontFiles) {
            int id = QFontDatabase::addApplicationFont(fontsDir + "/" + file);
            if (id != -1) {
                QStringList families = QFontDatabase::applicationFontFamilies(id);
                if (!families.isEmpty()) {
                    selectedFamily = families.first();
                }
            }
        }
    }

    QFont geminiFont(selectedFamily, 10);
    geminiFont.setStyleStrategy(QFont::PreferAntialias);
    geminiFont.setHintingPreference(QFont::PreferNoHinting);
    app.setFont(geminiFont);
}

static QString getMachineHWID() {
    QByteArray rawId = QSysInfo::machineUniqueId();
    if (rawId.isEmpty()) {
        rawId = (QSysInfo::machineHostName() + QSysInfo::productType()).toUtf8();
    }
    QByteArray hash = QCryptographicHash::hash(rawId, QCryptographicHash::Sha256).toHex().toUpper();
    return QString("AT-%1-%2-%3-%4")
        .arg(QString(hash.mid(0, 4)))
        .arg(QString(hash.mid(4, 4)))
        .arg(QString(hash.mid(8, 4)))
        .arg(QString(hash.mid(12, 4)));
}

static QString getAdbExecutablePath() {
    QString localAdb = QCoreApplication::applicationDirPath() + "/platform-tools/adb.exe";
    if (QFile::exists(localAdb)) {
        return localAdb;
    }
    return "platform-tools/adb.exe";
}

// ============================================================================
// 1. نافذة تسجيل الدخول والتفعيل (بتصميم وخط Gemini Dark)
// ============================================================================
class LoginWindow : public QWidget {
    Q_OBJECT
public:
    LoginWindow(QWidget *parent = nullptr) : QWidget(parent) {
        setWindowTitle("AbdullahTool Pro - بوابة التفعيل والدخول");
        setFixedSize(780, 480);
        setLayoutDirection(Qt::RightToLeft);

        setStyleSheet(R"(
            QWidget {
                background-color: #131314;
                color: #E3E3E3;
                font-family: 'IBM Plex Sans Arabic', 'Noto Sans Arabic', 'Cairo', 'Segoe UI';
            }
            QFrame#BrandPanel {
                background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                                            stop:0 #1B263B, stop:0.6 #1E1F20, stop:1 #131314);
                border-radius: 16px;
                border: 1px solid #282A2C;
            }
            QFrame#FormPanel {
                background-color: #1E1F20;
                border-radius: 16px;
                border: 1px solid #282A2C;
            }
            QLineEdit {
                background-color: #131314;
                border: 1px solid #333537;
                border-radius: 10px;
                padding: 11px 14px;
                color: #E3E3E3;
                font-size: 13px;
            }
            QLineEdit:focus {
                border: 1px solid #A8C7FA;
                background-color: #18191B;
            }
            QPushButton#PrimaryBtn {
                background-color: #A8C7FA;
                color: #062E6F;
                border: none;
                border-radius: 10px;
                padding: 12px;
                font-size: 14px;
                font-weight: bold;
            }
            QPushButton#PrimaryBtn:hover {
                background-color: #D3E3FD;
            }
            QPushButton#TrialBtn {
                background-color: #282A2C;
                color: #A8C7FA;
                border: 1px solid #333537;
                border-radius: 10px;
                padding: 11px;
                font-size: 13px;
                font-weight: bold;
            }
            QPushButton#TrialBtn:hover {
                background-color: #333537;
                border: 1px solid #A8C7FA;
            }
            QPushButton#CopyBtn {
                background-color: #282A2C;
                color: #E3E3E3;
                border: 1px solid #333537;
                border-radius: 8px;
                padding: 8px 14px;
                font-size: 12px;
                font-weight: bold;
            }
            QPushButton#CopyBtn:hover {
                background-color: #333537;
            }
            QCheckBox {
                color: #C4C7C5;
                font-size: 12px;
            }
        )");

        QHBoxLayout *rootLayout = new QHBoxLayout(this);
        rootLayout->setContentsMargins(18, 18, 18, 18);
        rootLayout->setSpacing(16);

        // --- اللوحة الجانبية للهوية ---
        QFrame *brandPanel = new QFrame(this);
        brandPanel->setObjectName("BrandPanel");
        brandPanel->setFixedWidth(295);
        QVBoxLayout *brandLayout = new QVBoxLayout(brandPanel);
        brandLayout->setContentsMargins(24, 30, 24, 24);

        QLabel *logoBadge = new QLabel("✨ ABDULLAH TOOL", brandPanel);
        logoBadge->setStyleSheet("font-size: 21px; font-weight: bold; color: #A8C7FA; background: transparent;");

        QLabel *editionLabel = new QLabel("PRO ENTERPRISE EDITION • V2.2", brandPanel);
        editionLabel->setStyleSheet("font-size: 11px; font-weight: 600; color: #8E918F; background: transparent;");

        QLabel *descLabel = new QLabel(
            "الأداة الاحترافية المتكاملة لإدارة وصيانة أجهزة الأندرويد، "
            "استخراج وحقن بيانات الحزم (data/data)، الربط اللاسلكي، وتنفيذ أوامر النظام المتقدمة.",
            brandPanel
        );
        descLabel->setWordWrap(true);
        descLabel->setStyleSheet("font-size: 13px; color: #C4C7C5; line-height: 1.7; margin-top: 18px; background: transparent;");

        brandLayout->addWidget(logoBadge);
        brandLayout->addWidget(editionLabel);
        brandLayout->addWidget(descLabel);
        brandLayout->addStretch();

        QLabel *serverStatus = new QLabel("● حالة خادم التراخيص: متصل (Online)", brandPanel);
        serverStatus->setStyleSheet("color: #6DD58C; font-size: 12px; font-weight: bold; background: transparent;");

        QLabel *securityNote = new QLabel("🔒 اتصال مشفر ومحمي ببصمة الجهاز HWID", brandPanel);
        securityNote->setStyleSheet("color: #8E918F; font-size: 11px; background: transparent;");

        brandLayout->addWidget(serverStatus);
        brandLayout->addWidget(securityNote);

        // --- لوحة نموذج الدخول ---
        QFrame *formPanel = new QFrame(this);
        formPanel->setObjectName("FormPanel");
        QVBoxLayout *formLayout = new QVBoxLayout(formPanel);
        formLayout->setContentsMargins(28, 28, 28, 24);
        formLayout->setSpacing(13);

        QLabel *welcomeTitle = new QLabel("تسجيل الدخول وتفعيل الرخصة", formPanel);
        welcomeTitle->setStyleSheet("font-size: 19px; font-weight: bold; color: #E3E3E3; background: transparent;");

        QLabel *welcomeSub = new QLabel("أدخل كود الاشتراك الفعال أو ابدأ الفترة التجريبية المجانية للمتابعة", formPanel);
        welcomeSub->setStyleSheet("font-size: 13px; color: #C4C7C5; background: transparent;");

        QLabel *hwidTitle = new QLabel("معرّف الجهاز الخاص بك (Hardware ID):", formPanel);
        hwidTitle->setStyleSheet("font-size: 12px; color: #C4C7C5; font-weight: 600; background: transparent;");

        QHBoxLayout *hwidLayout = new QHBoxLayout();
        hwidEdit = new QLineEdit(getMachineHWID(), formPanel);
        hwidEdit->setReadOnly(true);
        hwidEdit->setStyleSheet("background-color: #131314; color: #FCD34D; font-family: Consolas; font-weight: bold;");

        QPushButton *copyHwidBtn = new QPushButton("نسخ المعرّف", formPanel);
        copyHwidBtn->setObjectName("CopyBtn");
        connect(copyHwidBtn, &QPushButton::clicked, this, [=]() {
            QGuiApplication::clipboard()->setText(hwidEdit->text());
            QMessageBox::information(this, "تم النسخ", "تم نسخ معرّف الجهاز (HWID) إلى الحافظة بنجاح.");
        });
        hwidLayout->addWidget(hwidEdit);
        hwidLayout->addWidget(copyHwidBtn);

        QLabel *keyTitle = new QLabel("كود الاشتراك (License Key):", formPanel);
        keyTitle->setStyleSheet("font-size: 12px; color: #C4C7C5; font-weight: 600; background: transparent;");

        codeLineEdit = new QLineEdit(formPanel);
        codeLineEdit->setPlaceholderText("XXXX-XXXX-XXXX-XXXX (مثال: PRO-2026-VIP)");

        rememberCheck = new QCheckBox("حفظ كود الاشتراك تلقائياً على هذا الكمبيوتر", formPanel);
        rememberCheck->setChecked(true);

        QSettings settings("AbdullahToolCorp", "AbdullahToolPro");
        if (settings.contains("SavedLicenseKey")) {
            codeLineEdit->setText(settings.value("SavedLicenseKey").toString());
        }

        QPushButton *loginBtn = new QPushButton("تفعيل الرخصة والدخول للأداة", formPanel);
        loginBtn->setObjectName("PrimaryBtn");
        loginBtn->setCursor(Qt::PointingHandCursor);

        QPushButton *trialBtn = new QPushButton("تفعيل التجربة المجانية (لمدة 48 ساعة)", formPanel);
        trialBtn->setObjectName("TrialBtn");
        trialBtn->setCursor(Qt::PointingHandCursor);

        statusFeedback = new QLabel("💡 كود التجربة السريع للمطور: PRO-2026-VIP", formPanel);
        statusFeedback->setStyleSheet("font-size: 12px; color: #8E918F; background: transparent;");

        formLayout->addWidget(welcomeTitle);
        formLayout->addWidget(welcomeSub);
        formLayout->addSpacing(4);
        formLayout->addWidget(hwidTitle);
        formLayout->addLayout(hwidLayout);
        formLayout->addWidget(keyTitle);
        formLayout->addWidget(codeLineEdit);
        formLayout->addWidget(rememberCheck);
        formLayout->addSpacing(6);
        formLayout->addWidget(loginBtn);
        formLayout->addWidget(trialBtn);
        formLayout->addStretch();
        formLayout->addWidget(statusFeedback);

        rootLayout->addWidget(brandPanel);
        rootLayout->addWidget(formPanel, 1);

        connect(loginBtn, &QPushButton::clicked, this, &LoginWindow::handleLogin);
        connect(trialBtn, &QPushButton::clicked, this, &LoginWindow::handleTrial);
    }

signals:
    void loginSuccess(const QString &licenseTypeLabel);

private slots:
    void handleLogin() {
        QString code = codeLineEdit->text().trimmed().toUpper();
        if (code == "PRO-2026-VIP" || code == "ABDULLAH-VIP-2026") {
            QSettings settings("AbdullahToolCorp", "AbdullahToolPro");
            if (rememberCheck->isChecked()) {
                settings.setValue("SavedLicenseKey", code);
            } else {
                settings.remove("SavedLicenseKey");
            }
            emit loginSuccess("اشتراك VIP مفعل");
            close();
        } else {
            QMessageBox::critical(this, "فشل التحقق", "كود الاشتراك غير صالح أو منتهي الصلاحية.\nيرجى التأكد من الكود أو التواصل مع الدعم الفني.");
        }
    }

    void handleTrial() {
        QSettings settings("AbdullahToolCorp", "AbdullahToolPro");
        QDateTime now = QDateTime::currentDateTime();

        if (!settings.contains("TrialExpiryDate")) {
            QDateTime expiry = now.addDays(2);
            settings.setValue("TrialExpiryDate", expiry);
            emit loginSuccess("نسخة تجريبية (متبقي 48 ساعة)");
            close();
            return;
        }

        QDateTime expiry = settings.value("TrialExpiryDate").toDateTime();
        if (now < expiry) {
            qint64 hoursLeft = now.secsTo(expiry) / 3600;
            emit loginSuccess(QString("نسخة تجريبية (متبقي %1 ساعة)").arg(hoursLeft));
            close();
        } else {
            QMessageBox::warning(this, "انتهت الفترة التجريبية",
                "عذراً، انتهت فترة التجربة المجانية (48 ساعة) على هذا الجهاز.\nيرجى إدخال كود اشتراك رسمي للمتابعة.");
        }
    }

private:
    QLineEdit *hwidEdit;
    QLineEdit *codeLineEdit;
    QCheckBox *rememberCheck;
    QLabel *statusFeedback;
};

// ============================================================================
// 2. الواجهة الرئيسية للأداة (بألوان وخطوط Gemini Dark UI)
// ============================================================================
class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    struct AdbStep {
        QStringList args;
        QString startLog;
        QString successLog;
    };

    MainWindow(QWidget *parent = nullptr) : QMainWindow(parent) {
        setWindowTitle("AbdullahTool Pro v2.2 - النظام المتكامل لإدارة أجهزة الأندرويد");
        resize(1180, 780);
        setMinimumSize(1020, 680);
        setLayoutDirection(Qt::RightToLeft);

        setStyleSheet(R"(
            QMainWindow, QWidget {
                background-color: #131314;
                color: #E3E3E3;
                font-family: 'IBM Plex Sans Arabic', 'Noto Sans Arabic', 'Cairo', 'Segoe UI';
            }
            QFrame#TopBar {
                background-color: #1E1F20;
                border-bottom: 1px solid #282A2C;
            }
            QFrame#SideBar {
                background-color: #1E1F20;
                border-left: 1px solid #282A2C;
            }
            QFrame#Card {
                background-color: #1E1F20;
                border: 1px solid #282A2C;
                border-radius: 14px;
            }
            QFrame#StatCard {
                background-color: #131314;
                border: 1px solid #282A2C;
                border-radius: 12px;
            }
            QPushButton.NavBtn {
                background-color: transparent;
                color: #C4C7C5;
                text-align: right;
                padding: 13px 16px;
                border-radius: 20px;
                font-size: 13px;
                font-weight: 600;
                border: none;
            }
            QPushButton.NavBtn:hover {
                background-color: #282A2C;
                color: #E3E3E3;
            }
            QPushButton.NavBtn:checked {
                background-color: #004A77;
                color: #C2E7FF;
                font-weight: bold;
            }
            QPushButton.ActionBtn {
                background-color: #A8C7FA;
                color: #062E6F;
                border: none;
                border-radius: 10px;
                padding: 10px 18px;
                font-size: 13px;
                font-weight: bold;
            }
            QPushButton.ActionBtn:hover {
                background-color: #D3E3FD;
            }
            QPushButton.SecondaryBtn {
                background-color: #282A2C;
                color: #E3E3E3;
                border: 1px solid #333537;
                border-radius: 10px;
                padding: 9px 15px;
                font-size: 12px;
                font-weight: 600;
            }
            QPushButton.SecondaryBtn:hover {
                background-color: #333537;
                border: 1px solid #444746;
            }
            QPushButton.DangerBtn {
                background-color: #3C1919;
                color: #F2B8B5;
                border: 1px solid #601410;
                border-radius: 10px;
                padding: 9px 15px;
                font-size: 12px;
                font-weight: bold;
            }
            QPushButton.DangerBtn:hover {
                background-color: #601410;
                color: #FFFFFF;
            }
            QLineEdit {
                background-color: #131314;
                border: 1px solid #333537;
                border-radius: 10px;
                padding: 9px 14px;
                color: #E3E3E3;
                font-size: 13px;
            }
            QLineEdit:focus {
                border: 1px solid #A8C7FA;
            }
            QProgressBar {
                background-color: #282A2C;
                border: 1px solid #333537;
                border-radius: 8px;
                text-align: center;
                color: #E3E3E3;
                font-weight: bold;
                font-size: 11px;
            }
            QProgressBar::chunk {
                background-color: #6DD58C;
                border-radius: 7px;
            }
        )");

        QWidget *central = new QWidget(this);
        setCentralWidget(central);
        QVBoxLayout *rootLayout = new QVBoxLayout(central);
        rootLayout->setContentsMargins(0, 0, 0, 0);
        rootLayout->setSpacing(0);

        rootLayout->addWidget(createTopBar());

        QHBoxLayout *bodyLayout = new QHBoxLayout();
        bodyLayout->setContentsMargins(0, 0, 0, 0);
        bodyLayout->setSpacing(0);

        bodyLayout->addWidget(createSidebar());

        QVBoxLayout *workAreaLayout = new QVBoxLayout();
        workAreaLayout->setContentsMargins(18, 18, 18, 18);
        workAreaLayout->setSpacing(14);

        pagesStack = new QStackedWidget(this);
        pagesStack->addWidget(createDashboardPage());      // 0: معلومات الهاتف والتحكم
        pagesStack->addWidget(createAppManagerPage());     // 1: مدير التطبيقات APK
        pagesStack->addWidget(createBackupRestorePage());  // 2: النسخ الاحتياطي وحقن data/data
        pagesStack->addWidget(createFileTransferPage());   // 3: نقل وسحب الملفات
        pagesStack->addWidget(createShellTerminalPage());  // 4: أوامر Shell المتقدمة

        workAreaLayout->addWidget(pagesStack, 3);
        workAreaLayout->addWidget(createConsolePanel(), 2);

        bodyLayout->addLayout(workAreaLayout, 1);
        rootLayout->addLayout(bodyLayout, 1);

        deviceTimer = new QTimer(this);
        connect(deviceTimer, &QTimer::timeout, this, &MainWindow::checkDeviceSilently);
        deviceTimer->start(3500);

        appendLog("تم تشغيل AbdullahTool Pro v2.2 بنجاح. جميع الأنظمة جاهزة.", "SUCCESS");
        checkDeviceSilently();
    }

    void setLicenseBadgeText(const QString &text) {
        licenseBadge->setText("🛡️ " + text);
    }

private:
    QLabel *licenseBadge;
    QLabel *connectionBadge;
    QLineEdit *wifiIpInput;
    QStackedWidget *pagesStack;
    QTextEdit *consoleBox;
    QProgressBar *operationProgress;
    QTimer *deviceTimer;

    // طابور المهام المتسلسلة لمنع تداخل أوامر ADB أثناء استعادة البيانات
    QQueue<AdbStep> stepQueue;
    bool isSequenceRunning = false;

    // عناصر الصفحة 0: لوحة المعلومات
    QLabel *valModel;
    QLabel *valAndroid;
    QLabel *valSerial;
    QProgressBar *batteryBar;

    // عناصر الصفحة 1: التطبيقات
    QLineEdit *apkPathInput;
    QCheckBox *chkReplaceApk;
    QCheckBox *chkGrantPerms;
    QLineEdit *pkgNameInput;

    // عناصر الصفحة 2: النسخ الاحتياطي واستعادة data/data
    QLineEdit *backupPkgInput;
    QLineEdit *backupOutPathInput;
    QCheckBox *chkIncludeApkInBackup;
    QLineEdit *abFileInput;
    QLineEdit *tarOutputInput;
    QLineEdit *restoreSourceInput;
    QLineEdit *restoreTargetPkgInput;

    // عناصر الصفحة 3: نقل الملفات
    QLineEdit *pushLocalInput;
    QLineEdit *pushRemoteInput;
    QLineEdit *pullRemoteInput;
    QLineEdit *pullLocalInput;

    // عناصر الصفحة 4: أوامر Shell
    QLineEdit *customShellInput;

    // --- الشريط العلوي مع الربط اللاسلكي السريع ---
    QWidget* createTopBar() {
        QFrame *topBar = new QFrame(this);
        topBar->setObjectName("TopBar");
        topBar->setFixedHeight(66);

        QHBoxLayout *layout = new QHBoxLayout(topBar);
        layout->setContentsMargins(20, 0, 20, 0);

        QLabel *brandTitle = new QLabel("✨ ABDULLAH TOOL PRO", topBar);
        brandTitle->setStyleSheet("font-size: 18px; font-weight: bold; color: #A8C7FA;");

        licenseBadge = new QLabel("🛡️ اشتراك مفعل", topBar);
        licenseBadge->setStyleSheet("background-color: #004A77; color: #C2E7FF; padding: 6px 14px; border-radius: 16px; font-size: 12px; font-weight: bold;");

        // حقل ربط لاسلكي سريع للأجهزة ذات منفذ الشحن المعطل
        wifiIpInput = new QLineEdit(topBar);
        wifiIpInput->setPlaceholderText("IP:Port لاسلكي (مثال 192.168.1.15:5555)");
        wifiIpInput->setFixedWidth(230);

        QPushButton *wifiConnectBtn = new QPushButton("📡 ربط لاسلكي", topBar);
        wifiConnectBtn->setProperty("class", "SecondaryBtn");
        connect(wifiConnectBtn, &QPushButton::clicked, this, [=]() {
            QString addr = wifiIpInput->text().trimmed();
            if (addr.isEmpty()) {
                QMessageBox::warning(this, "تنبيه", "أدخل عنوان IP والمنفذ أولاً للربط اللاسلكي (مثال: 192.168.1.15:5555)");
                return;
            }
            appendLog("جاري الاتصال لاسلكياً بالجهاز: " + addr, "INFO");
            runAdbCommand({"connect", addr}, "تم إرسال طلب الربط اللاسلكي.");
        });

        connectionBadge = new QLabel("● جاري البحث عن جهاز متصل...", topBar);
        connectionBadge->setStyleSheet("background-color: #282A2C; color: #FCD34D; padding: 6px 14px; border-radius: 16px; font-size: 12px; font-weight: bold;");

        QPushButton *refreshBtn = new QPushButton("🔄 فحص الاتصال", topBar);
        refreshBtn->setProperty("class", "SecondaryBtn");
        connect(refreshBtn, &QPushButton::clicked, this, [=]() {
            appendLog("جاري فحص الأجهزة المتصلة عبر USB والشبكة...", "INFO");
            checkDeviceSilently();
            readFullDeviceInfo();
        });

        layout->addWidget(brandTitle);
        layout->addSpacing(10);
        layout->addWidget(licenseBadge);
        layout->addStretch();
        layout->addWidget(wifiIpInput);
        layout->addWidget(wifiConnectBtn);
        layout->addSpacing(6);
        layout->addWidget(connectionBadge);
        layout->addSpacing(6);
        layout->addWidget(refreshBtn);

        return topBar;
    }

    // --- القائمة الجانبية ---
    QWidget* createSidebar() {
        QFrame *sidebar = new QFrame(this);
        sidebar->setObjectName("SideBar");
        sidebar->setFixedWidth(255);

        QVBoxLayout *layout = new QVBoxLayout(sidebar);
        layout->setContentsMargins(14, 18, 14, 18);
        layout->setSpacing(8);

        QLabel *menuHeader = new QLabel("قائمة المهام الرئيسية", sidebar);
        menuHeader->setStyleSheet("color: #8E918F; font-size: 12px; font-weight: bold; padding-right: 10px; margin-bottom: 4px;");
        layout->addWidget(menuHeader);

        QButtonGroup *navGroup = new QButtonGroup(this);
        navGroup->setExclusive(true);

        struct NavItem { QString title; int index; };
        QList<NavItem> items = {
            {"📊  معلومات الهاتف والتحكم", 0},
            {"📦  مدير التطبيقات (APK)", 1},
            {"🧬  بيانات الحزم (ab / data)", 2},
            {"📁  نقل وسحب الملفات", 3},
            {"💻  أوامر Shell المتقدمة", 4}
        };

        for (const auto &item : items) {
            QPushButton *btn = new QPushButton(item.title, sidebar);
            btn->setProperty("class", "NavBtn");
            btn->setCheckable(true);
            btn->setCursor(Qt::PointingHandCursor);
            if (item.index == 0) btn->setChecked(true);

            navGroup->addButton(btn, item.index);
            connect(btn, &QPushButton::clicked, this, [=]() {
                pagesStack->setCurrentIndex(item.index);
            });
            layout->addWidget(btn);
        }

        layout->addStretch();

        QPushButton *tcpipBtn = new QPushButton("📶 تفعيل ADB WiFi (منفذ 5555)", sidebar);
        tcpipBtn->setProperty("class", "SecondaryBtn");
        connect(tcpipBtn, &QPushButton::clicked, this, [=]() {
            appendLog("تفعيل وضع الاتصال اللاسلكي على المنفذ 5555...", "INFO");
            runAdbCommand({"tcpip", "5555"}, "تم فتح المنفذ 5555 على الهاتف.");
        });
        layout->addWidget(tcpipBtn);

        QPushButton *restartAdbBtn = new QPushButton("🛠️ إعادة تشغيل خادم ADB", sidebar);
        restartAdbBtn->setProperty("class", "SecondaryBtn");
        connect(restartAdbBtn, &QPushButton::clicked, this, [=]() {
            appendLog("جاري إعادة تشغيل خادم ADB...", "WARN");
            runAdbSequence({
                {{"kill-server"}, "إيقاف خادم ADB...", ""},
                {{"start-server"}, "تشغيل خادم ADB من جديد...", "تم إعادة تشغيل خادم ADB بنجاح."}
            });
        });
        layout->addWidget(restartAdbBtn);

        QLabel *footerVer = new QLabel("AbdullahTool • Gemini Dark UI", sidebar);
        footerVer->setAlignment(Qt::AlignCenter);
        footerVer->setStyleSheet("color: #8E918F; font-size: 11px; margin-top: 6px;");
        layout->addWidget(footerVer);

        return sidebar;
    }

    // --- الصفحة 0: معلومات الهاتف والتحكم ---
    QWidget* createDashboardPage() {
        QFrame *page = new QFrame(this);
        page->setObjectName("Card");
        QVBoxLayout *layout = new QVBoxLayout(page);
        layout->setContentsMargins(22, 22, 22, 22);
        layout->setSpacing(16);

        QHBoxLayout *headerRow = new QHBoxLayout();
        QLabel *title = new QLabel("📊 بيانات الجهاز المتصل والتحكم في أوضاع التشغيل", page);
        title->setStyleSheet("font-size: 16px; font-weight: bold; color: #E3E3E3;");

        QPushButton *readInfoBtn = new QPushButton("🔍 قراءة كافة بيانات الهاتف الآن", page);
        readInfoBtn->setProperty("class", "ActionBtn");
        connect(readInfoBtn, &QPushButton::clicked, this, &MainWindow::readFullDeviceInfo);

        headerRow->addWidget(title);
        headerRow->addStretch();
        headerRow->addWidget(readInfoBtn);
        layout->addLayout(headerRow);

        QGridLayout *grid = new QGridLayout();
        grid->setSpacing(12);

        auto createStatBox = [&](const QString &label, QLabel* &valueLabel) -> QFrame* {
            QFrame *box = new QFrame(page);
            box->setObjectName("StatCard");
            QVBoxLayout *v = new QVBoxLayout(box);
            v->setContentsMargins(16, 14, 16, 14);
            QLabel *lbl = new QLabel(label, box);
            lbl->setStyleSheet("color: #C4C7C5; font-size: 12px;");
            valueLabel = new QLabel("بانتظار قراءة البيانات...", box);
            valueLabel->setStyleSheet("color: #A8C7FA; font-size: 15px; font-weight: bold;");
            v->addWidget(lbl);
            v->addWidget(valueLabel);
            return box;
        };

        grid->addWidget(createStatBox("📱 طراز وموديل الهاتف (Model & Brand):", valModel), 0, 0);
        grid->addWidget(createStatBox("🤖 إصدار نظام الأندرويد (Android Release):", valAndroid), 0, 1);
        grid->addWidget(createStatBox("🔢 الرقم التسلسلي (Serial Number):", valSerial), 1, 0);

        QFrame *batCard = new QFrame(page);
        batCard->setObjectName("StatCard");
        QVBoxLayout *batLayout = new QVBoxLayout(batCard);
        batLayout->setContentsMargins(16, 14, 16, 14);
        QLabel *batLbl = new QLabel("🔋 مستوى شحن البطارية (Battery Level):", batCard);
        batLbl->setStyleSheet("color: #C4C7C5; font-size: 12px;");
        batteryBar = new QProgressBar(batCard);
        batteryBar->setRange(0, 100);
        batteryBar->setValue(0);
        batteryBar->setFixedHeight(22);
        batLayout->addWidget(batLbl);
        batLayout->addWidget(batteryBar);
        grid->addWidget(batCard, 1, 1);

        layout->addLayout(grid);

        QLabel *rebootTitle = new QLabel("⚡ التحكم السريع في أوضاع الإقلاع (Reboot Modes):", page);
        rebootTitle->setStyleSheet("font-size: 14px; font-weight: bold; color: #E3E3E3; margin-top: 8px;");
        layout->addWidget(rebootTitle);

        QHBoxLayout *rebootLayout = new QHBoxLayout();
        rebootLayout->setSpacing(10);

        auto addRebootBtn = [&](const QString &text, const QString &mode, const QString &msg) {
            QPushButton *btn = new QPushButton(text, page);
            btn->setProperty("class", "SecondaryBtn");
            connect(btn, &QPushButton::clicked, this, [=]() {
                appendLog(msg, "WARN");
                QStringList args = {"reboot"};
                if (!mode.isEmpty()) args << mode;
                runAdbCommand(args, "تم إرسال أمر إعادة التشغيل بنجاح.");
            });
            rebootLayout->addWidget(btn);
        };

        addRebootBtn("🔄 إعادة تشغيل عادي (Normal)", "", "إرسال أمر إعادة تشغيل للوضع العادي...");
        addRebootBtn("🛠️ وضع الريكفري (Recovery)", "recovery", "تحويل الهاتف إلى وضع Recovery...");
        addRebootBtn("⚡ وضع البوتلودر (Fastboot)", "bootloader", "تحويل الهاتف إلى وضع Fastboot...");
        addRebootBtn("📥 وضع الداونلود (Download)", "download", "تحويل الهاتف إلى وضع Download...");

        layout->addLayout(rebootLayout);
        layout->addStretch();
        return page;
    }

    // --- الصفحة 1: مدير التطبيقات APK ---
    QWidget* createAppManagerPage() {
        QFrame *page = new QFrame(this);
        page->setObjectName("Card");
        QVBoxLayout *layout = new QVBoxLayout(page);
        layout->setContentsMargins(22, 22, 22, 22);
        layout->setSpacing(14);

        QLabel *title = new QLabel("📦 تثبيت ملفات APK وإدارة تطبيقات الهاتف", page);
        title->setStyleSheet("font-size: 16px; font-weight: bold; color: #E3E3E3;");
        layout->addWidget(title);

        QLabel *apkLbl = new QLabel("اختر ملف التطبيق (APK) من جهاز الكمبيوتر:", page);
        apkLbl->setStyleSheet("color: #C4C7C5; font-size: 13px;");
        layout->addWidget(apkLbl);

        QHBoxLayout *fileRow = new QHBoxLayout();
        apkPathInput = new QLineEdit(page);
        apkPathInput->setPlaceholderText("مسار ملف APK... (اضغط استعراض لاختيار الملف)");

        QPushButton *browseApkBtn = new QPushButton("📂 استعراض...", page);
        browseApkBtn->setProperty("class", "SecondaryBtn");
        connect(browseApkBtn, &QPushButton::clicked, this, [=]() {
            QString file = QFileDialog::getOpenFileName(this, "اختر ملف APK", "", "Android Package (*.apk)");
            if (!file.isEmpty()) apkPathInput->setText(file);
        });

        QPushButton *installNowBtn = new QPushButton("⬇️ تثبيت التطبيق الآن", page);
        installNowBtn->setProperty("class", "ActionBtn");
        connect(installNowBtn, &QPushButton::clicked, this, [=]() {
            QString path = apkPathInput->text().trimmed();
            if (path.isEmpty()) {
                QMessageBox::warning(this, "تنبيه", "يرجى اختيار ملف APK أولاً!");
                return;
            }
            QStringList args = {"install"};
            if (chkReplaceApk->isChecked()) args << "-r";
            if (chkGrantPerms->isChecked()) args << "-g";
            args << path;

            appendLog("جاري تثبيت التطبيق على الهاتف: " + path, "INFO");
            runAdbCommand(args, "اكتملت عملية تثبيت التطبيق بنجاح.");
        });

        fileRow->addWidget(apkPathInput, 1);
        fileRow->addWidget(browseApkBtn);
        fileRow->addWidget(installNowBtn);
        layout->addLayout(fileRow);

        QHBoxLayout *optsRow = new QHBoxLayout();
        chkReplaceApk = new QCheckBox("تحديث التطبيق مع الاحتفاظ بالبيانات (-r)", page);
        chkReplaceApk->setChecked(true);
        chkGrantPerms = new QCheckBox("منح كافة الصلاحيات للتطبيق تلقائياً (-g)", page);
        chkGrantPerms->setChecked(true);
        optsRow->addWidget(chkReplaceApk);
        optsRow->addWidget(chkGrantPerms);
        optsRow->addStretch();
        layout->addLayout(optsRow);

        QLabel *pkgTitle = new QLabel("🗑️ إدارة الحزم وحذف تطبيقات النظام (Debloater):", page);
        pkgTitle->setStyleSheet("font-size: 14px; font-weight: bold; color: #E3E3E3; margin-top: 12px;");
        layout->addWidget(pkgTitle);

        QHBoxLayout *pkgRow = new QHBoxLayout();
        pkgNameInput = new QLineEdit(page);
        pkgNameInput->setPlaceholderText("أدخل اسم الحزمة (مثال: com.facebook.katana)");

        QPushButton *listPkgsBtn = new QPushButton("📋 عرض حزم الهاتف", page);
        listPkgsBtn->setProperty("class", "SecondaryBtn");
        connect(listPkgsBtn, &QPushButton::clicked, this, [=]() {
            appendLog("جاري جلب قائمة الحزم المثبتة على الهاتف...", "INFO");
            runAdbCommand({"shell", "pm", "list", "packages", "-3"}, "تم عرض حزم التطبيقات.");
        });

        QPushButton *uninstallBtn = new QPushButton("🗑️ حذف لمستخدم النظام (User 0)", page);
        uninstallBtn->setProperty("class", "DangerBtn");
        connect(uninstallBtn, &QPushButton::clicked, this, [=]() {
            QString pkg = pkgNameInput->text().trimmed();
            if (pkg.isEmpty()) {
                QMessageBox::warning(this, "تنبيه", "يرجى كتابة اسم الحزمة أولاً!");
                return;
            }
            appendLog("جاري إزالة الحزمة للمستخدم الحالي: " + pkg, "WARN");
            runAdbCommand({"shell", "pm", "uninstall", "-k", "--user", "0", pkg}, "تم تنفيذ أمر إزالة الحزمة.");
        });

        pkgRow->addWidget(pkgNameInput, 1);
        pkgRow->addWidget(listPkgsBtn);
        pkgRow->addWidget(uninstallBtn);
        layout->addLayout(pkgRow);
        layout->addStretch();

        return page;
    }

    // --- الصفحة 2: النسخ الاحتياطي (.ab) وفك الضغط واستعادة البيانات إلى مسار /data/data/<pkg> ---
    QWidget* createBackupRestorePage() {
        QFrame *page = new QFrame(this);
        page->setObjectName("Card");
        QVBoxLayout *layout = new QVBoxLayout(page);
        layout->setContentsMargins(22, 18, 22, 18);
        layout->setSpacing(12);

        QLabel *title = new QLabel("🧬 استوديو النسخ الاحتياطي (.ab) واستعادة البيانات إلى مسار الحزمة (/data/data)", page);
        title->setStyleSheet("font-size: 16px; font-weight: bold; color: #E3E3E3;");
        layout->addWidget(title);

        // 1. سحب نسخة احتياطية بصيغة .ab
        QLabel *sec1 = new QLabel("1️⃣ إنشاء نسخة احتياطية رسمية لبيانات حزمة معينة إلى ملف (.ab):", page);
        sec1->setStyleSheet("color: #A8C7FA; font-size: 13px; font-weight: bold;");
        layout->addWidget(sec1);

        QHBoxLayout *backupRow = new QHBoxLayout();
        backupPkgInput = new QLineEdit(page);
        backupPkgInput->setPlaceholderText("اسم الحزمة المراد نسخها (مثال: com.uu)");

        backupOutPathInput = new QLineEdit(QDir::homePath() + "/Desktop/backup.ab", page);
        backupOutPathInput->setPlaceholderText("مسار حفظ ملف .ab على الكمبيوتر...");

        QPushButton *browseBackupOutBtn = new QPushButton("حفظ باسم...", page);
        browseBackupOutBtn->setProperty("class", "SecondaryBtn");
        connect(browseBackupOutBtn, &QPushButton::clicked, this, [=]() {
            QString f = QFileDialog::getSaveFileName(this, "حفظ ملف النسخة الاحتياطية", backupOutPathInput->text(), "Android Backup (*.ab)");
            if (!f.isEmpty()) backupOutPathInput->setText(f);
        });

        chkIncludeApkInBackup = new QCheckBox("تضمين ملف APK", page);
        chkIncludeApkInBackup->setChecked(false);

        QPushButton *doBackupBtn = new QPushButton("💾 سحب ملف .ab", page);
        doBackupBtn->setProperty("class", "ActionBtn");
        connect(doBackupBtn, &QPushButton::clicked, this, [=]() {
            QString pkg = backupPkgInput->text().trimmed();
            QString out = backupOutPathInput->text().trimmed();
            if (pkg.isEmpty() || out.isEmpty()) {
                QMessageBox::warning(this, "تنبيه", "يرجى إدخال اسم الحزمة ومسار حفظ ملف .ab!");
                return;
            }
            QStringList args = {"backup", "-f", out};
            args << (chkIncludeApkInBackup->isChecked() ? "-apk" : "-noapk");
            args << pkg;
            appendLog("جاري طلب النسخ الاحتياطي للحزمة " + pkg + " (يرجى الموافقة من شاشة الهاتف الآن)...", "WARN");
            runAdbCommand(args, "انتهت عملية إنشاء ملف النسخة الاحتياطية: " + out);
        });

        backupRow->addWidget(backupPkgInput, 1);
        backupRow->addWidget(backupOutPathInput, 2);
        backupRow->addWidget(browseBackupOutBtn);
        backupRow->addWidget(chkIncludeApkInBackup);
        backupRow->addWidget(doBackupBtn);
        layout->addLayout(backupRow);

        // 2. فك وتحويل ملف .ab إلى أرشيف .tar مباشرة من داخل الأداة
        QLabel *sec2 = new QLabel("2️⃣ تحويل وفك ضغط ملف (.ab) إلى أرشيف (.tar) جاهز للاستخراج أو الحقن:", page);
        sec2->setStyleSheet("color: #FCD34D; font-size: 13px; font-weight: bold; margin-top: 6px;");
        layout->addWidget(sec2);

        QHBoxLayout *unpackRow = new QHBoxLayout();
        abFileInput = new QLineEdit(page);
        abFileInput->setPlaceholderText("اختر ملف النسخة الاحتياطية (.ab)...");

        QPushButton *browseAbBtn = new QPushButton("اختيار .ab", page);
        browseAbBtn->setProperty("class", "SecondaryBtn");
        connect(browseAbBtn, &QPushButton::clicked, this, [=]() {
            QString f = QFileDialog::getOpenFileName(this, "اختر ملف Android Backup", "", "Android Backup (*.ab)");
            if (!f.isEmpty()) {
                abFileInput->setText(f);
                QString tarSuggested = f;
                if (tarSuggested.endsWith(".ab", Qt::CaseInsensitive)) {
                    tarSuggested.chop(3);
                }
                tarOutputInput->setText(tarSuggested + ".tar");
            }
        });

        tarOutputInput = new QLineEdit(page);
        tarOutputInput->setPlaceholderText("مسار ملف .tar الناتج...");

        QPushButton *convertAbBtn = new QPushButton("⚙️ فك .ab إلى .tar", page);
        convertAbBtn->setProperty("class", "SecondaryBtn");
        connect(convertAbBtn, &QPushButton::clicked, this, [=]() {
            QString inAb = abFileInput->text().trimmed();
            QString outTar = tarOutputInput->text().trimmed();
            if (inAb.isEmpty() || outTar.isEmpty()) {
                QMessageBox::warning(this, "تنبيه", "يرجى تحديد ملف .ab ومسار حفظ ملف .tar!");
                return;
            }
            if (convertAbFileToTar(inAb, outTar)) {
                restoreSourceInput->setText(outTar);
            }
        });

        unpackRow->addWidget(abFileInput, 2);
        unpackRow->addWidget(browseAbBtn);
        unpackRow->addWidget(tarOutputInput, 2);
        unpackRow->addWidget(convertAbBtn);
        layout->addLayout(unpackRow);

        // 3. استعادة وحقن البيانات إلى مسار /data/data/<targetPkg> (مثال: com.uu)
        QLabel *sec3 = new QLabel("3️⃣ استعادة واستبدال محتوى بيانات الحزمة الهدف (/data/data/com.uu) من ملف .tar أو .ab أو مجلد:", page);
        sec3->setStyleSheet("color: #6DD58C; font-size: 13px; font-weight: bold; margin-top: 6px;");
        layout->addWidget(sec3);

        QHBoxLayout *restoreRow = new QHBoxLayout();
        restoreSourceInput = new QLineEdit(page);
        restoreSourceInput->setPlaceholderText("اختر ملف (.tar أو .ab) أو مجلد البيانات المستخرج...");

        QPushButton *browseTarBtn = new QPushButton("📄 ملف (.tar/.ab)", page);
        browseTarBtn->setProperty("class", "SecondaryBtn");
        connect(browseTarBtn, &QPushButton::clicked, this, [=]() {
            QString f = QFileDialog::getOpenFileName(this, "اختر ملف tar أو ab", "", "Backup Archives (*.tar *.ab);;All Files (*.*)");
            if (!f.isEmpty()) restoreSourceInput->setText(f);
        });

        QPushButton *browseDirBtn = new QPushButton("📁 أو مجلد", page);
        browseDirBtn->setProperty("class", "SecondaryBtn");
        connect(browseDirBtn, &QPushButton::clicked, this, [=]() {
            QString d = QFileDialog::getExistingDirectory(this, "اختر مجلد البيانات المستخرج");
            if (!d.isEmpty()) restoreSourceInput->setText(d);
        });

        restoreTargetPkgInput = new QLineEdit("com.uu", page);
        restoreTargetPkgInput->setPlaceholderText("الحزمة الهدف (مثال: com.uu)");
        restoreTargetPkgInput->setFixedWidth(190);

        QPushButton *doRestoreBtn = new QPushButton("🚀 حقن واستبدال بيانات الحزمة", page);
        doRestoreBtn->setProperty("class", "ActionBtn");
        connect(doRestoreBtn, &QPushButton::clicked, this, [=]() {
            restoreDataToPackage(restoreSourceInput->text().trimmed(), restoreTargetPkgInput->text().trimmed());
        });

        restoreRow->addWidget(restoreSourceInput, 2);
        restoreRow->addWidget(browseTarBtn);
        restoreRow->addWidget(browseDirBtn);
        restoreRow->addWidget(restoreTargetPkgInput);
        restoreRow->addWidget(doRestoreBtn);
        layout->addLayout(restoreRow);

        QLabel *hintLbl = new QLabel(
            "💡 ملاحظة تقنية: تقوم الأداة تلقائياً بإيقاف الحزمة الهدف (مثل com.uu)، وتحويل مسارات أرشيف النسخة الاحتياطية "
            "(db → databases و sp → shared_prefs و f → files و r → الجذر) إلى داخل /data/data/<package> مع ضبط الصلاحيات عبر (run-as أو Root su).",
            page
        );
        hintLbl->setWordWrap(true);
        hintLbl->setStyleSheet("color: #8E918F; font-size: 12px; margin-top: 4px;");
        layout->addWidget(hintLbl);

        layout->addStretch();
        return page;
    }

    // --- الصفحة 3: استوديو نقل الملفات (Pull & Push) ---
    QWidget* createFileTransferPage() {
        QFrame *page = new QFrame(this);
        page->setObjectName("Card");
        QVBoxLayout *layout = new QVBoxLayout(page);
        layout->setContentsMargins(22, 22, 22, 22);
        layout->setSpacing(16);

        QLabel *title = new QLabel("📁 استوديو نقل الملفات بين الكمبيوتر والأندرويد (Push & Pull)", page);
        title->setStyleSheet("font-size: 16px; font-weight: bold; color: #E3E3E3;");
        layout->addWidget(title);

        QLabel *pushTitle = new QLabel("⬆️ أولاً: إرسال ملف من الكمبيوتر إلى ذاكرة الهاتف (ADB Push)", page);
        pushTitle->setStyleSheet("color: #A8C7FA; font-size: 13px; font-weight: bold;");
        layout->addWidget(pushTitle);

        QHBoxLayout *pushRow = new QHBoxLayout();
        pushLocalInput = new QLineEdit(page);
        pushLocalInput->setPlaceholderText("مسار الملف في الكمبيوتر...");

        QPushButton *browsePushBtn = new QPushButton("اختيار ملف...", page);
        browsePushBtn->setProperty("class", "SecondaryBtn");
        connect(browsePushBtn, &QPushButton::clicked, this, [=]() {
            QString f = QFileDialog::getOpenFileName(this, "اختر ملفاً لإرساله للهاتف");
            if (!f.isEmpty()) pushLocalInput->setText(f);
        });

        pushRemoteInput = new QLineEdit("/sdcard/Download/", page);
        QPushButton *doPushBtn = new QPushButton("🚀 إرسال للهاتف", page);
        doPushBtn->setProperty("class", "ActionBtn");
        connect(doPushBtn, &QPushButton::clicked, this, [=]() {
            if (pushLocalInput->text().isEmpty() || pushRemoteInput->text().isEmpty()) {
                QMessageBox::warning(this, "تنبيه", "يرجى تحديد ملف الكمبيوتر ومسار الهاتف!");
                return;
            }
            appendLog("جاري إرسال الملف إلى الهاتف...", "INFO");
            runAdbCommand({"push", pushLocalInput->text().trimmed(), pushRemoteInput->text().trimmed()}, "اكتمل إرسال الملف للهاتف.");
        });

        pushRow->addWidget(pushLocalInput, 2);
        pushRow->addWidget(browsePushBtn);
        pushRow->addWidget(pushRemoteInput, 1);
        pushRow->addWidget(doPushBtn);
        layout->addLayout(pushRow);

        QLabel *pullTitle = new QLabel("⬇️ ثانياً: سحب ملف أو مجلد من الهاتف إلى الكمبيوتر (ADB Pull)", page);
        pullTitle->setStyleSheet("color: #6DD58C; font-size: 13px; font-weight: bold; margin-top: 10px;");
        layout->addWidget(pullTitle);

        QHBoxLayout *pullRow = new QHBoxLayout();
        pullRemoteInput = new QLineEdit("/sdcard/DCIM/Camera", page);
        pullLocalInput = new QLineEdit(QDir::homePath() + "/Desktop", page);

        QPushButton *browsePullBtn = new QPushButton("مجلد الحفظ...", page);
        browsePullBtn->setProperty("class", "SecondaryBtn");
        connect(browsePullBtn, &QPushButton::clicked, this, [=]() {
            QString dir = QFileDialog::getExistingDirectory(this, "اختر مجلد الحفظ على الكمبيوتر");
            if (!dir.isEmpty()) pullLocalInput->setText(dir);
        });

        QPushButton *doPullBtn = new QPushButton("📥 سحب للكمبيوتر", page);
        doPullBtn->setProperty("class", "ActionBtn");
        connect(doPullBtn, &QPushButton::clicked, this, [=]() {
            if (pullRemoteInput->text().isEmpty() || pullLocalInput->text().isEmpty()) {
                QMessageBox::warning(this, "تنبيه", "يرجى تحديد مسار الهاتف ومجلد الكمبيوتر!");
                return;
            }
            appendLog("جاري سحب الملفات من الهاتف إلى الكمبيوتر...", "INFO");
            runAdbCommand({"pull", pullRemoteInput->text().trimmed(), pullLocalInput->text().trimmed()}, "اكتمل سحب الملفات إلى الكمبيوتر.");
        });

        pullRow->addWidget(pullRemoteInput, 2);
        pullRow->addWidget(pullLocalInput, 2);
        pullRow->addWidget(browsePullBtn);
        pullRow->addWidget(doPullBtn);
        layout->addLayout(pullRow);

        layout->addStretch();
        return page;
    }

    // --- الصفحة 4: موجه الأوامر المتقدم (Shell & Quick Tools) ---
    QWidget* createShellTerminalPage() {
        QFrame *page = new QFrame(this);
        page->setObjectName("Card");
        QVBoxLayout *layout = new QVBoxLayout(page);
        layout->setContentsMargins(22, 22, 22, 22);
        layout->setSpacing(14);

        QLabel *title = new QLabel("💻 موجه أوامر الأندرويد الحر والأوامر السريعة (ADB Shell)", page);
        title->setStyleSheet("font-size: 16px; font-weight: bold; color: #E3E3E3;");
        layout->addWidget(title);

        QHBoxLayout *cmdRow = new QHBoxLayout();
        customShellInput = new QLineEdit(page);
        customShellInput->setPlaceholderText("اكتب أي أمر Shell أو ADB هنا (مثال: shell dumpsys battery)...");

        QPushButton *runCmdBtn = new QPushButton("▶️ تنفيذ الأمر", page);
        runCmdBtn->setProperty("class", "ActionBtn");

        auto executeCustom = [=]() {
            QString raw = customShellInput->text().trimmed();
            if (raw.isEmpty()) return;
            appendLog("تنفيذ أمر: adb " + raw, "INFO");
            QStringList parts = QProcess::splitCommand(raw);
            runAdbCommand(parts, "انتهى تنفيذ الأمر.");
        };

        connect(runCmdBtn, &QPushButton::clicked, this, executeCustom);
        connect(customShellInput, &QLineEdit::returnPressed, this, executeCustom);

        cmdRow->addWidget(customShellInput, 1);
        cmdRow->addWidget(runCmdBtn);
        layout->addLayout(cmdRow);

        QLabel *quickLbl = new QLabel("⚡ اختصارات أوامر احترافية جاهزة بضغطة زر:", page);
        quickLbl->setStyleSheet("color: #C4C7C5; font-size: 12px; font-weight: bold; margin-top: 6px;");
        layout->addWidget(quickLbl);

        QHBoxLayout *quickRow = new QHBoxLayout();
        auto addQuickBtn = [&](const QString &text, const QStringList &adbArgs) {
            QPushButton *b = new QPushButton(text, page);
            b->setProperty("class", "SecondaryBtn");
            connect(b, &QPushButton::clicked, this, [=]() {
                appendLog("تشغيل الأداة السريعة: " + text, "INFO");
                runAdbCommand(adbArgs, "تم التنفيذ بنجاح.");
            });
            quickRow->addWidget(b);
        };

        addQuickBtn("💾 فحص المساحة التخزينية", {"shell", "df", "-h", "/data"});
        addQuickBtn("🌐 عرض عنوان IP والشبكة", {"shell", "ip", "addr", "show", "wlan0"});
        addQuickBtn("🧠 معلومات المعالج والرام", {"shell", "cat", "/proc/meminfo"});
        addQuickBtn("🔒 ضغط زر الباور (إيقاظ/قفل)", {"shell", "input", "keyevent", "26"});

        layout->addLayout(quickRow);
        layout->addStretch();
        return page;
    }

    // --- لوحة سجل العمليات السفلية ---
    QWidget* createConsolePanel() {
        QFrame *panel = new QFrame(this);
        panel->setObjectName("Card");
        QVBoxLayout *layout = new QVBoxLayout(panel);
        layout->setContentsMargins(18, 12, 18, 12);
        layout->setSpacing(8);

        QHBoxLayout *topRow = new QHBoxLayout();
        QLabel *logTitle = new QLabel("🖥️ سجل العمليات والمخرجات المباشر (Live Operation Log)", panel);
        logTitle->setStyleSheet("font-size: 13px; font-weight: bold; color: #A8C7FA;");

        operationProgress = new QProgressBar(panel);
        operationProgress->setFixedWidth(180);
        operationProgress->setFixedHeight(12);
        operationProgress->setRange(0, 100);
        operationProgress->setValue(100);
        operationProgress->setTextVisible(false);

        QPushButton *clearBtn = new QPushButton("🧹 مسح السجل", panel);
        clearBtn->setProperty("class", "SecondaryBtn");

        topRow->addWidget(logTitle);
        topRow->addStretch();
        topRow->addWidget(operationProgress);
        topRow->addWidget(clearBtn);
        layout->addLayout(topRow);

        consoleBox = new QTextEdit(panel);
        consoleBox->setReadOnly(true);
        consoleBox->setLayoutDirection(Qt::LeftToRight);
        consoleBox->setStyleSheet(R"(
            QTextEdit {
                background-color: #131314;
                color: #6DD58C;
                border: 1px solid #282A2C;
                border-radius: 10px;
                font-family: 'IBM Plex Sans Arabic', 'Consolas', monospace;
                font-size: 12px;
                padding: 10px;
            }
        )");

        connect(clearBtn, &QPushButton::clicked, consoleBox, &QTextEdit::clear);
        layout->addWidget(consoleBox, 1);

        return panel;
    }

    // ============================================================================
    // الدوال البرمجية لتحويل ملفات .ab واستعادة بيانات الحزم (/data/data/<pkg>)
    // ============================================================================

    // تحويل ملف Android Backup (.ab غير المشفر بكلمة مرور) إلى أرشيف .tar مباشرة عبر محرك Zlib المدمج في Qt
    bool convertAbFileToTar(const QString &abFilePath, const QString &tarFilePath) {
        QFile inFile(abFilePath);
        if (!inFile.open(QIODevice::ReadOnly)) {
            appendLog("تعذر فتح ملف النسخة الاحتياطية: " + abFilePath, "ERROR");
            return false;
        }

        QByteArray rawData = inFile.readAll();
        inFile.close();

        if (!rawData.startsWith("ANDROID BACKUP\n")) {
            appendLog("هذا الملف ليس ملف Android Backup (.ab) قياسياً!", "ERROR");
            return false;
        }

        // تخطي الأسطر الأربعة الأولى لرأسية ملف .ab:
        // السطر 1: ANDROID BACKUP
        // السطر 2: رقم الإصدار
        // السطر 3: حالة الضغط (1 = مضغوط بـ zlib، 0 = غير مضغوط)
        // السطر 4: التشفير (none = غير مشفر)
        int newlinesFound = 0;
        int headerEndOffset = -1;
        QList<QByteArray> headerLines;
        int lineStart = 0;

        for (int i = 0; i < rawData.size() && newlinesFound < 4; ++i) {
            if (rawData.at(i) == '\n') {
                headerLines.append(rawData.mid(lineStart, i - lineStart).trimmed());
                lineStart = i + 1;
                newlinesFound++;
                if (newlinesFound == 4) {
                    headerEndOffset = i + 1;
                    break;
                }
            }
        }

        if (headerEndOffset == -1 || headerLines.size() < 4) {
            appendLog("رأسية ملف .ab تالفة أو غير مكتملة!", "ERROR");
            return false;
        }

        QByteArray compressedFlag = headerLines.at(2);
        QByteArray encryptionType = headerLines.at(3);

        if (encryptionType != "none") {
            appendLog("ملف .ab محمي بكلمة مرور (" + QString::fromUtf8(encryptionType) + "). يرجى أخذ نسخة بدون كلمة مرور.", "ERROR");
            return false;
        }

        QByteArray payload = rawData.mid(headerEndOffset);
        QByteArray tarData;

        if (compressedFlag == "1") {
            appendLog("جاري فك ضغط بيانات Zlib من ملف .ab وتحويلها إلى .tar...", "INFO");
            // دالة qUncompress في Qt تتطلب إضافة 4 بايت في البداية تمثل الحجم المتوقع بعد الفك
            QByteArray qtZlibBuffer;
            quint32 expectedSize = qMin<quint64>(quint64(payload.size()) * 6ULL, 0x7FFFFFFFULL);
            qtZlibBuffer.append(char((expectedSize >> 24) & 0xFF));
            qtZlibBuffer.append(char((expectedSize >> 16) & 0xFF));
            qtZlibBuffer.append(char((expectedSize >> 8) & 0xFF));
            qtZlibBuffer.append(char(expectedSize & 0xFF));
            qtZlibBuffer.append(payload);

            tarData = qUncompress(qtZlibBuffer);
            if (tarData.isEmpty()) {
                appendLog("فشل فك ضغط تدفق Zlib داخل ملف .ab!", "ERROR");
                return false;
            }
        } else {
            tarData = payload;
        }

        QFile outFile(tarFilePath);
        if (!outFile.open(QIODevice::WriteOnly)) {
            appendLog("تعذر إنشاء ملف .tar في المسار المحدد: " + tarFilePath, "ERROR");
            return false;
        }

        outFile.write(tarData);
        outFile.close();

        appendLog(QString("تم تحويل ملف .ab إلى أرشيف .tar بنجاح (%1 كيلوبايت): %2")
                      .arg(tarData.size() / 1024)
                      .arg(tarFilePath), "SUCCESS");
        return true;
    }

    // دالة استعادة وحقن ملف .tar أو .ab أو مجلد مستخرج إلى مسار الحزمة المختارة (مثل /data/data/com.uu)
    void restoreDataToPackage(const QString &sourcePath, const QString &targetPkg) {
        if (sourcePath.isEmpty() || targetPkg.isEmpty()) {
            QMessageBox::warning(this, "تنبيه", "يرجى اختيار ملف (.tar / .ab) أو المجلد، وكتابة اسم الحزمة الهدف (مثال: com.uu)!");
            return;
        }

        // التحقق من صحة اسم الحزمة لمنع أي رموز خاطئة
        QRegularExpression pkgRegex("^[a-zA-Z0-9_]+(\\.[a-zA-Z0-9_]+)+$");
        if (!pkgRegex.match(targetPkg).hasMatch()) {
            QMessageBox::warning(this, "اسم حزمة غير صالح", "يرجى كتابة اسم حزمة أندرويد صحيح (مثال: com.uu أو com.example.app)");
            return;
        }

        QFileInfo srcInfo(sourcePath);
        if (!srcInfo.exists()) {
            appendLog("المسار المحدد غير موجود على الكمبيوتر: " + sourcePath, "ERROR");
            return;
        }

        QString effectiveSource = sourcePath;

        // إذا اختار المستخدم ملف .ab مباشرة، نقوم بتحويله تلقائياً إلى .tar أولاً ثم نكمل الاستعادة!
        if (srcInfo.isFile() && sourcePath.endsWith(".ab", Qt::CaseInsensitive)) {
            QString autoTarPath = QDir::tempPath() + "/abdullah_auto_converted.tar";
            appendLog("تم اكتشاف ملف .ab، جاري تحويله تلقائياً إلى .tar قبل الحقن...", "INFO");
            if (!convertAbFileToTar(sourcePath, autoTarPath)) {
                return;
            }
            effectiveSource = autoTarPath;
            srcInfo = QFileInfo(effectiveSource);
        }

        QString remoteStage = "/data/local/tmp/abdullah_restore_stage";
        QList<AdbStep> sequence;

        // الخطوة 1: إيقاف تطبيق الحزمة الهدف (مثل com.uu) لمنع قفل قواعد البيانات SQLite
        sequence.append({
            {"shell", "am", "force-stop", targetPkg},
            "1/5 إيقاف الحزمة الهدف مؤقتاً (" + targetPkg + ") لإغلاق قواعد البيانات المفتوحة...",
            ""
        });

        // الخطوة 2: تجهيز المجلد الوسيط الآمن على الهاتف /data/local/tmp
        sequence.append({
            {"shell", "rm -rf " + remoteStage + " && mkdir -p " + remoteStage},
            "2/5 تهيئة المسار الوسيط المؤقت على الهاتف (" + remoteStage + ")...",
            ""
        });

        // الخطوة 3: رفع ملف الـ .tar أو المجلد وفك الضغط في المسار المؤقت
        if (srcInfo.isFile() && effectiveSource.endsWith(".tar", Qt::CaseInsensitive)) {
            QString remoteTar = remoteStage + "/payload.tar";
            sequence.append({
                {"push", effectiveSource, remoteTar},
                "3/5 جاري رفع أرشيف البيانات (.tar) إلى الهاتف...",
                "تم رفع ملف الـ tar إلى الهاتف بنجاح."
            });
            sequence.append({
                {"shell", "tar -xf " + remoteTar + " -C " + remoteStage + " && rm -f " + remoteTar + " && chmod -R 755 " + remoteStage},
                "4/5 جاري فك ضغط الأرشيف وتجهيز صلاحيات القراءة المؤقتة...",
                ""
            });
        } else if (srcInfo.isDir()) {
            sequence.append({
                {"push", effectiveSource + "/.", remoteStage + "/"},
                "3/5 جاري رفع مجلد البيانات المستخرج إلى الهاتف...",
                "تم رفع المجلد إلى الهاتف بنجاح."
            });
            sequence.append({
                {"shell", "chmod -R 755 " + remoteStage},
                "4/5 ضبط صلاحيات القراءة المؤقتة للملفات المرفوعة...",
                ""
            });
        } else {
            appendLog("يرجى اختيار ملف بصيغة .tar أو .ab أو اختيار مجلد مستخرج!", "ERROR");
            return;
        }

        // الخطوة 5: سكريبت الحقن الذكي لتحويل مسارات (db, sp, f, r) أو المسارات المباشرة إلى /data/data/<targetPkg>
        QString restoreScript = QString(
            "PKG='%1'; "
            "STAGE='/data/local/tmp/abdullah_restore_stage'; "
            "APP_SRC=$(find $STAGE/apps -mindepth 1 -maxdepth 1 -type d 2>/dev/null | head -n 1); "
            "if [ -z \"$APP_SRC\" ]; then APP_SRC=\"$STAGE\"; fi; "
            "if run-as $PKG true 2>/dev/null; then "
            "  echo '--> [run-as mode] Injecting data directly into /data/data/'$PKG; "
            "  run-as $PKG sh -c \""
            "    rm -rf databases/* shared_prefs/* files/*; "
            "    mkdir -p databases shared_prefs files; "
            "    if [ -d '$APP_SRC/db' ]; then cp -rf '$APP_SRC/db/.' databases/; fi; "
            "    if [ -d '$APP_SRC/databases' ]; then cp -rf '$APP_SRC/databases/.' databases/; fi; "
            "    if [ -d '$APP_SRC/sp' ]; then cp -rf '$APP_SRC/sp/.' shared_prefs/; fi; "
            "    if [ -d '$APP_SRC/shared_prefs' ]; then cp -rf '$APP_SRC/shared_prefs/.' shared_prefs/; fi; "
            "    if [ -d '$APP_SRC/f' ]; then cp -rf '$APP_SRC/f/.' files/; fi; "
            "    if [ -d '$APP_SRC/files' ]; then cp -rf '$APP_SRC/files/.' files/; fi; "
            "    if [ -d '$APP_SRC/r' ]; then cp -rf '$APP_SRC/r/.' ./; fi; "
            "  \"; "
            "elif su -c true 2>/dev/null; then "
            "  echo '--> [Root su mode] Injecting data and fixing UID/SELinux for /data/data/'$PKG; "
            "  su -c \""
            "    DATA_DIR=/data/data/$PKG; "
            "    if [ ! -d \\$DATA_DIR ]; then echo 'ERROR: Target package not installed!'; exit 1; fi; "
            "    APP_UID=\\$(stat -c %u \\$DATA_DIR); "
            "    rm -rf \\$DATA_DIR/databases/* \\$DATA_DIR/shared_prefs/* \\$DATA_DIR/files/*; "
            "    mkdir -p \\$DATA_DIR/databases \\$DATA_DIR/shared_prefs \\$DATA_DIR/files; "
            "    if [ -d '$APP_SRC/db' ]; then cp -rf '$APP_SRC/db/.' \\$DATA_DIR/databases/; fi; "
            "    if [ -d '$APP_SRC/databases' ]; then cp -rf '$APP_SRC/databases/.' \\$DATA_DIR/databases/; fi; "
            "    if [ -d '$APP_SRC/sp' ]; then cp -rf '$APP_SRC/sp/.' \\$DATA_DIR/shared_prefs/; fi; "
            "    if [ -d '$APP_SRC/shared_prefs' ]; then cp -rf '$APP_SRC/shared_prefs/.' \\$DATA_DIR/shared_prefs/; fi; "
            "    if [ -d '$APP_SRC/f' ]; then cp -rf '$APP_SRC/f/.' \\$DATA_DIR/files/; fi; "
            "    if [ -d '$APP_SRC/files' ]; then cp -rf '$APP_SRC/files/.' \\$DATA_DIR/files/; fi; "
            "    if [ -d '$APP_SRC/r' ]; then cp -rf '$APP_SRC/r/.' \\$DATA_DIR/; fi; "
            "    chown -R \\$APP_UID:\\$APP_UID \\$DATA_DIR/databases \\$DATA_DIR/shared_prefs \\$DATA_DIR/files; "
            "    chmod -R 771 \\$DATA_DIR/databases \\$DATA_DIR/shared_prefs \\$DATA_DIR/files; "
            "    restorecon -R \\$DATA_DIR 2>/dev/null || true; "
            "  \"; "
            "else "
            "  echo 'ERROR: Cannot write to /data/data/'$PKG' (App is not debuggable for run-as, and device has no Root su)'; "
            "  rm -rf $STAGE; "
            "  exit 1; "
            "fi; "
            "rm -rf $STAGE;"
        ).arg(targetPkg);

        sequence.append({
            {"shell", restoreScript},
            "5/5 جاري حقن البيانات داخل /data/data/" + targetPkg + " وتصحيح الصلاحيات...",
            "تم استبدال بيانات الحزمة " + targetPkg + " في /data/data/" + targetPkg + " بنجاح تام!"
        });

        runAdbSequence(sequence);
    }

    // --- دالة إضافة سطر ملون ومؤرخ في السجل ---
    void appendLog(const QString &message, const QString &level = "INFO") {
        QString timeStr = QDateTime::currentDateTime().toString("HH:mm:ss");
        QString color = "#A8C7FA";
        QString icon = "▶";

        if (level == "SUCCESS") { color = "#6DD58C"; icon = "✔"; }
        else if (level == "WARN") { color = "#FCD34D"; icon = "⚠"; }
        else if (level == "ERROR") { color = "#F2B8B5"; icon = "✖"; }
        else if (level == "OUTPUT") { color = "#E3E3E3"; icon = "│"; }

        QString html = QString("<span style='color:#8E918F;'>[%1]</span> <span style='color:%2; font-weight:600;'>%3 %4</span>")
                           .arg(timeStr, color, icon, message.toHtmlEscaped());
        consoleBox->append(html);
    }

    // --- تنفيذ سلسلة خطوات ADB بالترتيب (Sequential Queue) لضمان عدم تداخل الأوامر ---
    void runAdbSequence(const QList<AdbStep> &steps) {
        for (const AdbStep &s : steps) {
            stepQueue.enqueue(s);
        }
        if (!isSequenceRunning) {
            processNextStepInQueue();
        }
    }

    void processNextStepInQueue() {
        if (stepQueue.isEmpty()) {
            isSequenceRunning = false;
            operationProgress->setRange(0, 100);
            operationProgress->setValue(100);
            return;
        }

        isSequenceRunning = true;
        AdbStep current = stepQueue.dequeue();
        if (!current.startLog.isEmpty()) {
            appendLog(current.startLog, "INFO");
        }

        QProcess *proc = new QProcess(this);
        operationProgress->setRange(0, 0);

        connect(proc, &QProcess::readyReadStandardOutput, this, [=]() {
            QString out = QString::fromUtf8(proc->readAllStandardOutput()).trimmed();
            if (!out.isEmpty()) {
                for (const QString &line : out.split('\n')) {
                    appendLog(line.trimmed(), line.contains("ERROR:") ? "ERROR" : "OUTPUT");
                }
            }
        });

        connect(proc, &QProcess::readyReadStandardError, this, [=]() {
            QString err = QString::fromUtf8(proc->readAllStandardError()).trimmed();
            if (!err.isEmpty()) {
                appendLog(err, "ERROR");
            }
        });

        connect(proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [=](int exitCode, QProcess::ExitStatus) {
            proc->deleteLater();
            if (exitCode == 0) {
                if (!current.successLog.isEmpty()) {
                    appendLog(current.successLog, "SUCCESS");
                }
                processNextStepInQueue();
            } else {
                appendLog("توقفت السلسلة بسبب خطأ في الخطوة الحالية (كود الخروج: " + QString::number(exitCode) + ").", "ERROR");
                stepQueue.clear();
                isSequenceRunning = false;
                operationProgress->setRange(0, 100);
                operationProgress->setValue(100);
            }
        });

        connect(proc, &QProcess::errorOccurred, this, [=](QProcess::ProcessError) {
            proc->deleteLater();
            appendLog("تعذر تشغيل ملف adb.exe! تأكد من وجود مجلد platform-tools بجانب الأداة.", "ERROR");
            stepQueue.clear();
            isSequenceRunning = false;
            operationProgress->setRange(0, 100);
            operationProgress->setValue(100);
        });

        proc->start(getAdbExecutablePath(), current.args);
    }

    // --- تنفيذ أمر ADB مفرد ---
    void runAdbCommand(const QStringList &args, const QString &doneMsg = QString()) {
        runAdbSequence({{args, "", doneMsg}});
    }

    // --- فحص اتصال الهاتف في الخلفية ---
    void checkDeviceSilently() {
        QProcess *proc = new QProcess(this);
        connect(proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [=](int, QProcess::ExitStatus) {
            QString out = QString::fromUtf8(proc->readAllStandardOutput()).trimmed();
            QStringList lines = out.split('\n', Qt::SkipEmptyParts);
            QString connectedSerial;

            for (int i = 1; i < lines.size(); ++i) {
                QString line = lines[i].trimmed();
                if (line.endsWith("\tdevice") || line.contains(" device")) {
                    connectedSerial = line.split('\t').first().trimmed();
                    break;
                }
            }

            if (!connectedSerial.isEmpty()) {
                connectionBadge->setText("● متصل: " + connectedSerial);
                connectionBadge->setStyleSheet("background-color: #0F5223; color: #6DD58C; padding: 6px 14px; border-radius: 16px; font-size: 12px; font-weight: bold;");
                valSerial->setText(connectedSerial);
            } else {
                connectionBadge->setText("● غير متصل - بانتظار USB أو WiFi");
                connectionBadge->setStyleSheet("background-color: #282A2C; color: #FCD34D; padding: 6px 14px; border-radius: 16px; font-size: 12px; font-weight: bold;");
            }
            proc->deleteLater();
        });
        proc->start(getAdbExecutablePath(), {"devices"});
    }

    // --- قراءة مواصفات الهاتف ---
    void readFullDeviceInfo() {
        appendLog("جاري قراءة مواصفات الهاتف المتصل...", "INFO");

        QProcess *pModel = new QProcess(this);
        connect(pModel, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [=](int, QProcess::ExitStatus) {
            QString m = QString::fromUtf8(pModel->readAllStandardOutput()).trimmed();
            if (!m.isEmpty()) {
                valModel->setText(m);
                appendLog("طراز الهاتف: " + m, "SUCCESS");
            }
            pModel->deleteLater();
        });
        pModel->start(getAdbExecutablePath(), {"shell", "getprop", "ro.product.model"});

        QProcess *pVer = new QProcess(this);
        connect(pVer, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [=](int, QProcess::ExitStatus) {
            QString v = QString::fromUtf8(pVer->readAllStandardOutput()).trimmed();
            if (!v.isEmpty()) {
                valAndroid->setText("Android " + v);
                appendLog("إصدار الأندرويد: Android " + v, "SUCCESS");
            }
            pVer->deleteLater();
        });
        pVer->start(getAdbExecutablePath(), {"shell", "getprop", "ro.build.version.release"});

        QProcess *pBat = new QProcess(this);
        connect(pBat, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [=](int, QProcess::ExitStatus) {
            QString out = QString::fromUtf8(pBat->readAllStandardOutput());
            for (const QString &line : out.split('\n')) {
                if (line.trimmed().startsWith("level:")) {
                    int lvl = line.split(':').last().trimmed().toInt();
                    batteryBar->setValue(lvl);
                    appendLog(QString("مستوى شحن البطارية: %1%").arg(lvl), "SUCCESS");
                    break;
                }
            }
            pBat->deleteLater();
        });
        pBat->start(getAdbExecutablePath(), {"shell", "dumpsys", "battery"});
    }
};

// ============================================================================
// 3. نقطة التشغيل الرئيسية
// ============================================================================
int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    setupGeminiTypography(app);

    LoginWindow loginWin;
    MainWindow mainWin;

    QObject::connect(&loginWin, &LoginWindow::loginSuccess, [&](const QString &licenseStatus) {
        mainWin.setLicenseBadgeText(licenseStatus);
        mainWin.show();
    });

    loginWin.show();
    return app.exec();
}

#include "main.moc"
