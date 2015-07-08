# Migration Issues

## Migrating legacy accounts

There are around 410,000 legacy Lavabit accounts that should be migrated
as part of the Lavabit reboot. Of these account, about 10,000 are paid, and
hence potentially have encrypted email still in the account's mailbox. The remaining 
ones are free accounts, and therefore havei only plaintext email.

## Indicating legacy logins

Question one is, how do we recognize a legacy login account. There are
three possibilities:
* presence of a legacy password
* absence of a stacie token
* absence of a stacie salt

The best choice to use is the absence of a stacie token. All new account
provisions will, as part of the initial account creation, obtain a user password
and create the iaccount's stacie token. Every account with a non-NULL stacie token will
be an account that is new or that has been *stacified* (set up to use stacie authentication).

The absence of a stacie salt could ibe used to indicate a legacy account, but it is less 
obvious and straightforward than recognizing the NULL stacie token. It is possible to 
populate the stacie salt field with random values during initial database 
loading, in part to reduce the load on system entropy required to generate random
salts, but this is a minor concern. However, changing a stacie password
might use the existing stacie salt to increase entropy in the newly generated stacie 
salt by XORing the old stacie salt with a newly generated random value and using the
result as the new stacie salt. If we choose to add this extra step, changing from a 
legacy password to the initial staciepassword would more closely resemble a vanilla 
stacie password change with the stacie salt present. Of course, the disadvantage is
the necessity of generating the stacie salt during initial population of the
new database. This slight disadvantage might be outweighed by the entropy gain, but
it clearly complicates migration.

Finally, the presence of a non-NULL legacy password is a good candidate for indicating 
the presence of a legacy account. However, at some point, we might wish to 
remove the legacy password column from the database. Removing as much dependency in
the code as possible tips the choice back to the stacie token's value as the 
legacy account flag.

## Migrating Encrypted mail

To prevent the necessity of reencrypting the contents of a stacie account's 
existing mailbox contents during a password change, we'll keep fixed encryption 
keys for mail messages and simply XOR them with a realm key before storing them in the database
during a stacie password change.
This means the same mailbox key is used to encrypt the mailbox contents before and after a
stacie password change, but the encryption
of that key in the database changes during a stacie password change. Legacy paid mailboxes will have
existing mailbox keys retained during *staciefication* of the account.

## Migrating unencrypted mail

Existing free uses have plaintext messages in their account mailboxes. The new Lavabit trustful server
encrypts all email in mailboxes. The choice here is between:
* Choosing keys and encrypting mail during initial database population.
* Transferring plaintext mail to the mailbox then generating keys and encrypting
the mailbox as part of *staciefication* when the user first logs in.

Encrypting the email for initial database population has the advantage that the new Lavabit
server never has plaintext mail in mailboxes, and the *staciefication* password change
is more similar to a standard stacie password change. Choosing mail keys during migration
also reduces the entropy needed during *staciefication* of the account, but again, this is
a minor concern.

I can't think of any advantage for leaving plaintext mail in the mailbox prior to *stacification*
beyond simplification of the initial database load. This is, however, counterbalanced by increasing
the complexity of *staciefication* as the entire contents of the mailbox must be 
encrypted immediately after generating new mailbox encryption keys. This brings up a further point,
addressed below.

## Forcing a change of mailbox encryption keys

While ordinarily we do not reencrypt the entire contents of a mailbox during a password change,
it might be prudent to offer a mechanism to force such a reset of mailbox keys and reencryption
of messages on the server. This mechanism does not necessarily need to be present at launch of 
the new Lavabot service, but I'd think might be a useful addition early on. While it might be difficult
to imagine circumstances in which the encrypted mailbox keys could be compromised, having
a way to request a key change couldn't hurt and might just help.

## Other issues

We should probably set some time after which legacy accounts will be purged from the system, if only to put
a bit of urgency in the minds of existing customers. Good housekeeping also suggests removing old, unclaimed
accounts at some point, and the released mailbox names could be made available again if the original
customer does not claim her account.

I suggest we work to make all *stacification* mechanisms resemble non-legacy mechanisms as closely
as possible (e.g., the initial *staciefication* password change should resemble a normal password
change). This will reduce the amount of customer visible, operational code complexity at a slight
cost in initial database population complexity. The latter is completely under our control, and can
be redone if we discover (prior to launch) any problematic aspects.

Finally, it might be nice to have an invited beta period at launch where legacy customers are invited back
in small batches to help wring out any flaws in migration or operation of the new Lavabit. The size of 
invited legacy customers could be increased with each iteration until all 410,000 accounts have been turned on.
If we have no contact other than the lavabit email address, the web interface could handle who gets in and
when it happens. 

It might also be nice to take reservations for new accounts until we are sure of system stability and
operational aspects, including scaling. When we are confident that all is working well, we can open the
floodgates for immediate signups.

All of the customer interaction and timing for account activation should be run through some marketing
analysis to choose the optimal plan for rolling out the new service, consistent with publicity, scaling, and
system stability. As the saying goes, the best surprise is no surprise!

