/*
 * Copyright (c) 2000-2002 by Solar Designer
 * Copyright (c) 2008,2009 by Dmitry V. Levin
 * See LICENSE
 */

#ifndef PASSWDQC_H__
#define PASSWDQC_H__

typedef struct {
	int min[5], max;
	int passphrase_words;
	int match_length;
	int similar_deny;
	int random_bits;
} passwdqc_params_qc_t;

typedef struct {
	int flags;
	int retry;
} passwdqc_params_pam_t;

typedef struct {
	passwdqc_params_qc_t qc;
	passwdqc_params_pam_t pam;
} passwdqc_params_t;

extern const char *passwdqc_check(const passwdqc_params_qc_t *params,
    const char *newpass, const char *oldpass /*, const struct passwd *pw*/);
extern char *passwdqc_random(const passwdqc_params_qc_t *params);

extern int passwdqc_params_parse(passwdqc_params_t *params,
    char **reason, int argc, const char *const *argv);
extern int passwdqc_params_load(passwdqc_params_t *params,
    char **reason, const char *pathname);
extern void passwdqc_params_reset(passwdqc_params_t *params);

#define F_ENFORCE_MASK			0x00000003
#define F_ENFORCE_USERS			0x00000001
#define F_ENFORCE_ROOT			0x00000002
#define F_ENFORCE_EVERYONE		F_ENFORCE_MASK
#define F_NON_UNIX			0x00000004
#define F_ASK_OLDAUTHTOK_MASK		0x00000030
#define F_ASK_OLDAUTHTOK_PRELIM		0x00000010
#define F_ASK_OLDAUTHTOK_UPDATE		0x00000020
#define F_CHECK_OLDAUTHTOK		0x00000040
#define F_USE_FIRST_PASS		0x00000100
#define F_USE_AUTHTOK			0x00000200

#define PASSWDQC_VERSION		"1.2.2"

#endif /* PASSWDQC_H__ */
