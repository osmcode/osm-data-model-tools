# characters

Check characters used in tag keys and member roles. Definitely good characters
are ASCII a-z, A-Z, 0-9, `_` (underscore), `-` (hyphen), and `:` (colon). They
are in very many keys and roles. Objects which only contain tag keys and roles
with those characters are ignored. Everything else will be written out to
`undecided-chars.osm.pbf` or `bad-chars.osm.pbf` if there are characters from a
list of possibly problematic characters.

## Run

`odmt-characters -o OUTPUT-DIR INPUT-FILE`

