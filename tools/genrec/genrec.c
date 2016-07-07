#include <openssl/buffer.h>
#include <openssl/pem.h>

#include <dime/common/network.h>
#include <dime/common/misc.h>
#include <dime/common/dcrypto.h>

#include <dime/signet-resolver/cache.h>
#include <dime/signet-resolver/dmtp.h>
#include <dime/signet-resolver/mrec.h>
#include <dime/signet-resolver/signet-ssl.h>

#include "symbols.h"

static void usage(const char *progname) {

	fprintf(stderr, "\nUsage: %s <-k privkey_file> [-c cert_file] [-d dx] [-p policy] [-s policy] [-e expiry] [-y syndicate] [-v version]  where\n", progname);
	fprintf(stderr, "  -k specifies the mandatory path to the ed25519 private key file in PEM format.\n");
	fprintf(stderr, "  -c specifies the optional (but suggested) path to the X509 certificate file in PEM format.\n");
	fprintf(stderr, "  -d is the (optional) hostname of the domain's DX server.\n");
	fprintf(stderr, "  -p is the (optional) domain message policy. Allowed values are experimental(default), mixed, or strict.\n");
	fprintf(stderr, "  -s is the (optional) subdomain policy. Allowed values are strict(default), relaxed, or explicit.\n");
	fprintf(stderr, "  -e is the (optional) expiration time of the domain in days.\n");
	fprintf(stderr, "  -y is the (optional) syndicates value in the DIME management record.\n");
	fprintf(stderr, "  -v is the (optional DIME record version number (defaults to %u).\n", DIME_VERSION_NO);
	fprintf(stderr, "\nNote: -d, -k, and -c can be specified as many times as desired.\n");
	fprintf(stderr, "If -c is specified, it must ALWAYS follow the -k parameter being used to sign the certificate.\n\n");

	exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {

	X509 *cert = NULL;
	ED25519_KEY *key;
	ed25519_signature sigbuf;
	FILE *certfp;
	unsigned char certhash[SHA_512_SIZE];
	char *pubb64, *certfile = NULL, *tls_hmac = NULL, *syndicates = NULL, *dx = NULL, *expiry = NULL, *pubkey = NULL, dimebuf[4096];
	dime_msg_policy msg_policy = msg_experimental;
	dime_sub_policy sub_policy = sub_strict;
	int opt, maxprint;
	unsigned int version = DIME_VERSION_NO;
	size_t i, dlen;

	// Load the OpenSSL symbols used by libdime.
	if (lib_load()) {
		fprintf(stderr, "Error: unable to bind the program to the required dynamic symbols.\n");
		exit(EXIT_FAILURE);
	}

	crypto_init();

	while ((opt = getopt(argc, argv, "c:d:e:k:p:s:v:y:")) != -1) {

		switch (opt) {
		case 'k':

			if (!(key = load_ed25519_privkey(optarg))) {
				_clear_error_stack();

				//if (!(key = keys_file_fetch_sign_key(optarg))) {
					fprintf(stderr, "Error: could not read ed25519 POK from keyfile.\n");
					exit(EXIT_FAILURE);
				//}

			}

			// Derive public key from private key.
			if (!(pubb64 = b64encode_nopad(key->public_key, ED25519_KEY_SIZE))) {
				fprintf(stderr, "Error: Unable to base64 encode public key data.\n");
				dump_error_stack();
				exit(EXIT_FAILURE);
			}

			if (str_printf(&pubkey, "pok=%s", pubb64) <= 0) {
				fprintf(stderr, "Error: unable to add DX server to record buffer.\n");
				dump_error_stack();
				exit(EXIT_FAILURE);
			}

			free(pubb64);
			break;
		case 'c':
			certfile = optarg;
			break;
		case 'd':

			if (str_printf(&dx, " dx=%s", optarg) <= 0) {
				fprintf(stderr, "Error: unable to add DX server to record buffer.\n");
				dump_error_stack();
				exit(EXIT_FAILURE);
			}

			break;
		case 'e':

			if (!atoi(expiry = optarg)) {
				fprintf(stderr, "Error: invalid DIME record expiration value was specified.\n");
				exit(EXIT_FAILURE);
			}

			break;
		case 'p':

			if (!strcasecmp(optarg, "experimental")) {
				msg_policy = msg_experimental;
			} else if (!strcasecmp(optarg, "mixed")) {
				msg_policy = msg_mixed;
			} else if (!strcasecmp(optarg, "strict")) {
				msg_policy = msg_strict;
			} else {
				fprintf(stderr, "Error: invalid DIME message policy type was specified. Must be experimental, mixed, or strict.\n");
				exit(EXIT_FAILURE);
			}

			break;

		case 's':

			if (!strcasecmp(optarg, "strict")) {
				sub_policy = sub_strict;
			} else if (!strcasecmp(optarg, "relaxed")) {
				sub_policy = sub_relaxed;
			} else if (!strcasecmp(optarg, "explicit")) {
				sub_policy = sub_explicit;
			} else {
				fprintf(stderr, "Error: invalid DIME subdomain policy type was specified. Must be strict, relaxed, or explicit.\n");
				exit(EXIT_FAILURE);
			}

			break;
		case 'y':
			syndicates = optarg;
			break;
		case 'v':

			if (!(version = atoi(optarg))) {
				fprintf(stderr, "Error: invalid DIME version number was specified.\n");
				exit(EXIT_FAILURE);
			}

			break;
		default:
			usage(argv[0]);
			break;
		}

	}

	if (!pubkey) {
		usage(argv[0]);
	}

	if (certfile) {

		if (!(certfp = fopen(certfile, "r"))) {
			perror("fopen");
			exit(EXIT_FAILURE);
		}

		if (!(cert = PEM_read_X509(certfp, NULL, NULL, NULL))) {
			ERR_print_errors_fp(stderr);
			exit(EXIT_FAILURE);
		}

		memset(certhash, 0, sizeof(certhash));

		// QUESTION: Do we hash the entire cert as-is iN DER format, or do we hash only the TBS portion (TBSCertificate) - excluding signatureAlgorithm + signatureValue, etc.?
		if (get_x509_cert_sha_hash(cert, 512, certhash) < 0) {
			fprintf(stderr, "Error: unable to compute SHA-512 hash of X509 certificate.\n");
			dump_error_stack();
			exit(EXIT_FAILURE);
		}


		// TODO: Looks like the prior SHA512 hash might be redundant?
		ed25519_sign_data(certhash, sizeof(certhash), key, sigbuf);
		free_ed25519_key(key);

		if (!(tls_hmac = b64encode_nopad(sigbuf, ED25519_SIG_SIZE))) {
			fprintf(stderr, "Error: unable to base64 encode TLS certificate signature.\n");
			dump_error_stack();
			exit(EXIT_FAILURE);
		}

	}

	memset(dimebuf, 0, sizeof(dimebuf));
	snprintf(dimebuf, sizeof(dimebuf), "ver=%u %s", version, pubkey);

	if (tls_hmac) {
		BUF_strlcat(dimebuf, " tls=", sizeof(dimebuf));
		BUF_strlcat(dimebuf, tls_hmac, sizeof(dimebuf));
	}

	BUF_strlcat(dimebuf, " pol=", sizeof(dimebuf));

	switch(msg_policy) {
	case msg_experimental:
		BUF_strlcat(dimebuf, "experimental", sizeof(dimebuf));
		break;
	case msg_mixed:
		BUF_strlcat(dimebuf, "mixed", sizeof(dimebuf));
		break;
	case msg_strict:
		BUF_strlcat(dimebuf, "strict", sizeof(dimebuf));
		break;
	}

	if (syndicates) {
		BUF_strlcat(dimebuf, "syn=", sizeof(dimebuf));
		BUF_strlcat(dimebuf, syndicates, sizeof(dimebuf));
	}

	if (dx) {
		BUF_strlcat(dimebuf, dx, sizeof(dimebuf));
	}

	if (expiry) {
		BUF_strlcat(dimebuf, " exp=", sizeof(dimebuf));
		BUF_strlcat(dimebuf, expiry, sizeof(dimebuf));
	}

	BUF_strlcat(dimebuf, " sub=", sizeof(dimebuf));

	switch(sub_policy) {
	case sub_strict:
		BUF_strlcat(dimebuf, "strict", sizeof(dimebuf));
		break;
	case sub_relaxed:
		BUF_strlcat(dimebuf, "relaxed", sizeof(dimebuf));
		break;
	case sub_explicit:
		BUF_strlcat(dimebuf, "explicit", sizeof(dimebuf));
		break;
	}

	// Now we need to break this up into suitably sized chunks (TXT records longer than 255 characters must consist of
	// multiple TXT records that are concatenated together.
	if (strlen(dimebuf) <= 255) {
		printf("%s\n", dimebuf);
	} else {
		i = 0;
		dlen = strlen(dimebuf);

		while (dlen) {
			maxprint = (dlen > 254) ? 254 : dlen;
			printf("\"%.*s\" ", maxprint, &(dimebuf[i]));
			i += maxprint;
			dlen -= maxprint;
		}

		printf("\n");
	}

	return 0;
}
