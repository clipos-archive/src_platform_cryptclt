// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright © 2009-2018 ANSSI. All Rights Reserved.
/**
 * @file frmClientUp.cpp
 * cryptclt diode up client main window implementation.
 * @author Olivier Levillain <clipos@ssi.gouv.fr>
 * @author Vincent Strubel <clipos@ssi.gouv.fr>
 *
 * Copyright (C) 2009 SGDN
 * Copyright (C) 2010-2011 ANSSI
 * @n
 * All rights reserved.
 */

#include "frmClientUp.h"
#include "FileField.h"
#include "err.h"

#include <QApplication>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QTabWidget>
#include <QFrame>
#include <QLayout>
#include <QListWidget>
#include <QDir>
#include <QMessageBox>
#include <QKeyEvent>
#include <QFileInfo>

#include <unistd.h>

#include "emblem-configuration.h"

// Constantes

static const char *socketPath = RED_SOCKET_PATH;
static const QString upIcon(PREFIX"/share/icons/cryptclt-up.png");

frmClientUp::frmClientUp(int argc, char** argv)
{
  setWindowTitle("Client diode haut");
  setWindowIcon(QIcon(upIcon));

  QVBoxLayout* lv = new QVBoxLayout(this);
  lv->setSpacing(space);

  // Liste des identifiants
  lv->addWidget(lblListeIds = new QLabel("Liste des fichiers pouvant "
                                        "être importés", this));
  lv->addSpacing(5);
  lv->addWidget(lstIds = new QListWidget(this));
  lstIds->setSelectionMode(QAbstractItemView::ExtendedSelection);
  lstIds->setToolTip("Sélectionnez un fichier à importer depuis la diode.");

  // Archive destination
  lv->addWidget(destinationPath = new FileField(this, 
        "Répertoire de destination", "",
        "Veuillez sélectionner un répertoire de destination", 
        "", EXISTING_DIRECTORY));
  destinationPath->insertFile(loadDefaultPath(CRYPTCLT_OUTPUT_PATH));
  destinationPath->setToolTip("Utilisez '+' pour sélectionner un répertoire"
              "\ndans lequel importer le fichier.");

  // Bouton Importer
  QHBoxLayout* layBottom = new QHBoxLayout(NULL);
  layBottom->setSpacing(space);
  // Bouton Quitter
  layBottom->addWidget(btnRefresh = 
          new QPushButton(QIcon::fromTheme("edit-redo"), "Rafraîchir", this));
  btnRefresh->setToolTip("Rafraîchir la liste des fichiers "
                                        "pouvant être importés.");
  layBottom->addStretch(2);
  layBottom->addWidget(btnImport = 
          new QPushButton(QIcon::fromTheme("arrow-up"), "Importer", this));
  btnImport->setToolTip("Importer dans le répertoire destination les "
                        "fichiers sélectionnés,\nou tous les fichiers "
                        "disponibles si aucun n'a été sélectionné");
  layBottom->addStretch(2);
  layBottom->addWidget(btnImportAndQuit = 
          new QPushButton(QIcon::fromTheme("arrow-up-double"), 
                                    "Importer et quitter", this));
  btnImportAndQuit->setToolTip("Importer les fichiers puis "
                                "quitter l'application");
  layBottom->addStretch(2);
  layBottom->addWidget(btnQuit = 
          new QPushButton(QIcon::fromTheme("dialog-close"), "Quitter", this));
  lv->addLayout(layBottom);

  // Connexions
  //-----------

  connect(btnImport, SIGNAL(clicked()), this, SLOT(import()));
  connect(btnImportAndQuit, SIGNAL(clicked()), this, SLOT(importAndQuit()));
  connect(btnRefresh, SIGNAL(clicked()), this, SLOT(refresh()));
  connect(btnQuit, SIGNAL(clicked()), this, SLOT(quit()));

  if (argc == 3 && !strcmp(argv[1], "-d")) {
    QDir d(QString::fromUtf8(argv[2]));
    if (d.exists())
      destinationPath->insertFile(QString::fromUtf8(argv[2]));
  }

  refresh();
}


void 
frmClientUp::keyPressEvent(QKeyEvent* e) 
{
  if (e->key() == Qt::Key_Escape) {
    e->accept();
    quit();
  }
}


void 
frmClientUp::quit() 
{
  QApplication::exit(0);
}

// Export
//-------

/* Copy needed, as we use cryptd_get_list_diode, which is
 * only in libcryptdcr -> can't be put in common.cpp
 */
static void 
refreshListDiode(const char* const socketName, QListWidget& l) 
{
  int s = -1; 
  uint32_t len, ret;
  QString selection, str;
  char *buf = NULL, *ptr, *base;
  QIcon icon = QIcon::fromTheme("document-open");

  s = sock_connect(socketName);
  if (s < 0) {
    error ("Erreur lors de la connexion au démon cryptd.");
    goto err;
  }

  ret = cryptd_get_diode_list(s, &buf, &len);
  if (ret != CMD_OK) {
    error(QString("Impossible de récupérer la liste des "
                    "fichiers importables : ") + cmderr(ret));
    goto err;
  }

  if (buf == NULL) {
    l.clear();
    return;
  }

  if (buf[len - 1] != '\0') {
    error ("Liste de fichiers mal formée.");
    goto err;
  }

  base = buf;
  l.setSelectionMode(QAbstractItemView::NoSelection);
  l.clearSelection();
  l.clear();
  l.setSelectionMode(QAbstractItemView::ExtendedSelection);

  while ((ptr = strchr(base, '\n'))) {
    *ptr = 0;
    str = QString::fromUtf8(base);
    l.addItem(new QListWidgetItem(icon, str, 0));
    base = ptr + 1;
  }

  close(s);
  return;

 err:
  if (s >= 0)
    close(s);
  if (buf)
    free(buf);
}

