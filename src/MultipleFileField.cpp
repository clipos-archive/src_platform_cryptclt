// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright © 2009-2018 ANSSI. All Rights Reserved.
/**
 * @file MultipleFileField.cpp
 * cryptclt multiple file selector implementation.
 * @author Olivier Levillain <clipos@ssi.gouv.fr>
 * @author Vincent Strubel <clipos@ssi.gouv.fr>
 *
 * Copyright (C) 2009 SGDN
 * Copyright (C) 2010-2011 ANSSI
 * @n
 * All rights reserved.
 */

#include "MultipleFileField.h"

#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFileDialog>
#include <QDir>
#include <QStringList>

#include <exception>

MultipleFileField::MultipleFileField(QWidget* parent, const char* columnTitle, 
      const char* dialogTitle, const char* f, file_field_t t)
  : QTreeWidget(parent), filterSuffix(f), title(dialogTitle),
    defPath(QDir::homePath()), type(t)
{
  if (t != MULTIPLE_EXISTING_FILES)
    throw std::exception();

  setColumnCount(2);
  QStringList headers;
  headers << columnTitle << "Emplacement";
  setHeaderLabels(headers);

  if (filterSuffix.length() > 0)
    QTFilter = QString(columnTitle) + " (*" + filterSuffix + ")";

  setSelectionMode(QAbstractItemView::ExtendedSelection);
  setAcceptDrops(true);
}

void 
MultipleFileField::dropEvent(QDropEvent* e) 
{
  QStringList l = filenamesFromDnD(e->mimeData(), filterSuffix, type);

  QStringListIterator i(l);
  while (i.hasNext()) {
    insertFile(i.next());
  }
}

QStringList 
MultipleFileField::mimeTypes() const
{
  QStringList l;
  l.append("text/uri-list");
  return l;
}

Qt::DropActions 
MultipleFileField::supportedDropActions() const
{
  return Qt::CopyAction | Qt::MoveAction;
}

void 
MultipleFileField::askUserForFiles() 
{
  if (!QDir(defaultPath()).exists()) 
    setDefaultPath(QDir::homePath());

  QStringList l = QFileDialog::getOpenFileNames(this, title, defaultPath(), QTFilter);

  QStringListIterator i(l);
  while (i.hasNext())
    insertFile(i.next());
}

void 
MultipleFileField::insertFile(const QString& s) 
{
  if (checkFile(s, filterSuffix, type)) {
    QFileInfo info(s);
    QStringList cols; 
    cols << info.fileName() << info.dir().path();
    QTreeWidgetItem *item = new QTreeWidgetItem(this, cols);
    if (!icon.isNull())
    item->setIcon(0, icon);
    defPath = info.absolutePath();
    resizeColumnToContents(0);
  }
}

void 
MultipleFileField::deleteSelectedFiles() 
{
  //QTreeWidgetItemIterator i(this, QTreeWidgetIterator::Selected);
  QList<QTreeWidgetItem *> l = selectedItems();
  QMutableListIterator<QTreeWidgetItem *> i(l);
  while (i.hasNext()) {
    int index = indexOfTopLevelItem(i.next());
    delete takeTopLevelItem(index);
  }
}

void 
MultipleFileField::clean() 
{
  clear();
}

const QStringList 
MultipleFileField::returnFilenames() 
{
  QStringList res;
  QTreeWidgetItemIterator i(this);

  while (*i) {
    res.append((*i)->text(1) + QDir::separator() + (*i)->text(0));
    ++i;
  }
  return res;
}

const QStringList 
MultipleFileField::returnSelectedFilenames() 
{
  QStringList res;
  QTreeWidgetItemIterator i(this, QTreeWidgetItemIterator::Selected);

  while (*i) {
    res.append((*i)->text(1) + QDir::separator() + (*i)->text(0));
    ++i;
  }
  return res;
}

void 
MultipleFileField::updateVirtualId(const QString &s __attribute__((unused)),
                                   const QString &path __attribute__((unused)))
{ }
// vi:sw=2:ts=2:et:co=80:
