import requests
from flask import Response

def proxy_stream(cam_id, stream_url, stream_log):
    # Generator forwarding MJPEG stream from camera
    def generate():
        try:
            # Open HTTP stream to camera
            with requests.get(stream_url, stream=True, timeout=(3, 10)) as r:
                for chunk in r.iter_content(chunk_size=1024):
                    if chunk:
                        # Forward raw stream chunks to client
                        yield chunk
        except Exception as e:
            # Log stream connection errors
            stream_log.error(f"[{cam_id}] stream error: {e}")

    # Create MJPEG streaming HTTP response
    response = Response(generate(), mimetype="multipart/x-mixed-replace; boundary=frame")
    
    # Disable keep-alive to avoid hanging connections
    response.headers["Connection"] = "close"
    return response
