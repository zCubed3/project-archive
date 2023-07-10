import abc


class ExtensionScript(abc.ABC):
    # The name of this extension
    @abc.abstractmethod
    def get_name(self) -> str:
        pass

    # The author of this extension
    @abc.abstractmethod
    def get_author(self) -> str:
        pass

    # Used to block certain models from being visible
    # They may still be loaded though if a user knows it by name!
    def filter_models(self) -> list[str]:
        return []