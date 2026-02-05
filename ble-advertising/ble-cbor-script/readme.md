# BLE CBOR to OSC

This Python script works in conjunction with the BLE-advertising example to expose sensor values sent as CBOR by BLE advertisement packages as OSC messages.

## Run BLE-CBOR-to-OSC Script:

### Installation

Clone/Download the BLE-CBOR-to-OSC repository if not already done.
Create and activate a Python virtual environment then install dependencies: 
```bash
python -m venv venv
source ./venv/bin/activate
pip install -r requirements.txt
```

### Runnning script

`python ble-cbor-to-osc.py`

You can specify the osc output's address and port with `-a` and `-p`.
