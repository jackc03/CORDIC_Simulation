import math
#test to see if there is a power of 2 addition sequence so I can avoid any multiplications in my cordic
x = 1
k = 0.607253 * x

# I legit just kept trying different pairs here lol
# picked this one bc its not too many additions and still has low error
y = x * 2**-1 + x * 2**-3 + x * 2**-6 - x * 2**-5 
print(str(k) + " - " + str(y) + " = " + str(y - k) + " or a " + str((y-k) * 100/k) + "% error")
rotations = 0,1,2,3,4,5,6,7,8,9,10,11
angles = []
for rot in rotations:
    angles.append(float("%.4f" % math.atan(2**(-rot))))
print(str(angles))