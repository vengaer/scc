#!/usr/bin/env python

import argparse
import contextlib
import json
import os
import sys
import tempfile

import pycparser

from pycparser.c_ast import (
    ArrayDecl,
    Decl,
    Enum,
    IdentifierType,
    Struct,
    Typedef,
    TypeDecl,
    Union,
)

_verbose = False  # pylint: disable=invalid-name


def generate_typedefs(filename, tmppath, ent):
    to_generate = [t for t in ent["types"] if ent["types"][t]]

    with open(tmppath, "w", encoding="ascii") as outfile:
        with open(filename, "r", encoding="ascii") as infile:
            outfile.write(infile.read())

        for ctype in to_generate:
            outfile.write(
                f'typedef {ent["types"][ctype]["from"]}({", ".join(ent["types"][ctype]["params"])}) {ctype};\n'  # pylint: disable=line-too-long
            )


def parse_ast(filename, include_dirs):
    ast = pycparser.parse_file(
        filename,
        use_cpp=True,
        cpp_path=os.environ.get("CC", "cpp"),
        cpp_args=["-E"] + [f"-I{dir}" for dir in include_dirs],
    )

    return ast


def declname(field):
    with contextlib.suppress(AttributeError):
        return field.declname
    with contextlib.suppress(AttributeError):
        return field.type.declname
    return field.type.type.declname

def chk_field(left, right, field):
    left = left.type
    right = right.type
    if field == "type":
        if isinstance(left.type, IdentifierType):
            assert (
                left.type.names == right.type.names
            ), f"Field mismatch in identifier {left.type}, {right.type}"
        elif type(left.type) in (Enum, Struct, Union):
            assert (
                left.type.name == right.type.name
            ), f"Field mismatch in {type(left.type)} {left.type}, {right.type}"
        elif isinstance(left.type, TypeDecl):
            assert (
                left.type.type.name == right.type.type.name
            ), f"Field mismatch in {left.type} {right.type}"
        else:
            raise RuntimeError(f"Unsupported type {left.type}")
    else:
        assert (
            getattr(left, field, None) == getattr(right, field, None)
        ), f"{field} mismatch in {left}, {right}"


def parse_typedefs(names, ast):
    typedefs = []
    for child in ast.children():
        if not isinstance(child[1], Typedef):
            continue
        if not child[1].name in names:
            continue
        if not isinstance(child[1].type, TypeDecl):
            continue
        if type(child[1].type.type) not in (Struct, Union):
            continue
        typedefs.append(child[1].type.type)

    return typedefs


def parse_structs(names, ast):
    structs = []
    for child in ast.children():
        if not isinstance(child[1], Decl):
            continue
        if not isinstance(child[1].type, Struct):
            continue
        if not child[1].type.name in names:
            continue
        structs.append(child[1].type)
    return structs


def check_compatibility(names, ast, expect_typedefs):
    if _verbose:
        print(f"Checking {names}")
    typedefs = parse_typedefs(names, ast)
    if expect_typedefs:
        assert typedefs, 'Found no typedefs'
    candidates = typedefs + parse_structs(names, ast)

    for i, type0 in enumerate(candidates):
        for type1 in candidates[i + 1:]:
            for left, right in zip(type0.decls, type1.decls):
                if tuple(
                    (
                        f
                        for f in (left.type, right.type)
                        if isinstance(f, ArrayDecl) and not f.dim_quals
                    )
                ):
                    if _verbose:
                        print(
                            f"Skipping FAM comparison of {declname(left.type)} and {declname(right.type)}"  # pylint: disable=line-too-long
                        )
                    continue

                for field in "quals", "align", "type":
                    chk_field(left, right, field)


def main(filename, include_dir, verbose, config):
    global _verbose  # pylint: disable=invalid-name,global-statement
    _verbose = verbose

    with open(config, "r", encoding='ascii') as handle:
        cfg = json.load(handle)

    try:
        ent = cfg[os.path.basename(filename)]
    except KeyError:
        sys.exit(0)

    fdesc, tmppath = tempfile.mkstemp()
    try:
        os.close(fdesc)
        generate_typedefs(filename, tmppath, ent)

        ast = parse_ast(
            tmppath,
            [os.path.dirname(filename)] + ([include_dir]
            if include_dir is not None
            else []),
        )
        for chkset in ent["check"]:
            check_compatibility(chkset, ast, ent['types'])
    finally:
        os.remove(tmppath)


if __name__ == "__main__":
    parser = argparse.ArgumentParser("Detect struct memory compatibility issues")
    parser.add_argument("filename", metavar="FILE", help="Path to file to validate")
    parser.add_argument(
        "-I",
        "--include-dir",
        default=None,
        help="Path to pycparser fake system headers",
    )
    parser.add_argument(
        "-v", "--verbose", action="store_true", help="Enable verbose output"
    )
    parser.add_argument(
        "-c", "--config", default=".mcompat.json", help="Path to config file"
    )

    main(**vars(parser.parse_args()))
