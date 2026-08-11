# Email Authentication Update

## Supported email addresses

The authentication module no longer restricts users to Gmail. It accepts normal Internet email addresses such as:

- `user@gmail.com`
- `user@outlook.com`
- `user@yahoo.net`
- `user@company.org`
- `user@company.co.in`
- `user@company.net`

## Validation

The validator checks:

- one `@` character
- local-part length <= 64
- total email length <= 254
- valid local-part characters
- no leading/trailing/consecutive dots in the local part
- domain labels containing letters, digits and hyphens
- no leading/trailing hyphens in domain labels
- at least one domain dot
- alphabetic TLD of 2-63 characters

## Uniqueness

Email addresses are normalized to lowercase before signup/login. Therefore:

`User@Example.COM`

and

`user@example.com`

are treated as the same account.

## Password security

The existing salted password hashing remains unchanged:

`email : salt_hex : sha256(salt + password)`

Passwords and password hashes are never written to the application log.
