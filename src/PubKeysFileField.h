// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright © 2009-2018 ANSSI. All Rights Reserved.
/**
 * @file PubKeysFileField.h
 * cryptclt multiple public keys selector header.
 * @author Olivier Levillain <clipos@ssi.gouv.fr>
 * @author Vincent Strubel <clipos@ssi.gouv.fr>
 *
 * Copyright (C) 2010-2011 ANSSI
 * @n
 * All rights reserved.
 */

#ifndef PUBKEYSFILEFIELD_H
#define PUBKEYSFILEFIELD_H

#include "MultipleFileField.h"
#include <QStringList>

class PubKeysFileField : public MultipleFileField
{
    Q_OBJECT

 public:
  PubKeysFileField(QWidget *parent, const char *dialogTitle, 
                             const char *filter, file_field_t t);
  const QStringList returnFilenames ();
  const QStringList returnSelectedFilenames ();
  virtual void setVirtualIcon(const QIcon &_icon) { virtualIcon = _icon; };

 public slots:
  void deleteSelectedFiles();
  void clean();
  void insertFile(const QString&);
  void updateVirtualId(const QString&, const QString &);
  void handleDoubleClick(QTreeWidgetItem *, int);

 private:
  QStringList ids;
  QString virtualId;
  QString virtualIdPath;
  QTreeWidgetItem *virtualItem;
  QIcon virtualIcon;
};

#endif // PUBKEYSFILEFIELD_H
// vi:sw=2:ts=2:et:co=80:
