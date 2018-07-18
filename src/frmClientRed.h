// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright © 2009-2018 ANSSI. All Rights Reserved.
/**
 * @file frmClientRed.h
 * cryptclt red client main window header.
 * @author Olivier Levillain <clipos@ssi.gouv.fr>
 * @author Vincent Strubel <clipos@ssi.gouv.fr>
 *
 * Copyright (C) 2009 SGDN
 * Copyright (C) 2010 ANSSI
 * @n
 * All rights reserved.
 */

#ifndef FRMCLIENTRED_H
#define FRMCLIENTRED_H

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

class QLabel;
class QLineEdit;
class QTabWidget;
class QFrame;
class QTreeWidget;

class FileField;
class DecryptFrame;

class frmClientRed : public QDialog
{
  Q_OBJECT

public:
  frmClientRed(int argc, char **argv);
  void clean(direction_t dir);
  void freeze();
  void unfreeze();
  virtual ~frmClientRed();

private:
  uint32_t features;

  QTabWidget *tabs;
  QFrame *tabEncrypt;
  QFrame *tabDecrypt;
  QFrame *tabPassword;
  bool ok;
  int eventType;

  /* Encryption */
  FileField *files;
  FileField *recipients;
  FileField *sender;
  QLabel *lblId;
  QLineEdit *txtId;
  QPushButton *btnEncrypt;
  QPushButton *btnEncryptAndQuit;
  QPushButton *btnEncryptLocal;
  EncryptionThread *encryptionThread;

  /* Decryption */
  DecryptFrame *frmDecrypt;

  /* Password change */
  FileField *inputKey;
  FileField *outputKey;
  QPushButton *btnChPw;
  QThread *passwordThread;

  bool done;
  QPushButton *btnQuit;

  void doEncrypt(bool local, char **content, uint32_t *len);

  uint32_t writeKey(privkey_t *, const QString &);

protected:
  void keyPressEvent(QKeyEvent *e);
  void customEvent(QEvent *e);

private slots:
  void encrypt();
  void encryptLocal();

  void chpw();
  void updateOutputKey();

  void quit();
};

#endif // FRMCLIENTRED_H

// vi:sw=2:ts=2:et:co=80:
