import abc

from .action import Action
from .stage import Stage


class ConfigError(Exception):
    def __init__(self) -> None:
        super().__init__(self.what)

    @property
    @abc.abstractmethod
    def what(self) -> str:
        ...


class ConfigNotFound(ConfigError):
    what = "Could not find config file"


class ActionInappropriateForStage(ConfigError):
    def __init__(self, stage: Stage, action: Action) -> None:
        self._stage = stage
        self._action = action
        super().__init__()

    @property
    def stage(self) -> Stage:
        return self._stage

    @property
    def action(self) -> Action:
        return self._action

    @property
    def what(self) -> str:
        return f"Action {self._action} not allowed in stage {self._stage}"


class InvalidAction(ConfigError):
    def __init__(self, action):
        self._action = action
        super().__init__()

    @property
    def action(self) -> Action:
        return self._action

    @property
    def what(self) -> str:
        return f"Invalid action {self._action.lname}"
