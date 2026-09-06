#!/usr/bin/env python3
"""Generates the rest_parser_fuzz corpus from examples/json/rest/.

rest_parser_fuzz multiplexes every RestApiMessageParser::parseXxx() method
into one binary by reading a selector byte off the front of each input (see
fuzz/rest_parser_fuzz.cpp). That means a corpus file for this target can't be
plain JSON - each one needs the selector byte for the parser it's meant to
exercise prepended to it.

examples/json/rest/<parserName>/<variant>.json holds the plain, valid JSON
(directly reusable for anything else - docs, mocking, OpenAPI examples). This
script reads that tree, resolves each <parserName> directory against the
kParsers[] table in fuzz/rest_parser_fuzz.cpp to find its selector byte, and
writes the prefixed files into the output corpus directory.

websocket_parser_fuzz needs no such transform - examples/json/websocket/ is
already a valid corpus directory for it as-is.
"""
import argparse
import re
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent


def parser_index_map() -> dict[str, int]:
    src = (REPO_ROOT / "fuzz" / "rest_parser_fuzz.cpp").read_text()
    names = re.findall(r"invoke<&RestApiMessageParser::(\w+)>", src)
    if not names:
        sys.exit("no parsers found in fuzz/rest_parser_fuzz.cpp - kParsers[] pattern changed?")
    return {name: i for i, name in enumerate(names)}


def main() -> None:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--out", default=str(REPO_ROOT / "fuzz" / ".generated-corpus" / "rest_parser_fuzz"))
    args = ap.parse_args()

    index_of = parser_index_map()
    src_root = REPO_ROOT / "examples" / "json" / "rest"
    out_root = Path(args.out)
    out_root.mkdir(parents=True, exist_ok=True)

    written = 0
    skipped = []
    for parser_dir in sorted(src_root.iterdir()):
        if not parser_dir.is_dir():
            continue
        parser_name = parser_dir.name
        if parser_name not in index_of:
            skipped.append(parser_name)
            continue
        selector = index_of[parser_name]
        for json_file in sorted(parser_dir.glob("*.json")):
            payload = json_file.read_bytes()
            out_file = out_root / f"{parser_name}_{json_file.stem}"
            out_file.write_bytes(bytes([selector]) + payload)
            written += 1

    if skipped:
        print(f"warning: {len(skipped)} example dirs don't match any parser in kParsers[]: {skipped}", file=sys.stderr)

    covered = {p.name for p in src_root.iterdir() if p.is_dir()} & index_of.keys()
    print(f"wrote {written} corpus files to {out_root} ({len(covered)}/{len(index_of)} parsers have an example)")


if __name__ == "__main__":
    main()
