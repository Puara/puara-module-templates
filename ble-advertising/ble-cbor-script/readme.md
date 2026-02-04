# BLE CBOR to OSC

This python script works in conjunction with the esp32 template defined [here](https://github.com/guillaumeriousat/puara-module-template/tree/feat/ble-advert-template) to expose sensor values sent as CBOR by BLE advertisement packages as osc messages.

## Installation

`pip install -r requirements.txt`

## Usage

`python ble-cbor-to-osc.py`

You can specify the osc output's address and port with `-a` and `-p`.
