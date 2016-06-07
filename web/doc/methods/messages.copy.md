Copy messages from one folder to another or the same folder.

### Request Params

"messageIDs"

- array of ints

"sourceFolderID"

- int

"targetFolderID"

- int

### Response Params

Each object in the response array is associated with a messageID in the request.

"sourceMessageID"

- integer

"targetMessageID"

- integer
