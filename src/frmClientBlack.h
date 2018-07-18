// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright © 2009-2018 ANSSI. All Rights Reserved.
/**
 * @file frmClientBlack.h
 * cryptclt black client main window header.
 * @author Olivier Levillain <clipos@ssi.gouv.fr>
 * @author Vincent Strubel <clipos@ssi.gouv.fr>
 *
 * Copyright (C) 2009 SGDN
 * Copyright (C) 2010 ANSSI
 * @n
 * All rights reserved.
 */

#ifndef FRMCLIENTBLACK_H
#define FRMCLIENTBLACK_H

#include <QDialog>
#include <QString>
#include <QDir>

#include <stdlib.h>
#include "common.h"
#include <cryptd_black.h>

class QLabel;
class QLineEdit;
class QTabWidget;
class QFrame;
class QListWidget;
class QListWidgetItem;
class QTreeWidgetItem;
class QKeyEvent;
class FileField;


class frmClientBlack : public QDialog
{
    Q_OBJECT

public:
  frmClientBlack(int argc, char **argv);

private:
  QTabWidget* tabs;
  QFrame* tabExport;
  QFrame* tabImport;
  bool ok;

  QLabel *lblListeIds;
  QListWidget *lstIds;
  FileField *exportDir;
  bool automaticArchiveNaming;
  QPushButton *btnExport;
  QPushButton *btnRefresh;
  QPushButton *btnExportAndQuit;

  FileField *toImport;
  QPushButton *btnImport;
  QPushButton *btnImportAndQuit;

  QPushButton *btnQuit;
  
protected:
  virtual void keyPressEvent(QKeyEvent *e);

public slots:
  void handleDoubleClick(QTreeWidgetItem *, int);

private slots:
  virtual void refreshListIds();
  virtual uint32_t writeFile(ciphertext_t *, const QString &);
  virtual void doExport();
  virtual void exportAndQuit();
  virtual bool exportOneFile(const QString &, const QString &);

  virtual void import();
  virtual void importAndQuit();
  virtual bool importOneFile(const QString &);

  virtual void quit();
  
};

#endif // FRMCLIENTBLACK_H
// vi:sw=2:ts=2:et:co=80:
