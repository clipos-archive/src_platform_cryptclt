// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright © 2009-2018 ANSSI. All Rights Reserved.
/**
 * @file frmClientDecrypt.h
 * cryptclt decrypt client main window header.
 * @author Vincent Strubel <clipos@ssi.gouv.fr>
 *
 * Copyright (C) 2011 ANSSI
 * @n
 * All rights reserved.
 */

#ifndef FRMCLIENTDECRYPT_H
#define FRMCLIENTDECRYPT_H

#include "common.h"

#include <QDialog>
#include <QThread>
#include <QRadioButton>
#include <QEvent>
#include "RedThreads.h"

extern "C" {
#include "list.h"
}
#include <cryptd_red.h>

class QPushButton;

class FileField;
class DecryptFrame;

class frmClientDecrypt : public QDialog
{
  Q_OBJECT
public:
  frmClientDecrypt(int argc, char **argv);

private:
  uint32_t features;
  int eventType;

  FileField *archive;
  DecryptFrame *frmDecrypt;
  QPushButton *btnDecrypt;
  QPushButton *btnInfo;
  QPushButton *btnQuit;

protected:
  void keyPressEvent(QKeyEvent *e);
  void customEvent(QEvent *e);

private slots:
  void decrypt();
  void quit();
  void info();
};

#endif // FRMCLIENTDECRYPT_H

// vi:sw=2:ts=2:et:co=80:
