// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright © 2009-2018 ANSSI. All Rights Reserved.
/**
 * @file err.h
 * cryptclt error pretty-printing header.
 * @author Olivier Levillain <clipos@ssi.gouv.fr>
 * @author Vincent Strubel <clipos@ssi.gouv.fr>
 *
 * Copyright (C) 2009 SGDN
 * Copyright (C) 2010-2012 ANSSI
 * @n
 * All rights reserved.
 */

#ifndef _CRYPTD_ERR_H
#define _CRYPTD_ERR_H

#define UNIX
#include <acid.h>

/* Error codes */
#define CMD_OK     0x0000 /* Success */
#define CMD_ORDER  0x0001 /* Unexpected command at this point */
#define CMD_FAULT  0x0002 /* Failure executing command */
#define CMD_INVAL  0x0003 /* Invalid command */
#define CMD_NOMEM  0x0004 /* Out of memory */
#define CMD_TIMOUT 0x0005 /* Timed-out waiting for command completion */
#define CMD_NOENT  0x0006 /* No such element */
#define CMD_PERM   0x0007 /* Permission denied */
#define CMD_EXIST  0x0008 /* Object already exists */
#define CMD_EMPTY  0x0009 /* Empty answer */
#define CMD_CRYPT  0x000a /* Crypto Error */
#define CMD_NOTSUP 0x000b /* Feature not supported */
#define CMD_VERCMP 0x000c /* Incompatible versions */
#define CMD_VERCMP 0x000c /* Incompatible versions */
#define CMD_CANCEL 0x000d /* Cancelled by user */

static inline const char *
cmderr(uint32_t err)
{
  switch (err) {
    case CMD_OK:
      return "succès (?)";
    case CMD_ORDER:
      return "mauvais séquencement de commandes.\n"
        "Le client n'est peut-être pas à jour ?";
    case CMD_FAULT:
      return "erreur interne du démon.\n"
        "Le client n'est peut-être pas à jour ?";
    case CMD_INVAL:
      return "arguments incorrects.\n"
        "Le client n'est peut-être pas à jour ?";
    case CMD_NOMEM:
      return "mémoire insuffisante.";
    case CMD_TIMOUT:
      return "expiration du délai d'attente.";
    case CMD_NOENT:
      return "aucun fichier ne correspond à cet identifiant.";
    case CMD_PERM:
      return "opération refusée.";
    case CMD_EXIST:
      return "un fichier correspondant à cet identifiant existe déjà.";
    case CMD_EMPTY:
      return "résultat vide.";
    case CMD_CRYPT:
      return "erreur cryptographique.";
    case CMD_NOTSUP:
      return "fonctionnalité non supportée par le serveur.";
    case CMD_VERCMP:
      return "version serveur incompatible.";
    case CMD_CANCEL:
      return "opération annulée par l'utilisateur.";
    default:
      return "erreur inconnue.";
  }
}

static inline const char *
cryptoerr(uint32_t err)
{
  ErrCode ec = (ErrCode)err;

  switch (ec) {
    case CC_OK:
      return "pas d'erreur (?)";
    case CC_MIN_NOISE:
      return "aléa insuffisant";
    case CC_MEM:
      return "mémoire insuffisante";
    case CC_MODE_INCORRECT:
      return "mode de chiffrement inconnu";
    case CC_VI_INCORRECT:
      return "vecteur d'initialisation invalide";
    case CC_LONGUEUR_INCORRECTE:
      return "longueur à chiffrer invalide";
    case CC_INVALID:
      return "erreur de séquencement ou appel non supporté";
    case CC_BAD_SIGN:
      return "signature invalide";
    case CC_BAD_PASSWORD:
      return "mot de passe invalide";
    case CC_BAD_PERS:
      return "jeton cryptographique incorrect";
    case CC_IO:
      return "erreur d'entrée/sortie";
    case CC_DATE:
      return "certificat périmé";
    case CC_SYS_DATE:
      return "impossible d'obtenir la date du système";
    case CC_SYM_CHECK:
      return "chiffrement symétrique incorrect";
    case CC_HASH_CHECK:
      return "hachage incorrect";
    case CC_BAD_SESSION:
      return "session corrompue";
    case CC_PARSE_CERTIFICATES:
      return "certificats invalides";
    case CC_USERINFO_CERTIF_INCOHERENCE:
      return "incohérence dans le certificat";
    case CC_BAD_SIGN_CERTIFICATE:
      return "clé publique de signature incorrecte";
    case CC_BAD_CHIF_CERTIFICATE:
      return "clé publique de chiffrement incorrecte";
    case CC_BAD_ARGS:
      return "arguments incorrects";
    case CC_HARD_NOISE:
      return "erreur de génération d'aléa";
    case CC_BAD_TAG:
      return "motif d'intégrité incorrect";
    case CC_BAD_ENCAPS:
      return "encapsulation non intègre";
    case CC_NEED_CERTIFICATE:
      return "certificat manquant pour la vérification de la chaîne de certificats";
    case CC_LOOP_IN_CHAIN:
      return "chaîne de certificats trop longue";
    case CC_BAD_CERTIFICATE:
      return "type de certificat invalide";
    case CC_ISSUER_SN_NONCOHERENT:
      return "issuerSerialNumbers incohérents";
    case CC_BAD_SIGN_AT_ROOT:
      return "signature du certificat racine invalide";
    case CC_BAD_SIGN_IN_CHAIN:
      return "signature d'un certificat dans la chaîne invalide";
    case CC_INVALID_LICENSE:
      return "license de la bibliothèque cryptographique invalide";
    case CC_INVALID_SERIALNUMBER:
      return "numéros de série en conflit dans le cache";
    case CC_BAD_NEGOCIATION:
      return "négocation de clef invalide";
    default:
      return "erreur cryptographique inconnue";
  }
}

#endif /* _CRYPTD_ERR_H */
// vi:sw=2:ts=2:et:co=80:
