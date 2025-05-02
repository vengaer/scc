import logging
import json
import pathlib

from typing import Any, Dict, Iterator, MutableMapping, Union


class Cache(MutableMapping):
    def __init__(self, path: Union[str, pathlib.Path]) -> None:
        self._path = path if isinstance(path, pathlib.Path) else pathlib.Path(path)

        if not self._path.parent.resolve().exists():
            self._path.parent.mkdir(parents=True, exist_ok=True)

        self._data = self.load()

    @property
    def path(self) -> pathlib.Path:
        return self._path

    def load(self) -> Dict[str, Union[int, str]]:
        if self._path.exists():
            try:
                with open(self._path, "r", encoding="ascii") as file:
                    return json.load(file)
            except json.JSONDecodeError:
                logging.warning("Cache corrupted")
                self._path.unlink()

        return {}

    def save(self) -> None:
        with open(self._path, "w", encoding="ascii") as file:
            json.dump(self._data, file)

    def get(self, key: str, default=None) -> Any:
        return self._data.get(key, default)

    def __setitem__(self, key: str, value: Union[int, str]) -> None:
        self._data[key] = value

    def __getitem__(self, key: str) -> Union[int, str]:
        return self._data[key]

    def __delitem__(self, key: str) -> None:
        del self._data[key]

    def __len__(self) -> int:
        return len(self._data)

    def __iter__(self) -> Iterator[str]:
        return iter(self._data)

    def __enter__(self) -> "Cache":
        return self

    def __exit__(self, *args) -> bool:
        self.save()
        return False
