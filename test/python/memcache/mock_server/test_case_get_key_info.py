from test_case_common import TestClient

if __name__ == "__main__":
    client = TestClient('127.0.0.1', 5000)
    client.execute("help")
    client.execute("getServerCommands")

    media = 0

    print("============ test get key info =========== ")
    client.init_mmc()
    key1 = "test_key1"
    key2 = "test_key2"
    client.put_from(key1, 1024, media)
    client.is_exist(key1)

    client.put_from(key2, 1024, media)
    client.is_exist(key2)

    client.get_key_info(key1)

    keylist = [key1, key2]
    client.batch_get_key_info(keylist)

    client.close_mmc()
