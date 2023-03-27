# Usage:
#
# python3 gen_ranges.py > example_ranges.txt

total_length=6 # digits in total_rows
total_rows = 420000
disjoint_sets = 1000
rows_per_set = 60

def to_row(i):
    s = str(i)
    return "row" + (total_length-len(s))*'0' + s

for i in range(disjoint_sets):
    start = i * total_rows // disjoint_sets
    end = start + rows_per_set - 1
    print("%s, %s" % (to_row(start), to_row(end)))
