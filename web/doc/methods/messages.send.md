Send a composed message and all of its associated attachments.

### Request Params

"composeID" - *the ID of the composition, as returned by the messages.compose method*

- integer

"from" - *the email address of the sender*

- string

["to"] - *an array of at least one or more email recipients*

- array of strings

["cc"] - *an array of CC: recipients, which may optionally be left empty*

- array of strings

["bcc"] - *an array of BCC: recipients, which may optionally be left empty*

- array of strings

["subject"] - *the email subject line; not sent if empty - user warned before sent*

- string

["priority"] - 

1. "low"
1. "normal" - *(def)*
1. "high"

["body"] - *the message body; not sent if empty - user warned before sent (at least html OR text must be specified)*

1. "text" - *the plain text version of the email message body*
    - string
1. "html" - *the html version of the email message body*
    - string
