// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright © 2009-2018 ANSSI. All Rights Reserved.
/**
 * @file DecryptFrame.cpp
 * cryptclt red client decryption window implementation.
 * @author Olivier Levillain <clipos@ssi.gouv.fr>
 * @author Vincent Strubel <clipos@ssi.gouv.fr>
 *
 * Copyright (C) 2009 SGDN
 * Copyright (C) 2010-2011 ANSSI
 * @n
 * All rights reserved.
 */

#include "DecryptFrame.h"
#include "RedThreads.h"
#include "frmWait.h"
#include "common.h"
#include "FileField.h"
#include "err.h"

#include <QApplication>
#include <QCheckBox>
#include <QDir>
#include <QFileDialog>
#include <QFrame>
#include <QKeyEvent>
#include <QIcon>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QTabWidget>

// ajout de l'origine
#include "emblem-configuration.h"
#include "list.h"

// Constantes

static const char *socketPath = RED_SOCKET_PATH;
static const char *privateKeyExtension = ".acidpvr";

// Description de la fenetre
//--------------------------

DecryptFrame::DecryptFrame(int features, int eventType, 
                                  QDialog *parent, bool drawAll)
  : features(features), eventType(eventType), parent(parent)
{
  decryptionThread = NULL; 

  QVBoxLayout* lv = new QVBoxLayout(this);
  lv->setSpacing(space);

  // Emetteur
  lv->addWidget(decryptionKey = new FileField(this, 
              "<b>Clé privée de déchiffrement</b>", "Clé privée ACID",
              "Veuillez sélectionner une clé privée", privateKeyExtension,
              SINGLE_EXISTING_PRIVATE_KEY));
  decryptionKey->insertFile(loadDefaultPath(CRYPTCLT_PVR_PATH));
  decryptionKey->setToolTip("Utilisez '+' ou un glisser-déposer pour "
                    "sélectionner une clé privée de déchiffrement.");

  // Repertoire destination
  lv->addWidget (destinationPath = new FileField(this, 
                "<b>Répertoire de destination</b> (fichiers déchiffrés)", "",
                "Veuillez sélectionner un répertoire de destination", "",
                EXISTING_DIRECTORY));
  destinationPath->insertFile(loadDefaultPath(CRYPTCLT_OUTPUT_PATH));
  destinationPath->setToolTip("Utilisez '+' pour sélectionner un répertoire de "
                        "destination où écrire les fichiers déchiffrés.");


  QHBoxLayout* laySave = new QHBoxLayout();
  laySave->setSpacing(space);
  laySave->addSpacing(5);
  laySave->addWidget(btnSavePubKey = 
          new QCheckBox("Sauver la clé publique de l'émetteur", this));
  btnSavePubKey->setToolTip("Cochez cette option pour obtenir une copie de la "
                 "clé publique de l'émetteur dans le répertoire destination.");
  laySave->addStretch(2);
  lv->addLayout(laySave);
  lv->addSpacing(10);

  QHBoxLayout *lhb = new QHBoxLayout;
  lhb->addSpacing(10);

  // Niveau bas
  if ((features & CryptdCrypt) && drawAll) {
    QFrame *frame = new QFrame(this);
    frame->setFrameStyle(QFrame::Panel | QFrame::Raised);
    QVBoxLayout *lvv = new QVBoxLayout(frame);
    lvv->addWidget(
      new QLabel("<b><u>Déchiffrement depuis le niveau bas</u> :</b>", this));
    lvv->addWidget(
      new QLabel("Choisissez le fichier déchiffrer dans la liste ci-dessous.", 
                 this));
    lvv->addSpacing(5);

    lstIds = new QListWidget(this);
    lstIds->setToolTip("Sélectionnez une archive à déchiffrer.\n"
                       "L'archive doit avoir été importée dans\n"
                       "la diode depuis le niveau bas." );
    
    lvv->addSpacing(5);
    lvv->addWidget(lstIds);
    lvv->addSpacing(5);

    QHBoxLayout* lh_d = new QHBoxLayout();
    lh_d->setSpacing(space);

    btnRefresh = new QPushButton( QIcon::fromTheme("edit-redo"),
                "Rafraîchir", this);
    btnRefresh->setToolTip("Rafraîchir la liste des archives "
                                        "pouvant être déchiffrées.");
    lh_d->addStretch(2);
    lh_d->addWidget(btnRefresh);
    lh_d->addStretch(2);
    btnDecrypt = new QPushButton(
                QIcon(ICONPATH"/cryptclt-low.png"),
                "Déchiffrer depuis le niveau bas", this);
    btnDecrypt->setToolTip("Déchiffrer l'archive sélectionnée dans le\n"
                           "répertoire destination.");
    lh_d->addWidget(btnDecrypt);
    lh_d->addStretch(2);

    btnDelete = new QPushButton( QIcon::fromTheme("archive-remove"),
                "Supprimer", this);

    if (features & CryptdDelete) {
      btnDelete->setToolTip("Supprimer cette archive sans la déchiffrer.");
    } else {
      btnDelete->setEnabled(false); // feature not supported
      btnDelete->setToolTip("Fonctionalité non supportée par cette version "
                            "de la diode.");
    }

    lh_d->addWidget(btnDelete);
    lh_d->addStretch(2);

    lvv->addLayout(lh_d);

    lhb->addWidget(frame);
    if (features & CryptdEncrypt)
      lhb->addSpacing(20);

    connect(btnDecrypt, SIGNAL(clicked()), this, SLOT(decrypt()));
    connect(btnRefresh, SIGNAL(clicked()), this, SLOT(refreshListIds()));
    connect(btnDelete, SIGNAL(clicked()), this, SLOT(deleteArchive()));
  } else {
    lstIds = NULL;
    btnRefresh = NULL;
    btnDecrypt = NULL;
    btnDelete = NULL;
  }
  //
  // Niveau haut
  if ((features & CryptdEncrypt) && drawAll) {
    QFrame *frame = new QFrame(this);
    frame->setFrameStyle(QFrame::Panel | QFrame::Raised);
    QVBoxLayout *lvv = new QVBoxLayout(frame);
    lvv->addWidget(
      new QLabel("<b><u>Déchiffrement depuis le niveau haut</u> :</b>", this));
    lvv->addWidget(
      new QLabel("Choisissez directement un fichier à déchiffrer.", this));
    lvv->addSpacing(5);
    btnDecryptLocal = new QPushButton(
                QIcon(ICONPATH"/cryptclt-high.png"),
                "Déchiffrer depuis le niveau haut", this);
    btnDecryptLocal->setToolTip("Déchiffrer une archive disponible\n"
                                "directement au niveau haut.");
    lvv->addWidget(btnDecryptLocal);
    lvv->addSpacing(10);
    lvv->addStretch(5);

    lhb->addWidget(frame);

    connect(btnDecryptLocal, SIGNAL(clicked()), this, SLOT(decryptLocalAsk()));
  } else { 
    btnDecryptLocal = NULL;
  }

  lhb->addSpacing(10);
  lv->addLayout(lhb);

  lv->addStretch(4);

  refreshListIds();
  if (lstIds) {
    QListWidgetItem *first = lstIds->itemAt(0, 0);
    if (first)
      lstIds->setCurrentItem(first);
  }
}

