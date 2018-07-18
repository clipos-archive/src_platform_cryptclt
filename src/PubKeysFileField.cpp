// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright © 2009-2018 ANSSI. All Rights Reserved.
/**
 * @file PubKeysFileField.cpp
 * cryptclt multiple public keys selector implementation.
 * @author Olivier Levillain <clipos@ssi.gouv.fr>
 * @author Vincent Strubel <clipos@ssi.gouv.fr>
 *
 * Copyright (C) 2010-2011 ANSSI
 * @n
 * All rights reserved.
 */

#include "PubKeysFileField.h"

#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFileDialog>
#include <QDir>
#include <QProcess>
#include <cryptd/cryptd_files.h>
#include <clip/acidfile.h>

#include <exception>
#include "err.h"

PubKeysFileField::PubKeysFileField(QWidget* parent,
      const char* dialog, const char* f, file_field_t t)
  : MultipleFileField(parent, "Identité", dialog, f, MULTIPLE_EXISTING_FILES),
    virtualId(), virtualIdPath()
{
  if (t != MULTIPLE_EXISTING_PUBLIC_KEYS)
    throw std::exception();

  virtualItem = NULL;
  connect(this, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)),
            this, SLOT(handleDoubleClick(QTreeWidgetItem *, int)));

  setIcon(QIcon(ICONPATH"/acidppr.png"));
  setToolTip("Utilisez '+' ou un glisser-déposer pour ajouter des clés "
             "publiques de destinataires.\n"
             "Double-cliquez sur une clé pour afficher "
             "ses informations détaillées.");
}

void 
PubKeysFileField::insertFile(const QString& s) 
{
  acid_key_t *certs = NULL;
  const char *id;
  char *bundle = NULL;
  uint32_t blen, ret;
  size_t bsize;
  QFileInfo info(s);
  QStringList cols; 
  QTreeWidgetItem *item; 

  if (!checkFile(s, filterSuffix, type)) {
    qDebug("unsupported key file: %s", s.toUtf8().data());
    return;
  }

  ret = cryptd_get_file(s.toUtf8().data(), &bundle, &bsize);
  if (ret != CMD_OK || bsize > UINT32_MAX)
    return;

  blen = (uint32_t) bsize;
  if (acidfile_check_headers(bundle, blen, Acid_FType_Pub, NULL)) {
    error(QString("Le fichier %1 n'est pas une clé "
                  "publique correctement formatée.").arg(s));
    goto out;
  }
  if (acidfile_pxr_get_certs(bundle, &certs)) {
    error(QString("Le fichier %1 n'est pas une clé "
                  "publique correctement formatée.").arg(s));
    goto out;
  }

  id = acid_key_field(certs->next, ACID_Subject);
  if (!id) {
    error(QString("Le fichier %1 n'est pas une clé "
                  "publique correctement formatée.").arg(s));
    goto out;
  }
  if (ids.contains(id)) {
    error(QString("Une clé publique est déjà présente pour "
                  "l'identité %1.").arg(id));
    goto out;
  }

  cols << id << info.absoluteFilePath();
  item = new QTreeWidgetItem(this, cols);
  if (!icon.isNull())
    item->setIcon(0, icon);
  ids.append(QString(id));

  defPath = info.absolutePath();
  resizeColumnToContents(0);
  
  /* Fall through */
out:
  if (bundle)
    free(bundle);
  if (certs)
    acid_key_free_all(certs);
}

const QStringList 
PubKeysFileField::returnFilenames() 
{
  QStringList res;
  QTreeWidgetItemIterator i(this);

  while (*i) {
    if (!(*i)->text(1).isEmpty())
      res.append((*i)->text(1));
    ++i;
  }
  return res;
}

const QStringList 
PubKeysFileField::returnSelectedFilenames() 
{
  QStringList res;
  QTreeWidgetItemIterator i(this, QTreeWidgetItemIterator::Selected);

  while (*i) {
    if (!(*i)->text(1).isEmpty())
      res.append((*i)->text(1));
    ++i;
  }
  return res;
}

void 
PubKeysFileField::deleteSelectedFiles() 
{
  QList<QTreeWidgetItem *> l = selectedItems();
  QMutableListIterator<QTreeWidgetItem *> i(l);
  while (i.hasNext()) {
    QTreeWidgetItem *item = i.next();

    if (item->text(1).isEmpty()) {
      error("L'identifiant associé à la clé privée est implicite, "
            "il ne peut pas être supprimé.");
      continue;
    }

    int index = ids.indexOf(item->text(0));
    if (index == -1)
      qDebug("ID not found in IDs: %s", item->text(0).toUtf8().data());
    else
      ids.removeAt(index);

    index = indexOfTopLevelItem(item);
    delete takeTopLevelItem(index);
  }
}

void
PubKeysFileField::updateVirtualId(const QString &s, const QString &path)
{
  QStringList cols;

  if (!virtualId.isEmpty()) {
    int index = ids.indexOf(virtualId);
    if (index != -1)
      ids.removeAt(index);
    if (virtualItem) {
      index = indexOfTopLevelItem(virtualItem);
      delete takeTopLevelItem(index);
    }
  }
  
  cols << QString("[%1]").arg(s) << "";
  virtualItem = new QTreeWidgetItem(this, cols);
  if (!virtualIcon.isNull())
    virtualItem->setIcon(0, virtualIcon);
  ids.append(s);
  virtualId = s;
  virtualIdPath = path;
  resizeColumnToContents(0);
}

void 
PubKeysFileField::clean() 
{
  clear();
  ids.clear();
  virtualItem = NULL;
  updateVirtualId(virtualId, virtualIdPath);
}

void
PubKeysFileField::handleDoubleClick(QTreeWidgetItem *item, 
                               int col __attribute__((unused)))
{
  QProcess pxrinfo;
  QStringList args;
  QString path = item->text(1);
  if (!item->text(1).isEmpty()) {
    args << item->text(1);
    pxrinfo.startDetached("pxrinfo4", args);
  } else if (!virtualIdPath.isEmpty()) {
    args << virtualIdPath;
    pxrinfo.startDetached("pxrinfo4", args);
  }
}
// vi:sw=2:ts=2:et:co=80:
