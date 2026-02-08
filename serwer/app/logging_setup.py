import os
import logging
from datetime import datetime
from logging.handlers import RotatingFileHandler

LOG_DIR = "logs"
os.makedirs(LOG_DIR, exist_ok=True)

LOG_FILE = os.path.join(LOG_DIR, f"{datetime.now().strftime('%Y-%m-%d')}.log")
LOG_FORMAT = "%(asctime)s | %(levelname)s | %(name)s | %(message)s"

# Configure application logging
def setup_logging():

    # Root logger shared by all modules
    root_logger = logging.getLogger()

    # Check if handlers already exist (Flask debug reload)
    already_initialized = bool(root_logger.handlers)

    # Global log level
    root_logger.setLevel(logging.INFO)

    if not already_initialized:
        # File logger with rotation
        file_handler = RotatingFileHandler(LOG_FILE, maxBytes=5*1024*1024, backupCount=5, encoding="utf-8")
        file_handler.setFormatter(logging.Formatter(LOG_FORMAT))

        # Console logger
        console_handler = logging.StreamHandler()
        console_handler.setFormatter(logging.Formatter(LOG_FORMAT))

        root_logger.addHandler(file_handler)
        root_logger.addHandler(console_handler)

    # Named loggers used across the project
    return {
        "server": logging.getLogger("server"),
        "motion": logging.getLogger("motion"),
        "stream": logging.getLogger("stream"),
        "notifications": logging.getLogger("notifications"),
        "error": logging.getLogger("errors")
    }
