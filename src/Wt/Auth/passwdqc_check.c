/*
 * Copyright (c) 2000-2002,2010 by Solar Designer.
 *
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

$Owl: Owl/packages/passwdqc/passwdqc/LICENSE,v 1.7 2009/10/21 19:38:39 solar Exp $
 *
 * Modified by koen@emweb.be to leave out dictionary checks
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "passwdqc.h"

#define REASON_ERROR        1
#define REASON_SAME         2
#define REASON_SIMILAR      3
#define REASON_SHORT        4
#define REASON_LONG         5
#define REASON_SIMPLESHORT  6
#define REASON_SIMPLE       7
#define REASON_PERSONAL     8
#define REASON_WORD         9
#define REASON_SEQ          10

#define FIXED_BITS			15

typedef unsigned long fixed;

/*
 * Calculates the expected number of different characters for a random
 * password of a given length.  The result is rounded down.  We use this
 * with the _requested_ minimum length (so longer passwords don't have
 * to meet this strict requirement for their length).
 */
static int expected_different(int charset, int length)
{
	fixed x, y, z;

	x = ((fixed)(charset - 1) << FIXED_BITS) / charset;
	y = x;
	while (--length > 0)
		y = (y * x) >> FIXED_BITS;
	z = (fixed)charset * (((fixed)1 << FIXED_BITS) - y);

	return (int)(z >> FIXED_BITS);
}

/*
 * A password is too simple if it is too short for its class, or doesn't
 * contain enough different characters for its class, or doesn't contain
 * enough words for a passphrase.
 *
 * The bias may be positive or negative.  It is added to the length,
 * except that a negative bias is not considered in the passphrase
 * length check because a passphrase is expected to contain words.
 * The bias does not apply to the number of different characters; the
 * actual number is used in all checks.
 */
static int is_simple(const passwdqc_params_qc_t *params, const char *newpass,
    int bias)
{
	int length, classes, words, chars;
	int digits, lowers, uppers, others, unknowns;
	int c, p;

	length = classes = words = chars = 0;
	digits = lowers = uppers = others = unknowns = 0;
	p = ' ';
	while ((c = (unsigned char)newpass[length])) {
		length++;

		if (!isascii(c))
			unknowns++;
		else if (isdigit(c))
			digits++;
		else if (islower(c))
			lowers++;
		else if (isupper(c))
			uppers++;
		else
			others++;

/* A word starts when a letter follows a non-letter or when a non-ASCII
 * character follows a space character.  We treat all non-ASCII characters
 * as non-spaces, which is not entirely correct (there's the non-breaking
 * space character at 0xa0, 0x9a, or 0xff), but it should not hurt. */
		if (isascii(p)) {
			if (isascii(c)) {
				if (isalpha(c) && !isalpha(p))
					words++;
			} else if (isspace(p))
				words++;
		}
		p = c;

/* Count this character just once: when we're not going to see it anymore */
		if (!strchr(&newpass[length], c))
			chars++;
	}

	if (!length)
		return 1;

/* Upper case characters and digits used in common ways don't increase the
 * strength of a password */
	c = (unsigned char)newpass[0];
	if (uppers && isascii(c) && isupper(c))
		uppers--;
	c = (unsigned char)newpass[length - 1];
	if (digits && isascii(c) && isdigit(c))
		digits--;

/* Count the number of different character classes we've seen.  We assume
 * that there are no non-ASCII characters for digits. */
	classes = 0;
	if (digits)
		classes++;
	if (lowers)
		classes++;
	if (uppers)
		classes++;
	if (others)
		classes++;
	if (unknowns && classes <= 1 && (!classes || digits || words >= 2))
		classes++;

	for (; classes > 0; classes--)
	switch (classes) {
	case 1:
		if (length + bias >= params->min[0] &&
		    chars >= expected_different(10, params->min[0]) - 1)
			return 0;
		return 1;

	case 2:
		if (length + bias >= params->min[1] &&
		    chars >= expected_different(36, params->min[1]) - 1)
			return 0;
		if (!params->passphrase_words ||
		    words < params->passphrase_words)
			continue;
		if (length + (bias > 0 ? bias : 0) >= params->min[2] &&
		    chars >= expected_different(27, params->min[2]) - 1)
			return 0;
		continue;

	case 3:
		if (length + bias >= params->min[3] &&
		    chars >= expected_different(62, params->min[3]) - 1)
			return 0;
		continue;

	case 4:
		if (length + bias >= params->min[4] &&
		    chars >= expected_different(95, params->min[4]) - 1)
			return 0;
		continue;
	}

	return 1;
}

