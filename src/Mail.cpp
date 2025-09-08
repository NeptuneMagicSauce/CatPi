#include "Mail.hpp"

#include <QApplication>
#include <QDate>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QList>
#include <QProcess>
#include <QString>
#include <QTimer>
#include <optional>

#include "Logs.hpp"
#include "System.hpp"

using std::optional;

struct MailImpl {
  Logs const& logs;

  MailImpl(Logs const& logs);
  void sendYesterday();
};

namespace {
  MailImpl* impl = nullptr;
}

Mail::Mail(Logs const& logs) {
  AssertSingleton();
  impl = new MailImpl{logs};
}

MailImpl::MailImpl(Logs const& logs) : logs(logs) {
  // send it on boot if needed
  sendYesterday();

  // then check regularly if it needs sending
  auto timer = new QTimer;
  timer->setSingleShot(false);
  timer->setInterval(5 * 60 * 1000);  // every 5 minutes
  QObject::connect(timer, &QTimer::timeout, [this] { sendYesterday(); });
  timer->start();
}

void MailImpl::sendYesterday() {
  auto yesterday = QDate::currentDate().addDays(-1);
  auto dataDirectory = logs.dataDirectory();

  // verify if it is not already sent
  auto fileOfLatestSend = QFile{dataDirectory + "/latest.email.txt"};
  if (fileOfLatestSend.exists()) {
    fileOfLatestSend.open(QIODeviceBase::ReadOnly);
    auto latestSend = QDate::fromJulianDay(QString{fileOfLatestSend.readAll()}.toInt());
    if (latestSend == yesterday) {
      // qDebug() << Q_FUNC_INFO << "already sent";
      return;
    }
    fileOfLatestSend.close();
  }
  fileOfLatestSend.open(QIODeviceBase::WriteOnly);
  fileOfLatestSend.write(QString::number(yesterday.toJulianDay()).toUtf8());

  // build list of recipients from csv file
  auto recipients = [] {
    QList<QString> ret;
    static auto pathOfRecipients = QString{};
    if (pathOfRecipients.isEmpty()) {
      auto pid = QString::number(QApplication::applicationPid());
      auto exe = QFile{"/proc/" + pid + "/exe"};
      auto realPath = exe.symLinkTarget();
      auto dirOfExe = QFileInfo{realPath}.absoluteDir();
      pathOfRecipients = dirOfExe.filePath("mail.recipients.csv");
    }
    auto fileOfRecipients = QFile{pathOfRecipients};
    if (fileOfRecipients.exists()) {
      fileOfRecipients.open(QIODeviceBase::ReadOnly);
      for (auto const& line : fileOfRecipients.readAll().split('\n')) {
        for (auto const& recipient : line.split(',')) {
          if (recipient.isEmpty()) {
            continue;
          }
          // qDebug() << recipient;
          ret << recipient;
        }
      }
    }
    return ret;
  }();

  // build logs content as base64 for in-line attaching
  auto logPath = logs.dateToFilePath(yesterday);
  auto logsContent = [&] -> optional<QString> {
    auto logFile = QFile{logPath};
    if (logFile.exists() == false) {
      return {};
    }
    logFile.open(QIODeviceBase::ReadOnly);
    auto log = logFile.readAll();
    return QString{log.toBase64()};
  }();

  // find hostname
  static auto hostName = QString{};
  if (hostName.isEmpty()) {
    auto hostNameFile = QFile{"/proc/sys/kernel/hostname"};
    hostNameFile.open(QIODeviceBase::ReadOnly);
    hostName = hostNameFile.readLine();
    hostName.remove("\n");
    // qDebug() << "HostName:" << hostName;
  }

  int recipientIndex = 0;
  for (const auto& recipient : recipients) {
    auto mailFile =
        QFile{dataDirectory + "/mail.recipient." + QString::number(recipientIndex) + ".txt"};
    auto mailFileName = mailFile.fileName();
    // qDebug() << "mailFile:" << mailFile.fileName();

    mailFile.open(QIODeviceBase::WriteOnly);

    // build message, RFC 5322 formatted
    auto content =
        QString{
            R"(From: "Cat Pi" <foobar@gmail.com>
To: <)"} +
        recipient + ">";
    content += R"(
Subject: Croquettes Report
MIME-Version: 1.0
Content-Type: multipart/mixed; boundary="BOUNDARYFOO"

--BOUNDARYFOO
Content-Type: text/plain; charset="UTF-8"
Content-Transfer-Encoding: 7bit

)";
    // body of the e-mail
    content += yesterday.toString("dddd, MMMM d");
    content += " [" + hostName + "]\n";

    if (logsContent.has_value()) {
      content += R"(
--BOUNDARYFOO
Content-Type: application/txt
Content-Disposition: attachment; filename=")";
      content += QFileInfo{logPath}.fileName();
      content += R"("
Content-Transfer-Encoding: base64

)";
      content += logsContent.value();
    } else {
      content += R"(
-- no content --
)";
    }
    content += R"(
--BOUNDARYFOO--
)";

    mailFile.write(content.toUtf8());
    mailFile.close();

    QTimer::singleShot(recipientIndex * 10 * 1000, [=] {
      // qDebug() << "Mail:" << recipient;
      auto p = QProcess{};
      p.setProgram("curl");
      p.setArguments({
          "--ssl-reqd",
          "--url",
          "smtps://smtp.gmail.com:465",
          "--netrc",
          "--mail-from",
          "foobar@gmail.com",
          "--mail-rcpt",
          recipient,
          "--upload-file",
          mailFileName,
      });
      // #warning DEBUG
      // qDebug() << p.program() + " " + p.arguments().join(" ");
      p.startDetached();
    });

    ++recipientIndex;
  }
}
