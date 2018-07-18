// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright © 2009-2018 ANSSI. All Rights Reserved.
/**
 * @file frmClientDecrypt.cpp
 * cryptclt decrypt client main window implementation.
 * @author Vincent Strubel <clipos@ssi.gouv.fr>
 *
 * Copyright (C) 2011 ANSSI
 * @n
 * All rights reserved.
 */

#include "frmClientDecrypt.h"
#include "DecryptFrame.h"
#include "frmWait.h"
#include "common.h"
#include "FileField.h"
#include "err.h"

#include <QApplication>
#include <QFileDialog>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QFrame>
#include <QLayout>
#include <QTreeWidget>
#include <QKeyEvent>
#include <QMessageBox>
#include <QIcon>
#include <QProcess>

// Constantes

static const char *socketPath = RED_SOCKET_PATH;
static const char *archiveExtension = ".acidcsa";
static const QString redIcon(PREFIX"/share/icons/cryptclt-red.png");

// Description de la fenetre
//--------------------------

frmClientDecrypt::frmClientDecrypt(int argc, char **argv)
{
  if (!getServerInfo(socketPath, &features) || !(features & CryptdEncrypt)) {
    error("Le serveur cryptd ne supporte pas le\n"
          "déchiffrement depuis le niveau haut.");
    exit(1);
  }
  setWindowTitle("Déchiffrement ACID");
  setWindowIcon(QIcon(redIcon));
  setMinimumWidth(600);
  eventType = QEvent::registerEventType();

  btnDecrypt = new QPushButton(QIcon::fromTheme("document-decrypt"), 
                                                   "Déchiffrer", this);
  btnInfo = new QPushButton(QIcon::fromTheme("help-about"),
                                               "Propriétés", this);
  btnInfo->setToolTip("Afficher les propriétés de l'archive");
  btnQuit = new QPushButton(QIcon::fromTheme("dialog-close"), "Quitter", this);

  archive = new FileField(this, 
              "<b>Archive à déchiffrer</b>", "Archive à déchiffrer",
              "Veuillez sélectionner l'archive à déchiffrer", 
              archiveExtension, SINGLE_EXISTING_FILE);
  archive->setToolTip("Utilisez '+' ou un glisser-déposer pour sélectionner "
                    "une archive à déchiffrer.");
  
  // N.B. : we don't want any other feature here (i.e. not CryptdCrypt)
  frmDecrypt = new DecryptFrame(CryptdEncrypt, eventType, this);

  QVBoxLayout* lv = new QVBoxLayout(this);
  lv->setSpacing(space);
  lv->addWidget(archive);
  lv->addSpacing(10);
  lv->addWidget(frmDecrypt);
  lv->addSpacing(10);
  
  QHBoxLayout* layBottom = new QHBoxLayout();
  layBottom->setSpacing(space);
  layBottom->addStretch(2);
  layBottom->addWidget(btnDecrypt);
  layBottom->addStretch(2);
  layBottom->addWidget(btnInfo); 
  layBottom->addStretch(2);
  layBottom->addWidget(btnQuit); 
  layBottom->addStretch(2);

  lv->addSpacing(10);
  lv->addLayout(layBottom);
  lv->addStretch(4);

  if (argc >= 2) {
    archive->insertFile(QString::fromUtf8(argv[1]));
  }

  // Connexions
  //-----------
  connect(btnDecrypt, SIGNAL(clicked()), this, SLOT(decrypt()));
  connect(btnInfo, SIGNAL(clicked()), this, SLOT(info()));
  connect(btnQuit, SIGNAL(clicked()), this, SLOT(quit()));
}

void 
frmClientDecrypt::keyPressEvent(QKeyEvent *e) 
{
  if (e->key() == Qt::Key_Escape) {
    e->accept();
    quit();
  }
  if (e->key() == Qt::Key_Return) {
    e->accept();
    decrypt();
  }
}

void frmClientDecrypt::customEvent(QEvent *event) {
  if (event->type() != eventType) {
    qDebug("received unknown event %d (!= %d)", event->type(), eventType);
    return;
  }
  EncryptionEvent *e = (EncryptionEvent *)event;
  if (!e->status())
    error(e->message());

  frmDecrypt->gotEvent(e->status());
}

void 
frmClientDecrypt::decrypt()
{
  QStringList l = archive->returnFilenames();
  if (l.empty() || (l.first()).length() == 0) {
    error("Aucune archive spécifiée.");
    return;
  }

  frmDecrypt->decryptLocal(l.first());

  if (frmDecrypt->isOk())
    QApplication::exit(0);
}

void 
frmClientDecrypt::info()
{
  QProcess csainfo;
  QStringList l = archive->returnFilenames();
  if (l.empty() || (l.first()).length() == 0) {
    error("Aucune archive spécifiée.");
    return;
  }
  
  csainfo.startDetached("csainfo4", l);
}

void 
frmClientDecrypt::quit()
{
    QApplication::exit(0);
}

// vi:sw=2:ts=2:et:co=80:
