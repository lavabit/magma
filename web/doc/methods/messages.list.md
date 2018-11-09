List messages of the given folder.

### Request Params

"folderID"

- integer

["start"] - *given to skip the specified number of messages in the folder, before constructing the response*

- integer

["limit"] - *stop adding messages to the response, when it reaches this number*

- integer

### Response Params

"messageID"

- integer

["attachment"]

- boolean

["flags"]

- array of strings

"from"

- string

"to"

- string

"addressedTo"

- string

"replyTo"

- string

"returnPath"

- string

["carbon"]

- string

"subject"

- string

"utc" - *server utc time in seconds (unix time)*

- integer

"arrivalUtc"

- integer

"snippet"

- string

"bytes" - *size in bytes*

- integer

["tags"]

- array of strings
