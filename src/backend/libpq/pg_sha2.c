/*---------------------------------------------------------------------
 *
 * pg_sha2.c
 *	  Interfaces to SHA-256 hashing.
 *
 * Portions Copyright (c) 2011 EMC Corporation All Rights Reserved
 * Portions Copyright (c) 2012-Present Pivotal Software, Inc.
 *
 *
 * IDENTIFICATION
 *	    src/backend/libpq/pg_sha2.c
 *
 *---------------------------------------------------------------------
 */
#include "c.h"

#include "common/sha2.h"

#include "libpq/pg_sha2.h"

static void
to_hex(uint8 b[PG_SHA256_DIGEST_LENGTH], char *s)
{
	static const char *hex = "0123456789abcdef";
	int			q,
				w;

	for (q = 0, w = 0; q < PG_SHA256_DIGEST_LENGTH; q++)
	{
		s[w++] = hex[(b[q] >> 4) & 0x0F];
		s[w++] = hex[b[q] & 0x0F];
	}
	s[w] = '\0';
}


bool
pg_sha256_encrypt(const char *pass, char *salt, size_t salt_len,
				  char *cryptpass)
{
	size_t passwd_len = strlen(pass);
	char *target = malloc(passwd_len + salt_len + 1);
	pg_sha256_ctx ctx;
	uint8 digest[PG_SHA256_DIGEST_LENGTH];

	memcpy(target, pass, passwd_len);
	memcpy(target + passwd_len, salt, salt_len);
	target[passwd_len + salt_len] = '\0';

	pg_sha256_init(&ctx);
	pg_sha256_update(&ctx, (uint8 *)target, passwd_len + salt_len);
	pg_sha256_final(&ctx, digest);

	strcpy(cryptpass, SHA256_PREFIX);

	to_hex((uint8 *)digest, cryptpass + strlen(SHA256_PREFIX));

	cryptpass[SHA256_PASSWD_LEN] = '\0';
	free(target);

	return true;
}
