

## install

```bash
python -m venv .venv
source .venv/bin/activate
pip install -r requirements.txt
```


## use 

the site executes a command through `FLASH_COMMAND`.
add `.env` with:

```bash
FLASK_SECRET="..."
FLASH_COMMAND="esptool.py --chip esp32 --port /dev/ttyUSB0 --baud 921600 write_flash 0x10000 {firmware}"
```

