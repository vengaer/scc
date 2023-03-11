import enum


class Action(enum.IntEnum):
    INIT = 0
    ADD = 1
    COMMIT = 2

    @property
    def lname(self) -> str:
        return self.name.lower()
