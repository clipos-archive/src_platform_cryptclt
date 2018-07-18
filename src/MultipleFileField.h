// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright © 2009-2018 ANSSI. All Rights Reserved.
/**
 * @file MultipleFileField.h
 * cryptclt multiple file selector header.
 * @author Olivier Levillain <clipos@ssi.gouv.fr>
 * @author Vincent Strubel <clipos@ssi.gouv.fr>
 *
 * Copyright (C) 2009 SGDN
 * Copyright (C) 2010-2011 ANSSI
 * @n
 * All rights reserved.
 */

#ifndef MULTIPLEFILEFIELD_H
#define MULTIPLEFILEFIELD_H

#include <QTreeWidget>
#include <QStringList>
#include <QIcon>
#include "common.h"

class QMimeData;

class MultipleFileField : public QTreeWidget
{
    Q_OBJECT

 public:
  MultipleFileField(QWidget *parent, const char *columnTitle, const char *dialogTitle, const char *filter, file_field_t t);
  virtual const QStringList returnFilenames();
  virtual const QStringList returnSelectedFilenames();

  QString defaultPath() const { return defPath; };
  void setDefaultPath(const QString &path) { defPath = path; };
  void setIcon(const QIcon &_icon) { icon = _icon; };
  virtual void setVirtualIcon(const QIcon &_icon __attribute__((unused))) {};

 protected:
  virtual void dropEvent(QDropEvent *e);
  QStringList mimeTypes() const;
  Qt::DropActions supportedDropActions() const;

  QString filterSuffix;
  QString QTFilter;
  QString title;
  QString defPath;
  file_field_t type;
  QIcon icon;

 public slots:
  void askUserForFiles();
  virtual void deleteSelectedFiles();
  virtual void clean();
  virtual void insertFile(const QString& s);
  virtual void updateVirtualId(const QString &, const QString &);
};

#endif // MULTIPLEFILEFIELD_H
// vi:sw=2:ts=2:et:co=80:
