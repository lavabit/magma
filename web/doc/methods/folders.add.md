Adds a folder with given name and returns folderID.

### Request Params

"context" - *the type of the new folder to be created; can be either "mail" or "contacts"*

- string

"name" - *name of the new folder*

- string

["parentID"] - *given if added folder is a child of a parent folder other than the root*

- integer


### Response Params

"folderID" - *global folderID assigned by server*

- integer
