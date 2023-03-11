import enum


class Stage(enum.IntEnum):
    NOEXIST = 0
    IN_PROGRESS = 1
    COMMITED = 2

    @property
    def lname(self) -> str:
        return self.name.lower()
