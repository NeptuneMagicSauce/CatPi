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
#include <cstdlib>
#include <optional>

#include "System.hpp"

using std::optional;

struct MailImpl {
  QTimer timer;
  QString senderAddress;

  MailImpl();
  void sendYesterday(QString const& logPath, QString const& ascii, QString const& dataDirectory);
};

namespace {
  MailImpl* impl = nullptr;
}

Mail::Mail() {
  AssertSingleton();
  impl = new MailImpl;
}

void Mail::sendYesterday(QString const& logPath, QString const& ascii,
                         QString const& dataDirectory) {
  impl->sendYesterday(logPath, ascii, dataDirectory);
}

MailImpl::MailImpl() {
  // find sender address
  senderAddress = [] -> QString {
    auto homeDir = getenv("HOME");
    if (homeDir == nullptr) {
      return "";
    }
    auto file = QFile{QString{homeDir} + "/.netrc"};
    if (file.exists() == false) {
      return "";
    }
    file.open(QIODeviceBase::ReadOnly);
    for (const auto& line : file.readAll().split('\n')) {
      static auto const pattern = QString{"login "};
      auto l = QString{line};
      if (l.startsWith(pattern) && l.endsWith("@gmail.com")) {
        auto ret = l;
        ret.remove(pattern);
        return ret;
      }
    }
    return "";
  }();
  if (senderAddress.isEmpty()) {
    return;
  }
  qDebug() << "SenderEmail:" << senderAddress;
}

void MailImpl::sendYesterday(QString const& logPath, QString const& ascii,
                             QString const& dataDirectory) {
  auto yesterday = QDate::currentDate().addDays(-1);

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
    auto content = QString{R"(From: "Cat Pi" <)"};
    content += senderAddress;
    content += R"(>
To: <)";
    content += recipient + ">";
    content += R"(
Subject: Croquettes Report
MIME-Version: 1.0
Content-Type: multipart/mixed; boundary="BOUNDARYFOO"

--BOUNDARYFOO
Content-Type: text/html; charset="UTF-8"
Content-Transfer-Encoding: quoted-printable

)";
    // body of the e-mail

    content += R"(
<html>
  <body>
    <pre style="font-family: 'Courier New', Courier, monospace;">
)";
    content += yesterday.toString("dddd, MMMM d");
    if (hostName != "catpi") {
      content += " [" + hostName + "]";
    }
    content += "\n";

    content += ascii + "\n";
    content += R"(
    </pre>
  </body>
</html>
)";

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

    QTimer::singleShot(recipientIndex * 10 * 1000, [=, this] {
      // qDebug() << "Mail:" << recipient;
      auto p = QProcess{};
      p.setProgram("curl");
      p.setArguments({
          "--ssl-reqd",
          "--url",
          "smtps://smtp.gmail.com:465",
          "--netrc",
          "--mail-from",
          senderAddress,
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
