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
#include <QTextStream>
#include <QRegularExpression>
#include <QFont>
#include <QFontDatabase>

// دالة لتحميل خط جوجل العربي المدمج وتطبيق تنعيم الحروف المطابق لواجهة Gemini
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

// دالة لتوليد بصمة الجهاز الفريدة (HWID) الخاصة بالكمبيوتر
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

// دالة لتحديد مسار ملف adb.exe المدمج بجانب الأداة
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
            "تنفيذ أوامر النظام المتقدمة، إدارة حزم البيانات وملفات النسخ الاحتياطي، ونقل الملفات السريع عبر السلك واللاسلكي.",
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
    MainWindow(QWidget *parent = nullptr) : QMainWindow(parent) {
        setWindowTitle("AbdullahTool Pro v2.2 - النظام المتكامل لإدارة أجهزة الأندرويد");
        resize(1160, 760);
        setMinimumSize(1000, 680);
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
                padding: 10px 14px;
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
        pagesStack->addWidget(createDashboardPage());      // 0: معلومات الهاتف والربط اللاسلكي
        pagesStack->addWidget(createAppManagerPage());     // 1: مدير التطبيقات
        pagesStack->addWidget(createFileTransferPage());   // 2: نقل وسحب الملفات
        pagesStack->addWidget(createBackupAndDataPage());  // 3: النسخ الاحتياطي (.ab) وبيانات الحزم
        pagesStack->addWidget(createShellTerminalPage());  // 4: أوامر Shell المتقدمة

        workAreaLayout->addWidget(pagesStack, 3);
        workAreaLayout->addWidget(createConsolePanel(), 2);

        bodyLayout->addLayout(workAreaLayout, 1);
        rootLayout->addLayout(bodyLayout, 1);

        deviceTimer = new QTimer(this);
        connect(deviceTimer, &QTimer::timeout, this, &MainWindow::checkDeviceSilently);
        deviceTimer->start(3500);

        appendLog("تم تشغيل AbdullahTool Pro v2.2 بواجهة وخط Gemini بنجاح.", "SUCCESS");
        checkDeviceSilently();
    }

    void setLicenseBadgeText(const QString &text) {
        licenseBadge->setText("🛡️ " + text);
    }

