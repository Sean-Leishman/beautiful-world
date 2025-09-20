import os

for f in os.listdir("./Image"):
    with open("./Image/"+f, "r") as file:
        m = file.readline().strip()
        dimm = file.readline().strip()
        max = file.readline().strip()

        pixels = file.read()

    max = '300'
    with open("./Image/" + f, "w") as file:
        file.write("P3\n")
        file.write(dimm + "\n")
        file.write(str(max) + "\n")
        file.write(pixels)