DecryptFrame::~DecryptFrame () {
  if (decryptionThread) {
    decryptionThread->wait();
    delete (decryptionThread);
  }
}

void 
DecryptFrame::gotEvent(bool status) 
{
  if (lstIds)
    lstIds->clear();
  refreshListIds();

  done = true;
  ok = status;
}

void 
DecryptFrame::setPath(const QString &path) 
{
  QDir d(path);
  if (d.exists())
    destinationPath->insertFile(path); 
}

// Gestion des threads
//--------------------

void 
DecryptFrame::refreshListIds() 
{
  if (!lstIds)
    return;

  QIcon icon(ICONPATH"/acidcsa.png");
  refreshList(socketPath, *lstIds, icon);
}

void 
DecryptFrame::doDecrypt(const QString &id, char *content, uint32_t len)
{
  done = false;
  frmWait d("Opération en cours", 
            "Déchiffrement en cours, veuillez patienter...", &done);
  QStringList l;
  QString dest, str;
  cleartext_t *clr = cleartext_alloc();
  bool pubKey;
  uint32_t ret;
  ok = false;
  size_t size;

  if (!clr) {
    error("Plus assez de mémoire disponible.");
    goto err;
  }

  // Clé privée
  l = decryptionKey->returnFilenames();
  if (l.empty() || (l.first()).length() == 0) {
    error("Aucune clé privée spécifiée.");
    goto err;
  }
  clr->prv = privkey_alloc();
  if (!clr->prv) {
    error("Plus assez de mémoire disponible pour la clé privée.");
    goto err;
  }
  ret = cryptd_get_file(l.first().toUtf8().data(), 
                              &(clr->prv->data), &size);
  if (ret != CMD_OK) {
    error(QString("Erreur de lecture de la clé privée : ") + cmderr(ret));
    goto err;
  }
  if (size > UINT32_MAX) {
    error(QString("Taille de la clé privée invalide"));
    goto err;
  }
  clr->prv->len = (uint32_t) size;
  saveDefaultPath(l.first(), CRYPTCLT_PVR_PATH);

  clr->title = strdup(id.toUtf8().data());
  if (!clr->title) {
    error("Plus assez de mémoire disponible pour l'identifiant.");
    goto err;
  }
  clr->tlen = strlen(clr->title) + 1;

  // Nom de l'archive à exporter
  l = destinationPath->returnFilenames();
  if (l.empty()) {
    error("Aucun répertoire de destination n'a été indiqué.");
    goto err;
  }
  dest = l.first();

  parent->setEnabled(false);
  if (decryptionThread)
    delete (decryptionThread);
  pubKey = btnSavePubKey->isChecked();
  decryptionThread = new DecryptionThread(parent, clr, pubKey, 
                                          eventType, content, len);
  decryptionThread->start();

  d.exec();

  if (!ok)
    goto err;

  ret = writeFiles(clr, dest, str.toUtf8().data());
  if (ret != CMD_OK) {
    error(QString("La sauvegarde des fichiers déchiffrés n'a pas pu être "
                  "réalisée : ") + cmderr (ret));
    ok = false;
  }

  // Fall through

 err:
  parent->setEnabled(true);
  cleartext_free(clr);
}


void 
DecryptFrame::decrypt()
{
  QString str;

  // Id
  if (lstIds->currentItem())
    str = lstIds->currentItem()->text();
  if (str.isEmpty()) {
    error("Il faut spécifier un identifiant.");
    return;
  }
  doDecrypt(str);
}

void
DecryptFrame::decryptLocal(const QString &path)
{
  uint32_t ret;
  char *content;
  size_t size;
  uint32_t len;

  if (path.isEmpty())
      return;
  ret = cryptd_get_file(path.toUtf8().data(), &content, &size);
  if (ret != CMD_OK) {
    error(QString("Erreur de lecture de l'archive : ") + cmderr(ret));
    return;
  }
  if (size > UINT32_MAX) {
    error(QString("Taille de l'archive invalide"));
    return;
  }
  len = (uint32_t) size;
  QFileInfo info(path);
  saveDefaultPath(info.absolutePath(), CRYPTCLT_INPUT_PATH);

  doDecrypt(info.baseName(), content, len);
  free(content);
}

void
DecryptFrame::deleteArchive()
{ 
  QString str;

  // Id
  if (lstIds->currentItem())
    str = lstIds->currentItem()->text();
  if (str.isEmpty()) {
    error("Il faut spécifier un identifiant.");
    return;
  }

  int s = sock_connect(socketPath);
  if (s < 0) {
    error ("Erreur lors de la connexion au démon cryptd.");
    return;
  }

  char *name = str.toUtf8().data();
  uint32_t ret = cryptd_delete_ciphertext(s, name, strlen(name) + 1);
  if (ret != CMD_OK) {
    error(QString("Impossible de supprimer l'archive : ") + cmderr(ret));
    return;
  }

  info(QString("L'archive %1 a été supprimée").arg(str));
  refreshListIds();
}

void
DecryptFrame::decryptLocalAsk()
{
  QString path = QFileDialog::getOpenFileName(NULL, 
                        "Veuillez sélectionner l'archive à déchiffrer",
                        loadDefaultPath(CRYPTCLT_INPUT_PATH),
                        "Archives ACID (*.acidcsa)");
  if (path.isEmpty())
      return;
  return decryptLocal(path);
}

uint32_t 
DecryptFrame::writeFiles(cleartext_t *clr, const QString &dest, 
                  const char *name)
{
  uint32_t ret;
  int ow;
  QStringList l;
  QString newdest;

  // Ecriture de l'archive
  ret = cryptd_write_cleartext(dest.toUtf8().data(), clr, 0);
  if (ret != CMD_EXIST)
    goto out;

  ow = QMessageBox::question(0, "Ecraser les fichiers ?", 
        QString ("Certains des fichiers déchiffrés existent déjà dans %1,\n"
          "souhaitez-vous sauver les fichiers déchiffrés en écrasant les "
          "fichiers existants ?\nN.B. : répondre \"Annuler\" à cette question "
          "entraînera la perte des fichiers déchiffrés.").arg(dest), 
          "Oui", "Annuler", "Choisir un autre répertoire", 1, 1);
  if (!ow) {
    ret = cryptd_write_cleartext(dest.toUtf8().data(), clr, 1);
    goto out;
  }
  if (ow == 1)
    return CMD_EXIST;

  // Selection nouveau chemin
  destinationPath->askUser();
  l = destinationPath->returnFilenames();
  if (l.empty()) {
    error("Aucun répertoire destination n'a été spécifié.");
    return CMD_EMPTY;
  }
  newdest = l.first();
  return writeFiles(clr, newdest, name);
  
out:
  if (ret == CMD_OK) {
    info(QString("L'archive %1 a été correctement déchiffrée dans %2")
                    .arg(name).arg(dest));
    saveDefaultPath(dest, CRYPTCLT_OUTPUT_PATH);
  }

  // ajout de l'origine
  file_t* iter;
  list_for_each(iter, clr->files) {
    QString chemin_complet = dest + "/" + QString(iter->path);
    emblemConfiguration::setOrigin(chemin_complet.toUtf8().data(), emblemConfiguration::nom_niveau_haut);
  }

  return ret;
}

// vi:sw=2:ts=2:et:co=80:
