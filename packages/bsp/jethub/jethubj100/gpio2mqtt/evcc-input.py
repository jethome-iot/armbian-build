#!/usr/bin/env python3
"""Toggle evcc Relay 1 on IN1 presses."""

import argparse
import json
import logging
import signal
import subprocess
import sys
import time
import urllib.request


TOPIC = "jethub-mqtt-io/input/jethub_input_1"
DEBOUNCE = 0.3


def toggle_relay(base_url):
    with urllib.request.urlopen(f"{base_url}/api/state", timeout=5) as response:
        state = json.load(response)
    state = state.get("result", state)
    if not state.get("loadpoints"):
        raise ValueError("evcc loadpoint 1 is not configured")
    mode = "now" if state["loadpoints"][0]["mode"] == "off" else "off"
    request = urllib.request.Request(
        f"{base_url}/api/loadpoints/1/mode/{mode}", data=b"", method="POST"
    )
    with urllib.request.urlopen(request, timeout=5) as response:
        response.read()
    logging.info("IN1 sets Relay 1 mode to %s", mode)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--evcc-url", default="http://127.0.0.1:7070")
    args = parser.parse_args()
    logging.basicConfig(level=logging.INFO, format="%(levelname)s %(message)s")
    signal.signal(signal.SIGTERM, lambda *_: sys.exit(0))
    subscriber = subprocess.Popen(
        ["mosquitto_sub", "-h", "127.0.0.1", "-q", "1", "-R", "-t", TOPIC],
        stdout=subprocess.PIPE,
        text=True,
        bufsize=1,
    )
    pressed = False
    ignore_until = 0.0
    logging.info("Listening for IN1 presses on %s", TOPIC)
    try:
        for message in subscriber.stdout:
            value = message.strip()
            if value == "OFF":
                pressed = False
                continue
            if value != "ON" or pressed:
                continue
            pressed = True
            if time.monotonic() < ignore_until:
                continue
            try:
                toggle_relay(args.evcc_url.rstrip("/"))
            except (OSError, ValueError, KeyError, TypeError) as error:
                logging.error("Cannot toggle Relay 1: %s", error)
            finally:
                ignore_until = time.monotonic() + DEBOUNCE
        raise RuntimeError(f"MQTT subscriber exited: {subscriber.wait()}")
    finally:
        subscriber.terminate()
        try:
            subscriber.wait(timeout=5)
        except subprocess.TimeoutExpired:
            subscriber.kill()
            subscriber.wait()


if __name__ == "__main__":
    main()
