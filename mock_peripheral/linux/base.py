from abc import ABC, abstractmethod


class Peripheral(ABC):
    """Abstract base class for all mock peripheral implementations.

    A peripheral is any device that communicates over serial — sensor or actuator.
    Subclass this and implement handle_query() to respond to incoming commands.
    Register the instance in peripheral.py's peripherals list.
    """

    @abstractmethod
    def handle_query(self, message: str) -> str | None:
        """Handle an incoming query string.

        Returns a response string (without newline) if the query is recognised,
        or None to indicate that this peripheral does not handle the query.
        """
