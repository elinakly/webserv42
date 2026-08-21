#!/usr/bin/env python3

with open("test.txt", "r") as f:
    data = f.read()

print("Content-Type: text/plain")
print()
print(data)