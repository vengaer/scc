import contextlib
import hashlib
import logging
import pathlib

from typing import List, Optional, Union

from .action import Action
from .cache import Cache
from .exceptions import ActionInappropriateForStage
from .stage import Stage


class Config:
    def __init__(self, cache: Cache, guard: Optional[str]) -> None:
        self._cache = cache
        self._lines: List[str] = []

        if not isinstance(self._cache["config"], str):
            raise TypeError("Configuration type invalid")
        self._path = pathlib.Path(self._cache["config"]).resolve()
        self._guard = guard if guard is not None else f"{self._path.stem}_H".upper()

        self._validate_config()

    def _validate_config(self) -> None:
        md5 = self._hash()
        if self._cache.get("hash", None) not in (md5, None):
            logging.warning("Hash of config  is invalid, resetting")
            with contextlib.suppress(FileNotFoundError):
                self._path.unlink()
            self._cache["stage"] = Stage.NOEXIST

    def _validate_stage(self, action: Action, expected: Stage) -> None:
        stage = self._cache.get("stage", Stage.NOEXIST)

        if stage != expected:
            raise ActionInappropriateForStage(stage, action)

    def _hash(self) -> str:
        md5 = hashlib.md5()

        with contextlib.suppress(FileNotFoundError):
            with open(self._path, "rb") as file:
                while chunk := file.read(8192):
                    md5.update(chunk)

        return md5.hexdigest()

    def init(self) -> None:
        if self._path.exists():
            self._path.unlink()
            self._cache["stage"] = int(Stage.NOEXIST)

        self._lines.append(f"#ifndef {self._guard}")
        self._lines.append(f"#define {self._guard}")
        self._lines.append("")

        self._cache["stage"] = Stage.IN_PROGRESS

    def add(
        self,
        option: Optional[str],
        value: Optional[Union[int, str]],
        comment: Optional[str],
    ) -> None:
        if option is None:
            raise ValueError("Option must be provided for action add")

        self._validate_stage(Action.ADD, Stage.IN_PROGRESS)

        value = f" {value}" if value is not None else ""

        if comment is not None:
            self._lines.append(f"/* {comment} */")
        self._lines.append(f"#define {option}{value}")
        self._lines.append("")

    def commit(self) -> None:
        self._validate_stage(Action.COMMIT, Stage.IN_PROGRESS)

        self._lines.append(f"#endif  /* ifndef {self._guard} */")

        self._cache["stage"] = Stage.COMMITED

    def write(self) -> None:
        if not self._path.parent.exists():
            self._path.parent.mkdir(parents=True)
        with open(self._path, "a", encoding="ascii") as file:
            file.write("\n".join(self._lines) + "\n")

    def __enter__(self) -> "Config":
        return self

    def __exit__(self, *args) -> bool:
        self.write()
        self._cache["hash"] = self._hash()
        return False
