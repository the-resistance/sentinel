# rf-sentinel

Passive RF sweep utility using HackRF for signal detection, band exclusion, and signal persistence logging.

---

## Build Instructions (Kali Linux)

```bash
sudo apt update
sudo apt install -y build-essential libhackrf-dev libsqlite3-dev sqlite3 git
git clone https://github.com/the-resistance/sentinel ~/sentinel
cd ~/sentinel
make
