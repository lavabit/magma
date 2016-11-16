#!/bin/bash

TLSFILE="example.com.pem"

print_server_conf() {
	printf "INSERT INTO Host_Config (application, name, value) VALUES ('magmad', 'magma.servers[$1].name', '$4');\n"
	printf "INSERT INTO Host_Config (application, name, value) VALUES ('magmad', 'magma.servers[$1].protocol', '$4');\n"
	printf "INSERT INTO Host_Config (application, name, value) VALUES ('magmad', 'magma.servers[$1].network.type', '$3');\n" | grep -v TCP
	printf "INSERT INTO Host_Config (application, name, value) VALUES ('magmad', 'magma.servers[$1].network.port', '$2');\n"
	printf "INSERT INTO Host_Config (application, name, value) VALUES ('magmad', 'magma.servers[$1].tls.certificate', '/etc/pki/tls/private/$TLSFILE');\n"
}

print_server_conf 1 25 TCP SMTP
print_server_conf 2 80 TCP HTTP
print_server_conf 3 110 TCP POP
print_server_conf 4 143 TCP IMAP
print_server_conf 5 443 TLS HTTPS
print_server_conf 6 465 TLS SUBMISSION
print_server_conf 7 587 TCP SUBMISSION
print_server_conf 8 993 TLS IMAP
print_server_conf 9 995 TLS POP
print_server_conf 10 1776 TCP MOLTEN
