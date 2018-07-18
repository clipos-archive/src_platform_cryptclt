// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright © 2009-2018 ANSSI. All Rights Reserved.
/**
 * @file DecryptFrame.h
 * cryptclt red client main window header.
 * @author Olivier Levillain <clipos@ssi.gouv.fr>
 * @author Vincent Strubel <clipos@ssi.gouv.fr>
 *
 * Copyright (C) 2009 SGDN
 * Copyright (C) 2010-2011 ANSSI
 * @n
 * All rights reserved.
 */

#ifndef DECRYPTFRAME_H
#define DECRYPTFRAME_H

#include "common.h"
#include "RedThreads.h"

#include <QFrame>
#include <QThread>

class QCheckBox;
class QDialog;
class QFrame;
class QLabel;
class QPushButton;
class QLineEdit;
class QListWidget;
class QTabWidget;

class FileField;

class DecryptFrame : public QFrame
{
  Q_OBJECT

public:
  DecryptFrame(int features, int eventType, QDialog *parent, 
                                              bool drawAll = false);
  virtual ~DecryptFrame();
  void gotEvent(bool status);
  void setPath(const QString &path);
  bool isOk() const { return ok; };

private:
  uint32_t features;
  bool ok;
  int eventType;
  QDialog *parent;

  /* Decryption */
  QListWidget *lstIds;
  FileField *decryptionKey;
  FileField *destinationPath;
  QPushButton *btnRefresh;
  QPushButton *btnDelete;
  QPushButton *btnDecrypt;
  QPushButton *btnDecryptLocal;
  QCheckBox *btnSavePubKey;
  DecryptionThread *decryptionThread;

  bool done;

  void doDecrypt(const QString &id, 
              char *content = NULL, uint32_t len = 0);

  uint32_t writeFiles(cleartext_t *, const QString &, const char*);

public slots:
  void refreshListIds();
  void deleteArchive();
  void decrypt();
  void decryptLocal(const QString &name);
  void decryptLocalAsk();
};

#endif // DECRYPTFRAME_H

// vi:sw=2:ts=2:et:co=80:
