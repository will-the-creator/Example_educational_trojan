def fake_tool():
    print("this tool claims to do something usefull")
def Evil_trojan():
    filename = "Greekhorse.txt"
with open("Greekhorse.txt", "w") as f:
    f.write("Be Not afraid Example Trojan")
def main():
    fake_tool()
    Evil_trojan()
if __name__ == "__main__":
    main()
