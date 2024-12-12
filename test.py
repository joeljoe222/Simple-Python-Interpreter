a = 0
b = 1
c = 1

if a == 0:
    if b == 0:
        x = 0
    else:
        x = 1
else:
    x = 2

y = 0
if a == 0:
    if b == 1:
        if c == 1:
            y = 1

z = 2
if a == 1:
    if b == 1:
        z = 0
    else:
        z = 1
    y = y - 1
else:
    if b == 1:
        z = 2
    else:
        z = 3
    z = z + 1
    y = y + 1

print("x: ", x)
print("y: ", y)
print("z: ", z)
