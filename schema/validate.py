#! /usr/bin/env python3

import sys
import argparse
from lxml import etree

__author__ = 'Thom Hurks'


def parse_args():
    parser = argparse.ArgumentParser(prog='pgMark', description='Check if a given pgMark graph schema is valid')
    parser.add_argument('schema', metavar='schema_file', type=open, help='The graph schema XML file')
    return parser.parse_args()


def parse_input_schema(filename):
    relaxng = etree.RelaxNG(etree.parse('schema.rng'))
    parser = etree.XMLParser(remove_blank_text=True)
    document = etree.parse(filename, parser)
    try:
        relaxng.assertValid(document)
        print('Your graph schema appears to be valid.')
    except etree.DocumentInvalid as err:
        sys.exit('Input graph schema invalid: {}'.format(err))


def main():
    args = parse_args()
    parse_input_schema(args.schema)


if __name__ == "__main__":
    main()
