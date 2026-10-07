from dataclasses import FrozenInstanceError
from concurrent.futures import ThreadPoolExecutor

import pytest

from vision_v1.ai_vision_control import AiVisionControlState, AiVisionControlStore


def test_default_state_is_disabled():
    assert AiVisionControlStore().latest() == AiVisionControlState(False, False, False)


def test_store_replaces_all_flags_and_keeps_previous_snapshot_immutable():
    store = AiVisionControlStore()
    store.update(True, False, True)
    first = store.latest()
    assert first == AiVisionControlState(True, False, True)
    store.update(False, False, False)
    assert store.latest() == AiVisionControlState(False, False, False)
    assert first == AiVisionControlState(True, False, True)
    with pytest.raises(FrozenInstanceError):
        first.tracking = True


def test_concurrent_reads_observe_complete_states():
    store = AiVisionControlStore()

    def write():
        for index in range(2000):
            value = bool(index % 2)
            store.update(value, value, value)

    def read():
        for _ in range(2000):
            state = store.latest()
            assert state.bounding_box == state.tracking == state.following

    with ThreadPoolExecutor(max_workers=4) as executor:
        futures = [executor.submit(write), *(executor.submit(read) for _ in range(3))]
        for future in futures:
            future.result()
