Description: This document summarizes the JSON interface for the Magma email platform.

Version: *0.6.5*

Last Updated: ${updated}

Assumptions:

- User is identified with cookie a or optional sessionID in all requests.
- Responses for tables send all fields.
- No assumption is made about the order of items in a response unless specified otherwise.

Document MO:

- Some parameters with a designated set of options are listed as an ordered list.
- Optional parameters are indicated by surrounding param with square brackets.
    - e.g. ["type"]
- All parameters are required unless specified as optional.
- The default request value *(def)* is assumed by server if that parameter is not provided in the request.
- Mulptiple example requests map to response successes unless specified otherwise.
- Methods with no request params simply leave out the parameter summary and example requests.
