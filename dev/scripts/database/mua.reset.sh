#!/bin/bash

# Name: mua.reset.sh
# Author: Ladar Levison
#
# Description: Quickly wipe out the IMAP cache for Thunderbird, Kmail, and Evolution, making it easier to test magma.

# Thunderbird
rm --recursive --force $HOME/.thunderbird/*.default/ImapMail/*

# Kmail
rm --recursive --force $HOME/.kde/share/apps/kmail/imap/* $HOME/.kde/share/apps/kmail/dimap/*

#Evolution
rm --recursive --force $HOME/.local/share/evolution/mail/imap/*

