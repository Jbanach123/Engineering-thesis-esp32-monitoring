from flask import jsonify
from datetime import datetime
from snapshot import save_snapshot
from notifications import send_motion_notification

def handle_motion(cam_id, data, config, AVAILABLE_CAMERAS, logs, last_motion):
    # Validate camera identifier
    if cam_id not in AVAILABLE_CAMERAS:
        logs["error"].warning(f"motion_detected: unknown camera {cam_id}")
        return jsonify({"error": "unknown camera"}), 400
    
    # Validate received image data
    if not data:
        logs["error"].error(f"motion_detected: no image data from {cam_id}")
        return jsonify({"error": "no image"}), 400

    # Save snapshot to disk
    save_snapshot(cam_id, data, logs["motion"])

    # Update last motion timestamp
    last_motion[cam_id] = datetime.now().strftime("%H:%M:%S")

    # Send notification if enabled
    notif_type = config.get("notification", "none")
    if notif_type != "none":
        send_motion_notification(cam_id, notif_type, logs["notifications"])

    return jsonify({"status": "ok"})
