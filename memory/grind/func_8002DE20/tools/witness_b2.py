"""Feasibility witness for Ruling 11 (B)(2) as clarified by 9a0543e05 (func_8002DE20).

Runs candidate.c's C logic (after the GTE rotation, so the rotated points are the
inputs; with an identity rotation they are p_k - origin) on one concrete input and
traces cross_a / cross_b at each write. For the test-4 cross_a write and the
test-7 cross_b write it prints the branch conditions taken, the value the variable
held on entry and the value written. C int semantics: `/` truncates toward zero.
"""


def cdiv(a, b):
    q = abs(a) // abs(b)
    return q if (a >= 0) == (b >= 0) else -q


A = (4, 0)                       # obj->unkA8.vx, .vy
B = (0, 4)                       # obj->unkB8.vx, .vy
P = [(-1, 1, -1), (-1, 1, 1), (-3, 5, 1)]   # obj->unk118[0..2] (x, y, z)

max_i = min_i = 0
max_z = min_z = P[0][2]
for i in (1, 2):
    z = P[i][2]
    if z < min_z:
        min_i, min_z = i, z
    elif max_z < z:
        max_i, max_z = i, z
assert not (min_z > 0 or max_z < 0) and not (min_z == 0 and max_z == 0)
dz_a = P[max_i][2] - P[min_i][2] or 1
x1 = P[min_i][0] + cdiv(-P[min_i][2] * (P[max_i][0] - P[min_i][0]), dz_a)
y1 = P[min_i][1] + cdiv(-P[min_i][2] * (P[max_i][1] - P[min_i][1]), dz_a)
mid_i = 3 - min_i - max_i
assert P[mid_i][2] >= 0
dz_b = P[mid_i][2] - P[min_i][2] or 1
x2 = P[min_i][0] + cdiv(-P[min_i][2] * (P[mid_i][0] - P[min_i][0]), dz_b)
y2 = P[min_i][1] + cdiv(-P[min_i][2] * (P[mid_i][1] - P[min_i][1]), dz_b)
cx = cdiv(A[0] + B[0], 3)
cy = cdiv(A[1] + B[1], 3)


def cr(ex, ey, px, py):          # as spelled: E.vy * P.x - E.vx * P.y
    return ey * px - ex * py


print(f"x1,y1={x1},{y1}  x2,y2={x2},{y2}  cx,cy={cx},{cy}")
path = []
a1, b1 = cr(*A, cx, cy), cr(*A, x1, y1)
t1 = (a1 ^ b1) >= 0
path.append(f"T1 (cross_a ^ cross_b) >= 0 is {t1} ({a1}, {b1})")
a, b = a1, b1
assert t1
a2, b2 = cr(*B, cx, cy), cr(*B, x1, y1)
t2 = (a2 ^ b2) >= 0
path.append(f"T2 is {t2} ({a2}, {b2})")
a, b = a2, b2
assert not t2
held_a, a4 = a, cr(*A, cx, cy)
b4 = cr(*A, x2, y2)
print(" ; ".join(path))
print(f"test-4 write cross_a = A x c: variable held {held_a} (= B x c, test 2's value), writes {a4}")
t4 = (a4 ^ b4) >= 0
a, b = a4, b4
assert t4
a5, b5 = cr(*B, cx, cy), cr(*B, x2, y2)
t5 = (a5 ^ b5) >= 0
a, b = a5, b5
assert not t5
held_b, b7 = b, cr(*A, x2, y2)
print(f"T4 is {t4} ({a4}, {b4}); T5 is {t5} ({a5}, {b5})")
print(f"test-7 write cross_b = A x p2: variable held {held_b} (= B x p2, test 5's value), writes {b7}")
assert held_a != a4 and held_b != b7
print("both witnesses hold")
