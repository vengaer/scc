''' GDB plugin for managing Mull mutations interactively '''

import abc
import contextlib
import dataclasses
import inspect
import re

from elftools.elf.elffile import ELFFile

@dataclasses.dataclass
class Mutation:
    ''' A mutation as expressed by Mull '''
    name: str
    filename: str
    line: int
    column: int

    @property
    def envvar(self):
        return ':'.join((self.name, self.filename, self.line, self.column))

class _MullUserCommand(gdb.Command):
    ''' Abstract user command base. Any bottom-level class inheriting from
    it generates a gdb command on the form mull-<cmd> '''
    def __init__(self):
        super().__init__(f'mull-{self.cmd}', gdb.COMMAND_USER)

    @abc.abstractmethod
    def invoke(self, arg, from_tty):
        ...

    @property
    @abc.abstractmethod
    def cmd(self):
        ''' Name of the command '''
        ...

class Source(_MullUserCommand):
    ''' Source script for on-the-fly updates '''

    def invoke(self, arg, from_tty):
        frame = inspect.currentframe()
        if frame is None:
            gdb.write('Could not get current frame\n', gdb.STDERR)
            gdb.flush()
            return

        gdb.execute(f'source {inspect.getfile(frame)}')

    @property
    def cmd(self):
        return 'source'

class _MutationCommand(_MullUserCommand):
    ''' Mutation command base '''
    def __init__(self):
        super().__init__()
        self._parse_mutations()

    @property
    def mutations(self):
        return self._mutations

    @classmethod
    @property
    def envregex(self):
        # Mull mutators seem to, at least currently, be
        # on the form cxx_<op>:<path>:line:column.
        return r'^([^:]+):([^:]+):(\d+):(\d+)$'

    @classmethod
    @contextlib.contextmanager
    def _open_elf(cls, path):
        with open(path, 'rb') as blob:
            yield ELFFile(blob)

    @classmethod
    def _read_strtab(cls, elf):
        ''' Find all strings listed in the ELF file's
        string table '''
        for section in elf.iter_sections():
            if section.name != '.strtab':
                continue

            strings = []
            i = 0
            while i < section.data_size:
                strings.append(section.get_string(i))
                i += len(strings[-1]) + 1

            return strings

        gdb.write('No .strtab section in object\n', gdb.STDERR)
        gdb.flush()
        return []

    @classmethod
    def _filter_mutators(cls, strings):
        ''' Find the strings mull uses to look up mutations
        in environment '''

        pattern = re.compile(cls.envregex)
        matches = map(pattern.match, strings)
        filtered = filter(lambda m: m is not None, matches)
        return tuple(Mutation(*(f.group(i) for i in range(1,5))) for f in filtered)

    def _parse_mutations(self):
        for objfile in gdb.objfiles():
            with self._open_elf(objfile.filename) as elf:
                strings = self._read_strtab(elf)

        self._mutations = self._filter_mutators(strings)

    def _list_mutations(self):
        for i, mut in enumerate(self.mutations):
            gdb.write(f'{(str(i) + ":").ljust(5)}{mut.name.ljust(30)} at {mut.filename}:{mut.line}:{mut.column}\n')
        gdb.flush()

    def _get_mutation(self, arg):
        arg = arg.strip()
        if not arg:
            self._list_mutations()
            return None
        try:
            idx = int(arg)
        except ValueError:
            gdb.write(f'Invalid argument {arg}, integer expected\n', gdb.STDERR)
            gdb.flush()
            return None

        if not (0 <= idx < len(self.mutations)):
            gdb.write(f'Invalid index {idx}, must be in range(0,{len(self.mutations)})\n')
            gdb.flush()
            return None

        return self.mutations[idx]

class ListMutations(_MutationCommand):
    ''' List available mull mutations '''

    def invoke(self, arg, from_tty):
        self._list_mutations()

    @property
    def cmd(self):
        return 'ls'

class EnableMutation(_MutationCommand):
    ''' Enable specific mull mutation. Parameter is the index into
 the mutation list a shown by mull-ls '''

    def invoke(self, arg, from_tty):
        if (mut := self._get_mutation(arg)) is not None:
            gdb.execute(f'set env {mut.envvar}=1')

    @property
    def cmd(self):
        return 'enable'

class DisableMutation(_MutationCommand):
    ''' Disable specific mull mutation. Parameters is the index into
 the mutation list as shown by mull-ls'''

    def invoke(self, arg, from_tty):
        arg = arg.strip()
        mut = None

        # May have passed the actual environment variable
        if (match := re.match(self.envregex, arg)):
            mut = Mutation(*(match.group(i) for i in range(1,5)))

        if mut is not None or (mut := self._get_mutation(arg)) is not None:
            gdb.execute(f'unset env {mut.envvar}')

    @property
    def cmd(self):
        return 'disable'

Source()
ListMutations()
EnableMutation()
DisableMutation()
