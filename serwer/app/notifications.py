import os
import time
import smtplib
from datetime import datetime
from twilio.rest import Client

# Notification cooldown
NOTIFY_COOLDOWN_MINUTES = 30  
last_notification_time = {}   # stores last notification timestamp per camera

# Email settings
app_email = os.getenv("APP_EMAIL")
email_password = os.getenv("EMAIL_PASSWORD")
reciever_email = os.getenv("RECIEVER_EMAIL")

# SMS settings
account_sid = os.getenv("TWILIO_SID")
auth_token = os.getenv("TWILIO_TOKEN")
reciever_phone_number = os.getenv("RECIEVER_PHONE_NUMBER")
twilio_phone_number = os.getenv("TWILIO_PHONE_NUMBER")

def send_motion_email(message, notif_log):
    try:
        with smtplib.SMTP("smtp.gmail.com", port=587) as connection:
            connection.starttls()
            connection.login(user=app_email, password=email_password)
            connection.sendmail(
                from_addr=app_email,
                to_addrs=reciever_email,
                msg=f"Subject:Motion Detected!\n\n{message}"
            )
        notif_log.info("Email sent successfully")
    except Exception as e:
        notif_log.error(f"Email send failed: {e}")


def send_motion_sms(message, notif_log):
    try:
        client = Client(account_sid, auth_token)
        client.messages.create(
            body=message,
            from_=twilio_phone_number,
            to=reciever_phone_number
        )
        notif_log.info("SMS sent successfully")
    except Exception as e:
        notif_log.error(f"SMS send failed: {e}")

# --- Wysyłanie powiadomień ---
def send_motion_notification(cam_id, notif_type, notif_log):
    now = time.time()
    cooldown_seconds = NOTIFY_COOLDOWN_MINUTES * 60
    last_time = last_notification_time.get(cam_id, 0)

    # sprawdź czy minęło wystarczająco dużo czasu
    if now - last_time < cooldown_seconds:
        remaining = int((cooldown_seconds - (now - last_time)) / 60)
        notif_log.info(f"Skipping notification for {cam_id}, cooldown {remaining} min")
        return

    # Update last notification time
    last_notification_time[cam_id] = now

    message = f"Wykryto ruch na {cam_id} o {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}!"

    #Check notification type and send
    if notif_type in ("email", "E-mail & SMS"):
        notif_log.info(f"Sending EMAIL for {cam_id}")
        send_motion_email(message, notif_log)
        time.sleep(2)  # Prevent burst sending

    if notif_type in ("sms", "E-mail & SMS"):
        notif_log.info(f"Sending SMS for {cam_id}")
        send_motion_sms(message, notif_log)
