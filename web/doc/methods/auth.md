Sent when user submits login form. In this case, the example requests correspond to the errors respectively.

### Request Params

"username"

- string

"password"

- string

### Response Params

"auth"
- string
 1. "success"
 1. "failed"
 1. "locked"

["session"]() - *sessionID sent when successfully authenticated*

- string

["message"]() - *error message*

- string
