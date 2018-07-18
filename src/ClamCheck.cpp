// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright © 2009-2018 ANSSI. All Rights Reserved.
/**
 * @file ClamCheck.cpp
 * cryptclt clamAV check functions.
 * @author Vincent Strubel <clipos@ssi.gouv.fr>
 *
 * Copyright (C) 2010-2012 ANSSI
 * @n
 * All rights reserved.
 */

#include "ClamCheck.h"
#include "frmWait.h"
#include "common.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>
#include <QProcess>


bool
ClamCheck(const QString &path)
{
  QProcess checker;
  QStringList args;
  QString sigPath = QString("%1/.clamav/daily.cld").arg(QDir::homePath());
  bool haveSigFile = true;
  QMessageBox::StandardButton answer;

  if (!QFile::exists(path)) {
    QMessageBox::warning(0, "Fichier manquant",
          "Le fichier " + path + " n'existe pas.\n"
          "Echec de l'analyse antivirale.",
          QMessageBox::Ok, QMessageBox::Ok);
    return false;
  }

  QString dateString;
  if (!QFile::exists(sigPath)) {
    sigPath = QString("%1/.clamav/main.cld").arg(QDir::homePath());
    if (!QFile::exists(sigPath)) {
      dateString = "Aucune information disponible sur la mise à jour des "
                    "signatures antivirales.";
      haveSigFile = false;
    }
  }
  if (haveSigFile) {
    QFileInfo sigInfo(sigPath);
    QDateTime date = sigInfo.lastModified();

    dateString = QString("Dernière mise à jour des signatures antivirales : "
          "le %1 à %2.").arg(date.date().toString("dd.MM.yyyy"))
          .arg(date.time().toString("hh:mm"));
  }

  QFileInfo info(path);
  QString cmd("clamscan");

  args << "--infected" << "--stdout" << "--no-summary";
  if (QFile::exists(QString("%1/.clamav/clamd.sock").arg(QDir::homePath()))) {
    cmd = "clamdscan";
    args << "--fdpass";
  } else {
    args << "--detect-broken";
  }
  args << info.absoluteFilePath();

  checker.setReadChannel(QProcess::StandardOutput);
  checker.start(cmd, args);
  if (!checker.waitForStarted()) {
    answer = QMessageBox::question(0, "Analyse impossible",
          "L'utilitaire clamscan n'a pas pu être exécuté.\n"
          "Echec de l'analyse antivirale de " + info.fileName() + ".",
          QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    return (answer == QMessageBox::Yes);
  }
  frmWait waiter("Opération en cours", 
          QString("Analyse antivirale de %1 en cours.\n"
          "Veuillez patienter...\n%2").arg(info.fileName()).arg(dateString), 
          checker);
  waiter.exec();
  checker.waitForFinished(); // should not be needed, just in case

  int code = checker.exitCode();
  if (!code)
    return true;

  QByteArray output = checker.readAll();

  if (code == 1) {
    QMessageBox::warning(0, "Virus détecté",
          "L'analyse antivirale a détécté un virus dans le fichier\n" 
          + info.fileName() + ".\n"
          "Résultats de l'analyse :\n"
          + output,
          QMessageBox::Ok, QMessageBox::Ok);
    return false;
  }
  answer = QMessageBox::question(0, "Erreur d'analyse",
        "L'analyse antivirale du fichier " + info.fileName() + 
        " a rencontré une erreur.\n"
        "Résultats de l'analyse :\n"
        + output + "\nSouhaitez-vous transférer le "
        "fichier sans analyse antivirale ?",
        QMessageBox::Yes| QMessageBox::No, QMessageBox::No);
  return (answer == QMessageBox::Yes);
}
    

// vi:sw=2:ts=2:et:co=80:
