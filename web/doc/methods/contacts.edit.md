Change a contact's info. This method has several parameters that are optional since it only sends the updated fields. The response sends back all the fields that are set on the server.

### Request Params

"contactID"

- integer

["name"] - *full name of contact*

- string

["email"]

- ["primary"]
    - string
- ["alternate-1"]
    - string
- ["alternate-2"]
    - string

["chat"] - *IM/XMPP/IRC handles*

- ["yahoo"]
    - string
- ["live"]
    - string
- ["aim"]
    - string
- ["google"]
    - string
- ["icq"]
    - string

["phone"]

- ["home"]
    - string
- ["work"]
    - string
- ["mobile"]
    - string
- ["pager"]
    - string
- ["fax"]
    - string

["address"]

- ["home"]
    - string
- ["work"]
    - string

### Response Params

"contactID"

- integer

["name"] - *full name of contact*

- string

["email"]

- ["primary"]
    - string
- ["alternate-1"]
    - string
- ["alternate-2"]
    - string

["chat"] - *IM/XMPP/IRC handles*

- ["yahoo"]
    - string
- ["live"]
    - string
- ["aim"]
    - string
- ["google"]
    - string
- ["icq"]
    - string

["phone"]

- ["home"]
    - string
- ["work"]
    - string
- ["mobile"]
    - string
- ["pager"]
    - string
- ["fax"]
    - string

["address"]

- ["home"]
    - string
- ["work"]
    - string
