Loads message details to be viewed in reading screen or info accordian. Sections of the message can be requested in batches with an array.

### Request Params

"messageID"

- integer

"section" - *multiple can be requested*

- array of strings
    1. "header"
    1. "body"
    1. "attachments"
    1. "info"

### Response Params

["header"] - *header summary*

- "from"
    - string
- "subject"
    - string
- "utc" - *email client utc*
    - integer
- "to"
    - string
- ["cc"]
    - string
- ["bcc"]
    - string
- ["sender"]
    - string
- "replyto"
    - string

["body"]

- ["html"] - *if message is to displayed as html*
    - string
- ["text"] - *if message is ascii text*
    - string

["attachments"]

- "attachmentID"
    - integer
- "name"
    - string
- "bytes" - *size in bytes*
    - integer

["info"] - *extra message information*

- "source"
    - "ip"
        - string
    - "dns"
        - string
    - "reputation"
        - string
- "security"
    - "secure" - *displays red or green status indicator*
        - bool
    - "spf" - *displays red or green status indicator*
        - bool
    - "dkim" - *displays red or green status indicator*
        - bool
- "server"
    - "utc" - *server time*
        - integer
    - "images" - *displays red or green status indicator*
        - bool
    - "warnings"
        - bool
