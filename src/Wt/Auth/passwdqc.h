/*
 * Copyright (c) 2000-2002 by Solar Designer
 * Copyright (c) 2008,2009 by Dmitry V. Levin
Redistribution and use in source and binary forms, with or without
modification, are permitted.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
SUCH DAMAGE.
 * 
 * Modified by koen@emweb.be for inclusion in Wt
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
        const char *pw_name;
        const char *pw_email;
} passwdqc_user_t;

extern int passwdqc_check(const passwdqc_params_qc_t *params,
			  const char *newpass, const char *oldpass,
			  const passwdqc_user_t *pw);

#define PASSWDQC_VERSION		"1.2.2"

#endif /* PASSWDQC_H__ */
