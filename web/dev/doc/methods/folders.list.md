Lists the folders to be displayed in view menu.

### Request Params

"context" - *lets server know which folders to list*

1. "mail" - *(def)*
1. "contacts"
#1. "settings"
#1. "help"

### Response Params

"folderID" - *the ID of the listed folder*

- integer

["parentID"] - *is not provided in the response if the folder is at the root level*

- integer

"name" - *the name of the listed folder*

- string
