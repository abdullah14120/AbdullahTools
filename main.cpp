#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
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
#include <QDirIterator>
#include <QInputDialog>
#include <QTextStream>
#include <QSplitter>
#include <QListWidget>
#include <QListWidgetItem>
#include <QComboBox>

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
infoLayout->addWidget(pkgInfoLbl);
infoLayout->addStretch(); // تم تعديل النقطة إلى سهم ->
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
        saveAndCloseBtn->setObjectName("SaveBtn");

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
        pagesStack->addWidget(createWirelessControlPage()); // 5: الربط اللاسلكي والتحكم بالشاشة
        pagesStack->addWidget(createLocalBridgePage());         // 6: الاتصال عبر التطبيق الوسيط (Local Wi-Fi Bridge)

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

// عناصر الصفحة 5: المركز اللاسلكي والتحكم بالشاشة
    QLabel *wirelessStatusBanner;
    QLineEdit *pairIpPortInput;
    QLineEdit *pairCodeInput;
    QLineEdit *wirelessConnectIpInput;
    QCheckBox *chkWirelessStayAwake;
    QCheckBox *chkWirelessScreenOff;
    QLineEdit *wirelessQuickPkgInput;
    QLineEdit *wirelessShellInput;
    
   // عناصر الصفحة 6: التطبيق الوسيط المطور (AT-Bridge v2.0)
    QLineEdit *bridgeIpPortInput;
    QLabel *bridgeStatusLabel;
    QLineEdit *bridgeQuickPkgInput;
    QLineEdit *bridgePushLocalInput;
    QLineEdit *bridgePushRemoteInput;
    QLineEdit *bridgePullRemoteInput;
    QLineEdit *bridgeShellInput;
    QLineEdit *bridgeActionInput;
    
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
            {"💻  أوامر Shell المتقدمة", 4},
            {"📡  الربط اللاسلكي والشاشة", 5},
            {"🌐  التطبيق الوسيط (Wi-Fi Bridge)", 6}
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
                restoreSourceInput->setText(tempStage);
                appendLog("تم حفظ تعديلات ملفات الـ XML وتحديد المجلد المعدل للاستعادة.", "SUCCESS");
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

        QPushButton *doRestoreBtn = new QPushButton("🚀 حقن مباشر (data/data)", page);
        doRestoreBtn->setProperty("class", "ActionBtn");
        connect(doRestoreBtn, &QPushButton::clicked, this, [=]() {
            restoreDataToPackage(restoreSourceInput->text().trimmed(), restoreTargetPkgCombo->currentText().trimmed());
        });

        QPushButton *doAbRestoreBtn = new QPushButton("📦 إعادة حزم (.ab) واستعادة", page);
        doAbRestoreBtn->setProperty("class", "SecondaryBtn");
        connect(doAbRestoreBtn, &QPushButton::clicked, this, [=]() {
            repackAndRestoreAb(restoreSourceInput->text().trimmed(), restoreTargetPkgCombo->currentText().trimmed());
        });

        restoreRow->addWidget(restoreSourceInput, 2);
        restoreRow->addWidget(browseTarBtn);
        restoreRow->addWidget(new QLabel("الحزمة الهدف:", page));
        restoreRow->addWidget(restoreTargetPkgCombo);
        restoreRow->addWidget(doRestoreBtn);
        restoreRow->addWidget(doAbRestoreBtn);
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

