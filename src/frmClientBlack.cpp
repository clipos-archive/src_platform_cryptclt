// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright © 2009-2018 ANSSI. All Rights Reserved.
/**
 * @file frmClientBlack.cpp
 * cryptclt black client main window implementation.
 * @author Olivier Levillain <clipos@ssi.gouv.fr>
 * @author Vincent Strubel <clipos@ssi.gouv.fr>
 * @author Alain Ozann <clipos@ssi.gouv.fr>
 *
 * Copyright (C) 2009 SGDN
 * Copyright (C) 2010-2013 ANSSI
 * @n
 * All rights reserved.
 */

#include "frmClientBlack.h"
#include "common.h"
#include "FileField.h"
#include "err.h"

#include <QApplication>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QTabWidget>
#include <QFrame>
#include <QLayout>
#include <QProcess>
#include <QListWidget>
#include <QTreeWidget>
#include <QDir>
#include <QMessageBox>
#include <QKeyEvent>

// Constantes

static const char *socketPath = BLACK_SOCKET_PATH;
static const char *extensionArchive = ".acidcsa";
static const QString blackIcon(PREFIX"/share/icons/cryptclt-black.png");

frmClientBlack::frmClientBlack(int argc, char **argv)
{
  setWindowTitle("Client ACID noir");
  setWindowIcon(QIcon(blackIcon));

  automaticArchiveNaming = true;

  // Onglet export
  //--------------
  tabExport = new QFrame();
  QVBoxLayout* lv_e = new QVBoxLayout(tabExport);
  lv_e->setSpacing(space);

  lv_e->addWidget(
  	new QLabel("Liste des archives pouvant être exportées", this));
  lv_e->addSpacing(5);
  // Liste des identifiants
  lv_e->addWidget(lstIds = new QListWidget(tabExport));
  lstIds->setToolTip("Sélectionnez une ou plusieurs archives chiffrées à "
                     "exporter depuis la diode.");
  lstIds->setSelectionMode(QAbstractItemView::ExtendedSelection);

  // Archive destination
  lv_e->addWidget(exportDir = new FileField(tabExport, 
                        "Répertoire destination", "",
                        "Veuillez sélectionner un répertoire destination",
                        "", EXISTING_DIRECTORY));
  exportDir->insertFile(loadDefaultPath(CRYPTCLT_OUTPUT_PATH));
  exportDir->setToolTip("Utilisez '+' pour définir le répertoire où "
                           "sauvegarder les archives exportées.");

  // Bouton Exporter
  QHBoxLayout* layExporter = new QHBoxLayout(NULL);
  layExporter->setSpacing(space);
  layExporter->addStretch(2);
  layExporter->addWidget(btnRefresh = 
  	new QPushButton(QIcon::fromTheme("edit-redo"), "Rafraîchir", this));
  btnRefresh->setToolTip("Rafraîchir la liste des archives "
                                        "pouvant être exportées.");
  layExporter->addStretch(2);
  layExporter->addWidget(btnExport = 
  	new QPushButton(QIcon::fromTheme("arrow-down"), 
					"Exporter", tabExport));
  layExporter->addStretch(2);
  btnExport->setToolTip("Exporter les archives sélectionnées, ou toutes les\n"
        "archives disponibles si aucune n'est sélectionnée,\n"
        "dans le répertoire destination.");
  layExporter->addWidget(btnExportAndQuit = 
  	new QPushButton(QIcon::fromTheme("arrow-down-double"), 
                          		"Exporter et quitter", tabExport));
  btnExportAndQuit->setToolTip("Exporter les archives puis "
                               "quitter l'application");
  layExporter->addStretch(2);
  lv_e->addLayout(layExporter);


  // Onglet import
  //--------------
  tabImport = new QFrame();
  QVBoxLayout* lv_i = new QVBoxLayout(tabImport);
  lv_i->setSpacing(space);

  // Archive
  lv_i->addWidget(toImport = new FileField(tabImport, 
                          "Archives à déchiffrer", "Archive ACID",
                          "Veuillez sélectionner une ou "
                          "plusieurs archives ACID", 
                          extensionArchive, MULTIPLE_EXISTING_FILES));
  toImport->setDefaultPath(loadDefaultPath(CRYPTCLT_INPUT_PATH));
  toImport->setToolTip(
  	"Utilisez '+' ou un glisser-déposer pour sélectionner une\n"
	"ou plusieurs archives chiffrées à importer dans la diode.\n"
	"Double-cliquez sur une archive pour afficher ses propriétés.");
  toImport->setIcon(QIcon(ICONPATH"/acidcsa.png"));

  // Bouton Importer
  QHBoxLayout* layImporter = new QHBoxLayout(NULL);
  layImporter->setSpacing(space);
  layImporter->addStretch(2);
  layImporter->addWidget(btnImport = 
  	new QPushButton(QIcon::fromTheme("arrow-up"), "Importer", tabImport));
  btnImport->setToolTip("Transférer dans la diode les archives sélectionnés,\n"
                        "ou toutes les archives de la liste si aucune n'a été\n"
                        "sélectionnée.");
  layImporter->addStretch(2);
  layImporter->addWidget(btnImportAndQuit = 
  	new QPushButton(QIcon::fromTheme("arrow-up-double"), 
			"Importer et quitter", tabImport));
  btnImportAndQuit->setToolTip("Transférer les archives dans la diode puis\n"
                               "quitter l'application.");
  layImporter->addStretch(2);
  lv_i->addLayout(layImporter);


  // Gestion du layout de la frmClientBlack
  //--------------------------------
  // Création de l'objet tabs
  tabs = new QTabWidget(this);
  tabs->addTab(tabExport, 
        QIcon::fromTheme("arrow-down"), "Export");
  tabs->setTabToolTip(0, "Récupération d'archives chiffrées depuis la diode\n"
                        "(après chiffrement).");
  tabs->addTab(tabImport, 
        QIcon::fromTheme("arrow-up"), "Import");
  tabs->setTabToolTip(1, "Transfert d'archives chiffrées dans la diode\n"
                        "(avant déchiffrement).");

  // Bouton Quitter
  QHBoxLayout* layQuit = new QHBoxLayout();
  layQuit->setSpacing(space);
  layQuit->addStretch(2);
  layQuit->addWidget(btnQuit = 
  	new QPushButton(QIcon::fromTheme("dialog-close"), "Quitter", this));
  layQuit->addStretch(2);

  // Ajout final
  QVBoxLayout* formLayout = new QVBoxLayout(this);
  formLayout->setSpacing(space);
  formLayout->addWidget(tabs);
  formLayout->addLayout(layQuit);

  refreshListIds();

  // Si on est appelé en déchiffrement
  // Prise en compte de l'argument de la ligne de commande (déchiffrement)
  //----------------------------------------------------------------------
  tabs->setCurrentIndex(0);
  
  if (argc >= 3 && strcmp (argv[1], "-i") == 0) {
    tabs->setCurrentIndex(1);
    for (int i = 2; i < argc; i++)
      toImport->insertFile(QString::fromUtf8(argv[i]));
  }

  if (argc >= 3 && strcmp(argv[1], "-e") == 0) {
    exportDir->insertFile(QString::fromUtf8(argv[2]));
  }

  // Connexions
  //-----------

  connect(tabs, SIGNAL(currentChanged(QWidget*)), 
                                          this, SLOT(refreshListIds ()));
  connect(btnRefresh, SIGNAL(clicked()), this, SLOT(refreshListIds()));
  connect(btnExport, SIGNAL(clicked()), this, SLOT(doExport()));
  connect(btnExportAndQuit, SIGNAL(clicked()), this, SLOT(exportAndQuit()));
  connect(btnImport, SIGNAL(clicked()), this, SLOT(import()));
  connect(btnImportAndQuit, SIGNAL(clicked()), this, SLOT(importAndQuit()));
  connect(btnQuit, SIGNAL(clicked()), this, SLOT(quit()));
  connect(toImport, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)),
            this, SLOT(handleDoubleClick(QTreeWidgetItem *, int)));
}


void 
frmClientBlack::keyPressEvent(QKeyEvent* e) 
{
  if (e->key() == Qt::Key_Escape) {
    e->accept();
    quit();
  }
}


void frmClientBlack::quit() {
  QStringList l = toImport->returnFilenames();
  if (!l.empty() && !yesOrNo("Confirmation",
         "Certains fichiers n'ont pas été transférés dans la diode.\n"
         "Voulez-vous vraiment quitter l'application ?")) 
    return;
  QApplication::exit (0);
}

void
frmClientBlack::handleDoubleClick(QTreeWidgetItem *item, 
                                  int col __attribute__((unused)))
{
  QProcess info;
  QStringList args;
  QString name = item->text(0);
  QString path = item->text(1);
  if (!name.isEmpty() && !path.isEmpty()) {
    path += QDir::separator();
    path += name;
    args << path;
    info.startDetached("csainfo4", args);
  }
}

// Export
//-------

void 
frmClientBlack::refreshListIds() 
{
  QIcon icon(ICONPATH"/acidcsa.png");
  if (!checkServerInfo(socketPath, CryptdCrypt))
    exit(1);
  if (tabs->currentIndex() == 0) {
      refreshList(socketPath, *lstIds, icon);
  }
}

void 
frmClientBlack::exportAndQuit()
{
  doExport ();
  if (ok) 
    quit();
}

bool 
frmClientBlack::exportOneFile(const QString &id, const QString &path)
{
  QString dest;
  ciphertext_t *ciphertext = NULL;
  uint32_t ret;
  int s = -1;
  bool retval = false;

  ciphertext = ciphertext_alloc();
  if (!ciphertext) {
    error("Plus assez de mémoire disponible.");
    goto err;
  }
  
  // Id
  ciphertext->title = strdup(id.toUtf8().data());
  if (!ciphertext->title) {
    error("Plus assez de mémoire disponible pour l'identifiant.");
    goto err;
  }
  ciphertext->tlen = strlen(ciphertext->title) + 1;

  // Nom de l'archive à exporter
  dest = path + QDir::separator() + id + extensionArchive;

  // Ouverture socket
  s = sock_connect(socketPath);
  if (s < 0) {
    error("Erreur lors de la connexion au démon cryptd.");
    goto err;
  }

  // Sélection de l'archive
  ret = cryptd_recv_ciphertext(s, ciphertext);
  if (ret != CMD_OK) {
    error(QString("Impossible de récupérer l'archive ") 
                                + id + " : " + cmderr(ret));
    goto err;
  }

  // Ecriture de l'archive
  ret = writeFile(ciphertext, dest);
  if (ret != CMD_OK) {
    error(QString("Erreur lors de l'écriture du fichier ") 
                                          + dest + " : " + cmderr(ret));
    goto err;
  }
  retval = true;
  /* Fall through */
 err:
  if (s >= 0)
    close_socket(s);
  if (ciphertext)
    ciphertext_free(ciphertext);
  return retval;
}


void 
frmClientBlack::doExport()
{
  QStringList l;
  QString path;
  ok = true;

  // Nom de l'archive à exporter
  l = exportDir->returnFilenames();
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
    error("Aucune archive n'est disponible pour être exportée.");
    ok = false;
    return;
  }
  QString transfered;
  for (QStringList::ConstIterator i=l.constBegin(); i != l.constEnd(); i++) {
    if (exportOneFile(*i, path))
      transfered += "\n " + *i + extensionArchive;
    else
      ok = false;
  }
      
  if (!transfered.isEmpty()) 
    info(QString("Les archives suivantes ont été correctement exportées "
                  "depuis la diode dans %1 : %2").arg(path).arg(transfered));

  saveDefaultPath(QFileInfo(path).absoluteFilePath(), CRYPTCLT_OUTPUT_PATH);
  refreshListIds();
}

uint32_t 
frmClientBlack::writeFile(ciphertext_t *cpr, const QString &dest)
{
  uint32_t ret;
  int ow;
  QStringList l;
  QString newdest;

  // Ecriture de l'archive
  ret = cryptd_write_file(dest.toUtf8().data(), 
                              cpr->content, cpr->clen, 0);
  if (ret != CMD_EXIST)
    goto out;

  ow = QMessageBox::question(0, "Ecraser les fichiers ?", 
      QString("Le fichier %1 existe déjà, souhaitez-vous l'écraser ?\n"
        "N.B. : répondre \"Annuler\" à cette question entraînera la perte "
        "de l'archive chiffrée.").arg(dest), 
        "Oui", "Annuler", "Choisir une autre destination", 1, 1);
  if (!ow) {
    ret = cryptd_write_file(dest.toUtf8().data(), 
                                cpr->content, cpr->clen, 1);
    goto out;
  }
  if (ow == 1)
    return CMD_OK;

  // Selection nouveau chemin
  exportDir->askUser();
  l = exportDir->returnFilenames();
  if (l.empty()) {
    error("Aucun fichier destination n'a été spécifié.");
    return CMD_EMPTY;
  }
  newdest = l.first();

  return writeFile(cpr, newdest);
  
out:
  return ret;
}

// Import
//-------

void 
frmClientBlack::importAndQuit()
{
  import ();
  if (ok) 
    quit();
}

bool 
frmClientBlack::importOneFile(const QString &fname) 
{
  QString id;
  uint32_t ret, memsz, fsz;
  int s = -1;
  bool retval = false;
  ciphertext_t *ciphertext = NULL;
  size_t clen;

  id = QFileInfo(fname).baseName();
  if (id.isEmpty()) {
    error(QString("Impossible de déterminer un identifiant "
          "pour l'archive %1").arg(fname));
    goto err;
  }

  ciphertext = ciphertext_alloc();
  if (!ciphertext) {
    error("Plus assez de mémoire disponible.");
    goto err;
  }

  memsz = get_memsize();
  fsz = get_filesize(fname.toUtf8().data());
  if (fsz > memsz / 8) {
    error(QString("Le fichier %1 occuperait dans la diode plus d'un "
      "quart de la mémoire du poste.\nIl est préférable de le découper "
      "en plusieurs fichiers plus petits avant de le "
      "transférer.\nLa taille maximale des fichiers "
      "qui peuvent être déchiffrés par la diode sur ce poste est de %2 Mo.")
      .arg(fname).arg(memsz / (8 * 1024)));
    goto err;
  }
  ret = cryptd_get_file(fname.toUtf8().data(), 
                          &(ciphertext->content), &clen);

  if (ret != CMD_OK) {
    error(QString("Impossible de lire l'archive ") 
                          + fname + " : " + cmderr(ret));
    goto err;
  }

  if (clen > UINT32_MAX) {
    error(QString("Taille incorrecte pour l'archive ") + fname);
    goto err;
  }
  ciphertext->clen = (uint32_t) clen;

  ciphertext->title = strdup(id.toUtf8().data());
  if (!ciphertext->title) {
    error("Plus assez de mémoire disponible pour l'identifiant.");
    goto err;
  }
  ciphertext->tlen = strlen(ciphertext->title) + 1;

  // Ouverture socket
  s = sock_connect(socketPath);
  if (s < 0) {
    error("Erreur lors de la connexion au démon cryptd.");
    goto err;
  }

  // Import de l'archive
  ret = cryptd_send_ciphertext(s, ciphertext);
  if (ret != CMD_OK) {
    error(QString("Erreur lors de l'import de l'archive ") 
                                        + fname + " : " + cmderr(ret));
    goto err;
  }

  retval = true;
  /* Fall through */

 err:
  if (s >= 0)
    close_socket(s);
  
  if (ciphertext)
    ciphertext_free(ciphertext);
  return retval;
}

void 
frmClientBlack::import() 
{
  QStringList l;
  QString dest, str;
  bool onlySelected = true;
  ok = true;

  l = toImport->returnSelectedFilenames();
  if (l.empty()) {
	  l = toImport->returnFilenames();
    onlySelected = false;
  }
		
  if (l.empty()) {
    error("Aucune archive à importer dans la diode n'a été spécifiée.");
    ok = false;
    return;
  }
  QString transfered;
  for (QStringList::ConstIterator i=l.constBegin(); i != l.constEnd(); i++) {
    if (importOneFile(*i))
      transfered += "\n " + *i;
    else
      ok = false;
  }
      
  if (!transfered.isEmpty()) 
    info(QString("Les fichiers suivants ont été correctement transférés "
                  "dans la diode: %1").arg(transfered));

  saveDefaultPath(QFileInfo(l.first()).absolutePath(), CRYPTCLT_INPUT_PATH);
  if (onlySelected)
    toImport->deleteSelectedFiles();
  else
    toImport->clean();
}

// vi:sw=2:ts=2:et:co=80:

