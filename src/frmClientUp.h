// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright © 2009-2018 ANSSI. All Rights Reserved.
/**
 * @file frmClientUp.h
 * cryptclt diode up client main window header.
 * @author Olivier Levillain <clipos@ssi.gouv.fr>
 * @author Vincent Strubel <clipos@ssi.gouv.fr>
 *
 * Copyright (C) 2009 SGDN
 * Copyright (C) 2010 ANSSI
 * @n
 * All rights reserved.
 */

#ifndef FRMCLIENTUP_H
#define FRMCLIENTUP_H

#include <QDialog>
#include <QString>

#include "common.h"
#include <stdlib.h>
#include <cryptd_red.h>

class QLabel;
class QLineEdit;
class QTabWidget;
class QFrame;
class QListWidget;
class FileField;


class frmClientUp : public QDialog
{
    Q_OBJECT

public:
  frmClientUp(int argc, char** argv);

private:
  QFrame *frame;
  bool ok;

  QLabel *lblListeIds;
  QListWidget *lstIds;
  FileField *destinationPath;
  QPushButton *btnImport;
  QPushButton *btnImportAndQuit;
  QPushButton *btnRefresh;
  QPushButton *btnQuit;
  
protected:
  virtual void keyPressEvent(QKeyEvent* e);

private slots:

  virtual void refresh();
  virtual uint32_t writeFile(file_t *, const QString &, const char *);
  virtual void import();
  virtual void importAndQuit();
  virtual bool importOneFile(const QString &, const QString &);
  virtual void quit();
  
};

#endif // FRMCLIENTUP_H
// vi:sw=2:ts=2:et:co=80:
