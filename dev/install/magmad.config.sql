
INSERT INTO Host_Config (application, name, `value`, timestamp) VALUES ('magmad', 'magma.system.domain', '$DOMAIN', NOW());
INSERT INTO Host_Config (application, name, `value`, timestamp) VALUES ('magmad', 'magma.admin.abuse', 'abuse@$DOMAIN', NOW());
INSERT INTO Host_Config (application, name, `value`, timestamp) VALUES ('magmad', 'magma.admin.contact', 'admin@$DOMAIN', NOW());

INSERT INTO Host_Config (application, name, `value`, timestamp) VALUES ('magmad', 'magma.storage.default', 'local', NOW());
INSERT INTO Host_Config (application, name, `value`, timestamp) VALUES ('magmad', 'magma.storage.root', '/var/lib/magma/storage/', NOW());
INSERT INTO Host_Config (application, name, `value`, timestamp) VALUES ('magmad', 'magma.storage.tank', '/var/lib/magma/storage/tanks/', NOW());

INSERT INTO Host_Config (application, name, `value`, timestamp) VALUES ('magmad', 'magma.http.fonts', '/var/lib/magma/resources/fonts/', NOW());
INSERT INTO Host_Config (application, name, `value`, timestamp) VALUES ('magmad', 'magma.http.pages', '/var/lib/magma/resources/pages/', NOW());
INSERT INTO Host_Config (application, name, `value`, timestamp) VALUES ('magmad', 'magma.http.templates', '/var/lib/magma/resources/templates/', NOW());

INSERT INTO Host_Config (application, name, `value`, timestamp) VALUES ('magmad', 'magma.spool', '/var/spool/magma/', NOW());
INSERT INTO Host_Config (application, name, `value`, timestamp) VALUES ('magmad', 'magma.output.path', '/var/log/magma/', NOW());
INSERT INTO Host_Config (application, name, `value`, timestamp) VALUES ('magmad', 'magma.iface.virus.signatures', '/var/lib/clamav/', NOW());

INSERT INTO Host_Config (application, name, `value`, timestamp) VALUES ('magmad', 'magma.secure.salt', '$PSALT', NOW());
INSERT INTO Host_Config (application, name, `value`, timestamp) VALUES ('magmad', 'magma.secure.sessions', '$PSESS', NOW());
INSERT INTO Host_Config (application, name, `value`, timestamp) VALUES ('magmad', 'magma.iface.cryptography.seed_length', '1024', NOW());
INSERT INTO Host_Config (application, name, `value`, timestamp) VALUES ('magmad', 'magma.iface.cryptography.dhparams_rotate', 'true', NOW());
INSERT INTO Host_Config (application, name, `value`, timestamp) VALUES ('magmad', 'magma.iface.cryptography.dhparams_large_keys', 'true', NOW());
INSERT INTO Host_Config (application, name, `value`, timestamp) VALUES ('magmad', 'magma.system.daemonize', 'true', NOW());

INSERT INTO Host_Config (application, name, `value`, timestamp) VALUES ('magmad', 'magma.dkim.enabled', 'true', NOW());
INSERT INTO Host_Config (application, name, `value`, timestamp) VALUES ('magmad', 'magma.dkim.selector', '$SELECTOR', NOW());
INSERT INTO Host_Config (application, name, `value`, timestamp) VALUES ('magmad', 'magma.dkim.domain', '$DOMAIN', NOW());
INSERT INTO Host_Config (application, name, `value`, timestamp) VALUES ('magmad', 'magma.dkim.key', '/etc/pki/dkim/private/$DKIMFILE', NOW());
INSERT INTO Host_Config (application, name, `value`, timestamp) VALUES ('magmad', 'magma.dime.key', '/etc/pki/dime/private/$DIMEFILE', NOW());
INSERT INTO Host_Config (application, name, `value`, timestamp) VALUES ('magmad', 'magma.dime.signet', '/etc/pki/dime/private/$DIMEFILE', NOW());

INSERT INTO Host_Config (application, name, `value`, timestamp) VALUES ('magmad', 'magma.servers[1].name', 'SMTP', NOW());
INSERT INTO Host_Config (application, name, `value`, timestamp) VALUES ('magmad', 'magma.servers[1].protocol', 'SMTP', NOW());
INSERT INTO Host_Config (application, name, `value`, timestamp) VALUES ('magmad', 'magma.servers[1].network.port', '25', NOW());
INSERT INTO Host_Config (application, name, `value`, timestamp) VALUES ('magmad', 'magma.servers[1].tls.certificate', '/etc/pki/tls/private/$TLSFILE', NOW());

INSERT INTO Host_Config (application, name, `value`, timestamp) VALUES ('magmad', 'magma.servers[2].name', 'DMTP', NOW());
INSERT INTO Host_Config (application, name, `value`, timestamp) VALUES ('magmad', 'magma.servers[2].protocol', 'DMTP', NOW());
INSERT INTO Host_Config (application, name, `value`, timestamp) VALUES ('magmad', 'magma.servers[2].network.port', '26', NOW());
INSERT INTO Host_Config (application, name, `value`, timestamp) VALUES ('magmad', 'magma.servers[2].tls.certificate', '/etc/pki/tls/private/$TLSFILE', NOW());

INSERT INTO Host_Config (application, name, `value`, timestamp) VALUES ('magmad', 'magma.servers[3].name', 'HTTP', NOW());
INSERT INTO Host_Config (application, name, `value`, timestamp) VALUES ('magmad', 'magma.servers[3].protocol', 'HTTP', NOW());
INSERT INTO Host_Config (application, name, `value`, timestamp) VALUES ('magmad', 'magma.servers[3].network.port', '80', NOW());
INSERT INTO Host_Config (application, name, `value`, timestamp) VALUES ('magmad', 'magma.servers[3].tls.certificate', '/etc/pki/tls/private/$TLSFILE', NOW());
INSERT INTO Host_Config (application, name, `value`, timestamp) VALUES ('magmad', 'magma.servers[3].enabled', 'false', NOW());

INSERT INTO Host_Config (application, name, `value`, timestamp) VALUES ('magmad', 'magma.servers[4].name', 'POP', NOW());
INSERT INTO Host_Config (application, name, `value`, timestamp) VALUES ('magmad', 'magma.servers[4].protocol', 'POP', NOW());
INSERT INTO Host_Config (application, name, `value`, timestamp) VALUES ('magmad', 'magma.servers[4].network.port', '110', NOW());
INSERT INTO Host_Config (application, name, `value`, timestamp) VALUES ('magmad', 'magma.servers[4].tls.certificate', '/etc/pki/tls/private/$TLSFILE', NOW());

INSERT INTO Host_Config (application, name, `value`, timestamp) VALUES ('magmad', 'magma.servers[5].name', 'IMAP', NOW());
INSERT INTO Host_Config (application, name, `value`, timestamp) VALUES ('magmad', 'magma.servers[5].protocol', 'IMAP', NOW());
INSERT INTO Host_Config (application, name, `value`, timestamp) VALUES ('magmad', 'magma.servers[5].network.port', '143', NOW());
INSERT INTO Host_Config (application, name, `value`, timestamp) VALUES ('magmad', 'magma.servers[5].tls.certificate', '/etc/pki/tls/private/$TLSFILE', NOW());

INSERT INTO Host_Config (application, name, `value`, timestamp) VALUES ('magmad', 'magma.servers[6].name', 'HTTP', NOW());
INSERT INTO Host_Config (application, name, `value`, timestamp) VALUES ('magmad', 'magma.servers[6].protocol', 'HTTP', NOW());
INSERT INTO Host_Config (application, name, `value`, timestamp) VALUES ('magmad', 'magma.servers[6].network.type', 'TLS', NOW());
INSERT INTO Host_Config (application, name, `value`, timestamp) VALUES ('magmad', 'magma.servers[6].network.port', '443', NOW());
INSERT INTO Host_Config (application, name, `value`, timestamp) VALUES ('magmad', 'magma.servers[6].tls.certificate', '/etc/pki/tls/private/$TLSFILE', NOW());
INSERT INTO Host_Config (application, name, `value`, timestamp) VALUES ('magmad', 'magma.servers[6].enabled', 'false', NOW());

INSERT INTO Host_Config (application, name, `value`, timestamp) VALUES ('magmad', 'magma.servers[7].name', 'SMTPS', NOW());
INSERT INTO Host_Config (application, name, `value`, timestamp) VALUES ('magmad', 'magma.servers[7].protocol', 'SUBMISSION', NOW());
INSERT INTO Host_Config (application, name, `value`, timestamp) VALUES ('magmad', 'magma.servers[7].network.type', 'TLS', NOW());
INSERT INTO Host_Config (application, name, `value`, timestamp) VALUES ('magmad', 'magma.servers[7].network.port', '465', NOW());
INSERT INTO Host_Config (application, name, `value`, timestamp) VALUES ('magmad', 'magma.servers[7].tls.certificate', '/etc/pki/tls/private/$TLSFILE', NOW());

INSERT INTO Host_Config (application, name, `value`, timestamp) VALUES ('magmad', 'magma.servers[8].name', 'SUBMISSION', NOW());
INSERT INTO Host_Config (application, name, `value`, timestamp) VALUES ('magmad', 'magma.servers[8].protocol', 'SUBMISSION', NOW());
INSERT INTO Host_Config (application, name, `value`, timestamp) VALUES ('magmad', 'magma.servers[8].network.port', '587', NOW());
INSERT INTO Host_Config (application, name, `value`, timestamp) VALUES ('magmad', 'magma.servers[8].tls.certificate', '/etc/pki/tls/private/$TLSFILE', NOW());

INSERT INTO Host_Config (application, name, `value`, timestamp) VALUES ('magmad', 'magma.servers[9].name', 'IMAP', NOW());
INSERT INTO Host_Config (application, name, `value`, timestamp) VALUES ('magmad', 'magma.servers[9].protocol', 'IMAP', NOW());
INSERT INTO Host_Config (application, name, `value`, timestamp) VALUES ('magmad', 'magma.servers[9].network.type', 'TLS', NOW());
INSERT INTO Host_Config (application, name, `value`, timestamp) VALUES ('magmad', 'magma.servers[9].network.port', '993', NOW());
INSERT INTO Host_Config (application, name, `value`, timestamp) VALUES ('magmad', 'magma.servers[9].tls.certificate', '/etc/pki/tls/private/$TLSFILE', NOW());

INSERT INTO Host_Config (application, name, `value`, timestamp) VALUES ('magmad', 'magma.servers[10].name', 'POP', NOW());
INSERT INTO Host_Config (application, name, `value`, timestamp) VALUES ('magmad', 'magma.servers[10].protocol', 'POP', NOW());
INSERT INTO Host_Config (application, name, `value`, timestamp) VALUES ('magmad', 'magma.servers[10].network.type', 'TLS', NOW());
INSERT INTO Host_Config (application, name, `value`, timestamp) VALUES ('magmad', 'magma.servers[10].network.port', '995', NOW());
INSERT INTO Host_Config (application, name, `value`, timestamp) VALUES ('magmad', 'magma.servers[10].tls.certificate', '/etc/pki/tls/private/$TLSFILE', NOW());

INSERT INTO Host_Config (application, name, `value`, timestamp) VALUES ('magmad', 'magma.servers[11].name', 'MOLTEN', NOW());
INSERT INTO Host_Config (application, name, `value`, timestamp) VALUES ('magmad', 'magma.servers[11].protocol', 'MOLTEN', NOW());
INSERT INTO Host_Config (application, name, `value`, timestamp) VALUES ('magmad', 'magma.servers[11].network.port', '1776', NOW());


-- INSERT INTO Host_Config (application, name, `value`, timestamp) VALUES ('magmad', 'magma.system.impersonate_user', 'magma', NOW());
-- INSERT INTO Host_Config (application, name, `value`, timestamp) VALUES ('magmad', 'magma.system.root_directory', '/var/lib/magma/', NOW());

