/*
 * Copyright (c) 1989 - 1994, Julianne Frances Haugh
 * Copyright (c) 1996 - 1998, Marek Michałkiewicz
 * Copyright (c) 2003 - 2005, Tomasz Kłoczko
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the copyright holders or contributors may not be used to
 *    endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT
 * HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <config.h>

#ident "$Id$"

#include <sys/types.h>
#include "prototypes.h"
#include "defines.h"
#include <stdio.h>
#define	FIELDS	9
#define	OFIELDS	5
/*
 * sgetspent - convert string in shadow file format to (struct spwd *)
 */
struct spwd *sgetspent (const char *string)
{
	static char spwbuf[1024];
	static struct spwd spwd;
	char *fields[FIELDS];
	char *cp;
	char *cpp;
	int i;

	/*
	 * Copy string to local buffer.  It has to be tokenized and we
	 * have to do that to our private copy.
	 */

	if (strlen (string) >= sizeof spwbuf) {
		return 0;	/* fail if too long */
	}
	strcpy (spwbuf, string);

	cp = strrchr (spwbuf, '\n');
	if (NULL != cp) {
		*cp = '\0';
	}

	/*
	 * Tokenize the string into colon separated fields.  Allow up to
	 * FIELDS different fields.
	 */

	for (cp = spwbuf, i = 0; ('\0' != *cp) && (i < FIELDS); i++) {
		fields[i] = cp;
		while (('\0' != *cp) && (':' != *cp)) {
			cp++;
		}

		if ('\0' != *cp) {
			*cp = '\0';
			cp++;
		}
	}

	if (i == (FIELDS - 1)) {
		fields[i++] = cp;
	}

	if ( ((NULL != cp) && ('\0' != *cp)) ||
	     ((i != FIELDS) && (i != OFIELDS)) ) {
		return 0;
	}

	/*
	 * Start populating the structure.  The fields are all in
	 * static storage, as is the structure we pass back.
	 */

	spwd.sp_namp = fields[0];
	spwd.sp_pwdp = fields[1];

	/*
	 * Get the last changed date.  For all of the integer fields,
	 * we check for proper format.  It is an error to have an
	 * incorrectly formatted number.
	 */

	if (fields[2][0] == '\0') {
		spwd.sp_lstchg = -1;
	} else if (   (getlong (fields[2], &spwd.sp_lstchg) == 0)
	           || (spwd.sp_lstchg < 0)) {
		return 0;
	}

	/*
	 * Get the minimum period between password changes.
	 */

	if (fields[3][0] == '\0') {
		spwd.sp_min = -1;
	} else if (   (getlong (fields[3], &spwd.sp_min) == 0)
	           || (spwd.sp_min < 0)) {
		return 0;
	}

	/*
	 * Get the maximum number of days a password is valid.
	 */

	if (fields[4][0] == '\0') {
		spwd.sp_max = -1;
	} else if (   (getlong (fields[4], &spwd.sp_max) == 0)
	           || (spwd.sp_max < 0)) {
		return 0;
	}

	/*
	 * If there are only OFIELDS fields (this is a SVR3.2 /etc/shadow
	 * formatted file), initialize the other field members to -1.
	 */

	if (i == OFIELDS) {
		spwd.sp_warn   = -1;
		spwd.sp_inact  = -1;
		spwd.sp_expire = -1;
		spwd.sp_flag   = SHADOW_SP_FLAG_UNSET;

		return &spwd;
	}

	/*
	 * Get the number of days of password expiry warning.
	 */

	if (fields[5][0] == '\0') {
		spwd.sp_warn = -1;
	} else if (   (getlong (fields[5], &spwd.sp_warn) == 0)
	           || (spwd.sp_warn < 0)) {
		return 0;
	}

	/*
	 * Get the number of days of inactivity before an account is
	 * disabled.
	 */

	if (fields[6][0] == '\0') {
		spwd.sp_inact = -1;
	} else if (   (getlong (fields[6], &spwd.sp_inact) == 0)
	           || (spwd.sp_inact < 0)) {
		return 0;
	}

	/*
	 * Get the number of days after the epoch before the account is
	 * set to expire.
	 */

	if (fields[7][0] == '\0') {
		spwd.sp_expire = -1;
	} else if (   (getlong (fields[7], &spwd.sp_expire) == 0)
	           || (spwd.sp_expire < 0)) {
		return 0;
	}

	/*
	 * This field is reserved for future use.  But it isn't supposed
	 * to have anything other than a valid integer in it.
	 */

	if (fields[8][0] == '\0') {
		spwd.sp_flag = SHADOW_SP_FLAG_UNSET;
	} else if (getlong (fields[8], &spwd.sp_flag) == 0) {
		/* FIXME: add a getulong function */
		return 0;
	}

	return (&spwd);
}

