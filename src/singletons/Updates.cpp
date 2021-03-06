#include "Updates.hpp"

#include "common/NetworkRequest.hpp"
#include "common/Version.hpp"
#include "util/CombinePath.hpp"
#include "util/PostToThread.hpp"

#include <QMessageBox>
#include <QProcess>

namespace chatterino {

Updates::Updates()
    : currentVersion_(CHATTERINO_VERSION)
{
    qDebug() << "init UpdateManager";
}

Updates &Updates::getInstance()
{
    // fourtf: don't add this class to the application class
    static Updates instance;

    return instance;
}

const QString &Updates::getCurrentVersion() const
{
    return currentVersion_;
}

const QString &Updates::getOnlineVersion() const
{
    return onlineVersion_;
}

void Updates::installUpdates()
{
    if (this->status_ != UpdateAvailable) {
        assert(false);
        return;
    }

#ifdef Q_OS_WIN
    QMessageBox *box = new QMessageBox(QMessageBox::Information, "Chatterino Update",
                                       "Chatterino is downloading the update "
                                       "in the background and will run the "
                                       "updater once it is finished.");
    box->setAttribute(Qt::WA_DeleteOnClose);
    box->show();

    NetworkRequest req(this->updateUrl_);
    req.setTimeout(600000);
    req.onError([this](int) -> bool {
        this->setStatus_(DownloadFailed);

        postToThread([] {
            QMessageBox *box = new QMessageBox(QMessageBox::Information, "Chatterino Update",
                                               "Failed while trying to download the update.");
            box->setAttribute(Qt::WA_DeleteOnClose);
            box->show();
            box->raise();
        });

        return true;
    });
    req.get([this](QByteArray &object) {
        auto filename = combinePath(getApp()->paths->miscDirectory, "update.zip");

        QFile file(filename);
        file.open(QIODevice::Truncate | QIODevice::WriteOnly);

        if (file.write(object) == -1) {
            this->setStatus_(WriteFileFailed);
            return false;
        }

        QProcess::startDetached(
            combinePath(QCoreApplication::applicationDirPath(), "updater.1/ChatterinoUpdater.exe"),
            {filename, "restart"});

        QApplication::exit(0);
        return false;
    });
    this->setStatus_(Downloading);
    req.execute();
#endif
}

void Updates::checkForUpdates()
{
#ifdef Q_OS_WIN
    QString url = "https://notitia.chatterino.com/version/chatterino/" CHATTERINO_OS "/stable";

    NetworkRequest req(url);
    req.setTimeout(30000);
    req.getJSON([this](QJsonObject &object) {
        QJsonValue version_val = object.value("version");
        QJsonValue update_val = object.value("update");

        if (!version_val.isString() || !update_val.isString()) {
            this->setStatus_(SearchFailed);
            qDebug() << "error updating";

            postToThread([] {
                QMessageBox *box = new QMessageBox(
                    QMessageBox::Information, "Chatterino Update",
                    "Error while searching for updates.\n\nEither the service is down "
                    "temporarily or everything is broken.");
                box->setAttribute(Qt::WA_DeleteOnClose);
                box->show();
                box->raise();
            });
            return;
        }

        this->onlineVersion_ = version_val.toString();
        this->updateUrl_ = update_val.toString();

        if (this->currentVersion_ != this->onlineVersion_) {
            this->setStatus_(UpdateAvailable);
            postToThread([this] {
                QMessageBox *box =
                    new QMessageBox(QMessageBox::Information, "Chatterino Update",
                                    "An update for chatterino is available.\n\nDo you "
                                    "want to download and install it?",
                                    QMessageBox::Yes | QMessageBox::No);
                box->setAttribute(Qt::WA_DeleteOnClose);
                box->show();
                box->raise();
                if (box->exec() == QMessageBox::Yes) {
                    this->installUpdates();
                }
            });
        } else {
            this->setStatus_(NoUpdateAvailable);
        }
    });
    this->setStatus_(Searching);
    req.execute();
#endif
}

Updates::UpdateStatus Updates::getStatus() const
{
    return this->status_;
}

void Updates::setStatus_(UpdateStatus status)
{
    if (this->status_ != status) {
        this->status_ = status;
        postToThread([this, status] { this->statusUpdated.invoke(status); });
    }
}

}  // namespace chatterino
