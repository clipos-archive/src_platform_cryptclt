// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright © 2009-2018 ANSSI. All Rights Reserved.
/**
 * @file frmClientDown.cpp
 * cryptclt diode down client main window implementation.
 * @author Olivier Levillain <clipos@ssi.gouv.fr>
 * @author Vincent Strubel <clipos@ssi.gouv.fr>
 *
 * Copyright (C) 2009 SGDN
 * Copyright (C) 2010-2013 ANSSI
 * @n
 * All rights reserved.
 */

#include "frmClientDown.h"
#include "common.h"
#include "FileField.h"
#include "ClamCheck.h"
#include "err.h"

#include <QApplication>
#include <QFile>
#include <QFileInfo>
#include <QFrame>
#include <QKeyEvent>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QProcess>
#include <QPushButton>
#include <QStringList>

#include <cryptd_black.h>

static const char *socketPath = BLACK_SOCKET_PATH;
static const QString downIcon(PREFIX"/share/icons/cryptclt-down.png");

frmClientDown::frmClientDown(int argc, char** argv)
{
  setWindowTitle("Client diode bas");
  setWindowIcon(QIcon(downIcon));

  clamCheck = QFile::exists(PREFIX"/bin/clamscan");

  QVBoxLayout* lv = new QVBoxLayout(this);
  lv->setSpacing(space);

  // Archive source
  lv->addWidget(toExport = new FileField(this, 
                      "Fichier à transférer", "", 
                      "Veuillez sélectionner un fichier à transférer", 
                      "", MULTIPLE_EXISTING_FILES));
  toExport->setDefaultPath(loadDefaultPath(CRYPTCLT_INPUT_PATH));
  toExport->setToolTip("Utilisez '+' ou un glisser-déposer pour "
              "ajouter\ndes fichiers à transférer dans la diode.");
  toExport->setIcon(QIcon::fromTheme("document-open"));

  // Bouton Exporter
  QHBoxLayout* layBottom = new QHBoxLayout(NULL);
  layBottom->setSpacing(space);
  // Bouton Quitter
  layBottom->addWidget(btnExport = 
            new QPushButton(QIcon::fromTheme("arrow-up"), "Transférer", this));
  btnExport->setToolTip("Transférer les fichiers sélectionnés, ou tous les\n"
                        "fichiers si aucun n'a été sélectionné.");
  layBottom->addStretch(2);
  layBottom->addWidget(btnExportAndQuit = 
            new QPushButton(QIcon::fromTheme("arrow-up-double"),
                                    "Transférer et quitter", this));
  btnExportAndQuit->setToolTip("Transférer les fichiers puis "
                               "quitter l'application.");
  layBottom->addStretch(2);
  layBottom->addWidget(btnQuit = 
            new QPushButton(QIcon::fromTheme("dialog-close"), "Quitter", this));
  btnQuit->setToolTip("Quitter l'application.");
  lv->addLayout(layBottom);

  // Connexions
  //-----------

  connect(btnExport, SIGNAL(clicked()), this, SLOT(doExport()));
  connect(btnExportAndQuit, SIGNAL(clicked()), this, SLOT(exportAndQuit()));
  connect(btnQuit, SIGNAL(clicked()), this, SLOT(quit()));

  if (argc >= 3 && !strcmp(argv[1], "-i")) {
    for (int i = 2; i < argc; i++)
      toExport->insertFile(QString::fromUtf8(argv[i]));
  }

  if (!checkServerInfo(socketPath, CryptdDiode))
    exit(1);
}


void 
frmClientDown::keyPressEvent(QKeyEvent* e) 
{
  if (e->key() == Qt::Key_Escape) {
    e->accept();
    quit();
  }
}


void 
frmClientDown::quit() 
{
  QStringList l = toExport->returnFilenames();
  if (!l.empty() && !yesOrNo("Confirmation",
         "Certains fichiers n'ont pas été transférés dans la diode.\n"
         "Voulez-vous vraiment quitter l'application ?")) 
    return;
  QApplication::exit(0);
}

// Export
//-------

void 
frmClientDown::exportAndQuit()
{
  doExport();
  if (ok) 
    quit();
}


void 
frmClientDown::doExport()
{
  QStringList l;
  ok = true;
  bool onlySelected = true;

  if (!checkServerInfo(socketPath, CryptdDiode))
    exit(1);

    // Id
  l = toExport->returnSelectedFilenames();
  if (l.empty()) {
	  l = toExport->returnFilenames();
    onlySelected = false;
  }
		
  if (l.empty()) {
    error("Aucun fichier à exporter n'a été spécifié.");
    return;
  }
  QString transfered;
  for (QStringList::ConstIterator i=l.constBegin(); i != l.constEnd(); i++) {
    if (exportOneFile(*i)) {
      QFileInfo tmp(*i);
      transfered += "\n " + tmp.fileName();
    } else
      ok = false;
  }
      
  if (!transfered.isEmpty()) 
    info(QString("Les fichiers suivants ont été correctement transférés "
                  "dans la diode: %1").arg(transfered));

  saveDefaultPath(QFileInfo(l.first()).absolutePath(), CRYPTCLT_INPUT_PATH);
  if (onlySelected)
    toExport->deleteSelectedFiles();
  else
    toExport->clean();
}

bool
frmClientDown::exportOneFile(const QString &fname)
{
  char *name = NULL, *bname;
  int s = -1;
  uint32_t ret, memsz, fsz;
  bool retval = false;

  if (clamCheck) {
    if (!ClamCheck(fname))
      return false;
  }

  file_t *file = file_alloc();
  if (!file) {
    error ("Plus assez de mémoire disponible.");
    goto err;
  }

  name = strdup(fname.toUtf8().data());
  if (!name) {
    error ("Plus assez de mémoire disponible.");
    goto err;
  }
  
  memsz = get_memsize();
  fsz = get_filesize(name);
  if (fsz > memsz / 4) {
    error(QString ("Le fichier %1 occuperait dans la diode plus d'un "
                    "quart de la mémoire du poste.\nIl est préférable de le "
                    "découper en plusieurs fichiers plus petits avant de le "
                    "transférer.\nLa taille maximale des fichiers "
                    "transférable par la diode sur ce poste est de %2 Mo.")
                    .arg(name).arg(memsz / (4 * 1024)));
    goto err;
  }
  ret = cryptd_get_cleartext_file(name, 0, &file);
  if (ret != CMD_OK) {
    error(QString("Impossible de lire le fichier ") + name 
                                                + " : " + cmderr(ret));
    goto err;
  }
  bname = basename(name);
  // Ouverture socket
  s = sock_connect(socketPath);
  if (s < 0) {
    error("Erreur lors de la connexion au démon cryptd.");
    goto err;
  }

  // Sélection de l'archive
  ret = cryptd_send_diode(s, file);
  if (ret != CMD_OK) {
    error(QString("Impossible d'exporter le fichier ") + bname 
                                                     + " : " + cmderr(ret));
    goto err;
  }

  retval = true;
  /* Fall through */
 err:
  if (s>=0)
    close_socket(s);
  if (name)
    free(name);
  
  file_free(file);
  return retval;
}

// vi:sw=2:ts=2:et:co=80:
