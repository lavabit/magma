Gets advertisement and fun facts for loading screen and ad banner for messages.

### Request Params

["context"] - *where the ad will be displayed*

1. "loading"
1. "messages" *(def)*

### Response Params

"ad"

- "href" - *href of advertisement*
    - string
- ["title"] - *link title*
    - string
- ["img"] - *if image ad*
    - "src"
        - string
    - "alt"
        - string

- ["text"] - *if text ad*
    - string

["fact"]

- "img"
    - "src"
        - string
    - "alt"
        - string
- "text"
    - array of strings
