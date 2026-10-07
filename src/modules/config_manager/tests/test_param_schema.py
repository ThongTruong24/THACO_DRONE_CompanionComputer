import os
import yaml
import pytest

CONFIG_DIR = os.path.abspath(os.path.join(os.path.dirname(__file__), "../config"))

def test_config_files_exist():
    assert os.path.exists(os.path.join(CONFIG_DIR, "param_schema.yaml")), "param_schema.yaml missing"
    assert os.path.exists(os.path.join(CONFIG_DIR, "factory_default.yaml")), "factory_default.yaml missing"
    assert os.path.exists(os.path.join(CONFIG_DIR, "cc_system.yaml")), "cc_system.yaml missing"

def test_param_schema_structure_and_constraints():
    schema_path = os.path.join(CONFIG_DIR, "param_schema.yaml")
    with open(schema_path, "r") as f:
        schema = yaml.safe_load(f)

    assert "parameters" in schema, "Schema must have root 'parameters' key"
    params = schema["parameters"]
    assert len(params) > 0, "Schema must not be empty"

    mav_ids = set()
    for full_name, p in params.items():
        # Name format: node.param_name or subsystem.param_name
        assert "." in full_name, f"Parameter '{full_name}' must follow <node>.<param> format"
        
        # MAVLink ID constraint: <= 16 characters
        assert "mav_id" in p, f"Parameter '{full_name}' missing mav_id"
        mav_id = p["mav_id"]
        assert isinstance(mav_id, str), f"mav_id for '{full_name}' must be string"
        assert len(mav_id) <= 16, f"mav_id '{mav_id}' exceeds 16 chars limit: {len(mav_id)}"
        assert mav_id not in mav_ids, f"Duplicate mav_id detected: '{mav_id}'"
        mav_ids.add(mav_id)

        # Type constraint
        assert "type" in p, f"Parameter '{full_name}' missing type"
        assert p["type"] in ["int", "float", "string", "bool"], f"Invalid type for '{full_name}': {p['type']}"

        # Default constraint
        assert "default" in p, f"Parameter '{full_name}' missing default value"
        val = p["default"]
        if p["type"] == "int":
            assert isinstance(val, int) and not isinstance(val, bool)
        elif p["type"] == "float":
            assert isinstance(val, (int, float)) and not isinstance(val, bool)
        elif p["type"] == "string":
            assert isinstance(val, str)
        elif p["type"] == "bool":
            assert isinstance(val, bool)

        # Tier constraint
        assert "tier" in p, f"Parameter '{full_name}' missing tier"
        assert p["tier"] in [1, 2, 3], f"Invalid tier for '{full_name}': {p['tier']}"

        if "options" in p:
            assert isinstance(p["options"], list)
            assert val in p["options"], f"Default '{val}' not in options for '{full_name}'"

        if "min" in p and "max" in p:
            assert p["min"] <= p["max"]
            assert p["min"] <= val <= p["max"], f"Default '{val}' out of [{p['min']}, {p['max']}] for '{full_name}'"

def test_factory_default_and_cc_system_compliance():
    schema_path = os.path.join(CONFIG_DIR, "param_schema.yaml")
    with open(schema_path, "r") as f:
        schema = yaml.safe_load(f)["parameters"]

    for config_filename in ["factory_default.yaml", "cc_system.yaml"]:
        cfg_path = os.path.join(CONFIG_DIR, config_filename)
        with open(cfg_path, "r") as f:
            cfg = yaml.safe_load(f)

        assert "version" in cfg, f"{config_filename} missing version"
        assert "router" in cfg, f"{config_filename} missing router section"
        assert "network" in cfg, f"{config_filename} missing network section"
        assert "camera" in cfg, f"{config_filename} missing camera section"
        assert "vision" in cfg, f"{config_filename} missing vision section"
