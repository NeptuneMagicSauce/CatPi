#include "Mail.hpp"

#include <QApplication>
#include <QDate>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QList>
#include <QProcess>
#include <QString>

#include "Logs.hpp"
#include "System.hpp"

struct MailImpl {
  Logs const& logs;
  QList<QString> recipients;

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
  auto pid = QString::number(QApplication::applicationPid());
  auto exe = QFile{"/proc/" + pid + "/exe"};
  auto realPath = exe.symLinkTarget();
  auto dirOfExe = QFileInfo{realPath}.absoluteDir();
  auto pathOfRecipients = dirOfExe.filePath("mail.recipients.csv");
  auto fileOfRecipients = QFile{pathOfRecipients};
  if (fileOfRecipients.exists()) {
    fileOfRecipients.open(QIODeviceBase::ReadOnly);
    for (auto const& recipient : fileOfRecipients.readAll().split(',')) {
      // qDebug() << recipient;
      recipients << recipient;
    }
  }
}

void Mail::sendYesterday() {
  try {
    impl->sendYesterday();
  } catch (const std::exception& e) {
    qDebug() << e.what();
  }
}

void MailImpl::sendYesterday() {
  auto logPath = logs.dateToFilePath(QDate::currentDate().addDays(-1));
  auto logFile = QFile{logPath};
  if (logFile.exists() == false) {
    logPath = "/dev/null";
  }
  logFile.open(QIODeviceBase::ReadOnly);

  static auto hostName = QString{};
  if (hostName.isEmpty()) {
    auto hostNameFile = QFile{"/proc/sys/kernel/hostname"};
    hostNameFile.open(QIODeviceBase::ReadOnly);
    hostName = hostNameFile.readLine();
    hostName.remove("\n");
    // qDebug() << "HostName:" << hostName;
  }

  auto writeLine = [](auto& file, const auto& line) { file.write((line + "\n").toUtf8()); };

  int recipientIndex = 0;
  for (const auto& recipient : recipients) {
    auto mailFile = QFile{logs.dataDirectory() + "/mail.recipient." +
                          QString::number(recipientIndex++) + ".txt"};
    // qDebug() << "mailFile:" << mailFile.fileName();
    mailFile.open(QIODeviceBase::WriteOnly);

    writeLine(mailFile, QString{"From: \"Cat Pi\" <foobar@gmail.com>"});
    writeLine(mailFile, QString{"To: \"\" <" + recipient + ">"});
    writeLine(mailFile, QString{"Subject: Cat Pi Daily"});
    writeLine(mailFile, QString{"<samp>"});
    writeLine(mailFile, QString{"HostName: "} + hostName);
    writeLine(mailFile, QString{logFile.readAll()});
    writeLine(mailFile, QString{"</samp>"});
    mailFile.close();

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
        mailFile.fileName(),
    });
    qDebug() << p.program() + " " + p.arguments().join(" ");
    // p.startDetached();
  }
}
