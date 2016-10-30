from __future__ import print_function, with_statement, division
import argparse

def parse_args():
    parser = argparse.ArgumentParser(description=
                                 'preprocess markdown to include code files.')
    parser.add_argument('input_file', type=argparse.FileType('r'),
                        help='markdown file.', required=True)
    parser.add_argument('path', type=str, help='Search path for include files.')
    return parser.parse_args()

def main():
    args = parse_args()
    for line in args.input_file:
        print(line, eol='')

if __name__ == '__main__':
    return main()
