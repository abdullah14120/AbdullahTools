#include <QApplication>
#include <QWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTextEdit>
#include <QMessageBox>
#include <QSettings>
#include <QDateTime>
#include <QProcess>
#include <QTabWidget>

// === 1. نافذة تسجيل الدخول والتجربة المجانية ===
class LoginWindow : public QWidget {
    Q_OBJECT
public:
    LoginWindow(QWidget *parent = nullptr) : QWidget(parent) {
        setWindowTitle("تسجيل الدخول - أداة إدارة الأندرويد");
        resize(400, 300);
        
        // تصميم الشكل (Dark Theme أنيق)
        setStyleSheet("background-color: #1e1e1e; color: #ffffff; font-family: Arial; font-size: 14px;");

        QVBoxLayout *layout = new QVBoxLayout(this);

        QLabel *titleLabel = new QLabel("تسجيل الدخول للأداة", this);
        titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #00adb5;");
        titleLabel->setAlignment(Qt::AlignCenter);
        layout->addWidget(titleLabel);

        codeLineEdit = new QLineEdit(this);
        codeLineEdit->setPlaceholderText("أدخل كود الاشتراك هنا...");
        codeLineEdit->setStyleSheet("padding: 8px; background-color: #2d2d2d; border: 1px solid #444; border-radius: 4px; color: white;");
        layout->addWidget(codeLineEdit);

        QPushButton *loginBtn = new QPushButton("دخول بالكود", this);
        loginBtn->setStyleSheet("background-color: #00adb5; color: white; padding: 10px; border-radius: 4px; font-weight: bold;");
        layout->addWidget(loginBtn);

        QPushButton *trialBtn = new QPushButton("تجربة مجانية (ليومين)", this);
        trialBtn->setStyleSheet("background-color: #393e46; color: white; padding: 10px; border-radius: 4px;");
        layout->addWidget(trialBtn);

        connect(loginBtn, &QPushButton::clicked, this, &LoginWindow::handleLogin);
        connect(trialBtn, &QPushButton::clicked, this, &LoginWindow::handleTrial);
    }

signals:
    void loginSuccess();

private slots:
    void handleLogin() {
        QString code = codeLineEdit->text().trimmed();
        // كود تجريبي افتراضي للتفعيل (يمكن ربطه بسيرفر لاحقاً)
        if (code == "PRO-2026-VIP") {
            QMessageBox::information(this, "نجاح", "تم تفعيل الاشتراك بنجاح!");
            emit loginSuccess();
            close();
        } else {
            QMessageBox::warning(this, "خطأ", "كود الاشتراك غير صحيح!");
        }
    }

    void handleTrial() {
        QSettings settings("MyAndroidTool", "LicenseManager");
        if (settings.contains("TrialExpiry")) {
            QDateTime expiry = settings.value("TrialExpiry").toDateTime();
            if (QDateTime::currentDateTime() < expiry) {
                QMessageBox::information(this, "تجربة", "أنت تستخدم النسخة التجريبية المتبقي منها وقت.");
                emit loginSuccess();
                close();
                return;
            } else {
                QMessageBox::warning(this, "انتهت المدة", "عذراً، انتهت فترة التجربة المجانية (يومان).");
                return;
            }
        }

        // تفعيل تجربة يومين من الآن
        QDateTime expiryTime = QDateTime::currentDateTime().addDays(2);
        settings.setValue("TrialExpiry", expiryTime);
        QMessageBox::information(this, "تجربة مجانية", "تم تفعيل الفترة التجريبية بنجاح لمدة يومين!");
        emit loginSuccess();
        close();
    }

private:
    QLineEdit *codeLineEdit;
};


// === 2. الواجهة الرئيسية (مهام الأندرويد) ===
class MainWindow : public QWidget {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr) : QWidget(parent) {
        setWindowTitle("أداة التحكم بالهاتف (الإصدار الأول)");
        resize(700, 500);
        setStyleSheet("background-color: #121212; color: #e0e0e0; font-size: 13px;");

        QVBoxLayout *mainLayout = new QVBoxLayout(this);

        // شريط الأزرار العلوية للمهام
        QHBoxLayout *btnLayout = new QHBoxLayout();
        QPushButton *btnDevices = new QPushButton("فحص الأجهزة المتصلة", this);
        QPushButton *btnInstall = new QPushButton("تثبيت تطبيق (APK)", this);
        QPushButton *btnShell = new QPushButton("تنفيذ أمر Shell حُر", this);
        
        QString btnStyle = "background-color: #2196F3; color: white; padding: 8px; border-radius: 4px; font-weight: bold;";
        btnDevices->setStyleSheet(btnStyle);
        btnInstall->setStyleSheet(btnStyle);
        btnShell->setStyleSheet(btnStyle);

        btnLayout->addWidget(btnDevices);
        btnLayout->addWidget(btnInstall);
        btnLayout->addWidget(btnShell);
        mainLayout->addLayout(btnLayout);

        // شاشة عرض النتائج السوداء
        outputBox = new QTextEdit(this);
        outputBox->setReadOnly(true);
        outputBox->setStyleSheet("background-color: #000000; color: #00FF00; font-family: Consolas; font-size: 13px; border: 1px solid #333;");
        mainLayout->addWidget(outputBox);

        // ربط الأحداث بالأزرار
        connect(btnDevices, &QPushButton::clicked, [=]() { runAdb({"devices"}); });
        connect(btnInstall, &QPushButton::clicked, [=]() { runAdb({"install", "app.apk"}); });
        connect(btnShell, &QPushButton::clicked, [=]() { runAdb({"shell", "getprop", "ro.product.model"}); });
    }

private:
    QTextEdit *outputBox;

    void runAdb(const QStringList &args) {
        QProcess *process = new QProcess(this);
        
        connect(process, &QProcess::readyReadStandardOutput, [=]() {
            outputBox->append(QString::fromUtf8(process->readAllStandardOutput()));
        });
        
        connect(process, &QProcess::readyReadStandardError, [=]() {
            outputBox->append("<font color='red'>" + QString::fromUtf8(process->readAllStandardError()) + "</font>");
        });

        // تشغيل أمر adb المدمج بجانب الأداة
        process->start("platform-tools/adb.exe", args);
    }
};


// === 3. نقطة التشغيل الرئيسية ===
int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    LoginWindow loginWin;
    MainWindow mainWin;

    // عند نجاح تسجيل الدخول، اغلق نافذة الدخول وافتح الواجهة الرئيسية
    QObject::connect(&loginWin, &LoginWindow::loginSuccess, [&]() {
        mainWin.show();
    });

    loginWin.show();
    return app.exec();
}

#include "main.moc"
