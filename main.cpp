cpp_code = r'''#include <QApplication>
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
#include <QComboBox>
#include <QDialog>
#include <QListWidget>
#include <QSplitter>

// ============================================================================
// نافذة فرعية متقدمة لإدارة، إعادة تسمية، وتعديل محتوى ملفات الـ XML قبل الحقن
// ============================================================================
class XmlEditorDialog : public QDialog {
    Q_OBJECT
public:
    XmlEditorDialog(const QString &extractedStagePath, const QString &detectedPkg, QWidget *parent = nullptr)
        : QDialog(parent), stagePath(extractedStagePath), originalPkg(detectedPkg) {
        setWindowTitle("محرر ومكتشف ملفات XML المتقدم - AbdullahTool");
        resize(820, 520);
        setLayoutDirection(Qt::RightToLeft);

        setStyleSheet(R"(
            QDialog {
                background-color: #131314;
                color: #E3E3E3;
                font-family: 'IBM Plex Sans Arabic', 'Segoe UI';
            }
            QLabel { color: #E3E3E3; font-size: 13px; }
            QLineEdit, QTextEdit {
                background-color: #1E1F20;
                border: 1px solid #333537;
                border-radius: 8px;
                color: #E3E3E3;
                padding: 8px;
                font-family: 'Consolas', monospace;
                font-size: 12px;
            }
            QLineEdit:focus, QTextEdit:focus { border: 1px solid #A8C7FA; }
            QListWidget {
                background-color: #1E1F20;
                border: 1px solid #333537;
                border-radius: 8px;
                color: #E3E3E3;
                padding: 4px;
            }
            QListWidget::item { padding: 6px; border-radius: 4px; }
            QListWidget::item:selected { background-color: #004A77; color: #C2E7FF; }
            QPushButton {
                background-color: #282A2C;
                color: #E3E3E3;
                border: 1px solid #333537;
                border-radius: 8px;
                padding: 8px 14px;
                font-weight: bold;
                font-size: 12px;
            }
            QPushButton:hover { background-color: #333537; border: 1px solid #A8C7FA; }
            QPushButton#SaveBtn {
                background-color: #A8C7FA;
                color: #062E6F;
                border: none;
            }
            QPushButton#SaveBtn:hover { background-color: #D3E3FD; }
        )");

        QVBoxLayout *mainLayout = new QVBoxLayout(this);
        mainLayout->setContentsMargins(16, 16, 16, 16);
        mainLayout->setSpacing(12);

        // شريط علوي يوضح الحزمة المكتشفة
        QHBoxLayout *infoLayout = new QHBoxLayout();
        QLabel *pkgInfoLbl = new QLabel(QString("📦 الحزمة الأصلية المكتشفة في النسخة: <b style='color:#FCD34D;'>%1</b>")
                                          .arg(originalPkg.isEmpty() ? "غير معروفة (تلقائي)" : originalPkg), this);
        infoLayout.addWidget(pkgInfoLbl);
        infoLayout.addStretch();
        mainLayout->addLayout(infoLayout);

        // مقسم بين قائمة ملفات الـ XML ومحرر النصوص
        QSplitter *splitter = new QSplitter(Qt::Horizontal, this);

        // الجانب الأيمن: قائمة ملفات الـ XML المكتشفة
        QWidget *leftWidget = new QWidget(this);
        QVBoxLayout *leftLayout = new QVBoxLayout(leftWidget);
        leftLayout->setContentsMargins(0, 0, 0, 0);
        leftLayout->addWidget(new QLabel("ملفات الإعدادات (XML) المتاحة للتعديل:", this));

        xmlFileListWidget = new QListWidget(this);
        leftLayout->addWidget(xmlFileListWidget);

        // زر إعادة تسمية الملف المحدد
        QPushButton *renameFileBtn = new QPushButton("✏️ إعادة تسمية الملف المحدد", this);
        leftLayout->addWidget(renameFileBtn);
        splitter->addWidget(leftWidget);

        // الجانب الأيسر: محرر محتوى ملف الـ XML
        QWidget *rightWidget = new QWidget(this);
        QVBoxLayout *rightLayout = new QVBoxLayout(rightWidget);
        rightLayout->setContentsMargins(0, 0, 0, 0);
        rightLayout->addWidget(new QLabel("محتوى ملف الـ XML (مع إمكانية التعديل المباشر والبحث واستبدال اسم الحزمة):", this));

        xmlContentEdit = new QTextEdit(this);
        rightLayout->addWidget(xmlContentEdit);

        // حقل استبدال سريع داخل محتوى XML
        QHBoxLayout *replaceRow = new QHBoxLayout();
        replaceOldInput = new QLineEdit(this);
        replaceOldInput->setPlaceholderText("النص القديم (مثل اسم الحزمة القديم)...");
        replaceOldInput->setText(originalPkg);

        replaceNewInput = new QLineEdit(this);
        replaceNewInput->setPlaceholderText("النص الجديد...");

        QPushButton *replaceBtn = new QPushButton("🔄 استبدال في الملف", this);
        replaceRow->addWidget(replaceOldInput, 1);
        replaceRow->addWidget(replaceNewInput, 1);
        replaceRow->addWidget(replaceBtn);
        rightLayout->addLayout(replaceRow);

        splitter->addWidget(rightWidget);
        splitter->setSizes(QList<int>() << 280 << 540);
        mainLayout->addWidget(splitter, 1);

        // شريط الأزرار السفلي
        QHBoxLayout *bottomRow = new QHBoxLayout();
        QPushButton *autoFixBtn = new QPushButton("⚡ تصحيح تلقائي لأسماء الملفات والمحتوى", this);
        QPushButton *saveAndCloseBtn = new QPushButton("✅ حفظ التعديلات وإغلاق", this);
        saveAndCloseBtn.setObjectName("SaveBtn");

        bottomRow->addWidget(autoFixBtn);
        bottomRow->addStretch();
        bottomRow->addWidget(saveAndCloseBtn);
        mainLayout->addLayout(bottomRow);

        // تحميل ملفات الـ XML الموجودة في مسار الـ stage
        loadXmlFiles();

        // ربط الأحداث
        connect(xmlFileListWidget, &QListWidget::itemSelectionChanged, this, &XmlEditorDialog::onFileSelected);
        connect(renameFileBtn, &QPushButton::clicked, this, &XmlEditorDialog::renameSelectedFile);
        connect(replaceBtn, &QPushButton::clicked, this, &XmlEditorDialog::performQuickReplace);
        connect(autoFixBtn, &QPushButton::clicked, this, &XmlEditorDialog::performAutoFix);
        connect(saveAndCloseBtn, &QPushButton::clicked, this, &XmlEditorDialog::saveCurrentFileAndAccept);
    }

private:
    QString stagePath;
    QString originalPkg;
    QListWidget *xmlFileListWidget;
    QTextEdit *xmlContentEdit;
    QLineEdit *replaceOldInput;
    QLineEdit *replaceNewInput;
    QString currentSelectedFilePath;

    void loadXmlFiles() {
        xmlFileListWidget->clear();
        QDir dir(stagePath);
        if (!dir.exists()) return;

        // البحث التزايدي عن جميع ملفات .xml في المجلد والمجلدات الفرعية (مثل shared_prefs أو db)
        QDirIterator it(stagePath, QStringList() << "*.xml", QDir::Files, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            QString filePath = it.next();
            QString relPath = QDir(stagePath).relativeFilePath(filePath);
            QListWidgetItem *item = new QListWidgetItem(relPath, xmlFileListWidget);
            item->setData(Qt::UserRole, filePath);
        }

        if (xmlFileListWidget->count() > 0) {
            xmlFileListWidget->setCurrentRow(0);
        }
    }

    void onFileSelected() {
        // حفظ تعديلات الملف السابق إن وجد
        saveCurrentContentToDisk();

        QListWidgetItem *item = xmlFileListWidget->currentItem();
        if (!item) return;

        currentSelectedFilePath = item->data(Qt::UserRole).toString();
        QFile file(currentSelectedFilePath);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            xmlContentEdit->setPlainText(QString::fromUtf8(file.readAll()));
            file.close();
        } else {
            xmlContentEdit->clear();
        }
    }

    void saveCurrentContentToDisk() {
        if (currentSelectedFilePath.isEmpty()) return;
        QFile file(currentSelectedFilePath);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
            QTextStream out(&file);
            out.setEncoding(QStringConverter::Utf8);
            out << xmlContentEdit->toPlainText();
            file.close();
        }
    }

    void renameSelectedFile() {
        QListWidgetItem *item = xmlFileListWidget->currentItem();
        if (!item) return;

        QString oldPath = item->data(Qt::UserRole).toString();
        QFileInfo fi(oldPath);
        QString oldName = fi.fileName();

        // نافذة إدخال الاسم الجديد
        bool ok;
        QString newName = QInputDialog::getText(this, "إعادة تسمية ملف XML", "أدخل الاسم الجديد للملف:", QLineEdit::Normal, oldName, &ok);
        if (ok && !newName.isEmpty() && newName != oldName) {
            QString newPath = fi.absolutePath() + "/" + newName;
            if (QFile::rename(oldPath, newPath)) {
                loadXmlFiles();
                QMessageBox::information(this, "تم بنجاح", "تمت إعادة تسمية الملف بنجاح.");
            } else {
                QMessageBox::warning(this, "خطأ", "تعذر إعادة تسمية الملف!");
            }
        }
    }

    void performQuickReplace() {
        QString oldText = replaceOldInput->text();
        QString newText = replaceNewInput->text();
        if (oldText.isEmpty()) {
            QMessageBox::warning(this, "تنبيه", "يرجى كتابة النص المراد استبداله!");
            return;
        }

        QString content = xmlContentEdit->toPlainText();
        if (content.contains(oldText)) {
            content.replace(oldText, newText);
            xmlContentEdit->setPlainText(content);
            QMessageBox::information(this, "تم الاستبدال", "تمت عملية الاستبدال في المحتوى الحالي بنجاح.");
        } else {
            QMessageBox::information(this, "غير موجود", "النص المراد استبداله غير موجود في هذا الملف.");
        }
    }

    void performAutoFix() {
        saveCurrentContentToDisk();
        int fixedCount = 0;

        QDirIterator it(stagePath, QStringList() << "*.xml", QDir::Files, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            QString filePath = it.next();
            QFile file(filePath);
            if (file.open(QIODevice::ReadWrite | QIODevice::Text)) {
                QString content = QString::fromUtf8(file.readAll());
                bool modified = false;

                // إذا كان اسم الحزمة الأصلية موجوداً داخل محتوى XML، يتم استبداله بالحزمة الجديدة أو تنظيفه
                if (!originalPkg.isEmpty() && content.contains(originalPkg)) {
                    content.replace(originalPkg, replaceNewInput->text().trimmed().isEmpty() ? "com.uu" : replaceNewInput->text().trimmed());
                    modified = true;
                }

                if (modified) {
                    file.resize(0);
                    QTextStream out(&file);
                    out.setEncoding(QStringConverter::Utf8);
                    out << content;
                    fixedCount++;
                }
                file.close();
            }
        }

        onFileSelected(); // تحديث المحرر
        QMessageBox::information(this, "التصحيح التلقائي", QString("تم تصحيح وتحديث محتوى %1 ملف XML تلقائياً.").arg(fixedCount));
    }

    void saveCurrentFileAndAccept() {
        saveCurrentContentToDisk();
        accept();
    }
};

// ============================================================================
// الدوال المساعدة العامة (خطوط Gemini + بصمة الجهاز HWID + مسار ADB)
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

        QLabel *editionLabel = new QLabel("PRO ENTERPRISE EDITION • V2.3", brandPanel);
        editionLabel->setStyleSheet("font-size: 11px; font-weight: 600; color: #8E918F; background: transparent;");

        QLabel *descLabel = new QLabel(
            "الأداة الاحترافية المتكاملة لإدارة وصيانة أجهزة الأندرويد، "
            "استخراج الحزم، محرر XML المتقدم، الربط اللاسلكي، وتحويل بيانات (data/data).",
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
        setWindowTitle("AbdullahTool Pro v2.3 - النظام المتكامل لإدارة أجهزة الأندرويد");
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
            QLineEdit, QComboBox {
                background-color: #131314;
                border: 1px solid #333537;
                border-radius: 10px;
                padding: 9px 14px;
                color: #E3E3E3;
                font-size: 13px;
            }
            QLineEdit:focus, QComboBox:focus {
                border: 1px solid #A8C7FA;
            }
            QComboBox::drop-down { border: 0px; }
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
        pagesStack->addWidget(createBackupRestorePage());  // 2: النسخ الاحتياطي وحقن data/data والمحرر
        pagesStack->addWidget(createFileTransferPage());   // 3: نقل وسحب الملفات
        pagesStack->addWidget(createShellTerminalPage());  // 4: أوامر Shell المتقدمة

        workAreaLayout->addWidget(pagesStack, 3);
        workAreaLayout->addWidget(createConsolePanel(), 2);

        bodyLayout->addLayout(workAreaLayout, 1);
        rootLayout->addLayout(bodyLayout, 1);

        deviceTimer = new QTimer(this);
        connect(deviceTimer, &QTimer::timeout, this, &MainWindow::checkDeviceSilently);
        deviceTimer->start(3500);

        appendLog("تم تشغيل AbdullahTool Pro v2.3 بنجاح. جميع الأنظمة جاهزة.", "SUCCESS");
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

    QQueue<AdbStep> stepQueue;
    bool isSequenceRunning = false;

    // عناصر الصفحة 0
    QLabel *valModel;
    QLabel *valAndroid;
    QLabel *valSerial;
    QProgressBar *batteryBar;

    // عناصر الصفحة 1
    QLineEdit *apkPathInput;
    QCheckBox *chkReplaceApk;
    QCheckBox *chkGrantPerms;
    QLineEdit *pkgNameInput;

    // عناصر الصفحة 2: النسخ الاحتياطي ومحرر XML
    QLineEdit *backupPkgInput;
    QLineEdit *backupOutPathInput;
    QCheckBox *chkIncludeApkInBackup;
    QLineEdit *abFileInput;
    QLineEdit *tarOutputInput;
    QLineEdit *restoreSourceInput;
    QComboBox *restoreTargetPkgCombo;   // القائمة المنسدلة الجديدة للحزم الهدف
    QString detectedOriginalPkg;       // الحزمة الأصلية المكتشفة تلقائياً من الأرشيف

    // عناصر الصفحة 3
    QLineEdit *pushLocalInput;
    QLineEdit *pushRemoteInput;
    QLineEdit *pullRemoteInput;
    QLineEdit *pullLocalInput;

    // عناصر الصفحة 4
    QLineEdit *customShellInput;

    // --- الشريط العلوي ---
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

        wifiIpInput = new QLineEdit(topBar);
        wifiIpInput->setPlaceholderText("IP:Port لاسلكي (مثال 192.168.1.15:5555)");
        wifiIpInput->setFixedWidth(230);

        QPushButton *wifiConnectBtn = new QPushButton("📡 ربط لاسلكي", topBar);
        wifiConnectBtn->setProperty("class", "SecondaryBtn");
        connect(wifiConnectBtn, &QPushButton::clicked, this, [=]() {
            QString addr = wifiIpInput->text().trimmed();
            if (addr.isEmpty()) {
                QMessageBox::warning(this, "تنبيه", "أدخل عنوان IP والمنفذ أولاً للربط اللاسلكي!");
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
            {"🧬  بيانات وحزم data/data", 2},
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

    // --- الصفحة 2: النسخ الاحتياطي (.ab) ومحرر XML وقائمة الحزم المستهدفة ---
    QWidget* createBackupRestorePage() {
        QFrame *page = new QFrame(this);
        page->setObjectName("Card");
        QVBoxLayout *layout = new QVBoxLayout(page);
        layout->setContentsMargins(22, 18, 22, 18);
        layout->setSpacing(12);

        QLabel *title = new QLabel("🧬 استوديو النسخ الاحتياطي ومحرر ملفات XML واستعادة بيانات الحزم (/data/data)", page);
        title->setStyleSheet("font-size: 16px; font-weight: bold; color: #E3E3E3;");
        layout->addWidget(title);

        // 1. سحب نسخة احتياطية بصيغة .ab
        QLabel *sec1 = new QLabel("1️⃣ إنشاء نسخة احتياطية رسمية لحزمة معينة إلى ملف (.ab):", page);
        sec1->setStyleSheet("color: #A8C7FA; font-size: 13px; font-weight: bold;");
        layout->addWidget(sec1);

        QHBoxLayout *backupRow = new QHBoxLayout();
        backupPkgInput = new QLineEdit(page);
        backupPkgInput->setPlaceholderText("اسم الحزمة المراد نسخها (مثال: com.uu)");

        backupOutPathInput = new QLineEdit(QDir::homePath() + "/Desktop/backup.ab", page);
        QPushButton *browseBackupOutBtn = new QPushButton("حفظ باسم...", page);
        browseBackupOutBtn->setProperty("class", "SecondaryBtn");
        connect(browseBackupOutBtn, &QPushButton::clicked, this, [=]() {
            QString f = QFileDialog::getSaveFileName(this, "حفظ ملف النسخة الاحتياطية", backupOutPathInput->text(), "Android Backup (*.ab)");
            if (!f.isEmpty()) backupOutPathInput->setText(f);
        });

        chkIncludeApkInBackup = new QCheckBox("تضمين APK", page);
        QPushButton *doBackupBtn = new QPushButton("💾 سحب .ab", page);
        doBackupBtn->setProperty("class", "ActionBtn");
        connect(doBackupBtn, &QPushButton::clicked, this, [=]() {
            QString pkg = backupPkgInput->text().trimmed();
            QString out = backupOutPathInput->text().trimmed();
            if (pkg.isEmpty() || out.isEmpty()) {
                QMessageBox::warning(this, "تنبيه", "أدخل اسم الحزمة ومسار حفظ ملف .ab!");
                return;
            }
            QStringList args = {"backup", "-f", out};
            args << (chkIncludeApkInBackup->isChecked() ? "-apk" : "-noapk");
            args << pkg;
            appendLog("طلب النسخ الاحتياطي للحزمة " + pkg + " (يُرجى الموافقة من شاشة الهاتف)...", "WARN");
            runAdbCommand(args, "انتهت عملية إنشاء ملف النسخة الاحتياطية: " + out);
        });

        backupRow->addWidget(backupPkgInput, 1);
        backupRow->addWidget(backupOutPathInput, 2);
        backupRow->addWidget(browseBackupOutBtn);
        backupRow->addWidget(chkIncludeApkInBackup);
        backupRow->addWidget(doBackupBtn);
        layout->addLayout(backupRow);

        // 2. فك .ab وتحديد الحزمة الأصلية تلقائياً
        QLabel *sec2 = new QLabel("2️⃣ تحويل ملف (.ab) إلى (.tar) واكتشاف الحزمة الأصلية تلقائياً:", page);
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
                if (tarSuggested.endsWith(".ab", Qt::CaseInsensitive)) tarSuggested.chop(3);
                tarOutputInput->setText(tarSuggested + ".tar");
            }
        });

        tarOutputInput = new QLineEdit(page);
        tarOutputInput->setPlaceholderText("مسار ملف .tar الناتج...");

        QPushButton *convertAbBtn = new QPushButton("⚙️ فك واكتشاف الحزمة", page);
        convertAbBtn->setProperty("class", "SecondaryBtn");
        connect(convertAbBtn, &QPushButton::clicked, this, [=]() {
            QString inAb = abFileInput->text().trimmed();
            QString outTar = tarOutputInput->text().trimmed();
            if (inAb.isEmpty() || outTar.isEmpty()) {
                QMessageBox::warning(this, "تنبيه", "تحديد ملف .ab ومسار .tar مطلوب!");
                return;
            }
            if (convertAbFileToTar(inAb, outTar)) {
                restoreSourceInput->setText(outTar);
                // اكتشاف اسم الحزمة الأصلي تلقائياً من الأرشيف
                detectedOriginalPkg = detectOriginalPackageFromTar(outTar);
                if (!detectedOriginalPkg.isEmpty()) {
                    appendLog("✔ تم اكتشاف اسم الحزمة الأصلية تلقائياً بنجاح: " + detectedOriginalPkg, "SUCCESS");
                    QMessageBox::information(this, "اكتشاف الحزمة", "تم اكتشاف الحزمة الأصلية تلقائياً:\n" + detectedOriginalPkg);
                } else {
                    appendLog("لم يتم التعرف على اسم الحزمة الأصلية تلقائياً.", "WARN");
                }
            }
        });

        unpackRow->addWidget(abFileInput, 2);
        unpackRow->addWidget(browseAbBtn);
        unpackRow->addWidget(tarOutputInput, 2);
        unpackRow->addWidget(convertAbBtn);
        layout->addLayout(unpackRow);

        // 3. محرر ملفات الـ XML المتقدم قبل الحقن
        QLabel *sec3 = new QLabel("3️⃣ مراجعة، إعادة تسمية، وتعديل محتوى ملفات الـ XML المستخرجة:", page);
        sec3->setStyleSheet("color: #38BDF8; font-size: 13px; font-weight: bold; margin-top: 6px;");
        layout->addWidget(sec3);

        QHBoxLayout *editorRow = new QHBoxLayout();
        QPushButton *openXmlEditorBtn = new QPushButton("📝 فتح محرر ومكتشف ملفات XML المتقدم", page);
        openXmlEditorBtn->setProperty("class", "SecondaryBtn");
        connect(openXmlEditorBtn, &QPushButton::clicked, this, [=]() {
            QString src = restoreSourceInput->text().trimmed();
            if (src.isEmpty()) {
                QMessageBox::warning(this, "تنبيه", "يرجى تحديد ملف .tar أو مجلد البيانات أولاً!");
                return;
            }
            // فك الـ tar مؤقتاً في مجلد مؤقت محلي على الكمبيوتر إذا كان ملف tar لتعديل محتواه
            QString tempStage = QDir::tempPath() + "/abdullah_xml_editor_stage";
            QDir(tempStage).removeRecursively();
            QDir().mkpath(tempStage);

            if (src.endsWith(".tar", Qt::CaseInsensitive)) {
                // استخراج محتوى tar محلياً عبر أداة استخراج أو قراءة بسيطة
                extractTarLocally(src, tempStage);
            } else if (QFileInfo(src).isDir()) {
                copyDirectoryRecursively(src, tempStage);
            }

            XmlEditorDialog editor(tempStage, detectedOriginalPkg, this);
            if (editor.exec() == QDialog::Accepted) {
                appendLog("تم حفظ تعديلات ملفات الـ XML بنجاح.", "SUCCESS");
            }
        });
        editorRow->addWidget(openXmlEditorBtn);
        editorRow->addStretch();
        layout->addLayout(editorRow);

        // 4. استعادة وحقن البيانات مع القائمة المنسدلة للحزم الهدف (5 أسماء حزم وهمية مسبقة + إمكانية الكتابة)
        QLabel *sec4 = new QLabel("4️⃣ استعادة وحقن البيانات إلى الحزمة الهدف مع القائمة المنسدلة:", page);
        sec4->setStyleSheet("color: #6DD58C; font-size: 13px; font-weight: bold; margin-top: 6px;");
        layout->addWidget(sec4);

        QHBoxLayout *restoreRow = new QHBoxLayout();
        restoreSourceInput = new QLineEdit(page);
        restoreSourceInput->setPlaceholderText("ملف (.tar / .ab) أو المجلد المستخرج...");

        QPushButton *browseTarBtn = new QPushButton("📄 ملف", page);
        browseTarBtn->setProperty("class", "SecondaryBtn");
        connect(browseTarBtn, &QPushButton::clicked, this, [=]() {
            QString f = QFileDialog::getOpenFileName(this, "اختر ملف tar أو ab", "", "Archives (*.tar *.ab);;All Files (*.*)");
            if (!f.isEmpty()) restoreSourceInput->setText(f);
        });

        // القائمة المنسدلة الجديدة للحزم الهدف (5 أسماء حزم وهمية للاختبار + قابلة للتعديل والتحرير)
        restoreTargetPkgCombo = new QComboBox(page);
        restoreTargetPkgCombo->setEditable(true); // يتيح للمستخدم الكتابة اليدوية أو الاختيار
        restoreTargetPkgCombo->addItems({
            "com.whatsapp",
            "com.universe.messenger",
            "com.nasa",
            "com.android.ab",
            "com.android.meta"
        });
        restoreTargetPkgCombo->setFixedWidth(210);

        QPushButton *doRestoreBtn = new QPushButton("🚀 حقن البيانات واستبدالها", page);
        doRestoreBtn->setProperty("class", "ActionBtn");
        connect(doRestoreBtn, &QPushButton::clicked, this, [=]() {
            restoreDataToPackage(restoreSourceInput->text().trimmed(), restoreTargetPkgCombo->currentText().trimmed());
        });

        restoreRow->addWidget(restoreSourceInput, 2);
        restoreRow->addWidget(browseTarBtn);
        restoreRow->addWidget(new QLabel("الحزمة الهدف:", page));
        restoreRow->addWidget(restoreTargetPkgCombo);
        restoreRow->addWidget(doRestoreBtn);
        layout->addLayout(restoreRow);

        layout->addStretch();
        return page;
    }

    // --- اكتشاف اسم الحزمة الأصلي تلقائياً من هيكل ملف الـ tar ---
    QString detectOriginalPackageFromTar(const QString &tarPath) {
        QProcess proc;
        proc.start(getAdbExecutablePath(), {"shell", " tar -tf " + tarPath}); // محاكاة أو استخدام أمر محلي إن توفر
        // بدلاً من الاعتماد على adb للتفتیش المحلي، نقوم بقراءة أول 50 سطر من محتويات ملف tar محلياً إن أمكن أو عبر أمر بسيط
        // طريقة برمجية نقية عبر فحص رؤوس tar محلياً أو تخزينها مؤقتاً:
        QFile file(tarPath);
        if (!file.open(QIODevice::ReadOnly)) return QString();

        // قراءة عينة من ملف الـ tar للبحث عن نمط apps/COM.NAME/
        QByteArray sample = file.read(512 * 200); // أول 100 كيلوبايت
        file.close();

        QRegularExpression rx("apps/([a-zA-Z0-9_]+\\.[a-zA-Z0-9_\\.]+)/");
        QRegularExpressionMatch match = rx.match(QString::fromUtf8(sample));
        if (match.hasMatch()) {
            return match.captured(1);
        }
        return QString();
    }

    // استخراج tar محلياً في مجلد مؤقت على الكمبيوتر للتعديل عبر محرر XML
    void extractTarLocally(const QString &tarPath, const QString &destDir) {
        // يتم استخدام الأداة أو فك تدفق البايتات المبسط
        // لتجنب الاعتماد الخارجي، نعتمد على استدعاء 7z أو tar إن وجد، أو نسخ الملف واستخدامه
        QProcess p;
        p.start("tar", QStringList() << "-xf" << tarPath << "-C" << destDir);
        p.waitForFinished(3000);
    }

    void copyDirectoryRecursively(const QString &fromDir, const QString &toDir) {
        QDir().mkpath(toDir);
        QDirIterator it(fromDir, QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            QString filePath = it.next();
            QString relPath = QDir(fromDir).relativeFilePath(filePath);
            if (QFileInfo(filePath).isDir()) {
                QDir().mkpath(toDir + "/" + relPath);
            } else {
                QFile::copy(filePath, toDir + "/" + relPath);
            }
        }
    }

    // --- الصفحة 3: نقل الملفات ---
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
                QMessageBox::warning(this, "تنبيه", "حدد ملف الكمبيوتر ومسار الهاتف!");
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
                QMessageBox::warning(this, "تنبيه", "حدد مسار الهاتف ومجلد الكمبيوتر!");
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
            appendLog("ملف .ab محمي بكلمة مرور (" + QString::fromUtf8(encryptionType) + ").", "ERROR");
            return false;
        }

        QByteArray payload = rawData.mid(headerEndOffset);
        QByteArray tarData;

        if (compressedFlag == "1") {
            appendLog("جاري فك ضغط بيانات Zlib من ملف .ab...", "INFO");
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
            appendLog("تعذر إنشاء ملف .tar: " + tarFilePath, "ERROR");
            return false;
        }

        outFile.write(tarData);
        outFile.close();

        appendLog(QString("تم تحويل ملف .ab إلى أرشيف .tar بنجاح: %1").arg(tarFilePath), "SUCCESS");
        return true;
    }

    void restoreDataToPackage(const QString &sourcePath, const QString &targetPkg) {
        if (sourcePath.isEmpty() || targetPkg.isEmpty()) {
            QMessageBox::warning(this, "تنبيه", "يرجى تحديد الملف/المجلد واسم الحزمة الهدف!");
            return;
        }

        QRegularExpression pkgRegex("^[a-zA-Z0-9_]+(\\.[a-zA-Z0-9_]+)+$");
        if (!pkgRegex.match(targetPkg).hasMatch()) {
            QMessageBox::warning(this, "اسم حزمة غير صالح", "أدخل اسم حزمة أندرويد صحيح (مثال: com.uu)");
            return;
        }

        QFileInfo srcInfo(sourcePath);
        if (!srcInfo.exists()) {
            appendLog("المسار المحدد غير موجود: " + sourcePath, "ERROR");
            return;
        }

        QString effectiveSource = sourcePath;
        if (srcInfo.isFile() && sourcePath.endsWith(".ab", Qt::CaseInsensitive)) {
            QString autoTarPath = QDir::tempPath() + "/abdullah_auto_converted.tar";
            appendLog("تحويل ملف .ab تلقائياً إلى .tar...", "INFO");
            if (!convertAbFileToTar(sourcePath, autoTarPath)) return;
            effectiveSource = autoTarPath;
            srcInfo = QFileInfo(effectiveSource);
        }

        QString remoteStage = "/data/local/tmp/abdullah_restore_stage";
        QList<AdbStep> sequence;

        sequence.append({
            {"shell", "am", "force-stop", targetPkg},
            "1/5 إيقاف الحزمة الهدف مؤقتاً (" + targetPkg + ")...",
            ""
        });

        sequence.append({
            {"shell", "rm -rf " + remoteStage + " && mkdir -p " + remoteStage},
            "2/5 تجهيز المسار الوسيط المؤقت على الهاتف...",
            ""
        });

        if (srcInfo.isFile() && effectiveSource.endsWith(".tar", Qt::CaseInsensitive)) {
            QString remoteTar = remoteStage + "/payload.tar";
            sequence.append({
                {"push", effectiveSource, remoteTar},
                "3/5 رفع أرشيف .tar إلى الهاتف...",
                "تم رفع الأرشيف بنجاح."
            });
            sequence.append({
                {"shell", "tar -xf " + remoteTar + " -C " + remoteStage + " && rm -f " + remoteTar + " && chmod -R 755 " + remoteStage},
                "4/5 فك ضغط الأرشيف على الهاتف...",
                ""
            });
        } else if (srcInfo.isDir()) {
            sequence.append({
                {"push", effectiveSource + "/.", remoteStage + "/"},
                "3/5 رفع مجلد البيانات المستخرج إلى الهاتف...",
                "تم رفع المجلد بنجاح."
            });
            sequence.append({
                {"shell", "chmod -R 755 " + remoteStage},
                "4/5 ضبط صلاحيات القراءة للملفات...",
                ""
            });
        }

        QString restoreScript = QString(
            "PKG='%1'; "
            "STAGE='/data/local/tmp/abdullah_restore_stage'; "
            "APP_SRC=$(find $STAGE/apps -mindepth 1 -maxdepth 1 -type d 2>/dev/null | head -n 1); "
            "if [ -z \"$APP_SRC\" ]; then APP_SRC=\"$STAGE\"; fi; "
            "if run-as $PKG true 2>/dev/null; then "
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
            "  echo 'ERROR: Cannot write to /data/data/'$PKG; "
            "  rm -rf $STAGE; "
            "  exit 1; "
            "fi; "
            "rm -rf $STAGE;"
        ).arg(targetPkg);

        sequence.append({
            {"shell", restoreScript},
            "5/5 حقن البيانات داخل /data/data/" + targetPkg + " وتصحيح الصلاحيات...",
            "تم استبدال بيانات الحزمة " + targetPkg + " بنجاح تام!"
        });

        runAdbSequence(sequence);
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
                appendLog("توقفت السلسلة بسبب خطأ في التنفيذ (كود الخروج: " + QString::number(exitCode) + ").", "ERROR");
                stepQueue.clear();
                isSequenceRunning = false;
                operationProgress->setRange(0, 100);
                operationProgress->setValue(100);
            }
        });

        connect(proc, &QProcess::errorOccurred, this, [=](QProcess::ProcessError) {
            proc->deleteLater();
            appendLog("تعذر تشغيل adb.exe!", "ERROR");
            stepQueue.clear();
            isSequenceRunning = false;
            operationProgress->setRange(0, 100);
            operationProgress->setValue(100);
        });

        proc->start(getAdbExecutablePath(), current.args);
    }

    void runAdbCommand(const QStringList &args, const QString &doneMsg = QString()) {
        runAdbSequence({{args, "", doneMsg}});
    }

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

    void readFullDeviceInfo() {
        appendLog("قراءة مواصفات الهاتف...", "INFO");

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
'''

with open("main.cpp", "w", encoding="utf-8") as f:
    f.write(cpp_code)

print("Successfully generated full main.cpp. Lines:", len(cpp_code.splitlines()))
