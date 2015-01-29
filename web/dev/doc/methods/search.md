Sent when searching with advanced search.

### Request Params

"searchin"

- "folderID" - *will need some way to specify all*
    - integer

"queries" - *array of objects containing the following params*

- "field"
    1. "from"
    1. "to"
    1. "subject"
    1. "date" - *changes filter to date range*
    1. "size" - *changes filter to greater than, less than*
- ["filter"]
    1. "contains"
    1. "does not contain"
    1. "greater" - *for searching size*
    1. "less" - *for searching size*
- ["query"] - *sent only for filtered items*
    1. string
    1. integer - *size in bytes when searching size*
- ["range"]
    - "from"
        - unix time
    - "to"
        - unix time
