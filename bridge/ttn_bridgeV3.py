#!/usr/bin/env python3
"""
TTN -> Mosquitto bridge for the Smart Garden dashboard.

Subscribes to The Things Network's MQTT server, reads the decoded
temperature/humidity from each LoRaWAN uplink, and republishes them to
your local Mosquitto broker on the topics the dashboard listens for:

    garden/temperature
    garden/humidity

Run it on any machine that can reach BOTH the internet (for TTN) and your
Mosquitto broker (your Home Assistant box, your laptop, a Raspberry Pi...).

Setup:
    pip install paho-mqtt
    # fill in the CONFIG section below, then:
    python3 ttn_bridge.py
"""

import json
import time
import paho.mqtt.client as mqtt

# ============================ CONFIG ============================
# ---- The Things Network (source) ----
# Find these in TTN Console:
#   TTN_APP_ID  : your application's ID (Application > Overview)
#   TTN_REGION  : the cluster your app is in (eu1 / nam1 / au1)
#   TTN_API_KEY : Application > Integrations > MQTT > "Generate new API key"
TTN_APP_ID  = ""
TTN_REGION  = ""
TTN_API_KEY = ""

# ---- Your Mosquitto broker (destination) ----
LOCAL_HOST = ""   # your HA / broker IP
LOCAL_PORT = 1883               # normal MQTT port (NOT 1884 - that's for the browser)
LOCAL_USER = ""
LOCAL_PASS = ""

# ---- Destination topics (must match the dashboard settings) ----
TOPIC_TEMP = "garden/temperature"
TOPIC_HUM  = "garden/humidity"
# ===============================================================

# TTN connection details (derived)
TTN_HOST = f"{TTN_REGION}.cloud.thethings.network"
TTN_PORT = 8883  # TLS port (1883 is plaintext and will reset the connection)
TTN_USER = f"{TTN_APP_ID}@ttn"  # tenant is 'ttn' per the console MQTT page, e.g. greenhome2@ttn
# Subscribe to every device's uplinks in this application
TTN_TOPIC = f"v3/{TTN_USER}/devices/+/up"


# ---- local publisher (to Mosquitto) ----
local = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2, client_id="ttn-bridge-pub")
if LOCAL_USER:
    local.username_pw_set(LOCAL_USER, LOCAL_PASS)


def connect_local():
    while True:
        try:
            local.connect(LOCAL_HOST, LOCAL_PORT, keepalive=60)
            local.loop_start()
            print(f"[local] connected to {LOCAL_HOST}:{LOCAL_PORT}")
            return
        except Exception as e:
            print(f"[local] connect failed: {e} - retrying in 5s")
            time.sleep(5)


def deep_find(obj, keys):
    """Recursively search a dict for the first numeric value under any of `keys`."""
    if isinstance(obj, dict):
        for k, v in obj.items():
            if k.lower() in keys and isinstance(v, (int, float)):
                return v
        for v in obj.values():
            r = deep_find(v, keys)
            if r is not None:
                return r
    return None


# ---- TTN subscriber callbacks ----
def on_connect(client, userdata, flags, reason_code, properties):
    if reason_code == 0:
        print(f"[ttn] connected to {TTN_HOST}")
        client.subscribe(TTN_TOPIC)
        print(f"[ttn] subscribed to {TTN_TOPIC}")
    else:
        print(f"[ttn] connect failed, rc={reason_code} "
              "(check APP_ID / API key / region)")


def on_message(client, userdata, msg):
    try:
        payload = json.loads(msg.payload.decode())
    except Exception:
        return

    decoded = payload.get("uplink_message", {}).get("decoded_payload", {})
    if not decoded:
        # No decoded payload - did you set the TTN payload formatter?
        print("[ttn] uplink with no decoded_payload (set the formatter in TTN)")
        return

    temp = deep_find(decoded, {"temperature", "temp"})
    hum  = deep_find(decoded, {"humidity", "hum", "rh"})

    dev = msg.topic.split("/")[-2] if "/" in msg.topic else "?"

    if temp is not None:
        local.publish(TOPIC_TEMP, str(temp), retain=True)
    if hum is not None:
        local.publish(TOPIC_HUM, str(hum), retain=True)

    print(f"[bridge] {dev}: temp={temp}  hum={hum}  -> republished")


def main():
    connect_local()

    ttn = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2, client_id="ttn-bridge-sub")
    ttn.username_pw_set(TTN_USER, TTN_API_KEY)
    ttn.tls_set()  # TTN requires TLS
    ttn.on_connect = on_connect
    ttn.on_message = on_message

    while True:
        try:
            ttn.connect(TTN_HOST, TTN_PORT, keepalive=60)
            break
        except Exception as e:
            print(f"[ttn] connect failed: {e} - retrying in 5s")
            time.sleep(5)

    print("[bridge] running - press Ctrl+C to stop")
    try:
        ttn.loop_forever()
    except KeyboardInterrupt:
        print("\n[bridge] stopping")
        ttn.disconnect()
        local.loop_stop()
        local.disconnect()


if __name__ == "__main__":
    main()
