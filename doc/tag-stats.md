# tag-stats

Create key or tag frequency statistics.

## Run

`odmt-tag-stats [OPTIONS] INPUT-FILE`

OPTIONS are:

* `--help, -h`: Print usage information.
* `--max-tags, -m`: count tags only on objects with no more than this many tags (default: all)
* `--min-count, -c`: tags with a count smaller than this will not be output
* `--with-values, -v`: also count values, not only keys

