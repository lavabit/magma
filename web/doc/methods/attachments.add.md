Sent by a user that wants to attach a file to a composition. Note that a separate non-ajax upload must be made to actually affix the attachment data (%CAMELHOST%/portal/camel/attach/[compid]/[attachid]) where [compid] is the message composition ID and attachid is the attachment ID to which the file about to be uploaded will be attached.

### Request Params

"composeID" - *a composition ID provided by server when the user starts composing a new message*

- integer

"filename" - *the name of the filename to be attached to the composition*

- string

### Response Params

"attachmentID" - *an attachment ID that will later be passed to the messages.send method*

- integer
