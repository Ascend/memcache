from test_case_common import TestClient

if __name__ == "__main__":
    client = TestClient('127.0.0.1', 10001)
    client.execute("help")
    client.execute("getServerCommands")

    count = 16 * 1024
    keys = []
    put_sizes = []
    get_sizes = []
    for i in range(count):
        keys.append('test' + str(i))
        put_sizes.append(1024)
        get_sizes.append(1024)

    print("============ test cpu =========== ")
    media = 0
    client.init_mmc()
    client.batch_put_from(keys, put_sizes, media)
    client.batch_get_into(keys, get_sizes, media)
    client.batch_is_exit(keys)
    client.batch_remove(keys)
    client.batch_is_exit(keys)

    print("============ test npu =========== ")
    media = 1
    client.batch_put_from(keys, put_sizes, media)
    client.batch_get_into(keys, get_sizes, media)
    client.batch_is_exit(keys)
    client.batch_remove(keys)
    client.batch_is_exit(keys)

    client.close_mmc()
