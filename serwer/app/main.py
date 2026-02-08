from flask import Flask, render_template, request, redirect, url_for
from datetime import datetime
import os

from logging_setup import setup_logging
from config import load_config, save_config
from snapshot import cleanup_old_snapshots
from video import proxy_stream
from motion import handle_motion

# Flask application instance
app = Flask(__name__)

AVAILABLE_CAMERAS = {
    "cam1": "http://192.168.137.100/stream",
    "cam2": "http://192.168.137.102/stream",
    "cam3": "http://192.168.137.103/stream",
}

# Last motion timestamps
last_motion = {cam: None for cam in AVAILABLE_CAMERAS.keys()}

# Initialize logging system
logs = setup_logging()
logs["server"].info("Server started.")

# Load configuration
config = load_config(logs["server"], initial=True)

# Main dashboard view
@app.route("/")
def index():
    # Redirect to setup if configuration is missing
    if not config:
        return redirect(url_for("setup"))

    return render_template("dashboard.html",
                           cameras=AVAILABLE_CAMERAS,
                           selected_cams=config.get("selected_cameras", []),
                           notif=config.get("notification", "none"),
                           last_motion=last_motion)


# System configuration page
@app.route("/setup", methods=["GET", "POST"])
def setup():
    global config

    if request.method == "POST":
        # Read user-selected cameras and notification type
        selected = request.form.getlist("cameras")
        notif = request.form.get("notification")

        # Save configuration
        config = {"selected_cameras": selected, "notification": notif}
        save_config(config, logs["server"])
        return redirect(url_for("index"))

    return render_template("setup.html", cameras=AVAILABLE_CAMERAS)


# MJPEG stream proxy endpoint
@app.route("/video/<cam_id>")
def video_feed(cam_id):
    # Validate camera identifier
    if cam_id not in AVAILABLE_CAMERAS:
        return "Camera not found", 404
    
    return proxy_stream(cam_id, AVAILABLE_CAMERAS[cam_id], logs["stream"])

# Motion event endpoint (ESP32)
@app.route("/motion_detected", methods=["POST"])
def motion_detected():
    # Camera identifier from ESP32
    cam_id = request.args.get("camera_id", "cam1")

    # Raw JPEG data from ESP32
    data = request.data

    return handle_motion(cam_id, data, config, AVAILABLE_CAMERAS, logs, last_motion)

# Cleanup old snapshots and run the Flask app
if __name__ == "__main__":
    cleanup_old_snapshots("snapshots", logger=logs["server"])
    app.run(host="0.0.0.0", port=5000, debug=True)
