Load the contact info for a particular contact.

### Request Params

"contactID"

- integer

"folderID"

- integer

### Response Params

"name"

- string

"email"

- "primary"
    - string
- ["alternate1"]
    - string
- ["alternate2"]
    - string

["chat"]

- ["yahoo"]
    - string
- ["live"]
    - string
- ["aim"]
    - string
- ["google"]
    - string

["contact"]

- ["home"]
    - string
- ["work"]
    - string
- ["mobile"]
    - string
- ["address"] - *include <br /> for new lines*
    - strings