void 
frmClientUp::refresh() 
{
  if (!checkServerInfo(socketPath, CryptdDiode))
    exit(1);
  refreshListDiode(socketPath, *lstIds);
}


void 
frmClientUp::importAndQuit()
{
  import();
  if (ok) 
    QApplication::exit(0);
}

bool
frmClientUp::importOneFile(const QString &id, const QString &path)
{
  QStringList l;
  QString dest;
  char *name = NULL;
  int s = -1;
  uint32_t ret;
  file_t *file = NULL;
  bool retval = false;

  file = file_alloc();
  if (!file) {
    error("Plus assez de mémoire disponible.");
    goto err;
  }

  name = strdup(id.toUtf8().data());
  if (!name) {
    error("Plus assez de mémoire pour l'identifiant");
    goto err;
  }

  dest = path + QDir::separator() + id;

  // Ouverture socket
  s = sock_connect(socketPath);
  if (s < 0) {
    error ("Erreur lors de la connexion au démon cryptd.");
    goto err;
  }

  // Sélection de l'archive
  ret = cryptd_recv_diode(s, name, strlen(name) + 1, &file);
  if (ret != CMD_OK) {
    if (ret == CMD_CANCEL)
      error(QString("Le transfert du fichier ") + name 
                                + "a été annulé dans le socle.");
    else
      error(QString("Impossible de récupérer le fichier ") 
                                + name + " : " + cmderr(ret));
    goto err;
  }

  ret = writeFile(file, path, name);
  if (ret != CMD_OK) {
    error(QString("Erreur lors de l'écriture du fichier ") 
                                + dest + " : " + cmderr(ret));
    goto err;
  }

 emblemConfiguration::setOrigin((path+"/"+name).toUtf8().data(), emblemConfiguration::nom_niveau_bas);

  retval = true;
  /* Fall through */
 err:
  if (s >= 0)
    close_socket(s);
  if (name)
    free(name);
  if (file)
    file_free(file);
  return retval;
}

void 
frmClientUp::import()
{
  QStringList l;
  QString path;

  ok = true;
  
  // Id
  l = destinationPath->returnFilenames();
  if (l.empty()) {
    error("Aucun répertoire destination n'a été spécifié.");
    ok = false;
    return;
  }
  path = l.first();
  l.clear();

  // Id
  QList<QListWidgetItem *> selected = lstIds->selectedItems();
  QList<QListWidgetItem *>::const_iterator iter;

  for (iter = selected.constBegin(); iter != selected.constEnd(); iter++) {
    l.append((*iter)->text());
  }

  if (l.empty()) {
    for (int i = 0; i < lstIds->count(); i++) {
      l.append(lstIds->item(i)->text());
    }
  }

  if (l.empty()) {
    error("Aucun fichier à importer");
    ok = false;
    return;
  }
  QString transfered;
  for (QStringList::ConstIterator i=l.constBegin(); i != l.constEnd(); i++) {
    if (importOneFile(*i, path))
      transfered += "\n " + *i;
    else
      ok = false;
  }
      
  if (!transfered.isEmpty()) 
    info(QString("Les fichiers suivants suivantes ont été correctement exportées "
                  "depuis la diode dans %1 : %2").arg(path).arg(transfered));

  saveDefaultPath(QFileInfo(path).absoluteFilePath(), CRYPTCLT_OUTPUT_PATH);
  refresh();
}

uint32_t 
frmClientUp::writeFile(file_t *file, const QString &dest, const char *name)
{
  uint32_t ret;
  int ow;
  QStringList l;
  QString newdest;

  // Ecriture de l'archive
  ret = cryptd_write_cleartext_file(dest.toUtf8().data(), file, 0);
  if (ret != CMD_EXIST)
    goto out;

  ow = QMessageBox::question(0, "Ecraser le fichier ?", 
          QString ("Le fichier %1/%2 existe déjà, souhaitez-vous l'écraser ?\n"
          "N.B. : répondre \"Annuler\" à cette question entraînera la perte du "
          "fichier importé.")
          .arg(dest).arg(QString::fromUtf8(name)), "Oui", "Annuler", 
          "Choisir un autre répertoire", 1, 1);
  if (!ow) {
    ret = cryptd_write_cleartext_file(dest.toUtf8().data(), file, 1);
    goto out;
  }
  if (ow == 1)
    return CMD_OK;

  // Selection nouveau chemin
  destinationPath->askUser();
  l = destinationPath->returnFilenames();
  if (l.empty()) {
    error ("Aucun répertoire destination n'a été spécifié.");
    return CMD_EMPTY;
  }
  newdest = l.first();
  return writeFile(file, newdest, name);
  
out:
  return ret;
}
// vi:sw=2:ts=2:et:co=80:
