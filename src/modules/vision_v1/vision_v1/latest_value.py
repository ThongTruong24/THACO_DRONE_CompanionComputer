"""Bounded single-slot mailbox with wakeable shutdown semantics."""
from __future__ import annotations

import threading
from typing import Generic, Optional, Tuple, TypeVar


T = TypeVar("T")


class LatestValue(Generic[T]):
    def __init__(self) -> None:
        self._condition = threading.Condition()
        self._value: Optional[T] = None
        self._version = 0
        self._pending = False
        self._closed = False
        self._dropped = 0

    def put(self, value: T) -> bool:
        """Replace the pending value. Returns False after the mailbox is closed."""
        with self._condition:
            if self._closed:
                return False
            if self._pending:
                self._dropped += 1
            self._value = value
            self._version += 1
            self._pending = True
            self._condition.notify_all()
            return True

    def wait_next(self, last_version: int, timeout: Optional[float] = None) -> Optional[Tuple[int, T]]:
        with self._condition:
            self._condition.wait_for(
                lambda: self._closed or self._version != last_version,
                timeout=timeout,
            )
            if self._version == last_version:
                return None
            value = self._value
            version = self._version
            self._pending = False
            if value is None:
                return None
            return version, value

    def close(self) -> None:
        with self._condition:
            self._closed = True
            self._condition.notify_all()

    @property
    def dropped(self) -> int:
        with self._condition:
            return self._dropped

    @property
    def closed(self) -> bool:
        with self._condition:
            return self._closed