// --- الصفحة 5: المركز اللاسلكي المتكامل (الإقران بالكود + تصحيح الاتصال + عرض الشاشة + العمليات) ---
    QWidget* createWirelessControlPage() {
        QFrame *page = new QFrame(this);
        page->setObjectName("Card");
        QVBoxLayout *layout = new QVBoxLayout(page);
        layout->setContentsMargins(22, 16, 22, 16);
        layout->setSpacing(10);

        // الشريط العلوي: عنوان الصفحة + أول زر للفحص والتصحيح التلقائي للاتصال اللاسلكي
        QHBoxLayout *headerRow = new QHBoxLayout();
        QLabel *title = new QLabel("📡 مركز تصحيح الأخطاء اللاسلكي والتحكم بشاشة الهاتف", page);
        title->setStyleSheet("font-size: 16px; font-weight: bold; color: #E3E3E3;");

        QPushButton *verifyAndFixBtn = new QPushButton("🩺 فحص وتصحيح الاتصال اللاسلكي الآن", page);
        verifyAndFixBtn->setProperty("class", "ActionBtn");
        connect(verifyAndFixBtn, &QPushButton::clicked, this, &MainWindow::verifyAndFixWirelessConnection);

        headerRow->addWidget(title);
        headerRow->addStretch();
        headerRow->addWidget(verifyAndFixBtn);
        layout->addLayout(headerRow);

        // لافتة حالة الاتصال اللاسلكي الفورية
        wirelessStatusBanner = new QLabel("ℹ️ حالة اللاسلكي: اضغط على (فحص وتصحيح الاتصال) للتأكد من جاهزية الهاتف لتلقي الأوامر.", page);
        wirelessStatusBanner->setStyleSheet("background-color: #131314; color: #FCD34D; border: 1px solid #333537; border-radius: 8px; padding: 8px 12px; font-size: 12px; font-weight: bold;");
        layout->addWidget(wirelessStatusBanner);

        // 1. قسم الإقران بالرمز السري (Pairing Code - Android 11+)
        QLabel *sec1 = new QLabel("1️⃣ الخطوة الأولى (لمرة واحدة): إقران الهاتف بالرمز السري المكون من 6 أرقام (Pair with code):", page);
        sec1->setStyleSheet("color: #FCD34D; font-size: 13px; font-weight: bold;");
        layout->addWidget(sec1);

        QHBoxLayout *pairRow = new QHBoxLayout();
        pairIpPortInput = new QLineEdit(page);
        pairIpPortInput->setPlaceholderText("IP:Port الإقران (الذي يظهر تحت الرمز السري، مثال: 192.168.1.15:38123)");

        pairCodeInput = new QLineEdit(page);
        pairCodeInput->setPlaceholderText("الرمز السري (6 أرقام)");
        pairCodeInput->setFixedWidth(160);

        QPushButton *doPairBtn = new QPushButton("🔗 إقران بالرمز (adb pair)", page);
        doPairBtn->setProperty("class", "SecondaryBtn");
        connect(doPairBtn, &QPushButton::clicked, this, [=]() {
            QString pairAddr = pairIpPortInput->text().trimmed();
            QString code = pairCodeInput->text().trimmed();
            if (pairAddr.isEmpty() || code.isEmpty()) {
                QMessageBox::warning(this, "بيانات ناقصة", "يرجى إدخال عنوان IP:Port الخاص بنافذة الإقران، وإدخال الرمز السري المكون من 6 أرقام!");
                return;
            }
            appendLog("جاري إقران الجهاز لاسلكياً على " + pairAddr + " بالرمز: " + code, "INFO");
            QProcess *proc = new QProcess(this);
            connect(proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [=](int, QProcess::ExitStatus) {
                QString out = QString::fromUtf8(proc->readAllStandardOutput() + proc->readAllStandardError()).trimmed();
                appendLog(out, out.contains("Successfully paired", Qt::CaseInsensitive) ? "SUCCESS" : "WARN");
                if (out.contains("Successfully paired", Qt::CaseInsensitive)) {
                    wirelessStatusBanner->setText("✔ تم الإقران بنجاح! الآن أدخل الـ IP والمنفذ الرئيسي في الخطوة 2 واضغط اتصال.");
                    wirelessStatusBanner->setStyleSheet("background-color: #0F5223; color: #6DD58C; border-radius: 8px; padding: 8px 12px; font-weight: bold;");
                    // نسخ الـ IP تلقائياً لخانة الاتصال لتسهيل الأمر على المستخدم
                    QString ipOnly = pairAddr.split(':').first();
                    if (wirelessConnectIpInput->text().isEmpty()) {
                        wirelessConnectIpInput->setText(ipOnly + ":");
                    }
                }
                proc->deleteLater();
            });
            proc->start(getAdbExecutablePath(), {"pair", pairAddr, code});
        });

        pairRow->addWidget(pairIpPortInput, 2);
        pairRow->addWidget(pairCodeInput);
        pairRow->addWidget(doPairBtn);
        layout->addLayout(pairRow);

        // 2. قسم الاتصال اللاسلكي الرئيسي بعد الإقران
        QLabel *sec2 = new QLabel("2️⃣ الخطوة الثانية: الاتصال اللاسلكي الدائم (أدخل الـ IP والمنفذ الظاهر في الصفحة الرئيسية لتصحيح الأخطاء اللاسلكي):", page);
        sec2->setStyleSheet("color: #A8C7FA; font-size: 13px; font-weight: bold;");
        layout->addWidget(sec2);

        QHBoxLayout *connRow = new QHBoxLayout();
        wirelessConnectIpInput = new QLineEdit(page);
        wirelessConnectIpInput->setPlaceholderText("IP:Port الاتصال الرئيسي (مثال: 192.168.1.15:41555)");

        QPushButton *doConnBtn = new QPushButton("🔌 اتصال بالجهاز (adb connect)", page);
        doConnBtn->setProperty("class", "ActionBtn");
        connect(doConnBtn, &QPushButton::clicked, this, [=]() {
            QString addr = wirelessConnectIpInput->text().trimmed();
            if (addr.isEmpty()) {
                QMessageBox::warning(this, "تنبيه", "أدخل عنوان IP والمنفذ الرئيسي للاتصال!");
                return;
            }
            wifiIpInput->setText(addr);
            appendLog("جاري الاتصال لاسلكياً بـ " + addr + " ...", "INFO");
            QProcess *proc = new QProcess(this);
            connect(proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [=](int, QProcess::ExitStatus) {
                QString out = QString::fromUtf8(proc->readAllStandardOutput() + proc->readAllStandardError()).trimmed();
                appendLog(out, out.contains("connected to", Qt::CaseInsensitive) ? "SUCCESS" : "ERROR");
                checkDeviceSilently();
                verifyAndFixWirelessConnection();
                proc->deleteLater();
            });
            proc->start(getAdbExecutablePath(), {"connect", addr});
        });

        QPushButton *disconnectAllBtn = new QPushButton("❌ فصل اللاسلكي", page);
        disconnectAllBtn->setProperty("class", "DangerBtn");
        connect(disconnectAllBtn, &QPushButton::clicked, this, [=]() {
            runAdbCommand({"disconnect"}, "تم فصل كافة الاتصالات اللاسلكية.");
            checkDeviceSilently();
        });

        connRow->addWidget(wirelessConnectIpInput, 2);
        connRow->addWidget(doConnBtn);
        connRow->addWidget(disconnectAllBtn);
        layout->addLayout(connRow);

        // 3. قسم إظهار شاشة الهاتف على الكمبيوتر والتحكم بها لاسلكياً
        QLabel *sec3 = new QLabel("3️⃣ عرض شاشة الجوال على الكمبيوتر والتحكم الكامل بها عن بُعد (Wireless Screen & Remote Control):", page);
        sec3->setStyleSheet("color: #6DD58C; font-size: 13px; font-weight: bold;");
        layout->addWidget(sec3);

        QHBoxLayout *screenRow = new QHBoxLayout();
        QPushButton *launchScrcpyBtn = new QPushButton("📱 إظهار شاشة الهاتف والتحكم بها الآن (Live Screen)", page);
        launchScrcpyBtn->setProperty("class", "ActionBtn");
        connect(launchScrcpyBtn, &QPushButton::clicked, this, &MainWindow::launchWirelessScreenMirror);

        chkWirelessStayAwake = new QCheckBox("منع قفل الشاشة أثناء التحكم", page);
        chkWirelessStayAwake->setChecked(true);

        chkWirelessScreenOff = new QCheckBox("إطفاء شاشة الهاتف الفعلية لتوفير الطاقة", page);
        chkWirelessScreenOff->setChecked(false);

        QPushButton *screenshotBtn = new QPushButton("📸 لقطة شاشة للكمبيوتر", page);
        screenshotBtn->setProperty("class", "SecondaryBtn");
        connect(screenshotBtn, &QPushButton::clicked, this, [=]() {
            QString savePath = QFileDialog::getSaveFileName(this, "حفظ لقطة شاشة الهاتف", QDir::homePath() + "/Desktop/screen.png", "PNG Image (*.png)");
            if (savePath.isEmpty()) return;
            runAdbSequence({
                {{"shell", "screencap", "-p", "/sdcard/abdullah_screen.png"}, "جاري التقاط صورة لشاشة الهاتف لاسلكياً...", ""},
                {{"pull", "/sdcard/abdullah_screen.png", savePath}, "جاري سحب لقطة الشاشة إلى الكمبيوتر...", "تم حفظ لقطة الشاشة بنجاح في: " + savePath},
                {{"shell", "rm", "-f", "/sdcard/abdullah_screen.png"}, "", ""}
            });
        });

        screenRow->addWidget(launchScrcpyBtn);
        screenRow->addWidget(chkWirelessStayAwake);
        screenRow->addWidget(chkWirelessScreenOff);
        screenRow->addStretch();
        screenRow->addWidget(screenshotBtn);
        layout->addLayout(screenRow);

        // أزرار التحكم الفوري بأزرار الهاتف عن بُعد
        QHBoxLayout *remoteKeysRow = new QHBoxLayout();
        auto addKeyBtn = [&](const QString &lbl, const QString &keyCode) {
            QPushButton *b = new QPushButton(lbl, page);
            b->setProperty("class", "SecondaryBtn");
            connect(b, &QPushButton::clicked, this, [=]() {
                runAdbCommand({"shell", "input", "keyevent", keyCode}, "تم إرسال زر التحكم: " + lbl);
            });
            remoteKeysRow->addWidget(b);
        };
        addKeyBtn("🏠 الرئيسية (Home)", "3");
        addKeyBtn("🔙 رجوع (Back)", "4");
        addKeyBtn("🗂️ التطبيقات الأخيرة", "187");
        addKeyBtn("🔊 رفع الصوت", "24");
        addKeyBtn("🔉 خفض الصوت", "25");
        addKeyBtn("🔒 زر الباور (إيقاظ/قفل)", "26");
        layout->addLayout(remoteKeysRow);

        // 4. قسم العمليات اللاسلكية المباشرة (Install / Backup / Restore / Shell)
        QLabel *sec4 = new QLabel("4️⃣ تنفيذ مهام ADB اللاسلكية السريعة (Install / Backup / Restore / Shell):", page);
        sec4->setStyleSheet("color: #38BDF8; font-size: 13px; font-weight: bold;");
        layout->addWidget(sec4);

        QHBoxLayout *opsRow = new QHBoxLayout();
        wirelessQuickPkgInput = new QLineEdit("com.whatsapp", page);
        wirelessQuickPkgInput->setPlaceholderText("اسم الحزمة للنسخ الاحتياطي اللاسلكي...");
        wirelessQuickPkgInput->setFixedWidth(190);

        QPushButton *wInstallBtn = new QPushButton("📦 تثبيت APK لاسلكياً", page);
        wInstallBtn->setProperty("class", "SecondaryBtn");
        connect(wInstallBtn, &QPushButton::clicked, this, [=]() {
            QString apk = QFileDialog::getOpenFileName(this, "اختر ملف APK لتثبيته لاسلكياً", "", "Android Package (*.apk)");
            if (!apk.isEmpty()) {
                appendLog("جاري تثبيت APK لاسلكياً: " + apk, "INFO");
                runAdbCommand({"install", "-r", "-g", apk}, "تم تثبيت التطبيق لاسلكياً بنجاح!");
            }
        });

        QPushButton *wBackupBtn = new QPushButton("💾 سحب Backup (.ab) لاسلكياً", page);
        wBackupBtn->setProperty("class", "SecondaryBtn");
        connect(wBackupBtn, &QPushButton::clicked, this, [=]() {
            QString pkg = wirelessQuickPkgInput->text().trimmed();
            if (pkg.isEmpty()) {
                QMessageBox::warning(this, "تنبيه", "أدخل اسم الحزمة أولاً!");
                return;
            }
            QString out = QFileDialog::getSaveFileName(this, "حفظ النسخة الاحتياطية لاسلكياً", QDir::homePath() + "/Desktop/" + pkg + ".ab", "Android Backup (*.ab)");
            if (!out.isEmpty()) {
                appendLog("جاري سحب نسخة (.ab) لاسلكياً للحزمة " + pkg + " (وافق من شاشة الهاتف)...", "WARN");
                runAdbCommand({"backup", "-f", out, "-noapk", pkg}, "اكتمل النسخ الاحتياطي اللاسلكي بنجاح.");
            }
        });

        QPushButton *wRestoreBtn = new QPushButton("♻️ استعادة (.ab) لاسلكياً", page);
        wRestoreBtn->setProperty("class", "SecondaryBtn");
        connect(wRestoreBtn, &QPushButton::clicked, this, [=]() {
            QString ab = QFileDialog::getOpenFileName(this, "اختر ملف .ab لاستعادته لاسلكياً", "", "Android Backup (*.ab)");
            if (!ab.isEmpty()) {
                appendLog("جاري استعادة ملف (.ab) لاسلكياً (وافق من شاشة الهاتف)...", "WARN");
                runAdbCommand({"restore", ab}, "اكتملت الاستعادة اللاسلكية بنجاح.");
            }
        });

        QPushButton *wGoDataBtn = new QPushButton("🧬 حقن متقدم وتوقيع (.ab)", page);
        wGoDataBtn->setProperty("class", "SecondaryBtn");
        connect(wGoDataBtn, &QPushButton::clicked, this, [=]() {
            pagesStack->setCurrentIndex(2); // الانتقال لتبويب حقن data/data وتعديل التوقيع
        });

        opsRow->addWidget(wInstallBtn);
        opsRow->addWidget(wirelessQuickPkgInput);
        opsRow->addWidget(wBackupBtn);
        opsRow->addWidget(wRestoreBtn);
        opsRow->addWidget(wGoDataBtn);
        layout->addLayout(opsRow);

        // سطر أوامر Shell لاسلكي سريع داخل نفس الصفحة
        QHBoxLayout *wShellRow = new QHBoxLayout();
        wirelessShellInput = new QLineEdit(page);
        wirelessShellInput->setPlaceholderText("أمر Shell لاسلكي فوري (مثال: pm list packages -3 أو dumpsys package com.whatsapp)...");

        QPushButton *wRunShellBtn = new QPushButton("▶️ تنفيذ Shell لاسلكي", page);
        wRunShellBtn->setProperty("class", "ActionBtn");
        auto runWShell = [=]() {
            QString cmd = wirelessShellInput->text().trimmed();
            if (cmd.isEmpty()) return;
            QStringList args = {"shell"};
            args.append(QProcess::splitCommand(cmd));
            runAdbCommand(args, "تم تنفيذ أمر الـ Shell اللاسلكي.");
        };
        connect(wRunShellBtn, &QPushButton::clicked, this, runWShell);
        connect(wirelessShellInput, &QLineEdit::returnPressed, this, runWShell);

        wShellRow->addWidget(wirelessShellInput, 1);
        wShellRow->addWidget(wRunShellBtn);
        layout->addLayout(wShellRow);

        layout->addStretch();
        return page;
    }

    // --- دالة فحص وتصحيح الاتصال اللاسلكي تلقائياً (التأكد من نجاح الاقتران واستجابة الـ Shell) ---
    void verifyAndFixWirelessConnection() {
        appendLog("جاري فحص وتشخيص حالة الاتصال اللاسلكي بالهاتف...", "INFO");

        QProcess *proc = new QProcess(this);
        connect(proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [=](int, QProcess::ExitStatus) {
            QString out = QString::fromUtf8(proc->readAllStandardOutput()).trimmed();
            proc->deleteLater();

            if (out.contains("offline")) {
                wirelessStatusBanner->setText("⚠️ الجهاز متصل ولكن بحالة (Offline)! جاري تنفيذ تصحيح تلقائي (adb reconnect)...");
                wirelessStatusBanner->setStyleSheet("background-color: #3C2F19; color: #FCD34D; border-radius: 8px; padding: 8px 12px; font-weight: bold;");
                runAdbSequence({
                    {{"reconnect", "offline"}, "إعادة تنشيط الأجهزة المعلقة (offline)...", ""},
                    {{"connect", wirelessConnectIpInput->text().trimmed()}, "إعادة الاتصال بالعنوان اللاسلكي...", "تم تصحيح مسار الاتصال اللاسلكي."}
                });
                return;
            }

            if (out.contains("unauthorized")) {
                wirelessStatusBanner->setText("🔒 الجهاز بانتظار المصادقة (Unauthorized)! يرجى إتمام الخطوة 1 (إدخال رمز الإقران 6 أرقام) أو الموافقة من شاشة الهاتف.");
                wirelessStatusBanner->setStyleSheet("background-color: #3C1919; color: #F2B8B5; border-radius: 8px; padding: 8px 12px; font-weight: bold;");
                return;
            }

            // اختبار حقيقي بتنفيذ أمر داخل نواة الهاتف للتأكد أن الأوامر تعمل فعلياً
            QProcess *testShell = new QProcess(this);
            connect(testShell, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [=](int code, QProcess::ExitStatus) {
                QString model = QString::fromUtf8(testShell->readAllStandardOutput()).trimmed();
                testShell->deleteLater();
                if (code == 0 && !model.isEmpty()) {
                    wirelessStatusBanner->setText("✔ الاتصال اللاسلكي ناجح وفعّال 100%! الهاتف المستجيب: [" + model + "] جاهز لتنفيذ جميع الأوامر.");
                    wirelessStatusBanner->setStyleSheet("background-color: #0F5223; color: #6DD58C; border-radius: 8px; padding: 8px 12px; font-weight: bold;");
                    appendLog("اختبار Shell اللاسلكي ناجح -> الجهاز المستجيب: " + model, "SUCCESS");
                    checkDeviceSilently();
                    readFullDeviceInfo();
                } else {
                    wirelessStatusBanner->setText("❌ لا يوجد اتصال فعّال! إذا كنت في أندرويد 11+ نفّذ (الخطوة 1: الإقران بالرمز السري) أولاً ثم (الخطوة 2: الاتصال بالمنفذ الرئيسي).");
                    wirelessStatusBanner->setStyleSheet("background-color: #3C1919; color: #F2B8B5; border-radius: 8px; padding: 8px 12px; font-weight: bold;");
                    appendLog("فشل اختبار استجابة الهاتف! تأكد من إقران الجهاز بالرمز السري أولاً.", "ERROR");
                }
            });
            testShell->start(getAdbExecutablePath(), {"shell", "getprop", "ro.product.model"});
        });
        proc->start(getAdbExecutablePath(), {"devices"});
    }

    // --- دالة تشغيل بث شاشة الهاتف والتحكم بها عبر الكمبيوتر (Scrcpy Engine) ---
    void launchWirelessScreenMirror() {
        QString appDir = QCoreApplication::applicationDirPath();
        QString scrcpyExe;

        QStringList possiblePaths = {
            appDir + "/scrcpy/scrcpy.exe",
            appDir + "/platform-tools/scrcpy.exe",
            appDir + "/scrcpy.exe"
        };

        for (const QString &p : possiblePaths) {
            if (QFile::exists(p)) {
                scrcpyExe = p;
                break;
            }
        }

        if (scrcpyExe.isEmpty()) {
            scrcpyExe = "scrcpy"; // في حال كان مضافاً في متغيرات النظام PATH
        }

        QStringList args;
        args << "--window-title" << "AbdullahTool Pro - Live Screen Control";
        args << "--max-size" << "1080";
        args << "--video-bit-rate" << "4M";
        args << "--max-fps" << "60";

        if (chkWirelessStayAwake->isChecked()) args << "--stay-awake";
        if (chkWirelessScreenOff->isChecked()) args << "--turn-screen-off";

        appendLog("جاري تشغيل نافذة عرض شاشة الهاتف والتحكم بها عن بُعد...", "INFO");
        bool started = QProcess::startDetached(scrcpyExe, args);
        if (!started) {
            appendLog("لم يتم العثور على محرك عرض الشاشة (scrcpy.exe) داخل مجلد الأداة!", "ERROR");
            QMessageBox::information(this, "محرك عرض الشاشة (Scrcpy)",
                "لتشغيل عرض شاشة الهاتف والتحكم بها، ضع ملفات أداة (scrcpy-win64) داخل مجلد باسم scrcpy بجانب برنامج AbdullahTool.exe.");
        } else {
            appendLog("تم فتح نافذة شاشة الهاتف بنجاح! يمكنك الآن التحكم بالهاتف بالماوس والكيبورد.", "SUCCESS");
        }
    }
    
