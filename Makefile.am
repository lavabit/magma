# Use m4 macros from m4
ACLOCAL_AMFLAGS = -I m4 --install
EXTRA_DIST = m4/NOTES
AM_CFLAGS = -Wfatal-errors -Werror

# Note that we specify the include directories for the bundled dependencies
# here.
COMMON_CPPFLAGS = \
		-I$(top_srcdir)/src \
		-I$(top_srcdir)/build_lib/sources/spf2/src/include \
		-I$(top_srcdir)/build_lib/sources/clamav/libclamav \
		-I$(top_srcdir)/build_lib/sources/mysql/include \
		-I$(top_srcdir)/build_lib/sources/openssl/include \
		-I$(top_srcdir)/build_lib/sources/lzo/include \
		-I$(top_srcdir)/build_lib/sources/xml2/include \
		-I$(top_srcdir)/build_lib/sources/zlib \
		-I$(top_srcdir)/build_lib/sources/bzip2 \
		-I$(top_srcdir)/build_lib/sources/tokyocabinet \
		-I$(top_srcdir)/build_lib/sources/memcached \
		-I$(top_srcdir)/build_lib/sources/dkim/libopendkim \
		-I$(top_srcdir)/build_lib/sources/dspam/src \
		-I$(top_srcdir)/build_lib/sources/jansson/src \
		-I$(top_srcdir)/build_lib/sources/gd \
		-I$(top_srcdir)/build_lib/sources/png \
		-I$(top_srcdir)/build_lib/sources/jpeg \
		-I$(top_srcdir)/build_lib/sources/freetype/include

noinst_LIBRARIES = \
		libmagma.a

bin_PROGRAMS = magmad

magmad_LDADD = \
		libmagma.a \
		-ldl

magmad_CPPFLAGS = $(COMMON_CPPFLAGS)
magmad_SOURCES = src/magma.c

