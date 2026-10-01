#!/usr/bin/env python3
"""Validate executable ARM64 manifests/configs; ignore provenance attestations."""
import json
import sys


def fail(message):
    raise SystemExit("ERROR: " + message)


def validate_manifest(data):
    if "manifests" not in data:
        if data.get("schemaVersion") != 2 or "config" not in data:
            fail("not an image manifest/index")
        return  # Architecture is checked separately using the image config.
    executable = []
    for item in data["manifests"]:
        if item.get("annotations", {}).get("vnd.docker.reference.type") == "attestation-manifest":
            continue
        executable.append(item.get("platform", {}))
    if not executable or any(p.get("os") != "linux" or p.get("architecture") != "arm64" for p in executable):
        fail("all executable manifests must target linux/arm64")


def validate_config(data, revision):
    configs = []

    def walk(value):
        if isinstance(value, list):
            for child in value:
                walk(child)
            return
        if not isinstance(value, dict):
            return
        if "architecture" in value or "Architecture" in value:
            configs.append(value)
        else:
            for child in value.values():
                walk(child)

    walk(data)
    executable = [c for c in configs if c.get("architecture", c.get("Architecture")) != "unknown"]
    if not executable:
        fail("image config has no executable platform")
    for config in executable:
        arch = config.get("architecture", config.get("Architecture"))
        os_name = config.get("os", config.get("Os"))
        if (os_name, arch) != ("linux", "arm64"):
            fail(f"expected linux/arm64, found {os_name}/{arch}")
        labels = config.get("config", config.get("Config", {})).get("Labels", {}) or {}
        if revision and labels.get("org.opencontainers.image.revision") != revision:
            fail("image revision does not match the checked-out Git commit")


if __name__ == "__main__":
    payload = json.load(sys.stdin)
    if sys.argv[1] == "manifest":
        validate_manifest(payload)
    else:
        validate_config(payload, sys.argv[2] if len(sys.argv) > 2 else "")
    print("PASS: image metadata validated")
