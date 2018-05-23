#!/usr/bin/env python3

import argparse
import sys
from new_command import NewCommand
from build_command import BuildCommand
from run_command import RunCommand
from assist_command import AssistCommand
from version import Version

if __name__ == '__main__':
    parser = argparse.ArgumentParser(prog='biodynamo',
        description='This is the BioDynaMo command line interface. It guides '
                    'you during the whole simulation workflow. From starting '
                    'a new project, to compiling and executing, all the way '
                    'to requesting assistance from the BioDynaMo developers.',
        epilog='')

    sp = parser.add_subparsers(dest='cmd')
    parser.add_argument('-v', '--version',
                        action='store_true',
                        help='Display BioDynaMo version')

    assist_sp = sp.add_parser('assist', help='Use this command if you need help from the '
                                 'BiodynaMo developers. This command helps you '
                                 'to gather information which is required to '
                                 'reproduce and debug your issue. First, '
                                 'it will ask you to commit all uncommited changes. '
                                 'Afterwards, it will attempt to build and run your '
                                 'simulation. During this process it will collect '
                                 'logs which will be commited and uploaded in a '
                                 'seperate git branch. In the end it will output '
                                 'a link that you should add to your e-mail or '
                                 'slack message when you describe your issue.' )

    build_sp = sp.add_parser('build', help='Builds the simulation binary')

    clean_sp = sp.add_parser('clean', help='Removes all build files')

    new_sp = sp.add_parser('new', help='Creates a new simulation project. Downloads '
    'a template project from BioDynaMo, renames it to the given simulation name, '
    'creates a new Github repository and configures git.')
    new_sp.add_argument('SIMULATION_NAME', type=str, help='simulation name help')
    new_sp.add_argument('--no-github', action='store_true', help='Do not create a Github repository.'    )

    run_sp = sp.add_parser('run', help='Executes the simulation')

    args, unknown = parser.parse_known_args()

    if args.cmd == 'new':
        if len(unknown) != 0:
            new_sp.print_help()
            sys.exit()
        NewCommand(args.SIMULATION_NAME, args.no_github)
    elif args.cmd == 'build':
        if len(unknown) != 0:
            build_sp.print_help()
            sys.exit()
        BuildCommand()
    elif args.cmd == 'clean':
        if len(unknown) != 0:
            clean_sp.print_help()
            sys.exit()
        BuildCommand(clean=True, build=False)
    elif args.cmd == 'run':
        RunCommand(args=unknown)
    elif args.cmd == 'assist':
        if len(unknown) != 0:
            assist_sp.print_help()
            sys.exit()
        AssistCommand()
    elif args.version:
        print(Version.string())
        sys.exit()
    else:
        parser.print_help()