private:
    QLabel *licenseBadge;
    QLabel *connectionBadge;
    QStackedWidget *pagesStack;
    QTextEdit *consoleBox;
    QProgressBar *operationProgress;
    QTimer *deviceTimer;

    QLabel *valModel;
    QLabel *valAndroid;
    QLabel *valSerial;
    QProgressBar *batteryBar;
    QLineEdit *wifiIpInput;

    QLineEdit *apkPathInput;
    QCheckBox *chkReplaceApk;
    QCheckBox *chkGrantPerms;
    QLineEdit *pkgNameInput;

    QLineEdit *pushLocalInput;
    QLineEdit *pushRemoteInput;
    QLineEdit *pullRemoteInput;
    QLineEdit *pullLocalInput;

    // حقول صفحة النسخ الاحتياطي وملفات .ab وبيانات الحزم
    QLineEdit *backupPkgInput;
    QLineEdit *abFileInput;
    QLineEdit *targetOwnPkgInput;

    QLineEdit *customShellInput;

    QWidget* createTopBar() {
        QFrame *topBar = new QFrame(this);
        topBar->setObjectName("TopBar");
        topBar->setFixedHeight(64);

        QHBoxLayout *layout = new QHBoxLayout(topBar);
        layout->setContentsMargins(20, 0, 20, 0);

        QLabel *brandTitle = new QLabel("✨ ABDULLAH TOOL PRO", topBar);
        brandTitle->setStyleSheet("font-size: 18px; font-weight: bold; color: #A8C7FA;");

        licenseBadge = new QLabel("🛡️ اشتراك مفعل", topBar);
        licenseBadge->setStyleSheet("background-color: #004A77; color: #C2E7FF; padding: 6px 14px; border-radius: 16px; font-size: 12px; font-weight: bold;");

        connectionBadge = new QLabel("● جاري البحث عن جهاز أندرويد متصل...", topBar);
        connectionBadge->setStyleSheet("background-color: #282A2C; color: #FCD34D; padding: 6px 14px; border-radius: 16px; font-size: 12px; font-weight: bold;");

        QPushButton *refreshBtn = new QPushButton("🔄 فحص الاتصال الآن", topBar);
        refreshBtn->setProperty("class", "SecondaryBtn");
        connect(refreshBtn, &QPushButton::clicked, this, [=]() {
            appendLog("جاري فحص منافذ USB والشبكة للأجهزة المتصلة عبر ADB...", "INFO");
            checkDeviceSilently();
            readFullDeviceInfo();
        });

        layout->addWidget(brandTitle);
        layout->addSpacing(12);
        layout->addWidget(licenseBadge);
        layout->addStretch();
        layout->addWidget(connectionBadge);
        layout->addSpacing(8);
        layout->addWidget(refreshBtn);

        return topBar;
    }

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
            {"📊  معلومات الهاتف والربط", 0},
            {"📦  مدير التطبيقات (APK)", 1},
            {"📁  نقل وسحب الملفات", 2},
            {"🗄️  النسخ الاحتياطي وملفات .ab", 3},
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

        QPushButton *restartAdbBtn = new QPushButton("🛠️ إعادة تشغيل خادم ADB", sidebar);
        restartAdbBtn->setProperty("class", "SecondaryBtn");
        connect(restartAdbBtn, &QPushButton::clicked, this, [=]() {
            appendLog("جاري إعادة تشغيل خادم ADB...", "WARN");
            runAdbCommand({"kill-server"}, "تم إيقاف خادم ADB مؤقتاً.");
            runAdbCommand({"start-server"}, "تم تشغيل خادم ADB بنجاح.");
        });
        layout->addWidget(restartAdbBtn);

        QLabel *footerVer = new QLabel("AbdullahTool • Gemini Dark UI", sidebar);
        footerVer->setAlignment(Qt::AlignCenter);
        footerVer->setStyleSheet("color: #8E918F; font-size: 11px; margin-top: 6px;");
        layout->addWidget(footerVer);

        return sidebar;
    }

    // --- الصفحة 0: معلومات الهاتف، أوضاع الإقلاع، والربط اللاسلكي ---
    QWidget* createDashboardPage() {
        QFrame *page = new QFrame(this);
        page->setObjectName("Card");
        QVBoxLayout *layout = new QVBoxLayout(page);
        layout->setContentsMargins(22, 20, 22, 20);
        layout->setSpacing(14);

        QHBoxLayout *headerRow = new QHBoxLayout();
        QLabel *title = new QLabel("📊 بيانات الجهاز المتصل، الربط اللاسلكي، وأوضاع التشغيل", page);
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
            v->setContentsMargins(16, 12, 16, 12);
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
        batLayout->setContentsMargins(16, 12, 16, 12);
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

        // قسم الربط اللاسلكي للأجهزة ذات منفذ الشحن المعطل
        QLabel *wifiTitle = new QLabel("📡 الربط اللاسلكي عبر الشبكة (ADB over Wi-Fi للأجهزة ذات منفذ الشحن المعطل):", page);
        wifiTitle->setStyleSheet("font-size: 13px; font-weight: bold; color: #A8C7FA; margin-top: 4px;");
        layout->addWidget(wifiTitle);

        QHBoxLayout *wifiRow = new QHBoxLayout();
        wifiIpInput = new QLineEdit(page);
        wifiIpInput->setPlaceholderText("أدخل عنوان IP والمنفذ للهاتف (مثال: 192.168.1.15:5555)");

        QPushButton *btnWifiConnect = new QPushButton("🔗 اتصال لاسلكي (Connect)", page);
        btnWifiConnect->setProperty("class", "ActionBtn");
        connect(btnWifiConnect, &QPushButton::clicked, this, [=]() {
            QString addr = wifiIpInput->text().trimmed();
            if (addr.isEmpty()) {
                QMessageBox::warning(this, "تنبيه", "يرجى كتابة عنوان IP والمنفذ أولاً!");
                return;
            }
            appendLog("جاري الربط اللاسلكي بالهاتف: " + addr, "INFO");
            runAdbCommand({"connect", addr}, "تم إرسال طلب الربط اللاسلكي.");
        });

        QPushButton *btnTcpip = new QPushButton("⚙️ تفعيل منفذ 5555", page);
        btnTcpip->setProperty("class", "SecondaryBtn");
        connect(btnTcpip, &QPushButton::clicked, this, [=]() {
            appendLog("تفعيل وضع الاتصال اللاسلكي عبر المنفذ 5555...", "INFO");
            runAdbCommand({"tcpip", "5555"}, "تم فتح المنفذ 5555 بنجاح.");
        });

        wifiRow->addWidget(wifiIpInput, 1);
        wifiRow->addWidget(btnWifiConnect);
        wifiRow->addWidget(btnTcpip);
        layout->addLayout(wifiRow);

        // قسم أوضاع إعادة التشغيل
        QLabel *rebootTitle = new QLabel("⚡ التحكم السريع في أوضاع الإقلاع (Reboot Modes):", page);
        rebootTitle->setStyleSheet("font-size: 13px; font-weight: bold; color: #E3E3E3; margin-top: 4px;");
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

    // --- الصفحة 1: مدير التطبيقات ---
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

    // --- الصفحة 2: استوديو نقل الملفات ---
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

    // --- الصفحة 3: النسخ الاحتياطي وملفات (.ab) وحقن بيانات الحزم ---
    QWidget* createBackupAndDataPage() {
        QFrame *page = new QFrame(this);
        page->setObjectName("Card");
        QVBoxLayout *layout = new QVBoxLayout(page);
        layout->setContentsMargins(22, 20, 22, 20);
        layout->setSpacing(14);

        QLabel *title = new QLabel("🗄️ إدارة النسخ الاحتياطي (.ab)، فك التشفير إلى (.tar)، وحقن بيانات الحزم", page);
        title->setStyleSheet("font-size: 16px; font-weight: bold; color: #E3E3E3;");
        layout->addWidget(title);

        // 1. أخذ نسخة احتياطية لحزمة بصيغة .ab
        QLabel *sec1 = new QLabel("1️⃣ إنشاء نسخة احتياطية رسمية لبيانات حزمة معينة (ADB Backup to .ab):", page);
        sec1->setStyleSheet("color: #A8C7FA; font-size: 13px; font-weight: bold;");
        layout->addWidget(sec1);

        QHBoxLayout *row1 = new QHBoxLayout();
        backupPkgInput = new QLineEdit(page);
        backupPkgInput->setPlaceholderText("اسم الحزمة المسموح نسخها (مثال: com.example.myapp)");

        QPushButton *btnBackupAb = new QPushButton("💾 سحب بصيغة (.ab)", page);
        btnBackupAb->setProperty("class", "ActionBtn");
        connect(btnBackupAb, &QPushButton::clicked, this, [=]() {
            QString pkg = backupPkgInput->text().trimmed();
            if (pkg.isEmpty()) {
                QMessageBox::warning(this, "تنبيه", "يرجى كتابة اسم الحزمة أولاً!");
                return;
            }
            QString savePath = QFileDialog::getSaveFileName(this, "حفظ ملف النسخة الاحتياطية", pkg + ".ab", "Android Backup (*.ab)");
            if (savePath.isEmpty()) return;

            appendLog("جاري طلب النسخ الاحتياطي للحزمة (يرجى الموافقة من شاشة الهاتف): " + pkg, "WARN");
            runAdbCommand({"backup", "-f", savePath, "-noapk", pkg}, "اكتمل إنشاء ملف النسخة الاحتياطية (.ab).");
        });

        row1->addWidget(backupPkgInput, 1);
        row1->addWidget(btnBackupAb);
        layout->addLayout(row1);

        // 2. فك ضغط ملف .ab إلى أرشيف .tar مباشرة ثم استبدال بيانات تطبيقك الخاص
        QLabel *sec2 = new QLabel("2️⃣ فك ملف (.ab) غير المشفر إلى (.tar) أو حقنه مباشرة داخل مسار تطبيقك الخاص:", page);
        sec2->setStyleSheet("color: #6DD58C; font-size: 13px; font-weight: bold; margin-top: 6px;");
        layout->addWidget(sec2);

        QHBoxLayout *row2 = new QHBoxLayout();
        abFileInput = new QLineEdit(page);
        abFileInput->setPlaceholderText("اختر ملف (.ab) المراد فك ضغطه أو استخراج بياناته...");

        QPushButton *browseAbBtn = new QPushButton("📂 اختيار ملف .ab", page);
        browseAbBtn->setProperty("class", "SecondaryBtn");
        connect(browseAbBtn, &QPushButton::clicked, this, [=]() {
            QString f = QFileDialog::getOpenFileName(this, "اختر ملف Android Backup", "", "Android Backup (*.ab);;Tar Archive (*.tar)");
            if (!f.isEmpty()) abFileInput->setText(f);
        });

        QPushButton *unpackTarBtn = new QPushButton("📦 تحويل (.ab) إلى (.tar)", page);
        unpackTarBtn->setProperty("class", "SecondaryBtn");
        connect(unpackTarBtn, &QPushButton::clicked, this, &MainWindow::convertAbToTar);

        row2->addWidget(abFileInput, 1);
        row2->addWidget(browseAbBtn);
        row2->addWidget(unpackTarBtn);
        layout->addLayout(row2);

        // 3. حقن واستبدال بيانات تطبيقك الخاص (عبر run-as أو Restore)
        QHBoxLayout *row3 = new QHBoxLayout();
        targetOwnPkgInput = new QLineEdit(page);
        targetOwnPkgInput->setPlaceholderText("اسم حزمة تطبيقك الخاص لاستبدال بياناته (مثال: com.abdullah.myapp)");

        QPushButton *injectDataBtn = new QPushButton("⚡ استبدال بيانات تطبيقي من الأرشيف", page);
        injectDataBtn->setProperty("class", "ActionBtn");
        connect(injectDataBtn, &QPushButton::clicked, this, &MainWindow::injectTarIntoOwnApp);

        row3->addWidget(targetOwnPkgInput, 1);
        row3->addWidget(injectDataBtn);
        layout->addLayout(row3);

        layout->addStretch();
        return page;
    }

    // --- الصفحة 4: موجه الأوامر المتقدم ---
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

    // --- تحويل ملف (.ab) غير المشفر إلى (.tar) محلياً باستخدام محرك فك ضغط Zlib في Qt ---
    void convertAbToTar() {
        QString abPath = abFileInput->text().trimmed();
        if (abPath.isEmpty() || !QFile::exists(abPath)) {
            QMessageBox::warning(this, "تنبيه", "يرجى اختيار ملف (.ab) صحيح أولاً!");
            return;
        }

        QFile inFile(abPath);
        if (!inFile.open(QIODevice::ReadOnly)) {
            appendLog("تعذر فتح ملف .ab للقراءة!", "ERROR");
            return;
        }

        QByteArray rawData = inFile.readAll();
        inFile.close();

        // تخطي الرأسية النصية لملف Android Backup (أول 4 أسطر تنتهي بـ \n)
        int newlineCount = 0;
        int offset = 0;
        while (offset < rawData.size() && newlineCount < 4) {
            if (rawData[offset] == '\n') {
                newlineCount++;
            }
            offset++;
        }

        if (newlineCount < 4 || offset >= rawData.size()) {
            appendLog("ملف (.ab) غير صالح أو تالف!", "ERROR");
            return;
        }

        QByteArray headerText = rawData.left(offset);
        if (!headerText.contains("ANDROID BACKUP")) {
            appendLog("هذا الملف ليس ملف Android Backup (.ab) قياسي!", "ERROR");
            return;
        }
        if (!headerText.contains("none")) {
            appendLog("تنبيه: ملف (.ab) محمي بكلمة مرور تشفير AES، يرجى إنشاء النسخة بدون كلمة مرور.", "ERROR");
            return;
        }

        QByteArray payload = rawData.mid(offset);
        QString tarPath = abPath;
        if (tarPath.endsWith(".ab", Qt::CaseInsensitive)) {
            tarPath.chop(3);
        }
        tarPath += ".tar";

        // فك ضغط Zlib إذا كان مضغوطاً
        QByteArray tarData;
        QList<QByteArray> headerLines = headerText.split('\n');
        bool isCompressed = (headerLines.size() >= 3 && headerLines[2].trimmed() == "1");

        if (isCompressed) {
            // إضافة 4 بايت لحجم تقريبي متوقع قبل تمريرها لدالة qUncompress
            quint32 expectedSize = payload.size() * 6;
            QByteArray qzlibHeader(4, 0);
            qzlibHeader[0] = (expectedSize >> 24) & 0xFF;
            qzlibHeader[1] = (expectedSize >> 16) & 0xFF;
            qzlibHeader[2] = (expectedSize >> 8) & 0xFF;
            qzlibHeader[3] = expectedSize & 0xFF;

            tarData = qUncompress(qzlibHeader + payload);
            if (tarData.isEmpty()) {
                appendLog("تعذر فك ضغط تدفق Zlib داخل ملف .ab!", "ERROR");
                return;
            }
        } else {
            tarData = payload;
        }

        QFile outFile(tarPath);
        if (outFile.open(QIODevice::WriteOnly)) {
            outFile.write(tarData);
            outFile.close();
            abFileInput->setText(tarPath);
            appendLog("تم فك ضغط وتحويل ملف (.ab) بنجاح إلى: " + tarPath, "SUCCESS");
        } else {
            appendLog("تعذر حفظ ملف (.tar) في المسار المحدد!", "ERROR");
        }
    }

    // --- حقن واستبدال بيانات تطبيقك الخاص من ملف .tar ---
    void injectTarIntoOwnApp() {
        QString filePath = abFileInput->text().trimmed();
        QString targetPkg = targetOwnPkgInput->text().trimmed();

        if (filePath.isEmpty() || targetPkg.isEmpty()) {
            QMessageBox::warning(this, "تنبيه", "يرجى اختيار ملف (.tar) وكتابة اسم حزمة تطبيقك الخاص أولاً!");
            return;
        }

        if (filePath.endsWith(".ab", Qt::CaseInsensitive)) {
            convertAbToTar();
            filePath = abFileInput->text().trimmed();
            if (!filePath.endsWith(".tar", Qt::CaseInsensitive)) return;
        }

        appendLog("1/2 جاري رفع أرشيف البيانات المؤقت إلى الهاتف (/data/local/tmp/restore_data.tar)...", "INFO");
        runAdbCommand({"push", filePath, "/data/local/tmp/restore_data.tar"}, "تم رفع الأرشيف المؤقت للهاتف.");

        appendLog("2/2 جاري استبدال بيانات الحزمة الخاصة بك عبر صلاحية المطور (run-as " + targetPkg + ")...", "WARN");
        QString shellCmd = QString("run-as %1 tar -xvf /data/local/tmp/restore_data.tar").arg(targetPkg);
        runAdbCommand({"shell", shellCmd}, "تم استخراج واستبدال بيانات تطبيقك بنجاح!");
    }

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

    void runAdbCommand(const QStringList &args, const QString &doneMsg = QString()) {
        QProcess *proc = new QProcess(this);
        operationProgress->setRange(0, 0);

        connect(proc, &QProcess::readyReadStandardOutput, this, [=]() {
            QString out = QString::fromUtf8(proc->readAllStandardOutput()).trimmed();
            if (!out.isEmpty()) {
                for (const QString &line : out.split(QRegularExpression("[\r\n]+"), Qt::SkipEmptyParts)) {
                    appendLog(line.trimmed(), "OUTPUT");
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
            operationProgress->setRange(0, 100);
            operationProgress->setValue(100);
            if (exitCode == 0 && !doneMsg.isEmpty()) {
                appendLog(doneMsg, "SUCCESS");
            }
            proc->deleteLater();
        });

        connect(proc, &QProcess::errorOccurred, this, [=](QProcess::ProcessError) {
            operationProgress->setRange(0, 100);
            operationProgress->setValue(100);
            appendLog("تعذر تشغيل ملف adb.exe! تأكد من وجود مجلد platform-tools بجانب الأداة.", "ERROR");
            proc->deleteLater();
        });

        proc->start(getAdbExecutablePath(), args);
    }

    void checkDeviceSilently() {
        QProcess *proc = new QProcess(this);
        connect(proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [=](int, QProcess::ExitStatus) {
            QString out = QString::fromUtf8(proc->readAllStandardOutput()).trimmed();
            QStringList lines = out.split(QRegularExpression("[\r\n]+"), Qt::SkipEmptyParts);
            QString connectedSerial;

            for (int i = 1; i < lines.size(); ++i) {
                QString line = lines[i].trimmed();
                if (line.endsWith("\tdevice") || line.contains(" device")) {
                    connectedSerial = line.split(QRegularExpression("\\s+")).first().trimmed();
                    break;
                }
            }

            if (!connectedSerial.isEmpty()) {
                connectionBadge->setText("● جهاز متصل: " + connectedSerial);
                connectionBadge->setStyleSheet("background-color: #0F5223; color: #6DD58C; padding: 6px 14px; border-radius: 16px; font-size: 12px; font-weight: bold;");
                valSerial->setText(connectedSerial);
            } else {
                connectionBadge->setText("● غير متصل - بانتظار توصيل USB أو شبكة Wi-Fi");
                connectionBadge->setStyleSheet("background-color: #282A2C; color: #FCD34D; padding: 6px 14px; border-radius: 16px; font-size: 12px; font-weight: bold;");
            }
            proc->deleteLater();
        });
        proc->start(getAdbExecutablePath(), {"devices"});
    }

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
            for (const QString &line : out.split(QRegularExpression("[\r\n]+"), Qt::SkipEmptyParts)) {
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