static char *unify(char *dst, const char *src)
{
	const char *sptr;
	char *dptr;
	int c;

	if (!dst && !(dst = malloc(strlen(src) + 1)))
		return NULL;

	sptr = src;
	dptr = dst;
	do {
		c = (unsigned char)*sptr;
		if (isascii(c) && isupper(c))
			c = tolower(c);
		switch (c) {
		case 'a': case '@':
			c = '4'; break;
		case 'e':
			c = '3'; break;
/* Unfortunately, if we translate both 'i' and 'l' to '1', this would
 * associate these two letters with each other - e.g., "mile" would
 * match "MLLE", which is undesired. To solve this, we'd need to test
 * different translations separately, which is not implemented yet. */
		case 'i': case '|':
			c = '!'; break;
		case 'l':
			c = '1'; break;
		case 'o':
			c = '0'; break;
		case 's': case '$':
			c = '5'; break;
		case 't': case '+':
			c = '7'; break;
		}
		*dptr++ = c;
	} while (*sptr++);

	return dst;
}

static char *reverse(const char *src)
{
	const char *sptr;
	char *dst, *dptr;

	if (!(dst = malloc(strlen(src) + 1)))
		return NULL;

	sptr = &src[strlen(src)];
	dptr = dst;
	while (sptr > src)
		*dptr++ = *--sptr;
	*dptr = '\0';

	return dst;
}

static void clean(char *dst)
{
	if (dst) {
		memset(dst, 0, strlen(dst));
		free(dst);
	}
}

/*
 * Needle is based on haystack if both contain a long enough common
 * substring and needle would be too simple for a password with the
 * substring either removed with partial length credit for it added
 * or partially discounted for the purpose of the length check.
 */
static int is_based(const passwdqc_params_qc_t *params,
    const char *haystack, const char *needle, const char *original,
    int mode)
{
	char *scratch;
	int length;
	int i, j;
	const char *p;
	int worst_bias;

	if (!params->match_length)	/* disabled */
		return 0;

	if (params->match_length < 0)	/* misconfigured */
		return 1;

	scratch = NULL;
	worst_bias = 0;

	length = strlen(needle);
	for (i = 0; i <= length - params->match_length; i++)
	for (j = params->match_length; i + j <= length; j++) {
		int bias = 0, j1 = j - 1;
		const char q0 = needle[i], *q1 = &needle[i + 1];
		for (p = haystack; *p; p++)
		if (*p == q0 && !strncmp(p + 1, q1, j1)) { /* or memcmp() */
			if ((mode & 0xff) == 0) { /* remove & credit */
				if (!scratch) {
					if (!(scratch = malloc(length + 1)))
						return 1;
				}
				/* remove j chars */
				{
					int pos = length - (i + j);
					if (!(mode & 0x100)) /* not reversed */
						pos = i;
					memcpy(scratch, original, pos);
					memcpy(&scratch[pos],
					    &original[pos + j],
					    length + 1 - (pos + j));
				}
				/* add credit for match_length - 1 chars */
				bias = params->match_length - 1;
				if (is_simple(params, scratch, bias)) {
					clean(scratch);
					return 1;
				}
			} else { /* discount */
/* Require a 1 character longer match for substrings containing leetspeak
 * when matching against dictionary words */
				bias = -1;
				if ((mode & 0xff) == 1) { /* words */
					int pos = i, end = i + j;
					if (mode & 0x100) { /* reversed */
						pos = length - end;
						end = length - i;
					}
					for (; pos < end; pos++)
					if (!isalpha((int)(unsigned char)
					    original[pos])) {
						if (j == params->match_length)
							goto next_match_length;
						bias = 0;
						break;
					}
				}

				/* discount j - (match_length + bias) chars */
				bias += (int)params->match_length - j;
				/* bias <= -1 */
				if (bias < worst_bias) {
					if (is_simple(params, original, bias))
						return 1;
					worst_bias = bias;
				}
			}
		}
/* Zero bias implies that there were no matches for this length.  If so,
 * there's no reason to try the next substring length (it would result in
 * no matches as well).  We break out of the substring length loop and
 * proceed with all substring lengths for the next position in needle. */
		if (!bias)
			break;
next_match_length:
		;
	}

	clean(scratch);

	return 0;
}

/*
 * Common sequences of characters.
 * We don't need to list any of the characters in reverse order because the
 * code checks the new password in both "unified" and "unified and reversed"
 * form against these strings (unifying them first indeed).  We also don't
 * have to include common repeats of characters (e.g., "777", "!!!", "1000")
 * because these are often taken care of by the requirement on the number of
 * different characters.
 */
const char *seq[] = {
	"0123456789",
	"`1234567890-=",
	"~!@#$%^&*()_+",
	"abcdefghijklmnopqrstuvwxyz",
	"qwertyuiop[]\\asdfghjkl;'zxcvbnm,./",
	"qwertyuiop{}|asdfghjkl:\"zxcvbnm<>?",
	"qwertyuiopasdfghjklzxcvbnm",
	"1qaz2wsx3edc4rfv5tgb6yhn7ujm8ik,9ol.0p;/-['=]\\",
	"qazwsxedcrfvtgbyhnujmikolp"
};

static int is_word_based(const passwdqc_params_qc_t *params,
    const char *needle, const char *original, int is_reversed)
{
        char word[7];
        char *unified;
        unsigned int i;
        int length;
        int mode;

        if (!params->match_length)      /* disabled */
                return 0;

        mode = is_reversed | 2;
        for (i = 0; i < sizeof(seq) / sizeof(seq[0]); i++) {
                unified = unify(NULL, seq[i]);
                if (!unified)
                        return REASON_ERROR;
                if (is_based(params, unified, needle, original, mode)) {
                        free(unified);
                        return REASON_SEQ;
                }
                free(unified);
        }

	/*
        mode = is_reversed | 1;
        word[6] = '\0';

        for (i = 0; i < 0x1000; i++) {
                memcpy(word, _passwdqc_wordset_4k[i], 6);
                length = strlen(word);
                if (length < params->match_length)
                        continue;
                if (i < 0xfff &&
                    !memcmp(word, _passwdqc_wordset_4k[i + 1], length))
                        continue;
                unify(word, word);
                if (is_based(params, word, needle, original, mode))
                        return REASON_WORD;
        }
	*/

        mode = is_reversed | 2;
        if (params->match_length <= 4)
        for (i = 1900; i <= 2039; i++) {
                sprintf(word, "%u", i);
                if (is_based(params, word, needle, original, mode))
                        return REASON_SEQ;
        }

        return 0;
}

int passwdqc_check(const passwdqc_params_qc_t *params,
		   const char *newpass, const char *oldpass,
		   const passwdqc_user_t *pw)
{
	char truncated[9];
	char *u_newpass, *u_reversed;
	char *u_oldpass;
	char *u_name, *u_email;
	int reason;
	int length;

	u_newpass = u_reversed = NULL;
	u_oldpass = NULL;
	u_name = u_email = NULL;

	reason = REASON_ERROR;

	if (oldpass && !strcmp(oldpass, newpass)) {
		reason = REASON_SAME;
		goto out;
	}

	length = strlen(newpass);

	if (length < params->min[4]) {
		reason = REASON_SHORT;
		goto out;
	}

	if (length > params->max) {
		if (params->max == 8) {
			truncated[0] = '\0';
			strncat(truncated, newpass, 8);
			newpass = truncated;
			if (oldpass && !strncmp(oldpass, newpass, 8)) {
				reason = REASON_SAME;
				goto out;
			}
		} else {
			reason = REASON_LONG;
			goto out;
		}
	}

	if (is_simple(params, newpass, 0)) {
		reason = REASON_SIMPLE;
		if (length < params->min[1] && params->min[1] <= params->max)
			reason = REASON_SIMPLESHORT;
		goto out;
	}

	if (!(u_newpass = unify(NULL, newpass)))
		goto out; /* REASON_ERROR */
	if (!(u_reversed = reverse(u_newpass)))
		goto out;
	if (oldpass && !(u_oldpass = unify(NULL, oldpass)))
		goto out;
	if (pw) {
		if (!(u_name = unify(NULL, pw->pw_name)) ||
		    !(u_email = unify(NULL, pw->pw_email)))
			goto out;
	}

	if (pw &&
	    (is_based(params, u_name, u_newpass, newpass, 0) ||
	     is_based(params, u_name, u_reversed, newpass, 0x100) ||
	     is_based(params, u_email, u_newpass, newpass, 0) ||
	     is_based(params, u_email, u_reversed, newpass, 0x100))) {
		reason = REASON_PERSONAL;
		goto out;
	}

	reason = is_word_based(params, u_newpass, newpass, 0);
        if (!reason)
	  reason = is_word_based(params, u_reversed, newpass, 0x100);

out:
	memset(truncated, 0, sizeof(truncated));
	clean(u_newpass);
	clean(u_reversed);
	clean(u_oldpass);
	clean(u_name);
	clean(u_email);

	return reason;
}
