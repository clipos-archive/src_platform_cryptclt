// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright © 2009-2018 ANSSI. All Rights Reserved.
/**
 * @file RedThreads.cpp
 * cryptclt red client threads implementation.
 * @author Olivier Levillain <clipos@ssi.gouv.fr>
 * @author Vincent Strubel <clipos@ssi.gouv.fr>
 *
 * Copyright (C) 2009 SGDN
 * Copyright (C) 2010-2011 ANSSI
 * @n
 * All rights reserved.
 */

#include "RedThreads.h"
#include "DecryptFrame.h"
#include "frmClientRed.h"
#include "common.h"
#include "err.h"

#include <QApplication>

// Constantes

static const char *socketPath = RED_SOCKET_PATH;

// Evenements
//-----------

EncryptionEvent::EncryptionEvent(int type, direction_t dir)
  : QEvent(QEvent::Type(type)), dir(dir), ok(ok)
{}

// Gestion des threads
//--------------------

EncryptionThread::EncryptionThread(QDialog *p, 
                    cleartext_t *cleartext, int type, bool local)
  : parent(p), clr(cleartext), eventType(type), local(local) 
{}


DecryptionThread::DecryptionThread(QDialog *p, cleartext_t *cleartext, 
        bool pk, int type, char *cipher, uint32_t clen)
  : parent(p), clr(cleartext), pubKey(pk), eventType(type), 
    ciphertext(cipher), clen(clen)
{}

PasswordThread::PasswordThread(QDialog *p, privkey_t *prv, int type)
  : parent(p), prv(prv), eventType(type)
{}


void 
EncryptionThread::run() {
  uint32_t ret, err;
  EncryptionEvent *e = new EncryptionEvent(eventType, ENCRYPTION); 

  int s = sock_connect(socketPath);
  if (s < 0) {
    e->setStatus(false);
    e->setMessage("Erreur lors de la connexion au démon cryptd.");
    QApplication::postEvent(parent, e);
    goto fin_tc;
  }

  if (local && ciphertext && clen) {
    ret = cryptd_encrypt_cleartext(s, clr, ciphertext, clen, &err);
  } else {
    ret = cryptd_send_cleartext(s, clr, &err);
  }

  if (ret != CMD_OK) {
    QString errstr;
    if (ret == CMD_CRYPT) {
      if (err == CC_BAD_PERS)
        errstr = QString("Le jeton cryptographique associé à la clé privée "
                          "ne permet pas de chiffrer pour l'ensemble des "
                          "destinataires.");
      else
        errstr = QString("Erreur cryptographique lors du "
                        "chiffrement : %1.").arg(cryptoerr(err));
    } else if (ret == CMD_CANCEL)
      errstr = QString("Opération de chiffrement annulée dans le socle.");
    else
      errstr = QString ("Erreur lors de la transmission au "
                        "démon de chiffrement : %1").arg(cmderr(ret));
    e->setStatus(false);
    e->setMessage(errstr);
    QApplication::postEvent(parent, e);
    goto fin_tc;
  }

  if (!local) {
    e->setMessage(QString("L'archive a été correctement chiffrée "
                  "sous l'identifiant %1.").arg(clr->title));
  }
  e->setStatus(true);
  QApplication::postEvent(parent, e);
  // Fall through
 fin_tc:
  if (s >= 0)
    close_socket(s);
  cleartext_free(clr);
}

void 
DecryptionThread::run() 
{
  uint32_t ret, err;
  EncryptionEvent *e = new EncryptionEvent(eventType, DECRYPTION); 
  int s = sock_connect(socketPath);
  int pub = (pubKey) ? 1 : 0;
  if (s < 0) {
    e->setStatus(false);
    e->setMessage("Erreur lors de la connexion au démon cryptd.");
    QApplication::postEvent(parent, e);
    goto fin_td;
  }

  if (ciphertext && clen) {
    ret = cryptd_decrypt_ciphertext(s, clr, ciphertext, clen, pub, &err);
  } else {
    ret = cryptd_recv_cleartext(s, clr, pub, &err);
  }
  if (ret != CMD_OK) {
    QString errstr;
    if (ret == CMD_CRYPT) {
      if (err == CC_BAD_PERS)
        errstr = QString("La clé privée ne fournit pas le jeton "
                          "cryptographique nécessaire au déchiffrement.");
      else
        errstr = QString("Erreur cryptographique lors du déchiffrement "
              ": %1.").arg(cryptoerr(err));
    } else if (ret == CMD_CANCEL)
      errstr = QString("Opération de déchiffrement annulée dans le socle.");
    else
      errstr = QString ("Erreur lors du déchiffrement "
              ": %1.").arg(cmderr(ret));
    e->setStatus(false);
    e->setMessage(errstr);
    QApplication::postEvent(parent, e);
    goto fin_td;
  }

  e->setStatus(true);
  e->setMessage("");
  QApplication::postEvent(parent, e);

 fin_td:
  if (s >= 0)
    close_socket(s);
}

void 
PasswordThread::run() 
{
  uint32_t ret, err;
  EncryptionEvent *e = new EncryptionEvent(eventType, CHPASSWD); 
  int s = sock_connect(socketPath);
  if (s < 0) {
    e->setStatus(false);
    e->setMessage("Erreur lors de la connexion au démon cryptd.");
    QApplication::postEvent(parent, e);
    goto fin_td;
  }

  ret = cryptd_change_password(s, prv, &err);
  if (ret != CMD_OK) {
    QString errstr;
    if (ret == CMD_CRYPT)
      errstr = QString("Erreur cryptographique lors du changement "
              "de mot de passe "
              ": %1.").arg(cryptoerr(err));
    else if (ret == CMD_CANCEL)
      errstr = QString("Changement de mot de passe annulé dans le socle.");
    else
      errstr = QString ("Impossible de récupérer la nouvelle clé "
              ": %1.").arg(cmderr(ret));
    e->setStatus(false);
    e->setMessage(errstr);
    QApplication::postEvent(parent, e);
    goto fin_td;
  }

  e->setStatus(true);
  e->setMessage("");
  QApplication::postEvent(parent, e);

 fin_td:
  if (s >= 0)
    close_socket(s);
}

// vi:sw=2:ts=2:et:co=80:
