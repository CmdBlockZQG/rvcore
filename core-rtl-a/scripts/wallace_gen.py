# 华莱士树生成
# 见 MUL_wallace.sv

n = 34 # YLEN

tree = [[] for _ in range(2 * n)]

# p0
for i in range(n + 4):
  tree[i].append("p0[%d]" % i)

# p
for i in range(1, n // 2):
  j = (i - 1) * 2
  tree[j].append("c[%d][%d]" % (i - 1, 0))
  tree[j + 1].append("c[%d][%d]" % (i - 1, 1))
  j += 2
  for k in range(n + 2):
    tree[j + k].append("p[%d][%d]" % (i, k))
  j += n + 2
  if i != n // 2 - 1:
    tree[j].append("1'b1")

# tree compress
w_cnt = 0
fa_cnt = 0
ha_cnt = 0

def wire():
  global w_cnt;
  w = "w[%d]" % w_cnt
  w_cnt += 1
  return w

def FA(a, b, cin):
  global fa_cnt;
  s = wire()
  cout = wire()
  print("fa fa_%d (.a(%s), .b(%s), .cin(%s), .s(%s), .cout(%s));" % (fa_cnt, a, b, cin, s, cout))
  fa_cnt += 1
  return s, cout

def HA(a, b):
  global ha_cnt;
  s = wire()
  cout = wire()
  print("ha ha_%d (.a(%s), .b(%s), .s(%s), .cout(%s));" % (ha_cnt, a, b, s, cout))
  ha_cnt += 1
  return s, cout

while max(map(len, tree)) > 2:
  tree_new = [[] for _ in range(2 * n)]
  for col in range(2 * n):
    while len(tree[col]) > 0:
      s = None
      c = None
      match len(tree[col]):
        case 0: pass
        case 1: s = tree[col].pop()
        case 2: s, c = HA(tree[col].pop(), tree[col].pop())
        case _: s, c = FA(tree[col].pop(), tree[col].pop(), tree[col].pop())
      if s: tree_new[col].append(s)
      if c and col != 2 * n - 1: tree[col + 1].append(c)
  tree = tree_new

print("wire w[%d];" % w_cnt)

for i in range(2):
  l = []
  for col in range(2 * n):
    if len(tree[col]):
      l.append(tree[col].pop())
    else:
      l.append("1'b0")
  l.reverse()
  print("wire [%d:0] row_%d = {%s};" % (2 * n - 1, i, ", ".join(l)))

print("wire [%d:0] prod = row_0 + row_1;" % (2 * n - 1))
