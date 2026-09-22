import os
import shlex
import subprocess
import threading
from datetime import datetime, timezone
from pathlib import Path
from dotenv import get_key

from flask import Flask, flash, redirect, render_template, request, send_from_directory, url_for
from werkzeug.utils import secure_filename

import serial, threading, time
from collections import deque

BASE_DIR = Path(__file__).resolve().parent
FIRMWARE_DIR = BASE_DIR / "firmware"
LOG_DIR = BASE_DIR / "logs"
FIRMWARE_DIR.mkdir(exist_ok=True)
LOG_DIR.mkdir(exist_ok=True)

app = Flask(__name__)
app.secret_key = get_key(".env", "FLASK_SECRET")
app.config["MAX_CONTENT_LENGTH"] = 32 * 1024 * 1024

FLASH_COMMAND = get_key(".env", "FLASH_COMMAND")
ALLOWED_EXTENSIONS = {".bin", ".hex", ".elf", ".uf2", ".hex"}

flash_lock = threading.Lock()
flash_running = False
current_log = None

serial_buf = deque(maxlen=500)
serial_stop = threading.Event()
serial_conn = None

serial_log_path = None

def reader():
    global serial_conn, serial_log_path
    serial_log_path = LOG_DIR / f"serial_{datetime.now():%Y-%m-%d_%H-%M-%S}.log"
    try:
        serial_conn = serial.Serial("/dev/ttyUSB0", 115200, timeout=1)
        with serial_log_path.open("w", encoding="utf-8") as f:
            while not serial_stop.is_set():
                line = serial_conn.readline()
                if line:
                    text = line.decode("utf-8", errors="replace").rstrip()
                    serial_buf.append(text)
                    f.write(text + "\n")
                    f.flush()
    except Exception as e:
        serial_buf.append(f"ERROR: {e}")
    finally:
        if serial_conn: serial_conn.close()

@app.post("/monitor/start")
def monitor_start():
    serial_stop.clear()
    serial_buf.clear()
    threading.Thread(target=reader, daemon=True).start()
    return redirect(url_for("monitor"))

@app.post("/monitor/stop")
def monitor_stop():
    serial_stop.set()
    return redirect(url_for("monitor"))

@app.get("/monitor/data")
def monitor_data():
    return {"lines": list(serial_buf), "running": not serial_stop.is_set()}


def log_path():
    stamp = datetime.now().strftime("%Y-%m-%d_%H-%M-%S")
    return LOG_DIR / f"flash_{stamp}.log"


def write_log(path, message):
    timestamp = datetime.now(timezone.utc).astimezone().strftime("%H:%M:%S")
    with path.open("a", encoding="utf-8") as f:
        f.write(f"[{timestamp}] {message}\n")


def allowed_firmware(filename):
    return Path(filename).suffix.lower() in ALLOWED_EXTENSIONS


def run_flash(firmware_path, log_file):
    global flash_running
    serial_stop.set()
    time.sleep(0.5)

    try:
        if not FLASH_COMMAND:
            write_log(log_file, "error: flash_command is not configured.")
            write_log(log_file, "set flash_command in your environment and restart flask.")
            return

        command = FLASH_COMMAND.format(firmware=shlex.quote(str(firmware_path)))
        write_log(log_file, f"$ {command}")

        process = subprocess.Popen(
            command,
            shell=True,
            cwd=BASE_DIR,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
            bufsize=1,
        )

        for line in process.stdout:
            write_log(log_file, line.rstrip())

        code = process.wait()
        write_log(log_file, f"flash process exited with code {code}.")
        if code == 0:
            write_log(log_file, "STATUS: firmware flash completed successfully.")
        else:
            write_log(log_file, "STATUS: firmware flash failed.")
    except Exception as exc:
        write_log(log_file, f"ERROR: {type(exc).__name__}: {exc}")
    finally:
        flash_running = False


@app.get("/")
def index():
    logs = sorted(LOG_DIR.glob("*.log"), key=lambda p: p.stat().st_mtime, reverse=True)
    latest = logs[0] if logs else None
    return render_template(
        "index.html",
        flash_running=flash_running,
        latest_log=latest,
        logs=logs[:12],
        flash_configured=bool(FLASH_COMMAND),
    )


@app.post("/flash")
def flash_firmware():
    global flash_running, current_log

    if flash_running:
        flash("a flash operation is already running.", "error")
        return redirect(url_for("index"))

    upload = request.files.get("firmware")
    if not upload or not upload.filename:
        flash("choose a firmware file first.", "error")
        return redirect(url_for("index"))

    filename = secure_filename(upload.filename)
    if not allowed_firmware(filename):
        flash("unsupported firmware type. use .bin, .hex, .elf or .uf2.", "error")
        return redirect(url_for("index"))

    firmware_path = FIRMWARE_DIR / filename
    upload.save(firmware_path)

    current_log = log_path()
    write_log(current_log, f"firmware: {filename}")
    write_log(current_log, "STATUS: starting firmware flash...")

    flash_running = True
    threading.Thread(
        target=run_flash,
        args=(firmware_path, current_log),
        daemon=True,
    ).start()

    flash("flash started. open the monitor to follow the saved log.", "ok")
    return redirect(url_for("monitor"))


@app.get("/monitor")
def monitor():
    logs = sorted(LOG_DIR.glob("*.log"), key=lambda p: p.stat().st_mtime, reverse=True)
    selected = request.args.get("file")
    selected_path = LOG_DIR / Path(selected).name if selected else (logs[0] if logs else None)

    content = ""
    if selected_path and selected_path.exists():
        content = selected_path.read_text(encoding="utf-8", errors="replace")

    return render_template(
        "monitor.html",
        logs=logs[:20],
        selected=selected_path,
        content=content,
        flash_running=flash_running,
    )


@app.get("/logs/<path:filename>")
def download_log(filename):
    return send_from_directory(LOG_DIR, Path(filename).name, as_attachment=True)


@app.get("/health")
def health():
    return {"ok": True, "flash_running": flash_running}


if __name__ == "__main__":
    app.run(host="0.0.0.0", port=int(os.environ.get("PORT", "5000")), debug=False)
