import asyncio
import argparse
import cbor2
from bleak import BleakScanner
from pythonosc import udp_client
import array


parser = argparse.ArgumentParser()
parser.add_argument("-a", "--osc-address", default="127.0.0.1",
    help="Ip of the device to send the osc to.")
parser.add_argument("-o", "--osc-port", type=int, default=9001,
    help="Port of the remote osc device")
arguments = parser.parse_args()

# This is used to block the asyncio loop.
stop_event = asyncio.Event()

osc_client = udp_client.SimpleUDPClient(arguments.osc_address, arguments.osc_port)

special_manufacturer_id = 0xffff

async def main():
    async def read_advertisement(device, advertising_data):
        """This callback will be called everytime bleak receives an advertisement. If the advertisement is from a polar oh1, it
        sends the last byte of the manufacturer_data as an osc message with the address format /oh1/<device id>/bpm"""
        if special_manufacturer_id in advertising_data.manufacturer_data:
            data = cbor2.loads(advertising_data.manufacturer_data[special_manufacturer_id])
            for key, value in data.items():
                osc_client.send_message(f"/{device.name}/{key}", value)
    async with BleakScanner(read_advertisement) as scanner:
        # this basically waits forever, until you stop the program with C-c
        await stop_event.wait()
        # close the osc server
        transport.close()

# This is also set by the restart method.
asyncio.run(main())
