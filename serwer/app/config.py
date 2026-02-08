import os
import json

CONFIG_FILE = "config.json"

# Load configuration from JSON file
def load_config(logger, initial=False):

    # Check if config file exists
    if not os.path.exists(CONFIG_FILE):
        logger.info("No configuration file found. Creating empty config.")
        return {}

    try:
        # Read config from  file
        with open(CONFIG_FILE, "r") as f:
            config = json.load(f)

        logger.info("Configuration loaded.")
        return config

    except Exception as e:
        # Handle JSON or file read errors
        logger.error(f"Failed to load config: {e}")
        return {}

# Save configuration to JSON file
def save_config(config, logger):
    """Save configuration to disk."""
    try:
        # Write config to file
        with open(CONFIG_FILE, "w") as f:
            json.dump(config, f, indent=4)

        logger.info(f"Configuration saved: {config}")

    except Exception as e:
        # Handle file write errors
        logger.error(f"Failed to save config: {e}")
