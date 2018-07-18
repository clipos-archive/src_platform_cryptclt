// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright © 2009-2018 ANSSI. All Rights Reserved.
/**
 * @file RedThreads.h
 * cryptclt red client threads header.
 * @author Olivier Levillain <clipos@ssi.gouv.fr>
 * @author Vincent Strubel <clipos@ssi.gouv.fr>
 *
 * Copyright (C) 2009 SGDN
 * Copyright (C) 2010-2011 ANSSI
 * @n
 * All rights reserved.
 */

#ifndef REDTHREADS_H
#define REDTHREADS_H

#define QT_THREAD_SUPPORT 1

#include "common.h"

#include <QThread>
#include <QEvent>

extern "C" {
#include "list.h"
}
#include <cryptd_red.h>

class QDialog;

typedef enum {
  ENCRYPTION,
  DECRYPTION,
  CHPASSWD,
} direction_t;

class EncryptionEvent : public QEvent
{
  public:
    EncryptionEvent(int type, direction_t dir);
  private:
    direction_t dir;
    bool ok;
    QString msg;
  public:
    void setMessage(const QString& _msg) { msg = _msg; };
    QString message() const { return msg; };

    void setStatus(bool b) { ok = b; };
    bool status() const { return ok; };

    direction_t direction() const { return dir; };
};

class EncryptionThread : public QThread
{
 public:
  EncryptionThread(QDialog *p, cleartext_t *cleartext, 
                                int type, bool local = false);
  void setOutput(char **cipher, uint32_t *len) { ciphertext = cipher; clen = len; };
  virtual void run();
 private:
  QDialog *parent;
  cleartext_t *clr;
  char **ciphertext;
  uint32_t *clen;
  int eventType;
  bool local;
};


class DecryptionThread : public QThread
{
 public:
  DecryptionThread(QDialog *p, cleartext_t *cleartext, bool pk, int type, 
                      char *ciphertext = NULL, uint32_t clen = 0);
  virtual void run();
 private:
  QDialog *parent;
  cleartext_t *clr;
  bool pubKey;
  int eventType;
  char *ciphertext;
  uint32_t clen;
};

class PasswordThread : public QThread
{
  public:
    PasswordThread(QDialog *p, privkey_t *prv, int type);
    virtual void run();
  private:
    QDialog *parent;
    privkey_t *prv;
    int eventType;
};

#endif // REDTHREADS_H

// vi:sw=2:ts=2:et:co=80:
