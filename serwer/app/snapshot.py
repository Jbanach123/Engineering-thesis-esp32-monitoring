import os
import time
from datetime import datetime

def save_snapshot(cam_id, data, motion_log):
    date_folder = datetime.now().strftime("%Y-%m-%d")
    save_dir = os.path.join("snapshots", f"snapshots_{date_folder}")
    os.makedirs(save_dir, exist_ok=True)

    timestamp = datetime.now().strftime("%Y-%m-%d_%H-%M-%S")
    filename = os.path.join(save_dir, f"{cam_id}_{timestamp}.jpg")

    with open(filename, "wb") as f:
        f.write(data)

    motion_log.info(f"[{cam_id}] Saved snapshot: {filename}")
    return filename

def cleanup_old_snapshots(base_dir="snapshots", days=30, logger=None):
    # Remove snapshot files older than given number of days
    cutoff = time.time() - days * 86400
    removed = 0
    for root, _, files in os.walk(base_dir):
        for name in files:
            path = os.path.join(root, name)
            try:
                # Check file age
                if os.path.getmtime(path) < cutoff:
                    os.remove(path)
                    logger.info(f"Removed old snapshot: {path}")
                    removed += 1
            except Exception as e:
                logger.error(f"Error removing {path}: {e}")
    logger.info(f"cleanup_old_snapshots completed, removed {removed} files.")

