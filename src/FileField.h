// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright © 2009-2018 ANSSI. All Rights Reserved.
/**
 * @file FileField.h
 * cryptclt generic file selector header.
 * @author Olivier Levillain <clipos@ssi.gouv.fr>
 * @author Vincent Strubel <clipos@ssi.gouv.fr>
 *
 * Copyright (C) 2009 SGDN
 * Copyright (C) 2010-2011 ANSSI
 * @n
 * All rights reserved.
 */

#ifndef FILEFIELD_H
#define FILEFIELD_H

#include <QFrame>
#include <QIcon>
#include <QStringList>
#include "common.h"

class QLabel;
class QPushButton;
class QTreeWidgetItem;
class MultipleFileField;
class SingleFileField;

class FileField : public QFrame
{
    Q_OBJECT

 public:
  FileField (QWidget* parent, const char* label, const char* desc,
	     const char* dialogTitle, const char* filter, file_field_t t);
  
  void insertFile(const QString& s);
  void clean();
  virtual const QStringList returnFilenames();
  virtual const QStringList returnSelectedFilenames();
  virtual void deleteSelectedFiles();

  QString defaultPath() const;
  void setDefaultPath(const QString &);
  void setIcon(const QIcon &);
  void setVirtualIcon(const QIcon &);

 private:
  MultipleFileField *multipleFiles;
  SingleFileField *singleFile;

 signals:
  void valueChanged(const QString&);
  void idChanged(const QString &, const QString &s);
  void cleanSignal();
  void insertSignal(const QString&);
  void itemDoubleClicked(QTreeWidgetItem *item, int col);

 public slots:
  void askUser();  
  void updateVirtualId(const QString&, const QString &s);
};

#endif // FILEFIELD_H
// vi:sw=2:ts=2:et:co=80:
