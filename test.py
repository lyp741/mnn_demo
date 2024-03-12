import bip_msg
import pickle

with open("game_msg2.pkl", "rb") as f:
    game_msg = pickle.load(f)

print(type(game_msg))
bip_msg.init_shm()
ret = bip_msg.dispatch_proto(123456, "123456", game_msg, "123456")
msg = bip_msg.fetch_proto()
print(len(msg[2]))
print(len(game_msg))
# ret = bip_msg.process_string(game_msg)
# print((ret))

