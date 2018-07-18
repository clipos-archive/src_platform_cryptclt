// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright © 2009-2018 ANSSI. All Rights Reserved.
/**
 * @file SingleFileField.h
 * cryptclt single file selector header.
 * @author Olivier Levillain <clipos@ssi.gouv.fr>
 * @author Vincent Strubel <clipos@ssi.gouv.fr>
 *
 * Copyright (C) 2009 SGDN
 * Copyright (C) 2010 ANSSI
 * @n
 * All rights reserved.
 */

#ifndef SINGLEFILEFIELD_H
#define SINGLEFILEFIELD_H

#include <QLineEdit>
#include "common.h"


class SingleFileField : public QLineEdit
{
    Q_OBJECT

 public:
  SingleFileField(QWidget *parent, const char *desc, const char *titreDialogue, const char *f, file_field_t t);
  virtual const QStringList returnFilename();

  QString defaultPath() const { return defPath; };
  void setDefaultPath(const QString &path) { defPath = path; };

 protected:
  void dragEnterEvent(QDragEnterEvent *e);
  void dropEvent(QDropEvent *e);
 
  QString filterSuffix;
  QString QTFilter;
  QString title;
  QString defPath;
  file_field_t type;

 public slots:
  void askUserForAFile();
  void clean();
  virtual void insertFile(const QString &s);

 signals:
  void idChanged(const QString &id, const QString &path);
};

#endif // SINGLEFILEFIELD_H
// vi:sw=2:ts=2:et:co=80:
