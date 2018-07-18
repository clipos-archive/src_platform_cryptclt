// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright © 2009-2018 ANSSI. All Rights Reserved.
/**
 * @file frmClientRed.cpp
 * cryptclt red client main window implementation.
 * @author Olivier Levillain <clipos@ssi.gouv.fr>
 * @author Vincent Strubel <clipos@ssi.gouv.fr>
 *
 * Copyright (C) 2009 SGDN
 * Copyright (C) 2010-2012 ANSSI
 * @n
 * All rights reserved.
 */

#include "frmClientRed.h"
#include "DecryptFrame.h"
#include "frmWait.h"
#include "common.h"
#include "FileField.h"
#include "err.h"

#include <QApplication>
#include <QFileDialog>
#include <QFileInfo>
#include <QLabel>
#include <QDir>
#include <QPushButton>
#include <QLineEdit>
#include <QTabWidget>
#include <QFrame>
#include <QLayout>
#include <QTreeWidget>
#include <QKeyEvent>
#include <QMessageBox>
#include <QIcon>

// Constantes

static const char *socketPath = RED_SOCKET_PATH;
static const char *publicKeyExtension = ".acidppr";
static const char *privateKeyExtension = ".acidpvr";
static const QString redIcon(ICONPATH"/cryptclt-red.png");

// Description de la fenetre
//--------------------------

frmClientRed::frmClientRed(int argc, char **argv)
{
  if (!getServerInfo(socketPath, &features))
    exit(1);
  setWindowTitle("Client ACID rouge");
  setWindowIcon(QIcon(redIcon));
  eventType = QEvent::registerEventType();

  encryptionThread = NULL;
  passwordThread = NULL;

  // Onglet chiffrement
  //-------------------
  tabEncrypt = new QFrame();
  QVBoxLayout* lv_c = new QVBoxLayout(tabEncrypt);
  lv_c->setSpacing(space);

  // Fichiers / Destinataires
  QHBoxLayout* lh_c = new QHBoxLayout(NULL);
  lh_c->setSpacing(space);
  files = new FileField(tabEncrypt, "<b>Fichiers à chiffrer</b>", 
        "Nom du fichier",
        "Veuillez sélectionner le ou les fichier(s) à ajouter l'archive", 
        "", MULTIPLE_EXISTING_FILES);
  files->setDefaultPath(loadDefaultPath(CRYPTCLT_INPUT_PATH));
  files->setToolTip("Utilisez '+' ou un glisser-déposer pour ajouter "
                    "des fichiers à chiffrer.");
  files->setIcon(QIcon::fromTheme("document-open"));


  recipients = new FileField(tabEncrypt, "<b>Destinataires</b>", 
          "Clé publique ACID", "Veuillez sélectionner le ou les "
          "destinataire(s) à ajouter à l'archive",
          publicKeyExtension, MULTIPLE_EXISTING_PUBLIC_KEYS);
  recipients->setDefaultPath(loadDefaultPath(CRYPTCLT_PPR_PATH));
  recipients->setIcon(QIcon(ICONPATH"/acidppr.png"));
  recipients->setVirtualIcon(QIcon(ICONPATH"/acidpvr.png"));

  lh_c->addWidget(files);
  lh_c->addWidget(recipients);
  lv_c->addLayout(lh_c);

  // Emetteur
  lv_c->addWidget(sender = new FileField(tabEncrypt, 
              "<b>Clé privée de signature</b> (émetteur)", "Clé privée ACID",
              "Veuillez sélectionner une clé privée ACID", 
              privateKeyExtension, SINGLE_EXISTING_PRIVATE_KEY));

  sender->setToolTip("Utilisez '+' ou un glisser-déposer pour sélectionner "
                    "une clé privée de chiffrement.");

  // Identifiant
  QHBoxLayout *lh_tmp;
  lh_tmp = new QHBoxLayout;
  lh_tmp->addSpacing(space);
  lh_tmp->addWidget(lblId = 
                new QLabel("<b>Identifiant de l'archive</b>", tabEncrypt));
  lh_tmp->addStretch(2);
  lv_c->addLayout(lh_tmp);
  lh_tmp = new QHBoxLayout;
  lh_tmp->addSpacing(space);
  lh_tmp->addWidget(txtId = new QLineEdit(tabEncrypt));
  txtId->setMinimumWidth(300);
  lh_tmp->addStretch(2);
  lv_c->addLayout(lh_tmp);
  txtId->setToolTip("Saisissez le nom de l'archive chiffrée (forme libre).");

  // Bouton Encrypt
  QHBoxLayout* layEncrypt = new QHBoxLayout(NULL);
  layEncrypt->setSpacing(space);
  layEncrypt->addStretch(2);
  if (features & CryptdCrypt) {
    btnEncrypt = new QPushButton(
              QIcon(ICONPATH"/cryptclt-low.png"),
              "Chiffrer pour le niveau bas", tabEncrypt);
    btnEncrypt->setToolTip("Chiffrer les fichiers pour les destinataires,\n"
                           "avec l'identifiant spécifié et placer l'archive\n"
                           "en attente de récupération au niveau bas.");
    layEncrypt->addWidget(btnEncrypt);
    layEncrypt->addStretch(2);
  } else {
    btnEncrypt = NULL;
  }
  if (features & CryptdEncrypt) {
    btnEncryptLocal = new QPushButton(
              QIcon(ICONPATH"/cryptclt-high.png"),
              "Chiffrer pour le niveau haut", tabEncrypt);
    btnEncryptLocal->setToolTip("Chiffrer les fichiers et récupérer directement\n"
                                "l'archive chiffrée au niveau haut.");
    layEncrypt->addWidget(btnEncryptLocal); 
    layEncrypt->addStretch(2);
  } else { 
    btnEncryptLocal = NULL;
  }
  lv_c->addSpacing(10);
  lv_c->addLayout(layEncrypt);
  lv_c->addStretch(4);



  // Onglet déchiffrement
  //---------------------

  frmDecrypt = new DecryptFrame(features, eventType, this, true);

  // Onglet changement de mot de passe
  //----------------------------------
  if (features & CryptdChPw) {
    tabPassword = new QFrame();
    QVBoxLayout* lv_p = new QVBoxLayout(tabPassword);
    lv_p->setSpacing(space);

    lv_p->addWidget(inputKey = new FileField(tabPassword, 
                "<b>Clé privée d'origine</b>", "Clé privée ACID",
                "Veuillez sélectionner une clé privée ACID", 
                privateKeyExtension, SINGLE_EXISTING_PRIVATE_KEY));
    inputKey->insertFile(loadDefaultPath(CRYPTCLT_PVR_PATH));
    inputKey->setToolTip("Utilisez '+' ou un glisser-déposer pour sélectionner "
                "une clé privée\ndont vous souhaitez changer le mot de passe.");

    lv_p->addSpacing(space);
    lv_p->addWidget(outputKey = new FileField(tabPassword, 
                "<b>Nom de la nouvelle clé</b>", "Clé privée ACID",
                "Veuillez sélectionner une clé privée ACID destination", 
                privateKeyExtension, SINGLE_DESTINATION_FILE));
    outputKey->insertFile(loadDefaultPath(CRYPTCLT_PVR_PATH));
    outputKey->setToolTip("Utilisez '+' ou un glisser-déposer pour "
                        "sélectionner le fichier dans lequel sauvegarder\n"
                        "la clé privée protégée par le nouveau mot de passe.");


    QHBoxLayout* layPassword = new QHBoxLayout(NULL);
    layPassword->setSpacing(space);
    layPassword->addStretch(2);
    btnChPw = new QPushButton(
                QIcon::fromTheme("preferences-desktop-user-password"),
                "Changer le mot de passe", tabPassword);
    btnChPw->setToolTip("Modifier le mot de passe de la clé origine,\n"
                        "et enregistrer la clé résultante.");
    layPassword->addWidget(btnChPw);
    layPassword->addStretch(2);
    lv_p->addSpacing(10);
    lv_p->addLayout(layPassword);
    lv_p->addStretch(3);
  } else {
    tabPassword = NULL;
  }

  // Gestion du layout de la fenetre
  //--------------------------------

  // Création de l'objet tabs
  tabs = new QTabWidget(this);
  tabs->addTab(tabEncrypt, 
    QIcon::fromTheme("document-encrypt"), "Chiffrement");
  tabs->addTab(frmDecrypt, 
    QIcon::fromTheme("document-decrypt"), "Déchiffrement");
  if (tabPassword)
    tabs->addTab(tabPassword, 
      QIcon::fromTheme("preferences-desktop-user-password"), 
      "Changement de mot de passe");

  // Bouton Quitter
  btnQuit = new QPushButton(QIcon::fromTheme("dialog-close"), 
                                  "Quitter", this);
  QHBoxLayout* layQuit = new QHBoxLayout(NULL);
  layQuit->setSpacing(space);
  layQuit->addStretch(2);
  layQuit->addWidget(btnQuit);
  layQuit->addStretch(2);

  // Ajout final
  QVBoxLayout* formLayout = new QVBoxLayout (this);
  formLayout->setSpacing(space);
  formLayout->addWidget(tabs);
  formLayout->addLayout(layQuit);


  // Si on est appelé en chiffrement
  // Prise en compte des arguments de la ligne de commande (chiffrement)
  //--------------------------------------------------------------------
  
  tabs->setCurrentIndex(1);
  frmDecrypt->refreshListIds();

  if (argc >= 2 && !strcmp(argv[1], "-c")) {
    tabs->setCurrentIndex(0);
    if (argc >= 3) {
      QFileInfo info(QString::fromUtf8(argv[2]));
      txtId->setText(info.baseName());
    }
    for (int i = 2; i<argc; i++)
      files->insertFile(QString::fromUtf8(argv[i]));
  }

  if (argc >= 2 && !strcmp(argv[1], "-p")) {
    if (tabPassword) {
      tabs->setCurrentIndex(2);
    } else {
      error("Le démon cryptd ne supporte pas le changement de mot de passe");
      exit(1);
    }
  }

  if (argc >= 3 && !strcmp(argv[1], "-d")) {
    frmDecrypt->setPath(QString::fromUtf8(argv[2]));
  }

  // Connexions
  //-----------

  if (btnEncrypt)
    connect(btnEncrypt, SIGNAL(clicked()), this, SLOT(encrypt()));
  if (btnEncryptLocal)
    connect(btnEncryptLocal, SIGNAL(clicked()), this, SLOT(encryptLocal()));

  connect(tabs, SIGNAL(currentChanged(QWidget*)), 
                                        frmDecrypt, SLOT(refreshListIds()));

  connect(btnQuit, SIGNAL(clicked()), this, SLOT(quit()));
  connect(sender, SIGNAL(idChanged(const QString&, const QString&)),
        recipients, SLOT(updateVirtualId(const QString&, const QString&)));
  			
  if (tabPassword) {
    connect(btnChPw, SIGNAL(clicked()), this, SLOT(chpw()));
    connect(inputKey, SIGNAL(valueChanged(const QString&)), 
                                  this, SLOT(updateOutputKey()));
  }

  /* Only do it here, after we've connected idChanged() */
  sender->insertFile(loadDefaultPath(CRYPTCLT_PVR_PATH));
}

frmClientRed::~frmClientRed () {
  if (encryptionThread) {
    encryptionThread->wait();
    delete (encryptionThread);
  }
  if (passwordThread) {
    passwordThread->wait();
    delete (passwordThread);
  }
}

void 
frmClientRed::clean(direction_t dir) 
{
  switch (dir) {
    case DECRYPTION:
      break;
    case ENCRYPTION:
      files->clean();
      recipients->clean();
      txtId->setText("");
      break;
    case CHPASSWD:
      inputKey->clean();
      outputKey->clean();
      break;
  }
}

void 
frmClientRed::keyPressEvent(QKeyEvent *e) 
{
  if (e->key() == Qt::Key_Escape) {
    e->accept();
    quit();
  }
}

void 
frmClientRed::quit()
{
  if (yesOrNo("Confirmation",
         "Voulez-vous vraiment quitter l'application ?"))    
    QApplication::exit(0);
}

// Gestion des threads
//--------------------

void
frmClientRed::freeze() 
{
  tabs->setEnabled(false);
  btnQuit->setEnabled(false);
}

void 
frmClientRed::unfreeze()
{
  tabs->setEnabled (true);
  btnQuit->setEnabled (true);
  frmDecrypt->refreshListIds();
}

void frmClientRed::customEvent(QEvent *event) {
  if (event->type() != eventType) {
    qDebug("received unknown event %d (!= %d)", event->type(), eventType);
    return;
  }
  EncryptionEvent *e = (EncryptionEvent *)event;
  if (e->status()) {
    if (e->direction() == ENCRYPTION) {
      if (!e->message().isEmpty())
        info(e->message());
    }
    clean(e->direction());
    ok = true;
  } else 
    error(e->message());

  if (e->direction() == DECRYPTION) 
    frmDecrypt->gotEvent(e->status());

  done = true;
}

// Encrypt
//---------

void 
frmClientRed::doEncrypt(bool local, char **content, uint32_t *len)
{
  done = false;
  frmWait d("Opération en cours", 
          "Chiffrement en cours, veuillez patienter...", &done);
  QStringList l;
  cleartext_t *clr = cleartext_alloc();
  file_t *f = NULL;
  uint32_t ret, memsz, fsz;
  ok = false;
  size_t size;

  if (!clr) {
    error("Plus assez de mémoire disponible.");
    goto err;
  }

  // Fichiers
  l = files->returnFilenames();
  if (l.empty()) {
    error("Aucun fichier n'a été fourni.");
    goto err;
  }
  memsz = get_memsize();
  fsz = 0;
  for (QStringList::ConstIterator i=l.constBegin(); i != l.constEnd(); i++) {
    fsz += get_filesize((*i).toUtf8().data());
  }
  if (fsz > memsz / 4) {
    error(QString("Les fichiers à chiffrer occuperaient dans la diode plus "
      "d'un quart de la mémoire du poste.\nIl est préférable de les découper "
      "en plusieurs fichiers plus petits avant de les transférer.\n"
      "La taille maximale des fichiers qui peuvent être chiffrés par la "
      "diode sur ce poste est de %1 Mo.").arg(memsz / (8 * 1024)));
    goto err;
  }
  for (QStringList::ConstIterator i=l.constBegin(); i != l.constEnd(); i++) {
    if (cryptd_get_cleartext_file_encode((*i).toUtf8().data(), 0, &f)) {
      error(QString ("Le fichier ") + (*i).toUtf8().data() + 
                " n'a pu être ajouté à l'archive.");
      goto err;
    }
    list_add(f, clr->files);
  }
  saveDefaultPath(files->defaultPath(), CRYPTCLT_INPUT_PATH);

  // Clés publiques
  l = recipients->returnFilenames();
  if (l.empty()) {
    int cancel = QMessageBox::question(0, "Chiffrer sans destinataire ?", 
        "Vous n'avez pas spécifié de destinataire - l'archive ne sera\n"
        "déchiffrable que par vous même. Confirmez-vous le chiffrement ?",
        "Oui", "Non");
    if (cancel == 1) 
      goto err;
  }
  for (QStringList::ConstIterator i=l.constBegin(); i != l.constEnd(); i++) {
    pubkey_t *pp = pubkey_alloc();
    if (!pp) {
      error("Plus assez de mémoire disponible pour une clé publique.");
      goto err;
    }
    ret = cryptd_get_file((*i).toUtf8().data(), &(pp->data), &size);
    if (ret != CMD_OK) {
      error(QString ("Erreur lors de l'ajout de la clé publique") 
                                            + (*i) + " : " + cmderr(ret));
      goto err;
    }
    if (size > UINT32_MAX) {
      error(QString ("Taille de la clé publique invalide"));
      goto err;
    }
    pp->len = (uint32_t) size;
    size = 0;
    list_add(pp, clr->pubs);
  }
  saveDefaultPath(recipients->defaultPath(), CRYPTCLT_PPR_PATH);

  // Clé privée
  l = sender->returnFilenames();
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
    error(QString ("Erreur de lecture de la clé privée : ") + cmderr(ret));
    goto err;
  }
  if (size > UINT32_MAX) {
    error(QString ("Taille de la clé privée invalide"));
    goto err;
  }
  clr->prv->len = (uint32_t) size;
  saveDefaultPath(l.first(), CRYPTCLT_PVR_PATH);


  // Id
  if (txtId->text().length() <= 0) {
    error("Il faut spécifier un identifiant.");
    goto err;
  }
  clr->title = strdup(txtId->text().toUtf8().data());
  if (!clr->title) {
    error("Plus assez de mémoire disponible pour l'identifiant.");
    goto err;
  }
  clr->tlen = strlen(clr->title) + 1;

  freeze();
  if (encryptionThread)
    delete(encryptionThread);
  encryptionThread = new EncryptionThread(this, clr, eventType, local);
  if (local)
    encryptionThread->setOutput(content, len);
  encryptionThread->start();

  d.exec();
  unfreeze();
  return;

 err:
  cleartext_free(clr);
}

void 
frmClientRed::encrypt() {
  doEncrypt(false, NULL, NULL);
}

void
frmClientRed::encryptLocal() {
  char *content = NULL;
  uint32_t len = 0;
  QString path;
  uint32_t ret;
  QString title = txtId->text(); // will be cleared after encryption

  doEncrypt(true, &content, &len);
  if (!content)
    return;

askUser:
  path =  QFileDialog::getExistingDirectory(NULL, 
                      "Veuillez sélectionner le répertoire destination",
                      loadDefaultPath(CRYPTCLT_OUTPUT_PATH));
  if (path.isEmpty())
    goto out;
  saveDefaultPath(path, CRYPTCLT_OUTPUT_PATH);

  path = path + "/" + title + ".acidcsa";
  ret = cryptd_write_file(path.toUtf8().data(), content, len, 0);
  if (ret == CMD_EXIST) {
    int ow = QMessageBox::question(0, "Ecraser la clé ?", 
        QString ("L'archive %1 existe déjà, souhaitez-vous l'écraser ?")
          .arg(path), "Oui", "Annuler", "Choisir un autre chemin");
    if (ow == 1) 
      goto out;
    if (ow == 2)
      goto askUser;
    ret = cryptd_write_file(path.toUtf8().data(), content, len, 1);
  }

  if (ret != CMD_OK) {
    error(QString("L'écriture de l'archive %1 a échoué.").arg(path));
    goto out;
  }

  info(QString("L'archive a été correctement chiffrée et "
                "enregistrée sous %1.").arg(path));
  // Fall through
out:
  if (content)
    free(content);
}

// Changement de mot de passe
//---------------------------

void 
frmClientRed::chpw()
{
  done = false;
  frmWait d("Opération en cours", 
    "Modification du mot de passe en cours, veuillez patienter...", &done);
  privkey_t *prv = privkey_alloc();
  uint32_t ret;
  QStringList l;
  QString dest;
  ok = false;
  size_t size;

  if (!prv) {
    error("Plus assez de mémoire disponible.");
    goto err;
  }

  // Clé privée
  l = inputKey->returnFilenames();
  if (l.empty() || (l.first()).length() == 0) {
    error("Aucune clé privée spécifiée.");
    goto err;
  }
  ret = cryptd_get_file(l.first().toUtf8().data(), 
                              &(prv->data), &size);
  if (ret != CMD_OK) {
    error(QString("Erreur de lecture de la clé privée : ") + cmderr(ret));
    goto err;
  }
  if (size > UINT32_MAX) {
    error(QString ("Taille de la clé privÃe invalide"));
    goto err;
  }
  prv->len = (uint32_t) size;
  saveDefaultPath(l.first(), CRYPTCLT_PVR_PATH);

  // Destination
  l = outputKey->returnFilenames();
  if (l.empty()) {
    error("Aucun nom n'a été indiqué pour la nouvelle clé.");
    goto err;
  }
  dest = l.first();

  freeze();
  if (passwordThread)
    delete (passwordThread);
  passwordThread = new PasswordThread(this, prv, eventType);
  passwordThread->start();

  d.exec();

  if (!ok) {
    unfreeze();
    goto err;
  }

  ret = writeKey(prv, dest.toUtf8().data());
  if (ret != CMD_OK)
    error(QString("La sauvegarde de la clé n'a pas pu être réalisée : ") 
                                                        + cmderr (ret));
  unfreeze();

  clean(CHPASSWD);
  privkey_free(prv);
  return;

 err:
  privkey_free(prv);
}

uint32_t 
frmClientRed::writeKey(privkey_t *prv, const QString &dest)
{
  uint32_t ret;
  int ow;
  QStringList l;
  QString newdest;

  // Ecriture de l'archive
  ret = cryptd_write_file(dest.toUtf8().data(), prv->data, prv->len, 0);
  if (ret != CMD_EXIST)
    goto out;

  ow = QMessageBox::question(0, "Ecraser la clé ?", 
        QString ("La clé destination existe déjà dans %1,\n"
          "souhaitez-vous l'écraser ?\n"
          "N.B. : répondre \"Annuler\" à cette question "
          "entraînera la perte de la nouvelle clé.").arg(dest), 
          "Oui", "Annuler", "Choisir une autre destination", 1, 1);
  if (!ow) {
    ret = cryptd_write_file(dest.toUtf8().data(), prv->data, prv->len, 1);
    goto out;
  }
  if (ow == 1)
    return CMD_OK;

  // Selection nouveau chemin
  outputKey->askUser();
  l = outputKey->returnFilenames();
  if (l.empty()) {
    error("Aucune clé destination n'a été spécifiée.");
    return CMD_EMPTY;
  }
  newdest = l.first();
  return writeKey(prv, newdest);
  
out:
  if (ret == CMD_OK)
    info(QString("La nouvelle clé a été sauvegardée sous %1").arg(dest));
  return ret;
}

void
frmClientRed::updateOutputKey()
{
  QStringList l = inputKey->returnFilenames();
  if (l.empty() || (l.first()).length() == 0) {
    return;
  }

  outputKey->insertFile(l.first());
}

// vi:sw=2:ts=2:et:co=80:
