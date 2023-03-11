import argparse
import functools

from typing import Optional, Union

import fasteners

from .action import Action
from .cache import Cache
from .config import Config
from .exceptions import InvalidAction


def _dispatch(
    guard: Optional[str],
    cache: str,
    config: str,
    action: Union[Action, str],
    option: Optional[str],
    value: Optional[str],
    comment: Optional[str],
) -> None:
    if isinstance(action, str):
        try:
            action = getattr(Action, action.upper())
        except AttributeError as exc:
            raise InvalidAction(action) from exc

    with Cache(cache) as ccache:
        if config is not None:
            ccache["config"] = config

        with Config(ccache, guard) as cfg:
            attr = getattr(cfg, action.lname)  # type: ignore[union-attr]
            if action == Action.ADD:
                attr = functools.partial(
                    attr, option=option, value=value, comment=comment
                )

            attr()


def main() -> None:
    parser = argparse.ArgumentParser("Configuration header tool")
    parser.add_argument("-g", "--guard", help="Header guard", default=None)
    parser.add_argument(
        "-l", "--lockfile", help="Path to lockfile", default=".scconfig.lock"
    )
    parser.add_argument(
        "-c", "--cache", help="Override cache directory", default=".scconfig.cache"
    )
    parser.add_argument(
        "-C",
        "--comment",
        help="Comment to associate with option, applies only to action 'add'",
        default=None,
    )
    parser.add_argument(
        "-o",
        "--config",
        help="Path to the config header. "
        "Read from the .scconfig.cache if not provided",
        default=None,
    )
    parser.add_argument(
        "action",
        metavar="ACTION",
        choices=tuple(v.lname for v in Action),
        help="The action to perform, choises are "
        f'{", ".join((v.lname for v in Action))}',
    )
    parser.add_argument(
        "option",
        metavar="OPTION",
        nargs="?",
        default=None,
        help='Option to add, required for action "add"',
    )
    parser.add_argument(
        "value",
        metavar="VALUE",
        nargs="?",
        default=None,
        help="Optional value to define the preceding option to. "
        'Used only if action is "add"',
    )
    args = parser.parse_args()
    with fasteners.InterProcessLock(args.lockfile):
        _dispatch(
            args.guard,
            args.cache,
            args.config,
            args.action,
            args.option,
            args.value,
            args.comment,
        )