libmagma_a_CPPFLAGS = $(COMMON_CPPFLAGS)
libmagma_a_SOURCES = \
		src/core/type.c \
		src/core/indexes/cursors.c \
		src/core/indexes/hashed.c \
		src/core/indexes/inx.c \
		src/core/indexes/linked.c \
		src/core/memory/align.c \
		src/core/memory/bits.c \
		src/core/memory/memory.c \
		src/core/memory/secure.c \
		src/core/log/log.c \
		src/core/thread/keys.c \
		src/core/thread/mutex.c \
		src/core/thread/rwlock.c \
		src/core/thread/thread.c \
		src/core/hash/adler.c \
		src/core/hash/crc.c \
		src/core/hash/fletcher.c \
		src/core/hash/murmur.c \
		src/core/classify/ascii.c \
		src/core/host/files.c \
		src/core/host/folder.c \
		src/core/host/host.c \
		src/core/host/mappings.c \
		src/core/host/process.c \
		src/core/host/spool.c \
		src/core/buckets/arrays.c \
		src/core/buckets/pool.c \
		src/core/buckets/stacked.c \
		src/core/strings/allocation.c \
		src/core/strings/data.c \
		src/core/strings/info.c \
		src/core/strings/length.c \
		src/core/strings/multi.c \
		src/core/strings/nuller.c \
		src/core/strings/opts.c \
		src/core/strings/print.c \
		src/core/strings/replace.c \
		src/core/strings/shortcuts.c \
		src/core/strings/validate.c \
		src/core/compare/ends.c \
		src/core/compare/equal.c \
		src/core/compare/search.c \
		src/core/compare/starts.c \
		src/core/encodings/base64.c \
		src/core/encodings/hex.c \
		src/core/encodings/mappings.c \
		src/core/encodings/qp.c \
		src/core/encodings/url.c \
		src/core/encodings/zbase32.c \
		src/core/parsers/formats/nvp.c \
		src/core/parsers/numbers/digits.c \
		src/core/parsers/numbers/numbers.c \
		src/core/parsers/special/bracket.c \
		src/core/parsers/case.c \
		src/core/parsers/line.c \
		src/core/parsers/time.c \
		src/core/parsers/token.c \
		src/core/parsers/trim.c \
		src/network/addresses.c \
		src/network/clients.c \
		src/network/connections.c \
		src/network/listeners.c \
		src/network/options.c \
		src/network/read.c \
		src/network/reverse.c \
		src/network/write.c \
		src/providers/symbols.c \
		src/providers/checkers/clamav.c \
		src/providers/checkers/dkim.c \
		src/providers/checkers/dspam.c \
		src/providers/checkers/spf.c \
		src/providers/compress/bzip.c \
		src/providers/compress/compress.c \
		src/providers/compress/engine.c \
		src/providers/compress/lzo.c \
		src/providers/compress/zlib.c \
		src/providers/consumers/cache.c \
		src/providers/consumers/counters.c \
		src/providers/consumers/deserialization.c \
		src/providers/consumers/serialization.c \
		src/providers/cryptography/ciphers.c \
		src/providers/cryptography/cryptex.c \
		src/providers/cryptography/digest.c \
		src/providers/cryptography/ecies.c \
		src/providers/cryptography/openssl.c \
		src/providers/cryptography/random.c \
		src/providers/cryptography/scramble.c \
		src/providers/cryptography/symmetric.c \
		src/providers/database/mysql.c \
		src/providers/database/query.c \
		src/providers/database/results.c \
		src/providers/database/stmts.c \
		src/providers/database/transaction.c \
		src/providers/images/freetype.c \
		src/providers/images/gd.c \
		src/providers/images/jpeg.c \
		src/providers/images/png.c \
		src/providers/parsers/json.c \
		src/providers/parsers/xml.c \
		src/providers/storage/data.c \
		src/providers/storage/tank.c \
		src/providers/storage/tokyo.c \
		src/providers/storage/tree.c \
		src/engine/config/cache/cache.c \
		src/engine/config/global/datatier.c \
		src/engine/config/global/global.c \
		src/engine/config/relay/relay.c \
		src/engine/config/servers/servers.c \
		src/engine/context/args.c \
		src/engine/context/process.c \
		src/engine/context/sanity.c \
		src/engine/context/signal.c \
		src/engine/context/system.c \
		src/engine/context/thread.c \
		src/engine/controller/protocol.c \
		src/engine/controller/queue.c \
		src/engine/status/build.c \
		src/engine/status/performance.c \
		src/engine/status/statistics.c \
		src/engine/status/status.c \
		src/objects/config/config.c \
		src/objects/config/datatier.c \
		src/objects/contacts/contacts.c \
		src/objects/contacts/datatier.c \
		src/objects/contacts/find.c \
		src/objects/folders/contacts.c \
		src/objects/folders/datatier.c \
		src/objects/folders/find.c \
		src/objects/folders/folders.c \
		src/objects/folders/messages.c \
		src/objects/mail/cache.c \
		src/objects/mail/cleanup.c \
		src/objects/mail/counters.c \
		src/objects/mail/datatier.c \
		src/objects/mail/headers.c \
		src/objects/mail/load_message.c \
		src/objects/mail/mime.c \
		src/objects/mail/objects.c \
		src/objects/mail/parsing.c \
		src/objects/mail/paths.c \
		src/objects/mail/remove_message.c \
		src/objects/mail/signatures.c \
		src/objects/mail/store_message.c \
		src/objects/messages/datatier.c \
		src/objects/messages/messages.c \
		src/objects/messages/meta.c \
		src/objects/neue/credentials.c \
		src/objects/neue/neue.c \
		src/objects/sessions/sessions.c \
		src/objects/users/alerts.c \
		src/objects/users/aliases.c \
		src/objects/users/contacts.c \
		src/objects/users/datatier.c \
		src/objects/users/folders.c \
		src/objects/users/messages.c \
		src/objects/users/users.c \
		src/objects/warehouse/datatier.c \
		src/objects/warehouse/domains.c \
		src/objects/warehouse/patterns.c \
		src/objects/warehouse/warehouse.c \
		src/objects/locks.c \
		src/objects/objects.c \
		src/objects/serials.c \
		src/servers/http/content.c \
		src/servers/http/data.c \
		src/servers/http/errors.c \
		src/servers/http/http.c \
		src/servers/http/parse.c \
		src/servers/http/response.c \
		src/servers/http/sessions.c \
		src/servers/imap/commands.c \
		src/servers/imap/fetch.c \
		src/servers/imap/fetch_response.c \
		src/servers/imap/flags.c \
		src/servers/imap/folders.c \
		src/servers/imap/imap.c \
		src/servers/imap/messages.c \
		src/servers/imap/output.c \
		src/servers/imap/parse.c \
		src/servers/imap/parse_address.c \
		src/servers/imap/range.c \
		src/servers/imap/search.c \
		src/servers/imap/sessions.c \
		src/servers/molten/commands.c \
		src/servers/molten/molten.c \
		src/servers/molten/sessions.c \
		src/servers/pop/commands.c \
		src/servers/pop/mailbox.c \
		src/servers/pop/parse.c \
		src/servers/pop/pop.c \
		src/servers/pop/sessions.c \
		src/servers/smtp/accept.c \
		src/servers/smtp/checkers.c \
		src/servers/smtp/commands.c \
		src/servers/smtp/datatier.c \
		src/servers/smtp/messages.c \
		src/servers/smtp/parse.c \
		src/servers/smtp/relay.c \
		src/servers/smtp/session.c \
		src/servers/smtp/smtp.c \
		src/servers/smtp/transmit.c \
		src/web/contact/abuse.c \
		src/web/contact/business.c \
		src/web/contact/contact.c \
		src/web/portal/config.c \
		src/web/portal/contacts.c \
		src/web/portal/endpoint.c \
		src/web/portal/flags.c \
		src/web/portal/folders.c \
		src/web/portal/mail.c \
		src/web/portal/messages.c \
		src/web/portal/parse.c \
		src/web/portal/portal.c \
		src/web/register/abuse.c \
		src/web/register/business.c \
		src/web/register/captcha.c \
		src/web/register/datatier.c \
		src/web/register/register.c \
		src/web/register/sessions.c \
		src/web/statistics/datatier.c \
		src/web/statistics/statistics.c \
		src/web/teacher/datatier.c \
		src/web/teacher/teacher.c

TESTS = tests
check_PROGRAMS = tests
tests_CPPFLAGS = -I$(top_srcdir)/check $(COMMON_CPPFLAGS)
tests_SOURCES = \
		check/magma_check.c \
		check/core/base64_check.c \
		check/core/core_check.c \
		check/core/hashed_check.c \
		check/core/hex_check.c \
		check/core/inx_check.c \
		check/core/linked_check.c \
		check/core/qp_check.c \
		check/core/qsort_check.c \
		check/core/string_check.c \
		check/core/system_check.c \
		check/core/tree_check.c \
		check/core/url_check.c \
		check/core/zbase32_check.c \
		check/network/address_check.c \
		check/network/network_check.c \
		check/objects/objects_check.c \
		check/providers/compress_check.c \
		check/providers/digest_check.c \
		check/providers/dspam_check.c \
		check/providers/ecies_check.c \
		check/providers/provide_check.c \
		check/providers/rand_check.c \
		check/providers/scramble_check.c \
		check/providers/symmetric_check.c \
		check/providers/tank_check.c \
		check/providers/virus_check.c \
		check/users/users_check.c
tests_CFLAGS = @CHECK_CFLAGS@
tests_LDADD = libmagma.a -ldl @CHECK_LIBS@

# Targets to build before we start compiling
BUILT_SOURCES = $(top_srcdir)/build_lib/magmad.so build.h

# The idea here is that the build_lib directory may be provided through
# external means (e.g., download).
$(top_srcdir)/build_lib/magmad.so:
		rm -rf $(top_srcdir)/build_lib && \
		mkdir $(top_srcdir)/build_lib && \
		cd $(top_srcdir)/build_lib && \
		../scripts/builders/build.lib.sh all

build.h: .FORCE
		printf "#define MAGMA_BUILD \"6.0.1-$$(git log --format="%H" -n 1 | cut -c33-40)\"\n" > build.h
		date +'#define MAGMA_STAMP "%Y%m%d.%H%M"' >> build.h

.PHONY: .FORCE
