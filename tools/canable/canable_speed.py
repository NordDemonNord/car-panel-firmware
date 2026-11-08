import can
import time

bus = can.Bus(interface="socketcan", channel="can0")

def speed_pack(speed):
    raw = speed * 100
    return [raw >> 8, raw & 0xFF]

try:
    while 1:
    
        for i in range(0, 201, 10):

            msg = can.Message(arbitration_id=0x1A0, data=speed_pack(i), is_extended_id=False)
            bus.send(msg)
            time.sleep(0.1)

        for i in range(200, -1, -10):
            msg = can.Message(arbitration_id=0x1A0, data=speed_pack(i), is_extended_id=False)
            bus.send(msg)
            time.sleep(0.1)
            
finally:

    bus.shutdown()