import bip_msg
import pickle
import time
with open("game_msg2.pkl", "rb") as f:
    game_msg = pickle.load(f)
bip_msg.remove_shm()
print(type(game_msg))
bip_msg.open_shm()
ret = bip_msg.dispatch_proto(123456, "123456", game_msg, "123456")
msg = bip_msg.fetch_proto()
print(len(msg[2]))
print(len(game_msg))
# ret = bip_msg.process_string(game_msg)
# print((ret))

bip_msg.push_actions("12342", game_msg)
time.sleep(0.16)
bip_msg.fetch_actions("12342")