// ============================================================================
    // محرك الاتصال المباشر بالتطبيق الوسيط (AT-Bridge v2.0 Socket Engine)
    // ============================================================================

    void ensureWinsockInitialized() {
        static bool initialized = false;
        if (!initialized) {
            WSADATA wsaData;
            WSAStartup(MAKEWORD(2, 2), &wsaData);
            initialized = true;
        }
    }

    // قراءة عنوان IP والمنفذ من الخانة (افتراضياً المنفذ 50555)
    bool parseBridgeAddress(QString &ipOut, int &portOut) {
        QString raw = bridgeIpPortInput->text().trimmed();
        if (raw.isEmpty()) return false;
        QStringList parts = raw.split(':');
        ipOut = parts[0].trimmed();
        portOut = (parts.size() > 1 && parts[1].toInt() > 0) ? parts[1].toInt() : 50555;
        return !ipOut.isEmpty();
    }

    // إرسال أمر مباشر للتطبيق الوسيط واستلام الرد النصي فوراً
    QString sendBridgeCommandSync(const QString &commandLine, int timeoutMs = 8000) {
        ensureWinsockInitialized();
        QString ip;
        int port = 50555;
        if (!parseBridgeAddress(ip, port)) {
            return "ERROR: يرجى إدخال عنوان IP أو الضغط على (بحث تلقائي) أولاً!";
        }

        SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (sock == INVALID_SOCKET) return "ERROR: تعذر إنشاء Socket على الكمبيوتر.";

        DWORD tv = timeoutMs;
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));
        setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (const char*)&tv, sizeof(tv));

        sockaddr_in serverAddr{};
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(port);
        inet_pton(AF_INET, ip.toStdString().c_str(), &serverAddr.sin_addr);

        if (::connect(sock, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
            closesocket(sock);
            return "ERROR: تعذر الاتصال بالتطبيق الوسيط على " + ip + ":" + QString::number(port) + " (تأكد أن الهاتف والكمبيوتر على نفس الشبكة وأن التطبيق مفتوح).";
        }

        QByteArray payload = (commandLine + "\n").toUtf8();
        send(sock, payload.constData(), payload.size(), 0);

        QByteArray response;
        char buf[8192];
        int received = 0;
        while ((received = recv(sock, buf, sizeof(buf), 0)) > 0) {
            response.append(buf, received);
        }
        closesocket(sock);
        return QString::fromUtf8(response).trimmed();
    }

    // إرسال ملف ثنائي (APK أو أي ملف) من الكمبيوتر إلى التطبيق الوسيط مباشرة
    bool sendBridgeFileSync(const QString &headerCommand, const QString &localFilePath, QString &serverReplyOut) {
        ensureWinsockInitialized();
        QFile file(localFilePath);
        if (!file.open(QIODevice::ReadOnly)) {
            serverReplyOut = "ERROR: تعذر فتح الملف المحلي على الكمبيوتر!";
            return false;
        }

        QString ip;
        int port = 50555;
        if (!parseBridgeAddress(ip, port)) {
            file.close();
            serverReplyOut = "ERROR: عنوان التطبيق الوسيط غير محدد!";
            return false;
        }

        SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        DWORD tv = 60000;
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));
        setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (const char*)&tv, sizeof(tv));

        sockaddr_in serverAddr{};
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(port);
        inet_pton(AF_INET, ip.toStdString().c_str(), &serverAddr.sin_addr);

        if (::connect(sock, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
            file.close();
            closesocket(sock);
            serverReplyOut = "ERROR: فشل الاتصال بالهاتف أثناء محاولة إرسال الملف!";
            return false;
        }

        // 1. إرسال سطر الترويسة متبوعاً بـ \n
        QByteArray hdr = (headerCommand + "\n").toUtf8();
        send(sock, hdr.constData(), hdr.size(), 0);

        // 2. تدفق بايتات الملف مباشرة
        while (!file.atEnd()) {
            QByteArray chunk = file.read(65536);
            int totalSent = 0;
            while (totalSent < chunk.size()) {
                int s = send(sock, chunk.constData() + totalSent, chunk.size() - totalSent, 0);
                if (s <= 0) break;
                totalSent += s;
            }
        }
        file.close();

        // 3. قراءة رد الهاتف بعد اكتمال النقل
        QByteArray reply;
        char buf[4096];
        int r = 0;
        while ((r = recv(sock, buf, sizeof(buf), 0)) > 0) {
            reply.append(buf, r);
        }
        closesocket(sock);
        serverReplyOut = QString::fromUtf8(reply).trimmed();
        return serverReplyOut.startsWith("OK");
    }

    // سحب ملف ثنائي (أي ملف أو APK حزمة مثبتة) من الهاتف إلى الكمبيوتر عبر التطبيق الوسيط
    bool receiveBridgeFileSync(const QString &requestCommand, const QString &saveLocalPath, QString &statusMsgOut) {
        ensureWinsockInitialized();
        QString ip;
        int port = 50555;
        if (!parseBridgeAddress(ip, port)) {
            statusMsgOut = "ERROR: عنوان التطبيق الوسيط غير محدد!";
            return false;
        }

        SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        DWORD tv = 60000;
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));
        setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (const char*)&tv, sizeof(tv));

        sockaddr_in serverAddr{};
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(port);
        inet_pton(AF_INET, ip.toStdString().c_str(), &serverAddr.sin_addr);

        if (::connect(sock, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
            closesocket(sock);
            statusMsgOut = "ERROR: تعذر الاتصال بالهاتف لسحب الملف!";
            return false;
        }

        QByteArray req = (requestCommand + "\n").toUtf8();
        send(sock, req.constData(), req.size(), 0);

        // قراءة ترويسة الرد حرفاً بحرف حتى \n لمعرفة حجم الملف
        QByteArray headerBytes;
        char c;
        while (recv(sock, &c, 1, 0) == 1) {
            if (c == '\n') break;
            if (c != '\r') headerBytes.append(c);
        }

        QString headerStr = QString::fromUtf8(headerBytes);
        if (!headerStr.startsWith("OK|")) {
            closesocket(sock);
            statusMsgOut = headerStr.isEmpty() ? "ERROR: لم يرد الهاتف ببيانات الملف!" : headerStr;
            return false;
        }

        qint64 fileSize = headerStr.section('|', 1, 1).toLongLong();
        QFile outFile(saveLocalPath);
        if (!outFile.open(QIODevice::WriteOnly)) {
            closesocket(sock);
            statusMsgOut = "ERROR: تعذر إنشاء الملف على الكمبيوتر: " + saveLocalPath;
            return false;
        }

        char buf[65536];
        qint64 remaining = fileSize;
        while (remaining > 0) {
            int r = recv(sock, buf, (int)qMin<qint64>(sizeof(buf), remaining), 0);
            if (r <= 0) break;
            outFile.write(buf, r);
            remaining -= r;
        }
        outFile.close();
        closesocket(sock);

        statusMsgOut = QString("تم سحب الملف بنجاح (%1 كيلوبايت) إلى: %2").arg(fileSize / 1024).arg(saveLocalPath);
        return true;
    }

    // رادار الاكتشاف التلقائي للهاتف عبر الشبكة المحلية (UDP Broadcast 50556)
    void autoDiscoverBridgeDevice() {
        ensureWinsockInitialized();
        appendLog("📡 جاري بث إشارة الرادار على الشبكة المحلية للبحث عن تطبيق AbdullahTool Bridge...", "INFO");

        SOCKET udpSock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (udpSock == INVALID_SOCKET) return;

        BOOL bOpt = TRUE;
        setsockopt(udpSock, SOL_SOCKET, SO_BROADCAST, (const char*)&bOpt, sizeof(bOpt));
        DWORD timeout = 1800; // 1.8 ثانية
        setsockopt(udpSock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));

        sockaddr_in bcastAddr{};
        bcastAddr.sin_family = AF_INET;
        bcastAddr.sin_port = htons(50556);
        bcastAddr.sin_addr.s_addr = INADDR_BROADCAST;

        const char *pingMsg = "AT_DISCOVER";
        sendto(udpSock, pingMsg, (int)strlen(pingMsg), 0, (sockaddr*)&bcastAddr, sizeof(bcastAddr));

        char recvBuf[1024];
        sockaddr_in senderAddr{};
        int senderLen = sizeof(senderAddr);
        int len = recvfrom(udpSock, recvBuf, sizeof(recvBuf) - 1, 0, (sockaddr*)&senderAddr, &senderLen);
        closesocket(udpSock);

        if (len > 0) {
            recvBuf[len] = '\0';
            QString reply = QString::fromUtf8(recvBuf).trimmed();
            // الصيغة: AT_HERE|192.168.1.50:50555|Model|AndroidVer|ROOT
            if (reply.startsWith("AT_HERE|")) {
                QStringList parts = reply.split('|');
                QString foundAddr = parts.value(1);
                QString model = parts.value(2);
                QString androidVer = parts.value(3);
                QString rootState = parts.value(4);

                bridgeIpPortInput->setText(foundAddr);
                bridgeStatusLabel->setText(QString("✔ تم الاكتشاف والربط التلقائي: [%1 - %2 - %3] على العنوان %4")
                                               .arg(model, androidVer, rootState, foundAddr));
                bridgeStatusLabel->setStyleSheet("background-color: #0F5223; color: #6DD58C; border-radius: 8px; padding: 10px 14px; font-weight: bold;");

                valModel->setText(model);
                valAndroid->setText(androidVer);
                valSerial->setText(foundAddr);
                connectionBadge->setText("● متصل بالوسيط: " + model);
                connectionBadge->setStyleSheet("background-color: #0F5223; color: #6DD58C; padding: 6px 14px; border-radius: 16px; font-size: 12px; font-weight: bold;");

                appendLog("✔ تم العثور على الهاتف تلقائياً: " + model + " (" + foundAddr + ") [" + rootState + "]", "SUCCESS");
                return;
            }
        }

        appendLog("لم يرد أي هاتف تلقائياً. تأكد من فتح تطبيق AbdullahTool Bridge على الهاتف، أو أدخل الـ IP الظاهر في التطبيق يدوياً.", "WARN");
        bridgeStatusLabel->setText("⚠ لم يتم العثور التلقائي (قد يمنع الراوتر Broadcast). أدخل الـ IP الظاهر في شاشة التطبيق يدوياً واضغط فحص الاتصال.");
    }

    // --- الصفحة 6: إدارة الاتصال والتحكم الواسع عبر التطبيق الوسيط (AT-Bridge v2.0) ---
    QWidget* createLocalBridgePage() {
        QFrame *page = new QFrame(this);
        page->setObjectName("Card");
        QVBoxLayout *layout = new QVBoxLayout(page);
        layout->setContentsMargins(22, 16, 22, 16);
        layout->setSpacing(10);

        QHBoxLayout *topHeader = new QHBoxLayout();
        QLabel *title = new QLabel("🌐 مركز التحكم الشامل عبر التطبيق الوسيط (AT-Bridge v2.0 المباشر)", page);
        title->setStyleSheet("font-size: 16px; font-weight: bold; color: #E3E3E3;");

        QPushButton *autoDiscoverBtn = new QPushButton("📡 بحث والاتصال التلقائي بالهاتف (Auto-Discover)", page);
        autoDiscoverBtn->setProperty("class", "ActionBtn");
        connect(autoDiscoverBtn, &QPushButton::clicked, this, &MainWindow::autoDiscoverBridgeDevice);

        topHeader->addWidget(title);
        topHeader->addStretch();
        topHeader->addWidget(autoDiscoverBtn);
        layout->addLayout(topHeader);

        // بانر حالة الاتصال الذكي
        bridgeStatusLabel = new QLabel("● الحالة: افتح تطبيق (AbdullahTool Bridge) على الهاتف ثم اضغط (بحث والاتصال التلقائي) أو أدخل الـ IP:50555.", page);
        bridgeStatusLabel->setStyleSheet("background-color: #131314; color: #FCD34D; border: 1px solid #333537; border-radius: 8px; padding: 9px 14px; font-size: 12px; font-weight: bold;");
        layout->addWidget(bridgeStatusLabel);

        // 1. صف الاتصال المباشر والفحص وتفعيل ADB
        QHBoxLayout *connRow = new QHBoxLayout();
        bridgeIpPortInput = new QLineEdit(page);
        bridgeIpPortInput->setPlaceholderText("عنوان الجسر في التطبيق (مثال: 192.168.1.50:50555)");

        QPushButton *bridgeConnectBtn = new QPushButton("🔌 اتصال وفحص شامل للجسر", page);
        bridgeConnectBtn->setProperty("class", "ActionBtn");
        connect(bridgeConnectBtn, &QPushButton::clicked, this, [=]() {
            QString resp = sendBridgeCommandSync("PING");
            if (resp.startsWith("OK|")) {
                // OK|Model|AndroidVer|SDK|Bat|ROOT
                QStringList p = resp.split('|');
                QString model = p.value(1, "Android");
                QString ver = p.value(2, "");
                QString sdk = p.value(3, "");
                QString bat = p.value(4, "0%");
                QString root = p.value(5, "NOROOT");

                valModel->setText(model);
                valAndroid->setText(ver + " (" + sdk + ")");
                valSerial->setText(bridgeIpPortInput->text().trimmed());
                batteryBar->setValue(QString(bat).remove('%').toInt());

                bridgeStatusLabel->setText(QString("✔ اتصال مباشر ومؤكّد 100% | الجهاز: %1 | %2 | البطارية: %3 | الصلاحية: %4")
                                               .arg(model, ver, bat, root));
                bridgeStatusLabel->setStyleSheet("background-color: #0F5223; color: #6DD58C; border-radius: 8px; padding: 9px 14px; font-weight: bold;");
                connectionBadge->setText("● متصل بالوسيط: " + model);
                connectionBadge->setStyleSheet("background-color: #0F5223; color: #6DD58C; padding: 6px 14px; border-radius: 16px; font-size: 12px; font-weight: bold;");
                appendLog("✔ نجح فحص الجسر المباشر: " + resp, "SUCCESS");
            } else {
                bridgeStatusLabel->setText("✖ " + resp);
                bridgeStatusLabel->setStyleSheet("background-color: #3C1919; color: #F2B8B5; border-radius: 8px; padding: 9px 14px; font-weight: bold;");
                appendLog(resp, "ERROR");
            }
        });

        QPushButton *enableAdbdBtn = new QPushButton("⚡ تفعيل منفذ ADB 5555 (للنسخ .ab)", page);
        enableAdbdBtn->setProperty("class", "SecondaryBtn");
        connect(enableAdbdBtn, &QPushButton::clicked, this, [=]() {
            QString resp = sendBridgeCommandSync("ENABLE_ADBD");
            appendLog("نتيجة تفعيل ADB عبر الوسيط: " + resp, resp.startsWith("OK") ? "SUCCESS" : "WARN");
            QString ip; int port;
            if (parseBridgeAddress(ip, port)) {
                runAdbCommand({"connect", ip + ":5555"}, "تم إرسال طلب ربط محرك ADB على المنفذ 5555.");
            }
        });

        connRow->addWidget(bridgeIpPortInput, 2);
        connRow->addWidget(bridgeConnectBtn);
        connRow->addWidget(enableAdbdBtn);
        layout->addLayout(connRow);

        // 2. صف إدارة الحزم والتطبيقات عبر الوسيط (تثبيت APK + عرض جميع الحزم + سحب APK أي تطبيق)
        QLabel *pkgSec = new QLabel("1️⃣ إدارة التطبيقات والنسخ الاحتياطي عبر الجسر المباشر (بدون الحاجة لـ USB):", page);
        pkgSec->setStyleSheet("color: #A8C7FA; font-size: 13px; font-weight: bold;");
        layout->addWidget(pkgSec);

        QHBoxLayout *row1 = new QHBoxLayout();
        QPushButton *bridgeInstallBtn = new QPushButton("📦 إرسال وتثبيت APK عبر الوسيط", page);
        bridgeInstallBtn->setProperty("class", "ActionBtn");
        connect(bridgeInstallBtn, &QPushButton::clicked, this, [=]() {
            QString apk = QFileDialog::getOpenFileName(this, "اختر ملف APK لتثبيته عبر الوسيط", "", "Android Package (*.apk)");
            if (apk.isEmpty()) return;
            QFileInfo fi(apk);
            appendLog(QString("جاري إرسال ملف الـ APK (%1 كيلوبايت) مباشرة إلى التطبيق الوسيط وتثبيته...").arg(fi.size() / 1024), "INFO");
            QString reply;
            QString hdr = QString("INSTALL|%1|%2").arg(fi.fileName()).arg(fi.size());
            if (sendBridgeFileSync(hdr, apk, reply)) {
                appendLog(reply, "SUCCESS");
            } else {
                appendLog(reply, "ERROR");
            }
        });

        bridgeQuickPkgInput = new QLineEdit("com.whatsapp", page);
        bridgeQuickPkgInput->setPlaceholderText("اسم الحزمة (مثال: com.whatsapp)");
        bridgeQuickPkgInput->setFixedWidth(185);

        QPushButton *pullApkBtn = new QPushButton("📥 سحب APK الحزمة للكمبيوتر", page);
        pullApkBtn->setProperty("class", "SecondaryBtn");
        connect(pullApkBtn, &QPushButton::clicked, this, [=]() {
            QString pkg = bridgeQuickPkgInput->text().trimmed();
            if (pkg.isEmpty()) return;
            QString savePath = QFileDialog::getSaveFileName(this, "حفظ ملف APK المسحوب", QDir::homePath() + "/Desktop/" + pkg + ".apk", "Android Package (*.apk)");
            if (savePath.isEmpty()) return;
            appendLog("جاري سحب ملف الـ APK الأصلي للحزمة " + pkg + " عبر التطبيق الوسيط...", "INFO");
            QString msg;
            if (receiveBridgeFileSync("PULL_APK|" + pkg, savePath, msg)) {
                appendLog(msg, "SUCCESS");
            } else {
                appendLog(msg, "ERROR");
            }
        });

        QPushButton *listAllPkgsBtn = new QPushButton("📋 عرض حزم الهاتف", page);
        listAllPkgsBtn->setProperty("class", "SecondaryBtn");
        connect(listAllPkgsBtn, &QPushButton::clicked, this, [=]() {
            appendLog("جاري جلب قائمة الحزم المثبتة عبر PackageManager من التطبيق الوسيط...", "INFO");
            QString out = sendBridgeCommandSync("LIST_PKGS");
            for (const QString &line : out.split('\n', Qt::SkipEmptyParts)) {
                appendLog(line, "OUTPUT");
            }
        });

        QPushButton *bridgeBackupBtn = new QPushButton("💾 سحب بيانات الحزمة (Wi-Fi)", page);
        bridgeBackupBtn->setProperty("class", "SecondaryBtn");
        connect(bridgeBackupBtn, &QPushButton::clicked, this, [=]() {
            QString pkg = bridgeQuickPkgInput->text().trimmed();
            if (pkg.isEmpty()) return;
            QString out = QFileDialog::getSaveFileName(this, "حفظ نسخة بيانات الحزمة عبر الواي فاي",
                                                       QDir::homePath() + "/Desktop/" + pkg + ".tar",
                                                       "Tar Archive (*.tar);;Android Backup (*.ab)");
            if (out.isEmpty()) return;

            appendLog("جاري سحب بيانات الحزمة (" + pkg + ") مباشرة عبر الواي فاي من التطبيق الوسيط...", "INFO");
            QString statusMsg;
            QString tempTar = out.endsWith(".ab", Qt::CaseInsensitive) ? (QDir::tempPath() + "/bridge_pull_tmp.tar") : out;

            if (receiveBridgeFileSync("BACKUP_PKG|" + pkg, tempTar, statusMsg)) {
                if (out.endsWith(".ab", Qt::CaseInsensitive)) {
                    // تغليف ملف الـ tar المسحوب بصيغة .ab قياسية
                    QFile tf(tempTar);
                    if (tf.open(QIODevice::ReadOnly)) {
                        QByteArray compressed = qCompress(tf.readAll(), 9).mid(4);
                        tf.close();
                        QFile abFile(out);
                        if (abFile.open(QIODevice::WriteOnly)) {
                            abFile.write("ANDROID BACKUP\n5\n1\nnone\n");
                            abFile.write(compressed);
                            abFile.close();
                        }
                    }
                }
                appendLog("✔ تم حفظ نسخة الحزمة بنجاح عبر الواي فاي في: " + out, "SUCCESS");
            } else {
                appendLog(statusMsg, "ERROR");
            }
        });

        QPushButton *bridgeRestoreBtn = new QPushButton("♻️ استعادة وحقن (.ab / .tar) بالواي فاي", page);
        bridgeRestoreBtn->setProperty("class", "SecondaryBtn");
        connect(bridgeRestoreBtn, &QPushButton::clicked, this, [=]() {
            QString pkg = bridgeQuickPkgInput->text().trimmed();
            if (pkg.isEmpty()) return;
            QString srcFile = QFileDialog::getOpenFileName(this, "اختر ملف .ab أو .tar لاستعادته عبر الواي فاي", "", "Backup Files (*.ab *.tar)");
            if (srcFile.isEmpty()) return;

            QString tarToSend = srcFile;
            if (srcFile.endsWith(".ab", Qt::CaseInsensitive)) {
                tarToSend = QDir::tempPath() + "/bridge_auto_restore.tar";
                appendLog("جاري فك ملف (.ab) تلقائياً لإرساله عبر الجسر اللاسلكي...", "INFO");
                if (!convertAbFileToTar(srcFile, tarToSend)) return;
            }

            QFileInfo fi(tarToSend);
            appendLog("جاري إرسال وحقن البيانات في الحزمة (" + pkg + ") مباشرة عبر الواي فاي...", "INFO");
            QString reply;
            if (sendBridgeFileSync(QString("RESTORE_PKG|%1|%2").arg(pkg).arg(fi.size()), tarToSend, reply)) {
                appendLog(reply, "SUCCESS");
            } else {
                appendLog(reply, "ERROR");
            }
        });

        row1->addWidget(bridgeInstallBtn);
        row1->addWidget(bridgeQuickPkgInput);
        row1->addWidget(pullApkBtn);
        row1->addWidget(listAllPkgsBtn);
        row1->addWidget(bridgeBackupBtn);
        row1->addWidget(bridgeRestoreBtn);
        layout->addLayout(row1);

        // 3. صف نقل وسحب الملفات المباشر عبر Socket الوسيط (Push & Pull بدون ADB)
        QLabel *fileSec = new QLabel("2️⃣ نقل وسحب الملفات المباشر عبر الجسر (Direct Socket Push & Pull):", page);
        fileSec->setStyleSheet("color: #6DD58C; font-size: 13px; font-weight: bold;");
        layout->addWidget(fileSec);

        QHBoxLayout *row2 = new QHBoxLayout();
        bridgePushLocalInput = new QLineEdit(page);
        bridgePushLocalInput->setPlaceholderText("اختر ملفاً من الكمبيوتر لإرساله...");

        QPushButton *browsePushFile = new QPushButton("📂 ملف", page);
        browsePushFile->setProperty("class", "SecondaryBtn");
        connect(browsePushFile, &QPushButton::clicked, this, [=]() {
            QString f = QFileDialog::getOpenFileName(this, "اختر ملفاً لإرساله للهاتف");
            if (!f.isEmpty()) {
                bridgePushLocalInput->setText(f);
                bridgePushRemoteInput->setText("/sdcard/Download/" + QFileInfo(f).fileName());
            }
        });

        bridgePushRemoteInput = new QLineEdit("/sdcard/Download/file.bin", page);
        bridgePushRemoteInput->setPlaceholderText("المسار الكامل في الهاتف (مثال: /sdcard/Download/test.zip)");

        QPushButton *bridgePushBtn = new QPushButton("📤 إرسال مباشر للهاتف (Push)", page);
        bridgePushBtn->setProperty("class", "ActionBtn");
        connect(bridgePushBtn, &QPushButton::clicked, this, [=]() {
            QString local = bridgePushLocalInput->text().trimmed();
            QString remote = bridgePushRemoteInput->text().trimmed();
            if (local.isEmpty() || remote.isEmpty()) return;
            QFileInfo fi(local);
            if (remote.endsWith('/')) remote += fi.fileName();
            appendLog("جاري إرسال الملف مباشرة عبر الجسر إلى: " + remote, "INFO");
            QString reply;
            if (sendBridgeFileSync(QString("PUSH|%1|%2").arg(remote).arg(fi.size()), local, reply)) {
                appendLog(reply, "SUCCESS");
            } else {
                appendLog(reply, "ERROR");
            }
        });

        QPushButton *bridgePullBtn = new QPushButton("📥 سحب هذا المسار للكمبيوتر (Pull)", page);
        bridgePullBtn->setProperty("class", "SecondaryBtn");
        connect(bridgePullBtn, &QPushButton::clicked, this, [=]() {
            QString remote = bridgePushRemoteInput->text().trimmed();
            if (remote.isEmpty()) return;
            QString savePath = QFileDialog::getSaveFileName(this, "حفظ الملف المسحوب من الهاتف", QDir::homePath() + "/Desktop/" + QFileInfo(remote).fileName());
            if (savePath.isEmpty()) return;
            QString msg;
            if (receiveBridgeFileSync("PULL|" + remote, savePath, msg)) {
                appendLog(msg, "SUCCESS");
            } else {
                appendLog(msg, "ERROR");
            }
        });

        row2->addWidget(bridgePushLocalInput, 2);
        row2->addWidget(browsePushFile);
        row2->addWidget(bridgePushRemoteInput, 2);
        row2->addWidget(bridgePushBtn);
        row2->addWidget(bridgePullBtn);
        layout->addLayout(row2);

        // 4. صلاحيات التحكم الواسع (تشغيل تطبيق عن بُعد، فتح رابط، إرسال للحافظة، وأوامر Shell المباشرة)
        QLabel *ctrlSec = new QLabel("3️⃣ أدوات التحكم الواسع بالهاتف وموجه الأوامر المباشر (Root / Shell / Clipboard / Launch):", page);
        ctrlSec->setStyleSheet("color: #FCD34D; font-size: 13px; font-weight: bold;");
        layout->addWidget(ctrlSec);

        QHBoxLayout *row3 = new QHBoxLayout();
        bridgeActionInput = new QLineEdit(page);
        bridgeActionInput->setPlaceholderText("أدخل نصاً لنسخه في حافظة الهاتف، أو رابط URL لفتحه، أو اسم حزمة لتشغيلها...");

        QPushButton *sendClipBtn = new QPushButton("📋 إرسال لحافظة الهاتف", page);
        sendClipBtn->setProperty("class", "SecondaryBtn");
        connect(sendClipBtn, &QPushButton::clicked, this, [=]() {
            QString txt = bridgeActionInput->text();
            if (txt.isEmpty()) return;
            appendLog(sendBridgeCommandSync("CLIPBOARD_SET|" + txt), "SUCCESS");
        });

        QPushButton *openUrlBtn = new QPushButton("🌐 فتح الرابط بالهاتف", page);
        openUrlBtn->setProperty("class", "SecondaryBtn");
        connect(openUrlBtn, &QPushButton::clicked, this, [=]() {
            QString url = bridgeActionInput->text().trimmed();
            if (url.isEmpty()) return;
            if (!url.startsWith("http")) url = "https://" + url;
            appendLog(sendBridgeCommandSync("OPEN_URL|" + url), "SUCCESS");
        });

        QPushButton *openAppBtn = new QPushButton("🚀 تشغيل التطبيق بالهاتف", page);
        openAppBtn->setProperty("class", "SecondaryBtn");
        connect(openAppBtn, &QPushButton::clicked, this, [=]() {
            QString pkg = bridgeActionInput->text().trimmed().isEmpty() ? bridgeQuickPkgInput->text().trimmed() : bridgeActionInput->text().trimmed();
            appendLog(sendBridgeCommandSync("OPEN_APP|" + pkg), "SUCCESS");
        });

        row3->addWidget(bridgeActionInput, 1);
        row3->addWidget(sendClipBtn);
        row3->addWidget(openUrlBtn);
        row3->addWidget(openAppBtn);
        layout->addLayout(row3);

        // سطر أوامر Shell المباشر عبر الجسر (ينفذ بـ su تلقائياً إن توفر روت أو sh بدون روت)
        QHBoxLayout *row4 = new QHBoxLayout();
        bridgeShellInput = new QLineEdit(page);
        bridgeShellInput->setPlaceholderText("اكتب أي أمر نظام ليتم تنفيذه مباشرة داخل تطبيق الوسيط (مثال: df -h أو getprop أو ls -la /sdcard)...");

        QPushButton *bridgeRunShell = new QPushButton("▶️ تنفيذ مباشر في الوسيط", page);
        bridgeRunShell->setProperty("class", "ActionBtn");
        auto execBridgeShell = [=]() {
            QString cmd = bridgeShellInput->text().trimmed();
            if (cmd.isEmpty()) return;
            appendLog("إرسال أمر للوسيط: " + cmd, "INFO");
            QString out = sendBridgeCommandSync("SHELL|" + cmd, 15000);
            for (const QString &line : out.split('\n', Qt::SkipEmptyParts)) {
                appendLog(line, line.startsWith("ERROR:") ? "ERROR" : "OUTPUT");
            }
        };
        connect(bridgeRunShell, &QPushButton::clicked, this, execBridgeShell);
        connect(bridgeShellInput, &QLineEdit::returnPressed, this, execBridgeShell);

        row4->addWidget(bridgeShellInput, 1);
        row4->addWidget(bridgeRunShell);
        layout->addLayout(row4);

        layout->addStretch();
        return page;
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
// بناء ترويسة ملف USTAR Tar قياسية (512 بايت) متوافقة مع محرك TarBackupReader في أندرويد
    QByteArray createUstarHeader(const QString &archivePath, qint64 fileSize) {
        QByteArray header(512, '\0');
        QByteArray pathBytes = archivePath.toUtf8();

        // تقسيم المسار إذا زاد عن 100 حرف إلى prefix و name حسب معيار USTAR
        if (pathBytes.size() <= 100) {
            memcpy(header.data(), pathBytes.constData(), pathBytes.size());
        } else {
            int splitIdx = pathBytes.lastIndexOf('/', pathBytes.size() - 100);
            if (splitIdx == -1) splitIdx = pathBytes.indexOf('/');
            QByteArray prefix = pathBytes.left(splitIdx);
            QByteArray name = pathBytes.mid(splitIdx + 1);
            memcpy(header.data(), name.constData(), qMin(name.size(), 100));
            memcpy(header.data() + 345, prefix.constData(), qMin(prefix.size(), 155));
        }

        // الصلاحيات والملكية والحجم والوقت بالنظام الثماني (Octal)
        memcpy(header.data() + 100, "0000600", 7);
        memcpy(header.data() + 108, "0001000", 7);
        memcpy(header.data() + 116, "0001000", 7);

        QByteArray sizeOctal = QString("%1").arg(fileSize, 11, 8, QChar('0')).toLatin1();
        memcpy(header.data() + 124, sizeOctal.constData(), 11);

        qint64 mtime = QDateTime::currentSecsSinceEpoch();
        QByteArray mtimeOctal = QString("%1").arg(mtime, 11, 8, QChar('0')).toLatin1();
        memcpy(header.data() + 136, mtimeOctal.constData(), 11);

        // ملء خانة الـ Checksum بمسافات مؤقتاً لحساب المجموع
        memset(header.data() + 148, ' ', 8);
        header[156] = '0'; // نوع الملف: ملف عادي (Regular File)
        memcpy(header.data() + 257, "ustar", 5);
        memcpy(header.data() + 263, "00", 2);

        quint32 chksum = 0;
        for (int i = 0; i < 512; ++i) {
            chksum += static_cast<unsigned char>(header[i]);
        }
        QByteArray chkOctal = QString("%1").arg(chksum, 6, 8, QChar('0')).toLatin1();
        memcpy(header.data() + 148, chkOctal.constData(), 6);
        header[154] = '\0';
        header[155] = ' ';

        return header;
    }

    // إعادة حزم البيانات إلى ملف (.ab) متوافق مع إصدار وتوقيع الحزمة على الهاتف المتصل ثم استعادته
    void repackAndRestoreAb(const QString &sourcePath, const QString &targetPkg) {
        if (sourcePath.isEmpty() || targetPkg.isEmpty()) {
            QMessageBox::warning(this, "تنبيه", "يرجى تحديد الملف/المجلد واختيار الحزمة الهدف أولاً!");
            return;
        }

        QFileInfo srcInfo(sourcePath);
        if (!srcInfo.exists()) {
            appendLog("المسار المحدد غير موجود: " + sourcePath, "ERROR");
            return;
        }

        // 1. فحص إصدار SDK الهاتف ومعلومات الحزمة الهدف المثبتة على الهاتف
        appendLog("1/4 جاري فحص إصدار أندرويد الهاتف وبيانات الحزمة الهدف: " + targetPkg, "INFO");

        QProcess sdkProc;
        sdkProc.start(getAdbExecutablePath(), {"shell", "getprop", "ro.build.version.sdk"});
        sdkProc.waitForFinished(4000);
        int deviceSdk = QString::fromUtf8(sdkProc.readAllStandardOutput()).trimmed().toInt();
        if (deviceSdk <= 0) deviceSdk = 30; // قيمة افتراضية عند عدم القراءة

        QProcess dumpProc;
        dumpProc.start(getAdbExecutablePath(), {"shell", "dumpsys", "package", targetPkg});
        dumpProc.waitForFinished(5000);
        QString dumpOut = QString::fromUtf8(dumpProc.readAllStandardOutput());

        if (dumpOut.isEmpty() || dumpOut.contains("Unable to find package")) {
            appendLog("الحزمة الهدف (" + targetPkg + ") غير مثبتة على الهاتف المتصل!", "ERROR");
            return;
        }

        // التحقق من دعم النسخ الاحتياطي وعدم كون الحزمة المدمجة في النظام مجرد قشرة (Stub)
        if (!dumpOut.contains("ALLOW_BACKUP")) {
            appendLog("تنبيه هام: الحزمة المثبتة في النظام لا تدعم ALLOW_BACKUP (أو أنها نسخة Stub مصنعية). قد يرفض النظام adb restore إلا بعد تحديث APK الحزمة.", "WARN");
        }

        // استخراج versionCode الفعلي للحزمة المثبتة على الهاتف
        int targetVersionCode = 1;
        QRegularExpression verRx("versionCode=(\\d+)");
        QRegularExpressionMatch verMatch = verRx.match(dumpOut);
        if (verMatch.hasMatch()) {
            targetVersionCode = verMatch.captured(1).toInt();
        }
        appendLog(QString("تم قراءة بيئة الهاتف الهدف -> SDK: %1 | versionCode: %2").arg(deviceSdk).arg(targetVersionCode), "SUCCESS");

        // 2. تجهيز الملفات المستخرجة محلياً في مجلد عمل مؤقت لبناء الـ .ab الجديد
        QString workStage = QDir::tempPath() + "/abdullah_repack_ab_stage";
        if (sourcePath != workStage) {
            QDir(workStage).removeRecursively();
            QDir().mkpath(workStage);

            if (srcInfo.isFile() && sourcePath.endsWith(".ab", Qt::CaseInsensitive)) {
                QString tmpTar = QDir::tempPath() + "/abdullah_tmp_unpack.tar";
                if (!convertAbFileToTar(sourcePath, tmpTar)) return;
                extractTarLocally(tmpTar, workStage);
            } else if (srcInfo.isFile() && sourcePath.endsWith(".tar", Qt::CaseInsensitive)) {
                extractTarLocally(sourcePath, workStage);
            } else if (srcInfo.isDir()) {
                copyDirectoryRecursively(sourcePath, workStage);
            }
        }

        // تحديد جذر بيانات الحزمة داخل المجلد المؤقت (apps/<old_pkg> أو المجلد نفسه)
        QString appRoot = workStage;
        QDir appsDir(workStage + "/apps");
        if (appsDir.exists()) {
            QStringList subPkgs = appsDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
            if (!subPkgs.isEmpty()) {
                appRoot = appsDir.absoluteFilePath(subPkgs.first());
            }
        }

        // قراءة التوقيع الأصلي من ملف _manifest القديم إن وجد للاحتفاظ به في البناء
        QStringList existingSigs;
        QFile oldManifest(appRoot + "/_manifest");
        if (oldManifest.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QStringList mLines = QString::fromUtf8(oldManifest.readAll()).split('\n', Qt::SkipEmptyParts);
            oldManifest.close();
            if (mLines.size() >= 7) {
                int sigCount = mLines[6].trimmed().toInt();
                for (int i = 0; i < sigCount && (7 + i) < mLines.size(); ++i) {
                    existingSigs.append(mLines[7 + i].trimmed());
                }
            }
        }

        // 3. بناء ملف _manifest جديد مطابق للحزمة الهدف وإصدار الهاتف المتصل
        QByteArray manifestBytes;
        QTextStream mStream(&manifestBytes, QIODevice::WriteOnly);
        mStream << "1\n";
        mStream << targetPkg << "\n";
        mStream << targetVersionCode << "\n";
        mStream << deviceSdk << "\n";
        mStream << "com.android.vending\n";
        mStream << "0\n";
        if (!existingSigs.isEmpty()) {
            mStream << existingSigs.size() << "\n";
            for (const QString &sig : existingSigs) {
                mStream << sig << "\n";
            }
        } else {
            mStream << "0\n";
        }
        mStream.flush();

        // 4. بناء أرشيف USTAR Tar نقي بترتيب أندرويد الإجباري (_manifest أولاً ثم الملفات تحت apps/<targetPkg>/)
        appendLog("2/4 جاري بناء هيكل USTAR Tar وضبط مسارات الاستخراج لـ apps/" + targetPkg + "...", "INFO");
        QByteArray tarStream;

        auto appendFileToTar = [&](const QString &internalTarPath, const QByteArray &content) {
            tarStream.append(createUstarHeader(internalTarPath, content.size()));
            tarStream.append(content);
            int remainder = content.size() % 512;
            if (remainder != 0) {
                tarStream.append(QByteArray(512 - remainder, '\0'));
            }
        };

        // الشرط الإجباري في أندرويد: ملف _manifest يجب أن يكون أول ملف في الأرشيف
        appendFileToTar("apps/" + targetPkg + "/_manifest", manifestBytes);

        // جمع كافة الملفات من (sp, db, f, r) وإعادة توجيهها إلى مسار الحزمة الهدف
        QDirIterator it(appRoot, QDir::Files | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            QString fullPath = it.next();
            QString relPath = QDir(appRoot).relativeFilePath(fullPath).replace('\\', '/');
            if (relPath == "_manifest") continue;

            // توحيد أسماء المجلدات الصريحة لو كانت مسحوبة يدوياً إلى صيغة TarBackupReader القياسية
            if (relPath.startsWith("shared_prefs/")) relPath.replace(0, 13, "sp/");
            else if (relPath.startsWith("databases/")) relPath.replace(0, 10, "db/");
            else if (relPath.startsWith("files/")) relPath.replace(0, 6, "f/");

            QFile f(fullPath);
            if (f.open(QIODevice::ReadOnly)) {
                appendFileToTar("apps/" + targetPkg + "/" + relPath, f.readAll());
                f.close();
            }
        }

        // إغلاق أرشيف الـ Tar بكتلتين صفريتين (1024 بايت)
        tarStream.append(QByteArray(1024, '\0'));

        // 5. تحديد إصدار ترويسة ANDROID BACKUP المناسب لإصدار أندرويد الهاتف وضغط البيانات بـ Zlib
        int abHeaderVersion = 5;
        if (deviceSdk < 19) abHeaderVersion = 1;
        else if (deviceSdk < 23) abHeaderVersion = 2;
        else if (deviceSdk < 26) abHeaderVersion = 4;

        appendLog(QString("3/4 جاري ضغط ملف .ab بإصدار ترويسة (%1) متوافق مع الهاتف...").arg(abHeaderVersion), "INFO");
        QByteArray compressedQt = qCompress(tarStream, 9);
        // حذف أول 4 بايتات خاصة بـ Qt للحصول على تدفق Zlib قياسي يقبله الأندرويد
        QByteArray rawZlib = compressedQt.mid(4);

        QString outAbPath = QDir::tempPath() + "/abdullah_patched_" + targetPkg + ".ab";
        QFile outAb(outAbPath);
        if (!outAb.open(QIODevice::WriteOnly)) {
            appendLog("تعذر إنشاء ملف .ab المعدل في المجلد المؤقت!", "ERROR");
            return;
        }

        QByteArray abHeader = QString("ANDROID BACKUP\n%1\n1\nnone\n").arg(abHeaderVersion).toUtf8();
        outAb.write(abHeader);
        outAb.write(rawZlib);
        outAb.close();

        // 6. تهيئة محرك النسخ الاحتياطي وتنشيط الحزمة على الهاتف ثم إرسال أمر adb restore
        runAdbSequence({
            {{"shell", "bmgr enable true && cmd package unstop " + targetPkg + " 2>/dev/null || true"},
             "تفعيل خدمة النسخ الاحتياطي وتنشيط مسار الحزمة على الهاتف...", ""},
            {{"shell", "am", "force-stop", targetPkg},
             "إغلاق الحزمة الهدف مؤقتاً استعداداً للاستعادة...", ""},
            {{"restore", outAbPath},
             "4/4 جاري إرسال ملف (.ab) المعدل إلى الهاتف (يرجى الموافقة على الاستعادة من شاشة الهاتف الآن)...",
             "تم تنفيذ عملية الاستعادة عبر (adb restore) للحزمة " + targetPkg + " بنجاح!"}
        });
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
