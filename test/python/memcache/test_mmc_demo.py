import unittest

from pymmc import DistributedObjectStore


DISCOVERY_URL = "tcp://127.0.0.1:5000"
RANK_ID = 0
TIMEOUT = 5


class TestExample(unittest.TestCase):
    def setUp(self):
        self._distributed_object_store = DistributedObjectStore()
        self._distributed_object_store.init(DISCOVERY_URL, RANK_ID, TIMEOUT)
        print("object store initialized")

    def test_put(self):
        pass

    def tearDown(self):
        self._distributed_object_store.close()
        print("object store destroyed")


if __name__ == '__main__':
    unittest.main()
