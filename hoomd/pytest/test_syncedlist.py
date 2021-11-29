import numpy as np
import pytest
from hoomd.conftest import BaseTestList, pickling_check
from hoomd.pytest.dummy import DummyOperation, DummySimulation
from hoomd.data.syncedlist import SyncedList, _PartialIsInstance


class OpInt(int):
    """Used to test SyncedList where item equality checks are needed."""

    def _attach(self):
        self._cpp_obj = None

    @property
    def _attached(self):
        return hasattr(self, '_cpp_obj')

    def _detach(self):
        del self._cpp_obj

    def _add(self, simulation):
        self._simulation = simulation

    def _remove(self):
        del self._simulation

    @property
    def _added(self):
        return hasattr(self, '_simulation')


class TestSyncedList(BaseTestList):
    _rng = np.random.default_rng(12564)

    @property
    def rng(self):
        return self._rng

    @pytest.fixture(autouse=True, params=(DummyOperation, OpInt))
    def item_cls(self, request):
        return request.param

    @pytest.fixture(autouse=True, params=(True, False))
    def attached(self, request):
        return request.param

    @pytest.fixture(autouse=True, params=(True, False))
    def attach_items(self, request):
        return request.param

    @pytest.fixture
    def generate_plain_list(self, item_cls):
        if item_cls == DummyOperation:

            def generate(n):
                return [DummyOperation() for _ in range(n)]

            return generate
        else:

            def generate(n):
                return [OpInt(self.rng.integers(100_000_000)) for _ in range(n)]

            return generate

    @pytest.fixture(scope="function")
    def empty_list(self, item_cls, attached, attach_items):
        list_ = SyncedList(validation=_PartialIsInstance(item_cls),
                           attach_members=attach_items)
        if attached:
            list_._sync(DummySimulation(), [])
        return list_

    @pytest.fixture(scope="function")
    def plain_list(self, generate_plain_list, n):
        return generate_plain_list(n)

    def is_equal(self, a, b):
        if isinstance(a, DummyOperation):
            return a is b
        return a == b

    def final_check(self, test_list):
        if test_list._synced:
            if test_list._attach_members:
                assert all(item._attached for item in test_list)
            else:
                assert not any(
                    getattr(item, "_attached", False) for item in test_list)
        if test_list._attach_members:
            assert all(item._added for item in test_list)
        else:
            assert not any(getattr(item, "_added", False) for item in test_list)

    def test_init(self, generate_plain_list, item_cls):

        validate = _PartialIsInstance(item_cls)

        # Test automatic to_synced_list function generation
        synced_list = SyncedList(validation=validate)
        assert synced_list._validate == validate
        op = item_cls()
        assert synced_list._to_synced_list_conversion(op) is op

        # Test specified to_synced_list
        def cpp_identity(x):
            return x._cpp_obj

        # Test full initialziation
        plain_list = generate_plain_list(5)
        synced_list = SyncedList(validation=validate,
                                 to_synced_list=cpp_identity,
                                 iterable=plain_list)
        assert synced_list._to_synced_list_conversion == cpp_identity
        op._cpp_obj = 2
        assert synced_list._to_synced_list_conversion(op) == 2
        assert all(op._added for op in synced_list)
        self.check_equivalent(plain_list, synced_list)

    def test_synced(self):
        test_list = SyncedList(lambda x: x)
        assert not test_list._synced
        test_list._sync(None, [])
        assert test_list._synced
        test_list._unsync()
        assert not test_list._synced

    def test_attach_value(self, empty_list, item_cls):
        op = item_cls()
        empty_list._attach_value(op)
        assert op._attached == (empty_list._synced
                                and empty_list._attach_members)
        assert op._added or not empty_list._attach_members

    def test_validate_or_error(self, empty_list, item_cls):
        with pytest.raises(ValueError):
            empty_list._validate_or_error(3)
        with pytest.raises(ValueError):
            empty_list._validate_or_error(None)
        with pytest.raises(ValueError):
            empty_list._validate_or_error("hello")
        empty_list._validate_or_error(item_cls())

    def test_syncing(self, populated_list):
        test_list, plain_list = populated_list
        self.final_check(test_list)

    def test_unsync(self, populated_list):
        test_list, plain_list = populated_list
        test_list._unsync()
        assert not hasattr(test_list, "_synced_list")
        self.final_check(test_list)

    def test_synced_iter(self, empty_list):
        empty_list._simulation = None
        empty_list._empty_list = [1, 2, 3]
        assert all(
            [i == j for i, j in zip(range(1, 4), empty_list._synced_iter())])

    def test_pickling(self, populated_list):
        test_list, _ = populated_list
        pickling_check(test_list)
